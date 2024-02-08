/**
 * @file
 */

#ifndef GTA_AST_NODE_BOOLEAN_H
#define GTA_AST_NODE_BOOLEAN_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_Boolean class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_boolean_vtable;

/**
 * The GTA_Ast_Node_Boolean class.
 */
typedef struct GTA_Ast_Node_Boolean {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The boolean value.
   */
  bool value;
} GTA_Ast_Node_Boolean;

/**
 * Creates a new GTA_Ast_Node_Boolean object.
 *
 * @param boolean The boolean value.
 * @param location The location of the boolean in the source code.
 * @return The new GTA_Ast_Node_Boolean object or NULL on failure.
 */
GTA_Ast_Node_Boolean * gta_ast_node_boolean_create(bool boolean, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Boolean object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Boolean object to destroy.
 */
void gta_ast_node_boolean_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Boolean object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Boolean object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_boolean_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Boolean object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Boolean object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Boolean object or NULL on failure.
 */
GTA_Ast_Node * gta_ast_node_boolean_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Boolean object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Boolean object to walk.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_boolean_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_BOOLEAN_H