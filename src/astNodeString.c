
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include "tang/astNodeString.h"
#include "tang/macros.h"

GTA_Ast_Node_VTable gta_ast_node_string_vtable = {
  .name = "String",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_string_destroy,
  .print = gta_ast_node_string_print,
  .simplify = gta_ast_node_string_simplify,
  .walk = gta_ast_node_string_walk,
};

GTA_Ast_Node_String * gta_ast_node_string_create(GTA_Unicode_String * string, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_String * self = gcu_malloc(sizeof(GTA_Ast_Node_String));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_string_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_STRING;
  self->string = string;
  return self;
}

void gta_ast_node_string_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_String * string = (GTA_Ast_Node_String *)self;
  gta_unicode_string_destroy(string->string);
  gcu_free(self);
}

void gta_ast_node_string_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_String * string = (GTA_Ast_Node_String *)self;
  printf("%s%s: \"%s\"\n", indent, self->vtable->name, string->string->buffer);
}

GTA_Ast_Node * gta_ast_node_string_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}

void gta_ast_node_string_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}
