
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
  .period = gta_computed_value_period_not_implemented,
  .index = gta_computed_value_string_index,
  .slice = gta_computed_value_slice_not_implemented,
  .iterator_get = gta_computed_value_iterator_get_not_implemented,
  .iterator_next = gta_computed_value_iterator_next_not_implemented,
  .cast = gta_computed_value_string_cast,
  .call = gta_computed_value_call_not_supported,
};

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


GTA_Computed_Value_String * gta_computed_value_string_create(GTA_Unicode_String * value, bool adopt, GTA_Execution_Context * context) {
  GTA_Computed_Value_String * self = gcu_malloc(sizeof(GTA_Computed_Value_String));
  if (!self) {
    return 0;
  }
  // Attempt to add the pointer to the context's garbage collection list.
  if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
    gcu_free(self);
    return NULL;
  }
  gta_computed_value_string_create_in_place(self, value, adopt, context);
  return self;
}


bool gta_computed_value_string_create_in_place(GTA_Computed_Value_String * self, GTA_Unicode_String * value, bool adopt, GTA_Execution_Context * context) {
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


void gta_computed_value_string_destroy(GTA_Computed_Value * self) {
  gta_computed_value_string_destroy_in_place(self);
  gcu_free(self);
}


void gta_computed_value_string_destroy_in_place(GTA_Computed_Value * self) {
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *) self;
  if (string->is_owned) {
    gta_unicode_string_destroy(string->value);
  }
}


GTA_Computed_Value * gta_computed_value_string_deep_copy(GTA_Computed_Value * value, GTA_Execution_Context * context) {
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *) value;
  GTA_Unicode_String * unicodeString = gta_unicode_string_substring(string->value, 0, string->value->grapheme_length);
  if (!unicodeString) {
    return 0;
  }
  return (GTA_Computed_Value *)gta_computed_value_string_create(unicodeString, true, context);
}


char * gta_computed_value_string_to_string(GTA_Computed_Value * value) {
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *) value;
  GTA_Unicode_String * source = string->value;
  char * str = gcu_malloc(source->byte_length + 1);
  if (!str) {
    return 0;
  }
  memcpy(str, source->buffer, source->byte_length + 1);
  return str;
}


GTA_Unicode_String * gta_computed_value_string_print(GTA_Computed_Value * self, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *)self;
  return gta_unicode_string_substring(string->value, 0, string->value->grapheme_length);
}


GTA_Computed_Value * gta_computed_value_string_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context) {
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *) self;
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
  if (!GTA_COMPUTED_VALUE_IS_INTEGER(index)) {
    return gta_computed_value_error_invalid_index;
  }

  GTA_Computed_Value_Integer * integer = (GTA_Computed_Value_Integer *)index;
  GTA_Computed_Value_String * string = (GTA_Computed_Value_String *) self;

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
