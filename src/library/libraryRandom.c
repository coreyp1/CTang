
#include <cutil/random.h>
#include <tang/library/libraryRandom.h>
#include <tang/computedValue/computedValueLibrary.h>
#include <tang/computedValue/computedValueRNG.h>

/**
 * Random library attribute to get the global random number generator.
 *
 * @param context The context of the program being executed.
 * @return The computed value for the global random number generator (as a
 *   singleton).
 */
static GTA_Computed_Value * GTA_CALL gta_library_random_make_global(GTA_Execution_Context * context);


/**
 * The attributes of the Random library.
 */
static GTA_Computed_Value_Library_Attribute_Pair attributes[] = {
  {"global", gta_library_random_make_global},
};


/**
 * The Random library singleton.
 */
static GTA_Computed_Value_Library gta_computed_value_library_random_singleton = {
  .base = {
    .vtable = &gta_computed_value_library_vtable,
    .context = 0,
    .is_true = true,
    .is_error = false,
    .is_temporary = false,
    .requires_deep_copy = false,
    .is_singleton = true,
    .is_a_reference = false,
  },
  .name = "random",
  .attributes = attributes,
  .attribute_count = 0,
  .library = 0,
};

GTA_Computed_Value * gta_computed_value_library_random = (GTA_Computed_Value *)&gta_computed_value_library_random_singleton;


static GTA_Computed_Value * GTA_CALL gta_library_random_make_global(GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_library_random_global;
}

GTA_Computed_Value * GTA_CALL gta_library_random_load(GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_library_random;
}

/**
 * Setup the proper attribute count and build the library attribute hash, which
 * cannot be done at compile time.
 */
GTA_INIT_FUNCTION(setup) {
  gta_computed_value_library_random_singleton.attribute_count = sizeof(attributes) / sizeof(GTA_Computed_Value_Library_Attribute_Pair);
  if (!gta_computed_value_library_build_library_attributes_hash(&gta_computed_value_library_random_singleton)) {
    if (gta_computed_value_library_random_singleton.library) {
      gta_library_destroy(gta_computed_value_library_random_singleton.library);
      gta_computed_value_library_random_singleton.library = 0;
    }
  }
}

