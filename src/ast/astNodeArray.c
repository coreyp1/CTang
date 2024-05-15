
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeArray.h>

GTA_Ast_Node_VTable gta_ast_node_array_vtable = {
  .name = "Array",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_array_destroy,
  .print = gta_ast_node_array_print,
  .simplify = gta_ast_node_array_simplify,
  .analyze = 0,
  .walk = gta_ast_node_array_walk,
};


GTA_Ast_Node_Array * gta_ast_node_array_create(GTA_VectorX * elements, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Array * self = gcu_malloc(sizeof(GTA_Ast_Node_Array));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_array_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->elements = elements;
  return self;
}


void gta_ast_node_array_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Array * array = (GTA_Ast_Node_Array *) self;
  GTA_VECTORX_DESTROY(array->elements);
  gcu_free(self);
}


void gta_ast_node_array_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Array * array = (GTA_Ast_Node_Array *) self;
  char * new_indent = gcu_malloc(strlen(indent) + 3);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);
  printf("%s%s\n", indent, self->vtable->name);
  for (size_t i = 0; i < GTA_VECTORX_COUNT(array->elements); i++) {
    gta_ast_node_print((GTA_Ast_Node *) array->elements->data[i].p, new_indent);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_array_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Array * array = (GTA_Ast_Node_Array *) self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(array->elements); ++i) {
    GTA_Ast_Node * element = (GTA_Ast_Node *) array->elements->data[i].p;
    GTA_Ast_Node * simplified = gta_ast_node_simplify(element, variable_map);
    if (simplified) {
      gta_ast_node_destroy(element);
      array->elements->data[i].p = simplified;
    }
  }
  return 0;
}


void gta_ast_node_array_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Array * array = (GTA_Ast_Node_Array *) self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(array->elements); ++i) {
    GTA_Ast_Node * element = (GTA_Ast_Node *) array->elements->data[i].p;
    gta_ast_node_walk(element, callback, data, return_value);
  }
}
