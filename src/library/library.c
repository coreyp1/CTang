
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
  library->manifest = GTA_HASHX_CREATE(32);
  if (library->manifest == NULL) {
    goto MANIFEST_CREATE_FAILED;
  }
  return library;

MANIFEST_CREATE_FAILED:
  gcu_free(library);
LIBRARY_CREATE_FAILED:
  return NULL;
}


void gta_library_destroy(GTA_Library * library) {
  assert(library);
  assert(library->manifest);

  GTA_HASHX_DESTROY(library->manifest);
  gcu_free(library);
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
