/**
 * @file
 *
 * Unicode String class for use by the Tang language library.
 *
 * Strings are stored as UTF-8 encoded byte arrays.  Strings exist to be used
 * as output from the Tang Language library, and may need to be encoded as
 * percent-encoded strings, html entities, or other formats.  As such, the
 * GTA_Unicode_String object is used to represent a string that is both Unicode
 * aware and can be encoded in a variety of formats, a property which is
 * retained even through concatenation or substring operations.
 *
 * Furthermore, the GTA_Unicode_String object is grapheme-aware, meaning that
 * it can be used to extract substrings based on grapheme positions, rather
 * than byte positions.  This is important for languages such as Thai, where
 * a single character may be represented by multiple bytes, or for emoji, where
 * a single character may be represented by multiple code points.  As an
 * extreme example, the flag of Scotland is represented by 1 grapheme, which is
 * 7 code points, and requires 28 bytes when encoded in UTF-8.  These bytes or
 * code points should not be split up when extracting substrings, so all string
 * operations are grapheme-aware.
 */

#ifndef G_TANG_UNICODESTRING_H
#define G_TANG_UNICODESTRING_H

#include <stddef.h>
#include <cutil/vector.h>
#include <tang/libver.h>
#include <tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get the top 32 bits of a 64-bit integer.
 *
 * @param x The 64-bit integer.
 * @return The top 32 bits of the integer.
 */
#define GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(x) ((x.ui64) >> 32)

/**
 * Get the bottom 32 bits of a 64-bit integer.
 *
 * @param x The 64-bit integer.
 * @return The bottom 32 bits of the integer.
 */
#define GTA_UC_GET_OFFSET_FROM_TYPE_OFFSET_PAIR(x) ((x.ui64) & 0xFFFFFFFF)

/**
 * Make a 64-bit integer from two 32-bit integers.
 *
 * @param top The top 32 bits of the integer.
 * @param bot The bottom 32 bits of the integer.
 * @return The 64-bit integer.
 */
#define GTA_UC_MAKE_TYPE_OFFSET_PAIR(top, bot) GCU_TYPE64_UI64(((uint64_t)(top) << 32) | (uint64_t)(bot))


/**
 * Basic Unicode String object.
 *
 * The vectors should be considered as internal to the object, and should not
 * be accessed directly.  Instead, use the functions provided.  They are
 * included here for efficiency in locality of reference when accessing the
 * string.
 *
 * A Unicode String object is a UTF-8 encoded string that is grapheme-aware.
 * It also tracks the type of the string in parts, so that the string can be
 * encoded correctly, even if it has been concatenated with other strings.  The
 * concatenation and substring functions will ensure that the type of the
 * string is maintained correctly as the string is manipulated.
 */
struct GTA_Unicode_String {
  const char * buffer;             ///< The string buffer.
  size_t grapheme_length;          ///< Length of the string in graphemes.
  size_t byte_length;              ///< Length of the string in bytes.  Does
                                   ///<   not include the null terminator.
  GCU_Vector32 * grapheme_offsets; ///< Mapping of grapheme to the index where
                                   ///<   the grapheme starts.  Will contain
                                   ///<   grapheme_length + 1 entries.
  GCU_Vector64 * string_type;      ///< Tracks the type of string in parts, so
                                   ///<   that the string can be encoded
                                   ///<   correctly, even if it has been
                                   ///<   concatenated with other strings.
                                   ///<   The type is the upper 32 bits of the
                                   ///<   64-bit integer, and the offset is the
                                   ///<   lower 32 bits.  The offset is the
                                   ///<   grapheme offset.
};

/**
 * Simple structure for returning a Unicode string that has been rendered.
 */
struct GTA_Unicode_Rendered_String {
  char * buffer;                   ///< The rendered string buffer.
  size_t length;                   ///< Length of the rendered string in bytes.
};

/**
 * The type of the substring.
 *
 * This allows a string to be manipulated in its original form, even if it will
 * later be encoded.  For example, a string may be "Untrusted", which will
 * result in a percent or html encoding being applied (depending on the
 * context), but it may be concatenated with a "Trusted" string, which will not
 * be encoded.  The resulting string will then "tag" the "Trusted" and
 * "Untrusted" parts, so that they can be encoded correctly, and yet the string
 * may be manipulated as a single string for further concatenation or
 * substring.
 */
