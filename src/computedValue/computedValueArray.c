
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValueArray.h>
#include <tang/computedValue/computedValueBoolean.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueInteger.h>
#include <tang/computedValue/computedValueIterator.h>
#include <tang/program/executionContext.h>


/**
 * Return the size of the array.
 *
 * @param self The array object.
 * @param context The execution context.
 * @return The size of the array.
 */
static GTA_Computed_Value * GTA_CALL array_size(GTA_Computed_Value * self, GTA_Execution_Context * context);


/**
 * The attributes for the GTA_Computed_Value_Array class.
 */
static GTA_Computed_Value_Attribute_Pair attributes[] = {
  {"size", array_size},
};


GTA_Computed_Value_VTable gta_computed_value_array_vtable = {
  .name = "Array",
  .destroy = gta_computed_value_array_destroy,
  .destroy_in_place = gta_computed_value_array_destroy_in_place,
  .deep_copy = gta_computed_value_array_deep_copy,
  .to_string = gta_computed_value_array_to_string,
  .print = gta_computed_value_generic_print_from_to_string,
  .assign_index = gta_computed_value_array_index_assign,
  .add = gta_computed_value_array_add,
  .subtract = gta_computed_value_subtract_not_supported,
  .multiply = gta_computed_value_array_multiply,
  .divide = gta_computed_value_divide_not_supported,
  .modulo = gta_computed_value_modulo_not_supported,
  .negative = gta_computed_value_negative_not_supported,
  .less_than = gta_computed_value_less_than_not_supported,
  .less_than_equal = gta_computed_value_less_than_equal_not_supported,
  .greater_than = gta_computed_value_greater_than_not_supported,
  .greater_than_equal = gta_computed_value_greater_than_equal_not_supported,
  .equal = gta_computed_value_array_equal,
  .not_equal = gta_computed_value_array_not_equal,
  .period = gta_computed_value_generic_period,
  .index = gta_computed_value_array_index,
  .slice = gta_computed_value_array_slice,
  .iterator_get = gta_computed_value_array_iterator_get,
  .iterator_next = gta_computed_value_iterator_next_not_implemented,
  .cast = gta_computed_value_cast,
  .call = gta_computed_value_call_not_supported,
  .attributes = attributes,
  .attributes_count = 0,
};


/*
 * Setup the proper attribute count, which cannot be done at compile time.
 */
GTA_INIT_FUNCTION(setup) {
  gta_computed_value_array_vtable.attributes_count = sizeof(attributes) / sizeof(GTA_Computed_Value_Attribute_Pair);
}


GTA_Computed_Value * GTA_CALL gta_computed_value_array_create(size_t size, GTA_Execution_Context * context) {
  assert(context);

  GTA_Computed_Value_Array * self = gcu_malloc(sizeof(GTA_Computed_Value_Array));
  if (self == NULL) {
    return gta_computed_value_error_out_of_memory;
  }
  if (!gta_computed_value_array_create_in_place(self, size, context)) {
    gcu_free(self);
    return gta_computed_value_error_out_of_memory;
  }
  if (context) {
    // Attempt to add the pointer to the context's garbage collection list.
    if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
      gta_computed_value_array_destroy_in_place(&self->base);
      return gta_computed_value_error_out_of_memory;
    }
  }
  return (GTA_Computed_Value *)self;
}


bool GTA_CALL gta_computed_value_array_create_in_place(GTA_Computed_Value_Array * self, size_t size, GTA_Execution_Context * context) {
  *self = (GTA_Computed_Value_Array) {
    .base = {
      .vtable = &gta_computed_value_array_vtable,
      .context = context,
      .is_true = false,
      .is_error = false,
      .is_temporary = true,
      .requires_deep_copy = false,
      .is_singleton = false,
      .is_a_reference = false,
    },
    .elements = GTA_VECTORX_CREATE(size),
  };
  return self->elements != NULL;
}


void GTA_CALL gta_computed_value_array_destroy(GTA_Computed_Value * self) {
  assert(self);
  gta_computed_value_array_destroy_in_place(self);
  gcu_free(self);
}


