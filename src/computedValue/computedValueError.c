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
#include <string.h>
#include <ghoti.io/cutil/memory.h>
#include <ghoti.io/tang/macros.h>
#include <ghoti.io/tang/computedValue/computedValueError.h>

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


// Identical to the error vtable except that these values print as themselves.
// An ordinary error prints as nothing, which is right when the operation is
// what failed. It is wrong when the value is what failed, because the choice
// there is not between a marker and silence - it is between a marker and a
// number that is not true.
GTA_Computed_Value_VTable gta_computed_value_error_marker_vtable = {
  .name = "Error",
  .destroy = gta_computed_value_null_destroy,
  .destroy_in_place = gta_computed_value_null_destroy_in_place,
  .deep_copy = gta_computed_value_null_deep_copy,
  .to_string = gta_computed_value_error_marker_to_string,
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


static GTA_Computed_Value_Error gta_computed_value_error_integer_too_large_singleton = {
  .base = {
    .vtable = &gta_computed_value_error_marker_vtable,
    .context = 0,
    .is_true = false,
    .is_error = true,
    .is_temporary = false,
    .requires_deep_copy = false,
    .is_singleton = true,
    .is_a_reference = false,
  },
  .message = "[INTEGER TOO LARGE]",
};


static GTA_Computed_Value_Error gta_computed_value_error_integer_too_small_singleton = {
  .base = {
    .vtable = &gta_computed_value_error_marker_vtable,
    .context = 0,
    .is_true = false,
    .is_error = true,
    .is_temporary = false,
    .requires_deep_copy = false,
    .is_singleton = true,
    .is_a_reference = false,
  },
  .message = "[INTEGER TOO SMALL]",
};


static GTA_Computed_Value_Error gta_computed_value_error_not_a_number_singleton = {
  .base = {
    .vtable = &gta_computed_value_error_marker_vtable,
    .context = 0,
    .is_true = false,
    .is_error = true,
    .is_temporary = false,
    .requires_deep_copy = false,
    .is_singleton = true,
    .is_a_reference = false,
  },
  .message = "[NOT A NUMBER]",
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
GTA_Computed_Value * gta_computed_value_error_integer_too_large = (GTA_Computed_Value *)&gta_computed_value_error_integer_too_large_singleton;
GTA_Computed_Value * gta_computed_value_error_integer_too_small = (GTA_Computed_Value *)&gta_computed_value_error_integer_too_small_singleton;
GTA_Computed_Value * gta_computed_value_error_not_a_number = (GTA_Computed_Value *)&gta_computed_value_error_not_a_number_singleton;


char * GTA_CALL gta_computed_value_error_marker_to_string(GTA_Computed_Value * self) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_ERROR(self));
  GTA_Computed_Value_Error * error = (GTA_Computed_Value_Error *)self;

  // No "Error: " prefix: the message is the whole answer, and the point of
  // these values is that what gets printed reads as a marker rather than as a
  // number that happens to be wrong.
  assert(error->message);
  size_t length = strlen(error->message);
  char * string = gcu_malloc(length + 1);
  if (!string) {
    return 0;
  }
  memcpy(string, error->message, length + 1);
  return string;
}


char * GTA_CALL gta_computed_value_error_to_string(GTA_Computed_Value * self) {
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
