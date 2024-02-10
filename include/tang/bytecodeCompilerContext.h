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
