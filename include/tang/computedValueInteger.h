/**
 * @file
 *
 * Header file for the ComputedValueInteger class.
 */

#ifndef TANG_COMPUTED_VALUE_INTEGER_H
#define TANG_COMPUTED_VALUE_INTEGER_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/computedValue.h>
#include <tang/macros.h>

/**
 * The VTable for the ComputedValueInteger class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_integer_vtable;

/**
 * The computed value for an integer.
 */
typedef struct GTA_Computed_Value_Integer {
  /**
   * The base class for the computed value.
   */
  GTA_Computed_Value base;
  /**
   * The value of the integer.
   */
  GTA_Integer value;
} GTA_Computed_Value_Integer;

/**
 * Create a new computed value for an integer.
 *
 * @param value The value of the integer.
 * @return The new computed value for the integer.
 */
GTA_NO_DISCARD GTA_Computed_Value_Integer * GTA_CALL gta_computed_value_integer_create(GTA_Integer value);

/**
 * Destroy a computed value for an integer.
 *
 * @see gta_computed_value_destroy
 *
 * @param computed_value The computed value for the integer.
 */
void GTA_CALL gta_computed_value_integer_destroy(GTA_Computed_Value * computed_value);

/**
 * Deep copy a computed value for an integer.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param value The computed value to be copied.
 * @return The deep copy of the ComputedValueInteger or NULL if an error occurred.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_integer_deep_copy(GTA_Computed_Value * value);

/**
 * Get a string representation of the computed value for an integer.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @see gta_computed_value_to_string
 *
 * @param self The computed value for the integer.
 * @return The string representation of the computed value for the integer.
 */
GTA_NO_DISCARD char * GTA_CALL gta_computed_value_integer_to_string(GTA_Computed_Value * self);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_COMPUTED_VALUE_INTEGER_H
