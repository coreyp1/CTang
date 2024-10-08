
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValue.h>
#include <tang/computedValue/computedValueBoolean.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueFloat.h>
#include <tang/computedValue/computedValueInteger.h>
#include <tang/computedValue/computedValueString.h>
#include <tang/program/executionContext.h>


/**
 * Return the length of the string in graphemes.
 *
 * @param self The string object.
 * @param context The execution context.
 * @return The length of the string in graphemes.
 */
static GTA_Computed_Value * GTA_CALL string_length(GTA_Computed_Value * self, GTA_Execution_Context * context);


/**
 * Return the length of the string in bytes.
 *
 * @param self The string object.
 * @param context The execution context.
 * @return The length of the string in bytes.
 */
static GTA_Computed_Value * GTA_CALL string_byte_length(GTA_Computed_Value * self, GTA_Execution_Context * context);


/**
 * Apply all encodings to the string.
 *
 * This allows a string to be double-encoded, which is useful in some
 * situations.
 *
 * @param self The string object.
 * @param context The execution context.
 * @return The encoded string computed value.
 */
static GTA_Computed_Value * GTA_CALL string_render(GTA_Computed_Value * self, GTA_Execution_Context * context);


/**
 * Helper function to change the type of a string.
 *
 * @param self The string object.
 * @param context The execution context.
 * @return The new string computed value.
 */
static GTA_Computed_Value * GTA_CALL string_change_type(GTA_Computed_Value * self, GTA_Execution_Context * context, GTA_String_Type new_type);


/**
 * Treat the entire string as raw text, without any encodings applied.
 *
 * @param self The string object.
 * @param context The execution context.
 * @return The raw string computed value.
 */
static GTA_Computed_Value * GTA_CALL string_raw(GTA_Computed_Value * self, GTA_Execution_Context * context);


/**
 * Treat the entire string as HTML text, with appropriate HTML entities
 * encoded to prevent HTML injection.
 *
 * @param self The string object.
 * @param context The execution context.
 * @return The HTML string computed value.
 */
static GTA_Computed_Value * GTA_CALL string_html(GTA_Computed_Value * self, GTA_Execution_Context * context);


/**
 * Treat the entire string as an HTML attribute value, with appropriate HTML
 * entities encoded to prevent HTML injection.
 *
 * @param self The string object.
 * @param context The execution context.
 * @return The HTML attribute string computed value.
 */
static GTA_Computed_Value * GTA_CALL string_html_attribute(GTA_Computed_Value * self, GTA_Execution_Context * context);


/**
 * Treat the entire string as a percent-encoded string, with appropriate
 * characters encoded to prevent URL injection.
 *
 * @param self The string object.
 * @param context The execution context.
 * @return The percent-encoded string computed value.
 */
static GTA_Computed_Value * GTA_CALL string_percent(GTA_Computed_Value * self, GTA_Execution_Context * context);


/**
 * Treat the entire string as a JavaScript string, with appropriate characters
 * encoded to prevent JavaScript injection.
 *
 * @param self The string object.
 * @param context The execution context.
 * @return The JavaScript string computed value.
 */
static GTA_Computed_Value * GTA_CALL string_javascript(GTA_Computed_Value * self, GTA_Execution_Context * context);


/**
 * The attributes for the GTA_Computed_Value_String class.
 */
static GTA_Computed_Value_Attribute_Pair attributes[] = {
  {"length", string_length},
  {"byte_length", string_byte_length},
  {"render", string_render},
  {"raw", string_raw},
  {"html", string_html},
  {"html_attribute", string_html_attribute},
  {"percent", string_percent},
  {"javascript", string_javascript},
};


