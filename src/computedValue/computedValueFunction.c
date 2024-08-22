
#include <assert.h>
#include <stdio.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValueBoolean.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueFunction.h>
#include <tang/program/executionContext.h>

GTA_Computed_Value_VTable gta_computed_value_function_vtable = {
  .name = "Function",
  .destroy = gta_computed_value_function_destroy,
  .destroy_in_place = gta_computed_value_function_destroy_in_place,
  .deep_copy = gta_computed_value_function_deep_copy,
  .to_string = gta_computed_value_function_to_string,
  .print = gta_computed_value_print_not_supported,
  .assign_index = gta_computed_value_assign_index_not_supported,
  .add = gta_computed_value_add_not_supported,
  .subtract = gta_computed_value_subtract_not_supported,
  .multiply = gta_computed_value_multiply_not_supported,
  .divide = gta_computed_value_divide_not_supported,
  .modulo = gta_computed_value_modulo_not_supported,
  .negative = gta_computed_value_negative_not_supported,
  .less_than = gta_computed_value_less_than_not_supported,
  .less_than_equal = gta_computed_value_less_than_equal_not_supported,
  .greater_than = gta_computed_value_greater_than_not_supported,
  .greater_than_equal = gta_computed_value_greater_than_equal_not_supported,
  .equal = gta_computed_value_equal_not_supported,
  .not_equal = gta_computed_value_not_equal_not_supported,
  .period = gta_computed_value_generic_period,
  .index = gta_computed_value_index_not_supported,
  .slice = gta_computed_value_slice_not_supported,
  .iterator_get = gta_computed_value_iterator_get_not_supported,
  .iterator_next = gta_computed_value_iterator_next_not_supported,
  .cast = gta_computed_value_cast_not_supported,
  .call = gta_computed_value_call_not_supported,
  .attributes = NULL,
  .attributes_count = 0,
};


GTA_Computed_Value_Function * GTA_CALL gta_computed_value_function_create(size_t num_arguments, size_t pointer, GTA_Execution_Context * context) {
  GTA_Computed_Value_Function * self = gcu_malloc(sizeof(GTA_Computed_Value_Function));
  if (!self) {
    return 0;
  }
  if (!gta_computed_value_function_create_in_place(self, num_arguments, pointer, context)) {
    gcu_free(self);
    return NULL;
  }
  if (context) {
    // Attempt to add the pointer to the context's garbage collection list.
    if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
      gta_computed_value_function_destroy((GTA_Computed_Value *)self);
      return NULL;
    }
  }
  return self;
}


bool gta_computed_value_function_create_in_place(GTA_Computed_Value_Function * self, size_t num_arguments, size_t pointer, GTA_Execution_Context * context) {
  assert(self);
  *self = (GTA_Computed_Value_Function) {
    .base = {
      .vtable = &gta_computed_value_function_vtable,
      .context = context,
      .is_true = true,
      .is_error = false,
      .is_temporary = false,
      .requires_deep_copy = false,
      .is_singleton = true,
      .is_a_reference = false,
    },
    .num_arguments = num_arguments,
    .pointer = pointer,
  };
  return true;
}


void gta_computed_value_function_destroy(GTA_Computed_Value * self) {
  assert(self);
  gcu_free(self);
}


void gta_computed_value_function_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {}


GTA_Computed_Value * gta_computed_value_function_deep_copy(GTA_Computed_Value * self, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  // These are always singletons.
  return self;
}

char * GTA_CALL gta_computed_value_function_to_string(GTA_Computed_Value * self) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FUNCTION(self));
  GTA_Computed_Value_Function * function = (GTA_Computed_Value_Function *)self;

  char * string = gcu_malloc(64);
  if (!string) {
    return NULL;
  }
  snprintf(string, 64, "Function(%zu, %zu)", function->num_arguments, function->pointer);
  return string;
}
