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

typedef struct GTA_Program GTA_Program;

typedef struct GTA_Bytecode_Compiler_Context {
  GTA_Program * program;
} GTA_Bytecode_Compiler_Context;

GTA_Bytecode_Compiler_Context * gta_bytecode_compiler_context_create(GTA_Program * program);
bool gta_bytecode_compiler_context_create_in_place(GTA_Bytecode_Compiler_Context * context, GTA_Program * program);

void gta_bytecode_compiler_context_destroy(GTA_Bytecode_Compiler_Context * context);
void gta_bytecode_compiler_context_destroy_in_place(GTA_Bytecode_Compiler_Context * context);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_BYTECODE_COMPILER_CONTEXT_H
