
#include <assert.h>
#include <cutil/hash.h>
#include <cutil/memory.h>
#include <tang/library/library.h>

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


bool gta_library_add_library(GTA_Library * library, GTA_UInteger hash, GTA_Library_Callback func) {
  assert(library);
  assert(library->manifest);
  return GTA_HASHX_SET(library->manifest, hash, GTA_TYPEX_MAKE_P((GTA_Library_Callback_Function_Converter){.f = func}.b));
}


GTA_HashX_Value gta_library_get_library(GTA_Library * library, GTA_UInteger hash) {
  assert(library);
  assert(library->manifest);
  return GTA_HASHX_GET(library->manifest, hash);
}
