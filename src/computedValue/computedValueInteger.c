
#include <stdio.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValueInteger.h>
#include <tang/program/executionContext.h>

GTA_Computed_Value_VTable gta_computed_value_integer_vtable = {
  .name = "Integer",
  .destroy = gta_computed_value_integer_destroy,
  .destroy_in_place = gta_computed_value_integer_destroy_in_place,
  .deep_copy = gta_computed_value_integer_deep_copy,
  .to_string = gta_computed_value_integer_to_string,
  .assign_index = gta_computed_value_assign_index_not_implemented,
  .add = gta_computed_value_add_not_implemented,
  .subtract = gta_computed_value_subtract_not_implemented,
  .multiply = gta_computed_value_multiply_not_implemented,
  .divide = gta_computed_value_divide_not_implemented,
  .modulo = gta_computed_value_modulo_not_implemented,
  .negative = gta_computed_value_integer_negative,
  .logical_and = gta_computed_value_logical_and_not_implemented,
  .logical_or = gta_computed_value_logical_or_not_implemented,
  .logical_not = gta_computed_value_null_logical_not,
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

#include <execinfo.h>
#include <stdlib.h>


GTA_Computed_Value_Integer * GTA_CALL gta_computed_value_integer_create(GTA_Integer value, GTA_Execution_Context * context) {
  GTA_Computed_Value_Integer * self = gcu_malloc(sizeof(GTA_Computed_Value_Integer));
  if (!self) {
    return 0;
  }
  // Attempt to add the pointer to the context's garbage collection list.
  if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
    gcu_free(self);
    return NULL;
  }
  gta_computed_value_integer_create_in_place(self, value, context);
  return self;
}


bool GTA_CALL gta_computed_value_integer_create_in_place(GTA_Computed_Value_Integer * self, GTA_Integer value, GTA_Execution_Context * context) {
  *self = (GTA_Computed_Value_Integer) {
    .base = {
      .vtable = &gta_computed_value_integer_vtable,
      .context = context,
      .is_true = (bool)value,
      .is_error = false,
      .is_temporary = true,
      .requires_deep_copy = false,
      .is_singleton = false,
      .is_a_reference = false,
    },
    .value = value,
  };
  return true;
}


void GTA_CALL gta_computed_value_integer_destroy(GTA_Computed_Value * computed_value) {
  gcu_free(computed_value);
}


void GTA_CALL gta_computed_value_integer_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {}


GTA_Computed_Value * GTA_CALL gta_computed_value_integer_deep_copy(GTA_Computed_Value * value) {
  GTA_Computed_Value_Integer * integer = (GTA_Computed_Value_Integer *)value;
  return (GTA_Computed_Value *)gta_computed_value_integer_create(integer->value, integer->base.context);
}


char * GTA_CALL gta_computed_value_integer_to_string(GTA_Computed_Value * self) {
  GTA_Computed_Value_Integer * integer = (GTA_Computed_Value_Integer *)self;
  char * str = (char *)gcu_malloc(32);
  if (!str) {
    return 0;
  }
  sprintf(str, sizeof(GTA_Integer) == 8 ? "%ld" : "%d", integer->value);
  return str;
}


GTA_Computed_Value * gta_computed_value_integer_negative(GTA_Computed_Value * self) {
  GTA_Computed_Value_Integer * integer = (GTA_Computed_Value_Integer *)self;
  if (integer->base.is_temporary) {
    integer->value = -integer->value;
    return self;
  }
  return (GTA_Computed_Value *)gta_computed_value_integer_create(-integer->value, integer->base.context);
}
