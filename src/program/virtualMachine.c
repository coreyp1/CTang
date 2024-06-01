
#include <stdio.h>
#include <cutil/memory.h>
#include <tang/program/bytecode.h>
#include <tang/program/virtualMachine.h>
#include <tang/computedValue/computedValueAll.h>

bool gta_virtual_machine_execute_bytecode(GTA_Execution_Context* context) {
  if (!context || !context->program || !context->program->bytecode) {
    return false;
  }
  // Only the bytecode interpreter uses the bp_stack and pc_stack, so
  // initialize them here.
  if (!(context->pc_stack = GTA_VECTORX_CREATE(32))) {
    return false;
  }
  GTA_TypeX_Union * current = context->program->bytecode->data;
  GTA_TypeX_Union * next = current;
  // Note that the stack pointer is the count of the stack, not the index of
  // the top of the stack.  This is done for effieciency reasons.  Otherwise,
  // we would have to maintain a separate variable for the stack pointer and
  // update both of them on every push and pop.
  size_t * const sp = &context->stack->count;

  // Execute the bytecode.
  while (next) {
    current = next++;
    switch (GTA_TYPEX_UI(*current)) {
      case GTA_BYTECODE_RETURN: {
        context->result = GTA_TYPEX_P(context->stack->data[--*sp]);
        // Pop the return address.
        next = context->pc_stack->count > 0
          ? GTA_TYPEX_P(context->pc_stack->data[--context->pc_stack->count])
          : 0;
        break;
      }
      case GTA_BYTECODE_BOOLEAN: {
        // Use boolean singletons to avoid memory allocation.
        if (!GTA_VECTORX_APPEND(context->stack, GTA_TYPEX_MAKE_P(GTA_TYPEX_B(*(next++))
          ? gta_computed_value_boolean_true
          : gta_computed_value_boolean_false))) {
          context->result = gta_computed_value_error_out_of_memory;
        }
        break;
      }
      case GTA_BYTECODE_FLOAT: {
        GTA_Computed_Value_Float * float_value = gta_computed_value_float_create(GTA_TYPEX_F(*next), context);
        if (!float_value || !GTA_VECTORX_APPEND(context->stack, GTA_TYPEX_MAKE_P(float_value))) {
          context->result = gta_computed_value_error_out_of_memory;
        }
        ++next;
        break;
      }
      case GTA_BYTECODE_INTEGER: {
        GTA_Computed_Value_Integer * integer = gta_computed_value_integer_create(GTA_TYPEX_I(*next), context);
        if (!integer || !GTA_VECTORX_APPEND(context->stack, GTA_TYPEX_MAKE_P(integer))) {
          context->result = gta_computed_value_error_out_of_memory;
        }
        ++next;
        break;
      }
      case GTA_BYTECODE_NULL: {
        if (!GTA_VECTORX_APPEND(context->stack, GTA_TYPEX_MAKE_P(gta_computed_value_null))) {
          context->result = gta_computed_value_error_out_of_memory;
        }
        break;
      }
      case GTA_BYTECODE_STRING: {
        GTA_Computed_Value_String * string = gta_computed_value_string_create(GTA_TYPEX_P(*next++), false, context);
        if (!string || !GTA_VECTORX_APPEND(context->stack, GTA_TYPEX_MAKE_P(string))) {
          context->result = gta_computed_value_error_out_of_memory;
        }
        break;
      }
      case GTA_BYTECODE_SET_NOT_TEMP: {
        // Set the top of the stack to not be temporary.
        GTA_Computed_Value * value = GTA_TYPEX_P(context->stack->data[*sp-1]);
        value->is_temporary = false;
        break;
      }
      case GTA_BYTECODE_POP: {
        // Simply decrease the stack pointer.  The garbage collector will take
        // care of the rest.
        --*sp;
        break;
      }
      case GTA_BYTECODE_PEEK_GLOBAL: {
        // Push a value on the stack, indexed by the base pointer.
        size_t index = GTA_TYPEX_UI(*next++);
        if (!GTA_VECTORX_APPEND(context->stack, context->stack->data[index])) {
          context->result = gta_computed_value_error_out_of_memory;
        }
        break;
      }
      case GTA_BYTECODE_POKE_GLOBAL: {
        // Poke a value into the stack, indexed by the base pointer.
        size_t index = GTA_TYPEX_UI(*next++);
        context->stack->data[index] = context->stack->data[--*sp];
        GTA_Computed_Value * value = GTA_TYPEX_P(context->stack->data[index]);
        value->is_temporary = false;
        break;
      }
      case GTA_BYTECODE_PEEK_LOCAL: {
        // Push a value on the stack, indexed by the frame pointer.
        size_t index = GTA_TYPEX_UI(*next++);
        if (!GTA_VECTORX_APPEND(context->stack, context->stack->data[context->fp + index])) {
          context->result = gta_computed_value_error_out_of_memory;
        }
        break;
      }
      case GTA_BYTECODE_POKE_LOCAL: {
        // Poke a value into the stack, indexed by the frame pointer.
        size_t index = GTA_TYPEX_UI(*next++);
        context->stack->data[context->fp + index] = context->stack->data[--*sp];
        break;
      }
      case GTA_BYTECODE_MARK_FP: {
        // Mark the current stack pointer as the frame pointer.
        context->fp = *sp;
        break;
      }
      case GTA_BYTECODE_PUSH_FP: {
        // Push the frame pointer onto the stack.
        if (!GTA_VECTORX_APPEND(context->stack, GTA_TYPEX_MAKE_UI(context->fp))) {
          context->result = gta_computed_value_error_out_of_memory;
        }
        break;
      }
      case GTA_BYTECODE_POP_FP: {
        // Pop the frame pointer from the stack.
        context->fp = GTA_TYPEX_UI(context->stack->data[--*sp]);
        break;
      }
      case GTA_BYTECODE_LOAD_LIBRARY: {
        // Load a library value.
        // The value will be left on the stack.
        GTA_HashX_Value func = GTA_HASHX_GET(context->globals, GTA_TYPEX_UI(*next++));
        GTA_Computed_Value * library_value = func.exists
          ? (GTA_Function_Converter){.b = GTA_TYPEX_P(func.value)}.f(context)
          : gta_computed_value_null;
        if (!library_value) {
          context->result = gta_computed_value_error_out_of_memory;
        }
        if (!library_value->is_singleton && library_value->is_temporary) {
          // This is an assignment, so make sure that it is not temporary.
          library_value->is_temporary = false;
        }
        if (!GTA_VECTORX_APPEND(context->stack, GTA_TYPEX_MAKE_P(library_value))) {
          context->result = gta_computed_value_error_out_of_memory;
        }
        break;
      }
      case GTA_BYTECODE_NEGATIVE: {
        // Perform a negation.
        // The value will be left on the stack.
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(gta_computed_value_negative(context->stack->data[*sp-1].p));
        break;
      }
      case GTA_BYTECODE_NOT: {
        // Perform a logical not.
        // The value will be left on the stack.
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(gta_computed_value_logical_not(context->stack->data[*sp-1].p));
        break;
      }
      case GTA_BYTECODE_ADD: {
        // Perform an addition.
        // The value will be left on the stack.
        GTA_Computed_Value * rhs = GTA_TYPEX_P(context->stack->data[--*sp]);
        GTA_Computed_Value * lhs = GTA_TYPEX_P(context->stack->data[*sp-1]);
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(gta_computed_value_add(lhs, rhs, context));
        break;
      }
      case GTA_BYTECODE_SUBTRACT: {
        // Perform a subtraction.
        // The value will be left on the stack.
        GTA_Computed_Value * rhs = GTA_TYPEX_P(context->stack->data[--*sp]);
        GTA_Computed_Value * lhs = GTA_TYPEX_P(context->stack->data[*sp-1]);
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(gta_computed_value_subtract(lhs, rhs, context));
        break;
      }
      case GTA_BYTECODE_MULTIPLY: {
        // Perform a multiplication.
        // The value will be left on the stack.
        GTA_Computed_Value * rhs = GTA_TYPEX_P(context->stack->data[--*sp]);
        GTA_Computed_Value * lhs = GTA_TYPEX_P(context->stack->data[*sp-1]);
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(gta_computed_value_multiply(lhs, rhs, context));
        break;
      }
      case GTA_BYTECODE_DIVIDE: {
        // Perform a division.
        // The value will be left on the stack.
        GTA_Computed_Value * rhs = GTA_TYPEX_P(context->stack->data[--*sp]);
        GTA_Computed_Value * lhs = GTA_TYPEX_P(context->stack->data[*sp-1]);
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(gta_computed_value_divide(lhs, rhs, context));
        break;
      }
      case GTA_BYTECODE_MODULO: {
        // Perform a modulo.
        // The value will be left on the stack.
        GTA_Computed_Value * rhs = GTA_TYPEX_P(context->stack->data[--*sp]);
        GTA_Computed_Value * lhs = GTA_TYPEX_P(context->stack->data[*sp-1]);
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(gta_computed_value_modulo(lhs, rhs, context));
        break;
      }
      case GTA_BYTECODE_LESS_THAN: {
        // Perform a less than comparison.
        // The value will be left on the stack.
        GTA_Computed_Value * rhs = GTA_TYPEX_P(context->stack->data[--*sp]);
        GTA_Computed_Value * lhs = GTA_TYPEX_P(context->stack->data[*sp-1]);
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(gta_computed_value_less_than(lhs, rhs, context));
        break;
      }
      case GTA_BYTECODE_LESS_THAN_EQUAL: {
        // Perform a less than or equal comparison.
        // The value will be left on the stack.
        GTA_Computed_Value * rhs = GTA_TYPEX_P(context->stack->data[--*sp]);
        GTA_Computed_Value * lhs = GTA_TYPEX_P(context->stack->data[*sp-1]);
        GTA_Computed_Value * result = gta_computed_value_equal(lhs, rhs, context);
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(GTA_COMPUTED_VALUE_IS_ERROR(result) || (result == gta_computed_value_boolean_true)
          ? result
          : gta_computed_value_less_than(lhs, rhs, context));
        break;
      }
      case GTA_BYTECODE_GREATER_THAN: {
        // Perform a greater than comparison.
        // The value will be left on the stack.
        GTA_Computed_Value * rhs = GTA_TYPEX_P(context->stack->data[--*sp]);
        GTA_Computed_Value * lhs = GTA_TYPEX_P(context->stack->data[*sp-1]);

        GTA_Computed_Value * result = gta_computed_value_equal(lhs, rhs, context);
        if (!GTA_COMPUTED_VALUE_IS_ERROR(result)) {
          if (result == gta_computed_value_boolean_true) {
            result = gta_computed_value_boolean_false;
          }
          else {
            result = gta_computed_value_less_than(lhs, rhs, context);
            if (!GTA_COMPUTED_VALUE_IS_ERROR(result)) {
              result = result == gta_computed_value_boolean_true
                ? gta_computed_value_boolean_false
                : gta_computed_value_boolean_true;
            }
          }
         }
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(result);
        break;
      }
      case GTA_BYTECODE_GREATER_THAN_EQUAL: {
        // Perform a greater than or equal comparison.
        // The value will be left on the stack.
        GTA_Computed_Value * rhs = GTA_TYPEX_P(context->stack->data[--*sp]);
        GTA_Computed_Value * lhs = GTA_TYPEX_P(context->stack->data[*sp-1]);

        GTA_Computed_Value * result = gta_computed_value_equal(lhs, rhs, context);
        if (!GTA_COMPUTED_VALUE_IS_ERROR(result) || (result == gta_computed_value_boolean_false)) {
          result = gta_computed_value_less_than(lhs, rhs, context);
          if (!GTA_COMPUTED_VALUE_IS_ERROR(result)) {
            result = result == gta_computed_value_boolean_true
              ? gta_computed_value_boolean_false
              : gta_computed_value_boolean_true;
          }
         }
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(result);
        break;
      }
      case GTA_BYTECODE_EQUAL: {
        // Perform an equality comparison.
        // The value will be left on the stack.
        GTA_Computed_Value * rhs = GTA_TYPEX_P(context->stack->data[--*sp]);
        GTA_Computed_Value * lhs = GTA_TYPEX_P(context->stack->data[*sp-1]);
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(gta_computed_value_equal(lhs, rhs, context));
        break;
      }
      case GTA_BYTECODE_NOT_EQUAL: {
        // Perform an inequality comparison.
        // The value will be left on the stack.
        GTA_Computed_Value * rhs = GTA_TYPEX_P(context->stack->data[--*sp]);
        GTA_Computed_Value * lhs = GTA_TYPEX_P(context->stack->data[*sp-1]);
        GTA_Computed_Value * result = gta_computed_value_equal(lhs, rhs, context);
        context->stack->data[*sp-1] = GTA_TYPEX_MAKE_P(GTA_COMPUTED_VALUE_IS_BOOLEAN(result)
          ? (result == gta_computed_value_boolean_true)
            ? gta_computed_value_boolean_false
            : gta_computed_value_boolean_true
          : GTA_COMPUTED_VALUE_IS_ERROR(result)
            ? result
            : gta_computed_value_error_not_supported);
        break;
      }
      default: {
        context->result = gta_computed_value_error_invalid_bytecode;
        break;
      }
    }
  }
  GTA_VECTORX_DESTROY(context->pc_stack);
  context->pc_stack = 0;

  return true;
}
