
#include <assert.h>
#include <cutil/hash.h>
#include <cutil/memory.h>
#include <tang/library/library.h>

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
