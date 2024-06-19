
#include <stdbool.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValue.h>
#include <tang/computedValue/computedValueBoolean.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueFloat.h>
#include <tang/computedValue/computedValueInteger.h>
#include <tang/computedValue/computedValueString.h>


#define BINARY_OPERATION_TRY_OR_REVERSE(A) \
  GTA_Computed_Value * result = self->vtable->A(self, other, self_is_lhs, is_assignment, context); \
  if (self_is_lhs && ((result == gta_computed_value_error_not_implemented) || (result == gta_computed_value_error_not_supported))) { \
    return other->vtable->A(other, self, !self_is_lhs, is_assignment, context); \
  } \
  return result;


#define BINARY_OPERATION_TRY_OR_REVERSE_COMPARISON(A) \
  GTA_Computed_Value * result = self->vtable->A(self, other, self_is_lhs, context); \
  if (self_is_lhs && ((result == gta_computed_value_error_not_implemented) || (result == gta_computed_value_error_not_supported))) { \
    return other->vtable->A(other, self, !self_is_lhs, context); \
  } \
  return result;


GTA_Computed_Value_VTable gta_computed_value_null_vtable = {
  .name = "Null",
  .destroy = gta_computed_value_null_destroy,
  .destroy_in_place = gta_computed_value_null_destroy_in_place,
  .deep_copy = gta_computed_value_null_deep_copy,
  .to_string = gta_computed_value_null_to_string,
  .assign_index = gta_computed_value_assign_index_not_implemented,
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
  .equal = gta_computed_value_equal_not_implemented,
  .not_equal = gta_computed_value_not_equal_not_implemented,
  .period = gta_computed_value_period_not_supported,
  .index = gta_computed_value_index_not_supported,
  .slice = gta_computed_value_slice_not_supported,
  .iterator_get = gta_computed_value_iterator_get_not_supported,
  .iterator_next = gta_computed_value_iterator_next_not_supported,
  .cast = gta_computed_value_null_cast,
  .call = gta_computed_value_call_not_supported,
};


static GTA_Computed_Value computed_value_null_singleton = {
  .vtable = &gta_computed_value_null_vtable,
  .context = 0,
  .is_true = false,
  .is_error = false,
  .is_temporary = true,
  .requires_deep_copy = false,
  .is_singleton = true,
  .is_a_reference = false,
};


GTA_Computed_Value * gta_computed_value_null = &computed_value_null_singleton;


GTA_Computed_Value * gta_computed_value_create(GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_null;
}


bool gta_computed_value_create_in_place(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  self->context = context;
  return true;
}


void gta_computed_value_destroy(GTA_Computed_Value * self) {
  self->vtable->destroy(self);
}


void gta_computed_value_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {
  self->vtable->destroy_in_place(self);
}


GTA_Computed_Value * gta_computed_value_deep_copy(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  return self->vtable->deep_copy(self, context);
}


char * gta_computed_value_to_string(GTA_Computed_Value * self) {
  return self->vtable->to_string(self);
}


GTA_Computed_Value * gta_computed_value_assign_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Computed_Value * other, GTA_Execution_Context * context) {
  return self->vtable->assign_index(self, index, other, context);
}


GTA_Computed_Value * gta_computed_value_add(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context) {
  BINARY_OPERATION_TRY_OR_REVERSE(add)
}


GTA_Computed_Value * gta_computed_value_subtract(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context) {
  BINARY_OPERATION_TRY_OR_REVERSE(subtract)
}


GTA_Computed_Value * gta_computed_value_multiply(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context) {
  BINARY_OPERATION_TRY_OR_REVERSE(multiply)
}


GTA_Computed_Value * gta_computed_value_divide(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context) {
  BINARY_OPERATION_TRY_OR_REVERSE(divide)
}


GTA_Computed_Value * gta_computed_value_modulo(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context) {
  BINARY_OPERATION_TRY_OR_REVERSE(modulo)
}


GTA_Computed_Value * gta_computed_value_negative(GTA_Computed_Value * self, bool is_assignment, GTA_Execution_Context * context) {
  return self->vtable->negative(self, is_assignment, context);
}


GTA_Computed_Value * gta_computed_value_less_than(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context) {
  BINARY_OPERATION_TRY_OR_REVERSE_COMPARISON(less_than)
}


