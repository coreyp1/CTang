
#include <cutil/memory.h>
#include "tang/bytecodeCompilerContext.h"

GTA_Bytecode_Compiler_Context * gta_bytecode_compiler_context_create(GTA_Bytecode_Vector * bytecode) {
  GTA_Bytecode_Compiler_Context * context = gcu_malloc(sizeof(GTA_Bytecode_Compiler_Context));
  if (!context) {
    return 0;
  }

  if (!gta_bytecode_compiler_context_create_in_place(context, bytecode)) {
    gcu_free(context);
    return 0;
  }

  return context;
}

bool gta_bytecode_compiler_context_create_in_place(GTA_Bytecode_Compiler_Context * context, GTA_Bytecode_Vector * bytecode) {
  context->bytecode = bytecode;
  return true;
}

void gta_bytecode_compiler_context_destroy(GTA_Bytecode_Compiler_Context * context) {
  gta_bytecode_compiler_context_destroy_in_place(context);
  gcu_free(context);
}

void gta_bytecode_compiler_context_destroy_in_place(GTA_Bytecode_Compiler_Context * context) {
  (void)context;
}
