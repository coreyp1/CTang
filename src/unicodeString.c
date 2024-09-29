
#include <assert.h>
#include <string.h>
#include <cutil/memory.h>
#include <unicode/uconfig.h>
#include <unicode/ustring.h>
#include <unicode/ubrk.h>
#include <tang/unicodeString.h>

#include <stdio.h>

/**
 * This pair will be punted into a uint64_t, for use in the string type vector.
 */
struct {
  uint32_t type;
  uint32_t grapheme_offset;
} GCU_Type_Offset_Pair;


bool gcu_unicode_string_get_grapheme_offsets(GCU_Vector32 * grapheme_offsets, const char * buffer, size_t length) {
  assert(grapheme_offsets);
  assert(buffer);

  bool success = false;

  // Early exit for empty strings.
  if (!length) {
    if (!gcu_vector32_append(grapheme_offsets, GCU_TYPE32_UI32(0))) {
      goto EARLY_ERROR;
    }
    success = true;
    goto EARLY_SUCCESS;
  }

  // Worst case: string is standard ASCII.
  if (!gcu_vector32_reserve(grapheme_offsets, length + 1)) {
    goto EARLY_ERROR;
  }

  // Create an ICU Character Break Iterator to identify the graphemes.
  UErrorCode err = U_ZERO_ERROR;
  UBreakIterator * iter = ubrk_open(UBRK_CHARACTER, NULL, NULL, 0, &err);
  if (!U_SUCCESS(err)) {
    goto EARLY_ERROR;
  }

  // Add the first offset.
  // This is always 0, even for an empty string.
  if (!gcu_vector32_append(grapheme_offsets, GCU_TYPE32_UI32(0))) {
    goto OFFSET_ADD_FAILED;
  }

  // Set the text to iterate through.
  // Convert buffer (which is a char *) to a UChar * using ICU conversion
  // functions.
  // This is safe because the buffer is guaranteed to be UTF-8.
  int32_t uLength = 0;
  UChar * uBuffer = NULL;
  // Pre-flight the conversion to determine the length of the resulting string.
  u_strFromUTF8(NULL, 0, &uLength, buffer, length, &err);
  if (err == U_BUFFER_OVERFLOW_ERROR) {
    err = U_ZERO_ERROR;
    uBuffer = gcu_malloc(sizeof(UChar) * (uLength + 1));
    if (uBuffer == NULL) {
      goto UBUFFER_CREATE_FAILED;
    }
    u_strFromUTF8(uBuffer, uLength + 1, NULL, buffer, length, &err);
  }
  if (!U_SUCCESS(err)) {
    goto STRFROMUTF8_FAILED;
  }

  ubrk_setText(iter, uBuffer, uLength, &err);
  if (!U_SUCCESS(err)) {
    goto STRFROMUTF8_FAILED;
  }

  // Print out each uBuffer character.
  // for (int32_t i = 0; i < uLength; ++i) {
  //   printf("%d: %d\n", i, uBuffer[i]);
  // }

  // Find each grapheme offset one by one.  When we find the next offset, we
  // can then determine the length of the previous grapheme in UTF-8 so that
  // we can calculate the offset of the currently found grapheme.
  int32_t previous_index_in_UTF16 = 0;
  // NOTE: Workaround for a bug? in the ICU library.
  // In the u_strToUTF8() call below, we *should* be able to leave the
  // destination buffer as NULL and the length as 0 to pre-flight the
  // conversion, but that causes bad results.  Instead, we will allocate a
  // buffer and then throw it away.
  // We know that the original string was valid UTF-8, so the maximum length
  // required for any grapheme is the length of the original string.
  char * dummyBuffer = gcu_malloc(length + 1);
  if (dummyBuffer == NULL) {
    goto DUMMY_BUFFER_CREATE_FAILED;
  }
  while (ubrk_next(iter) != UBRK_DONE) {
    int32_t current_index_in_UTF16 = ubrk_current(iter);

    // Find out how many bytes the grapheme is as UTF-8.
    int32_t grapheme_length_in_UTF8;
    u_strToUTF8(dummyBuffer, length + 1, &grapheme_length_in_UTF8, &uBuffer[previous_index_in_UTF16], current_index_in_UTF16 - previous_index_in_UTF16, &err);
    // We do not check for an error because we already know that this is valid
    // UTF-8, converted to UTF-16, and back to UTF-8, which is guaranteed to
    // be valid and lossless.

    // Add the offset to the vector.
    if (!gcu_vector32_append(grapheme_offsets, GCU_TYPE32_UI32(grapheme_length_in_UTF8 + grapheme_offsets->data[grapheme_offsets->count - 1].ui32))) {
      goto GRAPHEME_OFFSET_ADD_FAILED;
    }
    previous_index_in_UTF16 = current_index_in_UTF16;
  }

  success = true;
  // Fall-through for cleanup

GRAPHEME_OFFSET_ADD_FAILED:
  gcu_free(dummyBuffer);
DUMMY_BUFFER_CREATE_FAILED:
STRFROMUTF8_FAILED:
  if (uBuffer) {
    // Note: If the buffer was an empty string, then uBuffer will be NULL.
    gcu_free(uBuffer);
  }
OFFSET_ADD_FAILED:
UBUFFER_CREATE_FAILED:
  ubrk_close(iter);
EARLY_ERROR:
EARLY_SUCCESS:
  return success;
}