void GTA_CALL gta_computed_value_array_destroy_in_place(GTA_Computed_Value * self) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(self));
  GTA_VECTORX_DESTROY(((GTA_Computed_Value_Array *) self)->elements);
}


GTA_Computed_Value * GTA_CALL gta_computed_value_array_deep_copy(GTA_Computed_Value * value, GTA_Execution_Context * context) {
  assert(value);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(value));
  GTA_Computed_Value_Array * self = (GTA_Computed_Value_Array *)value;
  GTA_Computed_Value_Array * copy = (GTA_Computed_Value_Array *)gta_computed_value_array_create(self->elements->count, context);
  if (!copy) {
    return gta_computed_value_error_out_of_memory;
  }

  // Note: From this point forward, the `copy` object is tracked by the garbage
  // collection list of the context. If an error occurs, the garbage collector
  // will free the memory allocated for the `copy` object.  The same is true
  // for the `element_copy` object.  We will not explicitly delete anything
  // in this function if an error occurs.

  assert(self->elements);
  assert(self->elements->count ? (bool)self->elements->data : true);
  for (size_t i = 0; i < self->elements->count; i++) {
    GTA_Computed_Value * element = GTA_TYPEX_P(self->elements->data[i]);
    GTA_Computed_Value * element_copy = gta_computed_value_deep_copy(element, context);
    if (!element_copy) {
      return gta_computed_value_error_out_of_memory;
    }
    copy->elements->data[i] = GTA_TYPEX_MAKE_P(element_copy);
    copy->elements->count++;
  }
  return (GTA_Computed_Value *)copy;
}

char * GTA_CALL gta_computed_value_array_to_string(GTA_Computed_Value * self) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(self));
  GTA_Computed_Value_Array * array = (GTA_Computed_Value_Array *)self;

  assert(array->elements);
  assert(array->elements->count ? (bool)array->elements->data : true);

  // If the array is empty, return "[]".
  if (!array->elements->count) {
    char * output = gcu_malloc(3);
    if (!output) {
      return NULL;
    }
    output[0] = '[';
    output[1] = ']';
    output[2] = '\0';
    return output;
  }

  // The final output will be a string of the form "[element1, element2, ...]".
  char * * strings = gcu_malloc(sizeof(char *) * array->elements->count);
  if (!strings) {
    return NULL;
  }

  size_t total_length = 0;
  for (size_t i = 0; i < array->elements->count; ++i) {
    GTA_Computed_Value * element = GTA_TYPEX_P(array->elements->data[i]);
    strings[i] = gta_computed_value_to_string(element);
    if (!strings[i]) {
      for (size_t j = 0; j < i; j++) {
        gcu_free(strings[j]);
      }
      gcu_free(strings);
      return NULL;
    }
    total_length += strlen(strings[i]);
  }

  // Assemble the final string.
  char * output = gcu_malloc(total_length + ((array->elements->count - 1) * 2) + 3);

  if (!output) {
    for (size_t i = 0; i < array->elements->count; i++) {
      gcu_free(strings[i]);
    }
    gcu_free(strings);
    return NULL;
  }

  output[0] = '[';
  size_t offset = 1;
  for (size_t i = 0; i < array->elements->count; i++) {
    if (i > 0) {
      output[offset++] = ',';
      output[offset++] = ' ';
    }
    strcpy(output + offset, strings[i]);
    offset += strlen(strings[i]);
    gcu_free(strings[i]);
  }
  output[offset++] = ']';
  output[offset] = '\0';
  gcu_free(strings);
  return output;
}

