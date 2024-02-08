/**
 * @file
 */

#ifndef GTA_AST_NODE_WHILE_H
#define GTA_AST_NODE_WHILE_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_While class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_while_vtable;

/**
 * The GTA_Ast_Node_While class.
 */
typedef struct GTA_Ast_Node_While {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The condition of the while loop.
   */
  GTA_Ast_Node * condition;
  /**
   * The block of the while loop.
   */
  GTA_Ast_Node * block;
} GTA_Ast_Node_While;

/**
 * Creates a new GTA_Ast_Node_While object.
 *
 * @param condition The condition of the while loop.
 * @param block The block of the while loop.
 * @param location The location of the while loop in the source code.
 * @return The new GTA_Ast_Node_While object or NULL on failure.
 */
GTA_Ast_Node_While * gta_ast_node_while_create(GTA_Ast_Node * condition, GTA_Ast_Node * block, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_While object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_While object to destroy.
 */
void gta_ast_node_while_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_While object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_While object to print.
 * @param indent The indentation level.
 */
void gta_ast_node_while_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_While object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_While object to simplify.
 * @param variable_map The variable map.
 * @return The simplified GTA_Ast_Node_While object or NULL on failure.
 */
GTA_Ast_Node * gta_ast_node_while_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_While object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_While object to walk.
 * @param callback The callback function.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_while_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_WHILE_H