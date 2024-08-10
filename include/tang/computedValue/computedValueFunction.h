/**
 * @file
 */

#ifndef G_TANG_COMPUTED_VALUE_FUNCTION_H
#define G_TANG_COMPUTED_VALUE_FUNCTION_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <tang/computedValue/computedValue.h>

/**
 * The vtable for the GTA_Computed_Value_Function class.
 */
extern GTA_Computed_Value_VTable gta_computed_value_function_vtable;

/**
 * The GTA_Computed_Value_Function class.
 *
 * This type represents a function that was declared in the script itself.  It
 * is not called in the same way that a native function would be called.
 * It is more of a "jump here, do something, and jump back" behavior.
 *
 * The object itself only knows the number of arguments that it expects and the
 * pointer to the function.  The pointer is a bytecode offset when using the
 * bytecode interpreter and a pointer to the binary code when using the binary
 * JIT compiler.
 *
 * Regardless of the method used, a function call should follow this pattern:
 * 1. Evaluate the function arguments.
 * 2. The identifier is used to load the computed value object.
 * 3. If the object is not a function, then return an error.
 * 4. If the number of arguments does not match, then return an error.
 * 5. Call the function, passing the number of arguments and the argument list.
 *
 * In the bytecode interpreter, these functions are "called" by simply jumping
 * to the address of the function and then returning when the function is done.
 * The return address (pc) and frame pointer (fp) is pushed onto the virtual
 * machine's pc/fp stack, and restored when the function returns.  Arguments are
 * passed on the stack.  The function is responsible for cleaning up the stack
 * before returning.  The return value is placed on the stack by the function.
 *
 * In the binary JIT, the function is called by CALLing the address of the
 * function and then RETing back to the return address.  Because the JIT uses
 * the actual processor stack, arguments must still be pushed onto the stack.
 * The calling convention, however, necessitates that the return address is
 * pushed onto the stack *after* the arguments.  The functionality is then as
 * follows:
 *   1. The caller saves the frame pointer (r12).
 *   2. The caller aligns the stack so that, after the arguments are pushed,
 *     the stack is 16-byte aligned.
 *   3. The caller pushes the arguments onto the stack.
 *   4. The caller CALLs the function.
 *   5. Upon return, the caller pops the arguments off the stack.
 *   6. The caller restores the frame pointer (r12).
 * The function (the callee) is not responsible for cleaning the arguments off
 * the stack.  The return value is placed in RAX, like normal.  The function
 * should reference local variables using the frame pointer (r12).  As such,
 * the function must reserve an empty "slot" for the frame pointer when it is
 * analyzing the function, before the compilation step.
 */
typedef struct GTA_Computed_Value_Function {
  /**
   * The base class.
   */
  GTA_Computed_Value base;
  /**
   * The number of arguments expected.
   */
  size_t num_arguments;
  /**
   * The pointer to the function (either bytecode or binary).
   */
  size_t pointer;
} GTA_Computed_Value_Function;

/**
 * Creates a new GTA_Computed_Value_Function object.
 *
 * @param value The GTA_Computed_Value_Function project.
 * @param num_arguments The number of arguments expected.
 * @param pointer The address of the first instruction of the function.
 * @param context The execution context to create the value in.
 * @return The new object.
 */
GTA_Computed_Value_Function * GTA_CALL gta_computed_value_function_create(size_t num_arguments, size_t pointer, GTA_Execution_Context * context);

/**
 * Creates a new GTA_Computed_Value_Function object in place.
 *
 * @param self The memory address of the object.
 * @param num_arguments The number of arguments expected.
 * @param pointer The address of the first instruction of the function.
 * @param context The execution context to create the value in.
 * @return True if the operation was successful, false otherwise.
 */
bool gta_computed_value_function_create_in_place(GTA_Computed_Value_Function * self, size_t num_arguments, size_t pointer, GTA_Execution_Context * context);

/**
 * Destroys a GTA_Computed_Value_Function object.
 *
 * This function should not be called directly. Use gta_computed_value_destroy()
 * instead.
 *
 * @see gta_computed_value_destroy()
 *
 * @param self The GTA_Computed_Value_Function object to destroy.
 */
void gta_computed_value_function_destroy(GTA_Computed_Value * self);

/**
 * Destroys a GTA_Computed_Value_Function object in place.
 * 
 * @param self The memory address of the object.
 */
void gta_computed_value_function_destroy_in_place(GTA_Computed_Value * self);

/**
 * Create a deep copy of a GTA_Computed_Value_Function object.
 *
 * @param self The object.
 * @param context The execution context to create the value in.
 * @return The new object or NULL on failure.
 */
GTA_Computed_Value * GTA_CALL gta_computed_value_function_deep_copy(GTA_Computed_Value * self, GTA_Execution_Context * context);

/**
 * Get a string representation of a GTA_Computed_Value_Function object.
 *
 * The caller is responsible for freeing the returned string with gta_free().
 *
 * @param self The object.
 * @return The string representation or NULL on failure.
 */
char * GTA_CALL gta_computed_value_function_to_string(GTA_Computed_Value * self);

/**
 * Calls the object as a function.
 *
 * @param self The object to call.
 * @param arguments The arguments to call with.
 * @param context The execution context of the program.
 * @return The result of the operation or NULL if the operation failed.
 */
GTA_Computed_Value * gta_computed_value_function_call(GTA_Computed_Value * self, GTA_Computed_Value_Vector * arguments, GTA_Execution_Context * context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_COMPUTED_VALUE_FUNCTION_H
