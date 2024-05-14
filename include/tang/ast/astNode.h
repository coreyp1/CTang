/**
 * @file
 */

#ifndef GTA_AST_NODE_H
#define GTA_AST_NODE_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef struct GTA_Ast_Node GTA_Ast_Node;
typedef struct GTA_Bytecode_Compiler_Context GTA_Bytecode_Compiler_Context;
typedef struct GTA_Binary_Compiler_Context GTA_Binary_Compiler_Context;

#include <stdbool.h>
#include <cutil/float.h>
#include <cutil/hash.h>
#include <cutil/vector.h>
#include "tangParser.h"
#include <tang/location.h>
#include <tang/program/binaryCompilerContext.h>
#include <tang/program/bytecodeCompilerContext.h>
#include <tang/unicodeString.h>

/**
 * Callback function signature required by the gta_ast_node_walk() function.
 *
 * @param self The current node being visited.
 * @param data A pointer to the user-defined data that was passed to
 *   gta_ast_node_walk().
 * @param return_value A pointer to the user-defined return value that was
 *   passed to gta_ast_node_walk(). This value can be modified by the callback.
 */
typedef void (*GTA_Ast_Node_Walk_Callback)(GTA_Ast_Node * self, void * data, void * return_value);

/**
 * A map of variable names to the ast nodes that represent their values.
 *
 * The gta_ast_node_simplify() function uses this map to track the most recent
 * expression that is assigned to a value, so that computations can be done at
 * compile time.
 *
 * The map key is a 64-bit hash of the variable name, and the value is the ast
 * node that represents the value of the variable.  The hash should *not* delete
 * the ast nodes when it is destroyed.
 */
typedef GCU_Hash64 GTA_Ast_Simplify_Variable_Map;

/**
 * An enum of the different types of AST nodes that are possible as a result of
 * execution.  This can be used to produce more optimized bytecode/assembly.
 */
typedef enum GTA_Ast_Possible_Type {
  GTA_AST_POSSIBLE_TYPE_UNKNOWN = 0,
  GTA_AST_POSSIBLE_TYPE_ERROR = 1,
  GTA_AST_POSSIBLE_TYPE_NULL = 2,
  GTA_AST_POSSIBLE_TYPE_BOOLEAN = 4,
  GTA_AST_POSSIBLE_TYPE_INTEGER = 8,
  GTA_AST_POSSIBLE_TYPE_FLOAT = 16,
  GTA_AST_POSSIBLE_TYPE_STRING = 32,
} GTA_Ast_Possible_Type;

/**
 * Helper defintion to represent all possible types.
 */
#define GTA_AST_POSSIBLE_TYPE_ALL (GTA_AST_POSSIBLE_TYPE_ERROR | GTA_AST_POSSIBLE_TYPE_NULL | GTA_AST_POSSIBLE_TYPE_BOOLEAN | GTA_AST_POSSIBLE_TYPE_INTEGER | GTA_AST_POSSIBLE_TYPE_FLOAT | GTA_AST_POSSIBLE_TYPE_STRING)

/**
 * The vtable for the GTA_Ast_Node class.
 */
