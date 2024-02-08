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

extern GTA_Computed_Value_VTable gta_computed_value_error_vtable;

extern GTA_Computed_Value * gta_computed_value_error_not_implemented;

typedef struct GTA_Computed_Value_Error {
  GTA_Computed_Value base;
  char * message;
} GTA_Computed_Value_Error;

GTA_Computed_Value * gta_computed_value_error_create(const char * message);
char * gta_computed_value_error_to_string(GTA_Computed_Value * self);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_COMPUTED_VALUE_ERROR_H