GTA_Computed_Value_VTable gta_computed_value_string_vtable = {
  .name = "String",
  .destroy = gta_computed_value_string_destroy,
  .destroy_in_place = gta_computed_value_string_destroy_in_place,
  .deep_copy = gta_computed_value_string_deep_copy,
  .to_string = gta_computed_value_string_to_string,
  .print = gta_computed_value_string_print,
  .assign_index = gta_computed_value_assign_index_not_supported,
  .add = gta_computed_value_add_not_implemented,
  .subtract = gta_computed_value_subtract_not_supported,
  .multiply = gta_computed_value_multiply_not_supported,
  .divide = gta_computed_value_divide_not_supported,
  .modulo = gta_computed_value_modulo_not_supported,
  .negative = gta_computed_value_negative_not_supported,
  .less_than = gta_computed_value_less_than_not_implemented,
  .less_than_equal = gta_computed_value_less_than_equal_not_implemented,
  .greater_than = gta_computed_value_greater_than_not_implemented,
  .greater_than_equal = gta_computed_value_greater_than_equal_not_implemented,
  .equal = gta_computed_value_equal_not_implemented,
  .not_equal = gta_computed_value_not_equal_not_implemented,
  .period = gta_computed_value_generic_period,
  .index = gta_computed_value_string_index,
  .slice = gta_computed_value_string_slice,
  .iterator_get = gta_computed_value_iterator_get_not_implemented,
  .iterator_next = gta_computed_value_iterator_next_not_implemented,
  .cast = gta_computed_value_string_cast,
  .call = gta_computed_value_call_not_supported,
  .attributes = attributes,
  .attributes_count = 0,
};


/*
 * Setup the proper attribute count, which cannot be done at compile time.
 */
GTA_INIT_FUNCTION(setup) {
  gta_computed_value_string_vtable.attributes_count = sizeof(attributes) / sizeof(GTA_Computed_Value_Attribute_Pair);
}


/*
 * The following code defines a singleton for an empty string.
 */

// empty_singleton->value->string_type->data
GCU_Type64_Union gta_computed_value_string_empty_string_type_data[] = {
  {.ui64 = GTA_UNICODE_STRING_TYPE_TRUSTED},
};

// empty_singleton->value->string_type
GCU_Vector64 gta_unicode_string_empty_string_type_singleton = {
  .data = (GCU_Type64_Union *)&gta_computed_value_string_empty_string_type_data,
  .count = 1,
  .capacity = 1,
};

// empty_singleton->value->grapheme_offsets->data
GCU_Type32_Union gta_computed_value_string_empty_grapheme_offsets_data[] = {
  {.ui32 = 0},
};

// empty_singleton->value->grapheme_offsets
GCU_Vector32 gta_unicode_string_empty_grapheme_offsets_singleton = {
  .data = (GCU_Type32_Union *)&gta_computed_value_string_empty_grapheme_offsets_data,
  .count = 1,
  .capacity = 1,
};

// empty_singleton->value
GTA_Unicode_String gta_unicode_string_empty_singleton = {
  .buffer = "",
  .byte_length = 0,
  .grapheme_length = 0,
  .grapheme_offsets = &gta_unicode_string_empty_grapheme_offsets_singleton,
  .string_type = &gta_unicode_string_empty_string_type_singleton,
};

// empty_singleton
GTA_Computed_Value_String gta_computed_value_string_empty_singleton = {
  .base = {
    .vtable = &gta_computed_value_string_vtable,
    .context = NULL,
    .is_true = false,
    .is_error = false,
    .is_temporary = false,
    .requires_deep_copy = false,
    .is_singleton = true,
    .is_a_reference = false,
  },
  .value = &gta_unicode_string_empty_singleton,
  .is_owned = false,
};

GTA_Computed_Value * gta_computed_value_string_empty = (GTA_Computed_Value *)&gta_computed_value_string_empty_singleton;

/*
 * End of the empty string singleton code.
 */


GTA_Computed_Value_String * GTA_CALL gta_computed_value_string_create(GTA_Unicode_String * value, bool adopt, GTA_Execution_Context * context) {
  GTA_Computed_Value_String * self = gcu_malloc(sizeof(GTA_Computed_Value_String));
  if (!self) {
    return 0;
  }
  if (context) {
    // Attempt to add the pointer to the context's garbage collection list.
    if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
      gcu_free(self);
      return NULL;
    }
  }
  gta_computed_value_string_create_in_place(self, value, adopt, context);
  return self;
}


bool GTA_CALL gta_computed_value_string_create_in_place(GTA_Computed_Value_String * self, GTA_Unicode_String * value, bool adopt, GTA_Execution_Context * context) {
  assert(self);
  assert(value);
  *self = (GTA_Computed_Value_String) {
    .base = {
      .vtable = &gta_computed_value_string_vtable,
      .context = context,
      .is_true = value->byte_length > 0,
      .is_error = false,
      .is_temporary = false,
      .requires_deep_copy = true,
      .is_singleton = false,
      .is_a_reference = false,
    },
    .value = value,
    .is_owned = adopt,
  };
  return true;
}


void GTA_CALL gta_computed_value_string_destroy(GTA_Computed_Value * self) {
  if (!self) {
    return;
  }
  gta_computed_value_string_destroy_in_place(self);
  gcu_free(self);
}


