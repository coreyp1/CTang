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

#include "tang/computedValue.h"

/**
 * The vtable for the GTA_Computed_Value_Error class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_error_vtable;

/**
 * Generic "not implemented" singleton error value.
 */
extern GTA_Computed_Value * gta_computed_value_error_not_implemented;

/**
 * Generic "out of memory" singleton error value.
 */
extern GTA_Computed_Value * gta_computed_value_error_out_of_memory;

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
