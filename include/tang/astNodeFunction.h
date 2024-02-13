/**
 * @file
 */

#ifndef GTA_AST_NODE_FUNCTION_H
#define GTA_AST_NODE_FUNCTION_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_Function class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_function_vtable;

/**
 * The GTA_Ast_Node_Function class.
 */
typedef struct GTA_Ast_Node_Function {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The identifier of the function.
   */
  const char * identifier;
  /**
   * The parameters of the function.
   *
   * This is a vector of char * identifiers.
   */
  GTA_VectorX * parameters;
  /**
   * The block of the function.
   */
  GTA_Ast_Node * block;
} GTA_Ast_Node_Function;

/**
 * Creates a new GTA_Ast_Node_Function object.
 *
 * @param identifier The identifier of the function.
 * @param parameters The parameters of the function.
 * @param block The block of the function.
 * @param location The location of the function in the source code.
 * @return The new GTA_Ast_Node_Function object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Function * gta_ast_node_function_create(const char * identifier, GTA_VectorX * parameters, GTA_Ast_Node * block, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Function object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Function object to destroy.
 */
void gta_ast_node_function_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Function object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Function object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_function_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Function object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Function object to simplify.
 * @param variable_map The variable map to use when simplifying the function.
 * @return The simplified GTA_Ast_Node_Function object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_function_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Function object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Function object to walk.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_function_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_FUNCTION_H