typedef struct GTA_Ast_Node_VTable {
  /**
   * The name of the class.  It should be unique for each class, and should be
   * suitable for printing in error messages.
   */
  const char * name;
  /**
   * Compiles the node to binary.
   *
   * @param self The node to compile.
   * @param context Contextual information for the compile process.
   * @return True on success, false on failure.
   */
  bool (*compile_to_binary)(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context);
  /**
   * Compiles the node to bytecode.
   *
   * @param self The node to compile.
   * @param compiler The compiler to compile the node to.
   * @return True on success, false on failure.
   */
  bool (*compile_to_bytecode)(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context);
  /**
   * Destroys the node and all of its children.
   *
   * @param self The node to destroy.
   */
  void (GTA_CALL *destroy)(GTA_Ast_Node * self);
  /**
   * Prints the node and all of its children to stdout.
   *
   * @param self The node to print.
   * @param indent The string to print before each line of output.
   */
  void (*print)(GTA_Ast_Node * self, const char * indent);
  /**
   * Simplifies the node and all of its children.
   *
   * @param self The node to simplify.
   * @param variable_map A map of variable names to the ast nodes that represent
   *   their values.  The gta_ast_node_simplify() function uses this map to track
   *   the most recent expression that is assigned to a value, so that
   *   computations can be done at compile time.
   * @return The simplified node.  The original node should be *not* be destroyed.
   *   The calling function should destroy the original node if it is no longer
   *   needed.
   */
  GTA_Ast_Node * (*simplify)(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);
  /**
   * Analyzes the node and all of its children prior to compilation.
   *
   * This step includes allocating constants, identifying libraries and variables
   * (global and local), and creating namespace scopes for functions.
   *
   * @param self The node to analyze.
   * @param program The program that the node is part of.
   * @param scope The current variable scope.
   * @return NULL on success, otherwise return a parse error.
   */
  GTA_Ast_Node * (*analyze)(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

  /**
   * Generalized function to walk the AST.  The callback function is called for
   * each node in the tree.
   *
   * @param self The node to walk.
   * @param callback A callback function that is called for each node in the
   *   tree.
   * @param data A pointer to user-defined data that is passed to the callback
   *   function.
   * @param return_value A pointer to user-defined return value that is passed to
   *   the callback function.  This value can be modified by the callback.
   */
  void (*walk)(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);
} GTA_Ast_Node_VTable;

/**
 * The vtable for the GTA_Ast_Node class.
 *
 * A basic AST node of this type is used to represent the NULL value.
 */
extern GTA_Ast_Node_VTable gta_ast_node_null_vtable;

/**
 * The base class for all AST nodes.
 */
typedef struct GTA_Ast_Node {
  /**
   * The vtable for the GTA_Ast_Node class.
   */
  GTA_Ast_Node_VTable *vtable;
  /**
   * The location of the node in the source code.
   */
  GTA_PARSER_LTYPE location;
  /**
   * The possible type of the node.
   */
  GTA_Ast_Possible_Type possible_type;
  /**
   * Whether or not the AST node is a singleton.
   */
  bool is_singleton;
} GTA_Ast_Node;

/**
 * Create a new AST node.
 *
 * @param location The location of the node in the source code.
 * @return The new AST node or NULL on failure.
 */
GTA_NO_DISCARD GTA_Ast_Node * GTA_CALL gta_ast_node_create(GTA_PARSER_LTYPE location);

/**
 * Destroy the AST node and all of its children.
 *
 * The vtable's destroy function is called to destroy the node.  This function
 * serves as a general dispatch function, and should be used in preference to
 * calling the vtable's destroy function directly.
 *
 * @param self The node to destroy.
 */
void GTA_CALL gta_ast_node_destroy(GTA_Ast_Node * self);

/**
 * Compile the AST node to binary.
 *
 * The vtable's compile_to_binary function is called to compile the node.  This
 * function serves as a general dispatch function, and should be used in
 * preference to calling the vtable's compile_to_binary function directly.
 *
 * @param self The node to compile.
 * @param context Contextual information for the compile process.
 * @return True on success, false on failure.
 */
bool gta_ast_node_compile_to_binary(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context);

/**
 * Compile the AST node to bytecode.
 *
 * The vtable's compile_to_bytecode function is called to compile the node.  This
 * function serves as a general dispatch function, and should be used in
 * preference to calling the vtable's compile_to_bytecode function directly.
 *
 * @param self The node to compile.
 * @param context Contextual information for the compile process.
 * @return True on success, false on failure.
 */
bool gta_ast_node_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context);

