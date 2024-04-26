/**
 * @file
 *
 * The bytecode compiler context is used to compile the bytecode, which is the fallback
 * in the event that the JIT compiler is not available for the target platform.
 */

#ifndef G_TANG_BYTECODE_COMPILER_CONTEXT_H
#define G_TANG_BYTECODE_COMPILER_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <cutil/vector.h>
#include "tang/bytecode.h"
#include "tang/macros.h"

typedef struct GTA_Program GTA_Program;

#define GTA_BYTECODE_APPEND(X,Y) \
  GTA_VECTORX_APPEND(X, GTA_TYPEX_MAKE_UI(Y))

/**
 * The bytecode compiler context.
 *
 * The bytecode compiler context holds the state of the bytecode compiler as it
 * compiles a program.
 */
typedef struct GTA_Bytecode_Compiler_Context {
  /**
   * The program being compiled.
   */
  GTA_Program * program;
  /**
   * Bytecode offsets.
   *
   * Some bytecode instructions have different sizes, so we need to keep track
   * of the offsets of the bytecode instructions so that we can navigate
   * the bytecode later.
   */
  GTA_VectorX * bytecode_offsets;
  /**
   * The scope stack.
   *
   * The scope stack is used to keep track of the variable names that have been
   * declared in the current scope.
   *
   * The scope stack is a stack of hash tables.  Each hash table maps the name
   * of a variable to the index of the variable in the stack, with 0 being the
   * first variable in the stack frame.
   */
  GTA_VectorX * scope_stack;
  /**
   * The globals variables.
   *
   * Global variables can only be "declared" in the outermost scope, and are
   * used for library aliases.
   *
   * Each entry maps the name of a global variable to the absolute index of the
   * variable in the stack.
   */
  GTA_HashX * globals;
} GTA_Bytecode_Compiler_Context;

/**
 * Creates a new bytecode compiler context.
 *
 * Use with gta_bytecode_compiler_context_destroy().
 *
 * @see gta_bytecode_compiler_context_destroy()
 *
 * @param program The program to compile.
 * @return The new bytecode compiler context or NULL on failure.
 */
GTA_NO_DISCARD GTA_Bytecode_Compiler_Context * gta_bytecode_compiler_context_create(GTA_Program * program);

/**
 * Creates a new bytecode compiler context in place using the supplied memory
 * location.
 *
 * Use with gta_bytecode_compiler_context_destroy_in_place().
 *
 * @see gta_bytecode_compiler_context_destroy_in_place()
 *
 * @param context The context to create.
 * @param program The program to compile.
 * @return true on success, false on failure.
 */
bool gta_bytecode_compiler_context_create_in_place(GTA_Bytecode_Compiler_Context * context, GTA_Program * program);

/**
 * Destroys a bytecode compiler context.
 *
 * Use with gta_bytecode_compiler_context_create().
 *
 * @see gta_bytecode_compiler_context_create()
 *
 * @param self The bytecode compiler context to destroy.
 */
void gta_bytecode_compiler_context_destroy(GTA_Bytecode_Compiler_Context * context);

/**
 * Destroys a bytecode compiler context in place.
 *
 * Use with gta_bytecode_compiler_context_create_in_place().
 *
 * @see gta_bytecode_compiler_context_create_in_place()
 *
 * @param self The bytecode compiler context to destroy.
 */
void gta_bytecode_compiler_context_destroy_in_place(GTA_Bytecode_Compiler_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_BYTECODE_COMPILER_CONTEXT_H
