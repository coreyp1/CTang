/**
 * @file
 */

#ifndef G_TANG_COMPUTED_VALUE_FUNCTION_NATIVE_H
#define G_TANG_COMPUTED_VALUE_FUNCTION_NATIVE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/computedValue/computedValue.h>


/**
 * The vtable for the GTA_Computed_Value_Function_Native class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_function_native_vtable;

/**
 * The function signature required for use by the
 * GTA_Computed_Value_Function_Native class.
 *
 * @param bound_object The object to which the function is bound, or NULL if the
 *   function is not bound to an object.
 * @param argc The number of arguments passed to the function.
 * @param argv The array of computed value pointer arguments passed to the function.
 * @param context The execution context in which the function is being called.
 * @return The computed value resulting from the function call.
 */
typedef GTA_Computed_Value * GTA_Computed_Value_Function_Native_Callback(GTA_Computed_Value *bound_object, GTA_UInteger argc, GTA_Computed_Value * argv[], GTA_Execution_Context *context);

/**
 * The computed value representing a natively-compiled function that may or may
 * not be bound to an object.
 */
struct GTA_Computed_Value_Function_Native {
  /**
   * The base class of the computed value.
   */
  GTA_Computed_Value base;
  /**
   * The function to call when the computed value is called.
   */
  GTA_Computed_Value_Function_Native_Callback * callback;
  /**
   * The object to which the function is bound, or NULL if the function is not
   * bound to an object.
   */
  GTA_Computed_Value * bound_object;
};

/**
 * Create a new computed value for a natively-compiled function.
 *
 * @param callback The function to be called.
 * @param bound_object The object to which the function is bound, or NULL if the
 *   function is not bound to an object.
 * @param context The execution context to create the value in.
 * @return The new computed value for the natively-compiled function.
 */
GTA_NO_DISCARD GTA_Computed_Value_Function_Native * GTA_CALL gta_computed_value_function_native_create(GTA_Computed_Value_Function_Native_Callback * callback, GTA_Computed_Value * bound_object, GTA_Execution_Context * context);

/**
 * Create a new computed value for a natively-compiled function in place.
 *
 * @param self The memory address of the computed value.
 * @param callback The function to be called.
 * @param bound_object The object to which the function is bound, or NULL if the
 *   function is not bound to an object.
 * @param context The execution context to create the value in.
 * @return True if the operation was successful, false otherwise.
 */
bool GTA_CALL gta_computed_value_function_native_create_in_place(GTA_Computed_Value_Function_Native * self, GTA_Computed_Value_Function_Native_Callback * callback, GTA_Computed_Value * bound_object, GTA_Execution_Context * context);

/**
 * Destroy a computed value for a natively-compiled function.
 *
 * @see gta_computed_value_destroy
 *
 * @param computed_value The computed value for the natively-compiled function.
 */
void GTA_CALL gta_computed_value_function_native_destroy(GTA_Computed_Value * computed_value);

/**
 * Destroy a computed value for a natively-compiled function in place.
 *
 * @see gta_computed_value_destroy_in_place
 *
 * @param self The computed value for the natively-compiled function.
 */
void GTA_CALL gta_computed_value_function_native_destroy_in_place(GTA_Computed_Value * self);

/**
 * Deep copy a computed value for a natively-compiled function.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param value The computed value to be copied.
 * @param context The execution context of the program.
 * @return The deep copy of the ComputedValueInteger or NULL if an error
 *   occurred.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_function_native_deep_copy(GTA_Computed_Value * value, GTA_Execution_Context * context);

/**
 * Get a string representation of the computed value for a natively-compiled
 * function.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @see gta_computed_value_to_string
 *
 * @param self The computed value for the natively-compiled function.
 * @return The string representation of the computed value for the
 *   natively-compiled function.
 */
GTA_NO_DISCARD char * GTA_CALL gta_computed_value_function_native_to_string(GTA_Computed_Value * self);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_COMPUTED_VALUE_FUNCTION_NATIVE_H
