/**
 * @file
 *
 * Header file for the BinaryCompilerContext class.
 */

#ifndef TANG_COMPILER_CONTEXT_H
#define TANG_COMPILER_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <cutil/vector.h>
#include <tang/program/bytecode.h>
#include <tang/program/program.h>

#define GTA_BYTECODE_APPEND(X,Y) \
  GTA_VECTORX_APPEND(X, GTA_TYPEX_MAKE_UI(Y))

/**
 * The context for the compiler.
 */
typedef struct GTA_Compiler_Context {
  /**
   * The program that the compiler is compiling.
   */
  GTA_Program * program;
  /**
   * The program that the compiler is compiling.
   *
   * We need to know the final size of the binary before we can allocate
   * excutable memory for it, so this will serve as a resizable buffer for
   * the binary as it is being compiled.
   */
  GCU_Vector8 * binary_vector;
  /**
   * The final binary that is produced by the compiler.
  */
  void * binary;
  /**
   * Bytecode offsets.
   *
   * Some bytecode instructions have different sizes, so we need to keep track
   * of the offsets of the bytecode instructions so that we can navigate
   * the bytecode later.
   */
  GTA_VectorX * bytecode_offsets;
  /**
   * Tracking the current stack depth so that the stack can be properly byte
   * aligned.
   */
  size_t stack_depth;
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
} GTA_Compiler_Context;

/**
 * Create a new compiler context.
 *
 * @param program The program that the compiler is compiling.
 * @return The new compiler context or 0 on failure.
 */
GTA_NO_DISCARD GTA_Compiler_Context * gta_compiler_context_create(GTA_Program * program);

/**
 * Create a new compiler context in place using the supplied memory
 * location.
 *
 * @context The context to create.
 * @program The program to compile.
 * @return True on success, false on failure.
 */
bool gta_compiler_context_create_in_place(GTA_Compiler_Context * context, GTA_Program * program);

/**
 * Destroy a compiler context.
 *
 * @param context The compiler context to destroy.
 */
void gta_compiler_context_destroy(GTA_Compiler_Context * context);

/**
 * Destroy a compiler context in place.
 *
 * @param context The compiler context to destroy.
 */
void gta_compiler_context_destroy_in_place(GTA_Compiler_Context * context);

/**
 * Get a new label identifier.
 *
 * This will ensure that the label is unique within the program and that
 * adequate space is reserved in the labels vector.
 *
 * Note: 0 is a valid label id, so the return value should be checked against
 * -1.
 *
 * @param context The compiler context.
 * @return The new label id, or -1 on failure.
 */
GTA_NO_DISCARD GTA_Integer gta_compiler_context_get_label(GTA_Compiler_Context * context);

/**
 * Mark a byte offset as needing to be patched with the label's position, once
 * it is known.
 *
 * The byte offset should be the position in the binary where the label needs
 * to be patched.  It is relative to the start of the binary.  This is commonly
 * used for jump instructions.
 *
 * @param context The compiler context.
 * @param label The label id that the jump will go to.
 * @param byte_from_offset The byte offset to patch with the label's position.
 * @return True on success, false on failure.
 */
bool gta_compiler_context_add_label_jump(GTA_Compiler_Context * context, GTA_Integer label, GTA_Integer byte_from_offset);

/**
 * Record the position of a label in the binary.
 *
 * The byte offset is the "jump target" for jump instructions.  It will be used
 * to calculate the relative offset for jump instructions and will be patched
 * into the binary at the end of compilation.
 *
 * @param context The compiler context.
 * @param label The label id.
 * @param byte_offset The target position of the label in the binary.
 * @return True on success, false on failure.
 */
bool gta_compiler_context_set_label(GTA_Compiler_Context * context, GTA_Integer label, GTA_Integer byte_offset);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_COMPILER_CONTEXT_H
