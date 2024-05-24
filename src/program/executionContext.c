
#include <string.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValue.h>
#include <tang/program/executionContext.h>

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
  GTA_VectorX * stack = GTA_VECTORX_CREATE(32);
  if (!stack) {
    return false;
  }
  GTA_VectorX * garbage_collection = GTA_VECTORX_CREATE(32);
  if (!garbage_collection) {
    goto GARBAGE_COLLECTION_VECTOR_CREATE_FAILED;
  }
  GTA_HashX * globals = GTA_HASHX_CREATE(32);
  if (!globals) {
    goto GLOBALS_HASH_CREATE_FAILED;
  }
  GTA_Unicode_String * output = gta_unicode_string_create("", 0, GTA_UNICODE_STRING_TYPE_TRUSTED);
  if (!output) {
    goto OUTPUT_STRING_CREATE_FAILED;
  }

  *context = (GTA_Execution_Context) {
    .program = program,
    .output = output,
    .result = 0,
    .stack = stack,
    .pc_stack = 0,
    .garbage_collection = garbage_collection,
    .globals = globals,
    .user_data = 0,
    .fp = 0,
  };
  return true;

  // Failure conditions.
OUTPUT_STRING_CREATE_FAILED:
  GTA_HASHX_DESTROY(globals);
GLOBALS_HASH_CREATE_FAILED:
  GTA_VECTORX_DESTROY(garbage_collection);
GARBAGE_COLLECTION_VECTOR_CREATE_FAILED:
  GTA_VECTORX_DESTROY(stack);
  return false;
}


void gta_execution_context_destroy(GTA_Execution_Context * self) {
  gta_execution_context_destroy_in_place(self);
  gcu_free(self);
}


void gta_execution_context_destroy_in_place(GTA_Execution_Context * self) {
  GTA_VECTORX_DESTROY(self->stack);
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


bool gta_execution_context_add_library(GTA_Execution_Context * context, const char * identifier, GTA_Execution_Context_Global_Create func) {
  return GTA_HASHX_SET(context->globals, GTA_STRING_HASH(identifier, strlen(identifier)), GTA_TYPEX_MAKE_P((Function_Converter){.f = func}.b));
}