GTA_Computed_Value * gta_computed_value_less_than_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context) {
  BINARY_OPERATION_TRY_OR_REVERSE_COMPARISON(less_than_equal)
}


GTA_Computed_Value * gta_computed_value_greater_than(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context) {
  BINARY_OPERATION_TRY_OR_REVERSE_COMPARISON(greater_than)
}


GTA_Computed_Value * gta_computed_value_greater_than_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context) {
  BINARY_OPERATION_TRY_OR_REVERSE_COMPARISON(greater_than_equal)
}


GTA_Computed_Value * gta_computed_value_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context) {
  BINARY_OPERATION_TRY_OR_REVERSE_COMPARISON(equal)
}


GTA_Computed_Value * gta_computed_value_not_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context) {
  BINARY_OPERATION_TRY_OR_REVERSE_COMPARISON(not_equal)
}


GTA_Computed_Value * gta_computed_value_period(GTA_Computed_Value * self, const char * identifier, GTA_Execution_Context * context) {
  return self->vtable->period(self, identifier, context);
}


GTA_Computed_Value * gta_computed_value_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Execution_Context * context) {
  return self->vtable->index(self,  index, context);
}


GTA_Computed_Value * gta_computed_value_slice(GTA_Computed_Value * self, GTA_Computed_Value * start, GTA_Computed_Value * end, GTA_Computed_Value * step, GTA_Execution_Context * context) {
  return self->vtable->slice(self, start, end, step, context);
}


GTA_Computed_Value * gta_computed_value_iterator_get(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  return self->vtable->iterator_get(self, context);
}


GTA_Computed_Value * gta_computed_value_iterator_next(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  return self->vtable->iterator_next(self, context);
}


GTA_Computed_Value * gta_computed_value_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context) {
  return self->vtable->cast(self, type, context);
}


GTA_Computed_Value * gta_computed_value_call(GTA_Computed_Value * self, GTA_Computed_Value_Vector * arguments, GTA_Execution_Context * context) {
  return self->vtable->call(self, arguments, context);
}


void gta_computed_value_null_destroy(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {
  return;
}


void gta_computed_value_null_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {
  return;
}


GTA_Computed_Value * gta_computed_value_null_deep_copy(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return self;
}


char * gta_computed_value_null_to_string(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {
  char * str = (char *)gcu_malloc(5);
  if (!str) {
    return 0;
  }
  strcpy(str, "null");
  return str;
}


GTA_Computed_Value * gta_computed_value_null_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context) {
  if (type == &gta_computed_value_boolean_vtable) {
    return gta_computed_value_boolean_false;
  }
  if (type == &gta_computed_value_integer_vtable) {
    return (GTA_Computed_Value *)gta_computed_value_integer_create(0, context);
  }
  if (type == &gta_computed_value_float_vtable) {
    return (GTA_Computed_Value *)gta_computed_value_float_create(0.0, context);
  }
  if (type == &gta_computed_value_string_vtable) {
    char * str = gta_computed_value_null_to_string(self);
    if (!str) {
      return NULL;
    }
    GTA_Unicode_String * unicode_str = gta_unicode_string_create(str, strlen(str), GTA_UNICODE_STRING_TYPE_UNTRUSTED);
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
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_assign_index_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * index), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_add_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_subtract_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_multiply_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_divide_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_modulo_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_negative_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_less_than_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_less_than_equal_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_greater_than_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_greater_than_equal_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_equal_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_not_equal_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_period_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(const char * identifier), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_index_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * index), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_slice_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * start), GTA_MAYBE_UNUSED(GTA_Computed_Value * end), GTA_MAYBE_UNUSED(GTA_Computed_Value * step), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_iterator_get_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_iterator_next_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_cast_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value_VTable * type), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_call_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value_Vector * arguments), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_assign_index_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * index), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_add_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_subtract_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_multiply_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_divide_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_modulo_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_negative_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_less_than_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_less_than_equal_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_greater_than_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_greater_than_equal_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_equal_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_not_equal_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_period_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(const char * identifier), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_index_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * index), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_slice_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * start), GTA_MAYBE_UNUSED(GTA_Computed_Value * end), GTA_MAYBE_UNUSED(GTA_Computed_Value * step), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_iterator_get_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_iterator_next_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_cast_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value_VTable * type), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * gta_computed_value_call_not_supported(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value_Vector * arguments), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}
