/**
 * @file
 */

#ifndef GTA_AST_NODE_UNARY_H
#define GTA_AST_NODE_UNARY_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_Unary class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_unary_vtable;

/**
 * The types of unary operators used by GTA_Ast_Node_Unary.
 */
typedef enum GTA_Unary_Type {
  GTA_UNARY_TYPE_NEGATIVE, ///> -
  GTA_UNARY_TYPE_NOT,      ///> !
} GTA_Unary_Type;

/**
 * The GTA_Ast_Node_Unary class.
 */
typedef struct GTA_Ast_Node_Unary {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The expression to apply the unary operation to.
   */
  GTA_Ast_Node * expression;
  /**
   * The type of unary operation.
   */
  GTA_Unary_Type operator_type;
} GTA_Ast_Node_Unary;

/**
 * Creates a new GTA_Ast_Node_Unary object.
 *
 * @param expression The expression to apply the unary operation to.
 * @param operator_type The type of unary operation.
 * @param location The location of the unary operation in the source code.
 * @return The new GTA_Ast_Node_Unary object or NULL on failure.
 */
GTA_Ast_Node_Unary * gta_ast_node_unary_create(GTA_Ast_Node * expression, GTA_Unary_Type operator_type, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Unary object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Unary object to destroy.
 */
void gta_ast_node_unary_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Unary object to a string.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Unary object.
 * @param indent The indentation level.
 * @return The string or NULL on failure.
 */
void gta_ast_node_unary_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Unary object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Unary object.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Unary object or NULL on failure.
 */
GTA_Ast_Node * gta_ast_node_unary_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Unary object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Unary object.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_unary_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_UNARY_H