
#include <assert.h>
#include <string.h>
#include <cutil/hash.h>
#include <cutil/memory.h>
#include <cutil/string.h>
#include <tang/library/library.h>
#include <tang/program/program.h>
#include <tang/program/executionContext.h>
#include <tang/computedValue/computedValue.h>
#include <tang/computedValue/computedValueError.h>

/**
 * Helper class, only existing to aid in the transform a function pointer to a
 * value able to be stored in the hash.
 */
typedef union GTA_Library_Callback_Function_Converter {
  GTA_Library_Callback f;
  void * b;
} GTA_Library_Callback_Function_Converter;


GTA_Library * gta_library_create(void) {
  GTA_Library * library = gcu_malloc(sizeof(GTA_Library));
  if (library == NULL) {
    goto LIBRARY_CREATE_FAILED;
  }

  if (!gta_library_create_in_place(library)) {
    goto IN_PLACE_CREATE_FAILED;
  }
  return library;

IN_PLACE_CREATE_FAILED:
  gcu_free(library);
LIBRARY_CREATE_FAILED:
  return NULL;
}


bool gta_library_create_in_place(GTA_Library * library) {
  assert(library);
  *library = (GTA_Library){
    .manifest = GTA_HASHX_CREATE(32),
  };
  return library->manifest;
}


void gta_library_destroy(GTA_Library * library) {
  assert(library);
  assert(library->manifest);

  gta_library_destroy_in_place(library);
  gcu_free(library);
}


void gta_library_destroy_in_place(GTA_Library * library) {
  assert(library);
  assert(library->manifest);

  GTA_HASHX_DESTROY(library->manifest);
}


bool gta_library_add_library_from_hash(GTA_Library * library, GTA_UInteger hash, GTA_Library_Callback func) {
  assert(library);
  assert(library->manifest);
  return GTA_HASHX_SET(library->manifest, hash, GTA_TYPEX_MAKE_P((GTA_Library_Callback_Function_Converter){.f = func}.b));
}


bool gta_library_add_library_from_string(GTA_Library * library, const char * identifier, GTA_Library_Callback func) {
  assert(identifier);
  return gta_library_add_library_from_hash(library, GTA_STRING_HASH(identifier, strlen(identifier)), func);
}


GTA_Library_Callback gta_library_get_library(GTA_Library * library, GTA_UInteger hash) {
  assert(library);
  assert(library->manifest);
  GTA_HashX_Value result = GTA_HASHX_GET(library->manifest, hash);
  return result.exists ? (GTA_Library_Callback_Function_Converter){.b = GTA_TYPEX_P(result.value)}.f : NULL;
}


GTA_Library_Callback GTA_CALL gta_library_get_from_context(GTA_Execution_Context * context, GTA_UInteger hash) {
  assert(context);
  assert(context->library);
  assert(context->program);
  assert(context->program->language);
  assert(context->program->language->library);

  // Attempt to load the library from the current execution context.
  GTA_Library_Callback func = gta_library_get_library(context->library, hash);

  // If the library was not found in the current execution context, attempt to
  // load it from the program's library.
  if (!func) {
    func = gta_library_get_library(context->program->library, hash);
  }

  // If the library was not found in the program's library, attempt to load it
  // from the language library.
  if (!func) {
    func = gta_library_get_library(context->program->language->library, hash);
  }

  return func;
}
