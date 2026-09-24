/*
 * SPDX-License-Identifier: LGPL-3.0-only
 *
 * Copyright (C) 2024-2026 Corey Pennycuff
 *
 * This file is part of Ghoti.io Tang.
 *
 * Ghoti.io Tang is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Ghoti.io Tang is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ghoti.io/cutil/memory.h>
#include <ghoti.io/tang/macros.h>
#include <ghoti.io/tang/computedValue/computedValueArray.h>
#include <ghoti.io/tang/unicodeString.h>
#include <ghoti.io/tang/computedValue/computedValueBoolean.h>
#include <ghoti.io/tang/computedValue/computedValueString.h>
#include <ghoti.io/tang/computedValue/computedValueError.h>
#include <ghoti.io/tang/computedValue/computedValueInteger.h>
#include <ghoti.io/tang/computedValue/computedValueIterator.h>
#include <ghoti.io/tang/program/executionContext.h>


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
  .cast = gta_computed_value_array_cast,
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

  // Failure is NULL, not the out-of-memory value.  Every caller tests the
  // result with `if (!result)` and the x86-64 emitters test it with `test rax,
  // rax`, so returning a value said "this worked" to all of them - and what
  // they then dereferenced as an array was the error singleton, whose
  // `elements` field is whatever lies at that offset in a much smaller object.
  // Returning the error here and reporting it there is also duplication: each
  // caller already answers a failure with the same singleton.
  GTA_Computed_Value_Array * self = gcu_malloc(sizeof(GTA_Computed_Value_Array));
  if (self == NULL) {
    return NULL;
  }
  if (!gta_computed_value_array_create_in_place(self, size, context)) {
    gcu_free(self);
    return NULL;
  }
  if (context) {
    // Attempt to add the pointer to the context's garbage collection list.
    if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
      gta_computed_value_array_destroy_in_place(&self->base);
      return NULL;
    }
  }
  return (GTA_Computed_Value *)self;
}


bool GTA_CALL gta_computed_value_array_create_in_place(GTA_Computed_Value_Array * self, size_t size, GTA_Execution_Context * context) {
  *self = (GTA_Computed_Value_Array) {
    .base = {
      .vtable = &gta_computed_value_array_vtable,
      .context = context,
      // An empty array is false and a populated one is true.  is_true is a
      // stored byte that both engines read straight out of the object, so it
      // cannot be derived on demand; every caller passes the number of
      // elements the array is about to be filled with, and the two places
      // that can later grow an array from empty set it again.
      .is_true = size > 0,
      .is_error = false,
      .is_temporary = true,
      .requires_deep_copy = false,
      .is_singleton = false,
      .is_a_reference = false,
    },
    .elements = GTA_VECTORX_CREATE(size),
  };
  if (!self->elements) {
    return false;
  }
  // The vector's reserve is best-effort: cutil's create leaves `data` null and
  // `capacity` zero when the allocation it asked for was refused, and still
  // reports success, so nothing downstream can tell a reserved vector from an
  // unreserved one.  Every caller here passes the number of elements it is
  // about to write directly into `data`, so the room has to be real.  Without
  // this, `[[], [], 0] * 100000000` wanted 2.4 GB, was refused, and wrote the
  // first element through a null pointer.
  if (size && (self->elements->capacity < size)) {
    GTA_VECTORX_DESTROY(self->elements);
    self->elements = NULL;
    return false;
  }
  return true;
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

GTA_Computed_Value * GTA_CALL gta_computed_value_array_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ARRAY(self));

  if (type == &gta_computed_value_boolean_vtable) {
    // A container is true when it holds something; see is_true.
    return (GTA_Computed_Value *)(self->is_true
      ? gta_computed_value_boolean_true
      : gta_computed_value_boolean_false);
  }

  if (type == &gta_computed_value_string_vtable) {
    char * str = gta_computed_value_array_to_string(self);
    if (!str) {
      return gta_computed_value_error_out_of_memory;
    }
    // The rendering is this library's own text, not anything the template
    // was given, so it is trusted.
    GTA_Unicode_String * unicode_str = gta_unicode_string_create(str, strlen(str), GTA_UNICODE_STRING_TYPE_TRUSTED);
    gcu_free(str);
    if (!unicode_str) {
      return gta_computed_value_error_out_of_memory;
    }
    GTA_Computed_Value * result = (GTA_Computed_Value *)gta_computed_value_string_create(unicode_str, true, context);
    if (!result) {
      gta_unicode_string_destroy(unicode_str);
      return gta_computed_value_error_out_of_memory;
    }
    return result;
  }

  // There is no number a container could sensibly be.
  return gta_computed_value_error_not_supported;
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
    for (size_t i = array->elements->count; i < new_size; i++) {
      // Populate the "new" elements with null, the last one included.
      //
      // The loop used to stop one short, on the reasoning that the slot being
      // assigned is written immediately below - but not immediately enough.
      // The count is raised here, so between this line and that write the
      // array claims an element it does not have, and the deep copy of the
      // value being assigned runs in between.  When the value is the array
      // itself, that copy walks the slot that has not been written yet:
      // `a = [1]; a[5] = a;` read uninitialised heap and dereferenced it.
      // The same window is open on the out-of-memory path, which returns with
      // the array grown and the last slot never written at all.
      array->elements->data[i] = GTA_TYPEX_MAKE_P(gta_computed_value_null);
    }
    array->elements->count = new_size;
    array->base.is_true = true;
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
  //
  // An empty array's data pointer is null, and memcpy() is undefined when
  // either pointer is null even for a length of zero - which `[] + []` is.
  // UBSan says so; nothing else does.
  if (lhs->elements->count) {
    memcpy(result->elements->data, lhs->elements->data, sizeof(GTA_TypeX_Union) * lhs->elements->count);
  }
  if (rhs->elements->count) {
    memcpy(result->elements->data + lhs->elements->count, rhs->elements->data, sizeof(GTA_TypeX_Union) * rhs->elements->count);
  }

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

  // How many elements the result has, if it can have that many.  This product
  // used to be taken unchecked, so `[0, 0, 0] * 9223372036854775807` wrapped to
  // 9223372036854775805 and the allocator was asked for that many pointers.
  // The bound is what a vector of them can address, not what a size_t can
  // hold, because the vector multiplies by the element size again.
  size_t repetitions = (size_t)rhs->value;
  if (lhs->elements->count > (SIZE_MAX / sizeof(GTA_TypeX_Union)) / repetitions) {
    return gta_computed_value_error_out_of_memory;
  }
  size_t element_count = lhs->elements->count * repetitions;

  GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)gta_computed_value_array_create(element_count, context);
  if (!result) {
    return gta_computed_value_error_out_of_memory;
  }
  if (!element_count) {
    // Repeating an empty array is an empty array, however many times.  Said
    // here rather than left to the loops below, which would otherwise spin
    // `rhs->value` times over an empty body - and `[] * 9223372036854775807`
    // would not come back.
    return (GTA_Computed_Value *)result;
  }
  // The elements are written into the vector's storage directly below, so the
  // count has to be set here.  It was not, and `[1, 2] * 3` was `[]`: the
  // result had room for six elements, six elements were written into that
  // room, and the vector still said it held none.
  result->elements->count = element_count;

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
  self->base.is_true = true;
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

  // A start past the far end selects nothing, whichever way the step walks
  // (4.9).  What used to follow stepped in towards the boundary rather than
  // clamping to it, and a start already past the boundary sent the end one
  // whole step the wrong way: `[0, 1][10:65535:256]` came out with an end of
  // 266 against a start of 10, which passed the intersection check below, and
  // the build loop read element 10 of a two-element array.  A step of 1 hid
  // it, landing exactly on the boundary.  The clamps below cannot reproduce
  // it, but this says outright what the answer is.
  if (((step_value > 0) && (start_value >= array_count))
    || ((step_value < 0) && (start_value < 0))) {
    GTA_Computed_Value * empty = gta_computed_value_array_create(0, context);
    return empty ? empty : gta_computed_value_error_out_of_memory;
  }

  // Both ends are clamped to the container: 4.9 says the semantics are
  // Python's, and Python clamps.  This stepped in from outside instead,
  // keeping the start's phase, so `[0, 1, 2][-34::3]` selected element 2,
  // where 4.9's own worked example says a start that far back "starts at
  // the beginning".  Two of this suite's own slice cases had been written
  // down from the code, so the tests agreed with it.
  if (step_value > 0) {
    if (start_value < 0) {
      start_value = 0;
    }
    if (end_value > array_count) {
      end_value = array_count;
    }
  }
  else {
    if (start_value > array_count - 1) {
      start_value = array_count - 1;
    }
    if (end_value < -1) {
      end_value = -1;
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
  // In GTA_UInteger because neither half is safe signed: the span can be
  // wider than the signed type, and `-step_value` is itself undefined for the
  // most negative step.  Both ends are array-sized after the clamp above, so
  // the quotient is too.
  GTA_UInteger span = step_value > 0
    ? (GTA_UInteger)end_value - (GTA_UInteger)start_value
    : (GTA_UInteger)start_value - (GTA_UInteger)end_value;
  GTA_UInteger step_magnitude = step_value < 0
    ? (GTA_UInteger)0 - (GTA_UInteger)step_value
    : (GTA_UInteger)step_value;
  GTA_Integer slice_length = (GTA_Integer)(span / step_magnitude) + 1;

  // Create a new array that will be the slice of the source array.
  GTA_Computed_Value_Array * result = (GTA_Computed_Value_Array *)gta_computed_value_array_create(slice_length, context);
  if (!result) {
    return gta_computed_value_error_out_of_memory;
  }

  // Build the array.
  size_t new_i = 0;
  for (GTA_Integer i = start_value; step_value > 0 ? i < end_value : i > end_value; ) {
    GTA_Computed_Value * element_copy = gta_computed_value_deep_copy(GTA_TYPEX_P(array->elements->data[i]), context);
    if (!element_copy) {
      return gta_computed_value_error_out_of_memory;
    }
    result->elements->data[new_i++] = GTA_TYPEX_MAKE_P(element_copy);
    ++result->elements->count;
    // The last index plus the step need not be representable -
    // `[1, 2, 3][1:3:9223372036854775807]` is enough - and signed overflow is
    // undefined behaviour, not a wrap the condition above would then catch.
    // There is no next index in that case, so this is the end of the slice.
    if (step_value > 0
      ? (i > GTA_INTEGER_MAX - step_value)
      : (i < GTA_INTEGER_MIN - step_value)) {
      break;
    }
    i += step_value;
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
