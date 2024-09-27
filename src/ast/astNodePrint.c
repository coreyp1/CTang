
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodePrint.h>
#include <tang/program/binary.h>
#include <tang/computedValue/computedValueError.h>

GTA_Ast_Node_VTable gta_ast_node_print_vtable = {
  .name = "Print",
  .compile_to_bytecode = gta_ast_node_print_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_print_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_print_destroy,
  .print = gta_ast_node_print_print,
  .simplify = gta_ast_node_print_simplify,
  .analyze = gta_ast_node_print_analyze,
  .walk = gta_ast_node_print_walk,
};


GTA_Ast_Node_Print * gta_ast_node_print_create(GTA_Ast_Node * expression, GTA_PARSER_LTYPE location) {
  assert(expression);

  GTA_Ast_Node_Print * self = gcu_malloc(sizeof(GTA_Ast_Node_Print));
  if (!self) {
    return 0;
  }

  *self = (GTA_Ast_Node_Print) {
    .base = {
      .vtable = &gta_ast_node_print_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .expression = expression,
  };
  return self;
}


void gta_ast_node_print_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_PRINT(self));
  GTA_Ast_Node_Print * print = (GTA_Ast_Node_Print *)self;

  gta_ast_node_destroy(print->expression);
  gcu_free(self);
}


void gta_ast_node_print_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_PRINT(self));
  GTA_Ast_Node_Print * print = (GTA_Ast_Node_Print *)self;

  assert(indent);
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 3);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);

  assert(self->vtable);
  assert(self->vtable->name);
  printf("%s%s():\n", indent, self->vtable->name);

  gta_ast_node_print(print->expression, new_indent);
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_print_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_PRINT(self));
  GTA_Ast_Node_Print * print = (GTA_Ast_Node_Print *)self;

  GTA_Ast_Node * simplified_expression = gta_ast_node_simplify(print->expression, variable_map);
  if (simplified_expression) {
    gta_ast_node_destroy(print->expression);
    print->expression = simplified_expression;
  }
  return 0;
}


GTA_Ast_Node * gta_ast_node_print_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  assert(self);
  assert(GTA_AST_IS_PRINT(self));
  GTA_Ast_Node_Print * print = (GTA_Ast_Node_Print *)self;

  return gta_ast_node_analyze(print->expression, program, scope);
}


void gta_ast_node_print_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_PRINT(self));
  GTA_Ast_Node_Print * print = (GTA_Ast_Node_Print *)self;

  callback(self, data, return_value);
  callback(print->expression, data, return_value);
}


bool gta_ast_node_print_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_PRINT(self));
  GTA_Ast_Node_Print * print = (GTA_Ast_Node_Print *)self;

  assert(context);
  assert(context->program);
  assert(context->program->bytecode);
  assert(context->bytecode_offsets);
  return gta_ast_node_compile_to_bytecode(print->expression, context)
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_PRINT));
}


