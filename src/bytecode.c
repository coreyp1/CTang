
#include <stdio.h>
#include <cutil/vector.h>
#include "tang/bytecode.h"

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
      case GTA_BYTECODE_INTEGER:
        printf(GTA_64_BIT ? "%p\tINT\t%ld\n" : "%p\tINT\t%d\n", (void *)current, GTA_TYPEX_I(*(current + 1)));
        current += 2;
        break;
      case GTA_BYTECODE_POP:
        printf("%p\tPOP\n", (void *)current);
        ++current;
        break;
      default:
        printf("%p\tUnknown\n", (void *)current);
        ++current;
        break;
    }
  }
}
