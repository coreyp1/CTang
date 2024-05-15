
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeLibrary.h>

GTA_Ast_Node_VTable gta_ast_node_library_vtable = {
  .name = "Library",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_library_destroy,
  .print = gta_ast_node_library_print,
  .simplify = gta_ast_node_library_simplify,
  .analyze = 0,
  .walk = gta_ast_node_library_walk,
};


GTA_Ast_Node_Library * gta_ast_node_library_create(const char * identifier, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Library * self = gcu_malloc(sizeof(GTA_Ast_Node_Library));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_library_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->identifier = identifier;
  self->hash = GTA_STRING_HASH(identifier, strlen(identifier));
  return self;
}


void gta_ast_node_library_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Library * library = (GTA_Ast_Node_Library *) self;
  gcu_free((void *)library->identifier);
  gcu_free(self);
}


void gta_ast_node_library_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Library * library = (GTA_Ast_Node_Library *) self;
  printf("%s%s: %s\n", indent, self->vtable->name, library->identifier);
}


GTA_Ast_Node * gta_ast_node_library_simplify(GTA_MAYBE_UNUSED(GTA_Ast_Node * self), GTA_MAYBE_UNUSED(GTA_Ast_Simplify_Variable_Map * variable_map)) {
  return 0;
}


void gta_ast_node_library_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
}
