/**
 * @file
 *
 * The virtual machine is used to execute the bytecode, which is the fallback
 * in the event that the JIT compiler is not available for the target platform.
 */

#ifndef GHOTI_IO_GTA_PROGRAM_VIRTUALMACHINE_H
#define GHOTI_IO_GTA_PROGRAM_VIRTUALMACHINE_H

#include <ghoti.io/tang/macros.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <ghoti.io/tang/program/executionContext.h>
#include <ghoti.io/tang/computedValue/computedValue.h>
#include <ghoti.io/tang/program/program.h>

GTA_API bool gta_virtual_machine_execute_bytecode(GTA_Execution_Context* context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GHOTI_IO_GTA_PROGRAM_VIRTUALMACHINE_H