GTA_Computed_Value * GTA_CALL gta_computed_value_array_index_assign(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Computed_Value * other, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(self));
  GTA_Computed_Value_Array * array = (GTA_Computed_Value_Array *)self;

  assert(array->elements);
  assert(array->elements->count ? (bool)array->elements->data : true);

  if (!GTA_COMPUTED_VALUE_IS_INTEGER(index)) {
    return gta_computed_value_error_invalid_index;
  }
  GTA_Computed_Value_Integer * integer = (GTA_Computed_Value_Integer *)index;

  // Normalize the index.
  GTA_Integer normalized_index = integer->value >= 0
    ? integer->value
    : (GTA_Integer)array->elements->count + integer->value;

  // Check if the index is valid.
  if (normalized_index < 0) {
    return gta_computed_value_error_invalid_index;
  }

  // Expand the array if the index is beyond the current bounds.
  if (normalized_index >= (GTA_Integer)array->elements->count) {
    size_t new_size = normalized_index + 1;
    if (!GTA_VECTORX_RESERVE(array->elements, new_size)) {
      return gta_computed_value_error_out_of_memory;
    }
    for (size_t i = array->elements->count; i < new_size - 1; i++) {
      // Populate the "new" elements with null.
      array->elements->data[i] = GTA_TYPEX_MAKE_P(gta_computed_value_null);
    }
    array->elements->count = new_size;
  }

  // Either copy or adopt the new value.
  if (other->is_temporary || other->is_singleton) {
    other->is_temporary = false;
    array->elements->data[normalized_index] = GTA_TYPEX_MAKE_P(other);
    return other;
  }

  GTA_Computed_Value * element_copy = gta_computed_value_deep_copy(other, context);
  if (!element_copy) {
    return gta_computed_value_error_out_of_memory;
  }
  array->elements->data[normalized_index] = GTA_TYPEX_MAKE_P(element_copy);
  return element_copy;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_array_add(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_MAYBE_UNUSED(bool is_assignment), GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(self));
  GTA_Computed_Value_Array * lhs = (GTA_Computed_Value_Array *)self;

  if (!self_is_lhs || !GTA_COMPUTED_VALUE_IS_ARRAY(other)) {
    return gta_computed_value_error_not_supported;
  }
  GTA_Computed_Value_Array * rhs = (GTA_Computed_Value_Array *)other;

  assert(lhs->elements);
  assert(lhs->elements->count ? (bool)lhs->elements->data : true);
  assert(rhs->elements);
  assert(rhs->elements->count ? (bool)rhs->elements->data : true);

  // Create a new array that is the concatenation of the two arrays.
  GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)gta_computed_value_array_create(lhs->elements->count + rhs->elements->count, context);
  if (!result) {
    return gta_computed_value_error_out_of_memory;
  }

  // Copy the elements from the two arrays into the new array.
  memcpy(result->elements->data, lhs->elements->data, sizeof(GTA_TypeX_Union) * lhs->elements->count);
  memcpy(result->elements->data + lhs->elements->count, rhs->elements->data, sizeof(GTA_TypeX_Union) * rhs->elements->count);

  // Update the count of the new array.
  result->elements->count = lhs->elements->count + rhs->elements->count;

  return (GTA_Computed_Value *)result;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_array_multiply(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_MAYBE_UNUSED(bool is_assignment), GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(self));
  GTA_Computed_Value_Array * lhs = (GTA_Computed_Value_Array *)self;

  if (!self_is_lhs || !GTA_COMPUTED_VALUE_IS_INTEGER(other)) {
    return gta_computed_value_error_not_supported;
  }
  GTA_Computed_Value_Integer * rhs = (GTA_Computed_Value_Integer *)other;

  if (rhs->value == 0) {
    // If the right-hand side is zero, then the result is an empty array.
    GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)gta_computed_value_array_create(0, context);
    if (!result) {
      return gta_computed_value_error_out_of_memory;
    }
    return (GTA_Computed_Value *)result;
  }

  if (rhs->value < 0) {
    return gta_computed_value_error_not_supported;
  }

  // Create a new array that will be a repeated concatenation of the source array.
  assert(lhs->elements);
  assert(lhs->elements->count ? (bool)lhs->elements->data : true);
  GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)gta_computed_value_array_create(lhs->elements->count * rhs->value, context);
  if (!result) {
    return gta_computed_value_error_out_of_memory;
  }

  // Note: Similar to the `gta_computed_value_array_deep_copy` function, all
  // objects created in this function are tracked by the garbage collection,
  // and we will not explicitly delete anything if an error occurs.

  for (size_t i = 0; i < (size_t)rhs->value; i++) {
    for (size_t j = 0; j < lhs->elements->count; j++) {
      GTA_Computed_Value * element = GTA_TYPEX_P(lhs->elements->data[j]);
      GTA_Computed_Value * element_copy = gta_computed_value_deep_copy(element, context);
      if (!element_copy) {
        return gta_computed_value_error_out_of_memory;
      }
      result->elements->data[(i * lhs->elements->count) + j] = GTA_TYPEX_MAKE_P(element_copy);
    }
  }

  return (GTA_Computed_Value *)result;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_array_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(self));
  GTA_Computed_Value_Array * lhs = (GTA_Computed_Value_Array *)self;

  if (!self_is_lhs || !GTA_COMPUTED_VALUE_IS_ARRAY(other)) {
    return gta_computed_value_error_not_supported;
  }
  GTA_Computed_Value_Array * rhs = (GTA_Computed_Value_Array *)other;

  if (lhs->elements->count != rhs->elements->count) {
    return (GTA_Computed_Value *)gta_computed_value_boolean_false;
  }

  assert(lhs->elements);
  assert(lhs->elements->count ? (bool)lhs->elements->data : true);
  assert(rhs->elements);
  assert(rhs->elements->count ? (bool)rhs->elements->data : true);

  for (size_t i = 0; i < lhs->elements->count; i++) {
    GTA_Computed_Value * lhs_element = GTA_TYPEX_P(lhs->elements->data[i]);
    GTA_Computed_Value * rhs_element = GTA_TYPEX_P(rhs->elements->data[i]);
    GTA_Computed_Value * result = gta_computed_value_equal(lhs_element, rhs_element, true, context);
    if (!result) {
      return gta_computed_value_error_out_of_memory;
    }
    if (!GTA_COMPUTED_VALUE_IS_BOOLEAN(result) || !((GTA_Computed_Value_Boolean *)result)->value) {
      return (GTA_Computed_Value *)gta_computed_value_boolean_false;
    }
  }

  return (GTA_Computed_Value *)gta_computed_value_boolean_true;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_array_not_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(self));
  GTA_Computed_Value_Array * lhs = (GTA_Computed_Value_Array *)self;

  if (!self_is_lhs || !GTA_COMPUTED_VALUE_IS_ARRAY(other)) {
    return gta_computed_value_error_not_supported;
  }
  GTA_Computed_Value_Array * rhs = (GTA_Computed_Value_Array *)other;

  assert(lhs->elements);
  assert(lhs->elements->count ? (bool)lhs->elements->data : true);
  assert(rhs->elements);
  assert(rhs->elements->count ? (bool)rhs->elements->data : true);

  if (lhs->elements->count != rhs->elements->count) {
    return (GTA_Computed_Value *)gta_computed_value_boolean_false;
  }

  for (size_t i = 0; i < lhs->elements->count; i++) {
    GTA_Computed_Value * lhs_element = GTA_TYPEX_P(lhs->elements->data[i]);
    GTA_Computed_Value * rhs_element = GTA_TYPEX_P(rhs->elements->data[i]);
    GTA_Computed_Value * result = gta_computed_value_not_equal(lhs_element, rhs_element, true, context);
    if (!result) {
      return gta_computed_value_error_out_of_memory;
    }
    if (!GTA_COMPUTED_VALUE_IS_BOOLEAN(result) || !((GTA_Computed_Value_Boolean *)result)->value) {
      return (GTA_Computed_Value *)gta_computed_value_boolean_false;
    }
  }

  return (GTA_Computed_Value *)gta_computed_value_boolean_true;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_array_append(GTA_Computed_Value_Array * self, GTA_Computed_Value * value, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  assert(self);
  if (!GTA_VECTORX_APPEND(self->elements, GTA_TYPEX_MAKE_P(value))) {
    return gta_computed_value_error_out_of_memory;
  }
  value->is_temporary = false;
  return (GTA_Computed_Value *)self;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_array_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(self));
  GTA_Computed_Value_Integer * integer = (GTA_Computed_Value_Integer *)index;

  if (!GTA_COMPUTED_VALUE_IS_INTEGER(index)) {
    return gta_computed_value_error_invalid_index;
  }
  GTA_Computed_Value_Array * array = (GTA_Computed_Value_Array *)self;

  assert(array->elements);
  assert(array->elements->count ? (bool)array->elements->data : true);

  GTA_Integer normalized_index = integer->value >= 0
    ? integer->value
    : (GTA_Integer)array->elements->count + integer->value;

  if ((normalized_index >= (GTA_Integer)array->elements->count) || (normalized_index < 0)) {
    return gta_computed_value_null;
  }

  return GTA_TYPEX_P(array->elements->data[normalized_index]);
}


