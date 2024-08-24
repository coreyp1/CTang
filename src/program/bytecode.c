
#include <assert.h>
#include <stdio.h>
#include <cutil/vector.h>
#include <cutil/memory.h>
#include <tang/program/bytecode.h>
#include <tang/computedValue/computedValue.h>

void gta_bytecode_print(GTA_VectorX * bytecode) {
  assert(bytecode);
  GTA_TypeX_Union * current = &bytecode->data[0];
  GTA_TypeX_Union * end = &bytecode->data[bytecode->count];
  GTA_TypeX_Union * start = current;
  assert(current);
  assert(end);
  while (current < end) {
    switch (GTA_TYPEX_UI(*current)) {
      case GTA_BYTECODE_RETURN:
        printf("%4zu RETURN\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_NOP:
        printf("%4zu NOP\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_NULL:
        printf("%4zu NULL\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_BOOLEAN:
        printf("%4zu BOOLEAN\t%s\n", current - start, GTA_TYPEX_B(*(current + 1)) ? "true" : "false");
        current += 2;
        break;
      case GTA_BYTECODE_FLOAT:
        printf(GTA_64_BIT ? "%4zu FLOAT\t%lf\n" : "%4zu FLOAT\t%f\n", current - start, GTA_TYPEX_F(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_INTEGER:
        printf(GTA_64_BIT ? "%4zu INT\t%ld\n" : "%4zu INT\t%d\n", current - start, GTA_TYPEX_I(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_STRING:
        printf("%4zu STRING\t%p\n", current - start, (void *)GTA_TYPEX_P(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_ARRAY:
        printf(GTA_64_BIT ? "%4zu ARRAY\t%lu\n" : "%s\tARRAY\t%u", current - start, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_MAP:
        printf(GTA_64_BIT ? "%4zu MAP\t%lu\n" : "%4zu MAP\t%u\n", current - start, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_CAST:
        printf("%4zu CAST\t%p\n", current - start, GTA_TYPEX_P(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_SET_NOT_TEMP:
        printf("%4zu SET_NOT_TEMP\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_ADOPT:
        printf("%4zu ADOPT\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_POP:
        printf("%4zu POP\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_PEEK_GLOBAL:
        printf(GTA_64_BIT ? "%4zu PEEK_GLOBAL\t%lu\n" : "%4zu PEEK_GLOBAL\t%u\n", current - start, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_POKE_GLOBAL:
        printf(GTA_64_BIT ? "%4zu POKE_GLOBAL\t%lu\n" : "%4zu POKE_GLOBAL\t%u\n", current - start, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_PEEK_LOCAL:
        printf(GTA_64_BIT ? "%4zu PEEK_LOCAL\t%lu\n" : "%4zu PEEK_LOCAL\t%u\n", current - start, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_POKE_LOCAL:
        printf(GTA_64_BIT ? "%4zu POKE_LOCAL\t%lu\n" : "%4zu POKE_LOCAL\t%u\n", current - start, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_MARK_FP:
        printf("%4zu MARK_FP\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_PUSH_FP:
        printf("%4zu PUSH_FP\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_POP_FP:
        printf("%4zu POP_FP\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_LOAD: {
        GTA_Computed_Value * value = GTA_TYPEX_P(*(current + 1));
        char * output = gta_computed_value_to_string(value);
        printf(GTA_64_BIT ? "%4zu LOAD\t%lu\t%s\n" : "%4zu LOAD\t%u\t%s\n", current - start, GTA_TYPEX_UI(*(current + 1)), output);
        gcu_free(output);
        current += 2;
        break;
      }
      case GTA_BYTECODE_LOAD_LIBRARY:
        printf(GTA_64_BIT ? "%4zu LOAD_LIBRARY\t%lu\n" : "%4zu LOAD_LIBRARY\t%u\n", current - start, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_NEGATIVE:
        printf("%4zu NEGATIVE\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_NOT:
        printf("%4zu NOT\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_ADD:
        printf("%4zu ADD\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_SUBTRACT:
        printf("%4zu SUBTRACT\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_MULTIPLY:
        printf("%4zu MULTIPLY\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_DIVIDE:
        printf("%4zu DIVIDE\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_MODULO:
        printf("%4zu MODULO\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_LESS_THAN:
        printf("%4zu LESS_THAN\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_LESS_THAN_EQUAL:
        printf("%4zu LESS_THAN_EQUAL\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_GREATER_THAN:
        printf("%4zu GREATER_THAN\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_GREATER_THAN_EQUAL:
        printf("%4zu GREATER_THAN_EQUAL\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_EQUAL:
        printf("%4zu EQUAL\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_NOT_EQUAL:
        printf("%4zu NOT_EQUAL\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_AND:
        printf("%4zu AND\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_OR:
        printf("%4zu OR\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_JMP:
        printf(GTA_64_BIT ? "%4zu JMP\t%ld\n" : "%4zu JMP\t%d\n", current - start, GTA_TYPEX_I(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_JMPF:
        printf(GTA_64_BIT ? "%4zu JMPF\t%ld\n" : "%4zu JMPF\t%d\n", current - start, GTA_TYPEX_I(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_JMPT:
        printf(GTA_64_BIT ? "%4zu JMPT\t%ld\n" : "%4zu JMPT\t%d\n", current - start, GTA_TYPEX_I(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_PRINT:
        printf("%4zu PRINT\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_INDEX:
        printf("%4zu INDEX\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_PERIOD:
        printf("%4zu PERIOD\t%p (%s)\n", current - start, GTA_TYPEX_P(*(current + 1)), (char *)GTA_TYPEX_P(*(current + 2)));
        current += 3;
        break;
      case GTA_BYTECODE_SLICE:
        printf("%4zu SLICE\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_ASSIGN_INDEX:
        printf("%4zu ASSIGN_INDEX\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_ITERATOR:
        printf("%4zu ITERATOR\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_ITERATOR_NEXT:
        printf("%4zu ITERATOR_NEXT\n", current - start);
        ++current;
        break;
      case GTA_BYTECODE_CALL:
        printf(GTA_64_BIT ? "%4zu CALL\t%lu\n" : "%4zu CALL\t%u\n", current - start, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      default:
        printf("%4zu Unknown\n", current - start);
        ++current;
        break;
    }
  }
}
