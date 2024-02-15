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
#include "tang/macros.h"
#include "tang/program.h"

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
} GTA_Binary_Compiler_Context;

/**
 * Create a new binary compiler context.
 *
 * @param program The program that the binary compiler is compiling.
 * @return The new binary compiler context or 0 on failure.
 */
GTA_NO_DISCARD GTA_Binary_Compiler_Context * gta_binary_compiler_context_create(GTA_Program * program);

/**
 * Destroy a binary compiler context.
 *
 * @param context The binary compiler context to destroy.
 */
void gta_binary_compiler_context_destroy(GTA_Binary_Compiler_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_BINARY_COMPILER_CONTEXT_H