GTA_Unicode_String * gta_unicode_string_create(const char * source, size_t length, GTA_String_Type type) {
  assert(source);

  char * buffer = gcu_malloc(length + 1);
  if (!buffer) {
    return NULL;
  }
  if (length) {
    memcpy(buffer, source, length);
  }
  buffer[length] = '\0';
  GTA_Unicode_String * string = gta_unicode_string_create_and_adopt(buffer, length, type);
  if (!string) {
    gcu_free(buffer);
  }
  return string;
}


GTA_Unicode_String * gta_unicode_string_create_and_adopt(const char * source, size_t length, GTA_String_Type type) {
  assert(source);

  // Allocate space for the string.
  GTA_Unicode_String * string = gcu_calloc(sizeof(GTA_Unicode_String), 1);
  if (string == NULL) {
    return NULL;
  }

  // Adopt the buffer.
  string->buffer = source;
  string->byte_length = length;

  // Create the grapheme offsets.
  // For ease of use, we will add an extra offset at the end of the string
  // that points to the end of the string.
  // If the string only contains ASCII characters, then the grapheme offsets
  // will be the same as the byte offsets and will require (length + 1)
  // entries as a worst case.  If the string contains non-ASCII characters,
  // then the grapheme offsets will be different from the byte offsets and
  // will require fewer than (length + 1) entries, so we will allocate the
  // worst-case number of entries to ensure that no allocation failures can
  // happen later.
  string->grapheme_offsets = gcu_vector32_create(length + 1);
  if (string->grapheme_offsets == NULL) {
    gcu_free(string);
    return NULL;
  }
  if (!gcu_unicode_string_get_grapheme_offsets(string->grapheme_offsets, source, length)) {
    gcu_vector32_destroy(string->grapheme_offsets);
    gcu_free(string);
    return NULL;
  }
  string->grapheme_length = gcu_vector32_count(string->grapheme_offsets) - 1;

  // Create the string type vector.
  // It will only contain one type, so we will allocate one entry.
  string->string_type = gcu_vector64_create(1);
  if (string->string_type == NULL) {
    gcu_vector32_destroy(string->grapheme_offsets);
    gcu_free(string);
    return NULL;
  }
  gcu_vector64_append(string->string_type, GTA_UC_MAKE_TYPE_OFFSET_PAIR(type, 0));

  return string;
}


