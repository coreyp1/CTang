
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
      case GTA_BYTECODE_POP: {
        // Simply decrease the stack pointer.  The garbage collector will take
        // care of the rest.
        --*sp;
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
