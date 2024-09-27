
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeFunctionCall.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueFunction.h>
#include <tang/computedValue/computedValueFunctionNative.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_function_call_vtable = {
  .name = "FunctionCall",
  .compile_to_bytecode = gta_ast_node_function_call_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_function_call_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_function_call_destroy,
  .print = gta_ast_node_function_call_print,
  .simplify = gta_ast_node_function_call_simplify,
  .analyze = gta_ast_node_function_call_analyze,
  .walk = gta_ast_node_function_call_walk,
};


GTA_Ast_Node_Function_Call * gta_ast_node_function_call_create(GTA_Ast_Node * lhs, GTA_VectorX * arguments, GTA_PARSER_LTYPE location) {
  assert(lhs);
  assert(arguments);

  GTA_Ast_Node_Function_Call * self = gcu_malloc(sizeof(GTA_Ast_Node_Function_Call));
  if (!self) {
    return 0;
  }

  *self = (GTA_Ast_Node_Function_Call) {
    .base = {
      .vtable = &gta_ast_node_function_call_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .lhs = lhs,
    .arguments = arguments,
  };
  return self;
}


void gta_ast_node_function_call_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_FUNCTION_CALL(self));
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;

  gta_ast_node_destroy(function_call->lhs);

  assert(function_call->arguments);
  GTA_VECTORX_DESTROY(function_call->arguments);
  gcu_free(self);
}


void gta_ast_node_function_call_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_FUNCTION_CALL(self));
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;

  assert(indent);
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);

  assert(self->vtable);
  assert(self->vtable->name);
  printf("%s%s\n", indent, self->vtable->name);
  printf("%s  LHS:\n", indent);
  gta_ast_node_print(function_call->lhs, new_indent);

  assert(function_call->arguments);
  assert(function_call->arguments->count ? (bool)function_call->arguments->data : true);
  printf("%s  Arguments:\n", indent);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(function_call->arguments); i++) {
    printf("%s  %zu:\n", indent, i);
    gta_ast_node_print((GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[i]), new_indent);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_function_call_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_FUNCTION_CALL(self));
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;

  GTA_Ast_Node * simplified_lhs = gta_ast_node_simplify(function_call->lhs, variable_map);
  if (simplified_lhs) {
    gta_ast_node_destroy(function_call->lhs);
    function_call->lhs = simplified_lhs;
  }

  assert(function_call->arguments);
  assert(function_call->arguments->count ? (bool)function_call->arguments->data : true);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(function_call->arguments); ++i) {
    GTA_Ast_Node * argument = (GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[i]);
    GTA_Ast_Node * simplified = gta_ast_node_simplify(argument, variable_map);
    if (simplified) {
      gta_ast_node_destroy(argument);
      GTA_TYPEX_P(function_call->arguments->data[i]) = simplified;
    }
  }
  return 0;
}


void gta_ast_node_function_call_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_FUNCTION_CALL(self));
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;

  callback(self, data, return_value);
  gta_ast_node_walk(function_call->lhs, callback, data, return_value);

  assert(function_call->arguments);
  assert(function_call->arguments->count ? (bool)function_call->arguments->data : true);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(function_call->arguments); ++i) {
    GTA_Ast_Node * argument = (GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[i]);
    gta_ast_node_walk(argument, callback, data, return_value);
  }
}


GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_function_call_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  assert(self);
  assert(GTA_AST_IS_FUNCTION_CALL(self));
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;

  GTA_Ast_Node * error = gta_ast_node_analyze(function_call->lhs, program, scope);
  if (error) {
    return error;
  }

  assert(function_call->arguments);
  assert(function_call->arguments->count ? (bool)function_call->arguments->data : true);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(function_call->arguments); ++i) {
    GTA_Ast_Node * argument = (GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[i]);
    error = gta_ast_node_analyze(argument, program, scope);
    if (error) {
      return error;
    }
  }
  return 0;
}


