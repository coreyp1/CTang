
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/hash.h>
#include <cutil/string.h>
#include "tang/astNodeUse.h"
#include "tang/macros.h"

GTA_Ast_Node_VTable gta_ast_node_use_vtable = {
  .name = "Use",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_use_destroy,
  .print = gta_ast_node_use_print,
  .simplify = gta_ast_node_use_simplify,
  .walk = gta_ast_node_use_walk,
};

GTA_Ast_Node_Use * gta_ast_node_use_create(const char * identifier, GTA_Ast_Node * expression, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Use * self = gcu_malloc(sizeof(GTA_Ast_Node_Use));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_use_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->identifier = identifier;
  self->expression = expression;
  return self;
}

void gta_ast_node_use_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;
  gcu_free((void *)use->identifier);
  gta_ast_node_destroy(use->expression);
  gcu_free(self);
}

void gta_ast_node_use_print(GTA_Ast_Node * self, const char * indent) {
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 3);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;
  printf("%s%s(%s):\n", indent, self->vtable->name, use->identifier);
  if (use->expression) {
    gta_ast_node_print(use->expression, new_indent);
  }
  gcu_free(new_indent);
}

GTA_Ast_Node * gta_ast_node_use_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;
  GTA_Ast_Node * simplified_expression = gta_ast_node_simplify(use->expression, variable_map);
  if (simplified_expression) {
    gta_ast_node_destroy(use->expression);
    use->expression = simplified_expression;
  }
  // If the use expression has an alias, then add it to the variable map.
  if (use->identifier) {
    gcu_hash64_set(variable_map, gcu_string_hash_64(use->identifier, strlen(use->identifier)), GCU_TYPE64_P(use->expression));
  }
  return 0;
}

void gta_ast_node_use_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Use * use = (GTA_Ast_Node_Use *)self;
  gta_ast_node_walk(use->expression, callback, data, return_value);
}
