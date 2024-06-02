/**
 * @file
 *
 * Header file for the computed value error class.
 *
 * Also defines some general-purpose error values.
 */

#ifndef G_TANG_COMPUTED_VALUE_ERROR_H
#define G_TANG_COMPUTED_VALUE_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/computedValue/computedValue.h>

/**
 * The vtable for the GTA_Computed_Value_Error class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_error_vtable;

/**
 * Generic "Not implemented" singleton error value.
 */
extern GTA_Computed_Value * gta_computed_value_error_not_implemented;

/**
 * Generic "Out of memory" singleton error value.
 */
extern GTA_Computed_Value * gta_computed_value_error_out_of_memory;

/**
 * Generic "invalid bytecode" singleton error value.
 */
extern GTA_Computed_Value * gta_computed_value_error_invalid_bytecode;

/**
 * Generic "Divide by zero" singleton error value.
 */
extern GTA_Computed_Value * gta_computed_value_error_divide_by_zero;

/**
 * Generic "Modulo by zero" singleton error value.
 */
extern GTA_Computed_Value * gta_computed_value_error_modulo_by_zero;

/**
 * Generic "not supported" singleton error value.
 *
 * In contrast to the "not implemented" error, this error is used when the
 * operation is known but not supported by the current implementation.
 * For example, in the code example `"hello" / 3`, the division operation is
 * known but you can't divide a string by a number.
 *
 * If this error is encountered when evaluating a binary operation, then the
 * operation should be retried with the operands swapped and the reversed flag
 * set.
 */
extern GTA_Computed_Value * gta_computed_value_error_not_supported;

/**
 * Represents an error value.
 */
typedef struct GTA_Computed_Value_Error {
  /**
   * The base class.
   */
  GTA_Computed_Value base;
  /**
   * The error message.
   */
  char * message;
} GTA_Computed_Value_Error;

/**
 * Creates a new GTA_Computed_Value_Error object.
 *
 * @param message The error message.
 * @return The new GTA_Computed_Value_Error object or NULL on failure.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_error_create(const char * message);

/**
 * Produce a string representation of the error.  The caller is responsible for
 * freeing the returned string.
 *
 * @param self The GTA_Computed_Value_Error object.
 * @return The string representation of the error.
 */
GTA_NO_DISCARD char * gta_computed_value_error_to_string(GTA_Computed_Value * self);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_COMPUTED_VALUE_ERROR_H
