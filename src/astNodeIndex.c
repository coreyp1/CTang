
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include "tang/astNodeIndex.h"
#include "tang/macros.h"

GTA_Ast_Node_VTable gta_ast_node_index_vtable = {
  .name = "Index",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_index_destroy,
  .print = gta_ast_node_index_print,
  .simplify = gta_ast_node_index_simplify,
  .analyze = 0,
  .walk = gta_ast_node_index_walk,
};

GTA_Ast_Node_Index * gta_ast_node_index_create(GTA_Ast_Node * lhs, GTA_Ast_Node * rhs, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Index * self = gcu_malloc(sizeof(GTA_Ast_Node_Index));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_index_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->lhs = lhs;
  self->rhs = rhs;
  return self;
}

void gta_ast_node_index_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Index * index = (GTA_Ast_Node_Index *) self;
  gta_ast_node_destroy(index->lhs);
  gta_ast_node_destroy(index->rhs);
  gcu_free(self);
}

void gta_ast_node_index_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_Index * index = (GTA_Ast_Node_Index *) self;
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  printf("%s%s\n", indent, self->vtable->name);
  printf("%s  LHS:\n", indent);
  gta_ast_node_print(index->lhs, new_indent);
  printf("%s  RHS:\n", indent);
  gta_ast_node_print(index->rhs, new_indent);
  gcu_free(new_indent);
}

GTA_Ast_Node * gta_ast_node_index_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Index * index = (GTA_Ast_Node_Index *) self;
  GTA_Ast_Node * simplified_lhs = gta_ast_node_simplify(index->lhs, variable_map);
  if (simplified_lhs) {
    gta_ast_node_destroy(index->lhs);
    index->lhs = simplified_lhs;
  }
  GTA_Ast_Node * simplified_rhs = gta_ast_node_simplify(index->rhs, variable_map);
  if (simplified_rhs) {
    gta_ast_node_destroy(index->rhs);
    index->rhs = simplified_rhs;
  }
  return 0;
}

void gta_ast_node_index_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Index * index = (GTA_Ast_Node_Index *) self;
  gta_ast_node_walk(index->lhs, callback, data, return_value);
  gta_ast_node_walk(index->rhs, callback, data, return_value);
}
