/**
 * @file
 */

#ifndef GTA_AST_NODE_PRINT_H
#define GTA_AST_NODE_PRINT_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_Print class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_print_vtable;

/**
 * The GTA_Ast_Node_Print class.
 */
enum GTA_Print_Type {
  GTA_PRINT_TYPE_DEFAULT, ///> No type specified.
  GTA_PRINT_TYPE_PERCENT, ///> Use percent encoding.
  GTA_PRINT_TYPE_HTML,    ///> Use HTML encoding.
  GTA_PRINT_TYPE_JSON,    ///> Use JSON encoding.
};

/**
 * The GTA_Ast_Node_Print class.
 */
typedef struct GTA_Ast_Node_Print {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The type of print.
   */
  GTA_Ast_Node * expression;
} GTA_Ast_Node_Print;

/**
 * Creates a new GTA_Ast_Node_Print object.
 *
 * @param expression The expression to print.
 * @param location The location of the print in the source code.
 * @return The new GTA_Ast_Node_Print object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Print * gta_ast_node_print_create(GTA_Ast_Node * expression, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Print object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Print object to destroy.
 */
void gta_ast_node_print_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Print object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Print object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_print_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Print object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Print object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Print object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_print_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Print object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Print object to walk.
 * @param callback The callback to call for each node.
 * @param data The user-defined data to pass to the callback.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_print_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_PRINT_H