/**
 * @file
 *
 * @brief Header file for the computedValueFloat class
 */

#ifndef G_TANG_COMPUTEDVALUEFLOAT_H
#define G_TANG_COMPUTEDVALUEFLOAT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/computedValue/computedValue.h>

/**
 * The VTable for the ComputedValueFloat class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_float_vtable;

/**
 * The Computed Value Float class.
 */
typedef struct GTA_Computed_Value_Float {
  /**
   * The base class.
   */
  GTA_Computed_Value base;
  /**
   * The value.
   */
  GTA_Float value;
} GTA_Computed_Value_Float;

/**
 * Create a new Computed Value Float.
 *
 * @param value The value.
 * @param context The execution context to create the value in.
 * @return The new Computed Value Float or NULL on failure.
 */
GTA_Computed_Value_Float * gta_computed_value_float_create(GTA_Float value, GTA_Execution_Context * context);

/**
 * Create a new computed value for a float in place.
 *
 * @param self The memory address of the computed value.
 * @param value The value of the float.
 * @param context The execution context to create the value in.
 * @return True if the operation was successful, false otherwise.
 */
bool GTA_CALL gta_computed_value_float_create_in_place(GTA_Computed_Value_Float * self, GTA_Float value, GTA_Execution_Context * context);

/**
 * Destroy a Computed Value Float.
 *
 * @param computed_value The Computed Value Float.
 */
void gta_computed_value_float_destroy(GTA_Computed_Value * computed_value);

/**
 * Destroy a computed value for a float in place.
 *
 * @see gta_computed_value_destroy_in_place
 *
 * @param self The computed value for the float.
 */
void GTA_CALL gta_computed_value_float_destroy_in_place(GTA_Computed_Value * self);

/**
 * Deep copy a computed value for an float.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param value The computed value to be copied.
 * @return The deep copy of the ComputedValueInteger or NULL if an error occurred.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_deep_copy(GTA_Computed_Value * value);

/**
 * Get a string representation of the computed value for a float.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @see gta_computed_value_to_string
 *
 * @param self The computed value for the float.
 * @return The string representation of the computed value for the float.
 */
GTA_NO_DISCARD char * GTA_CALL gta_computed_value_float_to_string(GTA_Computed_Value * self);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_COMPUTEDVALUEFLOAT_H
