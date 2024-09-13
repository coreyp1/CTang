/**
 * @file
 */

#ifndef GTA_AST_NODE_FUNCTION_H
#define GTA_AST_NODE_FUNCTION_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Function class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_function_vtable;

/**
 * The GTA_Ast_Node_Function class.
 *
 * Functions may be declared in any scope, but they are only callable from the
 * scope in which they are declared, or from a child scope of that scope.
 *
 * A function declaration will cause all identifiers in that scope or child
 * scopes to be resolved to the function declaration.
 *
 * Function identifiers of the same name, but in different scopes are treated
 * separately and will resolve to the correct function declaration according to
 * the scope resolution rules.
 */
struct GTA_Ast_Node_Function {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The identifier of the function.
   */
  const char * identifier;
  /**
   * A hash of the identifier.
   */
  GTA_UInteger hash;
  /**
   * A mangled name, if needed.
   */
  const char * mangled_name;
  /**
   * The mangled name hash.
   */
  GTA_UInteger mangled_name_hash;
  /**
   * The parameters of the function.
   *
   * This is a vector of AST Node Identifiers.
   */
  GTA_VectorX * parameters;
  /**
   * The block of the function.
   */
  GTA_Ast_Node * block;
  /**
   * The scope associated with this function declaration.
   *
   * This is used to store the local variables of the function.  It is stored
   * on the AST node so that it can be responsibly destroyed when the node is
   * destroyed.  It also protects against the edge case of a function of the
   * same name being declared twice in the same scope.
   */
  GTA_Variable_Scope * scope;
  /**
   * The Computed Value Function holding the function metadata to be invoked at
   * runtime.  Obviously, this cannot be created until compile time.
   */
  GTA_Computed_Value_Function * runtime_function;
};

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
GTA_Ast_Node * gta_ast_node_function_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

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
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_binary_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

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
bool gta_ast_node_function_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

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
bool gta_ast_node_function_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_FUNCTION_H