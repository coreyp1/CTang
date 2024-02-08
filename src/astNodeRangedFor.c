
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include "tang/astNodeRangedFor.h"
#include "tang/astNodeAssign.h"
#include "tang/astNodeIdentifier.h"
#include "tang/astNodeBinary.h"
#include "tang/macros.h"

GTA_Ast_Node_VTable gta_ast_node_ranged_for_vtable = {
  .name = "RangedFor",
  .compile_to_bytecode = 0,
  .destroy = gta_ast_node_ranged_for_destroy,
  .print = gta_ast_node_ranged_for_print,
  .simplify = gta_ast_node_ranged_for_simplify,
  .walk = gta_ast_node_ranged_for_walk,
};

GTA_Ast_Node_Ranged_For * gta_ast_node_ranged_for_create(const char * identifier, GTA_Ast_Node * expression, GTA_Ast_Node * block, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Ranged_For * self = gcu_malloc(sizeof(GTA_Ast_Node_Ranged_For));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_ranged_for_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->identifier = identifier;
  self->expression = expression;
  self->block = block;
  return self;
}

void gta_ast_node_ranged_for_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Ranged_For * ranged_for = (GTA_Ast_Node_Ranged_For *) self;
  gcu_free((void *)ranged_for->identifier);
  gta_ast_node_destroy(ranged_for->expression);
  gta_ast_node_destroy(ranged_for->block);
  gcu_free(self);
}

void gta_ast_node_ranged_for_print(GTA_Ast_Node * self, const char * indent) {
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 5);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "    ", 5);
  GTA_Ast_Node_Ranged_For * ranged_for = (GTA_Ast_Node_Ranged_For *) self;
  printf("%s%s: %s\n", indent, self->vtable->name, ranged_for->identifier);
  printf("%s  Expression:\n", indent);
  gta_ast_node_print(ranged_for->expression, new_indent);
  printf("%s  Block:\n", indent);
  gta_ast_node_print(ranged_for->block, new_indent);
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

GTA_Ast_Node * gta_ast_node_ranged_for_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Ranged_For * ranged_for = (GTA_Ast_Node_Ranged_For *) self;

  // The expression is always executed once, so we can simplify it.
  GTA_Ast_Node * simplified_expression = gta_ast_node_simplify(ranged_for->expression, variable_map);
  if (simplified_expression) {
    gta_ast_node_destroy(ranged_for->expression);
    ranged_for->expression = simplified_expression;
  }

  // The block may be executed multiple times, so we cannot allow any nodes to
  // be simplified that are also assignments at any point in the code block.
  // Use a walk function to find all the assignments in the loop.
  // If any part of this fails, we must invalidate all the assignments in the
  // variable map, because we cannot guarantee that assignments did not happen.
  GCU_Hash64 * assignments = gcu_hash64_create(0);
  bool error = false;
  if (!assignments) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    return 0;
  }

  gta_ast_node_walk(ranged_for->block, findAssignments, assignments, &error);
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

  // Create copies of the variable map to pass to the block.
  GTA_Ast_Simplify_Variable_Map * block_variable_map = gcu_hash64_clone(variable_map);
  if (!block_variable_map) {
    gta_ast_simplify_variable_map_invalidate(variable_map);
    gcu_hash64_destroy(assignments);
    gcu_hash64_destroy(block_variable_map);
    return 0;
  }

  GTA_Ast_Node * simplified_block = gta_ast_node_simplify(ranged_for->block, block_variable_map);
  if (simplified_block) {
    gta_ast_node_destroy(ranged_for->block);
    ranged_for->block = simplified_block;
  }

  // Synchronize the variable map with the block variable map.
  gta_ast_simplify_variable_map_synchronize(variable_map, block_variable_map);

  // Destroy the temporary variable maps.
  gcu_hash64_destroy(assignments);
  gcu_hash64_destroy(block_variable_map);

  return 0;
}

void gta_ast_node_ranged_for_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Ranged_For * ranged_for = (GTA_Ast_Node_Ranged_For *) self;
  gta_ast_node_walk(ranged_for->expression, callback, data, return_value);
  gta_ast_node_walk(ranged_for->block, callback, data, return_value);
}
