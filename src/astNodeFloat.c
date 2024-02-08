
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include "tang/astNodeFloat.h"
#include "tang/macros.h"

GTA_Ast_Node_VTable gta_ast_node_float_vtable = {
  .name = "Float",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_float_destroy,
  .print = gta_ast_node_float_print,
  .simplify = gta_ast_node_float_simplify,
  .walk = gta_ast_node_float_walk,
};

GTA_Ast_Node_Float * gta_ast_node_float_create(GCU_float64_t value, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Float * self = gcu_malloc(sizeof(GTA_Ast_Node_Float));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_float_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_FLOAT;
  self->value = value;
  return self;
}

void gta_ast_node_float_destroy(GTA_Ast_Node * self) {
  gcu_free(self);
}

void gta_ast_node_float_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Float * float_node = (GTA_Ast_Node_Float *) self;
  printf("%s%s: %f\n", indent, self->vtable->name, float_node->value);
}

GTA_Ast_Node * gta_ast_node_float_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}

void gta_ast_node_float_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}
