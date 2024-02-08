/**
 * @file
 */

#ifndef GTA_AST_NODE_BINARY_H
#define GTA_AST_NODE_BINARY_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_Binary class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_binary_vtable;

/**
 * The types of binary operators used by GTA_Ast_Node_Binary.
 */
typedef enum GTA_Binary_Type {
  GTA_BINARY_TYPE_ADD,                ///> +
  GTA_BINARY_TYPE_SUBTRACT,           ///> -
  GTA_BINARY_TYPE_MULTIPLY,           ///> *
  GTA_BINARY_TYPE_DIVIDE,             ///> /
  GTA_BINARY_TYPE_MODULO,             ///> %
  GTA_BINARY_TYPE_LESS_THAN,          ///> <
  GTA_BINARY_TYPE_LESS_THAN_EQUAL,    ///> <=
  GTA_BINARY_TYPE_GREATER_THAN,       ///> >
  GTA_BINARY_TYPE_GREATER_THAN_EQUAL, ///> >=
  GTA_BINARY_TYPE_EQUAL,              ///> ==
  GTA_BINARY_TYPE_NOT_EQUAL,          ///> !=
  GTA_BINARY_TYPE_AND,                ///> &&
  GTA_BINARY_TYPE_OR,                 ///> ||
} GTA_Binary_Type;

/**
 * The GTA_Ast_Node_Binary class.
 */
typedef struct GTA_Ast_Node_Binary {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The left-hand side of the binary operation.
   */
  GTA_Ast_Node * lhs;
  /**
   * The right-hand side of the binary operation.
   */
  GTA_Ast_Node * rhs;
  /**
   * The type of binary operation.
   */
  GTA_Binary_Type operator_type;
} GTA_Ast_Node_Binary;

/**
 * Creates a new GTA_Ast_Node_Binary object.
 *
 * @param lhs The left-hand side of the binary operation.
 * @param rhs The right-hand side of the binary operation.
 * @param operator_type The type of binary operation.
 * @param location The location of the binary operation in the source code.
 * @return The new GTA_Ast_Node_Binary object or NULL on failure.
 */
GTA_Ast_Node_Binary * gta_ast_node_binary_create(GTA_Ast_Node * lhs, GTA_Ast_Node * rhs, GTA_Binary_Type operator_type, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Binary object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Binary object to destroy.
 */
void gta_ast_node_binary_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Binary object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Binary object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_binary_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Binary object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Binary object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Binary object or NULL on failure.
 */
GTA_Ast_Node * gta_ast_node_binary_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Binary object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Binary object to walk.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_binary_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_BINARY_H