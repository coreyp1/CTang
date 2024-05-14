
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodePrint.h>

GTA_Ast_Node_VTable gta_ast_node_print_vtable = {
  .name = "Print",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_print_destroy,
  .print = gta_ast_node_print_print,
  .simplify = gta_ast_node_print_simplify,
  .analyze = 0,
  .walk = gta_ast_node_print_walk,
};

GTA_Ast_Node_Print * gta_ast_node_print_create(GTA_Ast_Node * expression, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Print * self = gcu_malloc(sizeof(GTA_Ast_Node_Print));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_print_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->expression = expression;
  return self;
}

void gta_ast_node_print_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Print * print = (GTA_Ast_Node_Print *)self;
  gta_ast_node_destroy(print->expression);
  gcu_free(self);
}

void gta_ast_node_print_print(GTA_Ast_Node * self, const char * indent) {
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 3);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);
  GTA_Ast_Node_Print * print = (GTA_Ast_Node_Print *)self;
  printf("%s%s():\n", indent, self->vtable->name);
  gta_ast_node_print(print->expression, new_indent);
  gcu_free(new_indent);
}

GTA_Ast_Node * gta_ast_node_print_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Print * print = (GTA_Ast_Node_Print *)self;
  GTA_Ast_Node * simplified_expression = gta_ast_node_simplify(print->expression, variable_map);
  if (simplified_expression) {
    gta_ast_node_destroy(print->expression);
    print->expression = simplified_expression;
  }
  return 0;
}

void gta_ast_node_print_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Print * print = (GTA_Ast_Node_Print *)self;
  callback(print->expression, data, return_value);
}
