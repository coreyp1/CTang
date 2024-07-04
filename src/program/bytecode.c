
#include <stdio.h>
#include <cutil/vector.h>
#include <tang/program/bytecode.h>

void gta_bytecode_print(GTA_VectorX * bytecode) {
  GTA_TypeX_Union * current = &bytecode->data[0];
  GTA_TypeX_Union * end = &bytecode->data[bytecode->count];
  while (current < end) {
    switch (GTA_TYPEX_UI(*current)) {
      case GTA_BYTECODE_RETURN:
        printf("%p\tRETURN\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_NULL:
        printf("%p\tNULL\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_BOOLEAN:
        printf("%p\tBOOLEAN\t%s\n", (void *)current, GTA_TYPEX_B(*(current + 1)) ? "true" : "false");
        current += 2;
        break;
      case GTA_BYTECODE_FLOAT:
        printf(GTA_64_BIT ? "%p\tFLOAT\t%lf\n" : "%p\tFLOAT\t%f\n", (void *)current, GTA_TYPEX_F(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_INTEGER:
        printf(GTA_64_BIT ? "%p\tINT\t%ld\n" : "%p\tINT\t%d\n", (void *)current, GTA_TYPEX_I(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_STRING:
        printf("%p\tSTRING\t%p\n", (void *)current, (void *)GTA_TYPEX_P(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_ARRAY:
        printf(GTA_64_BIT ? "%p\tARRAY\t%lu\n" : "%s\tARRAY\t%u", (void *)current, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_CAST:
        printf("%p\tCAST\t%p\n", (void *)current, GTA_TYPEX_P(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_SET_NOT_TEMP:
        printf("%p\tSET_NOT_TEMP\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_ADOPT:
        printf("%p\tADOPT\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_POP:
        printf("%p\tPOP\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_PEEK_GLOBAL:
        printf("%p\tPEEK_GLOBAL\t%lu\n", (void *)current, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_POKE_GLOBAL:
        printf("%p\tPOKE_GLOBAL\t%lu\n", (void *)current, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_PEEK_LOCAL:
        printf("%p\tPEEK_LOCAL\t%lu\n", (void *)current, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_POKE_LOCAL:
        printf("%p\tPOKE_LOCAL\t%lu\n", (void *)current, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_MARK_FP:
        printf("%p\tMARK_FP\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_PUSH_FP:
        printf("%p\tPUSH_FP\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_POP_FP:
        printf("%p\tPOP_FP\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_LOAD_LIBRARY:
        printf("%p\tLOAD_LIBRARY\t%lu\n", (void *)current, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_NEGATIVE:
        printf("%p\tNEGATIVE\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_NOT:
        printf("%p\tNOT\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_ADD:
        printf("%p\tADD\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_SUBTRACT:
        printf("%p\tSUBTRACT\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_MULTIPLY:
        printf("%p\tMULTIPLY\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_DIVIDE:
        printf("%p\tDIVIDE\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_MODULO:
        printf("%p\tMODULO\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_LESS_THAN:
        printf("%p\tLESS_THAN\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_LESS_THAN_EQUAL:
        printf("%p\tLESS_THAN_EQUAL\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_GREATER_THAN:
        printf("%p\tGREATER_THAN\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_GREATER_THAN_EQUAL:
        printf("%p\tGREATER_THAN_EQUAL\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_EQUAL:
        printf("%p\tEQUAL\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_NOT_EQUAL:
        printf("%p\tNOT_EQUAL\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_AND:
        printf("%p\tAND\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_OR:
        printf("%p\tOR\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_JMP:
        printf(GTA_64_BIT ? "%p\tJMP\t%ld\n" : "%p\tJMP\t%d\n", (void *)current, GTA_TYPEX_I(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_JMPF:
        printf(GTA_64_BIT ? "%p\tJMPF\t%ld\n" : "%p\tJMPF\t%d\n", (void *)current, GTA_TYPEX_I(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_JMPT:
        printf(GTA_64_BIT ? "%p\tJMPT\t%ld\n" : "%p\tJMPT\t%d\n", (void *)current, GTA_TYPEX_I(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_PRINT:
        printf("%p\tPRINT\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_INDEX:
        printf("%p\tINDEX\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_SLICE:
        printf("%p\tSLICE\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_ASSIGN_INDEX:
        printf("%p\tASSIGN_INDEX\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_ITERATOR:
        printf("%p\tITERATOR\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_ITERATOR_NEXT:
        printf("%p\tITERATOR_NEXT\n", (void *)current);
        ++current;
        break;
      default:
        printf("%p\tUnknown\n", (void *)current);
        ++current;
        break;
    }
  }
}
