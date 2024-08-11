
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodePeriod.h>

GTA_Ast_Node_VTable gta_ast_node_period_vtable = {
  .name = "Period",
  .compile_to_bytecode = 0,
  .compile_to_binary__x86_64 = 0,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_period_destroy,
  .print = gta_ast_node_period_print,
  .simplify = gta_ast_node_period_simplify,
  .analyze = 0,
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


void gta_ast_node_period_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  assert(self);
  assert(GTA_AST_IS_PERIOD(self));
  GTA_Ast_Node_Period * period = (GTA_Ast_Node_Period *) self;

  callback(self, data, return_value);
  gta_ast_node_walk(period->lhs, callback, data, return_value);
}
