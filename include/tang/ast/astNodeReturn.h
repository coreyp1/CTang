/**
 * @file
 */

#ifndef GTA_AST_NODE_RETURN_H
#define GTA_AST_NODE_RETURN_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Return class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_return_vtable;

/**
 * The GTA_Ast_Node_Return class.
 */
typedef struct GTA_Ast_Node_Return {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The expression to return.
   */
  GTA_Ast_Node * expression;
} GTA_Ast_Node_Return;

/**
 * Creates a new GTA_Ast_Node_Return object.
 *
 * @param expression The expression to return.
 * @param location The location of the return statement in the source code.
 * @return The new GTA_Ast_Node_Return object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Return * gta_ast_node_return_create(GTA_Ast_Node * expression, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Return object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Return object to destroy.
 */
void gta_ast_node_return_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Return object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Return object to print.
 * @param indent The number of spaces to indent the output.
 */
void gta_ast_node_return_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Return object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Return object to simplify.
 * @param variable_map The variable map to use when simplifying the return
 *   expression.
 * @return The simplified GTA_Ast_Node_Return object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_return_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Return object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Return object to walk.
 * @param callback The callback function to call for each node in the tree.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_return_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_RETURN_H