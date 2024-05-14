
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
      case GTA_BYTECODE_POP:
        printf("%p\tPOP\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_ASSIGN_TO_BASE:
        printf("%p\tASSIGN_TO_BASE\t%lu\n", (void *)current, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_ASSIGN:
        printf("%p\tASSIGN\t%lu\n", (void *)current, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_LOAD_LIBRARY:
        printf("%p\tLOAD_LIBRARY\t%lu\n", (void *)current, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_PEEK_BP:
        printf("%p\tPEEK_BP\t%lu\n", (void *)current, GTA_TYPEX_UI(*(current + 1)));
        current += 2;
        break;
      default:
        printf("%p\tUnknown\n", (void *)current);
        ++current;
        break;
    }
  }
}
