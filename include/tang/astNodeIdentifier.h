/**
 * @file
 */

#ifndef GTA_AST_NODE_IDENTIFIER_H
#define GTA_AST_NODE_IDENTIFIER_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_Identifier class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_identifier_vtable;

/**
 * The GTA_Ast_Node_Identifier class.
 */
typedef struct GTA_Ast_Node_Identifier {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The identifier.
   */
  const char * identifier;
} GTA_Ast_Node_Identifier;

/**
 * Creates a new GTA_Ast_Node_Identifier object.
 *
 * @param identifier The identifier.
 * @param location The location of the identifier in the source code.
 * @return The new GTA_Ast_Node_Identifier object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Identifier * gta_ast_node_identifier_create(const char * identifier, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Identifier object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Identifier object to destroy.
 */
void gta_ast_node_identifier_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Identifier object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Identifier object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_identifier_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Identifier object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Identifier object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Identifier object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_identifier_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Identifier object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Identifier object to walk.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_identifier_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_IDENTIFIER_H