void gta_unicode_string_destroy(GTA_Unicode_String * string) {
  assert(string);

  gcu_vector32_destroy(string->grapheme_offsets);
  gcu_vector64_destroy(string->string_type);
  gcu_free((void *)string->buffer);
  gcu_free(string);
}


GTA_Unicode_String * gta_unicode_string_concat(const GTA_Unicode_String * string1, const GTA_Unicode_String * string2) {
  assert(string1);
  assert(string2);

  // Allocate space for the new string.
  GTA_Unicode_String * newString = gcu_calloc(sizeof(GTA_Unicode_String), 1);
  if (newString == NULL) {
    return NULL;
  }

  // Copy the source strings to the buffer.
  newString->buffer = gcu_malloc(string1->byte_length + string2->byte_length + 1);
  if (newString->buffer == NULL) {
    gcu_free(newString);
    return NULL;
  }
  if (string1->byte_length) {
    memcpy((char *)newString->buffer, string1->buffer, string1->byte_length);
  }
  if (string2->byte_length) {
    memcpy((char *)newString->buffer + string1->byte_length, string2->buffer, string2->byte_length);
  }
  newString->byte_length = string1->byte_length + string2->byte_length;
  ((char *)newString->buffer)[newString->byte_length] = '\0';

  // Create the grapheme offsets.
  // For ease of use, we will add an extra offset at the end of the string
  // that points to the end of the string.
  // If the string only contains ASCII characters, then the grapheme offsets
  // will be the same as the byte offsets and will require (length + 1)
  // entries as a worst case.  If the string contains non-ASCII characters,
  // then the grapheme offsets will be different from the byte offsets and
  // will require fewer than (length + 1) entries, so we will allocate the
  // worst-case number of entries to ensure that no allocation failures can
  // happen later.
  newString->grapheme_offsets = gcu_vector32_create(string1->grapheme_length + string2->grapheme_length + 1);
  if (newString->grapheme_offsets == NULL) {
    gcu_free((void *)newString->buffer);
    gcu_free(newString);
    return NULL;
  }
  // Copy the grapheme offsets from the first string.
  // The space is already reserved.
  memcpy(newString->grapheme_offsets->data, string1->grapheme_offsets->data, string1->grapheme_length * sizeof(GCU_Type32_Union));
  // Copy the grapheme offsets from the second string, adding the byte and
  // grapheme offsets from the first string to the second string.
  // The space is already reserved.
  size_t byte_offset = string1->byte_length;
  size_t grapheme_offset = string1->grapheme_length;
  for (size_t i = 0; i < string2->grapheme_length; ++i) {
    newString->grapheme_offsets->data[i + grapheme_offset] = GCU_TYPE32_UI32(string2->grapheme_offsets->data[i].ui32 + byte_offset);
  }
  // Add the final offset.
  newString->grapheme_offsets->data[string1->grapheme_length + string2->grapheme_length] = GCU_TYPE32_UI32(string1->byte_length + string2->byte_length);
  // Since we reached into the vector and manipulated data directly, we need to
  // manually fix the count.
  newString->grapheme_offsets->count = string1->grapheme_length + string2->grapheme_length + 1;
  // Set the grapheme length.
  newString->grapheme_length = string1->grapheme_length + string2->grapheme_length;

  // Create the string type vector.
  // Because the last type of the first string and the first type of the second
  // string may be the same, we may need to merge them.  We will allocate
  // enough space for the worst case, which is that the last type of the first
  // string and the first type of the second string are different.
  newString->string_type = gcu_vector64_create(string1->string_type->count + string2->string_type->count);
  if (newString->string_type == NULL) {
    gcu_vector32_destroy(newString->grapheme_offsets);
    gcu_free((void *)newString->buffer);
    gcu_free(newString);
    return NULL;
  }
  // If the first string is empty, then skip it.
  size_t types_copied_count = 0;
  bool strings_are_same_type = false;
  if (string1->byte_length != 0) {
    // Copy the string types from the first string.
    // The space is already reserved.
    memcpy(newString->string_type->data, string1->string_type->data, string1->string_type->count * sizeof(GCU_Type64_Union));
    strings_are_same_type = GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(string1->string_type->data[string1->string_type->count - 1]) == GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(string2->string_type->data[0]);
    // Since we reached into the vector and manipulated data directly, we need to
    // manually fix the count.
    newString->string_type->count = string1->string_type->count;
    types_copied_count = string1->string_type->count;
  }
  // Copy the string types from the second string, adding the grapheme count
  // from the first string to the second string's grapheme offsets.
  // The space is already reserved.
  if (string2->byte_length || !string1->byte_length) {
    size_t offset = strings_are_same_type ? 1 : 0;
    for (size_t i = offset; i < string2->string_type->count; ++i) {
      // Was originally:
      //   uint64_t currentUnion = string2->string_type->data[i].ui64;
      //   GTA_UC_MAKE_TYPE_OFFSET_PAIR(GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(currentUnion), GTA_UC_GET_OFFSET_FROM_TYPE_OFFSET_PAIR(currentUnion) + string1->grapheme_length)
      // but, since the union is a integer and the addition is happening to the
      // lower bytes, that simplifies to:
      //   string2->string_type->data[i].ui64 + string1->grapheme_length
      newString->string_type->data[types_copied_count + i - offset].ui64 = string2->string_type->data[i].ui64 + string1->grapheme_length;
    }
    // Since we reached into the vector and manipulated data directly, we need to
    // manually fix the count.
    newString->string_type->count = types_copied_count + string2->string_type->count - (strings_are_same_type ? 1 : 0);
  }

  return newString;
}


