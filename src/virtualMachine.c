
#include <stdio.h>
#include <tang/virtualMachine.h>
#include <tang/computedValueAll.h>

bool gta_virtual_machine_execute_bytecode(GTA_Execution_Context* context) {
  if (!context || !context->program || !context->program->bytecode) {
    return false;
  }
  GTA_TypeX_Union * current = context->program->bytecode->data;
  GTA_TypeX_Union * next = current;
  // Unlike x86, the stack grows upwards.
  // Push the "old" base pointer, which is in this case, 0, will signal the
  // "return" instruction to halt the program.
  if (!GTA_VECTORX_APPEND(context->stack, GTA_TYPEX_MAKE_UI(0))) {
    context->result = gta_computed_value_error_out_of_memory;
    return false;
  }
  size_t bp = context->stack->count - 1;
  (void)bp;
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
        // Pop the base pointer.
        bp = GTA_TYPEX_UI(context->stack->data[--*sp]);
        // Pop the return address.
        // Because the return address is the last entry of the calling frame,
        // and it is possible that we are in the outermost frame, then we
        // need to verify that we are not reading into memory that doesn't
        // belong to us.
        next = *sp > 0
          ? GTA_TYPEX_P(context->stack->data[--*sp])
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
        GTA_Computed_Value_Float * float_value = gta_computed_value_float_create(GTA_TYPEX_F(*next));
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
        GTA_Computed_Value_String * string = gta_computed_value_string_create(GTA_TYPEX_P(*next++), false);
        if (!string || !GTA_VECTORX_APPEND(context->stack, GTA_TYPEX_MAKE_P(string))) {
          context->result = gta_computed_value_error_out_of_memory;
        }
        break;
      }
      case GTA_BYTECODE_POP: {
        GTA_Computed_Value * value = GTA_TYPEX_P(context->stack->data[--*sp]);
        gta_computed_value_destroy(value);
        break;
      }
      default: {
        context->result = gta_computed_value_error_invalid_bytecode;
        break;
      }
    }
  }
  return true;
}
