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

#include <tang/program/executionContext.h>
#include <tang/computedValue/computedValue.h>
#include <tang/program/program.h>

bool gta_virtual_machine_execute_bytecode(GTA_Execution_Context* context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_VIRTUAL_MACHINE_H
