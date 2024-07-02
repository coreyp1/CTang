/**
 * @file
 */

#ifndef GTA_AST_NODE_BLOCK_H
#define GTA_AST_NODE_BLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Block class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_block_vtable;

/**
 * The GTA_Ast_Node_Block class.
 */
typedef struct GTA_Ast_Node_Block {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The statements in the block.
   */
  GTA_VectorX * statements;
} GTA_Ast_Node_Block;

/**
 * Creates a new GTA_Ast_Node_Block object.
 *
 * @param statements The statements in the block.
 * @param location The location of the block in the source code.
 * @return The new GTA_Ast_Node_Block object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Block * gta_ast_node_block_create(GTA_VectorX * statements, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Block object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Block object to destroy.
 */
void gta_ast_node_block_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Block object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Block object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_block_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Block object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Block object to simplify.
 * @param variable_map The variable map to use when simplifying the block.
 * @return The simplified GTA_Ast_Node_Block object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_block_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

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
GTA_Ast_Node * gta_ast_node_block_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

/**
 * Walks a GTA_Ast_Node_Block object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Block object to walk.
 * @param callback The callback function to call for each node in the block.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_block_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Compile the block to bytecode.
 *
 * This function should not be called directly. Use
 * gta_ast_node_compile_to_bytecode() instead.
 *
 * @see gta_ast_node_compile_to_bytecode()
 *
 * @param self The GTA_Ast_Node_Block object to compile.
 * @param context The compilation context to use when compiling the block.
 * @return True if the block was successfully compiled to bytecode, false
 *   otherwise.
 */
bool gta_ast_node_block_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

/**
 * Compile the block to binary for x86_64.
 *
 * This function should not be called directly. Use
 * gta_ast_node_compile_to_binary() instead.
 *
 * @see gta_ast_node_compile_to_binary__x86_64
 *
 * @param self The GTA_Ast_Node_Block object to compile.
 * @param context The compilation context to use when compiling the block.
 * @return True if the block was successfully compiled to binary, false
 *   otherwise.
 */
bool gta_ast_node_block_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif

#endif // GTA_AST_NODE_BLOCK_H
