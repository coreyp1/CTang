
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/computedValue/computedValueBoolean.h>
#include <tang/computedValue/computedValueError.h>
#include <tang/computedValue/computedValueFloat.h>
#include <tang/computedValue/computedValueInteger.h>
#include <tang/computedValue/computedValueString.h>
#include <tang/program/executionContext.h>

GTA_Computed_Value_VTable gta_computed_value_float_vtable = {
  .name = "Float",
  .destroy = gta_computed_value_float_destroy,
  .destroy_in_place = gta_computed_value_float_destroy_in_place,
  .deep_copy = gta_computed_value_float_deep_copy,
  .to_string = gta_computed_value_float_to_string,
  .print = gta_computed_value_generic_print_from_to_string,
  .assign_index = gta_computed_value_assign_index_not_supported,
  .add = gta_computed_value_float_add,
  .subtract = gta_computed_value_float_subtract,
  .multiply = gta_computed_value_float_multiply,
  .divide = gta_computed_value_float_divide,
  .modulo = gta_computed_value_float_modulo,
  .negative = gta_computed_value_float_negative,
  .less_than = gta_computed_value_float_less_than,
  .less_than_equal = gta_computed_value_float_less_than_equal,
  .greater_than = gta_computed_value_float_greater_than,
  .greater_than_equal = gta_computed_value_float_greater_than_equal,
  .equal = gta_computed_value_float_equal,
  .not_equal = gta_computed_value_float_not_equal,
  .period = gta_computed_value_generic_period,
  .index = gta_computed_value_index_not_supported,
  .slice = gta_computed_value_slice_not_supported,
  .iterator_get = gta_computed_value_iterator_get_not_supported,
  .iterator_next = gta_computed_value_iterator_next_not_supported,
  .cast = gta_computed_value_float_cast,
  .call = gta_computed_value_call_not_supported,
  .attributes = NULL,
  .attributes_count = 0,
};


GTA_Computed_Value_Float * GTA_CALL gta_computed_value_float_create(GTA_Float value, GTA_Execution_Context * context) {
  GTA_Computed_Value_Float * self = (GTA_Computed_Value_Float *) gcu_malloc(sizeof(GTA_Computed_Value_Float));
  if (!self) {
    return NULL;
  }
  if(context) {
    // Attempt to add the pointer to the context's garbage collection list.
    if (!GTA_VECTORX_APPEND(context->garbage_collection, GTA_TYPEX_MAKE_P(self))) {
      gcu_free(self);
      return NULL;
    }
  }
  gta_computed_value_float_create_in_place(self, value, context);
  return self;
}


bool GTA_CALL gta_computed_value_float_create_in_place(GTA_Computed_Value_Float * self, GTA_Float value, GTA_Execution_Context * context) {
  assert(self);
  *self = (GTA_Computed_Value_Float) {
    .base = {
      .vtable = &gta_computed_value_float_vtable,
      .context = context,
      .is_true = (bool)value,
      .is_error = false,
      .is_temporary = true,
      .requires_deep_copy = false,
      .is_singleton = false,
      .is_a_reference = false,
    },
    .value = value
  };
  return true;
}


void GTA_CALL gta_computed_value_float_destroy(GTA_Computed_Value * self) {
  assert(self);
  gcu_free(self);
}


void GTA_CALL gta_computed_value_float_destroy_in_place(GTA_MAYBE_UNUSED(GTA_Computed_Value * self)) {}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_deep_copy(GTA_Computed_Value * value, GTA_Execution_Context * context) {
  assert(value);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(value));
  GTA_Computed_Value_Float * float_value = (GTA_Computed_Value_Float *)value;
  return (GTA_Computed_Value *)gta_computed_value_float_create(float_value->value, context);
}


