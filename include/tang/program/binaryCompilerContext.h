/**
 * @file
 *
 * Header file for the BinaryCompilerContext class.
 */

#ifndef TANG_BINARY_COMPILER_CONTEXT_H
#define TANG_BINARY_COMPILER_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <cutil/vector.h>
#include <tang/macros.h>
#include <tang/program/program.h>

#define GTA_BINARY_WRITE1(X,A) \
  X->data[X->count++] = GCU_TYPE8_UI8(A);

#define GTA_BINARY_WRITE2(X,A,B) \
  GTA_BINARY_WRITE1(X,A) \
  GTA_BINARY_WRITE1(X,B)

#define GTA_BINARY_WRITE3(X,A,B,C) \
  GTA_BINARY_WRITE1(X, A) \
  GTA_BINARY_WRITE2(X, B, C)

#define GTA_BINARY_WRITE4(X,A,B,C,D) \
  GTA_BINARY_WRITE2(X, A, B) \
  GTA_BINARY_WRITE2(X, C, D)

#define GTA_BINARY_WRITE5(X,A,B,C,D,E) \
  GTA_BINARY_WRITE3(X, A, B, C) \
  GTA_BINARY_WRITE2(X, D, E)

#define GTA_BINARY_WRITE8(X,A,B,C,D,E,F,G,H) \
  GTA_BINARY_WRITE4(X, A, B, C, D) \
  GTA_BINARY_WRITE4(X, E, F, G, H)

#define GTA_BINARY_WRITE9(X,A,B,C,D,E,F,G,H,I) \
  GTA_BINARY_WRITE5(X, A, B, C, D, E) \
  GTA_BINARY_WRITE4(X, F, G, H, I)

#define GTA_BINARY_WRITE16(X,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) \
  GTA_BINARY_WRITE8(X, A, B, C, D, E, F, G, H) \
  GTA_BINARY_WRITE8(X, I, J, K, L, M, N, O, P)

/**
 * Helper union for converting between function pointers and integers.
 */
typedef union GTA_JIT_Function_Converter {
  void (*f)(void);
  GTA_UInteger i;
} GTA_JIT_Function_Converter;

/**
 * Convert a function pointer to an integer.
 */
#define GTA_JIT_FUNCTION_CONVERTER(F) \
  ((GTA_JIT_Function_Converter){.f = (void (*)(void))(F)}.i)

/**
 * The context for the binary compiler.
 */
typedef struct GTA_Binary_Compiler_Context{
  /**
   * The program that the binary compiler is compiling.
   *
   * We need to know the final size of the binary before we can allocate
   * excutable memory for it, so this will serve as a resizable buffer for
   * the binary as it is being compiled.
   */
  GCU_Vector8 * binary_vector;
  /**
   * The final binary that is produced by the binary compiler.
  */
  void * binary;
  /**
   * The program that the binary compiler is compiling.
   */
  GTA_Program * program;
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
} GTA_Binary_Compiler_Context;

/**
 * Create a new binary compiler context.
 *
 * @param program The program that the binary compiler is compiling.
 * @return The new binary compiler context or 0 on failure.
 */
GTA_NO_DISCARD GTA_Binary_Compiler_Context * gta_binary_compiler_context_create(GTA_Program * program);

/**
 * Create a new binary compiler context in place using the supplied memory
 * location.
 *
 * @context The context to create.
 * @program The program to compile.
 * @return True on success, false on failure.
 */
bool gta_binary_compiler_context_create_in_place(GTA_Binary_Compiler_Context * context, GTA_Program * program);

/**
 * Destroy a binary compiler context.
 *
 * @param context The binary compiler context to destroy.
 */
void gta_binary_compiler_context_destroy(GTA_Binary_Compiler_Context * context);

/**
 * Destroy a binary compiler context in place.
 *
 * @param context The binary compiler context to destroy.
 */
void gta_binary_compiler_context_destroy_in_place(GTA_Binary_Compiler_Context * context);

/**
 * Get a new label identifier.
 *
 * This will ensure that the label is unique within the program and that
 * adequate space is reserved in the labels vector.
 *
 * Note: 0 is a valid label id, so the return value should be checked against
 * -1.
 *
 * @param context The binary compiler context.
 * @return The new label id, or -1 on failure.
 */
GTA_NO_DISCARD GTA_Integer gta_binary_compiler_context_get_label(GTA_Binary_Compiler_Context * context);

/**
 * Mark a byte offset as needing to be patched with the label's position, once
 * it is known.
 *
 * The byte offset should be the position in the binary where the label needs
 * to be patched.  It is relative to the start of the binary.  This is commonly
 * used for jump instructions.
 *
 * @param context The binary compiler context.
 * @param label The label id that the jump will go to.
 * @param byte_from_offset The byte offset to patch with the label's position.
 * @return True on success, false on failure.
 */
bool gta_binary_compiler_context_add_label_jump(GTA_Binary_Compiler_Context * context, GTA_Integer label, GTA_Integer byte_from_offset);

/**
 * Record the position of a label in the binary.
 *
 * The byte offset is the "jump target" for jump instructions.  It will be used
 * to calculate the relative offset for jump instructions and will be patched
 * into the binary at the end of compilation.
 *
 * @param context The binary compiler context.
 * @param label The label id.
 * @param byte_offset The target position of the label in the binary.
 * @return True on success, false on failure.
 */
bool gta_binary_compiler_context_set_label(GTA_Binary_Compiler_Context * context, GTA_Integer label, GTA_Integer byte_offset);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_BINARY_COMPILER_CONTEXT_H
