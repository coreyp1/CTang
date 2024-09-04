
#include <assert.h>
#include <cutil/hash.h>
#include <cutil/memory.h>
#include <tang/program/language.h>

GTA_Language * gta_language_create(void) {
  GTA_Language * language = gcu_malloc(sizeof(GTA_Language));
  if (language == NULL) {
    goto LANGUAGE_CREATE_FAILED;
  }

  language->libraries = GTA_HASHX_CREATE(32);
  if (language->libraries == NULL) {
    goto LIBRARY_HASH_CREATE_FAILED;
  }
  return language;

LIBRARY_HASH_CREATE_FAILED:
  free(language);
LANGUAGE_CREATE_FAILED:
  return NULL;
}

void gta_language_destroy(GTA_Language * language) {
  assert(language);
  assert(language->libraries);

  GTA_HASHX_DESTROY(language->libraries);
  gcu_free(language);
}
