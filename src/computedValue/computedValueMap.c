
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <cutil/hash.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueMap.h>
#include <tang/computedValue/computedValueString.h>
#include <tang/program/executionContext.h>


GTA_Computed_Value_VTable gta_computed_value_map_vtable = {
  .name = "Iterator",
  .destroy = gta_computed_value_map_destroy,
  .destroy_in_place = gta_computed_value_map_destroy_in_place,
  .deep_copy = gta_computed_value_map_deep_copy,
  .to_string = gta_computed_value_null_to_string,
  .print = gta_computed_value_generic_print_from_to_string,
  .assign_index = gta_computed_value_map_assign_index,
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
  .index = gta_computed_value_map_index,
  .slice = gta_computed_value_slice_not_supported,
  .iterator_get = gta_computed_value_iterator_get_not_supported,
  .iterator_next = gta_computed_value_iterator_next_not_supported,
  .cast = gta_computed_value_cast_not_supported,
  .call = gta_computed_value_call_not_supported,
};


static GTA_Computed_Value_Error gta_computed_value_error_map_key_not_found_singleton = {
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
  .message = "Map Key Not Found",
};

GTA_Computed_Value * gta_computed_value_error_map_key_not_found = (GTA_Computed_Value *)&gta_computed_value_error_map_key_not_found_singleton;


static GTA_Computed_Value_Error gta_computed_value_error_map_key_not_string_singleton = {
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
  .message = "Map Key Is Not A String",
};

GTA_Computed_Value * gta_computed_value_error_map_key_not_string = (GTA_Computed_Value *)&gta_computed_value_error_map_key_not_string_singleton;

GTA_Computed_Value * GTA_CALL gta_computed_value_map_create(size_t size, GTA_Execution_Context * context) {
  GTA_Computed_Value_Map * self = gcu_malloc(sizeof(GTA_Computed_Value_Map));
  if (!self) {
    return NULL;
  }
  if (!gta_computed_value_map_create_in_place(self, size, context)) {
    gcu_free(self);
    return NULL;
  }
  // Attempt to add the pointer to the context's garbage collection list.
  if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
    gta_computed_value_map_destroy_in_place(&self->base);
    return gta_computed_value_error_out_of_memory;
  }
  return (GTA_Computed_Value *)self;
}


bool GTA_CALL gta_computed_value_map_create_in_place(GTA_Computed_Value_Map * self, size_t size, GTA_Execution_Context * context) {
  assert (self);

  GTA_HashX * key_hash = GTA_HASHX_CREATE(size);
  if (!key_hash) {
    goto KEY_HASH_CREATE_FAILED;
  }
  GTA_HashX * value_hash = GTA_HASHX_CREATE(size);
  if (!value_hash) {
    goto VALUE_HASH_CREATE_FAILED;
  }

  *self = (GTA_Computed_Value_Map){
    .base = {
      .vtable = &gta_computed_value_map_vtable,
      .context = context,
      .is_true = false,
      .is_error = false,
      .is_temporary = false,
      .requires_deep_copy = false,
      .is_singleton = false,
      .is_a_reference = false,
    },
    .key_hash = key_hash,
    .value_hash = value_hash,
  };
  return true;

VALUE_HASH_CREATE_FAILED:
  GTA_HASHX_DESTROY(key_hash);
KEY_HASH_CREATE_FAILED:
  return false;
}


void GTA_CALL gta_computed_value_map_destroy(GTA_Computed_Value * self) {
  assert(self);
  gta_computed_value_map_destroy_in_place(self);
  gcu_free(self);
}


void GTA_CALL gta_computed_value_map_destroy_in_place(GTA_Computed_Value * self) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_MAP(self));
  GTA_Computed_Value_Map * map = (GTA_Computed_Value_Map *)self;

  GTA_HASHX_DESTROY(map->key_hash);
  GTA_HASHX_DESTROY(map->value_hash);
}


GTA_Computed_Value * GTA_CALL gta_computed_value_map_deep_copy(GTA_Computed_Value * value, GTA_Execution_Context * context) {
  GTA_Computed_Value_Map * map = (GTA_Computed_Value_Map *)value;

  // Create a new map into which values can be copied.
  GTA_Computed_Value_Map * map_copy = (GTA_Computed_Value_Map *)gta_computed_value_map_create(map->key_hash->entries, context);
  if (!map_copy || !GTA_COMPUTED_VALUE_IS_MAP(map_copy)) {
    return gta_computed_value_error_out_of_memory;
  }

  // Copy the keys and values from the original map to the new map.
  GTA_HashX_Iterator key_iterator = GTA_HASHX_ITERATOR_GET(map->key_hash);
  GTA_HashX_Iterator value_iterator = GTA_HASHX_ITERATOR_GET(map->value_hash);

  while (key_iterator.exists && value_iterator.exists) {
    GTA_Computed_Value * key_copy = gta_computed_value_deep_copy((GTA_Computed_Value *)GTA_TYPEX_P(key_iterator.value), context);
    GTA_Computed_Value * value_copy = gta_computed_value_deep_copy((GTA_Computed_Value *)GTA_TYPEX_P(value_iterator.value), context);

    if (!key_copy
      || !value_copy
      || !GTA_HASHX_SET(map_copy->key_hash, key_iterator.hash, GTA_TYPEX_MAKE_P(key_copy))
      || !GTA_HASHX_SET(map_copy->value_hash, value_iterator.hash, GTA_TYPEX_MAKE_P(value_copy))) {
      gta_computed_value_map_destroy((GTA_Computed_Value *)map_copy);
      return gta_computed_value_error_out_of_memory;
    }

    // Make sure that the key and value objects are not marked temporary.
    key_copy->is_temporary = false;
    value_copy->is_temporary = false;

    // Advance the iterators.
    key_iterator = GTA_HASHX_ITERATOR_NEXT(key_iterator);
    value_iterator = GTA_HASHX_ITERATOR_NEXT(value_iterator);
  }

  // Sanity check: make sure that the hash tables have the same number of entries.
  if ((map->key_hash->entries != map_copy->key_hash->entries)
    || (map->value_hash->entries != map_copy->value_hash->entries)) {
    gta_computed_value_map_destroy((GTA_Computed_Value *)map_copy);
    return gta_computed_value_error_out_of_memory;
  }

  return (GTA_Computed_Value *)map_copy;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_map_assign_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Computed_Value * other, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_MAP(self));
  return gta_computed_value_map_set_key_val((GTA_Computed_Value_Map *)self, index, other);
}


