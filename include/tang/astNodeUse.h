/**
 * @file
 */

#ifndef GTA_AST_NODE_USE_H
#define GTA_AST_NODE_USE_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_Use class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_use_vtable;

/**
 * The GTA_Ast_Node_Use class.
 */
typedef struct GTA_Ast_Node_Use {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The identifier of the use statement.
   */
  const char * identifier;
  /**
   * The expression of the use statement.
   */
  GTA_Ast_Node * expression;
} GTA_Ast_Node_Use;

/**
 * Creates a new GTA_Ast_Node_Use object.
 *
 * @param identifier The identifier of the use statement.
 * @param expression The expression of the use statement.
 * @param location The location of the use statement in the source code.
 * @return The new GTA_Ast_Node_Use object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Use * gta_ast_node_use_create(const char * identifier, GTA_Ast_Node * expression, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Use object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Use object to destroy.
 */
void gta_ast_node_use_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Use object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Use object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_use_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Use object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Use object to simplify.
 * @param variable_map The variable map to use when simplifying the use
 *   statement.
 * @return The simplified GTA_Ast_Node_Use object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_use_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Use object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Use object to walk.
 * @param callback The callback to call for each node in the use statement.
 * @param data The user-defined data to pass to the callback.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_use_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_USE_H