GTA_Unicode_String * gta_unicode_string_substring(const GTA_Unicode_String * string, size_t grapheme_start, size_t grapheme_count) {
  assert(string);

  // If grapheme start is beyond the end of the string, then return an empty
  // string.
  if ((grapheme_start >= string->grapheme_length) || (grapheme_count == 0)) {
    return gta_unicode_string_create("", 0, GTA_UNICODE_STRING_TYPE_TRUSTED);
  }

  // Determine the byte offset of the start of the substring.
  size_t byte_start = string->grapheme_offsets->data[grapheme_start].ui32;

  // Determine the byte offset of the end of the substring.
  size_t byte_end = (grapheme_start + grapheme_count >= string->grapheme_length)
    ? string->grapheme_offsets->data[string->grapheme_length].ui32
    : string->grapheme_offsets->data[grapheme_start + grapheme_count].ui32;

  // Determine the actual last grapheme to be copied from the original string.
  size_t end_grapheme = (grapheme_start + grapheme_count >= string->grapheme_length)
    ? string->grapheme_length
    : grapheme_start + grapheme_count;

  // Determine the type of the substring by finding the first index whose range
  // includes the start grapheme.
  GTA_String_Type newStringType = GTA_UNICODE_STRING_TYPE_TRUSTED;
  size_t first_string_type_index_to_include = 0;
  size_t iterator = 0;
  while ((iterator < string->string_type->count) && (GTA_UC_GET_OFFSET_FROM_TYPE_OFFSET_PAIR(string->string_type->data[iterator]) > grapheme_start)) {
    newStringType = GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(string->string_type->data[iterator]);
    first_string_type_index_to_include = iterator;
    ++iterator;
  }
  // Create a new string.
  GTA_Unicode_String * newString = gta_unicode_string_create(string->buffer + byte_start, byte_end - byte_start, newStringType);
  if (newString == NULL) {
    return NULL;
  }

  // Determine the last string type offset that is less than the last grapheme
  // to be copied from the original string.
  size_t last_string_type_index_to_include = first_string_type_index_to_include;
  iterator = first_string_type_index_to_include;
  while ((iterator < string->string_type->count) && (GTA_UC_GET_OFFSET_FROM_TYPE_OFFSET_PAIR(string->string_type->data[iterator]) < end_grapheme)) {
    last_string_type_index_to_include = iterator;
    ++iterator;
  }

  // Add the remaining string types.
  // The first entry is already present.
  // We will allocate enough space from the start, because we know how many
  // entries are required.
  if (!gcu_vector64_reserve(newString->string_type, last_string_type_index_to_include - first_string_type_index_to_include + 1)) {
    gta_unicode_string_destroy(newString);
    return NULL;
  }
  for (size_t i = first_string_type_index_to_include + 1; i <= last_string_type_index_to_include; ++i) {
    // The original string_type already contains the correct type, but the
    // offset is relative to the start of the original string, so we need to
    // subtract the start of the substring.
    gcu_vector64_append(newString->string_type, GCU_TYPE64_I64(string->string_type->data[i].ui64 - grapheme_start));
  }
  return newString;
}