/**
 * Print the AST node and all of its children to stdout.
 *
 * The vtable's print function is called to print the node.  This function serves
 * as a general dispatch function, and should be used in preference to calling
 * the vtable's print function directly.
 *
 * @param self The node to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_print(GTA_Ast_Node * self, const char * indent);

/**
 * Simplify the AST node and all of its children.
 *
 * The vtable's simplify function is called to simplify the node.  This function
 * serves as a general dispatch function, and should be used in preference to
 * calling the vtable's simplify function directly.
 *
 * Simplifying is a tricky proposition.  The goal is to reduce the AST by
 * pre-computing as much as possible.  This is done by evaluating constant
 * expressions, and by replacing variables with their values.  The variable_map
 * is used to track the most recent expression that is assigned to a value, so
 * that computations can be done at compile time.
 *
 * Care must be taken when simplifying nodes that are not always evaluated,
 * such as the "if" or "ternary" nodes, or for repeated code blocks in which
 * the variable may change between evaluations.  In these cases, the node
 * should *remove* the variable from the variable_map if it is involved in
 * an assignment.
 *
 * @param self The node to simplify.
 * @param variable_map A map of variable names to the ast nodes that represent
 *   their values.
 * @return The simplified node.  The original node should be *not* be destroyed.
 *   The calling function should destroy the original node if it is no longer
 *   needed.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Generalized function to walk the AST.  The callback function is called for
 * each node in the tree.
 *
 * The vtable's walk function is called to walk the node.  This function serves
 * as a general dispatch function, and should be used in preference to calling
 * the vtable's walk function directly.
 *
 * @param self The node to walk.
 * @param callback A callback function that is called for each node in the tree.
 * @param data A pointer to user-defined data that is passed to the callback
 *   function.
 * @param return_value A pointer to user-defined return value that is passed to
 *   the callback function.  This value can be modified by the callback.
 */
void gta_ast_node_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Compile a NULL node to binary.
 *
 * @param self The node to compile.
 * @param context Contextual information for the compile process.
 * @return True on success, false on failure.
 */
bool gta_ast_node_null_compile_to_binary(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context);

/**
 * Compile a NULL node to bytecode.
 *
 * @param self The node to compile.
 * @param context Contextual information for the compile process.
 * @return True on success, false on failure.
 */
bool gta_ast_node_null_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context);

/**
 * The destroy function for the GTA_Ast_Node class when the node is a null.
 *
 * @param self The node to destroy.
 */
void GTA_CALL gta_ast_node_null_destroy(GTA_Ast_Node * self);

/**
 * The print function for the GTA_Ast_Node class when the node is a null.
 *
 * @param self The node to print.
 * @param indent The string to print before each line of output.
 */
void gta_ast_node_null_print(GTA_Ast_Node * self, const char * indent);

/**
 * The simplify function for the GTA_Ast_Node class when the node is a null.
 *
 * @param self The node to simplify.
 * @param variable_map A map of variable names to the ast nodes that represent
 *   their values.
 * @return The simplified node.  The original node should be *not* be destroyed.
 *   The calling function should destroy the original node if it is no longer
 *   needed.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_null_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * The walk function for the GTA_Ast_Node class when the node is a null.
 *
 * @param self The node to walk.
 * @param callback A callback function that is called for each node in the tree.
 * @param data A pointer to user-defined data that is passed to the callback
 *   function.
 * @param return_value A pointer to user-defined return value that is passed to
 *   the callback function.  This value can be modified by the callback.
 */
void gta_ast_node_null_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value);

/**
 * Invaliates all variables in the variable map.
 *
 * This function is used to invalidate the variable map when the simplification
 * of a node fails.  It is not safe to use the variable map to simplify any
 * subsequent values, because some of those variables may be assigned in the
 * node that failed to simplify.
 *
 * @param variable_map The variable map to invalidate.
 */
void gta_ast_simplify_variable_map_invalidate(GTA_Ast_Simplify_Variable_Map * variable_map);

/**
 * Remove any entry in target that is not in source.
 *
 * This function is used to synchronize the variable maps after simplifying a
 * node.  The variable map of the simplified node is used as the source, and the
 * variable map of the original node is used as the target.
 *
 * @param target The variable map whose elements will be removed if not in
 *   source.
 * @param source The variable map whose elements will be tested.
 */
void gta_ast_simplify_variable_map_synchronize(GTA_Ast_Simplify_Variable_Map * target, GTA_Ast_Simplify_Variable_Map * source);

/**
 * Perform pre-compilation analysis on the AST node.
 *
 * This step includes allocating constants, identifying libraries and variables
 * (global and local), and creating namespace scopes for functions.
 *
 * This function serves as a general dispatch function, and should be used in
 * preference to calling the vtable's analyze function directly.
 *
 * @param self The node to analyze.
 * @param program The program that the node is part of.
 * @return NULL on success, otherwise return a parse error.
 */
GTA_NO_DISCARD GTA_Ast_Node * gta_ast_node_analyze(GTA_Ast_Node * self, GTA_Program * program, GTA_Variable_Scope * scope);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_AST_NODE_H