/**
 * @file
 * Header file used to control the version numbering and function namespace
 * for all of the library.
 */

#ifndef G_TANG_LIBVER_H
#define G_TANG_LIBVER_H

#include <wchar.h>

/**
 * Used in conjunction with the G_TANG... macros to produce a namespaced
 * function name for use by all exported functions in this library.
 */
#define G_TANG_NAME ghotiio_tang_dev

/**
 * String representation of the version, provided as a convenience to the
 * programmer.
 */
#define G_TANG_VERSION "dev"

/**
 * Macro to generate a "namespaced" version of an identifier.
 *
 * Notice, we cannot use G_TANG_CONCAT2(), because the preprocessor dies
 * in some cases with nested use (see vector.template.c).
 *
 * @param NAME The name which will be prepended with the `G_TANG_NAME`.
 */
#define G_TANG(NAME) G_TANG_RENAME(G_TANG_NAME, _ ## NAME)

/**
 * Helper macro to concatenate the `#define`s properly.  It requires two levels
 * of processing.
 *
 * This macro should only be called by the `G_TANG_CONCAT()` macro.
 *
 * @param a The first part of the identifier.
 * @param b The second part of the identifier.
 * @returns The concatenation of `a` to `b`.
 */
#define G_TANG_RENAME_INNER(a,b) a ## b

/**
 * Helper macro to concatenate the `#define`s properly.  It requires two levels
 * of processing.
 *
 * @param a The first part of the identifier.
 * @param b The second part of the identifier.
 * @returns A call to the `G_TANG_RENAME_INNER()` macro.
 */
#define G_TANG_RENAME(a,b) G_TANG_RENAME_INNER(a,b)

/**
 * Helper macro to concatenate the identifiers.  It reuires two levels of
 * processing.
 *
 * This macro should not be called directly.  It should only be called by
 * G_TANG_CONCAT2().
 *
 * @param a The first part of the identifier.
 * @param b The second part of the identifier.
 * @returns The concatenation of `a` to `b`.
 */
#define G_TANG_CONCAT2_INNER(a,b) a ## b

/**
 * Helper macro to concatenate the identifiers.  It requires two levels
 * of processing.
 *
 * This macro may be called directly.
 *
 * @param a The first part of the identifier.
 * @param b The second part of the identifier.
 * @returns A call to the `G_TANG_CONCAT2_INNER()` macro.
 */
#define G_TANG_CONCAT2(a,b) G_TANG_CONCAT2_INNER(a,b)

/**
 * Helper macro to concatenate the identifiers.  It reuires two levels of
 * processing.
 *
 * This macro should not be called directly.  It should only be called by
 * G_TANG_CONCAT2().
 *
 * @param a The first part of the identifier.
 * @param b The second part of the identifier.
 * @param c The third part of the identifier.
 * @returns The concatenation of `a` to `b` to `c`..
 */
#define G_TANG_CONCAT3_INNER(a,b,c) a ## b ## c

/**
 * Helper macro to concatenate the identifiers.  It requires two levels
 * of processing.
 *
 * This macro may be called directly.
 *
 * @param a The first part of the identifier.
 * @param b The second part of the identifier.
 * @param c The third part of the identifier.
 * @returns A call to the `G_TANG_CONCAT3_INNER()` macro.
 */
#define G_TANG_CONCAT3(a,b,c) G_TANG_CONCAT3_INNER(a,b,c)

//-----------------------------------------------------------------------------
// Compiler Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Type-Related Definitions
//-----------------------------------------------------------------------------

/// @cond HIDDEN_SYMBOLS
#define GTA_Unicode_String G_TANG(GTA_Unicode_String)
/// @endcond HIDDEN_SYMBOLS



#if DOXYGEN
/**
 * Indicate the size of the `wchar` type.
 */
#define GCU_WCHAR_WIDTH

/**
 * Indicate whether the `wchar` type is signed in this implementation.
 */
#define GCU_WCHAR_SIGNED
#endif // DOXYGEN

// Define helper macros for signed and unsigned max values
#define MAX_UINT64 0xFFFFFFFFFFFFFFFF
#define MAX_UINT32 0xFFFFFFFF
#define MAX_UINT16 0xFFFF

#define MAX_INT64  0x7FFFFFFFFFFFFFFF
#define MAX_INT32  0x7FFFFFFF
#define MAX_INT16  0x7FFF


#if (WCHAR_MAX == MAX_UINT64) || (WCHAR_MAX == MAX_INT64)
// 64-bit signed
#define GCU_WCHAR_WIDTH 8
#define GCU_WCHAR_SIGNED (WCHAR_MAX == MAX_INT64)

#elif (WCHAR_MAX == MAX_UINT32) || (WCHAR_MAX == MAX_INT32)
// 32-bit signed
#define GCU_WCHAR_WIDTH 4
#define GCU_WCHAR_SIGNED (WCHAR_MAX == MAX_INT32)

#elif (WCHAR_MAX == MAX_UINT16) || (WCHAR_MAX == MAX_INT16)
// 16-bit signed
#define GCU_WCHAR_WIDTH 2
#define GCU_WCHAR_SIGNED (WCHAR_MAX == MAX_INT16)

#else
#error "Could not determine GCU_WCHAR_WIDTH and GCU_WCHAR_SIGNED"

#endif // GCU_WCHAR_WIDTH and GCU_WCHAR_SIGNED


#endif // G_TANG_LIBVER_H
 