GTA_Unicode_Rendered_String gta_unicode_string_render(const GTA_Unicode_String * string) {
  assert(string);

  if (!string->buffer) {
    return (GTA_Unicode_Rendered_String){
      .buffer = NULL,
      .length = 0
    };
  }

  assert(string->string_type);
  assert(string->string_type->count);

  size_t total_bytes_allocated = string->byte_length + 1;
  char * buffer = gcu_malloc(total_bytes_allocated);
  if (!buffer) {
    goto RENDER_ERROR;
  }
  size_t buffer_length = 0;

  for (size_t i = 0; i < string->string_type->count; ++i) {
    GTA_String_Type type = GTA_UC_GET_TYPE_FROM_TYPE_OFFSET_PAIR(string->string_type->data[i]);
    size_t source_offset = GTA_UC_GET_OFFSET_FROM_TYPE_OFFSET_PAIR(string->string_type->data[i]);
    size_t next_source_offset = (i + 1 < string->string_type->count)
      ? GTA_UC_GET_OFFSET_FROM_TYPE_OFFSET_PAIR(string->string_type->data[i + 1])
      : string->byte_length;
    switch (type) {
      case GTA_UNICODE_STRING_TYPE_TRUSTED: {
        // This is a direct copy.
        size_t bytes_to_copy = next_source_offset - source_offset;
        if (buffer_length + bytes_to_copy + 1 > total_bytes_allocated) {
          // The buffer is not large enough.  Resize it optimistically,
          // assuming that the rest of the string will also be copied directly.
          size_t bytes_remaining_in_source = string->byte_length - source_offset + 1;
          size_t new_allocated_size = buffer_length + bytes_remaining_in_source;
          char * new_buffer = gcu_realloc(buffer, new_allocated_size);
          if (!new_buffer) {
            goto RENDER_ERROR;
          }
          total_bytes_allocated = new_allocated_size;
          buffer = new_buffer;
        }
        memcpy(buffer + buffer_length, string->buffer + source_offset, bytes_to_copy);
        buffer_length += bytes_to_copy;
        break;
      }
      case GTA_UNICODE_STRING_TYPE_HTML:
        // Do nothing.
        break;
      case GTA_UNICODE_STRING_TYPE_PERCENT:
        // Do nothing.
        break;
      default:
        goto RENDER_ERROR;
    }
  }

  buffer[buffer_length] = '\0';

  return (GTA_Unicode_Rendered_String){
    .buffer = buffer,
    .length = buffer_length,
  };

RENDER_ERROR:
  printf("RENDER_ERROR\n");
  if (buffer) {
    gcu_free(buffer);
  }
  return (GTA_Unicode_Rendered_String){
    .buffer = NULL, 
    .length = 0,
  };
}


GTA_NO_DISCARD GTA_Unicode_Rendered_String gta_unicode_string_html_encode(const char * source, size_t length) {
  assert(length ? (bool)source : true);

  if (!length) {
    return (GTA_Unicode_Rendered_String){
      .buffer = NULL,
      .length = 0
    };
  }

  return (GTA_Unicode_Rendered_String){NULL, 0};
}
