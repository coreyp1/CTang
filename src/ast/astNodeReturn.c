
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeReturn.h>

GTA_Ast_Node_VTable gta_ast_node_return_vtable = {
  .name = "Return",
  .compile_to_bytecode = 0,
  .compile_to_binary__x86_64 = 0,
  .compile_to_binary__arm_64 = 0,
  .compile_to_binary__x86_32 = 0,
  .compile_to_binary__arm_32 = 0,
  .destroy = gta_ast_node_return_destroy,
  .print = gta_ast_node_return_print,
  .simplify = gta_ast_node_return_simplify,
  .walk = gta_ast_node_return_walk,
};


GTA_Ast_Node_Return * gta_ast_node_return_create(GTA_Ast_Node * expression, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Return * self = gcu_malloc(sizeof(GTA_Ast_Node_Return));
  if (!self) {
    return 0;
  }
  *self = (GTA_Ast_Node_Return) {
    .base = {
      .vtable = &gta_ast_node_return_vtable,
      .location = location,
      .possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN,
      .is_singleton = false,
    },
    .expression = expression,
  };
  return self;
}


void gta_ast_node_return_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Return * return_node = (GTA_Ast_Node_Return *)self;
  gta_ast_node_destroy(return_node->expression);
  gcu_free(self);
}


void gta_ast_node_return_print(GTA_Ast_Node * self, const char * indent) {
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 3);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);
  GTA_Ast_Node_Return * return_node = (GTA_Ast_Node_Return *)self;
  if (return_node->expression) {
    printf("%s%s:\n", indent, self->vtable->name);
    gta_ast_node_print(return_node->expression, new_indent);
  } else {
    printf("%s%s\n", indent, self->vtable->name);
  }
  gcu_free(new_indent);
}


GTA_Ast_Node * gta_ast_node_return_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Return * return_node = (GTA_Ast_Node_Return *)self;
  GTA_Ast_Node * simplified_expression = gta_ast_node_simplify(return_node->expression, variable_map);
  if (simplified_expression) {
    gta_ast_node_destroy(return_node->expression);
    return_node->expression = simplified_expression;
  }
  return 0;
}


void gta_ast_node_return_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Return * return_node = (GTA_Ast_Node_Return *)self;
  gta_ast_node_walk(return_node->expression, callback, data, return_value);
}