char * GTA_CALL gta_computed_value_float_to_string(GTA_Computed_Value * self) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * float_value = (GTA_Computed_Value_Float *)self;

  char * str = (char *)gcu_malloc(32);
  if (!str) {
    return 0;
  }
  sprintf(str, sizeof(GTA_Integer) == 8 ? "%lf" : "%f", float_value->value);
  for (size_t i = strlen(str); (i > 2) && (str[i-1] == '0'); i--) {
    str[i-1] = '\0';
  }
  return str;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_negative(GTA_Computed_Value * self, GTA_MAYBE_UNUSED(bool is_assignment), GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;

  if (number->base.is_temporary) {
    number->value = -number->value;
    return self;
  }
  return (GTA_Computed_Value *)gta_computed_value_float_create(-number->value, context);
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_add(GTA_Computed_Value * self, GTA_Computed_Value * other, GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;

  if (GTA_COMPUTED_VALUE_IS_FLOAT(other)) {
    GTA_Computed_Value_Float * other_number_float = (GTA_Computed_Value_Float *)other;
    if (number->base.is_temporary) {
      number->value += other_number_float->value;
      number->base.is_true = (bool)number->value;
      return (GTA_Computed_Value *)number;
    }
    if (other_number_float->base.is_temporary) {
      other_number_float->value += number->value;
      other_number_float->base.is_true = (bool)other_number_float->value;
      return (GTA_Computed_Value *)other_number_float;
    }
    return (GTA_Computed_Value *)gta_computed_value_float_create(number->value + other_number_float->value, number->base.context);
  }
  if (GTA_COMPUTED_VALUE_IS_INTEGER(other)) {
    GTA_Computed_Value_Integer * other_number_integer = (GTA_Computed_Value_Integer *)other;
    if (number->base.is_temporary) {
      number->value += (GTA_Float)other_number_integer->value;
      number->base.is_true = (bool)number->value;
      return (GTA_Computed_Value *)number;
    }
    return (GTA_Computed_Value *)gta_computed_value_float_create(number->value + (GTA_Float)other_number_integer->value, context);
  }
  return (GTA_Computed_Value *)gta_computed_value_error_not_supported;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_subtract(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_MAYBE_UNUSED(bool is_assignment), GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;

  if (GTA_COMPUTED_VALUE_IS_FLOAT(other)) {
    GTA_Computed_Value_Float * other_number_float = (GTA_Computed_Value_Float *)other;
    GTA_Float result = self_is_lhs
      ? number->value - other_number_float->value
      : other_number_float->value - number->value;
    if (number->base.is_temporary) {
      number->value = result;
      number->base.is_true = (bool)number->value;
      return (GTA_Computed_Value *)number;
    }
    if (other_number_float->base.is_temporary) {
      other_number_float->value = result;
      other_number_float->base.is_true = (bool)other_number_float->value;
      return (GTA_Computed_Value *)other_number_float;
    }
    return (GTA_Computed_Value *)gta_computed_value_float_create(result, context);
  }
  if (GTA_COMPUTED_VALUE_IS_INTEGER(other)) {
    GTA_Computed_Value_Integer * other_number_integer = (GTA_Computed_Value_Integer *)other;
    GTA_Float result = self_is_lhs
      ? number->value - (GTA_Float)other_number_integer->value
      : (GTA_Float)other_number_integer->value - number->value;
    if (number->base.is_temporary) {
      number->value = result;
      number->base.is_true = (bool)number->value;
      return (GTA_Computed_Value *)number;
    }
    return (GTA_Computed_Value *)gta_computed_value_float_create(result, context);
  }
  return (GTA_Computed_Value *)gta_computed_value_error_not_supported;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_multiply(GTA_Computed_Value * self, GTA_Computed_Value * other, GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;

  if (GTA_COMPUTED_VALUE_IS_FLOAT(other)) {
    GTA_Computed_Value_Float * other_number_float = (GTA_Computed_Value_Float *)other;
    if (number->base.is_temporary) {
      number->value *= other_number_float->value;
      number->base.is_true = (bool)number->value;
      return (GTA_Computed_Value *)number;
    }
    if (other_number_float->base.is_temporary) {
      other_number_float->value *= number->value;
      other_number_float->base.is_true = (bool)other_number_float->value;
      return (GTA_Computed_Value *)other_number_float;
    }
    return (GTA_Computed_Value *)gta_computed_value_float_create(number->value * other_number_float->value, context);
  }
  if (GTA_COMPUTED_VALUE_IS_INTEGER(other)) {
    GTA_Computed_Value_Integer * other_number_integer = (GTA_Computed_Value_Integer *)other;
    if (number->base.is_temporary) {
      number->value *= (GTA_Float)other_number_integer->value;
      number->base.is_true = (bool)number->value;
      return (GTA_Computed_Value *)number;
    }
    return (GTA_Computed_Value *)gta_computed_value_float_create(number->value * (GTA_Float)other_number_integer->value, context);
  }
  return (GTA_Computed_Value *)gta_computed_value_error_not_supported;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_divide(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_MAYBE_UNUSED(bool is_assignment), GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;

  if (GTA_COMPUTED_VALUE_IS_FLOAT(other)) {
    GTA_Computed_Value_Float * other_number_float = (GTA_Computed_Value_Float *)other;
    if ((self_is_lhs && other_number_float->value == 0)
      || (!self_is_lhs && number->value == 0)) {
      return (GTA_Computed_Value *)gta_computed_value_error_divide_by_zero;
    }
    GTA_Float result = self_is_lhs
      ? number->value / other_number_float->value
      : other_number_float->value / number->value;
    if (number->base.is_temporary) {
      number->value = result;
      number->base.is_true = (bool)number->value;
      return (GTA_Computed_Value *)number;
    }
    if (other_number_float->base.is_temporary) {
      other_number_float->value = result;
      other_number_float->base.is_true = (bool)other_number_float->value;
      return (GTA_Computed_Value *)other_number_float;
    }
    return (GTA_Computed_Value *)gta_computed_value_float_create(result, context);
  }
  if (GTA_COMPUTED_VALUE_IS_INTEGER(other)) {
    GTA_Computed_Value_Integer * other_number_integer = (GTA_Computed_Value_Integer *)other;
    if ((self_is_lhs && other_number_integer->value == 0)
      || (!self_is_lhs && number->value == 0)) {
      return (GTA_Computed_Value *)gta_computed_value_error_divide_by_zero;
    }
    GTA_Float result = self_is_lhs
      ? number->value / (GTA_Float)other_number_integer->value
      : (GTA_Float)other_number_integer->value / number->value;
    if (number->base.is_temporary) {
      number->value = result;
      number->base.is_true = (bool)number->value;
      return (GTA_Computed_Value *)number;
    }
    return (GTA_Computed_Value *)gta_computed_value_float_create(result, context);
  }
  return (GTA_Computed_Value *)gta_computed_value_error_not_supported;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_modulo(GTA_MAYBE_UNUSED(GTA_Computed_Value * self), GTA_MAYBE_UNUSED(GTA_Computed_Value * other), GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(bool is_assignment), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_less_than(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;

  if (GTA_COMPUTED_VALUE_IS_FLOAT(other)) {
    GTA_Computed_Value_Float * other_number_float = (GTA_Computed_Value_Float *)other;
    return (GTA_Computed_Value *)((self_is_lhs
      ? number->value < other_number_float->value
      : other_number_float->value < number->value) ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  if (GTA_COMPUTED_VALUE_IS_INTEGER(other)) {
    GTA_Computed_Value_Integer * other_number_integer = (GTA_Computed_Value_Integer *)other;
    return (GTA_Computed_Value *)((self_is_lhs
      ? number->value < (GTA_Float)other_number_integer->value
      : (GTA_Float)other_number_integer->value < number->value) ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_less_than_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;

  if (GTA_COMPUTED_VALUE_IS_FLOAT(other)) {
    GTA_Computed_Value_Float * other_number_float = (GTA_Computed_Value_Float *)other;
    return (GTA_Computed_Value *)((self_is_lhs
      ? number->value <= other_number_float->value
      : other_number_float->value <= number->value) ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  if (GTA_COMPUTED_VALUE_IS_INTEGER(other)) {
    GTA_Computed_Value_Integer * other_number_integer = (GTA_Computed_Value_Integer *)other;
    return (GTA_Computed_Value *)((self_is_lhs
      ? number->value <= (GTA_Float)other_number_integer->value
      : (GTA_Float)other_number_integer->value <= number->value) ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_greater_than(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;

  if (GTA_COMPUTED_VALUE_IS_FLOAT(other)) {
    GTA_Computed_Value_Float * other_number_float = (GTA_Computed_Value_Float *)other;
    return (GTA_Computed_Value *)((self_is_lhs
      ? number->value > other_number_float->value
      : other_number_float->value > number->value) ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  if (GTA_COMPUTED_VALUE_IS_INTEGER(other)) {
    GTA_Computed_Value_Integer * other_number_integer = (GTA_Computed_Value_Integer *)other;
    return (GTA_Computed_Value *)((self_is_lhs
      ? number->value > (GTA_Float)other_number_integer->value
      : (GTA_Float)other_number_integer->value > number->value) ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_greater_than_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;

  if (GTA_COMPUTED_VALUE_IS_FLOAT(other)) {
    GTA_Computed_Value_Float * other_number_float = (GTA_Computed_Value_Float *)other;
    return (GTA_Computed_Value *)((self_is_lhs
      ? number->value >= other_number_float->value
      : other_number_float->value >= number->value) ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  if (GTA_COMPUTED_VALUE_IS_INTEGER(other)) {
    GTA_Computed_Value_Integer * other_number_integer = (GTA_Computed_Value_Integer *)other;
    return (GTA_Computed_Value *)((self_is_lhs
      ? number->value >= (GTA_Float)other_number_integer->value
      : (GTA_Float)other_number_integer->value >= number->value) ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;

  if (GTA_COMPUTED_VALUE_IS_FLOAT(other)) {
    GTA_Computed_Value_Float * other_number_float = (GTA_Computed_Value_Float *)other;
    return (GTA_Computed_Value *)(number->value == other_number_float->value ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  if (GTA_COMPUTED_VALUE_IS_INTEGER(other)) {
    GTA_Computed_Value_Integer * other_number_integer = (GTA_Computed_Value_Integer *)other;
    return (GTA_Computed_Value *)(number->value == (GTA_Float)other_number_integer->value ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_not_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, GTA_MAYBE_UNUSED(bool self_is_lhs), GTA_MAYBE_UNUSED(GTA_Execution_Context * context)) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;

  if (GTA_COMPUTED_VALUE_IS_FLOAT(other)) {
    GTA_Computed_Value_Float * other_number_float = (GTA_Computed_Value_Float *)other;
    return (GTA_Computed_Value *)(number->value != other_number_float->value ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  if (GTA_COMPUTED_VALUE_IS_INTEGER(other)) {
    GTA_Computed_Value_Integer * other_number_integer = (GTA_Computed_Value_Integer *)other;
    return (GTA_Computed_Value *)(number->value != (GTA_Float)other_number_integer->value ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  return gta_computed_value_error_not_supported;
}


GTA_Computed_Value * GTA_CALL gta_computed_value_float_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context) {
  assert(self);
  assert(GTA_COMPUTED_VALUE_IS_FLOAT(self));
  GTA_Computed_Value_Float * number = (GTA_Computed_Value_Float *)self;

  if (type == &gta_computed_value_float_vtable) {
    return self;
  }
  if (type == &gta_computed_value_integer_vtable) {
    return (GTA_Computed_Value *)gta_computed_value_integer_create((GTA_Integer)number->value, context);
  }
  if (type == &gta_computed_value_boolean_vtable) {
    return (GTA_Computed_Value *)(number->value != 0 ? gta_computed_value_boolean_true : gta_computed_value_boolean_false);
  }
  if (type == &gta_computed_value_string_vtable) {
    char * str = gta_computed_value_float_to_string(self);
    if (!str) {
      return NULL;
    }
    GTA_Unicode_String * unicode_str = gta_unicode_string_create(str, strlen(str), GTA_UNICODE_STRING_TYPE_HTML);
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
