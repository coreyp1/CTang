
#include <cutil/memory.h>
#include "tang/context.h"
#include "tang/macros.h"

GTA_Context * gta_context_create(GTA_Program * program) {
  GTA_Context * context = gcu_malloc(sizeof(GTA_Context));
  if (!context) {
    return 0;
  }

  if (!gta_context_create_in_place(context, program)) {
    gcu_free(context);
    return 0;
  }

  return context;
}

bool gta_context_create_in_place(GTA_Context * context, GTA_Program * program) {
  *context = (GTA_Context) {
    .program = program,
    .output = 0,
    .result = 0,
    .stack = 0,
  };
  context->stack = gcu_vector64_create(32);
  if (!context->stack) {
    return false;
  }
  context->output = gta_unicode_string_create("", 0, GTA_UNICODE_STRING_TYPE_TRUSTED);
  if (!context->output) {
    gcu_vector64_destroy(context->stack);
    return false;
  }
  return true;
}

void gta_context_destroy(GTA_Context * self) {
  gta_context_destroy_in_place(self);
  gcu_free(self);
}

void gta_context_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Context * self)) {
  gcu_vector64_destroy(self->stack);
  gta_unicode_string_destroy(self->output);
}
