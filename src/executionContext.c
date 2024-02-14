
#include <cutil/memory.h>
#include <tang/executionContext.h>
#include <tang/macros.h>
#include <tang/computedValue.h>

GTA_Execution_Context * gta_execution_context_create(GTA_Program * program) {
  GTA_Execution_Context * context = gcu_malloc(sizeof(GTA_Execution_Context));
  if (!context) {
    return 0;
  }

  if (!gta_execution_context_create_in_place(context, program)) {
    gcu_free(context);
    return 0;
  }

  return context;
}

bool gta_execution_context_create_in_place(GTA_Execution_Context * context, GTA_Program * program) {
  *context = (GTA_Execution_Context) {
    .program = program,
    .output = 0,
    .result = 0,
    .stack = 0,
  };
  context->stack = GTA_VECTORX_CREATE(32);
  if (!context->stack) {
    return false;
  }
  context->output = gta_unicode_string_create("", 0, GTA_UNICODE_STRING_TYPE_TRUSTED);
  if (!context->output) {
    GTA_VECTORX_DESTROY(context->stack);
    return false;
  }
  return true;
}

void gta_execution_context_destroy(GTA_Execution_Context * self) {
  gta_bytecode_execution_context_destroy_in_place(self);
  gcu_free(self);
}

void gta_bytecode_execution_context_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Execution_Context * self)) {
  GTA_VECTORX_DESTROY(self->stack);
  if (self->result) {
    gta_computed_value_destroy(self->result);
  }
  gta_unicode_string_destroy(self->output);
}