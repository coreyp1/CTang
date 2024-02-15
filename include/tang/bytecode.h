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
  GTA_BYTECODE_RETURN,         ///< Get stack #, pop return val, pop (stack #) times,
                               ///<   push val, restore fp, restore pc
  GTA_BYTECODE_BOOLEAN,        ///< Push a boolean onto the stack
  GTA_BYTECODE_FLOAT,          ///< Push a floating point number onto the stack
  GTA_BYTECODE_INTEGER,        ///< Push an integer onto the stack
  GTA_BYTECODE_NULL,           ///< Push a null onto the stack


  GTA_BYTECODE_POP,            ///< Pop a val
  GTA_BYTECODE_PEEK,           ///< Stack # (from fp): push val from stack #
  GTA_BYTECODE_POKE,           ///< Stack # (from fp): Copy a val, store @ stack #
  GTA_BYTECODE_COPY,           ///< Stack # (from fp): Deep copy val @ stack #, store @
                  ///<   stack #
  GTA_BYTECODE_JMP,            ///< PC #: set pc to PC #
  GTA_BYTECODE_JMPF_S,         ///< PC #: read val, if false, set pc to PC #
  GTA_BYTECODE_JMPF_I,         ///< PC #: read val, if false, set pc to PC #
  GTA_BYTECODE_JMPF_POP,       ///< PC #: pop val, if false, set pc to PC #
  GTA_BYTECODE_JMPT_S,         ///< PC #: read val, if true, set pc to PC #
  GTA_BYTECODE_JMPT_I,         ///< PC #: read val, if true, set pc to PC #
  GTA_BYTECODE_JMPT_POP,       ///< PC #: pop val, if true, set pc to PC #
  GTA_BYTECODE_STRING,         ///< Get len, char string: push string
  GTA_BYTECODE_ARRAY,          ///< Get len, pop `len` items, putting them into an array
                  ///<   with the last array item popped first
  GTA_BYTECODE_MAP,            ///< Get len, pop `len` value then key pairs, putting them
                  ///<   into a map
  GTA_BYTECODE_LIBRARY,        ///< Pop name, push Library identified by name
  GTA_BYTECODE_LIBRARYSAVE,    ///< Get index, save top of stack to library[index]
  GTA_BYTECODE_LIBRARYCOPY,    ///< Get index, load from library[index]
  GTA_BYTECODE_FUNCTION,       ///< Get argc, PC#: push function(argc, PC #)
  GTA_BYTECODE_ASSIGNINDEX,    ///< Pop index, pop collection, pop value,
                  ///<   push (collection[index] = value)
  GTA_BYTECODE_ADD_SS,         ///< Push lhs + rhs
  GTA_BYTECODE_ADD_SI,         ///< Push lhs + rhs
  GTA_BYTECODE_ADD_IS,         ///< Push lhs + rhs
  GTA_BYTECODE_ADD_II,         ///< Push lhs + rhs
  GTA_BYTECODE_SUBTRACT_SS,    ///< Push lhs - rhs
  GTA_BYTECODE_SUBTRACT_SI,    ///< Push lhs - rhs
  GTA_BYTECODE_SUBTRACT_IS,    ///< Push lhs - rhs
  GTA_BYTECODE_SUBTRACT_II,    ///< Push lhs - rhs
  GTA_BYTECODE_MULTIPLY_SS,    ///< Push lhs * rhs
  GTA_BYTECODE_MULTIPLY_SI,    ///< Push lhs * rhs
  GTA_BYTECODE_MULTIPLY_IS,    ///< Push lhs * rhs
  GTA_BYTECODE_MULTIPLY_II,    ///< Push lhs * rhs
  GTA_BYTECODE_DIVIDE_SS,      ///< Push lhs / rhs
  GTA_BYTECODE_DIVIDE_SI,      ///< Push lhs / rhs
  GTA_BYTECODE_DIVIDE_IS,      ///< Push lhs / rhs
  GTA_BYTECODE_DIVIDE_II,      ///< Push lhs / rhs
  GTA_BYTECODE_MODULO_SS,      ///< Push lhs % rhs
  GTA_BYTECODE_MODULO_SI,      ///< Push lhs % rhs
  GTA_BYTECODE_MODULO_IS,      ///< Push lhs % rhs
  GTA_BYTECODE_MODULO_II,      ///< Push lhs % rhs
  GTA_BYTECODE_NEGATIVE_S,     ///< Push negative val
  GTA_BYTECODE_NEGATIVE_I,     ///< Push negative val
  GTA_BYTECODE_NOT_S,          ///< Push logical not of val
  GTA_BYTECODE_NOT_I,          ///< Push logical not of val
  GTA_BYTECODE_LT_SS,          ///< Push lhs < rhs
  GTA_BYTECODE_LT_SI,          ///< Push lhs < rhs
  GTA_BYTECODE_LT_IS,          ///< Push lhs < rhs
  GTA_BYTECODE_LT_II,          ///< Push lhs < rhs
  GTA_BYTECODE_LTE_SS,         ///< Push lhs <= rhs
  GTA_BYTECODE_LTE_SI,         ///< Push lhs <= rhs
  GTA_BYTECODE_LTE_IS,         ///< Push lhs <= rhs
  GTA_BYTECODE_LTE_II,         ///< Push lhs <= rhs
  GTA_BYTECODE_GT_SS,          ///< Push lhs > rhs
  GTA_BYTECODE_GT_SI,          ///< Push lhs > rhs
  GTA_BYTECODE_GT_IS,          ///< Push lhs > rhs
  GTA_BYTECODE_GT_II,          ///< Push lhs > rhs
  GTA_BYTECODE_GTE_SS,         ///< Push lhs >= rhs
  GTA_BYTECODE_GTE_SI,         ///< Push lhs >= rhs
  GTA_BYTECODE_GTE_IS,         ///< Push lhs >= rhs
  GTA_BYTECODE_GTE_II,         ///< Push lhs >= rhs
  GTA_BYTECODE_EQ_SS,          ///< Push lhs == rhs
  GTA_BYTECODE_EQ_SI,          ///< Push lhs == rhs
  GTA_BYTECODE_EQ_IS,          ///< Push lhs == rhs
  GTA_BYTECODE_EQ_II,          ///< Push lhs == rhs
  GTA_BYTECODE_NEQ_SS,         ///< Push lhs != rhs
  GTA_BYTECODE_NEQ_SI,         ///< Push lhs != rhs
  GTA_BYTECODE_NEQ_IS,         ///< Push lhs != rhs
  GTA_BYTECODE_NEQ_II,         ///< Push lhs != rhs
  GTA_BYTECODE_PERIOD_SS,      ///< Push lhs.rhs
  GTA_BYTECODE_PERIOD_SI,      ///< Push lhs.rhs
  GTA_BYTECODE_PERIOD_IS,      ///< Push lhs.rhs
  GTA_BYTECODE_PERIOD_II,      ///< Push lhs.rhs
  GTA_BYTECODE_INDEX_SS,       ///< Push collection[index]
  GTA_BYTECODE_INDEX_SI,       ///< Push collection[index]
  GTA_BYTECODE_INDEX_IS,       ///< Push collection[index]
  GTA_BYTECODE_INDEX_II,       ///< Push collection[index]
  GTA_BYTECODE_SLICE,          ///< Pop skip, pop end, pop begin, pop collection,
                  ///<   push collection[begin:end:skip]
  GTA_BYTECODE_GETITERATOR_SI, ///< Save collection iterator to a stack index
  GTA_BYTECODE_GETITERATOR_II, ///< Save collection iterator to a stack index
  GTA_BYTECODE_ITERATORNEXT_II,///< Use the iterator at supplied index to calculate the
                  ///<   next value, then store at 2nd supplied stack index
  GTA_BYTECODE_ISITERATOREND_I,///< Push bool(is val[at stack index] == iterator end)
  GTA_BYTECODE_CASTINTEGER_S,  ///< Typecast to int, push
  GTA_BYTECODE_CASTINTEGER_I,  ///< Typecast to int, push
  GTA_BYTECODE_CASTFLOAT_S,    ///< Typecast to float, push
  GTA_BYTECODE_CASTFLOAT_I,    ///< Typecast to float, push
  GTA_BYTECODE_CASTBOOLEAN_S,  ///< Typecast to boolean, push
  GTA_BYTECODE_CASTBOOLEAN_I,  ///< Typecast to boolean, push
  GTA_BYTECODE_CASTSTRING_S,   ///< Typecast to string, push
  GTA_BYTECODE_CASTSTRING_I,   ///< Typecast to string, push
  GTA_BYTECODE_CALLFUNC,       ///< Get argc, Pop a function, execute function if argc
                  ///<   matches.
  GTA_BYTECODE_CALLFUNC_I,     ///< Get argc, get index, execute function at index if argc
                  ///<   matches.
  GTA_BYTECODE_PRINT_S,        ///< Pop val, print(val), push error or NULL
  GTA_BYTECODE_PRINT_I,        ///< Pop val, print(val), push error or NULL
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