// Helper function to correct start and end values of a slice.
static GTA_Integer GTA_CALL correct_bounds(GTA_Integer value, GTA_Integer boundary, GTA_Integer step) {
  GTA_Integer interval = (boundary - value) / step;
  GTA_Integer roundup = (interval * step) == (boundary - value) ? 0 : 1;
  return value + ((interval + roundup) * step);
}


GTA_Computed_Value * GTA_CALL gta_computed_value_array_slice(GTA_Computed_Value * self, GTA_Computed_Value * start, GTA_Computed_Value * end, GTA_Computed_Value * step, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(self));
  GTA_Computed_Value_Array * array = (GTA_Computed_Value_Array *)self;

  assert(array->elements);
  assert(array->elements->count ? (bool)array->elements->data : true);

  // First, validate the step value.  If null, it will default to 1.
  if (!(GTA_COMPUTED_VALUE_IS_INTEGER(step) || GTA_COMPUTED_VALUE_IS_NULL(step))) {
    return gta_computed_value_error_invalid_index;
  }
  GTA_Integer step_value = GTA_COMPUTED_VALUE_IS_NULL(step)
    ? 1
    : ((GTA_Computed_Value_Integer *)step)->value;
  if (step_value == 0) {
    return gta_computed_value_error_invalid_index;
  }

  // Convert the start value to an integer.
  if (!(GTA_COMPUTED_VALUE_IS_INTEGER(start) || GTA_COMPUTED_VALUE_IS_NULL(start))) {
    return gta_computed_value_error_invalid_index;
  }
  GTA_Integer start_value = GTA_COMPUTED_VALUE_IS_NULL(start)
    ? step_value > 0
      ? 0
      : (GTA_Integer)array->elements->count - 1
    : ((GTA_Computed_Value_Integer *)start)->value >= 0
      ? ((GTA_Computed_Value_Integer *)start)->value
      : ((GTA_Computed_Value_Integer *)start)->value + (GTA_Integer)array->elements->count;

  // Convert the end value to an integer.
  if (!(GTA_COMPUTED_VALUE_IS_INTEGER(end) || GTA_COMPUTED_VALUE_IS_NULL(end))) {
    return gta_computed_value_error_invalid_index;
  }
  GTA_Integer end_value = GTA_COMPUTED_VALUE_IS_NULL(end)
    ? step_value > 0
      ? (GTA_Integer)array->elements->count
      : -1
    : ((GTA_Computed_Value_Integer *)end)->value >= 0
      ? ((GTA_Computed_Value_Integer *)end)->value
      : ((GTA_Computed_Value_Integer *)end)->value + (GTA_Integer)array->elements->count;

  GTA_Integer array_count = (GTA_Integer)array->elements->count;

  if (step_value > 0) {
    // If the step value is positive, then the start value should be the first
    // eligible value that is greater than or equal to 0.
    if (start_value < 0) {
      start_value = correct_bounds(start_value, 0, step_value);
    }
    // It is possible that the start_value is outside of the bounds of the
    // array (e.g., [1, 2, 3][-4::10]).
    // It is possible that end_value is less than start_value.  If so, then do
    // nothing so that the first sanity check will catch it.  Otherwise, if
    // end_value is greater than the end of the array, then set it to the first
    // eligible value that is after the end of the array.
    if ((end_value > start_value) && (end_value > array_count)) {
      end_value = correct_bounds(start_value, array_count, step_value);
    }
  }
  else {
    // If the step value is negative, then the start value should be the first
    // eligible value that is less than or equal to the end of the array.
    if (start_value > array_count - 1) {
      start_value = correct_bounds(start_value, array_count - 1, step_value);
    }
    // It is possible that the start_value is outside of the bounds of the
    // array (e.g., [1, 2, 3][4::-10]).
    // It is possible that end_value is greater than start_value.  If so, then
    // do nothing so that the first sanity check will catch it.  Otherwise, if
    // end_value is less than the start of the array, then set it to the first
    // eligible value that is before the start of the array.
    if ((end_value < start_value) && (end_value < -1)) {
      end_value = correct_bounds(start_value, -1, step_value);
    }
  }

  // First sanity check.  If the start, end, and step values do not intersect,
  // then the result is an empty array.
  if ((step_value > 0 && start_value >= end_value) || (step_value < 0 && start_value <= end_value)) {
    GTA_Computed_Value * result = gta_computed_value_array_create(0, context);
    if (!result) {
      return gta_computed_value_error_out_of_memory;
    }
    return result;
  }

  // TODO: Sometimes this is one larger than it should be, and sometimes not.
  //   Leaving it as-is for now because in this state it may allocate one item
  //   too large, but it does not allocate too few.
  //   Example of too few when not adding 1: [0,1,2,3,4,5,6,7,8,9,0][-13::3]
  GTA_Integer slice_length = (step_value > 0
    ? ((end_value - start_value) / step_value)
    : ((start_value - end_value) / -step_value)) + 1;

  // Create a new array that will be the slice of the source array.
  GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)gta_computed_value_array_create(slice_length, context);
  if (!result) {
    return gta_computed_value_error_out_of_memory;
  }

  // Build the array.
  size_t new_i = 0;
  for (GTA_Integer i = start_value; step_value > 0 ? i < end_value : i > end_value; i += step_value) {
    GTA_Computed_Value * element_copy = gta_computed_value_deep_copy(GTA_TYPEX_P(array->elements->data[i]), context);
    if (!element_copy) {
      return gta_computed_value_error_out_of_memory;
    }
    result->elements->data[new_i++] = GTA_TYPEX_MAKE_P(element_copy);
    ++result->elements->count;
  }

  return (GTA_Computed_Value *)result;
}


