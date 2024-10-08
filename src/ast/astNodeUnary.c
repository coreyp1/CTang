
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeBoolean.h>
#include <tang/ast/astNodeFloat.h>
#include <tang/ast/astNodeInteger.h>
#include <tang/ast/astNodeString.h>
#include <tang/ast/astNodeUnary.h>
#include <tang/computedValue/computedValueBoolean.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_unary_vtable = {
  .name = "Unary",
  .compile_to_bytecode = gta_ast_node_unary_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_unary_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_unary_destroy,
  .print = gta_ast_node_unary_print,
  .simplify = gta_ast_node_unary_simplify,
  .analyze = gta_ast_node_unary_analyze,
  .walk = gta_ast_node_unary_walk,
};


GTA_Ast_Node_Unary * gta_ast_node_unary_create(GTA_Ast_Node * expression, GTA_Unary_Type operator_type, GTA_PARSER_LTYPE location) {
  assert(expression);

  GTA_Ast_Node_Unary * self = gcu_malloc(sizeof(GTA_Ast_Node_Unary));
  if (!self) {
    return 0;
  }

  *self = (GTA_Ast_Node_Unary) {
    .base = {
      .vtable = &gta_ast_node_unary_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .expression = expression,
    .operator_type = operator_type,
  };
  return self;
}


void gta_ast_node_unary_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_UNARY(self));
  GTA_Ast_Node_Unary * unary = (GTA_Ast_Node_Unary *) self;

  gta_ast_node_destroy(unary->expression);
  gcu_free(self);
}


void gta_ast_node_unary_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_UNARY(self));
  GTA_Ast_Node_Unary * unary = (GTA_Ast_Node_Unary *) self;

  assert(indent);
  char * new_indent = gcu_malloc(strlen(indent) + 3);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);

  assert(self->vtable);
  assert(self->vtable->name);
  printf("%s%s(%s):\n", indent, self->vtable->name,
    unary->operator_type == GTA_UNARY_TYPE_NEGATIVE
      ? "-"
      : unary->operator_type == GTA_UNARY_TYPE_NOT
        ? "!"
        : "unknown");
  gta_ast_node_print(unary->expression, new_indent);
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_unary_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_UNARY(self));
  GTA_Ast_Node_Unary * unary = (GTA_Ast_Node_Unary *) self;

  assert(variable_map);
  GTA_Ast_Node * simplified_expression = gta_ast_node_simplify(unary->expression, variable_map);
  if (simplified_expression) {
    gta_ast_node_destroy(unary->expression);
    unary->expression = simplified_expression;
  }

  if (GTA_AST_IS_INTEGER(unary->expression)) {
    GTA_Ast_Node_Integer * integer = (GTA_Ast_Node_Integer *) unary->expression;
    if (unary->operator_type == GTA_UNARY_TYPE_NEGATIVE) {
      return (GTA_Ast_Node *)gta_ast_node_integer_create(-integer->value, self->location);
    }
    return (GTA_Ast_Node *)gta_ast_node_boolean_create(!integer->value, self->location);
  }
  else if (GTA_AST_IS_FLOAT(unary->expression)) {
    GTA_Ast_Node_Float * float_ = (GTA_Ast_Node_Float *) unary->expression;
    if (unary->operator_type == GTA_UNARY_TYPE_NEGATIVE) {
      return (GTA_Ast_Node *)gta_ast_node_float_create(-float_->value, self->location);
    }
    return (GTA_Ast_Node *)gta_ast_node_boolean_create(!float_->value, self->location);
  }
  else if (GTA_AST_IS_BOOLEAN(unary->expression)) {
    GTA_Ast_Node_Boolean * boolean = (GTA_Ast_Node_Boolean *) unary->expression;
    if (unary->operator_type == GTA_UNARY_TYPE_NOT) {
      return (GTA_Ast_Node *)gta_ast_node_boolean_create(!boolean->value, self->location);
    }
  }
  else if (GTA_AST_IS_STRING(unary->expression)) {
    GTA_Ast_Node_String * string = (GTA_Ast_Node_String *) unary->expression;
    if (unary->operator_type == GTA_UNARY_TYPE_NOT) {
      return (GTA_Ast_Node *)gta_ast_node_boolean_create(!string->string->byte_length, self->location);
    }
  }
  return 0;
}


