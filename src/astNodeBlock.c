
#include <stdio.h>
#include <string.h>
#include <cutil/memory.h>
#include <tang/astNodeBlock.h>
#include <tang/computedValue.h>
#include <tang/binaryCompilerContext.h>

GTA_Ast_Node_VTable gta_ast_node_block_vtable = {
  .name = "Block",
  .compile_to_bytecode = gta_ast_node_block_compile_to_bytecode,
  .compile_to_binary = gta_ast_node_block_compile_to_binary,
  .destroy = gta_ast_node_block_destroy,
  .print = gta_ast_node_block_print,
  .simplify = gta_ast_node_block_simplify,
  .walk = gta_ast_node_block_walk,
};

GTA_Ast_Node_Block * gta_ast_node_block_create(GTA_VectorX * statements, GTA_PARSER_LTYPE location) {
  GTA_Ast_Node_Block * self = gcu_malloc(sizeof(GTA_Ast_Node_Block));
  if (!self) {
    return 0;
  }
  self->base.vtable = &gta_ast_node_block_vtable;
  self->base.location = location;
  self->base.possible_type = GTA_AST_POSSIBLE_TYPE_UNKNOWN;
  self->statements = statements;
  return self;
}

void gta_ast_node_block_destroy(GTA_Ast_Node * self) {
  GTA_Ast_Node_Block * block = (GTA_Ast_Node_Block *) self;
  GTA_VECTORX_DESTROY(block->statements);
  gcu_free(self);
}

void gta_ast_node_block_print(GTA_Ast_Node * self, const char * indent) {
  size_t indent_len = strlen(indent);
  char * new_indent = gcu_malloc(indent_len + 3);
  if (!new_indent) {
    return;
  }
  memcpy(new_indent, indent, indent_len + 1);
  memcpy(new_indent + indent_len, "  ", 3);
  printf("%s%s:\n", indent, self->vtable->name);
  GTA_Ast_Node_Block * block = (GTA_Ast_Node_Block *) self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(block->statements); ++i) {
    GTA_Ast_Node * statement = (GTA_Ast_Node *)GTA_TYPEX_P(block->statements->data[i]);
    gta_ast_node_print(statement, new_indent);
  }
  gcu_free(new_indent);
}

GTA_Ast_Node * gta_ast_node_block_simplify(GTA_Ast_Node * self, GTA_Ast_Simplify_Variable_Map * variable_map) {
  GTA_Ast_Node_Block * block = (GTA_Ast_Node_Block *) self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(block->statements); ++i) {
    GTA_Ast_Node * simplified_statement = gta_ast_node_simplify((GTA_Ast_Node *)GTA_TYPEX_P(block->statements->data[i]), variable_map);
    if (simplified_statement) {
      gta_ast_node_destroy((GTA_Ast_Node *)GTA_TYPEX_P(block->statements->data[i]));
      GTA_TYPEX_P(block->statements->data[i]) = simplified_statement;
    }
  }
  return 0;
}

void gta_ast_node_block_walk(GTA_Ast_Node * self, GTA_Ast_Node_Walk_Callback callback, void * data, void * return_value) {
  callback(self, data, return_value);
  GTA_Ast_Node_Block * block = (GTA_Ast_Node_Block *) self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(block->statements); ++i) {
    GTA_Ast_Node * statement = (GTA_Ast_Node *)GTA_TYPEX_P(block->statements->data[i]);
    gta_ast_node_walk(statement, callback, data, return_value);
  }
}

bool gta_ast_node_block_compile_to_bytecode(GTA_Ast_Node * self, GTA_Bytecode_Compiler_Context * context) {
  GTA_Ast_Node_Block * block = (GTA_Ast_Node_Block *) self;
  if (!GTA_VECTORX_COUNT(block->statements)) {
    return GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count) && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_NULL));
  }
  for (size_t i = 0; i < GTA_VECTORX_COUNT(block->statements); ++i) {
    GTA_Ast_Node * statement = (GTA_Ast_Node *)GTA_TYPEX_P(block->statements->data[i]);
    if (!gta_ast_node_compile_to_bytecode(statement, context)) {
      return false;
    }
    if (i < GTA_VECTORX_COUNT(block->statements) - 1) {
      if (!(GTA_BYTECODE_APPEND(context->bytecode_offsets, context->program->bytecode->count)
        && GTA_VECTORX_APPEND(context->program->bytecode, GTA_TYPEX_MAKE_UI(GTA_BYTECODE_POP)))) {
        return false;
      }
    }
  }
  return true;
}

bool gta_ast_node_block_compile_to_binary(GTA_Ast_Node * self, GTA_Binary_Compiler_Context * context) {
  GTA_Ast_Node_Block * block = (GTA_Ast_Node_Block *) self;
  for (size_t i = 0; i < GTA_VECTORX_COUNT(block->statements); ++i) {
    GTA_Ast_Node * statement = (GTA_Ast_Node *)GTA_TYPEX_P(block->statements->data[i]);
    if (!gta_ast_node_compile_to_binary(statement, context)) {
      return false;
    }
    if (i < GTA_VECTORX_COUNT(block->statements) - 1) {
      if (!gcu_vector8_reserve(context->binary_vector, context->binary_vector->count + 15)) {
        return false;
      }
      // The result of the previous operation is in rax.  We need to delete it.
      //   mov rdi, rax
      GTA_BINARY_WRITE3(context->binary_vector, 0x48, 0x89, 0xC7);
      //   mov rax, gcu_computed_value_destroy
      GTA_BINARY_WRITE2(context->binary_vector, 0x48, 0xB8);
      GTA_BINARY_WRITE8(context->binary_vector, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);
      GTA_UInteger fp = GTA_JIT_FUNCTION_CONVERTER(gta_computed_value_destroy);
      memcpy(&context->binary_vector->data[context->binary_vector->count - 8], &fp, 8);
      //   call rax
      GTA_BINARY_WRITE2(context->binary_vector, 0xff, 0xd0);
    }
  }
  return true;
}
