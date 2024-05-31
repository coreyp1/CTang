
#include <string.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValue.h>
#include <tang/computedValue/computedValueBoolean.h>
#include <tang/computedValue/computedValueError.h>


GTA_Computed_Value_VTable gta_computed_value_null_vtable = {
  .name = "Null",
  .destroy = gta_computed_value_null_destroy,
  .destroy_in_place = gta_computed_value_null_destroy_in_place,
  .deep_copy = gta_computed_value_null_deep_copy,
  .to_string = gta_computed_value_null_to_string,
  .assign_index = gta_computed_value_assign_index_not_implemented,
  .add = gta_computed_value_add_not_implemented,
  .subtract = gta_computed_value_subtract_not_implemented,
  .multiply = gta_computed_value_multiply_not_implemented,
  .divide = gta_computed_value_divide_not_implemented,
  .modulo = gta_computed_value_modulo_not_implemented,
  .negative = gta_computed_value_negative_not_implemented,
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


GTA_Computed_Value * gta_computed_value_deep_copy(GTA_Computed_Value * self) {
  return self->vtable->deep_copy(self);
}


char * gta_computed_value_to_string(GTA_Computed_Value * self) {
  return self->vtable->to_string(self);
}


GTA_Computed_Value * gta_computed_value_assign_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Computed_Value * other) {
  return self->vtable->assign_index(self, index, other);
}


GTA_Computed_Value * gta_computed_value_add(GTA_Computed_Value * self, GTA_Computed_Value * other, bool reverse) {
  return self->vtable->add(self, other, reverse);
}


GTA_Computed_Value * gta_computed_value_subtract(GTA_Computed_Value * self, GTA_Computed_Value * other, bool reverse) {
  return self->vtable->subtract(self, other, reverse);
}


GTA_Computed_Value * gta_computed_value_multiply(GTA_Computed_Value * self, GTA_Computed_Value * other, bool reverse) {
  return self->vtable->multiply(self, other, reverse);
}


GTA_Computed_Value * gta_computed_value_divide(GTA_Computed_Value * self, GTA_Computed_Value * other, bool reverse) {
  return self->vtable->divide(self, other, reverse);
}


GTA_Computed_Value * gta_computed_value_modulo(GTA_Computed_Value * self, GTA_Computed_Value * other, bool reverse) {
  return self->vtable->modulo(self, other, reverse);
}


GTA_Computed_Value * gta_computed_value_negative(GTA_Computed_Value * self) {
  return self->vtable->negative(self);
}


GTA_Computed_Value * gta_computed_value_logical_and(GTA_Computed_Value * self, GTA_Computed_Value * other, bool reverse) {
  return self->vtable->logical_and(self, other, reverse);
}


GTA_Computed_Value * gta_computed_value_logical_or(GTA_Computed_Value * self, GTA_Computed_Value * other, bool reverse) {
  return self->vtable->logical_or(self, other, reverse);
}


GTA_Computed_Value * gta_computed_value_logical_not(GTA_Computed_Value * self) {
  return self->vtable->logical_not(self);
}


GTA_Computed_Value * gta_computed_value_less_than(GTA_Computed_Value * self, GTA_Computed_Value * other, bool reverse) {
  return self->vtable->less_than(self, other, reverse);
}


GTA_Computed_Value * gta_computed_value_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool reverse) {
  return self->vtable->equal(self, other, reverse);
}


GTA_Computed_Value * gta_computed_value_period(GTA_Computed_Value * self, const char * identifier) {
  return self->vtable->period(self, identifier);
}


GTA_Computed_Value * gta_computed_value_index(GTA_Computed_Value * self, GTA_Computed_Value * index) {
  return self->vtable->index(self,  index);
}


GTA_Computed_Value * gta_computed_value_slice(GTA_Computed_Value * self, GTA_Computed_Value * start, GTA_Computed_Value * end, GTA_Computed_Value * step) {
  return self->vtable->slice(self, start, end, step);
}


GTA_Computed_Value * gta_computed_value_iterator_get(GTA_Computed_Value * self) {
  return self->vtable->iterator_get(self);
}


GTA_Computed_Value * gta_computed_value_iterator_next(GTA_Computed_Value * self) {
  return self->vtable->iterator_next(self);
}


GTA_Computed_Value * gta_computed_value_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, bool reverse) {
  return self->vtable->cast(self, type, reverse);
}


GTA_Computed_Value * gta_computed_value_call(GTA_Computed_Value * self, GTA_Computed_Value_Vector * arguments) {
  return self->vtable->call(self, arguments);
}


void gta_computed_value_null_destroy(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {
  return;
}


void gta_computed_value_null_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {
  return;
}


GTA_Computed_Value * gta_computed_value_null_deep_copy(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {
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


GTA_Computed_Value * gta_computed_value_null_logical_not(GTA_Computed_Value * self) {
  return self->is_true ? gta_computed_value_boolean_false : gta_computed_value_boolean_true;
}


GTA_Computed_Value * gta_computed_value_assign_index_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * index), GTA_MAYBE_UNUSED(GTA_Computed_Value * other)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_add_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool reverse)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_subtract_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool reverse)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_multiply_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool reverse)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_divide_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool reverse)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_modulo_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool reverse)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_negative_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_logical_and_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool reverse)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_logical_or_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool reverse)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_logical_not_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_less_than_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool reverse)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_equal_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool reverse)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_period_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(const char * identifier)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_index_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * index)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_slice_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * start), GTA_MAYBE_UNUSED(GTA_Computed_Value * end), GTA_MAYBE_UNUSED(GTA_Computed_Value * step)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_iterator_get_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_iterator_next_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_cast_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value_VTable * type), GTA_MAYBE_UNUSED(bool reverse)) {
  return gta_computed_value_error_not_implemented;
}


GTA_Computed_Value * gta_computed_value_call_not_implemented(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value_Vector * arguments)) {
  return gta_computed_value_error_not_implemented;
}