void gta_ast_node_unary_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_UNARY(self));
  GTA_Ast_Node_Unary * unary = (GTA_Ast_Node_Unary *) self;

  callback(self, data, return_value);

  gta_ast_node_walk(unary->expression, callback, data, return_value);
}

GTA_Ast_Node * gta_ast_node_unary_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  assert(self);
  assert(GTA_AST_IS_UNARY(self));
  GTA_Ast_Node_Unary * unary = (GTA_Ast_Node_Unary *) self;

  return gta_ast_node_analyze(unary->expression, program, scope);
}


bool gta_ast_node_unary_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_UNARY(self));
  GTA_Ast_Node_Unary * unary_node = (GTA_Ast_Node_Unary *) self;

  assert(context);
  assert(context->program);
  assert(context->program->bytecode);
  assert(context->bytecode_offsets);
  return gta_ast_node_compile_to_bytecode(unary_node->expression, context)
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(unary_node->operator_type == GTA_UNARY_TYPE_NEGATIVE ? GTA_BYTECODE_NEGATIVE : GTA_BYTECODE_NOT));
}

bool gta_ast_node_unary_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_UNARY(self));
  GTA_Ast_Node_Unary * unary_node = (GTA_Ast_Node_Unary *) self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;

  if (unary_node->operator_type == GTA_UNARY_TYPE_NEGATIVE) {
    return true
    // Compile the expression.  The result will be in rax.
      && gta_ast_node_compile_to_binary__x86_64(unary_node->expression, context)
    // gta_computed_value_negative(RAX, is_assignment, context):
    //   mov GTA_X86_64_R1, rax
    //   mov GTA_X86_64_R2, is_assignment
    //   mov GTA_X86_64_R3, context
      && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R1, GTA_REG_RAX)
      && gta_mov_reg_imm__x86_64(v, GTA_X86_64_R2, 0)
      && gta_mov_reg_reg__x86_64(v, GTA_X86_64_R3, GTA_REG_R15)
#if defined(_WIN32) || defined(_WIN64)
    && gta_push_reg__x86_64(v, GTA_REG_RBP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)
    && gta_and_reg_imm__x86_64(v, GTA_REG_RSP, 0xFFFFFFF0)
    // Note: The 32 byte stack allocation is required by the Windows ABI
    // (see the link below and the comment in gta_binary_call__x86_64).
    // However, I do not know why the 40 byte stack allocation is required,
    // but the program crashes otherwise.
    // TODO: Investigate this further.
    //
    //   add rsp, -40
      && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, 0)
    //   push r15      ; context (the fifth argument)
      && gta_push_reg__x86_64(v, GTA_REG_R15)
    //   add rsp, -32  ; Allocate space for the function call.
      && gta_add_reg_imm__x86_64(v, GTA_REG_RSP, -32)
    //   call func
      && gta_binary_call__x86_64(v, (uint64_t)gta_computed_value_negative)
    // Restore the stack after the function call.
    //   mov rsp, rbp
    //   pop rbp
      && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_RBP)
      && gta_pop_reg__x86_64(v, GTA_REG_RBP);
#else
     && gta_binary_call__x86_64(v, (uint64_t)gta_computed_value_negative);
#endif
  }

  // Get the offset of is_true.
  int32_t is_true_offset = (int32_t)(size_t)(&((GTA_Computed_Value *)0)->is_true);
  //   lea GTA_X86_64_Scratch1, [rax + is_true_offset]
  //   mov rax, gta_computed_value_boolean_true
  //   mov GTA_X86_64_Scratch2, gta_computed_value_boolean_false
  //   cmp byte ptr [GTA_X86_64_Scratch1], 0
  //   cmovne rax, GTA_X86_64_Scratch2
  return true
    // Compile the expression.  The result will be in rax.
    && gta_ast_node_compile_to_binary__x86_64(unary_node->expression, context)
    && gta_lea_reg_ind__x86_64(v, GTA_X86_64_Scratch1, GTA_REG_RAX, GTA_REG_NONE, 0, is_true_offset)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (GTA_Integer)gta_computed_value_boolean_true)
    && gta_mov_reg_imm__x86_64(v, GTA_X86_64_Scratch2, (GTA_Integer)gta_computed_value_boolean_false)
    && gta_cmp_ind8_imm8__x86_64(v, GTA_X86_64_Scratch1, GTA_REG_NONE, 0, 0, 0)
    && gta_cmovcc_reg_reg__x86_64(v, GTA_CC_NE, GTA_REG_RAX, GTA_X86_64_Scratch2);
}
