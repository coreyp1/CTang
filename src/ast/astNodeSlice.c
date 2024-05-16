
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeSlice.h>

GTA_Ast_Node_VTable gta_ast_node_slice_vtable = {
  .name = "Slice",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_slice_destroy,
  .print = gta_ast_node_slice_print,
  .simplify = gta_ast_node_slice_simplify,
  .analyze = 0,
  .walk = gta_ast_node_slice_walk,
};


GTA_Ast_Node_Slice * gta_ast_node_slice_create(GTA_Ast_Node * lhs, GTA_Ast_Node * start, GTA_Ast_Node * end, GTA_Ast_Node * skip, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Slice * self = gcu_malloc(sizeof(GTA_Ast_Node_Slice));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Slice) {
    .base = {
      .vtable = &gta_ast_node_slice_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .lhs = lhs,
    .start = start,
    .end = end,
    .skip = skip,
  };
  return self;
}


void gta_ast_node_slice_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Slice * slice = (GTA_Ast_Node_Slice *)self;
  gta_ast_node_destroy(slice->lhs);
  gta_ast_node_destroy(slice->start);
  gta_ast_node_destroy(slice->end);
  gta_ast_node_destroy(slice->skip);
  gcu_free(self);
}


void gta_ast_node_slice_print(GTA_Ast_Node * self, const char * indent) {
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 5);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  GTA_Ast_Node_Slice * slice = (GTA_Ast_Node_Slice *)self;
  printf("%s%s:\n", indent, self->vtable->name);
  printf("%s  LHS:\n", indent);
  gta_ast_node_print(slice->lhs, new_indent);
  printf("%s  Start:\n", indent);
  gta_ast_node_print(slice->start, new_indent);
  printf("%s  End:\n", indent);
  gta_ast_node_print(slice->end, new_indent);
  printf("%s  Skip:\n", indent);
  gta_ast_node_print(slice->skip, new_indent);
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_slice_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Slice * slice = (GTA_Ast_Node_Slice *)self;
  GTA_Ast_Node * simplified_lhs = gta_ast_node_simplify(slice->lhs, variable_map);
  if (simplified_lhs) {
    gta_ast_node_destroy(slice->lhs);
    slice->lhs = simplified_lhs;
  }
  GTA_Ast_Node * simplified_start = gta_ast_node_simplify(slice->start, variable_map);
  if (simplified_start) {
    gta_ast_node_destroy(slice->start);
    slice->start = simplified_start;
  }
  GTA_Ast_Node * simplified_end = gta_ast_node_simplify(slice->end, variable_map);
  if (simplified_end) {
    gta_ast_node_destroy(slice->end);
    slice->end = simplified_end;
  }
  GTA_Ast_Node * simplified_skip = gta_ast_node_simplify(slice->skip, variable_map);
  if (simplified_skip) {
    gta_ast_node_destroy(slice->skip);
    slice->skip = simplified_skip;
  }
  return 0;
}


void gta_ast_node_slice_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Slice * slice = (GTA_Ast_Node_Slice *)self;
  gta_ast_node_walk(slice->lhs, callback, data, return_value);
  gta_ast_node_walk(slice->start, callback, data, return_value);
  gta_ast_node_walk(slice->end, callback, data, return_value);
  gta_ast_node_walk(slice->skip, callback, data, return_value);
}