GTA_Computed_Value * GTA_CALL gta_computed_value_map_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_MAP(self));
  GTA_Computed_Value_Map * map = (GTA_Computed_Value_Map *)self;

  if (!GTA_COMPUTED_VALUE_IS_STRING(index)) {
    return gta_computed_value_error_map_key_not_string;
  }
  GTA_Computed_Value_String * key = (GTA_Computed_Value_String *)index;

  GTA_Integer key_hash = gcu_string_hash_64(key->value->buffer, key->value->byte_length);
  GTA_HashX_Value result = GTA_HASHX_GET(map->value_hash, key_hash);
  if (!result.exists) {
    return gta_computed_value_null;
  }
  return (GTA_Computed_Value *)GTA_TYPEX_P(result.value);
}


/*
 * Helper function to expand and add to the provided string.
 */
static bool expand_and_concat(char * * buffer, size_t * len, const char * string) {
  assert(buffer);
  assert(len);
  assert(string);

  size_t string_length = strlen(string);
  char * new_buffer = gcu_realloc(*buffer, *len + string_length + 1);
  if (!new_buffer) {
    return false;
  }
  strcpy(new_buffer + *len, string);
  return new_buffer;
}


char * GTA_CALL gta_computed_value_map_to_string(GTA_Computed_Value * self) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_MAP(self));
  GTA_Computed_Value_Map * map = (GTA_Computed_Value_Map *)self;

  char * buffer = gcu_malloc(3);
  if (!buffer) {
    return NULL;
  }
  buffer[0] = '{';
  buffer[1] = '}';
  buffer[2] = '\0';
  size_t len = 0;
  if (map->key_hash->entries == 0) {
    return buffer;
  }
  buffer[1] = '\n';
  
  GTA_HashX_Iterator key_iterator = GTA_HASHX_ITERATOR_GET(map->key_hash);
  GTA_HashX_Iterator value_iterator = GTA_HASHX_ITERATOR_GET(map->value_hash);
  while (key_iterator.exists && value_iterator.exists) {
    char * key_str = gta_computed_value_to_string((GTA_Computed_Value *)GTA_TYPEX_P(key_iterator.value));
    if (!key_str) {
      gcu_free(buffer);
      return NULL;
    }

    char * value_str = gta_computed_value_to_string((GTA_Computed_Value *)GTA_TYPEX_P(value_iterator.value));
    if (!value_str) {
      gcu_free(key_str);
      gcu_free(buffer);
      return NULL;
    }

    if (!expand_and_concat(&buffer, &len, "  \"")
      || !expand_and_concat(&buffer, &len, key_str)
      || !expand_and_concat(&buffer, &len, "\": ")
      || !expand_and_concat(&buffer, &len, value_str)
      || !expand_and_concat(&buffer, &len, ",\n")) {
      gcu_free(key_str);
      gcu_free(value_str);
      gcu_free(buffer);
      return NULL;
    }

    gcu_free(key_str);
    gcu_free(value_str);
    key_iterator = GTA_HASHX_ITERATOR_NEXT(key_iterator);
    value_iterator = GTA_HASHX_ITERATOR_NEXT(value_iterator);
  }

  if (!expand_and_concat(&buffer, &len, "}")) {
    gcu_free(buffer);
    return NULL;
  }
  return buffer;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_map_get_from_cstring(GTA_Computed_Value_Map * self, const char * key) {
  assert(self);
  assert(key);

  GTA_HashX_Value result = GTA_HASHX_GET(self->value_hash, gcu_string_hash_64(key, strlen(key)));
  if (!result.exists) {
    return gta_computed_value_error_map_key_not_found;
  }

  return (GTA_Computed_Value *)GTA_TYPEX_P(result.value);
}


GTA_Computed_Value * GTA_CALL gta_computed_value_map_set_key_val(GTA_Computed_Value_Map * self, GTA_Computed_Value * key, GTA_Computed_Value * value) {
  if (!GTA_COMPUTED_VALUE_IS_STRING(key)) {
    return gta_computed_value_error_map_key_not_string;
  }

  // Compute the hash of the key.
  GTA_Computed_Value_String * key_string = (GTA_Computed_Value_String *)key;
  GTA_Integer key_hash = gcu_string_hash_64(key_string->value->buffer, key_string->value->byte_length);

  // Insert the key and value into the hash tables.
  if (!GTA_HASHX_SET(self->key_hash, key_hash, GTA_TYPEX_MAKE_P(key))
    || !GTA_HASHX_SET(self->value_hash, key_hash, GTA_TYPEX_MAKE_P(value))) {
    return gta_computed_value_error_out_of_memory;
  }

  // Claim ownership of the key and value.
  key->is_temporary = false;
  value->is_temporary = false;

  return value;
}
