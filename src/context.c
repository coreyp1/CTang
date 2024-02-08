
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
  context->program = program;
  return true;
}

void gta_context_destroy(GTA_Context * self) {
  gta_context_destroy_in_place(self);
  gcu_free(self);
}

void gta_context_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Context * self)) {
}
