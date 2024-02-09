
#include <stdio.h>
#include <cutil/vector.h>
#include "tang/bytecode.h"

void gta_bytecode_print(GTA_Bytecode_Vector * bytecode) {
  GCU_Type64_Union * current = &bytecode->data[0];
  GCU_Type64_Union * end = &bytecode->data[bytecode->count];
  while (current < end) {
    switch ((*current).ui64) {
      case GTA_BYTECODE_RETURN:
        printf("%p\tRETURN\n", (void *)current);
        ++current;
        break;
      case GTA_BYTECODE_NULL:
        printf("%p\tNULL\n", (void *)current);
        ++current;
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
