
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/computedValueBoolean.h>

GTA_Computed_Value_VTable gta_computed_value_boolean_vtable = {
  .name = "Boolean",
  .destroy = gta_computed_value_boolean_destroy,
  .destroy_in_place = gta_computed_value_boolean_destroy,
  .deep_copy = gta_computed_value_boolean_deep_copy,
  .to_string = gta_computed_value_boolean_to_string,
  .assign_index = gta_computed_value_assign_index_not_implemented,
  .add = gta_computed_value_add_not_implemented,
  .subtract = gta_computed_value_subtract_not_implemented,
  .multiply = gta_computed_value_multiply_not_implemented,
  .divide = gta_computed_value_divide_not_implemented,
  .modulo = gta_computed_value_modulo_not_implemented,
  .negative = gta_computed_value_negative_not_implemented,
  .logical_and = gta_computed_value_logical_and_not_implemented,
  .logical_or = gta_computed_value_logical_or_not_implemented,
  .logical_not = gta_computed_value_logical_not_not_implemented,
  .less_than = gta_computed_value_less_than_not_implemented,
  .equal = gta_computed_value_equal_not_implemented,
  .period = gta_computed_value_period_not_implemented,
  .index = gta_computed_value_index_not_implemented,
  .slice = gta_computed_value_slice_not_implemented,
  .iterator_get = gta_computed_value_iterator_get_not_implemented,
  .iterator_next = gta_computed_value_iterator_next_not_implemented,
  .cast = gta_computed_value_cast_not_implemented,
  .call = gta_computed_value_call_not_implemented,
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

GTA_Computed_Value_Boolean * gta_computed_value_boolean_create(bool value, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return value
    ? (GTA_Computed_Value_Boolean *)gta_computed_value_boolean_true
    : (GTA_Computed_Value_Boolean *)gta_computed_value_boolean_false;
}

bool gta_computed_value_boolean_create_in_place(GTA_Computed_Value_Boolean * self, bool value, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  *self = (GTA_Computed_Value_Boolean) {
    .base = {
      .vtable = &gta_computed_value_boolean_vtable,
      .context = 0,
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

void gta_computed_value_boolean_destroy(GTA_Computed_Value * self) {
  if (!self->is_singleton) {
    gcu_free(self);
  }
}

void gta_computed_value_boolean_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {}

GTA_Computed_Value * gta_computed_value_boolean_deep_copy(GTA_Computed_Value * value) {
  return value;
}

char * gta_computed_value_boolean_to_string(GTA_Computed_Value * self) {
  char * str = (char *)gcu_malloc(6);
  if (!str) {
    return 0;
  }
  strcpy(str, self->is_true ? "true" : "false");
  return str;
}
