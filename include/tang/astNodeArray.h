/**
 * @file
 */

#ifndef GTA_AST_NODE_ARRAY_H
#define GTA_AST_NODE_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include "tang/astNode.h"

/**
 * The vtable for the GTA_Ast_Node_Array class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_array_vtable;

/**
 * The GTA_Ast_Node_Array class.
 */
typedef struct GTA_Ast_Node_Array {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The elements of the array.
   */
  GCU_Vector64 * elements;
} GTA_Ast_Node_Array;

/**
 * Creates a new GTA_Ast_Node_Array object.
 *
 * @param elements The elements of the array.
 * @param location The location of the array in the source code.
 * @return The new GTA_Ast_Node_Array object or NULL on failure.
 */
GTA_Ast_Node_Array * gta_ast_node_array_create(GCU_Vector64 * elements, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Array object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Array object to destroy.
 */
void gta_ast_node_array_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Array object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Array object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_array_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Array object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Array object to simplify.
 * @param variable_map A map of variable names to their values.
 * @return The simplified GTA_Ast_Node_Array object or NULL on failure.
 */
GTA_Ast_Node * gta_ast_node_array_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Array object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Array object to walk.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_array_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_ARRAY_H