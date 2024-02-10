/**
 * @file
 */

#ifndef GTA_AST_NODE_FOR_H
#define GTA_AST_NODE_FOR_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_For class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_for_vtable;

/**
 * The GTA_Ast_Node_For class.
 */
typedef struct GTA_Ast_Node_For {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The initialization of the for loop.
   */
  GTA_Ast_Node * init;
  /**
   * The condition of the for loop.
   */
  GTA_Ast_Node * condition;
  /**
   * The update of the for loop.
   */
  GTA_Ast_Node * update;
  /**
   * The block of the for loop.
   */
  GTA_Ast_Node * block;
} GTA_Ast_Node_For;

/**
 * Creates a new GTA_Ast_Node_For object.
 *
 * @param init The initialization of the for loop.
 * @param condition The condition of the for loop.
 * @param update The update of the for loop.
 * @param block The block of the for loop.
 * @param location The location of the for loop in the source code.
 * @return The new GTA_Ast_Node_For object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_For * gta_ast_node_for_create(GTA_Ast_Node * init, GTA_Ast_Node * condition, GTA_Ast_Node * update, GTA_Ast_Node * block, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_For object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_For object to destroy.
 */
void gta_ast_node_for_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_For object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_For object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_for_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_For object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_For object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_For object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_for_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_For object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_For object to walk.
 * @param callback The callback function to call for each node in the tree.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_for_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_FOR_H