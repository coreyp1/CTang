/**
 * @file
 */

#ifndef GTA_AST_NODE_GLOBAL_H
#define GTA_AST_NODE_GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_Global class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_global_vtable;

/**
 * The GTA_Ast_Node_Global class.
 */
typedef struct GTA_Ast_Node_Global {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The identifier.
   */
  GTA_Ast_Node * identifier;
  /**
   * The assignment (if present).
   */
  GTA_Ast_Node * assignment;
} GTA_Ast_Node_Global;

/**
 * Creates a new GTA_Ast_Node_Global object.
 *
 * @param identifier The identifier.
 * @param assignment The assignment (if present).
 * @param location The location of the global in the source code.
 * @return The new GTA_Ast_Node_Global object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Global * gta_ast_node_global_create(GTA_Ast_Node * identifier, GTA_Ast_Node * assignment, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Global object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Global object to destroy.
 */
void gta_ast_node_global_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Global object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Global object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_global_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Global object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Global object to simplify.
 * @param variable_map The variable map.
 * @return The simplified GTA_Ast_Node_Global object.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_global_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Global object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Global object to walk.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_global_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_GLOBAL_H
