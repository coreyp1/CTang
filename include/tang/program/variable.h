/**
 * @file
 */

#ifndef TANG_PROGRAM_VARIABLE_H
#define TANG_PROGRAM_VARIABLE_H

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <tang/macros.h>

/**
 * This structure is used to store the identifiers in the program as well as
 * the scope in which they are declared.
 *
 * This is a recursive structure, but the depth is limited to only 1 level.
 * The outermost scope contains all library variables and local variables which
 * may be referenced by any scope using the "global" statement.  These
 * variables will be stored at the base of the stack of the program execution.
 *
 * Functions are considered to be accessible only from the scope in which they
 * are declared, or from a child scope.
 */
struct GTA_Variable_Scope {
  /**
   * The name of the scope.
   *
   * This name will be prepended to any child scope names, separated by a
   * "/" character.  For example, if the parent scope is named "foo" and the
   * child scope is named "bar", the child scope's full name will be
   * "foo/bar".
   */
  char * name;
  /**
   * The hash of the name.
   *
   * This is used for quick comparisons.
   */
  GTA_Integer name_hash;
  /**
   * The parent scope.
   *
   * This is NULL for the global scope.
   */
  GTA_Variable_Scope * parent_scope;
  /**
   * The AST node that created this scope.
   */
  GTA_Ast_Node * ast_node;
  /**
   * The library variables in this scope.
   *
   * Library variables are automatically available to all scopes.  They may
   * be declared in the outermost scope using the "use <expression> as
   * <identifier>" statement or "use <identifier>" statement.  In either
   * case, the identifier is added to the library scope as the key, and the
   * "use..." AST node is added as the value.
   *
   * An identifier may only be used once.  It is an error to attempt to use
   * an identifier that has already been declared in the local or global
   * scope.
   */
  GTA_HashX * library_declarations;
  /**
   * This records the first AST node in a scope for which a given identifier
   * was linked to a library, global, local, or function variable.
   *
   * The key is the identifier hash (non-mangled), and the value is the AST
   * node that first identified what the identifier should reference.
   *
   * This is used so that, as the scope is traversed, all identifiers can be
   * quickly identified as library, global, local, or function variables
   * without having to search through all of the hash tables.
   */
  GTA_HashX * identified_variables;
  /**
   * This records the position of the local variable within the stack.
   *
   * The key is the identifier hash, and the value is the position in the
   * stack (not accounting for any offsets from the base pointer).
   *
   * Function arguments are automatically considered to be local variables.
   */
  GTA_HashX * variable_positions;
  /**
   * The functions in this scope.
   *
   * The key is the mangled identifier hash, and the value is the
   * GTA_Variable_Scope object that represents the function.
   *
   * A function's mangled name is created by concatenating the parent scope's
   * name with the function's identifier, separated by a "/" character.
   *
   * Only the outermost scope will contain the function declarations,
   * regardless of where the function is declared in the program.
   *
   * This must be empty for scopes that are not the outermost scope.
   */
  GTA_HashX * function_scopes;
  /**
   * A list of name hashes that should be freed when the scope is destroyed.
   */
  GTA_VectorX * allocated_mangled_names;
};

/**
 * Create a new variable scope.
 *
 * @param name The name of the scope.  This will be adopted by the scope.
 * @param ast_node The AST node that created the scope.
 * @param parent_scope The parent scope.
 * @return The new scope, or NULL if an error occurred.
 */
GTA_NO_DISCARD GTA_Variable_Scope * gta_variable_scope_create(char * name, GTA_Ast_Node * ast_node, GTA_Variable_Scope * parent_scope);

/**
 * Destroy a variable scope.
 *
 * @param scope The scope to destroy.
 */
void gta_variable_scope_destroy(GTA_Variable_Scope * scope);

/**
 * Print a variable scope.
 *
 * @param scope The variable scope to be printed.
 * @param indent The string to print before each line of output.
 */
void gta_variable_scope_print(GTA_Variable_Scope * scope, const char * indent);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //TANG_PROGRAM_VARIABLE_H