void GTA_CALL gta_computed_value_string_destroy_in_place(GTA_Computed_Value * self) {
  if (!self) {
    return;
  }
  assert(GTA_COMPUTED_VALUE_IS_STRING(self));
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *) self;
  if (string->is_owned) {
    gta_unicode_string_destroy(string->value);
  }
}


GTA_Computed_Value * GTA_CALL gta_computed_value_string_deep_copy(GTA_Computed_Value * value, GTA_Execution_Context * context) {
  assert(value);
  assert(GTA_COMPUTED_VALUE_IS_STRING(value));
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *) value;

  assert(string->value);
  GTA_Unicode_String * unicodeString = gta_unicode_string_substring(string->value, 0, string->value->grapheme_length);
  if (!unicodeString) {
    return 0;
  }
  return (GTA_Computed_Value *)gta_computed_value_string_create(unicodeString, true, context);
}


char * GTA_CALL gta_computed_value_string_to_string(GTA_Computed_Value * value) {
  assert(value);
  assert(GTA_COMPUTED_VALUE_IS_STRING(value));
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *) value;

  assert(string->value);
  GTA_Unicode_String * source = string->value;
  char * str = gcu_malloc(source->byte_length + 1);
  if (!str) {
    return 0;
  }
  assert(source->buffer);
  memcpy(str, source->buffer, source->byte_length + 1);
  return str;
}


GTA_Unicode_String * GTA_CALL gta_computed_value_string_print(GTA_Computed_Value * self, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_STRING(self));
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *)self;

  assert(string->value);
  return gta_unicode_string_substring(string->value, 0, string->value->grapheme_length);
}


GTA_Computed_Value * GTA_CALL gta_computed_value_string_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_STRING(self));
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *) self;

  assert(type);
  assert(string->value);
  assert(string->value->buffer);
  if (type == &gta_computed_value_string_vtable) {
    return self;
  }
  if (type == &gta_computed_value_boolean_vtable) {
    return string->value->byte_length > 0 ? gta_computed_value_boolean_true : gta_computed_value_boolean_false;
  }
  if (type == &gta_computed_value_float_vtable) {
    return (GTA_Computed_Value *)gta_computed_value_float_create(atof(string->value->buffer), context);
  }
  if (type == &gta_computed_value_integer_vtable) {
    return (GTA_Computed_Value *)gta_computed_value_integer_create(atoll(string->value->buffer), context);
  }
  return (GTA_Computed_Value *)gta_computed_value_error_not_supported;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_string_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_STRING(self));
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *) self;

  if (!GTA_COMPUTED_VALUE_IS_INTEGER(index)) {
    return gta_computed_value_error_invalid_index;
  }
  GTA_Computed_Value_Integer * integer = (GTA_Computed_Value_Integer *)index;

  assert(string->value);
  GTA_Integer normalized_index = integer->value >= 0
    ? integer->value
    : (GTA_Integer)string->value->grapheme_length + integer->value;

  if (normalized_index < 0 || normalized_index >= (GTA_Integer)string->value->grapheme_length) {
    return gta_computed_value_string_empty;
  }

  GTA_Unicode_String * unicode_substring = gta_unicode_string_substring(string->value, normalized_index, 1);
  if (!unicode_substring) {
    return (GTA_Computed_Value *)gta_computed_value_error_out_of_memory;
  }
  GTA_Computed_Value * result = (GTA_Computed_Value *)gta_computed_value_string_create(unicode_substring, true, context);

  if (!result) {
    gta_unicode_string_destroy(unicode_substring);
    return (GTA_Computed_Value *)gta_computed_value_error_out_of_memory;
  }

  return result;
}


// Helper function to correct start and end values of a slice.
static GTA_Integer correct_bounds(GTA_Integer value, GTA_Integer boundary, GTA_Integer step) {
  GTA_Integer interval = (boundary - value) / step;
  GTA_Integer roundup = (interval * step) == (boundary - value) ? 0 : 1;
  return value + ((interval + roundup) * step);
}


