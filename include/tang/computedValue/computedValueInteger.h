/**
 * @file
 *
 * Header file for the ComputedValueInteger class.
 */

#ifndef TANG_COMPUTED_VALUE_INTEGER_H
#define TANG_COMPUTED_VALUE_INTEGER_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/computedValue/computedValue.h>

/**
 * The VTable for the ComputedValueInteger class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_integer_vtable;

/**
 * The computed value for an integer.
 */
typedef struct GTA_Computed_Value_Integer {
  /**
   * The base class for the computed value.
   */
  GTA_Computed_Value base;
  /**
   * The value of the integer.
   */
  GTA_Integer value;
} GTA_Computed_Value_Integer;

/**
 * Create a new computed value for an integer.
 *
 * @param value The value of the integer.
 * @param context The execution context to create the value in.
 * @return The new computed value for the integer.
 */
GTA_NO_DISCARD GTA_Computed_Value_Integer * GTA_CALL gta_computed_value_integer_create(GTA_Integer value, GTA_Execution_Context * context);

/**
 * Create a new computed value for an integer in place.
 *
 * @param self The memory address of the computed value.
 * @param value The value of the integer.
 * @param context The execution context to create the value in.
 * @return True if the operation was successful, false otherwise.
 */
bool GTA_CALL gta_computed_value_integer_create_in_place(GTA_Computed_Value_Integer * self, GTA_Integer value, GTA_Execution_Context * context);

/**
 * Destroy a computed value for an integer.
 *
 * @see gta_computed_value_destroy
 *
 * @param computed_value The computed value for the integer.
 */
void GTA_CALL gta_computed_value_integer_destroy(GTA_Computed_Value * computed_value);

/**
 * Destroy a computed value for an integer in place.
 *
 * @see gta_computed_value_destroy_in_place
 *
 * @param self The computed value for the integer.
 */
void GTA_CALL gta_computed_value_integer_destroy_in_place(GTA_Computed_Value * self);

/**
 * Deep copy a computed value for an integer.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param value The computed value to be copied.
 * @return The deep copy of the ComputedValueInteger or NULL if an error occurred.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_integer_deep_copy(GTA_Computed_Value * value);

/**
 * Get a string representation of the computed value for an integer.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @see gta_computed_value_to_string
 *
 * @param self The computed value for the integer.
 * @return The string representation of the computed value for the integer.
 */
GTA_NO_DISCARD char * GTA_CALL gta_computed_value_integer_to_string(GTA_Computed_Value * self);

/**
 * Adds two values together.
 *
 * Calls the `add` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_add(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment);

/**
 * Subtracts two values.
 *
 * Calls the `subtract` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_subtract(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment);

/**
 * Multiplies two values together.
 *
 * Calls the `multiply` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_multiply(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment);

/**
 * Divides two values.
 *
 * Calls the `divide` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_divide(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment);

/**
 * Computes the modulo of two values.
 *
 * Calls the `modulo` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_modulo(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment);

/**
 * Computes the negative of a value.
 *
 * Calls the `negative` method of the virtual table.
 *
 * @param self The value to negate.
 * @param is_assignment If `true`, operand is being assigned to.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_negative(GTA_Computed_Value * self, bool is_assignment);

/**
 * Compares two values to see if the first is less than the second.
 *
 * Calls the `less_than` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_less_than(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs);

/**
 * Compares two values to see if the first is less than or equal to the second.
 *
 * Calls the `less_than_equal` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_less_than_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs);

/**
 * Compares two values to see if the first is greater than the second.
 *
 * Calls the `greater_than` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_greater_than(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs);

/**
 * Compares two values to see if the first is greater than or equal to the
 * second.
 *
 * Calls the `greater_than_equal` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_greater_than_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs);

/**
 * Compares two values to see if they are equal.
 *
 * Calls the `equal` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs);

/**
 * Compares two values to see if they are not equal.
 *
 * Calls the `not_equal` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_not_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs);

/**
 * Casts a computed value to a different type.
 *
 * Calls the `cast` method of the virtual table.
 *
 * @param self The object to cast.
 * @param type The type to cast to.
 * @param context The execution context in which to create the value.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_integer_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_COMPUTED_VALUE_INTEGER_H
