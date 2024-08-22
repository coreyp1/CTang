
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include <tang/ast/astNodePeriod.h>
#include <tang/program/binary.h>

GTA_Ast_Node_VTable gta_ast_node_period_vtable = {
  .name = "Period",
  .compile_to_bytecode = gta_ast_node_period_compile_to_bytecode,
  .compile_to_binary__x86_64 = gta_ast_node_period_compile_to_binary__x86_64,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_period_destroy,
  .print = gta_ast_node_period_print,
  .simplify = gta_ast_node_period_simplify,
  .analyze = gta_ast_node_period_analyze,
  .walk = gta_ast_node_period_walk,
};


GTA_Ast_Node_Period * gta_ast_node_period_create(GTA_Ast_Node * lhs, const char * rhs, GTA_PARSER_LTYPE location) {
  assert(lhs);
  assert(rhs);

  GTA_Ast_Node_Period * self = gcu_malloc(sizeof(GTA_Ast_Node_Period));
  if (!self) {
    return 0;
  }

  *self = (GTA_Ast_Node_Period) {
    .base = {
      .vtable = &gta_ast_node_period_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .lhs = lhs,
    .rhs = rhs,
  };
  return self;
}


void gta_ast_node_period_destroy(GTA_Ast_Node * self) {
  assert(self);
  assert(GTA_AST_IS_PERIOD(self));
  GTA_Ast_Node_Period * period = (GTA_Ast_Node_Period *) self;

  gta_ast_node_destroy(period->lhs);
  gcu_free((void *)period->rhs);
  gcu_free(self);
}


void gta_ast_node_period_print(GTA_Ast_Node * self, const char * indent) {
  assert(self);
  assert(GTA_AST_IS_PERIOD(self));
  GTA_Ast_Node_Period * period = (GTA_Ast_Node_Period *) self;

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
  gta_ast_node_print(period->lhs, new_indent);

  printf("%s  RHS: %s\n", indent, period->rhs);
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_period_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  assert(self);
  assert(GTA_AST_IS_PERIOD(self));
  GTA_Ast_Node_Period * period = (GTA_Ast_Node_Period *) self;

  GTA_Ast_Node * simplified_lhs = gta_ast_node_simplify(period->lhs, variable_map);
  if (simplified_lhs) {
    gta_ast_node_destroy(period->lhs);
    period->lhs = simplified_lhs;
  }
  return 0;
}


GTA_Ast_Node * gta_ast_node_period_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope) {
  assert(self);
  assert(GTA_AST_IS_PERIOD(self));
  GTA_Ast_Node_Period * period = (GTA_Ast_Node_Period *) self;

  return gta_ast_node_analyze(period->lhs, program, scope);
}


void gta_ast_node_period_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_PERIOD(self));
  GTA_Ast_Node_Period * period = (GTA_Ast_Node_Period *) self;

  callback(self, data, return_value);
  gta_ast_node_walk(period->lhs, callback, data, return_value);
}


bool gta_ast_node_period_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_PERIOD(self));
  GTA_Ast_Node_Period * period = (GTA_Ast_Node_Period *) self;

  return true
    && gta_ast_node_compile_to_bytecode(period->lhs, context)
    && GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_PERIOD))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_STRING_HASH(period->rhs, strlen(period->rhs))))
    && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_P((void *)period->rhs))
  ;
}


bool gta_ast_node_period_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context) {
  assert(self);
  assert(GTA_AST_IS_PERIOD(self));
  GTA_Ast_Node_Period * period = (GTA_Ast_Node_Period *) self;

  assert(context);
  assert(context->binary_vector);
  GCU_Vector8 * v = context->binary_vector;
  GTA_UInteger attribute_hash = GTA_STRING_HASH(period->rhs, strlen(period->rhs));

  return true
  // Compile the LHS
    && gta_ast_node_compile_to_binary__x86_64(period->lhs, context)
  // The result is in RAX.  Call the period function.
  //   mov rdi, rax
  //   mov rsi, attribute_hash
  //   mov rdx, r15             ; context
  //   mov rax, gta_computed_value_period
  //   push rbp
  //   mov rbp, rsp
  //   and rsp, 0xFFFFFFF0
  //   call rax
  //   mov rsp, rbp
  //   pop rbp
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RDI, GTA_REG_RAX)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RSI, attribute_hash)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RDX, GTA_REG_R15)
    && gta_mov_reg_imm__x86_64(v, GTA_REG_RAX, (int64_t)gta_computed_value_period)
    && gta_push_reg__x86_64(v, GTA_REG_RBP)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RBP, GTA_REG_RSP)
    && gta_and_reg_imm__x86_64(v, GTA_REG_RSP, 0xFFFFFFF0)
    && gta_call_reg__x86_64(v, GTA_REG_RAX)
    && gta_mov_reg_reg__x86_64(v, GTA_REG_RSP, GTA_REG_RBP)
    && gta_pop_reg__x86_64(v, GTA_REG_RBP)
  ;
}
