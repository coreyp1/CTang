
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/ast/astNodeIfElse.h>

GTA_Ast_Node_VTable gta_ast_node_if_else_vtable = {
  .name = "IfElse",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_if_else_destroy,
  .print = gta_ast_node_if_else_print,
  .simplify = gta_ast_node_if_else_simplify,
  .analyze = 0,
  .walk = gta_ast_node_if_else_walk,
};

GTA_Ast_Node_If_Else * gta_ast_node_if_else_create(GTA_Ast_Node * condition, GTA_Ast_Node * ifBlock, GTA_Ast_Node * elseBlock, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_If_Else * self = gcu_malloc(sizeof(GTA_Ast_Node_If_Else));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_if_else_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->condition = condition;
  self->ifBlock = ifBlock;
  self->elseBlock = elseBlock;
  return self;
}

void gta_ast_node_if_else_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_If_Else * if_else = (GTA_Ast_Node_If_Else *) self;
  gta_ast_node_destroy(if_else->condition);
  gta_ast_node_destroy(if_else->ifBlock);
  if (if_else->elseBlock) {
    gta_ast_node_destroy(if_else->elseBlock);
  }
  gcu_free(self);
}

void gta_ast_node_if_else_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_If_Else * if_else = (GTA_Ast_Node_If_Else *) self;
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  printf("%s%s\n", indent, self->vtable->name);
  printf("%s  Condition:\n", indent);
  gta_ast_node_print(if_else->condition, new_indent);
  printf("%s  If Block:\n", indent);
  gta_ast_node_print(if_else->ifBlock, new_indent);
  if (if_else->elseBlock) {
    printf("%s  Else Block:\n", indent);
    gta_ast_node_print(if_else->elseBlock, new_indent);
  }
  gcu_free(new_indent);
}

GTA_Ast_Node * gta_ast_node_if_else_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_If_Else * if_else = (GTA_Ast_Node_If_Else *) self;
  GTA_Ast_Node * simplified_condition = gta_ast_node_simplify(if_else->condition, variable_map);
  if (simplified_condition) {
    gta_ast_node_destroy(if_else->condition);
    if_else->condition = simplified_condition;
  }

  // Create copies of the variable map for the if and else blocks.
  GTA_Ast_Simplify_Variable_Map * if_variable_map = gcu_hash64_clone(variable_map);
  GTA_Ast_Simplify_Variable_Map * else_variable_map = gcu_hash64_clone(variable_map);

  // If the creation of either of the new maps failed, then we have a problem.
  // We cannot check whether or not there are assignments in the true and false
  // branches, so we cannot simplify the ternary operator.  Moreover, it is not
  // safe for the calling functions to further simplify any subsequent values
  // using the existing variable map, because some of those variables may be
  // assigned in the ternary expressions here (that we cannot check).  The only
  // safe option is to invalidate all existing values in the variable map
  // before exiting.
  if (!if_variable_map || !else_variable_map) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    if (if_variable_map) {
      gcu_hash64_destroy(if_variable_map);
    }
    if (else_variable_map) {
      gcu_hash64_destroy(else_variable_map);
    }
    return 0;
  }

  GTA_Ast_Node * simplified_ifBlock = gta_ast_node_simplify(if_else->ifBlock, if_variable_map);
  if (simplified_ifBlock) {
    gta_ast_node_destroy(if_else->ifBlock);
    if_else->ifBlock = simplified_ifBlock;
  }
  if (if_else->elseBlock) {
    GTA_Ast_Node * simplified_elseBlock = gta_ast_node_simplify(if_else->elseBlock, else_variable_map);
    if (simplified_elseBlock) {
      gta_ast_node_destroy(if_else->elseBlock);
      if_else->elseBlock = simplified_elseBlock;
    }
  }

  // Synchronize the variable map with the if and else blocks.
  gta_ast_simplify_variable_map_synchronize(variable_map, if_variable_map);
  gta_ast_simplify_variable_map_synchronize(variable_map, else_variable_map);

  // Destroy the temporary variable maps.
  gcu_hash64_destroy(if_variable_map);
  gcu_hash64_destroy(else_variable_map);

  return 0;
}

void gta_ast_node_if_else_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_If_Else * if_else = (GTA_Ast_Node_If_Else *) self;
  gta_ast_node_walk(if_else->condition, callback, data, return_value);
  gta_ast_node_walk(if_else->ifBlock, callback, data, return_value);
  if (if_else->elseBlock) {
    gta_ast_node_walk(if_else->elseBlock, callback, data, return_value);
  }
}
