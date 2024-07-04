/**
 * @file
 */

#ifndef TANG_COMPUTEDVALUEITERATOR_H
#define TANG_COMPUTEDVALUEITERATOR_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "tang/computedValue/computedValue.h"

/**
 * The VTable for the ComputedValueIterator.
 */
extern GTA_Computed_Value_VTable gta_computed_value_iterator_vtable;

/**
 * A Computed Value Error for when an iterator has reached the end of the
 * collection.
 */
extern GTA_Computed_Value * gta_computed_value_error_iterator_end;

/**
 * A user-defined function to be called when the iterator is advanced.
 *
 * @param iterator The iterator object to mutate.
 * @return True if the operation was successful, false otherwise.
 */
typedef void (*GTA_Iterator_Advance)(GTA_Computed_Value_Iterator * iterator);

/**
 * A user-defined function to be called when the iterator is destroyed.
 *
 * This may be necessary to clean up any resources that the iterator has allocated.
 *
 * @param iterator The iterator object to mutate.
 */
typedef void (*GTA_Iterator_Destroy)(GTA_Computed_Value_Iterator * iterator);

/**
 * An iterator object over a collection of some kind.
 *
 * This object is a general structure that can be applied to any collection.
 * The the collection has its 'next' function called, an iterator object is
 * returned. The collection should set the 'resource' field of the iterator
 * if needed as well as the 'advance' and 'destroy' functions.
 *
 * When creating the iterator object, the collection should *not* set 'value'.
 *
 * Calling the iterator's 'next' function will, in turn, call the 'advance'
 * function that was supplied by the collection.  The 'advance' function should
 * set the 'value' field of the iterator.  The 'advance' function should not
 * modify the 'index' field.  The 'advance' function may modify the 'resource'
 * field if necessary.
 *
 * The 'index' field will have a value of 0 when the iterator is first created.
 * The 'index' field will be incremented after the 'advance' function is
 * invoked.
 *
 * The 'destroy' function will be called when the iterator is destroyed.  The
 * 'destroy' function should clean up any resources that the iterator has
 * allocated.
 *
 * The 'resource' field is a pointer to a resource that the iterator may need
 * to access.  The 'resource' field should be set by the collection when the
 * iterator is created.  The 'resource' field may be modified by the 'advance'
 * function.  The 'resource' field is fully controlled by the user functions
 * provided.  It may be used for any purpose.
 *
 * If a call to 'advance' is made and a valid value is available, the 'value'
 * field should be set to an appropriate value.  If a call to 'advance' is made
 * and no valid value is available, the 'value' field should be set to an
 * appropriate error message.  The 'gta_computed_value_error_iterator_end'
 * value is provided to signify that the iterator has reached the end of the
 * collection and no more values are available.
 */
typedef struct GTA_Computed_Value_Iterator {
  /**
   * The base class for the computed value.
   */
  GTA_Computed_Value base;
  /**
   * The collection over which to iterate.
   */
  GTA_Computed_Value * collection;
  /**
   * A monotonic counter that is incremented each time the iterator is
   * advanced.
   */
  GTA_Integer index;
  /**
   * The index of the next item to return.
   */
  void * resource;
  /**
   * The user-defined function to call when the iterator is advanced.
   */
  GTA_Iterator_Advance advance;
  /**
   * The user-defined function to call when the iterator is destroyed.
   */
  GTA_Iterator_Destroy destroy;
  /**
   * The value returned by the iterator when it is advanced.
   */
  GTA_Computed_Value * value;
} GTA_Computed_Value_Iterator;

/**
 * Create a new iterator object.
 *
 * @param collection The collection over which to iterate.
 * @param context The execution context to create the value in.
 * @return The new computed value for the integer.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_iterator_create(GTA_Computed_Value * collection, GTA_Execution_Context * context);

/**
 * Create a new iterator object in place.
 *
 * @param self The memory address of the computed value.
 * @param collection The collection over which to iterate.
 * @param context The execution context to create the value in.
 * @return True if the operation was successful, false otherwise.
 */
bool GTA_CALL gta_computed_value_iterator_create_in_place(GTA_Computed_Value_Iterator * self, GTA_Computed_Value * collection, GTA_Execution_Context * context);

/**
 * Destroy an iterator object.
 *
 * @see gta_computed_value_destroy
 *
 * @param computed_value The computed value for the integer.
 */
void GTA_CALL gta_computed_value_iterator_destroy(GTA_Computed_Value * computed_value);

/**
 * Destroy an iterator object in place.
 *
 * @see gta_computed_value_destroy_in_place
 *
 * @param self The computed value for the integer.
 */
void GTA_CALL gta_computed_value_iterator_destroy_in_place(GTA_Computed_Value * self);

/**
 * Advance an iterator to the next value.
 *
 * @param self The object to get the value from.
 * @param context The execution context of the program.
 * @return The value of the iterator or NULL if the operation failed.
 */
GTA_NO_DISCARD GTA_Computed_Value * GTA_CALL gta_computed_value_iterator_iterator_next(GTA_Computed_Value * self, GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TANG_COMPUTEDVALUEITERATOR_H
