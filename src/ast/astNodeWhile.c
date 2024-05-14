
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include <tang/ast/astNodeAssign.h>
#include <tang/ast/astNodeBinary.h>
#include <tang/ast/astNodeIdentifier.h>
#include <tang/ast/astNodeWhile.h>

GTA_Ast_Node_VTable gta_ast_node_while_vtable = {
  .name = "While",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_while_destroy,
  .print = gta_ast_node_while_print,
  .simplify = gta_ast_node_while_simplify,
  .analyze = 0,
  .walk = gta_ast_node_while_walk,
};

GTA_Ast_Node_While * gta_ast_node_while_create(GTA_Ast_Node * condition, GTA_Ast_Node * block, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_While * self = gcu_malloc(sizeof(GTA_Ast_Node_While));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_while_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->condition = condition;
  self->block = block;
  return self;
}

void gta_ast_node_while_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_While * while_node = (GTA_Ast_Node_While *) self;
  gta_ast_node_destroy(while_node->condition);
  gta_ast_node_destroy(while_node->block);
  gcu_free(self);
}

void gta_ast_node_while_print(GTA_Ast_Node * self, const char * indent) {
  GTA_Ast_Node_While * while_node = (GTA_Ast_Node_While *) self;
  char * new_indent = gcu_malloc(strlen(indent) + 5);
  if (!new_indent) {
    return;
  }
  size_t indent_len = strlen(indent);
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  printf("%s%s\n", indent, self->vtable->name);
  printf("%s  Condition:\n", indent);
  gta_ast_node_print(while_node->condition, new_indent);
  printf("%s  Block:\n", indent);
  gta_ast_node_print(while_node->block, new_indent);
  gcu_free(new_indent);
}

/**
 * This function is used to find all the assignments in the while loop.
 */
static void findAssignments(GTA_Ast_Node * self, void * data, void * error) {
  GCU_Hash64 * assignments = (GCU_Hash64 *)data;
  if (GTA_AST_IS_ASSIGN(self)) {
    GTA_Ast_Node_Assign * assignment = (GTA_Ast_Node_Assign *)self;
    if (GTA_AST_IS_IDENTIFIER(assignment->lhs)) {
      GTA_Ast_Node_Identifier * identifier = (GTA_Ast_Node_Identifier *)assignment->lhs;
      if (!gcu_hash64_set(assignments, gcu_string_hash_64(identifier->identifier, strlen(identifier->identifier)), GCU_TYPE64_B(true))) {
        *(bool *)error = true;
      }
    }
  }
}

GTA_Ast_Node * gta_ast_node_while_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_While * while_node = (GTA_Ast_Node_While *) self;

  // Both the condition and the block may be executed multiple times, so we
  // cannot allow any nodes to be simplified that are also assignments at any
  // point in the condition or code block.
  // Use a walk function to find all the assignments in the while loop.
  // If any part of this fails, we must invalidate all the assignments in the
  // variable map, because we cannot guarantee that assignments did not happen.
  GCU_Hash64 * assignments = gcu_hash64_create(0);
  bool error = false;
  if (!assignments) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    return 0;
  }

  gta_ast_node_walk(while_node->condition, findAssignments, assignments, &error);
  gta_ast_node_walk(while_node->block, findAssignments, assignments, &error);
  if (error) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    gcu_hash64_destroy(assignments);
    return 0;
  }

  // Remove the assignments from the variable map.
  GCU_Hash64_Iterator iterator = gcu_hash64_iterator_get(assignments);
  while (iterator.exists) {
    gcu_hash64_remove(variable_map, iterator.hash);
    iterator = gcu_hash64_iterator_next(iterator);
  }

  // Create copies of the variable map to pass to the condition and block.
  GTA_Ast_Simplify_Variable_Map * condition_variable_map = gcu_hash64_clone(variable_map);
  GTA_Ast_Simplify_Variable_Map * block_variable_map = gcu_hash64_clone(variable_map);
  if (!condition_variable_map || !block_variable_map) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    gcu_hash64_destroy(assignments);
    gcu_hash64_destroy(condition_variable_map);
    gcu_hash64_destroy(block_variable_map);
    return 0;
  }

  GTA_Ast_Node * simplified_condition = gta_ast_node_simplify(while_node->condition, condition_variable_map);
  if (simplified_condition) {
    gta_ast_node_destroy(while_node->condition);
    while_node->condition = simplified_condition;
  }
  GTA_Ast_Node * simplified_block = gta_ast_node_simplify(while_node->block, block_variable_map);
  if (simplified_block) {
    gta_ast_node_destroy(while_node->block);
    while_node->block = simplified_block;
  }

  // Synchronize the variable maps.
  gta_ast_simplify_variable_map_synchronize(variable_map, condition_variable_map);
  gta_ast_simplify_variable_map_synchronize(variable_map, block_variable_map);

  // Destroy the temporary variable maps.
  gcu_hash64_destroy(assignments);
  gcu_hash64_destroy(condition_variable_map);
  gcu_hash64_destroy(block_variable_map);

  return 0;
}

void gta_ast_node_while_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_While * while_node = (GTA_Ast_Node_While *) self;
  gta_ast_node_walk(while_node->condition, callback, data, return_value);
  gta_ast_node_walk(while_node->block, callback, data, return_value);
}
