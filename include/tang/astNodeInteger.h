/**
 * @file
 */

#ifndef GTA_AST_NODE_INTEGER_H
#define GTA_AST_NODE_INTEGER_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/astNode.h>
#include <tang/bytecodeCompilerContext.h>
#include <tang/binaryCompilerContext.h>

/**
 * The vtable for the GTA_Ast_Node_Integer class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_integer_vtable;

/**
 * The GTA_Ast_Node_Integer class.
 */
typedef struct GTA_Ast_Node_Integer {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The value of the integer.
   */
  int64_t value;
} GTA_Ast_Node_Integer;

/**
 * Creates a new GTA_Ast_Node_Integer object.
 *
 * @param integer The integer value.
 * @param location The location of the integer in the source code.
 * @return The new GTA_Ast_Node_Integer object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Integer * gta_ast_node_integer_create(int64_t integer, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Integer object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Integer object to destroy.
 */
void gta_ast_node_integer_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Integer object to stdout.
 *
 * @param self The GTA_Ast_Node_Integer object.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_integer_print(GTA_Ast_Node * self, const char * indent);

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
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_integer_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Walks a GTA_Ast_Node_Integer object.
 *
 * @param self The GTA_Ast_Node_Integer object.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_integer_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Compile the AST node to binary.
 *
 * The vtable's compile_to_binary function is called to compile the node.  This
 * function serves as a general dispatch function, and should be used in
 * preference to calling the vtable's compile_to_binary function directly.
 *
 * @see gta_ast_node_compile_to_binary
 *
 * @param self The node to compile.
 * @param context Contextual information for the compile process.
 * @return True on success, false on failure.
 */
bool gta_ast_node_integer_compile_to_binary(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context);

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
bool gta_ast_node_integer_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_INTEGER_H