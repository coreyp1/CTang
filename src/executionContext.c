
#include <string.h>
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
    .bp_stack = 0,
    .pc_stack = 0,
    .garbage_collection = 0,
    .globals = 0,
    .user_data = 0,
  };
  context->stack = GTA_VECTORX_CREATE(32);
  if (!context->stack) {
    return false;
  }
  context->garbage_collection = GTA_VECTORX_CREATE(32);
  if (!context->garbage_collection) {
    GTA_VECTORX_DESTROY(context->stack);
    return false;
  }
  context->output = gta_unicode_string_create("", 0, GTA_UNICODE_STRING_TYPE_TRUSTED);
  if (!context->output) {
    GTA_VECTORX_DESTROY(context->stack);
    GTA_VECTORX_DESTROY(context->garbage_collection);
    return false;
  }
  context->globals = GTA_HASHX_CREATE(32);
  if (!context->globals) {
    GTA_VECTORX_DESTROY(context->stack);
    GTA_VECTORX_DESTROY(context->garbage_collection);
    gta_unicode_string_destroy(context->output);
    return false;
  }
  return true;
}


void gta_execution_context_destroy(GTA_Execution_Context * self) {
  gta_execution_context_destroy_in_place(self);
  gcu_free(self);
}


void gta_execution_context_destroy_in_place(GTA_Execution_Context * self) {
  GTA_VECTORX_DESTROY(self->stack);
  if (self->bp_stack) {
    GTA_VECTORX_DESTROY(self->bp_stack);
  }
  if (self->pc_stack) {
    GTA_VECTORX_DESTROY(self->pc_stack);
  }
  for (size_t i = 0; i < self->garbage_collection->count; ++i) {
    gta_computed_value_destroy(GTA_TYPEX_P(self->garbage_collection->data[i]));
  }
  GTA_VECTORX_DESTROY(self->garbage_collection);
  GTA_HASHX_DESTROY(self->globals);
  gta_unicode_string_destroy(self->output);
}


/**
 * Helper class, only existing to aid in the transform a function pointer to a
 * value able to be stored in the hash.
 */
typedef union Function_Converter {
  GTA_Execution_Context_Global_Create f;
  void * b;
} Function_Converter;


bool gta_execution_context_add_global(GTA_Execution_Context * context, const char * identifier, GTA_Execution_Context_Global_Create func) {
  return GTA_HASHX_SET(context->globals, GTA_STRING_HASH(identifier, strlen(identifier)), GTA_TYPEX_MAKE_P((Function_Converter){.f = func}.b));
}
