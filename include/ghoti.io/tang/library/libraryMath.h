/**
 * @file
 */

#ifndef GHOTI_IO_GTA_LIBRARY_LIBRARYMATH_H
#define GHOTI_IO_GTA_LIBRARY_LIBRARYMATH_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <ghoti.io/tang/library/library.h>

/**
 * The Math library singleton.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_library_math;

/**
 * A singleton representing the value of pi.
 */
GTA_API extern GTA_Computed_Value * gta_computed_value_library_math_pi;

/**
 * Load the Math library.
 *
 * @param context The context of the program being executed.
 * @return The computed value for the Math library.
 */
GTA_API GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_library_math_load(GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GHOTI_IO_GTA_LIBRARY_LIBRARYMATH_H