bool gta_ast_node_function_call_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_FUNCTION_CALL(self));
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;

  assert(context);
  assert(context->program);
  assert(context->program->bytecode);
  assert(context->bytecode_offsets);
  GTA_VectorX * b = context->program->bytecode;
  GTA_VectorX * o = context->bytecode_offsets;

  bool error_free = true;

  // Compile the arguments.
  assert(function_call->arguments);
  assert(function_call->arguments->count ? (bool)function_call->arguments->data : true);
  for (size_t i = 0; error_free && (i < GTA_VECTORX_COUNT(function_call->arguments)); ++i) {
    error_free &= true
      && gta_ast_node_compile_to_bytecode((GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[i]), context)
      && GTA_BYTECODE_APPEND(o, b->count)
      && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_SET_NOT_TEMP))
    ;
  }

  return error_free
  // Compile the LHS.
    && gta_ast_node_compile_to_bytecode(function_call->lhs, context)
  // Call the function
    && GTA_BYTECODE_APPEND(o, b->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_CALL))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_VECTORX_COUNT(function_call->arguments)));
}


bool gta_ast_node_function_call_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_FUNCTION_CALL(self));
  GTA_Ast_Node_Function_Call * function_call = (GTA_Ast_Node_Function_Call *)self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  // Offsets
  int32_t vtable_offset = (int32_t)(size_t)(&((GTA_Computed_Value *)0)->vtable);
  int32_t num_arguments_offset = (int32_t)(size_t)(&((GTA_Computed_Value_Function *)0)->num_arguments);
  int32_t pointer_offset = (int32_t)(size_t)(&((GTA_Computed_Value_Function *)0)->pointer);
  int32_t bound_object = (int32_t)(size_t)(&((GTA_Computed_Value_Function_Native *)0)->bound_object);
  int32_t callback = (int32_t)(size_t)(&((GTA_Computed_Value_Function_Native *)0)->callback);
  bool * is_temporary_offset = &((GTA_Computed_Value *)0)->is_temporary;

  // Jump Labels
  GTA_Integer not_a_native_function;
  GTA_Integer not_a_function;
  GTA_Integer argument_count_mismatch;
  GTA_Integer cleanup;
  GTA_Integer restore_frame_pointer;

  assert(function_call->arguments);
  assert(function_call->arguments->count ? (bool)function_call->arguments->data : true);
  size_t num_arguments = function_call->arguments->count;

  // Figure out the stack alignment offsets.
  bool is_stack_padding_needed = !(GTA_VECTORX_COUNT(function_call->arguments) % 2);
  // The total bytes that RSP will be adjusted by:
  //   8                                 ; For the old frame pointer (r12).
  //   (is_stack_padding_needed ? 8 : 0) ; For alignment.
  //   (8 * num_arguments)               ; For the arguments.
  // NOTE: The shadow space is already present in the stack space.
  int32_t total_stack_adjustment = 8 + (is_stack_padding_needed ? 8 : 0) + (8 * num_arguments);
  // It should be a multiple of 16.
  assert((total_stack_adjustment % 16) == 0);
  // r12 is the frame pointer that will need to be restored.
  int32_t r12_offset = GTA_SHADOW_SIZE__X86_64 + total_stack_adjustment - 8;
  // This is the "first" argument that will be pushed onto the stack.  It is
  // actually the last argument of the function call.
  int32_t first_argument_offset = GTA_SHADOW_SIZE__X86_64 + (8 * num_arguments) - 8;

  bool error_free = true
  // Create jump labels.
    && ((not_a_native_function = gta_compiler_context_get_label(context)) >= 0)
    && ((not_a_function = gta_compiler_context_get_label(context)) >= 0)
    && ((argument_count_mismatch = gta_compiler_context_get_label(context)) >= 0)
    && ((cleanup = gta_compiler_context_get_label(context)) >= 0)
    && ((restore_frame_pointer = gta_compiler_context_get_label(context)) >= 0)

  // Adjust the stack pointer, maintaining 16-byte stack alignment and shadow
  // space, as calculated above.
    && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, -total_stack_adjustment)

  // Save the old frame pointer (r12).
  //   mov [rsp + r12_offset], r12
    && gta_mov_ind_reg__x86_64(v, GTA_REG_RSP, GTA_REG_NONE, 0, r12_offset, GTA_REG_R12)
  ;

  // Compile and push the arguments.
  // The entire stack space is already reserved so that the stack is aligned in
  // case an argument needs to call a function.  We just need to put the
  // arguments in the correct place on the stack.
  for (size_t i = 0; error_free && (i < num_arguments); ++i) {
    // Note: The arguments are pushed in reverse order.
    // TODO: Verify the behavior.  Currently, the is_temporary value is set to
    //   false.  This is because the value is being pushed onto the stack and
    //   is being "assigned" to the function's arguments.  Should this be a
    //   copy instead?
    error_free &= true
    // Compile the argument.
      && gta_ast_node_compile_to_binary__x86_64((GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[num_arguments - i - 1]), context)
    // Set is_temporary to 0.
    //   mov byte ptr [rax + is_temporary_offset], 0
      && gta_mov_ind8_imm8__x86_64(v, GTA_REG_RAX, GTA_REG_NONE, 0, (GTA_Integer)is_temporary_offset, 0)
    // "Push" the argument onto the stack.
    //   mov [rsp + first_argument_offset - (i * 8)], rax
      && gta_mov_ind_reg__x86_64(v, GTA_REG_RSP, GTA_REG_NONE, 0, first_argument_offset - (i * 8), GTA_REG_RAX)
    ;
  }

  // Try to call the function.  If the function is not found, then clean up the
  // stack and return an error.
  return error_free
  // Load the lhs into rax.
    && gta_ast_node_compile_to_binary__x86_64(function_call->lhs, context)

  // Set the new frame pointer (r12).
  // NOTE: r12 is expected to point one byte past the last local variable.
  //   lea r12, [rsp + first_argument_offset + 8]
    && gta_lea_reg_ind__x86_64(v, GTA_REG_R12, GTA_REG_RSP, GTA_REG_NONE, 0, first_argument_offset + 8)

  // If the lhs is not a native function, then see if it is a normal function.
  //   mov GTA_X86_64_Scratch1, [rax + vtable_offset]
  //   mov GTA_X86_64_Scratch2, gta_computed_value_function_native_vtable
  //   cmp GTA_X86_64_Scratch1, GTA_X86_64_Scratch2
  //   jne not_a_native_function
    && gta_mov_reg_ind__x86_64(v, GTA_X86_64_Scratch1, GTA_REG_RAX, GTA_REG_NONE, 0, vtable_offset)
    && gta_mov_reg_imm__x86_64(v, GTA_X86_64_Scratch2, (int64_t)&gta_computed_value_function_native_vtable)
    && gta_cmp_reg_reg__x86_64(v, GTA_X86_64_Scratch1, GTA_X86_64_Scratch2)
    && gta_jcc__x86_64(v, GTA_CC_NE, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, not_a_native_function, v->count - 4)

  // Call the native function.
  //   mov GTA_X86_64_R1, [rax + bound_object]
  //   mov GTA_X86_64_R2, GTA_VECTORX_COUNT(function_call->arguments)
  //   lea GTA_X86_64_R3, [rsp + GTA_SHADOW_SIZE__X86_64 - 8]
  //   mov GTA_X86_64_R4, r15
  //   mov rax, [rax + callback]
  //   call rax
  //   jmp cleanup
    && gta_mov_reg_ind__x86_64(v, GTA_X86_64_R1, GTA_REG_RAX, GTA_REG_NONE, 0, bound_object)
    && gta_mov_reg_imm__x86_64(v, GTA_X86_64_R2, GTA_VECTORX_COUNT(function_call->arguments))
    && gta_lea_reg_ind__x86_64(v, GTA_X86_64_R3, GTA_REG_RSP, GTA_REG_NONE, 0, GTA_SHADOW_SIZE__X86_64 - 8)
    && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R4, GTA_REG_R15)
    && gta_mov_reg_ind__x86_64(v, GTA_REG_RAX, GTA_REG_RAX, GTA_REG_NONE, 0, callback)
    && gta_call_reg__x86_64(v, GTA_REG_RAX)
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, cleanup, v->count - 4)

  // If the lhs is not a function, then bail.
  // not_a_native_function:
  //   mov GTA_X86_64_Scratch2, gta_computed_value_function_vtable
  //   cmp GTA_X86_64_Scratch1, GTA_X86_64_Scratch2
  //   jne not_a_function
    && gta_compiler_context_set_label(context, not_a_native_function, v->count)
    && gta_mov_reg_imm__x86_64(v, GTA_X86_64_Scratch2, (int64_t)&gta_computed_value_function_vtable)
    && gta_cmp_reg_reg__x86_64(v, GTA_X86_64_Scratch1, GTA_X86_64_Scratch2)
    && gta_jcc__x86_64(v, GTA_CC_NE, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, not_a_function, v->count - 4)

  // RAX is a function.  If the number of arguments does not match, then bail.
  //   mov GTA_X86_64_Scratch2, [rax + num_arguments_offset]
  //   mov GTA_X86_64_Scratch1, function_call->arguments->count
  //   cmp GTA_X86_64_Scratch2, GTA_X86_64_Scratch1
  //   jne argument_count_mismatch
    && gta_mov_reg_ind__x86_64(v, GTA_X86_64_Scratch2, GTA_REG_RAX, GTA_REG_NONE, 0, num_arguments_offset)
    && gta_mov_reg_imm__x86_64(v, GTA_X86_64_Scratch1, GTA_VECTORX_COUNT(function_call->arguments))
    && gta_cmp_reg_reg__x86_64(v, GTA_X86_64_Scratch2, GTA_X86_64_Scratch1)
    && gta_jcc__x86_64(v, GTA_CC_NE, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, argument_count_mismatch, v->count - 4)

  // Load the function pointer, call it, then clean up.
  // Note: The stack is already aligned.
  //   mov rax, [rax + pointer_offset]
  //   call rax
  //   jmp cleanup
    && gta_mov_reg_ind__x86_64(v, GTA_REG_RAX, GTA_REG_RAX, GTA_REG_NONE, 0, pointer_offset)
    && gta_call_reg__x86_64(v, GTA_REG_RAX)
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, cleanup, v->count - 4)

  // not_a_function:
  //   mov rax, gta_computed_value_error_invalid_function_call
  //   jmp cleanup
    && gta_compiler_context_set_label(context, not_a_function, v->count)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (GTA_Integer)gta_computed_value_error_invalid_function_call)
    && gta_jmp__x86_64(v, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, cleanup, v->count - 4)

  // argument_count_mismatch:
  //   mov rax, gta_computed_value_error_argument_count_mismatch
    && gta_compiler_context_set_label(context, argument_count_mismatch, v->count)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (GTA_Integer)gta_computed_value_error_argument_count_mismatch)
  // Fall-through to cleanup.

  // The cleanup assumes that either the result of the function call or an
  // error is in RAX.  Either way, we need to clean up the stack and restore
  // the frame pointer (r12).
  // cleanup:
    && gta_compiler_context_set_label(context, cleanup, v->count)
  //  mov r12, [rsp + r12_offset]
  //  add rsp, total_stack_adjustment
    && gta_mov_reg_ind__x86_64(v, GTA_REG_R12, GTA_REG_RSP, GTA_REG_NONE, 0, r12_offset)
    && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, total_stack_adjustment)
  ;
}
