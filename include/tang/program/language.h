/**
 * @file
 */

#ifndef G_TANG_PROGRAM_LANGUAGE_H
#define G_TANG_PROGRAM_LANGUAGE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/macros.h>

/**
 * This structure holds the metadata pertaining to a language.
 *
 * A program, when created, is associated with a language.  This allows a
 * single host program to support multiple incarnations of Tang by tailoring
 * the details of the language (such as the available libraries) to the needs
 * of the program(s) with which it is interacting.
 */
typedef struct GTA_Language {
  /**
   * The general libraries available to the language.
   */
  GTA_HashX * libraries;
} GTA_Language;

/**
 * Create a new language.
 *
 * @return A pointer to the new language structure, or NULL on failure.
 */
GTA_NO_DISCARD GTA_Language * gta_language_create(void);

/**
 * Destroy the given language.
 *
 * @param language The language to destroy.
 */
void gta_language_destroy(GTA_Language * language);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_PROGRAM_LANGUAGE_H