typedef enum {
  GTA_UNICODE_STRING_TYPE_TRUSTED,   ///< String is from a trusted source.
  GTA_UNICODE_STRING_TYPE_HTML,      ///< String is to be HTML encoded. Only
                                     ///<   characters that need to be encoded
                                     ///<   in order to prevent injection
                                     ///<   attacks will actually be encoded.
  GTA_UNICODE_STRING_TYPE_HTML_ATTRIBUTE, ///< String is to be HTML encoded,
                                     ///<   with all more aggressive encoding
                                     ///<   applied. This is used for attribute
                                     ///<   values.
  GTA_UNICODE_STRING_TYPE_PERCENT,   ///< String is to be percent encoded.
                                     ///<   This is used for query strings.
  GTA_UNICODE_STRING_TYPE_JAVASCRIPT,///< String is to be JavaScript encoded.
                                     ///<   This is used for inline scripts.
} GTA_String_Type;

/**
 * Construct a new Unicode String object and copy the source string buffer.
 *
 * @param source The source string.  It is not adopted.
 * @param length The length of the source string in bytes (not including the
 *   null terminator).
 * @param type The type of string being created.
 * @return A pointer to the Unicode String object, or NULL if there was an error.
 */
GTA_NO_DISCARD GTA_Unicode_String * gta_unicode_string_create(const char * source, size_t length, GTA_String_Type type);

/**
 * Construct a new Unicode String object and adopt ownership of the source
 * string buffer.
 *
 * @param source The source string.  It is adopted.
 * @param length The length of the source string in bytes (not including the
 *   null terminator).
 * @param type The type of string being created.
 * @return A pointer to the Unicode String object, or NULL if there was an error.
 */
GTA_NO_DISCARD GTA_Unicode_String * gta_unicode_string_create_and_adopt(const char * source, size_t length, GTA_String_Type type);

/**
 * Destroy a Unicode String object.
 * @param string The string to destroy.
 */
void gta_unicode_string_destroy(GTA_Unicode_String * string);

/**
 * Concatenate two Unicode Strings.
 *
 * The caller is responsible for handling the memory of the input strings.
 *
 * @param string1 The first string.
 * @param string2 The second string.
 * @return A pointer to the new string, or NULL if there was an error.
 */
GTA_NO_DISCARD GTA_Unicode_String * gta_unicode_string_concat(const GTA_Unicode_String * string1, const GTA_Unicode_String * string2);

/**
 * Get the substring of a Unicode String.
 *
 * The caller is responsible for handling the memory of the input string.
 *
 * @param string The string.
 * @param grapheme_start The start position of the substring in graphemes.
 * @param grapheme_count The number of graphemes in the substring.
 * @return A pointer to the new string, or NULL if there was an error.
 */
GTA_NO_DISCARD GTA_Unicode_String * gta_unicode_string_substring(const GTA_Unicode_String * string, size_t grapheme_start, size_t grapheme_count);

/**
 * Render a string according to the types of the string parts.
 *
 * TRUSTED strings will be rendered as-is.
 * HTML strings will be HTML escaped.
 * PERCENT strings will be percent encoded.
 *
 * The caller is responsible for the memory of the returned string.  It should
 * be freed with gcu_free().
 *
 * @param string The string to render.
 * @return The object containing the encoded string.
 */
GTA_NO_DISCARD GTA_Unicode_Rendered_String gta_unicode_string_render(const GTA_Unicode_String * string);

/**
 * Encode a string according to GTA_UNICODE_STRING_TYPE_HTML.
 *
 * The caller is responsible for the memory of the returned string.  It should
 * be freed with gcu_free().
 *
 * @param source The source string.
 * @param length The length of the source string in bytes (not including the
 *  null terminator).
 * @return The object containing the encoded string.
 */
GTA_NO_DISCARD GTA_Unicode_Rendered_String gta_unicode_string_html_encode(const char * source, size_t length);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_UNICODESTRING_H