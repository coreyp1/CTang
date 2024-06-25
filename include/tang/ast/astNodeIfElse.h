/**
 * @file
 */

#ifndef GTA_AST_NODE_IF_ELSE_H
#define GTA_AST_NODE_IF_ELSE_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_If_Else class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_if_else_vtable;

/**
 * The GTA_Ast_Node_If_Else class.
 */
typedef struct GTA_Ast_Node_If_Else {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The condition of the if-else statement.
   */
  GTA_Ast_Node * condition;
  /**
   * The block of the if-else statement.
   */
  GTA_Ast_Node * ifBlock;
  /**
   * The else block of the if-else statement.
   */
  GTA_Ast_Node * elseBlock;
} GTA_Ast_Node_If_Else;

/**
 * Creates a new GTA_Ast_Node_If_Else object.
 *
 * @param condition The condition of the if-else statement.
 * @param ifBlock The block of the if-else statement.
 * @param elseBlock The else block of the if-else statement.
 * @param location The location of the if-else statement in the source code.
 * @return The new GTA_Ast_Node_If_Else object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_If_Else * gta_ast_node_if_else_create(GTA_Ast_Node * condition, GTA_Ast_Node * ifBlock, GTA_Ast_Node * elseBlock, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_If_Else object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_If_Else object to destroy.
 */
void gta_ast_node_if_else_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_If_Else object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_If_Else object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_if_else_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_If_Else object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_If_Else object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_If_Else object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_if_else_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_If_Else object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_If_Else object to walk.
 * @param callback The callback to call for each node in the tree.
 * @param data The user-defined data to pass to the callback.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_if_else_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Compile the AST node to binary for x86_64.
 *
 * The vtable's compile_to_binary function is called to compile the node.  This
 * function serves as a general dispatch function, and should be used in
 * preference to calling the vtable's compile_to_binary function directly.
 *
 * @see gta_ast_node_compile_to_binary__x86_64
 *
 * @param self The node to compile.
 * @param context Contextual information for the compile process.
 * @return True on success, false on failure.
 */
bool gta_ast_node_if_else_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context);

/**
 * Compiles a GTA_Ast_Node_Integer object to bytecode.
 *
 * This function should not be called directly. Use gta_ast_node_compile_to_bytecode()
 * instead.
 *
 * @see gta_ast_node_compile_to_bytecode
 *
 * @param self The GTA_Ast_Node_Integer object.
 * @param context The compiler state to use for compilation.
 */
bool gta_ast_node_if_else_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_IF_ELSE_H