/**
 * @file
 *
 * @brief Header file for the computedValueFloat class
 */

#ifndef G_TANG_COMPUTEDVALUEFLOAT_H
#define G_TANG_COMPUTEDVALUEFLOAT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/computedValue/computedValue.h>

/**
 * The VTable for the ComputedValueFloat class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_float_vtable;

/**
 * The Computed Value Float class.
 */
struct GTA_Computed_Value_Float {
  /**
   * The base class.
   */
  GTA_Computed_Value base;
  /**
   * The value.
   */
  GTA_Float value;
};

/**
 * Create a new Computed Value Float.
 *
 * @param value The value.
 * @param context The execution context to create the value in.
 * @return The new Computed Value Float or NULL on failure.
 */
GTA_Computed_Value_Float * GTA_CALL gta_computed_value_float_create(GTA_Float value, GTA_Execution_Context * context);

/**
 * Create a new computed value for a float in place.
 *
 * @param self The memory address of the computed value.
 * @param value The value of the float.
 * @param context The execution context to create the value in.
 * @return True if the operation was successful, false otherwise.
 */
bool GTA_CALL gta_computed_value_float_create_in_place(GTA_Computed_Value_Float * self, GTA_Float value, GTA_Execution_Context * context);

/**
 * Destroy a Computed Value Float.
 *
 * @param computed_value The Computed Value Float.
 */
void GTA_CALL gta_computed_value_float_destroy(GTA_Computed_Value * computed_value);

/**
 * Destroy a computed value for a float in place.
 *
 * @see gta_computed_value_destroy_in_place
 *
 * @param self The computed value for the float.
 */
void GTA_CALL gta_computed_value_float_destroy_in_place(GTA_Computed_Value * self);

/**
 * Deep copy a computed value for an float.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param value The computed value to be copied.
 * @param context The execution context of the program.
 * @return The deep copy of the ComputedValueInteger or NULL if an error occurred.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_deep_copy(GTA_Computed_Value * value, GTA_Execution_Context * context);

/**
 * Get a string representation of the computed value for a float.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @see gta_computed_value_to_string
 *
 * @param self The computed value for the float.
 * @return The string representation of the computed value for the float.
 */
GTA_NO_DISCARD char * GTA_CALL gta_computed_value_float_to_string(GTA_Computed_Value * self);

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
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_add(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

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
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_subtract(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

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
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_multiply(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

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
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_divide(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

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
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_modulo(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

/**
 * Computes the negative of a value.
 *
 * Calls the `negative` method of the virtual table.
 *
 * @param self The value to negate.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_negative(GTA_Computed_Value * self, bool is_assignment, GTA_Execution_Context * context);

/**
 * Compares two values to see if the first is less than the second.
 *
 * Calls the `less_than` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_less_than(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Compares two values to see if the first is less than or equal to the second.
 *
 * Calls the `less_than_equal` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_less_than_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Compares two values to see if the first is greater than the second.
 *
 * Calls the `greater_than` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value *GTA_CALL  gta_computed_value_float_greater_than(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

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
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_greater_than_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Compares two values to see if they are equal.
 *
 * Calls the `equal` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Compares two values to see if they are not equal.
 *
 * Calls the `not_equal` method of the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_not_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Casts a computed value to a different type.
 *
 * Calls the `cast` method of the virtual table.
 *
 * @param self The object to cast.
 * @param type The type to cast to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_float_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_COMPUTEDVALUEFLOAT_H
