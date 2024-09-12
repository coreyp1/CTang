
#include <assert.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValueError.h>

GTA_Computed_Value_VTable gta_computed_value_error_vtable = {
  .name = "Error",
  .destroy = gta_computed_value_null_destroy,
  .destroy_in_place = gta_computed_value_null_destroy_in_place,
  .deep_copy = gta_computed_value_null_deep_copy,
  .to_string = gta_computed_value_error_to_string,
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
  .cast = gta_computed_value_cast_not_implemented,
  .call = gta_computed_value_call_not_supported,
  .attributes = NULL,
  .attributes_count = 0,
};


static GTA_Computed_Value_Error gta_computed_value_error_not_implemented_singleton = {
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
  .message = "Not implemented",
};


static GTA_Computed_Value_Error gta_computed_value_error_out_of_memory_singleton = {
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
  .message = "Out of memory",
};


static GTA_Computed_Value_Error gta_computed_value_error_invalid_bytecode_singleton = {
  .base = {
    .vtable = &gta_computed_value_error_vtable,
    .context = 0,
    .is_true = false,
    .is_error = true,
    .is_temporary = false,
    .requires_deep_copy = false,
    .is_singleton = false,
    .is_a_reference = false,
  },
  .message = "Invalid opcode",
};


static GTA_Computed_Value_Error gta_computed_value_error_not_supported_singleton = {
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
  .message = "Not supported",
};


static GTA_Computed_Value_Error gta_computed_value_error_divide_by_zero_singleton = {
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
  .message = "Divide by zero",
};


static GTA_Computed_Value_Error gta_computed_value_error_modulo_by_zero_singleton = {
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
  .message = "Modulo by zero",
};


static GTA_Computed_Value_Error gta_computed_value_error_invalid_index_singleton = {
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
  .message = "Invalid index",
};


static GTA_Computed_Value_Error gta_computed_value_error_invalid_function_call_singleton = {
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
  .message = "Invalid function call",
};


static GTA_Computed_Value_Error gta_computed_value_error_argument_count_mismatch_singleton = {
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
  .message = "Argument Count Mismatch",
};


static GTA_Computed_Value_Error gta_computed_value_error_global_rng_seed_not_changeable_singleton = {
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
  .message = "Cannot change the seed of the global random number generator",
};


GTA_Computed_Value * gta_computed_value_error_not_implemented = (GTA_Computed_Value *)&gta_computed_value_error_not_implemented_singleton;
GTA_Computed_Value * gta_computed_value_error_out_of_memory = (GTA_Computed_Value *)&gta_computed_value_error_out_of_memory_singleton;
GTA_Computed_Value * gta_computed_value_error_invalid_bytecode = (GTA_Computed_Value *)&gta_computed_value_error_invalid_bytecode_singleton;
GTA_Computed_Value * gta_computed_value_error_not_supported = (GTA_Computed_Value *)&gta_computed_value_error_not_supported_singleton;
GTA_Computed_Value * gta_computed_value_error_divide_by_zero = (GTA_Computed_Value *)&gta_computed_value_error_divide_by_zero_singleton;
GTA_Computed_Value * gta_computed_value_error_modulo_by_zero = (GTA_Computed_Value *)&gta_computed_value_error_modulo_by_zero_singleton;
GTA_Computed_Value * gta_computed_value_error_invalid_index = (GTA_Computed_Value *)&gta_computed_value_error_invalid_index_singleton;
GTA_Computed_Value * gta_computed_value_error_invalid_function_call = (GTA_Computed_Value *)&gta_computed_value_error_invalid_function_call_singleton;
GTA_Computed_Value * gta_computed_value_error_argument_count_mismatch = (GTA_Computed_Value *)&gta_computed_value_error_argument_count_mismatch_singleton;
GTA_Computed_Value * gta_computed_value_error_global_rng_seed_not_changeable = (GTA_Computed_Value *)&gta_computed_value_error_global_rng_seed_not_changeable_singleton;


char * gta_computed_value_error_to_string(GTA_Computed_Value * self) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ERROR(self));
  GTA_Computed_Value_Error * error = (GTA_Computed_Value_Error *)self;

  assert(error->message);
  size_t length = strlen(error->message);
  char * string = gcu_malloc(length + 1 + 7);
  if (!string) {
    return 0;
  }
  strcpy(string, "Error: ");
  strcpy(string + 7, error->message);
  return string;
}