/*
 * Helper function used as the 'advance' iterator operation.
 */
static void GTA_CALL __advance(GTA_Computed_Value_Iterator * self) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ITERATOR(self));
  GTA_Computed_Value_Array * array = (GTA_Computed_Value_Array *)self->collection;

  assert(array->elements);
  assert(array->elements->count ? (bool)array->elements->data : true);

  self->value = (self->index >= (GTA_Integer)array->elements->count)
    ? gta_computed_value_error_iterator_end
    : (GTA_Computed_Value *)GTA_TYPEX_P(array->elements->data[self->index]);
}


GTA_Computed_Value * GTA_CALL gta_computed_value_array_iterator_get(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(self));
  GTA_Computed_Value * iterator = gta_computed_value_iterator_create(self, context);

  if (!iterator || GTA_COMPUTED_VALUE_IS_ERROR(iterator)) {
    return iterator ? iterator : gta_computed_value_error_out_of_memory;
  }

  ((GTA_Computed_Value_Iterator *)iterator)->advance = __advance;

  return iterator;
}


GTA_Computed_Value * GTA_CALL array_size(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(self));
  GTA_Computed_Value_Array * array = (GTA_Computed_Value_Array *)self;
  return (GTA_Computed_Value *)gta_computed_value_integer_create(array->elements->count, context);
}
