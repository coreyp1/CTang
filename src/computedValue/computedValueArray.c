#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValueArray.h>
#include <tang/computedValue/computedValueBoolean.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueInteger.h>
#include <tang/program/executionContext.h>


GTA_Computed_Value_VTable gta_computed_value_array_vtable = {
  .name = "Array",
  .destroy = gta_computed_value_array_destroy,
  .destroy_in_place = gta_computed_value_array_destroy_in_place,
  .deep_copy = gta_computed_value_array_deep_copy,
  .to_string = gta_computed_value_array_to_string,
  .print = gta_computed_value_generic_print_from_to_string,
  .assign_index = gta_computed_value_assign_index_not_implemented,
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
  .period = gta_computed_value_period_not_implemented,
  .index = gta_computed_value_array_index,
  .slice = gta_computed_value_slice_not_implemented,
  .iterator_get = gta_computed_value_iterator_get_not_implemented,
  .iterator_next = gta_computed_value_iterator_next_not_implemented,
  .cast = gta_computed_value_cast,
  .call = gta_computed_value_call_not_supported,
};


GTA_Computed_Value * GTA_CALL gta_computed_value_array_create(size_t size, GTA_Execution_Context * context) {
  GTA_Computed_Value_Array * self = gcu_malloc(sizeof(GTA_Computed_Value_Array));
  if (self == NULL) {
    return gta_computed_value_error_out_of_memory;
  }
  if (!gta_computed_value_array_create_in_place(self, size, context)) {
    gcu_free(self);
    return gta_computed_value_error_out_of_memory;
  }
  // Attempt to add the pointer to the context's garbage collection list.
  if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
    gta_computed_value_array_destroy_in_place(&self->base);
    return gta_computed_value_error_out_of_memory;
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
  gta_computed_value_array_destroy_in_place(self);
  gcu_free(self);
}


void GTA_CALL gta_computed_value_array_destroy_in_place(GTA_Computed_Value * self) {
  GTA_VECTORX_DESTROY(((GTA_Computed_Value_Array *) self)->elements);
}


GTA_Computed_Value * GTA_CALL gta_computed_value_array_deep_copy(GTA_Computed_Value * value, GTA_Execution_Context * context) {
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
  GTA_Computed_Value_Array * array = (GTA_Computed_Value_Array *)self;

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


GTA_Computed_Value * gta_computed_value_array_add(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_MAYBE_UNUSED(bool is_assignment), GTA_Execution_Context * context) {
  if (!self_is_lhs || !GTA_COMPUTED_VALUE_IS_ARRAY(other)) {
    return gta_computed_value_error_not_supported;
  }

  GTA_Computed_Value_Array * lhs = (GTA_Computed_Value_Array *)self;
  GTA_Computed_Value_Array * rhs = (GTA_Computed_Value_Array *)other;

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


GTA_Computed_Value * gta_computed_value_array_multiply(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_MAYBE_UNUSED(bool is_assignment), GTA_Execution_Context * context) {
  if (!self_is_lhs || !GTA_COMPUTED_VALUE_IS_INTEGER(other)) {
    return gta_computed_value_error_not_supported;
  }

  GTA_Computed_Value_Array * lhs = (GTA_Computed_Value_Array *)self;
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


GTA_Computed_Value * gta_computed_value_array_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context) {
  if (!self_is_lhs || !GTA_COMPUTED_VALUE_IS_ARRAY(other)) {
    return gta_computed_value_error_not_supported;
  }

  GTA_Computed_Value_Array * lhs = (GTA_Computed_Value_Array *)self;
  GTA_Computed_Value_Array * rhs = (GTA_Computed_Value_Array *)other;

  if (lhs->elements->count != rhs->elements->count) {
    return (GTA_Computed_Value *)gta_computed_value_boolean_false;
  }

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


GTA_Computed_Value * gta_computed_value_array_not_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context) {
  if (!self_is_lhs || !GTA_COMPUTED_VALUE_IS_ARRAY(other)) {
    return gta_computed_value_error_not_supported;
  }

  GTA_Computed_Value_Array * lhs = (GTA_Computed_Value_Array *)self;
  GTA_Computed_Value_Array * rhs = (GTA_Computed_Value_Array *)other;

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
  if (!GTA_VECTORX_APPEND(self->elements, GTA_TYPEX_MAKE_P(value))) {
    return gta_computed_value_error_out_of_memory;
  }
  value->is_temporary = false;
  return (GTA_Computed_Value *)self;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_array_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  if (!GTA_COMPUTED_VALUE_IS_INTEGER(index)) {
    return gta_computed_value_error_invalid_index;
  }

  GTA_Computed_Value_Integer * integer = (GTA_Computed_Value_Integer *)index;
  GTA_Computed_Value_Array * array = (GTA_Computed_Value_Array *)self;

  GTA_Integer normalized_index = integer->value >= 0
    ? integer->value
    : (GTA_Integer)array->elements->count + integer->value;

  if ((normalized_index >= (GTA_Integer)array->elements->count) || (normalized_index < 0)) {
    return gta_computed_value_null;
  }

  return GTA_TYPEX_P(array->elements->data[normalized_index]);
}
