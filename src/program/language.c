
#include <assert.h>
#include <ghoti.io/cutil/memory.h>
#include <ghoti.io/tang/macros.h>
#include <ghoti.io/tang/computedValue/computedValueLibrary.h>
#include <ghoti.io/tang/library/library.h>
#include <ghoti.io/tang/library/libraryMath.h>
#include <ghoti.io/tang/library/libraryRandom.h>
#include <ghoti.io/tang/program/language.h>

GTA_Language * gta_language_create(void) {
  GTA_Language * language = gcu_malloc(sizeof(GTA_Language));
  if (language == NULL) {
    goto LANGUAGE_CREATE_FAILED;
  }

  language->library = gta_library_create();
  if (language->library == NULL) {
    goto LIBRARY_HASH_CREATE_FAILED;
  }

  GTA_Computed_Value_Library_Attribute_Pair libraries[] = {
    {"math", gta_library_math_load},
    {"random", gta_library_random_load},
  };
  size_t library_count = sizeof(libraries) / sizeof(GTA_Computed_Value_Library_Attribute_Pair);
  for (size_t i = 0; i < library_count; i++) {
    if (!gta_library_add_library_from_string(language->library, libraries[i].name, libraries[i].callback)) {
      goto ADD_LIBRARY_FAILED;
    }
  }
  return language;

ADD_LIBRARY_FAILED:
  gta_library_destroy(language->library);
  language->library = NULL;
LIBRARY_HASH_CREATE_FAILED:
  free(language);
LANGUAGE_CREATE_FAILED:
  return NULL;
}


void gta_language_destroy(GTA_Language * language) {
  assert(language);
  assert(language->library);

  gta_library_destroy(language->library);
  gcu_free(language);
}
