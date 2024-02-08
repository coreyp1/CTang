
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include "tang/astNodeBoolean.h"
#include "tang/macros.h"

GTA_Ast_Node_VTable gta_ast_node_boolean_vtable = {
  .name = "Boolean",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_boolean_destroy,
  .print = gta_ast_node_boolean_print,
  .simplify = gta_ast_node_boolean_simplify,
  .walk = gta_ast_node_boolean_walk,
};

GTA_Ast_Node_Boolean * gta_ast_node_boolean_create(bool value, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Boolean * self = gcu_malloc(sizeof(GTA_Ast_Node_Boolean));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_boolean_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_BOOLEAN;
  self->value = value;
  return self;
}

void gta_ast_node_boolean_destroy(GTA_Ast_Node * self) {
  gcu_free(self);
}

void gta_ast_node_boolean_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Boolean * boolean = (GTA_Ast_Node_Boolean *) self;
  printf("%s%s: %s\n", indent, self->vtable->name, boolean->value ? "true" : "false");
}

GTA_Ast_Node * gta_ast_node_boolean_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}

void gta_ast_node_boolean_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}
