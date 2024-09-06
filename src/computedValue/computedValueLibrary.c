
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueLibrary.h>
#include <tang/program/executionContext.h>

GTA_Computed_Value_VTable gta_computed_value_library_vtable = {
  .name = "Library",
  .destroy = gta_computed_value_library_destroy,
  .destroy_in_place = gta_computed_value_library_destroy_in_place,
  .deep_copy = gta_computed_value_library_deep_copy,
  .to_string = gta_computed_value_library_to_string,
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
  .period = gta_computed_value_library_period,
  .index = gta_computed_value_index_not_supported,
  .slice = gta_computed_value_slice_not_supported,
  .iterator_get = gta_computed_value_iterator_get_not_supported,
  .iterator_next = gta_computed_value_iterator_next_not_supported,
  .cast = gta_computed_value_cast_not_supported,
  .call = gta_computed_value_call_not_supported,
  .attributes = NULL,
  .attributes_count = 0,
};


GTA_Computed_Value_Library * GTA_CALL gta_computed_value_library_create(const char * name, GTA_Computed_Value_Library_Attribute_Pair * attributes, GTA_UInteger attribute_count, GTA_Execution_Context * context) {
  GTA_Computed_Value_Library * self = (GTA_Computed_Value_Library *)gcu_malloc(sizeof(GTA_Computed_Value_Library));
  if (!self) {
    return NULL;
  }
  if (!gta_computed_value_library_create_in_place(self, name, attributes, attribute_count, context)) {
    gcu_free(self);
    return NULL;
  }
  if (context) {
    // Attempt to add the pointer to the context's garbage collection list.
    if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
      gta_computed_value_library_destroy((GTA_Computed_Value *)self);
      return NULL;
    }
  }
  return self;
}


bool GTA_CALL gta_computed_value_library_create_in_place(GTA_Computed_Value_Library * self, const char * name, GTA_Computed_Value_Library_Attribute_Pair * attributes, GTA_UInteger attribute_count, GTA_Execution_Context * context) {
  assert(self);
  assert(name);
  assert(attributes || attribute_count == 0);

  size_t name_length = strlen(name);
  if (!name_length) {
    goto INVALID_NAME;
  }
  char * name_copy = gcu_malloc(name_length + 1);
  if (!name_copy) {
    goto NAME_MALLOC_FAILED;
  }
  strcpy(name_copy, name);

  GTA_Computed_Value_Library_Attribute_Pair * attributes_copy = 0;
  if (attribute_count) {
    attributes_copy = (GTA_Computed_Value_Library_Attribute_Pair *)gcu_malloc(attribute_count * sizeof(GTA_Computed_Value_Library_Attribute_Pair));
    if (!attributes_copy) {
      goto ATTRIBUTES_MALLOC_FAILED;
    }
  }
  
  *self = (GTA_Computed_Value_Library) {
    .base = {
      .vtable = &gta_computed_value_library_vtable,
      .context = context,
      .is_true = true,
      .is_error = false,
      .is_temporary = true,
      .requires_deep_copy = false,
      .is_singleton = false,
      .is_a_reference = false,
    },
    .name = name_copy,
    .attributes = attributes_copy,
    .attribute_count = attribute_count,
    .library = NULL,
  };

  if (!gta_computed_value_library_build_library_attributes_hash(self)) {
    goto BUILD_LIBRARY_ATTRIBUTES_HASH_FAILED;
  }
  return true;

BUILD_LIBRARY_ATTRIBUTES_HASH_FAILED:
  if (self->library) {
    gta_library_destroy(self->library);
  }
ATTRIBUTES_MALLOC_FAILED:
  gcu_free(name_copy);
NAME_MALLOC_FAILED:
INVALID_NAME:
  return false;
}


void GTA_CALL gta_computed_value_library_destroy(GTA_Computed_Value * self) {
  if (!self) {
    return;
  }
  gta_computed_value_library_destroy_in_place(self);
  gcu_free(self);
}


void GTA_CALL gta_computed_value_library_destroy_in_place(GTA_Computed_Value * self) {
  if (!self) {
    return;
  }
  assert(GTA_COMPUTED_VALUE_IS_LIBRARY(self));

  GTA_Computed_Value_Library * library = (GTA_Computed_Value_Library *)self;
  gcu_free((char *)library->name);
  if (library->attributes) {
    gcu_free(library->attributes);
  }
}


GTA_Computed_Value * GTA_CALL gta_computed_value_library_deep_copy(GTA_Computed_Value * self, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_LIBRARY(self));

  GTA_Computed_Value_Library * library = (GTA_Computed_Value_Library *)self;

  if (self->requires_deep_copy) {
    return (GTA_Computed_Value *)gta_computed_value_library_create(library->name, library->attributes, library->attribute_count, context);
  }

  return self;
}


char * GTA_CALL gta_computed_value_library_to_string(GTA_Computed_Value * self) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_LIBRARY(self));

  GTA_Computed_Value_Library * library = (GTA_Computed_Value_Library *)self;
  size_t length = strlen(library->name) + 1;
  char * buff = (char *)gcu_malloc(length + 10);
  if (!buff) {
    return NULL;
  }
  sprintf(buff, "Library: %s", library->name);
  return buff;
}


GTA_Computed_Value * gta_computed_value_library_period(GTA_Computed_Value * self, GTA_UInteger identifier_hash, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_LIBRARY(self));

  GTA_Computed_Value_Library * library = (GTA_Computed_Value_Library *)self;
  if (library->library) {
    GTA_Library_Callback callback = gta_library_get_library(library->library, identifier_hash);
    if (callback) {
      return callback(context);
    }
  }
  return gta_computed_value_error_not_implemented;
}


bool gta_computed_value_library_build_library_attributes_hash(GTA_Computed_Value_Library * self) {
  assert(self);

  if (self->attribute_count) {
    assert(self->attributes);

    self->library = gta_library_create();
    if (!self->library) {
      return false;
    }

    for (GTA_UInteger i = 0; i < self->attribute_count; ++i) {
      if (!gta_library_add_library_from_string(self->library, self->attributes[i].name, self->attributes[i].callback)) {
        return false;
      }
    }
  }
  return true;
}
