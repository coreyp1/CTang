/**
 * @file
 */

#ifndef GTA_AST_NODE_PRINT_H
#define GTA_AST_NODE_PRINT_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

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
struct GTA_Ast_Node_Print {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The type of print.
   */
  GTA_Ast_Node * expression;
};

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
 * Perform pre-compilation analysis on the AST node.
 *
 * This step includes allocating constants, identifying libraries and variables
 * (global and local), and creating namespace scopes for functions.
 *
 * This function serves as a general dispatch function, and should be used in
 * preference to calling the vtable's analyze function directly.
 *
 * @see gta_ast_node_analyze()
 *
 * @param self The node to analyze.
 * @param program The program that the node is part of.
 * @param scope The current variable scope.
 * @return NULL on success, otherwise return a parse error.
 */
GTA_Ast_Node * gta_ast_node_print_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

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
bool gta_ast_node_print_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

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
bool gta_ast_node_print_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_PRINT_H