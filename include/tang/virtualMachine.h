/**
 * @file
 *
 * The virtual machine is used to execute the bytecode, which is the fallback
 * in the event that the JIT compiler is not available for the target platform.
 */

#ifndef G_TANG_VIRTUAL_MACHINE_H
#define G_TANG_VIRTUAL_MACHINE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "tang/bytecode.h"
#include "tang/context.h"
#include "tang/computedValue.h"

GTA_Computed_Value * gta_virtual_machine_execute_bytecode(GTA_Context* context, GTA_Bytecode* bytecode);








#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_VIRTUAL_MACHINE_H
