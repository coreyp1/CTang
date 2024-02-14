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

#include <tang/computedValue.h>

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
 * @return The new Computed Value Float or NULL on failure.
 */
GTA_Computed_Value_Float * gta_computed_value_float_create(GTA_Float value);

/**
 * Destroy a Computed Value Float.
 *
 * @param computed_value The Computed Value Float.
 */
void gta_computed_value_float_destroy(GTA_Computed_Value * computed_value);

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
