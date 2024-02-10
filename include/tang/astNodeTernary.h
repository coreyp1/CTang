/**
 * @file
 */

#ifndef GTA_AST_NODE_TERNARY_H
#define GTA_AST_NODE_TERNARY_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_Ternary class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_ternary_vtable;

/**
 * The GTA_Ast_Node_Ternary class.
 */
typedef struct GTA_Ast_Node_Ternary {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The condition of the ternary operation.
   */
  GTA_Ast_Node * condition;
  /**
   * The value if the condition is true.
   */
  GTA_Ast_Node * ifTrue;
  /**
   * The value if the condition is false.
   */
  GTA_Ast_Node * ifFalse;
} GTA_Ast_Node_Ternary;

/**
 * Creates a new GTA_Ast_Node_Ternary object.
 *
 * @param condition The condition of the ternary operation.
 * @param ifTrue The value if the condition is true.
 * @param ifFalse The value if the condition is false.
 * @param location The location of the ternary operation in the source code.
 * @return The new GTA_Ast_Node_Ternary object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Ternary * gta_ast_node_ternary_create(GTA_Ast_Node * condition, GTA_Ast_Node * ifTrue, GTA_Ast_Node * ifFalse, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Ternary object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Ternary object to destroy.
 */
void gta_ast_node_ternary_destroy(GTA_Ast_Node * self);

/**
 * Print a GTA_Ast_Node_Ternary object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Ternary object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_ternary_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Ternary object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Ternary object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Ternary object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_ternary_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Ternary object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Ternary object to walk.
 * @param callback The callback to call for each node in the tree.
 * @param data The user-defined data to pass to the callback.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_ternary_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_TERNARY_H