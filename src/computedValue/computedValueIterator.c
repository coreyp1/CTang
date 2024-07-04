#include <stdbool.h>
#include <stdio.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueIterator.h>
#include <tang/program/executionContext.h>

GTA_Computed_Value_VTable gta_computed_value_iterator_vtable = {
  .name = "Iterator",
  .destroy = gta_computed_value_iterator_destroy,
  .destroy_in_place = gta_computed_value_iterator_destroy_in_place,
  .deep_copy = gta_computed_value_null_deep_copy,
  .to_string = gta_computed_value_null_to_string,
  .print = gta_computed_value_generic_print_from_to_string,
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
  .period = gta_computed_value_period_not_supported,
  .index = gta_computed_value_index_not_supported,
  .slice = gta_computed_value_slice_not_supported,
  .iterator_get = gta_computed_value_iterator_get_not_supported,
  .iterator_next = gta_computed_value_iterator_iterator_next,
  .cast = gta_computed_value_cast_not_supported,
  .call = gta_computed_value_call_not_supported,
};


static GTA_Computed_Value_Error gta_computed_value_error_iterator_end_singleton = {
  .base = {
    .vtable = &gta_computed_value_error_vtable,
    .context = 0,
    .is_true = false,
    .is_error = true,
    .is_temporary = false,
    .requires_deep_copy = false,
    .is_singleton = true,
    .is_a_reference = false,
  },
  .message = "Iterator end",
};

GTA_Computed_Value * gta_computed_value_error_iterator_end = (GTA_Computed_Value *)&gta_computed_value_error_iterator_end_singleton;


GTA_Computed_Value * GTA_CALL gta_computed_value_iterator_create(GTA_Computed_Value * collection, GTA_Execution_Context * context) {
  GTA_Computed_Value_Iterator * self = gcu_malloc(sizeof(GTA_Computed_Value_Iterator));
  if (!self) {
    return NULL;
  }
  if (!gta_computed_value_iterator_create_in_place(self, collection, context)) {
    gcu_free(self);
    return NULL;
  }
  // Attempt to add the pointer to the context's garbage collection list.
  if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
    gta_computed_value_iterator_destroy_in_place(&self->base);
    return gta_computed_value_error_out_of_memory;
  }
  return (GTA_Computed_Value *)self;
}


bool GTA_CALL gta_computed_value_iterator_create_in_place(GTA_Computed_Value_Iterator * self, GTA_Computed_Value * collection, GTA_Execution_Context * context) {
  *self = (GTA_Computed_Value_Iterator){
    .base = {
      .vtable = &gta_computed_value_iterator_vtable,
      .context = context,
      .is_true = true,
      .is_error = false,
      .is_temporary = false,
      .requires_deep_copy = false,
      .is_singleton = false,
      .is_a_reference = false,
    },
    .collection = collection,
    .index = 0,
    .resource = NULL,
    .advance = NULL,
    .destroy = NULL,
    .value = NULL,
  };
  return true;
}


void GTA_CALL gta_computed_value_iterator_destroy(GTA_Computed_Value * self) {
  gta_computed_value_destroy_in_place(self);
  gcu_free(self);
}


void GTA_CALL gta_computed_value_iterator_destroy_in_place(GTA_Computed_Value * self) {
  GTA_Computed_Value_Iterator * iterator = (GTA_Computed_Value_Iterator *)self;
  if (iterator->destroy) {
    iterator->destroy(iterator);
  }
}


GTA_Computed_Value * GTA_CALL gta_computed_value_iterator_iterator_next(GTA_Computed_Value * self, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  GTA_Computed_Value_Iterator * iterator = (GTA_Computed_Value_Iterator *)self;
  if (iterator->advance) {
    iterator->advance(iterator);
  }
  ++iterator->index;
  return iterator->value;
}
