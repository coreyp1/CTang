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
 * Library callback declaration type.
 *
 * Not all libraries that are available will be used by any particular script.
 * In order to prevent unnecessary memory allocations, libraries will not be
 * instantiated unless they are actually invoked in the script.  When
 * registering a global using gta_library_add_library(), a callback with this
 * signature must be provided.  This callback will be invoked during the script
 * execution.
 *
 * @param context The context of the currently executing script.
 * @return The resulting computed value.
 */
typedef GTA_Computed_Value * GTA_CALL (*GTA_Library_Callback) (GTA_Execution_Context * context);

/**
 * The library structure.
 */
struct GTA_Library {
  /**
   * The entries in the library.
   */
  GTA_HashX * manifest;
};

/**
 * Create a new library object.
 *
 * @return The new library object or NULL if it could not be created.
 */
GTA_NO_DISCARD GTA_Library * GTA_CALL gta_library_create(void);

/**
 * Create a new library object in place.
 *
 * @return True on success, false on failure.
 */
bool GTA_CALL gta_library_create_in_place(GTA_Library * library);

/**
 * Destroy the given library object.
 *
 * @param library The library object to destroy.
 */
void GTA_CALL gta_library_destroy(GTA_Library * library);

/**
 * Destroy the given library object in place.
 *
 * @param library The library object to destroy.
 */
void GTA_CALL gta_library_destroy_in_place(GTA_Library * library);

/**
 * Adds a library entry to the execution context.
 *
 * @param context The execution context.
 * @param hash The hash of the library entry's identifier.
 * @param func The function to be invoked to create the Computed Value.
 * @return true on success, false on failure.
 */
bool GTA_CALL gta_library_add_library_from_hash(GTA_Library * library, GTA_UInteger hash, GTA_Library_Callback func);

/**
 * Adds a library entry to the execution context.
 *
 * @param context The execution context.
 * @param identifier The character string that identifies the library.
 * @param func The function to be invoked to create the Computed Value.
 * @return true on success, false on failure.
 */
bool GTA_CALL gta_library_add_library_from_string(GTA_Library * library, const char * identifier, GTA_Library_Callback func);

/**
 * Get the library entry for the given hash.
 *
 * @param library The library object.
 * @param hash The hash of the library entry identifier.
 * @return The callback function, or NULL on failure.
 */
GTA_Library_Callback GTA_CALL gta_library_get_library(GTA_Library * library, GTA_UInteger hash);

/**
 * Load the library entry for the given hash.
 *
 * This function will search the execution context's library for the supplied
 * hash.  If the hash is not found, the Progam's library will be searched,
 * followed by the Language library.
 *
 * @param context The execution context.
 * @param hash The hash of the library entry identifier.
 * @return The callback function, or NULL on failure.
 */
GTA_Library_Callback GTA_CALL gta_library_get_from_context(GTA_Execution_Context * context, GTA_UInteger hash);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_LIBRARY_LIBRARY_H
