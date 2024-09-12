/**
 * @file
 *
 * The Random library for random number generators.
 */

#ifndef TANG_LIBRARY_LIBRARYRANDOM_H
#define TANG_LIBRARY_LIBRARYRANDOM_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/macros.h>
#include <tang/library/library.h>

/**
 * The Random library singleton.
 */
extern GTA_Computed_Value * gta_computed_value_library_random;

/**
 * Load the Random library.
 *
 * @param context The context of the program being executed.
 * @return The computed value for the Random library.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_library_random_load(GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_LIBRARY_LIBRARYRANDOM_H
