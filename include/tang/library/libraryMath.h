/**
 * @file
 */

#ifndef TANG_LIBRARY_LIBRARYMATH_H
#define TANG_LIBRARY_LIBRARYMATH_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/macros.h>
#include <tang/library/library.h>

/**
 * The Math library singleton.
 */
extern GTA_Computed_Value * gta_computed_value_library_math;

/**
 * A singleton representing the value of pi.
 */
extern GTA_Computed_Value * gta_computed_value_library_math_pi;

/**
 * Load the Math library.
 *
 * @param context The context of the program being executed.
 * @return The computed value for the Math library.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_library_math_load(GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_LIBRARY_LIBRARYMATH_H
