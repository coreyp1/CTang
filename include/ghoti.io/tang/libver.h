/**
 * @file
 * Identity of this build, and the machinery that stamps it onto a name.
 *
 * This file holds two things and nothing else: where the version token comes
 * from, and the macros that paste it onto an identifier.  The list of names to
 * stamp is in namespace.h; the compiler and platform attributes are in
 * macros.h.  See CONVENTIONS.md section 4.
 */

#ifndef GHOTI_IO_GTA_LIBVER_H
#define GHOTI_IO_GTA_LIBVER_H

/**
 * Used in conjunction with the GHOTIIO_TANG... macros to produce a namespaced
 * function name for use by all exported functions in this library.
 */
#include <ghoti.io/tang/libver_gen.h>

/**
 * Macro to generate a "namespaced" version of an identifier.
 *
 * Notice, we cannot use GHOTIIO_TANG_CONCAT2(), because the preprocessor dies
 * in some cases with nested use (see vector.template.c).
 *
 * @param NAME The name which will be prepended with the `GHOTIIO_TANG_NAME`.
 */
#define GHOTIIO_TANG(NAME) GHOTIIO_TANG_RENAME(GHOTIIO_TANG_NAME, _ ## NAME)


/**
 * Helper macro to concatenate the `#define`s properly.  It requires two levels
 * of processing.
 *
 * This macro should only be called by the `GHOTIIO_TANG_CONCAT()` macro.
 *
 * @param a The first part of the identifier.
 * @param b The second part of the identifier.
 * @returns The concatenation of `a` to `b`.
 */
#define GHOTIIO_TANG_RENAME_INNER(a,b) a ## b

/**
 * Helper macro to concatenate the `#define`s properly.  It requires two levels
 * of processing.
 *
 * @param a The first part of the identifier.
 * @param b The second part of the identifier.
 * @returns A call to the `GHOTIIO_TANG_RENAME_INNER()` macro.
 */
#define GHOTIIO_TANG_RENAME(a,b) GHOTIIO_TANG_RENAME_INNER(a,b)

/**
 * Helper macro to concatenate the identifiers.  It reuires two levels of
 * processing.
 *
 * This macro should not be called directly.  It should only be called by
 * GHOTIIO_TANG_CONCAT2().
 *
 * @param a The first part of the identifier.
 * @param b The second part of the identifier.
 * @returns The concatenation of `a` to `b`.
 */
#define GHOTIIO_TANG_CONCAT2_INNER(a,b) a ## b

/**
 * Helper macro to concatenate the identifiers.  It requires two levels
 * of processing.
 *
 * This macro may be called directly.
 *
 * @param a The first part of the identifier.
 * @param b The second part of the identifier.
 * @returns A call to the `GHOTIIO_TANG_CONCAT2_INNER()` macro.
 */
#define GHOTIIO_TANG_CONCAT2(a,b) GHOTIIO_TANG_CONCAT2_INNER(a,b)

/**
 * Helper macro to concatenate the identifiers.  It reuires two levels of
 * processing.
 *
 * This macro should not be called directly.  It should only be called by
 * GHOTIIO_TANG_CONCAT2().
 *
 * @param a The first part of the identifier.
 * @param b The second part of the identifier.
 * @param c The third part of the identifier.
 * @returns The concatenation of `a` to `b` to `c`..
 */
#define GHOTIIO_TANG_CONCAT3_INNER(a,b,c) a ## b ## c

/**
 * Helper macro to concatenate the identifiers.  It requires two levels
 * of processing.
 *
 * This macro may be called directly.
 *
 * @param a The first part of the identifier.
 * @param b The second part of the identifier.
 * @param c The third part of the identifier.
 * @returns A call to the `GHOTIIO_TANG_CONCAT3_INNER()` macro.
 */
#define GHOTIIO_TANG_CONCAT3(a,b,c) GHOTIIO_TANG_CONCAT3_INNER(a,b,c)

#endif // GHOTI_IO_GTA_LIBVER_H
 
