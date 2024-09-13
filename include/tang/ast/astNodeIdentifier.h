/**
 * @file
 */

#ifndef GTA_AST_NODE_IDENTIFIER_H
#define GTA_AST_NODE_IDENTIFIER_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/ast/astNode.h>

/**
 * The vtable for the GTA_Ast_Node_Identifier class.
 */
extern GTA_Ast_Node_VTable gta_ast_node_identifier_vtable;

/**
 * The type of the identifier in the scope.
 */
typedef enum GTA_Ast_Node_Identifier_Type {
  /**
   * The identifier type is unknown.
   */
  GTA_AST_NODE_IDENTIFIER_TYPE_NONE,
  /**
   * The identifier is a local variable.
   */
  GTA_AST_NODE_IDENTIFIER_TYPE_LOCAL,
  /**
   * The identifier is a global variable.
   */
  GTA_AST_NODE_IDENTIFIER_TYPE_GLOBAL,
  /**
   * The identifier is a function.
   */
  GTA_AST_NODE_IDENTIFIER_TYPE_FUNCTION,
  /**
   * The identifier is a library.
   */
  GTA_AST_NODE_IDENTIFIER_TYPE_LIBRARY,
} GTA_Ast_Node_Identifier_Type;

/**
 * The GTA_Ast_Node_Identifier class.
 *
 * There are 3 scope types that an identifier can be:
 * 1. Local variable
 * 2. Function declaration
 * 3. Global/Library variable
 *
 * Resolution rules are as follows:
 * 1. If the identifier is a function declaration, then it is resolved to the
 *   function declaration.
 * 2. If the identifier is declared Global in the current scope, then it is
 *   resolved to the global variable.  (The outermost scope is the global scope
 *   by default.)
 * 3. Otherwise, the identifier is resolved to the local variable.
 */
struct GTA_Ast_Node_Identifier {
  /**
   * The base class.
   */
  GTA_Ast_Node base;
  /**
   * The identifier.
   */
  const char * identifier;
  /**
   * A hash of the identifier.
   */
  GTA_UInteger hash;
  /**
   * A mangled name, if needed.
   *
   * The mangled name is used to uniquely identify the identifier in the
   * program.  It is mostly useful for function identifier.  The mangled
   * name pointer will not be deleted by the destructor.  Rather, when the
   * mangled name is created, it should be added to the GTA_Variable_Scope
   * `allocated_mangled_names` vector.  This vector will be cleaned up when the
   * scope is destroyed.
   */
  const char * mangled_name;
  /**
   * The mangled name hash.
   */
  GTA_UInteger mangled_name_hash;
  /**
   * The type of the identifier.
   */
  GTA_Ast_Node_Identifier_Type type;
  /**
   * The scope in which the node is operating.
   *
   * This will be set during the analysis phase, and will be used to determine
   * how to resolve a variable name.
   */
  GTA_Variable_Scope * scope;
};

/**
 * Creates a new GTA_Ast_Node_Identifier object.
 *
 * The identifier will be adopted by the new object, so the caller should not
 * free the identifier after calling this function.
 *
 * @param identifier The identifier.
 * @param location The location of the identifier in the source code.
 * @return The new GTA_Ast_Node_Identifier object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node_Identifier * gta_ast_node_identifier_create(const char * identifier, GTA_PARSER_LTYPE location);

/**
 * Destroys a GTA_Ast_Node_Identifier object.
 *
 * This function should not be called directly. Use gta_ast_node_destroy()
 * instead.
 *
 * @param self The GTA_Ast_Node_Identifier object to destroy.
 */
void gta_ast_node_identifier_destroy(GTA_Ast_Node * self);

/**
 * Prints a GTA_Ast_Node_Identifier object to stdout.
 *
 * This function should not be called directly. Use gta_ast_node_print()
 * instead.
 *
 * @param self The GTA_Ast_Node_Identifier object to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_identifier_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplifies a GTA_Ast_Node_Identifier object.
 *
 * This function should not be called directly. Use gta_ast_node_simplify()
 * instead.
 *
 * @param self The GTA_Ast_Node_Identifier object to simplify.
 * @param variable_map The variable map to use for simplification.
 * @return The simplified GTA_Ast_Node_Identifier object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_identifier_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

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
GTA_Ast_Node * gta_ast_node_identifier_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

/**
 * Walks a GTA_Ast_Node_Identifier object.
 *
 * This function should not be called directly. Use gta_ast_node_walk()
 * instead.
 *
 * @param self The GTA_Ast_Node_Identifier object to walk.
 * @param callback The callback function to call for each node.
 * @param data The user-defined data to pass to the callback function.
 * @param return_value The return value of the walk, populated by the callback.
 */
void gta_ast_node_identifier_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Compiles a GTA_Ast_Node_Identifier object to binary for x86_64.
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
bool gta_ast_node_identifier_compile_to_binary__x86_64(GTA_Ast_Node * self, GTA_Compiler_Context * context);

/**
 * Compiles a GTA_Ast_Node_Identifier object to bytecode.
 *
 * This function should not be called directly. Use gta_ast_node_compile_to_bytecode()
 * instead.
 *
 * @see gta_ast_node_compile_to_bytecode
 *
 * @param self The GTA_Ast_Node_Identifier object.
 * @param context The compiler state to use for compilation.
 */
bool gta_ast_node_identifier_compile_to_bytecode(GTA_Ast_Node * self, GTA_Compiler_Context * context);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_IDENTIFIER_H