GTA_Computed_Value * GTA_CALL gta_computed_value_string_slice(GTA_Computed_Value * self, GTA_Computed_Value * start, GTA_Computed_Value * end, GTA_Computed_Value * step, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_STRING(self));
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *) self;

  assert(string->value);
  GTA_Integer grapheme_length = (GTA_Integer)string->value->grapheme_length;

  // First, validate the step value.  If null, it will default to 1.
  if (!(GTA_COMPUTED_VALUE_IS_INTEGER(step) || GTA_COMPUTED_VALUE_IS_NULL(step))) {
    return gta_computed_value_error_invalid_index;
  }
  GTA_Integer step_value = GTA_COMPUTED_VALUE_IS_NULL(step)
    ? 1
    : ((GTA_Computed_Value_Integer *)step)->value;
  if (step_value == 0) {
    return gta_computed_value_error_invalid_index;
  }

  // Convert the start value to an integer.
  if (!(GTA_COMPUTED_VALUE_IS_INTEGER(start) || GTA_COMPUTED_VALUE_IS_NULL(start))) {
    return gta_computed_value_error_invalid_index;
  }
  GTA_Integer start_value = GTA_COMPUTED_VALUE_IS_NULL(start)
    ? step_value > 0
      ? 0
      : grapheme_length - 1
    : ((GTA_Computed_Value_Integer *)start)->value >= 0
      ? ((GTA_Computed_Value_Integer *)start)->value
      : ((GTA_Computed_Value_Integer *)start)->value + grapheme_length;

  // Convert the end value to an integer.
  if (!(GTA_COMPUTED_VALUE_IS_INTEGER(end) || GTA_COMPUTED_VALUE_IS_NULL(end))) {
    return gta_computed_value_error_invalid_index;
  }
  GTA_Integer end_value = GTA_COMPUTED_VALUE_IS_NULL(end)
    ? step_value > 0
      ? grapheme_length
      : -1
    : ((GTA_Computed_Value_Integer *)end)->value >= 0
      ? ((GTA_Computed_Value_Integer *)end)->value
      : ((GTA_Computed_Value_Integer *)end)->value + grapheme_length;

  if (step_value > 0) {
    // If the step value is positive, then the start value should be the first
    // eligible value that is greater than or equal to 0.
    if (start_value < 0) {
      start_value = correct_bounds(start_value, 0, step_value);
    }
    // It is possible that the start_value is outside of the bounds of the
    // string (e.g., "abc"[-4::10]).
    // It is possible that end_value is less than start_value.  If so, then do
    // nothing so that the first sanity check will catch it.  Otherwise, if
    // end_value is greater than the end of the string, then set it to the
    // first eligible value that is after the end of the string.
    if ((end_value > start_value) && (end_value > grapheme_length)) {
      end_value = correct_bounds(start_value, grapheme_length, step_value);
    }
  }
  else {
    // If the step value is negative, then the start value should be the first
    // eligible value that is less than or equal to the end of the string.
    if (start_value > grapheme_length - 1) {
      start_value = correct_bounds(start_value, grapheme_length - 1, step_value);
    }
    // It is possible that the start_value is outside of the bounds of the
    // string (e.g., "abc"[4::-10]).
    // It is possible that end_value is greater than start_value.  If so, then
    // do nothing so that the first sanity check will catch it.  Otherwise, if
    // end_value is less than the start of the string, then set it to the
    // first eligible value that is before the start of the string.
    if ((end_value < start_value) && (end_value < -1)) {
      end_value = correct_bounds(start_value, -1, step_value);
    }
  }

  // First sanity check.  If the start, end, and step values do not intersect,
  // then the result is an empty string.
  if ((step_value > 0 && start_value >= end_value) || (step_value < 0 && start_value <= end_value)) {
    return gta_computed_value_string_empty;
  }

  // If the step value is 1, then we can just return the substring.
  if (step_value == 1) {
    GTA_Unicode_String * substring = gta_unicode_string_substring(string->value, start_value, end_value - start_value);
    if (!substring) {
      return (GTA_Computed_Value *)gta_computed_value_error_out_of_memory;
    }
    GTA_Computed_Value * result = (GTA_Computed_Value *)gta_computed_value_string_create(substring, true, context);

    if (!result) {
      gta_unicode_string_destroy(substring);
      return (GTA_Computed_Value *)gta_computed_value_error_out_of_memory;
    }

    return result;
  }

  // If the step value is not 1, then we need to iterate through the string.
  // TODO: Make this more efficient!
  GTA_Unicode_String * new_string = &gta_unicode_string_empty_singleton;
  for (GTA_Integer i = start_value; step_value > 0 ? i < end_value : i > end_value; i += step_value) {
    GTA_Unicode_String * substring = gta_unicode_string_substring(string->value, i, 1);
    if (!substring) {
      if (new_string != &gta_unicode_string_empty_singleton) {
        gta_unicode_string_destroy(new_string);
      }
      return (GTA_Computed_Value *)gta_computed_value_error_out_of_memory;
    }
    GTA_Unicode_String * temp = gta_unicode_string_concat(new_string, substring);
    if (new_string != &gta_unicode_string_empty_singleton) {
      gta_unicode_string_destroy(new_string);
    }
    gta_unicode_string_destroy(substring);
    if (!temp) {
      return (GTA_Computed_Value *)gta_computed_value_error_out_of_memory;
    }
    new_string = temp;
  }

  GTA_Computed_Value * result = (GTA_Computed_Value *)gta_computed_value_string_create(new_string, new_string == &gta_unicode_string_empty_singleton ? false : true, context);

  if (!result) {
    gta_unicode_string_destroy(new_string);
    return (GTA_Computed_Value *)gta_computed_value_error_out_of_memory;
  }

  return result;
}


