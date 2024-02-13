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
  (gcu_vector8_append(X, GCU_TYPE8_UI8(A)))

#define GTA_BINARY_WRITE2(X,A,B) \
  (gcu_vector8_append(X, GCU_TYPE8_UI8(A)) \
   && gcu_vector8_append(X, GCU_TYPE8_UI8(B)))

#define GTA_BINARY_WRITE3(X,A,B,C) \
  (gcu_vector8_append(X, GCU_TYPE8_UI8(A)) \
   && gcu_vector8_append(X, GCU_TYPE8_UI8(B)) \
   && gcu_vector8_append(X, GCU_TYPE8_UI8(C)))

#define GTA_BINARY_WRITE4(X,A,B,C,D) \
  (gcu_vector8_append(X, GCU_TYPE8_UI8(A)) \
   && gcu_vector8_append(X, GCU_TYPE8_UI8(B)) \
   && gcu_vector8_append(X, GCU_TYPE8_UI8(C)) \
   && gcu_vector8_append(X, GCU_TYPE8_UI8(D)))

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
