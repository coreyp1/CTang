/**
 * @file
 *
 * Header file for the computed self class.
 */

#ifndef G_TANG_COMPUTED_VALUE_H
#define G_TANG_COMPUTED_VALUE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdbool.h>
#include <cutil/vector.h>
#include <tang/macros.h>
#include <tang/unicodeString.h>

/**
 * A vector of computed values.
 *
 * This structure is used when a function is called.  The arguments of the
 * function are stored in this vector.
 */
typedef GTA_VectorX GTA_Computed_Value_Vector;

typedef GTA_Computed_Value * GTA_CALL (*GTA_Computed_Value_Attribute_Callback)(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * This is a pair of an attribute name and a function to get the attribute
 * value.  It is used to set up the language's default attributes.
 */
struct GTA_Computed_Value_Attribute_Pair {
  /**
   * The attribute name.
   */
  const char * name;
  /**
   * The function to be called to get the attribute value.
   */
  GTA_Computed_Value_Attribute_Callback callback;
};

/**
 * Virtual table for the computed value class.
 *
 * The virtual table also serves as the type identifier for the class.  All
 * objects of the same class should share the same virtual table, preferably
 * stored in a static constant variable.
 *
 * If a method has a "reverse" argument, then the method will be called twice,
 * if needed.  For example, If the expression is `a + b`, then the method
 * `add` will be called with `a` as `self` and `b` as `other`, and if either
 * null or an error is returned, then the method will be called again with `b`
 * as `self` and `a` as `other`.  The `reverse` argument is used to indicate
 * which call is being made.
 *
 * @see GTA_Computed_Value
 */
struct GTA_Computed_Value_VTable {
  /**
   * The name of the class.
   */
  const char * name;
  /**
   * Destructor for the class.
   *
   * @param self The object to destroy.
   */
  void (*destroy)(GTA_Computed_Value * self);
  /**
   * Destructor for the class in place.
   *
   * @param self The object to destroy.
   */
  void (*destroy_in_place)(GTA_Computed_Value * self);
  /**
   * Performs a deep copy of the object.
   *
   * @param self The object to copy.
   * @param context The execution context of the program.
   * @return A deep copy of the object or NULL if the operation failed.
   */
  GTA_Computed_Value * (*deep_copy)(GTA_Computed_Value * self, GTA_Execution_Context * context);
  /**
   * Get a string representation of the object.
   *
   * The caller is responsible for freeing the returned string.
   *
   * @param self The object to convert.
   * @return A string representation of the object or NULL if the operation
   *   failed.
   */
  char * (*to_string)(GTA_Computed_Value * self);
  /**
   * Produce a "printable" version of the object.
   *
   * Because the text is intended to be displayed, it is up to the object to
   * indicate the type of text being provided, whether it should be trusted
   * as-is or otherwise escaped.
   *
   * @param self The object to print.
   * @param context The execution context of the program.
   * @return A string representation of the object or NULL if the operation
   *  failed.
   */
  GTA_Unicode_String * (*print)(GTA_Computed_Value * self, GTA_Execution_Context * context);
  /**
   * Assigns a value to an index of the object.
   *
   * @param self The object to modify.
   * @param index The index to assign to.
   * @param other The value to assign.
   * @param context The execution context of the program.
   * @return The modified object or NULL if the operation failed.
   */
  GTA_Computed_Value * (*assign_index)(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Computed_Value * other, GTA_Execution_Context * context);
  /**
   * Adds two values together.
   *
   * @param self The first value.
   * @param other The second value.
   * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
   *   otherwise.
   * @param is_assignment If `true`, lhs is being assigned to.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*add)(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);
  /**
   * Subtracts two values.
   *
   * @param self The first value.
   * @param other The second value.
   * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
   *   otherwise.
   * @param is_assignment If `true`, lhs is being assigned to.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*subtract)(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);
  /**
   * Multiplies two values together.
   *
   * @param self The first value.
   * @param other The second value.
   * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
   *   otherwise.
   * @param is_assignment If `true`, lhs is being assigned to.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*multiply)(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);
  /**
   * Divides two values.
   *
   * If the second value is zero, then the operation should return an error.
   *
   * @param self The first value.
   * @param other The second value.
   * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
   *   otherwise.
   * @param is_assignment If `true`, lhs is being assigned to.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*divide)(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);
  /**
   * Computes the modulo of two values.
   *
   * If the second value is zero, then the operation should return an error.
   *
   * @param self The first value.
   * @param other The second value.
   * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
   *   otherwise.
   * @param is_assignment If `true`, lhs is being assigned to.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*modulo)(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);
  /**
   * Computes the negative of a value.
   *
   * @param self The value to negate.
   * @param is_assignment If `true`, operand is being assigned to.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*negative)(GTA_Computed_Value * self, bool is_assignment, GTA_Execution_Context * context);
  /**
   * Compares two values to see if the first is less than the second.
   *
   * @param self The first value.
   * @param other The second value.
   * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
   *   otherwise.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*less_than)(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);
  /**
   * Compares two values to see if the first is less than or equal to the
   * second.
   *
   * @param self The first value.
   * @param other The second value.
   * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
   *   otherwise.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*less_than_equal)(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);
  /**
   * Compares two values to see if the first is greater than the second.
   *
   * @param self The first value.
   * @param other The second value.
   * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
   *   otherwise.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*greater_than)(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);
  /**
   * Compares two values to see if the first is greater than or equal to the
   * second.
   *
   * @param self The first value.
   * @param other The second value.
   * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
   *   otherwise.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*greater_than_equal)(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);
  /**
   * Compares two values to see if they are equal.
   *
   * @param self The first value.
   * @param other The second value.
   * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
   *   otherwise.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*equal)(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);
  /**
   * Compares two values to see if they are not equal.
   *
   * @param self The first value.
   * @param other The second value.
   * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
   *   otherwise.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*not_equal)(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);
  /**
   * Gets a value from the object using a period identifier.
   *
   * @param self The object to get the value from.
   * @param identifier_hash The hash of the identifier to get.
   * @param context The execution context of the program.
   * @return The value of the identifier or NULL if the operation failed.
   */
  GTA_Computed_Value * (*period)(GTA_Computed_Value * self, GTA_UInteger identifier_hash, GTA_Execution_Context * context);
  /**
   * Gets a value from the object using an index.
   *
   * @param self The object to get the value from.
   * @param index The index to get.
   * @param context The execution context of the program.
   * @return The value of the index or NULL if the operation failed.
   */
  GTA_Computed_Value * (*index)(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Execution_Context * context);
  /**
   * Gets a slice of the object.
   *
   * @param self The object to get the slice from.
   * @param start The start of the slice.
   * @param end The end of the slice.
   * @param step The step of the slice.
   * @param context The execution context of the program.
   * @return The slice of the object or NULL if the operation failed.
   */
  GTA_Computed_Value * (*slice)(GTA_Computed_Value * self, GTA_Computed_Value * start, GTA_Computed_Value * end, GTA_Computed_Value * step, GTA_Execution_Context * context);
  /**
   * Gets an iterator from the object.
   *
   * @param self The object to get the value from.
   * @param context The execution context of the program.
   * @return The value of the iterator or NULL if the operation failed.
   */
  GTA_Computed_Value * (*iterator_get)(GTA_Computed_Value * self, GTA_Execution_Context * context);
  /**
   * Advance an iterator to the next value.
   *
   * @param self The object to get the value from.
   * @param context The execution context of the program.
   * @return The value of the iterator or NULL if the operation failed.
   */
  GTA_Computed_Value * (*iterator_next)(GTA_Computed_Value * self, GTA_Execution_Context * context);
  /**
   * Casts the object to a different type.
   *
   * @param self The object to cast.
   * @param type The type to cast to.
   * @param context The execution context to cast in.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*cast)(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context);
  /**
   * Calls the object as a function.
   *
   * @param self The object to call.
   * @param arguments The arguments to call with.
   * @param context The execution context of the program.
   * @return The result of the operation or NULL if the operation failed.
   */
  GTA_Computed_Value * (*call)(GTA_Computed_Value * self, GTA_Computed_Value_Vector * arguments, GTA_Execution_Context * context);
  /**
   * The default attributes for this type.
   */
  GTA_Computed_Value_Attribute_Pair * attributes;
  /**
   * The number of attributes in the `attributes` array.
   */
  size_t attributes_count;
};

/**
 * The null virtual table singleton for the computed value class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_null_vtable;

/**
 * The base class for all computed values.
 */
struct GTA_Computed_Value {
  /**
   * The virtual table for the class.
   *
   * The virtual table also serves as the type identifier for the class.  All
   * objects of the same class should share the same virtual table, preferably
   * stored in a static constant variable.
   *
   * @see GTA_Computed_Value_VTable
   */
  GTA_Computed_Value_VTable * vtable;
  /**
   * The execution context that the value was created in.
   *
   * The computed value must register itself with the garbage collector so that
   * it can be destroyed when it is no longer needed without the overhead of
   * reference counting or manual memory management (by the user).
   *
   * @see GTA_Execution_Context
   */
  GTA_Execution_Context * context;
  /**
   * The attributes of the value.
   *
   * This is a hash map of read-only attributes that may be accessed.  The key
   * is the hash of the attribute name, and the value is the computed value
   * that should be returned.  The computed value may be of any type.
   *
   * These values are stored per-object and may be used to override the default
   * attribute values of the class.
   */
  GTA_HashX * attributes;
  /**
   * Whether or not the value is truthy, to aid in logical operations.
   */
  bool is_true;
  /**
   * Whether or not the value is an error, to aid in error handling.
   */
  bool is_error;
  /**
   * Whether or not the value is temporary, to aid in memory management.
   */
  bool is_temporary;
  /**
   * Whether or not the value requires a deep copy, to aid in memory management.
   */
  bool requires_deep_copy;
  /**
   * Whether or not the value is a singleton, to aid in memory management.
   *
   * Singleton values are shared between all instances of the same class and
   * should not be destroyed.
   */
  bool is_singleton;
  /**
   * Whether or not the value is a reference, to aid in memory management.
   *
   * Reference values share internal data with other values and should not be
   * destroyed.  This is useful for reducing memory usage when the same value
   * is used in multiple places, such as in passing an object to a function.
   */
  bool is_a_reference;
};

/**
 * The NULL computed value singleton.
 */
extern GTA_Computed_Value * gta_computed_value_null;

/**
 * Creates a new computed value.
 *
 * @param context The execution context to create the value in.
 * @return A new computed value or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_create(GTA_Execution_Context * context);

/**
 * Creates a new computed value in place.
 *
 * @param self The memory address of the computed value.
 * @param context The execution context of the program.
 * @return True on success, false on failure.
 */
bool gta_computed_value_create_in_place(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Destroys a computed value.
 *
 * Calls the `destroy` method of the virtual table.
 *
 * @param self The object to destroy.
 */
void gta_computed_value_destroy(GTA_Computed_Value * self);

/**
 * Destroys a computed value in place.
 *
 * Calls the `destroy_in_place` method of the virtual table.
 *
 * @param self The object to destroy.
 */
void gta_computed_value_destroy_in_place(GTA_Computed_Value * self);

/**
 * Performs a deep copy of a computed value.
 *
 * Calls the `deep_copy` method of the virtual table.
 *
 * @param self The object to copy.
 * @param context The execution context of the program.
 * @return A deep copy of the object or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_deep_copy(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Gets a string representation of a computed value.
 *
 * Calls the `to_string` method of the virtual table.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @param self The object to convert.
 * @return A string representation of the object or NULL if the operation
 *   failed.
 */
GTA_NO_DISCARD char * gta_computed_value_to_string(GTA_Computed_Value * self);

/**
 * Prints a computed value.
 *
 * Calls the `print` method of the virtual table.
 *
 * @param self The object to print.
 * @param context The execution context of the program.
 * @return A string representation of the object or NULL if the operation
 *  failed.
 */
GTA_NO_DISCARD GTA_Unicode_String * gta_computed_value_print(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Assigns a value to an index of a computed value.
 *
 * Calls the `assign_index` method of the virtual table.
 *
 * @param self The object to modify.
 * @param index The index to assign to.
 * @param other The value to assign.
 * @param context The execution context of the program.
 * @return The modified object or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_assign_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Computed_Value * other, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_add(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_subtract(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_multiply(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_divide(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_modulo(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_negative(GTA_Computed_Value * self, bool is_assignment, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_less_than(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Compares two values to see if the first is less than or equal to the
 * second.
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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_less_than_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_greater_than(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_greater_than_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_not_equal(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Gets a value from a computed value using a period identifier.
 *
 * Calls the `period` method of the virtual table.
 *
 * @param self The object to get the value from.
 * @param identifier_hash The hash of the identifier to get.
 * @param context The execution context of the program.
 * @return The value of the identifier or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_period(GTA_Computed_Value * self, GTA_UInteger identifier_hash, GTA_Execution_Context * context);

/**
 * Gets a value from a computed value using an index.
 *
 * Calls the `index` method of the virtual table.
 *
 * @param self The object to get the value from.
 * @param index The index to get.
 * @param context The execution context of the program.
 * @return The value of the index or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_index(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Execution_Context * context);

/**
 * Gets a slice of a computed value.
 *
 * Calls the `slice` method of the virtual table.
 *
 * @param self The object to get the slice from.
 * @param start The start of the slice.
 * @param end The end of the slice.
 * @param step The step of the slice.
 * @param context The execution context of the program.
 * @return The slice of the object or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_slice(GTA_Computed_Value * self, GTA_Computed_Value * start, GTA_Computed_Value * end, GTA_Computed_Value * step, GTA_Execution_Context * context);

/**
 * Gets an iterator from a computed value.
 *
 * Calls the `iterator_get` method of the virtual table.
 *
 * @param self The object to get the value from.
 * @param context The execution context of the program.
 * @return The value of the iterator or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_iterator_get(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Advance an iterator to the next value.
 *
 * Calls the `iterator_next` method of the virtual table.
 *
 * @param self The object to get the value from.
 * @param context The execution context of the program.
 * @return The value of the iterator or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_iterator_next(GTA_Computed_Value * self, GTA_Execution_Context * context);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context);

/**
 * Calls a computed value as a function.
 *
 * Calls the `call` method of the virtual table.
 *
 * @param self The object to call.
 * @param arguments The arguments to call with.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_call(GTA_Computed_Value * self, GTA_Computed_Value_Vector * arguments, GTA_Execution_Context * context);

/**
 * Destroys a computed value of the NULL class.
 *
 * @see gta_computed_value_destroy
 *
 * @param self The object to destroy.
 */
void gta_computed_value_null_destroy(GTA_Computed_Value * self);

/**
 * Destroys a computed value of the NULL class in place.
 *
 * @see gta_computed_value_destroy_in_place
 *
 * @param self The object to destroy.
 */
void gta_computed_value_null_destroy_in_place(GTA_Computed_Value * self);

/**
 * Performs a deep copy of a computed value of the NULL class.
 *
 * @see gta_computed_value_deep_copy
 *
 * @param self The object to copy.
 * @param context The execution context of the program.
 * @return A deep copy of the object or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_null_deep_copy(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Gets a string representation of a computed value of the NULL class.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @see gta_computed_value_to_string
 *
 * @param self The object to convert.
 * @return A string representation of the object or NULL if the operation
 *   failed.
 */
GTA_NO_DISCARD char * gta_computed_value_null_to_string(GTA_Computed_Value * self);

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
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_null_cast(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context);

/**
 * Prints a computed value.
 *
 * Calls the `print` method of the virtual table.
 *
 * @param self The object to print.
 * @param context The execution context of the program.
 * @return A string representation of the object or NULL if the operation
 *  failed.
 */
GTA_NO_DISCARD GTA_Unicode_String * gta_computed_value_print_not_implemented(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `assign_index` method for the
 * virtual table.
 *
 * @param self The object to modify.
 * @param index The index to assign to.
 * @param other The value to assign.
 * @param context The execution context of the program.
 * @return The modified object or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_assign_index_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Computed_Value * other, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `add` method for the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_add_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `subtract` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_subtract_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `multiply` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_multiply_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `divide` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_divide_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `modulo` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_modulo_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `negative` method for the virtual
 * table.
 *
 * @param self The value to negate.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_negative_not_implemented(GTA_Computed_Value * self, bool is_assignment, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `less_than` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_less_than_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `less_than_equal` method for the
 * virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_less_than_equal_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `greater_than` method for the
 * virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_greater_than_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `greater_than_equal` method for the
 * virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_greater_than_equal_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `equal` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_equal_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `not_equal` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_not_equal_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `period` method for the virtual
 * table.
 *
 * @param self The object to get the value from.
 * @param identifier_hash The hash of the identifier to get.
 * @param context The execution context of the program.
 * @return The value of the identifier or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_period_not_implemented(GTA_Computed_Value * self, GTA_UInteger identifier_hash, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `index` method for the virtual
 * table.
 *
 * @param self The object to get the value from.
 * @param index The index to get.
 * @param context The execution context of the program.
 * @return The value of the index or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_index_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `slice` method for the virtual
 * table.
 *
 * @param self The object to get the slice from.
 * @param start The start of the slice.
 * @param end The end of the slice.
 * @param step The step of the slice.
 * @param context The execution context of the program.
 * @return The slice of the object or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_slice_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value * start, GTA_Computed_Value * end, GTA_Computed_Value * step, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `iterator_get` method for the
 * virtual table.
 *
 * @param self The object to get the value from.
 * @param context The execution context of the program.
 * @return The value of the iterator or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_iterator_get_not_implemented(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `iterator_next` method for the
 * virtual table.
 *
 * @param self The object to get the value from.
 * @param context The execution context of the program.
 * @return The value of the iterator or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_iterator_next_not_implemented(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `cast` method for the virtual
 * table.
 *
 * @param self The object to cast.
 * @param type The type to cast to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_cast_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context);

/**
 * Generic "not implemented" version of the `call` method for the virtual
 * table.
 *
 * @param self The object to call.
 * @param arguments The arguments to call with.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_call_not_implemented(GTA_Computed_Value * self, GTA_Computed_Value_Vector * arguments, GTA_Execution_Context * context);

/**
 * Prints a computed value.
 *
 * Calls the `print` method of the virtual table.
 *
 * @param self The object to print.
 * @param context The execution context of the program.
 * @return A string representation of the object or NULL if the operation
 *  failed.
 */
GTA_NO_DISCARD GTA_Unicode_String * gta_computed_value_print_not_supported(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `assign_index` method for the virtual
 * table.
 *
 * @param self The object to modify.
 * @param index The index to assign to.
 * @param other The value to assign.
 * @param context The execution context of the program.
 * @return The modified object or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_assign_index_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Computed_Value * other, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `add` method for the virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_add_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `subtract` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_subtract_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `multiply` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_multiply_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `divide` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_divide_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `modulo` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_modulo_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, bool is_assignment, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `negative` method for the virtual
 * table.
 *
 * @param self The value to negate.
 * @param is_assignment If `true`, operand is being assigned to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_negative_not_supported(GTA_Computed_Value * self, bool is_assignment, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `less_than` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_less_than_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `less_than_equal` method for the
 * virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_less_than_equal_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `greater_than` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_greater_than_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `greater_than_equal` method for the
 * virtual table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_greater_than_equal_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `equal` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_equal_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `not_equal` method for the virtual
 * table.
 *
 * @param self The first value.
 * @param other The second value.
 * @param self_is_lhs `true` if `self` is the lhs of the expression, `false`
 *   otherwise.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_not_equal_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * other, bool self_is_lhs, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `period` method for the virtual
 * table.
 *
 * @param self The object to get the value from.
 * @param identifier_hash The hash of the identifier to get.
 * @param context The execution context of the program.
 * @return The value of the identifier or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_period_not_supported(GTA_Computed_Value * self, GTA_UInteger identifier_hash, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `index` method for the virtual
 * table.
 *
 * @param self The object to get the value from.
 * @param index The index to get.
 * @param context The execution context of the program.
 * @return The value of the index or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_index_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * index, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `slice` method for the virtual
 * table.
 *
 * @param self The object to get the slice from.
 * @param start The start of the slice.
 * @param end The end of the slice.
 * @param step The step of the slice.
 * @param context The execution context of the program.
 * @return The slice of the object or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_slice_not_supported(GTA_Computed_Value * self, GTA_Computed_Value * start, GTA_Computed_Value * end, GTA_Computed_Value * step, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `iterator_get` method for the virtual
 * table.
 *
 * @param self The object to get the value from.
 * @param context The execution context of the program.
 * @return The value of the iterator or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_iterator_get_not_supported(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `iterator_next` method for the
 * virtual table.
 *
 * @param self The object to get the value from.
 * @param context The execution context of the program.
 * @return The value of the iterator or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_iterator_next_not_supported(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `cast` method for the virtual table.
 *
 * @param self The object to cast.
 * @param type The type to cast to.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_cast_not_supported(GTA_Computed_Value * self, GTA_Computed_Value_VTable * type, GTA_Execution_Context * context);

/**
 * Generic "not supported" version of the `call` method for the virtual table.
 *
 * @param self The object to call.
 * @param arguments The arguments to call with.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_call_not_supported(GTA_Computed_Value * self, GTA_Computed_Value_Vector * arguments, GTA_Execution_Context * context);

/**
 * Generic function to use the "to_string" method to generate a printed value.
 *
 * Calls the `print` method of the virtual table.
 *
 * @param self The object to print.
 * @param context The execution context of the program.
 * @return A string representation of the object or NULL if the operation
 *  failed.
 */
GTA_NO_DISCARD GTA_Unicode_String * gta_computed_value_generic_print_from_to_string(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Generic "period" method which searches the object and the language for any
 * pre-defined attribute name/callback pairs and executes the callback if
 * found.
 *
 * @param self The object to get the value from.
 * @param identifier_hash The hash of the identifier to get.
 * @param context The execution context of the program.
 * @return The value of the identifier or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * gta_computed_value_generic_period(GTA_Computed_Value * self, GTA_UInteger identifier_hash, GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_COMPUTED_VALUE_H
