/**
 * @file
 *
 * Header file for the ComputedValueArray class.
 */

#ifndef TANG_COMPUTED_VALUE_ARRAY_H
#define TANG_COMPUTED_VALUE_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/computedValue/computedValue.h>

/**
 * The VTable for the ComputedValueArray class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_array_vtable;

/**
 * The computed value for an array.
 */
typedef struct GTA_Computed_Value_Array {
  /**
   * The base class for the computed value.
   */
  GTA_Computed_Value base;
  /**
   * The values in the array.
   */
  GTA_VectorX * elements;
} GTA_Computed_Value_Array;

/**
 * Create a new computed value for an array.
 *
 * @param size The expected size of the array.
 * @param context The execution context to create the value in.
 * @return The new computed value for the array.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_array_create(size_t size, GTA_Execution_Context * context);

/**
 * Create a new computed value for an array in place.
 *
 * @param self The memory address of the computed value.
 * @param size The expected size of the array.
 * @param context The execution context to create the value in.
 * @return True if the operation was successful, false otherwise.
 */
bool GTA_CALL gta_computed_value_array_create_in_place(GTA_Computed_Value_Array * self, size_t size, GTA_Execution_Context * context);

/**
 * Destroy a computed value for an array.
 *
 * @see gta_computed_value_destroy
 *
 * @param self The computed value for the array.
 */
void GTA_CALL gta_computed_value_array_destroy(GTA_Computed_Value * self);

/**
 * Destroy a computed value for an array in place.
 *
 * @see gta_computed_value_destroy_in_place
 *
 * @param self The computed value for the array.
 */
void GTA_CALL gta_computed_value_array_destroy_in_place(GTA_Computed_Value * self);

/**
 * Deep copy a computed value for an array.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param value The computed value to be copied.
 * @param context The execution context of the program.
 * @return The deep copy of the ComputedValueArray or NULL if an error occurred.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_array_deep_copy(GTA_Computed_Value * value, GTA_Execution_Context * context);

/**
 * Get a string representation of the computed value for an array.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @see gta_computed_value_to_string
 *
 * @param self The computed value for the array.
 * @return The string representation of the computed value for the array.
 */
GTA_NO_DISCARD char * GTA_CALL gta_computed_value_array_to_string(GTA_Computed_Value * self);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_array_add(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_array_multiply(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_array_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_array_not_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Assigns a value to an index of the object.
 *
 * @param self The object to modify.
 * @param index The index to assign to.
 * @param other The value to assign.
 * @param context The execution context of the program.
 * @return The modified object or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_array_index_assign(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Computed_Value * other, GTA_Execution_Context * context);

/**
 * Gets a value from the object using an index.
 *
 * @param self The object to get the value from.
 * @param index The index to get.
 * @param context The execution context of the program.
 * @return The value of the index or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_array_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Execution_Context * context);

/**
 * Append a value to the array.
 *
 * @param self The array to append to.
 * @param value The value to append.
 * @param context The execution context of the program.
 * @return The Computed Value that resulted from the expression.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_array_append(GTA_Computed_Value_Array * self, GTA_Computed_Value * value, GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_COMPUTED_VALUE_ARRAY_H