GTA_Computed_Value * GTA_CALL string_length(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_STRING(self));
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *)self;
  return (GTA_Computed_Value *)gta_computed_value_integer_create(string->value->grapheme_length, context);
}


static GTA_Computed_Value * GTA_CALL string_byte_length(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_STRING(self));
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *)self;
  return (GTA_Computed_Value *)gta_computed_value_integer_create(string->value->byte_length, context);
}


static GTA_Computed_Value * GTA_CALL string_render(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_STRING(self));
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *)self;
  GTA_Unicode_Rendered_String rendered_string = gta_unicode_string_render(string->value);
  if (!rendered_string.buffer) {
    goto RENDER_STRING_FAILED;
  }
  GTA_Unicode_String * new_string = gta_unicode_string_create_and_adopt(rendered_string.buffer, rendered_string.length, GTA_UNICODE_STRING_TYPE_TRUSTED);
  if (!new_string) {
    goto NEW_STRING_CREATE_FAILED;
  }

  GTA_Computed_Value * result = (GTA_Computed_Value *)gta_computed_value_string_create(new_string, true, context);
  if (!result) {
    gta_unicode_string_destroy(new_string);
    goto RENDER_STRING_FAILED;
  }

  return result;

NEW_STRING_CREATE_FAILED:
  gcu_free(rendered_string.buffer);
RENDER_STRING_FAILED:
  return (GTA_Computed_Value *)gta_computed_value_error_out_of_memory;
}


static GTA_Computed_Value * GTA_CALL string_change_type(GTA_Computed_Value * self, GTA_Execution_Context * context, GTA_String_Type new_type) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_STRING(self));
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *)self;

  if (self->is_temporary) {
    // If the string is temporary, then we can just adjust the type.
    assert(string->value);
    assert(string->value->string_type);
    string->value->string_type->data[0] = GTA_UC_MAKE_TYPE_OFFSET_PAIR(new_type, 0);
    string->value->string_type->count = 1;
    return self;
  }

  // If the string is not temporary, then we need to create a new string.
  GTA_Unicode_String * new_string = gta_unicode_string_create(string->value->buffer, string->value->byte_length, new_type);
  if (!new_string) {
    return (GTA_Computed_Value *)gta_computed_value_error_out_of_memory;
  }
  GTA_Computed_Value * result = (GTA_Computed_Value *)gta_computed_value_string_create(new_string, true, context);
  if (!result) {
    gta_unicode_string_destroy(new_string);
    return (GTA_Computed_Value *)gta_computed_value_error_out_of_memory;
  }
  return result;
}


static GTA_Computed_Value * GTA_CALL string_raw(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  return string_change_type(self, context, GTA_UNICODE_STRING_TYPE_TRUSTED);
}


static GTA_Computed_Value * GTA_CALL string_html(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  return string_change_type(self, context, GTA_UNICODE_STRING_TYPE_HTML);
}


static GTA_Computed_Value * GTA_CALL string_html_attribute(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  return string_change_type(self, context, GTA_UNICODE_STRING_TYPE_HTML_ATTRIBUTE);
}


static GTA_Computed_Value * GTA_CALL string_percent(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  return string_change_type(self, context, GTA_UNICODE_STRING_TYPE_PERCENT);
}


static GTA_Computed_Value * GTA_CALL string_javascript(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  return string_change_type(self, context, GTA_UNICODE_STRING_TYPE_JAVASCRIPT);
}
