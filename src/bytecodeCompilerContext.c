
#include <cutil/memory.h>
#include "tang/bytecodeCompilerContext.h"
#include "tang/program.h"

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
  context->program = program;
  return true;
}

void gta_bytecode_compiler_context_destroy(GTA_Bytecode_Compiler_Context * context) {
  gta_bytecode_compiler_context_destroy_in_place(context);
  gcu_free(context);
}

void gta_bytecode_compiler_context_destroy_in_place(GTA_Bytecode_Compiler_Context * context) {
  (void)context;
}
