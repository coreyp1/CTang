/**
 * @file
 * Declare the Opcodes used in the Bytecode representation of a program.
 */

#ifndef G_TANG_BYTECODE_H
#define G_TANG_BYTECODE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <cutil/vector.h>
#include <tang/macros.h>

/**
 * All possible Opcodes that may be used in the Bytecode.
 *
 * For S/I suffixed Opcodes, (S) indicates that a value will be popped from
 * the stack, and (I) indicates that an index is provided in the bytecode, to
 * be referenced from the frame pointer (fp) during execution.
 *
 * The S/I order in the Opcode indicates the nature (stack or index) of the
 * associated values.  For example, ADD_SS will add a lhs + rhs.  The lhs
 * will have been pushed onto the stack first, followed by the rhs.
 */
typedef enum GTA_Bytecode {
  GTA_BYTECODE_RETURN,         ///< Restore fp, restore pc. Does not pop.
  GTA_BYTECODE_NOP,            ///< Do nothing.  For debugging.
  GTA_BYTECODE_BOOLEAN,        ///< Push a boolean onto the stack
  GTA_BYTECODE_FLOAT,          ///< Push a floating point number onto the stack
  GTA_BYTECODE_INTEGER,        ///< Push an integer onto the stack
  GTA_BYTECODE_NULL,           ///< Push a null onto the stack
  GTA_BYTECODE_STRING,         ///< Get len, char string: push string
  GTA_BYTECODE_ARRAY,          ///< Get len, pop `len` items, putting them into
                               ///<   an array, expect the items were pushed in
                               ///<   order.
  GTA_BYTECODE_MAP,            ///< Get len, pop `len` value then key pairs,
                               ///<   putting them into a map.
  GTA_BYTECODE_CAST,           ///< Get type, pop val, push type(val)
  GTA_BYTECODE_SET_NOT_TEMP,   ///< Set the top of the stack to not be a temporary value
  GTA_BYTECODE_ADOPT,          ///< Pop val. Val will be adopted by the next
                               ///<   operation.  If temp, set not temp.  Otherwise,
                               ///<   copy val.

  GTA_BYTECODE_POP,            ///< Pop a val
  GTA_BYTECODE_PUSH_BP,        ///< Push the base pointer onto the stack
  GTA_BYTECODE_PUSH_PC,        ///< Push the program counter onto the stack
  GTA_BYTECODE_POP_BP,         ///< Pop the base pointer from the stack
  GTA_BYTECODE_POP_PC,         ///< Pop the program counter from the stack
  GTA_BYTECODE_PEEK_GLOBAL,    ///< Stack # (from bp): push val from stack #
  GTA_BYTECODE_POKE_GLOBAL,    ///< Stack # (from bp): Copy a val, store @ stack #
  GTA_BYTECODE_PEEK_LOCAL,     ///< Stack # (from fp): push val from stack #
  GTA_BYTECODE_POKE_LOCAL,     ///< Stack # (from fp): Copy a val, store @ stack #
  GTA_BYTECODE_MARK_FP,        ///< Mark the current stack pointer as the frame pointer
  GTA_BYTECODE_PUSH_FP,        ///< Push the frame pointer onto the stack
  GTA_BYTECODE_POP_FP,         ///< Pop the frame pointer from the stack
  GTA_BYTECODE_LOAD,           ///< Get pointer, push on stack
  GTA_BYTECODE_LOAD_LIBRARY,   ///< Get identifier hash and load a library value
  GTA_BYTECODE_NEGATIVE,       ///< Perform a negation
  GTA_BYTECODE_NOT,            ///< Perform a logical not
  GTA_BYTECODE_ADD,            ///< Perform an addition
  GTA_BYTECODE_SUBTRACT,       ///< Perform a subtraction
  GTA_BYTECODE_MULTIPLY,       ///< Perform a multiplication
  GTA_BYTECODE_DIVIDE,         ///< Perform a division
  GTA_BYTECODE_MODULO,         ///< Perform a modulo
  GTA_BYTECODE_LESS_THAN,      ///< Perform a less than comparison
  GTA_BYTECODE_LESS_THAN_EQUAL,///< Perform a less than or equal comparison
  GTA_BYTECODE_GREATER_THAN,   ///< Perform a greater than comparison
  GTA_BYTECODE_GREATER_THAN_EQUAL,///< Perform a greater than or equal comparison
  GTA_BYTECODE_EQUAL,          ///< Perform an equality comparison
  GTA_BYTECODE_NOT_EQUAL,      ///< Perform an inequality comparison
  GTA_BYTECODE_AND,            ///< Perform a logical and
  GTA_BYTECODE_OR,             ///< Perform a logical or
  GTA_BYTECODE_JMP,            ///< PC offset: set pc + offset
  GTA_BYTECODE_JMPF,           ///< PC offset: pop val, if false, set pc + offset
  GTA_BYTECODE_JMPT,           ///< PC offset: pop val, if true, set pc + offset
  GTA_BYTECODE_PRINT,          ///< Pop val, print(val), push error or NULL
  GTA_BYTECODE_INDEX,          ///< Pop index, pop collection, push collection[index]
  GTA_BYTECODE_SLICE,          ///< Pop skip, pop end, pop begin, pop collection,
                               ///<   push collection[begin:end:skip]
  GTA_BYTECODE_ASSIGN_INDEX,   ///< Pop value, pop index, pop collection,
                               ///<   push (collection[index] = value)
  GTA_BYTECODE_ITERATOR,       ///< Pop a collection, push an iterator, push true
                               ///<   if the iterator is valid, false otherwise
  GTA_BYTECODE_ITERATOR_NEXT,  ///< Pop an iterator, push the next value, push true
                               ///<   if there is a next value, false otherwise
  GTA_BYTECODE_CALL,           ///< argc: pop a function, prepare and call function
} GTA_Bytecode;

/**
 * Print the given bytecode to stdout.
 *
 * @param bytecode The bytecode to print.
 */
void gta_bytecode_print(GTA_VectorX * bytecode);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // G_TANG_BYTECODE_H

