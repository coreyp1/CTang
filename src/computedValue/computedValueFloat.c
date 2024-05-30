
#include <stdio.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValueFloat.h>
#include <tang/program/executionContext.h>

GTA_Computed_Value_VTable gta_computed_value_float_vtable = {
  .name = "Float",
  .destroy = gta_computed_value_float_destroy,
  .destroy_in_place = gta_computed_value_float_destroy_in_place,
  .deep_copy = gta_computed_value_float_deep_copy,
  .to_string = gta_computed_value_float_to_string,
  .assign_index = gta_computed_value_assign_index_not_implemented,
  .add = gta_computed_value_add_not_implemented,
  .subtract = gta_computed_value_subtract_not_implemented,
  .multiply = gta_computed_value_multiply_not_implemented,
  .divide = gta_computed_value_divide_not_implemented,
  .modulo = gta_computed_value_modulo_not_implemented,
  .negative = gta_computed_value_float_negative,
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


GTA_Computed_Value_Float * gta_computed_value_float_create(GTA_Float value, GTA_Execution_Context * context) {
  GTA_Computed_Value_Float * self = (GTA_Computed_Value_Float *) gcu_malloc(sizeof(GTA_Computed_Value_Float));
  if (!self) {
    return NULL;
  }
  // Attempt to add the pointer to the context's garbage collection list.
  if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
    gcu_free(self);
    return NULL;
  }
  gta_computed_value_float_create_in_place(self, value, context);
  return self;
}


bool gta_computed_value_float_create_in_place(GTA_Computed_Value_Float * self, GTA_Float value, GTA_Execution_Context * context) {
  *self = (GTA_Computed_Value_Float) {
    .base = {
      .vtable = &gta_computed_value_float_vtable,
      .context = context,
      .is_true = (bool)value,
      .is_error = false,
      .is_temporary = true,
      .requires_deep_copy = false,
      .is_singleton = false,
      .is_a_reference = false,
    },
    .value = value
  };
  return true;
}


void gta_computed_value_float_destroy(GTA_Computed_Value * self) {
  gcu_free(self);
}


void gta_computed_value_float_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {}


GTA_Computed_Value * gta_computed_value_float_deep_copy(GTA_Computed_Value * value) {
  GTA_Computed_Value_Float * float_value = (GTA_Computed_Value_Float *)value;
  return (GTA_Computed_Value *)gta_computed_value_float_create(float_value->value, float_value->base.context);
}


char * gta_computed_value_float_to_string(GTA_Computed_Value * self) {
  GTA_Computed_Value_Float * float_value = (GTA_Computed_Value_Float *)self;
  char * str = (char *)gcu_malloc(32);
  if (!str) {
    return 0;
  }
  sprintf(str, sizeof(GTA_Integer) == 8 ? "%lf" : "%f", float_value->value);
  return str;
}


GTA_Computed_Value * gta_computed_value_float_negative(GTA_Computed_Value * self) {
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;
  if (number->base.is_temporary) {
    number->value = -number->value;
    return self;
  }
  return (GTA_Computed_Value *)gta_computed_value_float_create(-number->value, number->base.context);
}
