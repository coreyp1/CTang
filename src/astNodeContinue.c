
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include "tang/astNodeContinue.h"
#include "tang/macros.h"

GTA_Ast_Node_VTable gta_ast_node_continue_vtable = {
  .name = "Continue",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_continue_destroy,
  .print = gta_ast_node_continue_print,
  .simplify = gta_ast_node_continue_simplify,
  .walk = gta_ast_node_continue_walk,
};

GTA_Ast_Node_Continue * gta_ast_node_continue_create(GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Continue * self = gcu_malloc(sizeof(GTA_Ast_Node_Continue));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_continue_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  return self;
}

void gta_ast_node_continue_destroy(GTA_Ast_Node * self) {
  gcu_free(self);
}

void gta_ast_node_continue_print(GTA_Ast_Node * self, const char * indent) {
  printf("%s%s\n", indent, self->vtable->name);
}

GTA_Ast_Node * gta_ast_node_continue_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}

void gta_ast_node_continue_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}
