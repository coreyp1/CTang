
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValueBoolean.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueInteger.h>
#include <tang/computedValue/computedValueFloat.h>
#include <tang/computedValue/computedValueString.h>

GTA_Computed_Value_VTable gta_computed_value_boolean_vtable = {
  .name = "Boolean",
  .destroy = gta_computed_value_boolean_destroy,
  .destroy_in_place = gta_computed_value_boolean_destroy,
  .deep_copy = gta_computed_value_boolean_deep_copy,
  .to_string = gta_computed_value_boolean_to_string,
  .print = gta_computed_value_print_not_supported,
  .assign_index = gta_computed_value_assign_index_not_implemented,
  .add = gta_computed_value_add_not_supported,
  .subtract = gta_computed_value_subtract_not_supported,
  .multiply = gta_computed_value_multiply_not_supported,
  .divide = gta_computed_value_divide_not_supported,
  .modulo = gta_computed_value_modulo_not_supported,
  .negative = gta_computed_value_negative_not_supported,
  .less_than = gta_computed_value_less_than_equal_not_supported,
  .less_than_equal = gta_computed_value_less_than_equal_not_supported,
  .greater_than = gta_computed_value_less_than_equal_not_supported,
  .greater_than_equal = gta_computed_value_greater_than_equal_not_supported,
  .equal = gta_computed_value_equal_not_implemented,
  .not_equal = gta_computed_value_not_equal_not_implemented,
  .period = gta_computed_value_generic_period,
  .index = gta_computed_value_index_not_supported,
  .slice = gta_computed_value_slice_not_supported,
  .iterator_get = gta_computed_value_iterator_get_not_supported,
  .iterator_next = gta_computed_value_iterator_next_not_supported,
  .cast = gta_computed_value_boolean_cast,
  .call = gta_computed_value_call_not_supported,
  .attributes = NULL,
  .attributes_count = 0,
};


static GTA_Computed_Value_Boolean gta_computed_value_boolean_true_singleton = {
  .base = {
    .vtable = &gta_computed_value_boolean_vtable,
    .context = 0,
    .is_true = true,
    .is_error = false,
    .is_temporary = false,
    .requires_deep_copy = false,
    .is_singleton = true,
    .is_a_reference = false,
  },
  .value = true,
};


static GTA_Computed_Value_Boolean gta_computed_value_boolean_false_singleton = {
  .base = {
    .vtable = &gta_computed_value_boolean_vtable,
    .context = 0,
    .is_true = false,
    .is_error = false,
    .is_temporary = false,
    .requires_deep_copy = false,
    .is_singleton = true,
    .is_a_reference = false,
  },
  .value = false,
};


GTA_Computed_Value * gta_computed_value_boolean_true = (GTA_Computed_Value *)&gta_computed_value_boolean_true_singleton;
GTA_Computed_Value * gta_computed_value_boolean_false = (GTA_Computed_Value *)&gta_computed_value_boolean_false_singleton;


GTA_Computed_Value_Boolean * GTA_CALL gta_computed_value_boolean_create(bool value, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return value
    ? (GTA_Computed_Value_Boolean *)gta_computed_value_boolean_true
    : (GTA_Computed_Value_Boolean *)gta_computed_value_boolean_false;
}


bool GTA_CALL gta_computed_value_boolean_create_in_place(GTA_Computed_Value_Boolean * self, bool value, GTA_Execution_Context * context) {
  assert(self);
  *self = (GTA_Computed_Value_Boolean) {
    .base = {
      .vtable = &gta_computed_value_boolean_vtable,
      .context = context,
      .is_true = value,
      .is_error = false,
      .is_temporary = false,
      .requires_deep_copy = false,
      .is_singleton = false,
      .is_a_reference = false,
    },
    .value = value,
  };
  return true;
}


void GTA_CALL gta_computed_value_boolean_destroy(GTA_Computed_Value * self) {
  assert(self);
  if (!self->is_singleton) {
    gcu_free(self);
  }
}


void GTA_CALL gta_computed_value_boolean_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {}


GTA_Computed_Value * GTA_CALL gta_computed_value_boolean_deep_copy(GTA_Computed_Value * value, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return value;
}


char * GTA_CALL gta_computed_value_boolean_to_string(GTA_Computed_Value * self) {
  assert(self);
  char * str = (char *)gcu_malloc(6);
  if (!str) {
    return 0;
  }
  strcpy(str, self->is_true ? "true" : "false");
  return str;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_boolean_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context) {
  assert(self);
  if (type == &gta_computed_value_boolean_vtable) {
    return self;
  }
  if (type == &gta_computed_value_integer_vtable) {
    return (GTA_Computed_Value *)gta_computed_value_integer_create(self->is_true ? 1 : 0, context);
  }
  if (type == &gta_computed_value_float_vtable) {
    return (GTA_Computed_Value *)gta_computed_value_float_create(self->is_true ? 1.0 : 0.0, context);
  }
  if (type == &gta_computed_value_string_vtable) {
    char * str = gta_computed_value_boolean_to_string(self);
    if (!str) {
      return NULL;
    }
    GTA_Unicode_String * unicode_str = gta_unicode_string_create(str, strlen(str), GTA_UNICODE_STRING_TYPE_HTML);
    gcu_free(str);
    if (!unicode_str) {
      return (GTA_Computed_Value *)gta_computed_value_error_out_of_memory;
    }
    GTA_Computed_Value * return_value = (GTA_Computed_Value *)gta_computed_value_string_create(unicode_str, true, context);
    if (!return_value) {
      gta_unicode_string_destroy(unicode_str);
      return (GTA_Computed_Value *)gta_computed_value_error_out_of_memory;
    }
    return return_value;
  }
  return 0;
}
