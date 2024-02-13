
#include <cutil/memory.h>
#include "tang/binaryCompilerContext.h"

GTA_Binary_Compiler_Context * gta_binary_compiler_context_create(GTA_Program * program) {
  GTA_Binary_Compiler_Context * context = gcu_malloc(sizeof(GTA_Binary_Compiler_Context));
  if (!context) {
    return 0;
  }
  GCU_Vector8 * binary_vector = gcu_vector8_create(1024);
  if (!binary_vector) {
    gcu_free(context);
    return 0;
  }
  *context = (GTA_Binary_Compiler_Context) {
    .program = program,
    .binary_vector = binary_vector,
    .binary = 0,
    .stack_depth = 0,
  };
  return context;
}

void gta_binary_compiler_context_destroy(GTA_Binary_Compiler_Context * context) {
  if (context) {
    gcu_vector8_destroy(context->binary_vector);
    gcu_free(context);
  }
}
