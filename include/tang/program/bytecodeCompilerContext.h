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
#include <tang/program/bytecode.h>

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
  /**
   * A vector of vectors to store, for each label, the binary offsets which
   * should be patched with the label's position, once it is known.
   */
  GTA_VectorX * labels_from;
  /**
   * A vector to store the position of each label, once it is known.
   */
  GTA_VectorX * labels;
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

/**
 * Get a new label identifier.
 *
 * This will ensure that the label is unique within the program and that
 * adequate space is reserved in the labels vector.
 *
 * Note: 0 is a valid label id, so the return value should be checked against
 * -1.
 *
 * @param context The bytecode compiler context.
 * @return The new label id, or -1 on failure.
 */
GTA_NO_DISCARD GTA_Integer gta_bytecode_compiler_context_get_label(GTA_Bytecode_Compiler_Context * context);

/**
 * Mark an offset as needing to be patched with the label's position, once
 * it is known.
 *
 * The offset should be the position in the bytecode where the label needs
 * to be patched.  It is relative to the start of the bytecode.  This is
 * commonly used for jump instructions.
 *
 * @param context The bytecode compiler context.
 * @param label The label id that the jump will go to.
 * @param from_offset The offset to patch with the label's position.
 * @return True on success, false on failure.
 */
bool gta_bytecode_compiler_context_add_label_jump(GTA_Bytecode_Compiler_Context * context, GTA_Integer label, GTA_Integer from_offset);

/**
 * Record the position of a label in the bytecode.
 *
 * The offset is the "jump target" for jump instructions.  It will be used
 * to calculate the relative offset for jump instructions and will be patched
 * into the bytecode at the end of compilation.
 *
 * @param context The bytecode compiler context.
 * @param label The label id.
 * @param offset The target position of the label in the bytecode.
 * @return True on success, false on failure.
 */
bool gta_bytecode_compiler_context_set_label(GTA_Bytecode_Compiler_Context * context, GTA_Integer label, GTA_Integer offset);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_BYTECODE_COMPILER_CONTEXT_H
