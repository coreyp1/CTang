
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueFunctionNative.h>
#include <tang/program/compilerContext.h>

/**
 * The vtable for the GTA_Computed_Value_Function_Native class.
 */
GTA_Computed_Value_VTable gta_computed_value_function_native_vtable = {
  .name = "NativeFunction",
  .destroy = gta_computed_value_function_native_destroy,
  .destroy_in_place = gta_computed_value_function_native_destroy_in_place,
  .deep_copy = gta_computed_value_function_native_deep_copy,
  .to_string = gta_computed_value_function_native_to_string,
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


GTA_Computed_Value_Function_Native * GTA_CALL gta_computed_value_function_native_create(GTA_Computed_Value_Function_Native_Callback * callback, GTA_Computed_Value * bound_object, GTA_Execution_Context * context) {
  GTA_Computed_Value_Function_Native * self = (GTA_Computed_Value_Function_Native *)gcu_malloc(sizeof(GTA_Computed_Value_Function_Native));
  if (!self) {
    return NULL;
  }
  if (!gta_computed_value_function_native_create_in_place(self, callback, bound_object, context)) {
    gcu_free(self);
    return NULL;
  }
  if (context) {
    // Attempt to add the pointer to the context's garbage collection list.
    if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
      gta_computed_value_function_native_destroy((GTA_Computed_Value *)self);
      return (GTA_Computed_Value_Function_Native *)gta_computed_value_error_out_of_memory;
    }
  }
  return self;
}


bool GTA_CALL gta_computed_value_function_native_create_in_place(GTA_Computed_Value_Function_Native * self, GTA_Computed_Value_Function_Native_Callback * callback, GTA_Computed_Value * bound_object, GTA_Execution_Context * context) {
  assert(self);
  assert(callback);
  assert(context);
  *self = (GTA_Computed_Value_Function_Native){
    .base = {
      .vtable = &gta_computed_value_function_native_vtable,
      .context = context,
      .is_true = false,
      .is_error = false,
      .is_temporary = true,
      .requires_deep_copy = false,
      .is_singleton = false,
      .is_a_reference = false,
    },
    .callback = callback,
    .bound_object = bound_object,
  };
  return true;
}


void GTA_CALL gta_computed_value_function_native_destroy(GTA_Computed_Value * self) {
  assert(self);
  gta_computed_value_function_native_destroy_in_place(self);
  gcu_free(self);
}


void GTA_CALL gta_computed_value_function_native_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {}


GTA_Computed_Value * GTA_CALL gta_computed_value_function_native_deep_copy(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  return (GTA_Computed_Value *)gta_computed_value_function_native_create(((GTA_Computed_Value_Function_Native *)self)->callback, ((GTA_Computed_Value_Function_Native *)self)->bound_object, context);
}


char * GTA_CALL gta_computed_value_function_native_to_string(GTA_Computed_Value * self) {
  const char * format_string = "FunctionNative<%p>(Callback<%p>, BoundObject<%p>)";
  const size_t buffer_size = strlen(format_string) + (2 * 3 * (sizeof(void *) + 2)) + 1;
  char * buffer = gcu_malloc(buffer_size);
  if (!buffer) {
    return NULL;
  }
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FUNCTION_NATIVE(self));
  GTA_Computed_Value_Function_Native * function_native = (GTA_Computed_Value_Function_Native *)self;
  snprintf(buffer, buffer_size, format_string, (void *)self, (void *)GTA_JIT_FUNCTION_CONVERTER(function_native->callback), (void *)function_native->bound_object);
  return buffer;
}
