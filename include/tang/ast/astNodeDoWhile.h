/**
 * @file
 */

#ifndef GTA_AST_NODE_DO_WHILE_H
#define GTA_AST_NODE_DO_WHILE_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Do_While class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_do_while_vtable;

/**
 * The GTA_Ast_Node_Do_While class.
 */
typedef struct GTA_Ast_Node_Do_While {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The condition of the do-while loop.
   */
  GTA_Ast_Node * condition;
  /**
   * The block of the do-while loop.
   */
  GTA_Ast_Node * block;
} GTA_Ast_Node_Do_While;

/**
 * Creates a new GTA_Ast_Node_Do_While object.
 *
 * @param condition The condition of the do-while loop.
 * @param block The block of the do-while loop.
 * @param location The location of the do-while loop in the source code.
 * @return The new GTA_Ast_Node_Do_While object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Do_While * gta_ast_node_do_while_create(GTA_Ast_Node * condition, GTA_Ast_Node * block, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Do_While object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Do_While object to destroy.
 */
void gta_ast_node_do_while_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Do_While object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Do_While object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_do_while_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Do_While object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Do_While object to simplify.
 * @param variable_map The map of variables to their values.
 * @return The simplified GTA_Ast_Node_Do_While object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_do_while_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Do_While object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Do_While object to walk.
 * @param callback The callback to call for each node in the tree.
 * @param data The user-defined data to pass to the callback.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_do_while_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Perform pre-compilation analysis on the AST node.
 *
 * This step includes allocating constants, identifying libraries and variables
 * (global and local), and creating namespace scopes for functions.
 *
 * This function should not be called directly. Use gta_ast_node_analyze()
 * instead.
 *
 * @param self The node to analyze.
 * @param program The program that the node is part of.
 * @return NULL on success, otherwise return a parse error.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_do_while_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_DO_WHILE_H