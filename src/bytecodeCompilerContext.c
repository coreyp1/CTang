
#include <cutil/memory.h>
#include "tang/bytecodeCompilerContext.h"
#include "tang/program.h"
#include "tang/program/variable.h"

GTA_Bytecode_Compiler_Context * gta_bytecode_compiler_context_create(GTA_Program * program) {
  GTA_Bytecode_Compiler_Context * context = gcu_malloc(sizeof(GTA_Bytecode_Compiler_Context));
  if (!context) {
    return 0;
  }

  if (!gta_bytecode_compiler_context_create_in_place(context, program)) {
    gcu_free(context);
    return 0;
  }

  return context;
}

bool gta_bytecode_compiler_context_create_in_place(GTA_Bytecode_Compiler_Context * context, GTA_Program * program) {
  GTA_VectorX * bytecode_offsets = GTA_VECTORX_CREATE(32);
  if (!bytecode_offsets) {
    return false;
  }
  GTA_VectorX * scope_stack = GTA_VECTORX_CREATE(32);
  if (!scope_stack) {
    GTA_VECTORX_DESTROY(bytecode_offsets);
    return false;
  }
  GTA_HashX * scope = GTA_HASHX_CREATE(32);
  if (!scope) {
    GTA_VECTORX_DESTROY(scope_stack);
    GTA_VECTORX_DESTROY(bytecode_offsets);
    return false;
  }
  GTA_VECTORX_APPEND(scope_stack, GTA_TYPEX_MAKE_P(scope));
  GTA_HashX * globals = GTA_HASHX_CREATE(32);
  if (!globals) {
    GTA_HASHX_DESTROY(scope);
    GTA_VECTORX_DESTROY(scope_stack);
    GTA_VECTORX_DESTROY(bytecode_offsets);
    return false;
  }

  *context = (GTA_Bytecode_Compiler_Context) {
    .program = program,
    .bytecode_offsets = bytecode_offsets,
    .scope_stack = scope_stack,
    .globals = globals,
  };
  return true;
}

void gta_bytecode_compiler_context_destroy(GTA_Bytecode_Compiler_Context * context) {
  gta_bytecode_compiler_context_destroy_in_place(context);
  gcu_free(context);
}

void gta_bytecode_compiler_context_destroy_in_place(GTA_Bytecode_Compiler_Context * context) {
  GTA_VECTORX_DESTROY(context->bytecode_offsets);
  for (size_t i = 0; i < context->scope_stack->count; ++i) {
    GTA_HASHX_DESTROY(GTA_TYPEX_P(context->scope_stack->data[i]));
  }
  GTA_VECTORX_DESTROY(context->scope_stack);
  GTA_HASHX_DESTROY(context->globals);
}
