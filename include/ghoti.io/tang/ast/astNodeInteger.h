/**
 * @file
 */

#ifndef GHOTI_IO_GTA_AST_ASTNODEINTEGER_H
#define GHOTI_IO_GTA_AST_ASTNODEINTEGER_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <ghoti.io/tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Integer class.
 */
GTA_API extern GTA_Ast_Node_VTable gta_ast_node_integer_vtable;

/**
 * The GTA_Ast_Node_Integer class.
 */
struct GTA_Ast_Node_Integer {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The value of the integer.
   */
  int64_t value;
};

/**
 * Creates a new GTA_Ast_Node_Integer object.
 *
 * @param integer The integer value.
 * @param location The location of the integer in the source code.
 * @return The new GTA_Ast_Node_Integer object or NULL on failure.
 */
GTA_API GTA_NO_DISCARD GTA_Ast_Node_Integer * gta_ast_node_integer_create(int64_t integer, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Integer object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Integer object to destroy.
 */
GTA_API void gta_ast_node_integer_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Integer object to stdout.
 *
 * @param self The GTA_Ast_Node_Integer object.
 * @param indent The string to print before each line of output.
 */
GTA_API void gta_ast_node_integer_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Integer object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Integer object.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Integer object or NULL on failure.
 */
GTA_API GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_integer_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Integer object.
 *
 * @param self The GTA_Ast_Node_Integer object.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
GTA_API void gta_ast_node_integer_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

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
GTA_API bool gta_ast_node_integer_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

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
GTA_API bool gta_ast_node_integer_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GHOTI_IO_GTA_AST_ASTNODEINTEGER_H