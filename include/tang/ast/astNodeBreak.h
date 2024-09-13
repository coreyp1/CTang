/**
 * @file
 */

#ifndef GTA_AST_NODE_BREAK_H
#define GTA_AST_NODE_BREAK_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Break class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_break_vtable;

/**
 * The GTA_Ast_Node_Break class.
 */
struct GTA_Ast_Node_Break {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
};

/**
 * Creates a new GTA_Ast_Node_Break object.
 *
 * @param location The location of the break in the source code.
 * @return The new GTA_Ast_Node_Break object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Break * gta_ast_node_break_create(GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Break object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Break object to destroy.
 */
void gta_ast_node_break_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Break object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Break object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_break_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Break object.
 *
 * @param self The GTA_Ast_Node_Break object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Break object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_break_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Break object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Break object to walk.
 * @param callback The callback function to call for each node in the tree.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_break_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Compile the AST node to binary for x86_64.
 *
 * This function should not be called directly. Use gta_ast_node_compile_to_binary()
 * instead.
 *
 * @see gta_ast_node_compile_to_binary__x86_64
 *
 * @param self The node to compile.
 * @param context Contextual information for the compile process.
 * @return True on success, false on failure.
 */
bool gta_ast_node_break_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

/**
 * Compiles the AST node to bytecode.
 *
 * This function should not be called directly. Use gta_ast_node_compile_to_bytecode()
 * instead.
 *
 * @see gta_ast_node_compile_to_bytecode
 *
 * @param self The node to compile.
 * @param context The compiler state to use for compilation.
 */
bool gta_ast_node_break_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_BREAK_H