bool gta_ast_node_print_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_PRINT(self));
  GTA_Ast_Node_Print * print = (GTA_Ast_Node_Print *)self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  bool print_to_stdout = context->program->flags & GTA_PROGRAM_FLAG_PRINT_TO_STDOUT;

  // Memory offsets (for use by the generated assembly code).
  GTA_Unicode_String * * context_output_offset = &((GTA_Execution_Context *)0)->output;
  size_t * context_output_byte_length_offset = &((GTA_Unicode_String *)0)->byte_length;
  GTA_Computed_Value_VTable * * vtable_offset = & ((GTA_Computed_Value *)0)->vtable;
  GTA_Unicode_String *(**vtable_print_offset)(GTA_Computed_Value *, GTA_Execution_Context *) = &((GTA_Computed_Value_VTable *)0)->print;
  // NOTE: These intentionally use the same offset as
  //   `original_value_stack_offset`.  Their usage is all mutually exclusive,
  // but giving them different names makes the code more readable.
  int32_t original_value_stack_offset = GTA_SHADOW_SIZE__X86_64;
  int32_t string_to_be_printed_stack_offset = GTA_SHADOW_SIZE__X86_64;
  int32_t concatenated_string_stack_offset = GTA_SHADOW_SIZE__X86_64;

  // Jump labels.
  GTA_Integer success_return_null;
  GTA_Integer no_string_created_by_print;
  GTA_Integer output_string_not_empty;
  GTA_Integer error_out_of_memory;
  GTA_Integer print_return;

  // JIT the print(<expression>) function.
  return true
    && gta_nop__x86_64(v)
    && gta_nop__x86_64(v)
  // Allocate space on the stack for temporary variables.
  //   add rsp, -16
    && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, -16)
  // Compile the expression to be printed.  The result will be in RAX.
    && gta_ast_node_compile_to_binary__x86_64(print->expression, context)
  // Create the labels.
    && ((success_return_null = gta_compiler_context_get_label(context)) >= 0)
    && ((no_string_created_by_print = gta_compiler_context_get_label(context)) >= 0)
    && ((output_string_not_empty = gta_compiler_context_get_label(context)) >= 0)
    && ((error_out_of_memory = gta_compiler_context_get_label(context)) >= 0)
    && ((print_return = gta_compiler_context_get_label(context)) >= 0)
  // ; Save (push) the computed value to the stack so that we can check it's
  // ; vtable later (if necessary) in the `no_string_created_by_print` section.
  //   mov [rsp + original_value_stack_offset], rax
    && gta_mov_ind_reg__x86_64(v, GTA_REG_RSP, GTA_REG_NONE, 0, original_value_stack_offset, GTA_REG_RAX)

  // ; gta_computed_value_print(rax, context)
  //   mov GTA_X86_64_R1, rax
  //   mov GTA_X86_64_R2, r15
    && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R1, GTA_REG_RAX)
    && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R2, GTA_REG_R15)
    && gta_binary_call__x86_64(v, (uint64_t)gta_computed_value_print)

  // ; RAX contains a pointer to the unicode string.
  //   xor GTA_X86_64_Scratch1, GTA_X86_64_Scratch1
  //   cmp rax, GTA_X86_64_Scratch1
  //   je no_string_created_by_print
    && gta_xor_reg_reg__x86_64(v, GTA_X86_64_Scratch1, GTA_X86_64_Scratch1)
    && gta_cmp_reg_reg__x86_64(v, GTA_REG_RAX, GTA_X86_64_Scratch1)
    && gta_jcc__x86_64(v, GTA_CC_E, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, no_string_created_by_print, v->count - 4)

  // ; Compile differently based on whether or not the string was printed
  // ; to stdout.
  // ; NOTE: original_value_stack_offset is no longer used, so we can reuse
  // ; its stack location.
    && (print_to_stdout
      ? true
      // The string was printed to stdout, so clean up and return NULL.
      // Destroy the returned string.
      //    mov GTA_X86_64_R1, rax
      //    call gta_unicode_string_destroy(GTA_X86_64_R1)
        && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R1, GTA_REG_RAX)
        && gta_binary_call__x86_64(v, (uint64_t)gta_unicode_string_destroy)
      //   jmp success_return_null
        && gta_jmp__x86_64(v, 0xDEADBEEF)
        && gta_compiler_context_add_label_jump(context, success_return_null, v->count - 4)

      : true
      // The string was not printed to stdout, so concatenate it with the
      // existing output string.
      // ; Check if the existing output string is empty.
      // ; Load context->output->byte_length into GTA_X86_64_Scratch2.
      //   mov GTA_X86_64_R1, [r15 + context_output_offset]
      //   mov GTA_X86_64_Scratch2, [GTA_X86_64_R1 + context_output_byte_length_offset]
      //   cmp GTA_X86_64_Scratch2, GTA_X86_64_Scratch1
      //   jne output_string_not_empty
        && gta_mov_reg_ind__x86_64(v, GTA_X86_64_R1, GTA_REG_R15, GTA_REG_NONE, 0, (size_t)context_output_offset)
        && gta_mov_reg_ind__x86_64(v, GTA_X86_64_Scratch2, GTA_X86_64_R1, GTA_REG_NONE, 0, (size_t)context_output_byte_length_offset)
        && gta_cmp_reg_reg__x86_64(v, GTA_X86_64_Scratch2, GTA_X86_64_Scratch1)
        && gta_jcc__x86_64(v, GTA_CC_NE, 0xDEADBEEF)
        && gta_compiler_context_add_label_jump(context, output_string_not_empty, v->count - 4)

      // ; This is the first string to be printed, so we can just adopt it.
      // ; Set the context output string to the current string.
      //   mov [r15 + context_output_offset], rax
        && gta_mov_ind_reg__x86_64(v, GTA_REG_R15, GTA_REG_NONE, 0, (size_t)context_output_offset, GTA_REG_RAX)

      // ; gta_unicode_string_destroy(GTA_X86_64_R1)
      // ;   (Note: GTA_X86_64_R1 was already set.)
        && gta_binary_call__x86_64(v, (uint64_t)gta_unicode_string_destroy)

      //   jmp success_return_null
        && gta_jmp__x86_64(v, 0xDEADBEEF)
        && gta_compiler_context_add_label_jump(context, success_return_null, v->count - 4)

      // output_string_not_empty:
          && gta_compiler_context_set_label(context, output_string_not_empty, v->count)

      // ; The output string is not empty, so concatenate the new string with
      // ; the existing output string.
      // ; First, save the "string to be printed" to the stack.  If the
      // ; concatenation fails, we will need to clean up the "string to be
      // ; printed" by destroying it.
      //   mov [rsp + string_to_be_printed_stack_offset], rax ; The unicode "string to be printed".
        && gta_mov_ind_reg__x86_64(v, GTA_REG_RSP, GTA_REG_NONE, 0, string_to_be_printed_stack_offset, GTA_REG_RAX)

      // ; Concatenate the string with the output.
      // ; gta_unicode_string_concat(GTA_X86_64_R1, GTA_X86_64_R2)
      //   mov GTA_X86_64_R1, [r15 + context_output_offset]
      //   mov GTA_X86_64_R2, rax
        && gta_mov_reg_ind__x86_64(v, GTA_X86_64_R1, GTA_REG_R15, GTA_REG_NONE, 0, (size_t)context_output_offset)
        && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R2, GTA_REG_RAX)
        && gta_binary_call__x86_64(v, (uint64_t)gta_unicode_string_concat)
      // ; RAX contains the concatenated string (or NULL if the concatenation
      // ; failed).

      // ; "pop" the "string to be printed" into GTA_X86_64_R1.
      //   mov GTA_X86_64_R1, [rsp + string_to_be_printed_stack_offset]
        && gta_mov_reg_ind__x86_64(v, GTA_X86_64_R1, GTA_REG_RSP, GTA_REG_NONE, 0, string_to_be_printed_stack_offset)

      // ; "push" the concatenated string back onto the stack.
      //   mov [rsp + concatenated_string_stack_offset], rax
        && gta_mov_ind_reg__x86_64(v, GTA_REG_RSP, GTA_REG_NONE, 0, concatenated_string_stack_offset, GTA_REG_RAX)

      // ; Destroy the "string to be printed" since it has been concatenated,
      // ; but save the concatenated string to the stack.
      // ; gta_unicode_string_destroy(GTA_X86_64_R1)
        && gta_binary_call__x86_64(v, (uint64_t)gta_unicode_string_destroy)

      // ; Verify that the contatenation was successful.
      // ; "pop" the concatenated string into RAX.
      //   mov rax, [rsp + concatenated_string_stack_offset]
        && gta_mov_reg_ind__x86_64(v, GTA_REG_RAX, GTA_REG_RSP, GTA_REG_NONE, 0, concatenated_string_stack_offset)
      //   cmp rax, GTA_X86_64_Scratch1
      //   je error_out_of_memory
        && gta_cmp_reg_reg__x86_64(v, GTA_REG_RAX, GTA_X86_64_Scratch1)
        && gta_jcc__x86_64(v, GTA_CC_E, 0xDEADBEEF)
        && gta_compiler_context_add_label_jump(context, error_out_of_memory, v->count - 4)

      // ; No failure, so adopt the new string.
      // ; gta_unicode_string_destroy(GTA_X86_64_R1)
      //   mov GTA_X86_64_R1, [r15 + context_output_offset]
      //   mov [r15 + context_output_offset], rax
        && gta_mov_reg_ind__x86_64(v, GTA_X86_64_R1, GTA_REG_R15, GTA_REG_NONE, 0, (size_t)context_output_offset)
        && gta_mov_ind_reg__x86_64(v, GTA_REG_R15, GTA_REG_NONE, 0, (size_t)context_output_offset, GTA_REG_RAX)
        && gta_binary_call__x86_64(v, (uint64_t)gta_unicode_string_destroy)

      //   jmp success_return_null
        && gta_jmp__x86_64(v, 0xDEADBEEF)
        && gta_compiler_context_add_label_jump(context, success_return_null, v->count - 4)
    )

  // no_string_created_by_print:
    && gta_compiler_context_set_label(context, no_string_created_by_print, v->count)

  // ; No string was produced by the print function.  Is this an error?
  // ; Check to see the function pointer in computed_value->vtable->print.
  //   mov rax, [rsp + original_value_stack_offset]
  //   mov GTA_X86_64_Scratch1, [rax + vtable_offset]
  //   mov GTA_X86_64_Scratch1, [GTA_X86_64_Scratch1 + vtable_print_offset]
    && gta_mov_reg_ind__x86_64(v, GTA_REG_RAX, GTA_REG_RSP, GTA_REG_NONE, 0, original_value_stack_offset)
    && gta_mov_reg_ind__x86_64(v, GTA_X86_64_Scratch1, GTA_REG_RAX, GTA_REG_NONE, 0, (size_t)vtable_offset)
    && gta_mov_reg_ind__x86_64(v, GTA_X86_64_Scratch1, GTA_X86_64_Scratch1, GTA_REG_NONE, 0, (size_t)vtable_print_offset)
  // ; Set a default error message.
  //   mov rax, gta_computed_value_error_out_of_memory
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (size_t)gta_computed_value_error_out_of_memory)
  // ; Set other possible error messages.
  //   mov GTA_X86_64_R4, gta_computed_value_print_not_implemented
  //   mov GTA_X86_64_Scratch2, gta_computed_value_print_not_supported
  //   mov GTA_X86_64_R1, gta_computed_value_null
    && gta_mov_reg_imm__x86_64(v, GTA_X86_64_R4, (size_t)gta_computed_value_print_not_implemented)
    && gta_mov_reg_imm__x86_64(v, GTA_X86_64_Scratch2, (size_t)gta_computed_value_print_not_supported)
    && gta_mov_reg_imm__x86_64(v, GTA_X86_64_R1, (size_t)gta_computed_value_null)
  // ; Set the error message based on the function pointer.
  // ; Use conditional moves so that the code is branchless.
  //   cmp GTA_X86_64_Scratch1, GTA_X86_64_R4
  //   cmove rax, GTA_X86_64_R1
  //   cmp GTA_X86_64_Scratch1, GTA_X86_64_Scratch2
  //   cmove rax, GTA_X86_64_R1
    && gta_cmp_reg_reg__x86_64(v, GTA_X86_64_Scratch1, GTA_X86_64_R4)
    && gta_cmovcc_reg_reg__x86_64(v, GTA_CC_E, GTA_REG_RAX, GTA_X86_64_R1)
    && gta_cmp_reg_reg__x86_64(v, GTA_X86_64_Scratch1, GTA_X86_64_Scratch2)
    && gta_cmovcc_reg_reg__x86_64(v, GTA_CC_E, GTA_REG_RAX, GTA_X86_64_R1)
  // ; That's it.  Return.
  //   jmp print_return
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, print_return, v->count - 4)

  // error_out_of_memory:
  //   mov rax, gta_computed_value_error_out_of_memory
  //   jmp print_return
    && gta_compiler_context_set_label(context, error_out_of_memory, v->count)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (size_t)gta_computed_value_error_out_of_memory)
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, print_return, v->count - 4)

  // success_return_null:
  //   mov rax, gta_computed_value_null
    && gta_compiler_context_set_label(context, success_return_null, v->count)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (size_t)gta_computed_value_null)

  // print_return:
    && gta_compiler_context_set_label(context, print_return, v->count)
  // Restore the stack.
  //   add rsp, 16
    && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, 16)
    && gta_nop__x86_64(v)
    && gta_nop__x86_64(v)
  ;
}
