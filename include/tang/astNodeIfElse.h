/**
 * @file
 */

#ifndef GTA_AST_NODE_IF_ELSE_H
#define GTA_AST_NODE_IF_ELSE_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_If_Else class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_if_else_vtable;

/**
 * The GTA_Ast_Node_If_Else class.
 */
typedef struct GTA_Ast_Node_If_Else {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The condition of the if-else statement.
   */
  GTA_Ast_Node * condition;
  /**
   * The block of the if-else statement.
   */
  GTA_Ast_Node * ifBlock;
  /**
   * The else block of the if-else statement.
   */
  GTA_Ast_Node * elseBlock;
} GTA_Ast_Node_If_Else;

/**
 * Creates a new GTA_Ast_Node_If_Else object.
 *
 * @param condition The condition of the if-else statement.
 * @param ifBlock The block of the if-else statement.
 * @param elseBlock The else block of the if-else statement.
 * @param location The location of the if-else statement in the source code.
 * @return The new GTA_Ast_Node_If_Else object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_If_Else * gta_ast_node_if_else_create(GTA_Ast_Node * condition, GTA_Ast_Node * ifBlock, GTA_Ast_Node * elseBlock, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_If_Else object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_If_Else object to destroy.
 */
void gta_ast_node_if_else_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_If_Else object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_If_Else object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_if_else_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_If_Else object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_If_Else object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_If_Else object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_if_else_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_If_Else object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_If_Else object to walk.
 * @param callback The callback to call for each node in the tree.
 * @param data The user-defined data to pass to the callback.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_if_else_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_IF_ELSE_H