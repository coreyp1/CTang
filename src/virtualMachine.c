
#include <stdio.h>
#include "tang/virtualMachine.h"
#include "tang/computedValueAll.h"

bool gta_virtual_machine_execute_bytecode(GTA_Context* context, GTA_Program * program) {
  if (!context || !program || !program->bytecode) {
    return false;
  }
  GCU_Type64_Union * current = program->bytecode->data;
  GCU_Type64_Union * next = current;
  // Unlike x86, the stack grows upwards.
  // Push the "old" base pointer, which is in this case, 0, will signal the
  // "return" instruction to halt the program.
  if (!gcu_vector64_append(context->stack, GCU_TYPE64_UI64(0))) {
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
    current = next;
    ++next;
    switch ((*current).ui64) {
      case GTA_BYTECODE_RETURN: {
        context->result = context->stack->data[--*sp].p;
        // Pop the base pointer.
        bp = context->stack->data[--*sp].ui64;
        // Pop the return address.
        // Because the return address is the last entry of the calling frame,
        // and it is possible that we are in the outermost frame, then we
        // need to verify that we are not reading into memory that doesn't
        // belong to us.
        next = *sp > 0
          ? context->stack->data[--*sp].p
          : 0;
        break;
      }
      case GTA_BYTECODE_NULL: {
        if (!gcu_vector64_append(context->stack, GCU_TYPE64_P(gta_computed_value_null))) {
          context->result = gta_computed_value_error_out_of_memory;
        }
        break;
      }
    }
  }
  return true;
}
