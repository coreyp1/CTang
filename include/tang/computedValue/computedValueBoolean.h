/**
 * @file
 *
 * Header file for the computedValueBoolean class.
 */

#ifndef G_TANG_COMPUTED_VALUE_BOOLEAN_H
#define G_TANG_COMPUTED_VALUE_BOOLEAN_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/computedValue/computedValue.h>

/**
 * The vtable for the GTA_Computed_Value_Boolean class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_boolean_vtable;

/**
 * A singleton representing a true value.
 */
extern GTA_Computed_Value * gta_computed_value_boolean_true;

/**
 * A singleton representing a false value.
 */
extern GTA_Computed_Value * gta_computed_value_boolean_false;

/**
 * The GTA_Computed_Value_Boolean class.
 */
typedef struct GTA_Computed_Value_Boolean {
  GTA_Computed_Value base;
  bool value;
} GTA_Computed_Value_Boolean;

/**
 * Initializes a GTA_Computed_Value_Boolean object.
 *
 * @param value The boolean value of the computed value.
 * @param context The execution context in which to create the value.
 * @return The initialized object or NULL if an error occurred.
 */
GTA_Computed_Value_Boolean * GTA_CALL gta_computed_value_boolean_create(bool value, GTA_Execution_Context * context);

/**
 * Create a computed value for a boolean in place.
 *
 * @param self The memory address of the computed value.
 * @param value The boolean value of the computed value.
 * @param context The execution context in which to create the value.
 * @return True if the operation was successful, false otherwise.
 */
bool GTA_CALL gta_computed_value_boolean_create_in_place(GTA_Computed_Value_Boolean * self, bool value, GTA_Execution_Context * context);

/**
 * Destroys a GTA_Computed_Value_Boolean object.
 *
 * This function should not be called directly. Instead, use the
 * gta_computed_value_destroy() function.
 *
 * @see gta_computed_value_destroy
 *
 * @param self The object to destroy.
 */
void GTA_CALL gta_computed_value_boolean_destroy(GTA_Computed_Value * self);

/**
 * Destroy a computed value for a boolean in place.
 *
 * @see gta_computed_value_destroy_in_place
 *
 * @param self The memory address of the computed value.
 */
void GTA_CALL gta_computed_value_boolean_destroy_in_place(GTA_Computed_Value * self);

/**
 * Deep copy a computed value for a boolean.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param value The computed value to be copied.
 * @param context The execution context of the program.
 * @return The deep copy of the GTA_Computed_Value_Boolean or NULL if an error occurred.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_boolean_deep_copy(GTA_Computed_Value * value, GTA_Execution_Context * context);

/**
 * Get a string representation of the computed value for a boolean.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @see gta_computed_value_to_string
 *
 * @param self The computed value for the boolean.
 * @return The string representation of the computed value for the boolean.
 */
GTA_NO_DISCARD char * GTA_CALL gta_computed_value_boolean_to_string(GTA_Computed_Value * self);

/**
 * Casts a computed value to a different type.
 *
 * Calls the `cast` method of the virtual table.
 *
 * @param self The object to cast.
 * @param type The type to cast to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_boolean_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_COMPUTED_VALUE_BOOLEAN_H
