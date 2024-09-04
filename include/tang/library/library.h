/**
 * @file
 */

#ifndef TANG_LIBRARY_LIBRARY_H
#define TANG_LIBRARY_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <cutil/hash.h>
#include <tang/macros.h>

/**
 * The library structure.
 */
typedef struct GTA_Library {
  /**
   * The entries in the library.
   */
  GTA_HashX * manifest;
} GTA_Library;

/**
 * Create a new library object.
 *
 * @return The new library object or NULL if it could not be created.
 */
GTA_NO_DISCARD GTA_Library * gta_library_create(void);

/**
 * Destroy the given library object.
 *
 * @param library The library object to destroy.
 */
void gta_library_destroy(GTA_Library * library);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_LIBRARY_LIBRARY_H
