
#include <tang/library/libraryMath.h>
#include <tang/computedValue/computedValueInteger.h>
#include <tang/computedValue/computedValueFloat.h>
#include <tang/computedValue/computedValueLibrary.h>

/**
 * Math library attribute to get the value of pi.
 *
 * @param context The context of the program being executed.
 * @return The computed value for pi (as a singleton).
 */
static GTA_Computed_Value * GTA_CALL gta_library_math_make_pi(GTA_Execution_Context * context);


/**
 * The attributes of the Math library.
 */
static GTA_Computed_Value_Library_Attribute_Pair attributes[] = {
  {"pi", gta_library_math_make_pi},
};


/**
 * The Math library singleton.
 */
static GTA_Computed_Value_Library gta_computed_value_library_math_singleton = {
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
  .name = "Math",
  .attributes = attributes,
  .attribute_count = 0,
  .library = 0,
};
GTA_Computed_Value * gta_computed_value_library_math = (GTA_Computed_Value *)&gta_computed_value_library_math_singleton;


/**
 * The computed value for pi singleton.
 */
static GTA_Computed_Value_Float gta_computed_value_library_math_pi_singleton = {
  .base = {
    .vtable = &gta_computed_value_float_vtable,
    .context = 0,
    .is_true = true,
    .is_error = false,
    .is_temporary = false,
    .requires_deep_copy = false,
    .is_singleton = true,
    .is_a_reference = false,
  },
  .value = 3.14159265358979323846,
};
GTA_Computed_Value * gta_computed_value_library_math_pi = (GTA_Computed_Value *)&gta_computed_value_library_math_pi_singleton;


static GTA_Computed_Value * GTA_CALL gta_library_math_make_pi(GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_library_math_pi;
}


GTA_Computed_Value * GTA_CALL gta_library_math_load(GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_library_math;
}


/**
 * Setup the proper attribute count and build the library attribute hash, which
 * cannot be done at compile time.
 */
GTA_INIT_FUNCTION(setup) {
  gta_computed_value_library_math_singleton.attribute_count = sizeof(attributes) / sizeof(GTA_Computed_Value_Library_Attribute_Pair);
  if (!gta_computed_value_library_build_library_attributes_hash(&gta_computed_value_library_math_singleton)) {
    if (gta_computed_value_library_math_singleton.library) {
      gta_library_destroy(gta_computed_value_library_math_singleton.library);
      gta_computed_value_library_math_singleton.library = 0;
    }
  }
}


