
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeFunctionCall.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueFunction.h>
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

  // Compile the arguments.
  assert(function_call->arguments);
  assert(function_call->arguments->count ? (bool)function_call->arguments->data : true);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(function_call->arguments); ++i) {
    if (!gta_ast_node_compile_to_bytecode((GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[i]), context)) {
      return false;
    }
  }

  return true
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

  // Jump Labels
  GTA_Integer not_a_function;
  GTA_Integer argument_count_mismatch;
  GTA_Integer cleanup;
  GTA_Integer restore_frame_pointer;

  assert(function_call->arguments);
  bool stack_padding_needed = GTA_VECTORX_COUNT(function_call->arguments) % 2;

  bool error_free = true
  // Create jump labels.
    && ((not_a_function = gta_compiler_context_get_label(context)) >= 0)
    && ((argument_count_mismatch = gta_compiler_context_get_label(context)) >= 0)
    && ((cleanup = gta_compiler_context_get_label(context)) >= 0)
    && ((restore_frame_pointer = gta_compiler_context_get_label(context)) >= 0)

  // Push the frame pointer (r12) onto the stack.
    && gta_push_reg__x86_64(v, GTA_REG_R12)

  // Prepare for stack alignment of the function call (much later).
  // We don't know the current alignment, so we will align to 16 bytes and then
  // pad it according to the number of arguments (8 bytes each).  Even though
  // it's early, doing this now will make it so that the arguments are pushed
  // onto the stack in the correct alignment.
  //   push rbp
  //   mov rbp, rsp
  //   and rsp, 0xFFFFFFF0
  //   add rsp, stack_padding_needed ? -8 : 0
    && gta_push_reg__x86_64(v, GTA_REG_RBP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)
    && gta_and_reg_imm__x86_64(v, GTA_REG_RSP, 0xFFFFFFF0)
    && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, stack_padding_needed ? -8 : 0)
  // Set the new frame pointer (r12) to the current stack pointer (rsp).
  //   mov r12, rsp
    && gta_mov_reg_reg__x86_64(v, GTA_REG_R12, GTA_REG_RSP)
  ;

  // Compile and push the arguments.
  assert(function_call->arguments->count ? (bool)function_call->arguments->data : true);
  for (size_t i = 0; error_free && (i < GTA_VECTORX_COUNT(function_call->arguments)); ++i) {
    // TODO: Does these values need to be marked as not temporary?
    error_free &= true
      && gta_ast_node_compile_to_binary__x86_64((GTA_Ast_Node *)GTA_TYPEX_P(function_call->arguments->data[i]), context)
      && gta_push_reg__x86_64(v, GTA_REG_RAX);
  }

  // Try to call the function.  If the function is not found, then clean up the
  // stack and return an error.
  return error_free
  // Load the lhs into rax.
    && gta_ast_node_compile_to_binary__x86_64(function_call->lhs, context)

  // If the lhs is not a function, then bail.
  //   mov rdx, [rax + vtable_offset]
  //   mov rcx, gta_computed_value_function_vtable
  //   cmp rdx, rcx
  //   jne not_a_function
    && gta_mov_reg_ind__x86_64(v, GTA_REG_RDX, GTA_REG_RAX, GTA_REG_NONE, 0, vtable_offset)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RCX, (int64_t)&gta_computed_value_function_vtable)
    && gta_cmp_reg_reg__x86_64(v, GTA_REG_RDX, GTA_REG_RCX)
    && gta_jcc__x86_64(v, GTA_CC_NE, 0xDEADBEEF)
    && gta_compiler_context_add_label_jump(context, not_a_function, v->count - 4)

  // RAX is a function.  If the number of arguments does not match, then bail.
  //   mov rdx, [rax + num_arguments_offset]
  //   mov rcx, function_call->arguments->count
  //   cmp rdx, rcx
  //   jne argument_count_mismatch
    && gta_mov_reg_ind__x86_64(v, GTA_REG_RDX, GTA_REG_RAX, GTA_REG_NONE, 0, num_arguments_offset)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RCX, GTA_VECTORX_COUNT(function_call->arguments))
    && gta_cmp_reg_reg__x86_64(v, GTA_REG_RDX, GTA_REG_RCX)
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
  //   add rsp, 8 * (function_call->arguments->count + (stack_padding_needed ? 1 : 0))
  //   mov rsp, rbp
  //   pop rbp
  //   pop r12
    && gta_compiler_context_set_label(context, cleanup, v->count)
    && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, 8 * (function_call->arguments->count + (stack_padding_needed ? 1 : 0)))
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_RBP)
    && gta_pop_reg__x86_64(v, GTA_REG_RBP)
    && gta_pop_reg__x86_64(v, GTA_REG_R12)
  ;
}
