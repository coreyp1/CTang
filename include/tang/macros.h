/**
 * @file
 *
 * Header file for the macros used throughout the project.
 *
 * Architecture macros are taken from:
 * https://sourceforge.net/p/predef/wiki/Architectures/
 * Endian macros are taken from:
 * https://sourceforge.net/p/predef/wiki/Endianness/
 */

#ifndef GTA_MACROS_H
#define GTA_MACROS_H


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

/**
 * A cross-compiler macro for marking a function parameter as unused.
 */
#define GTA_MAYBE_UNUSED(X) __attribute__((unused)) X

/**
 * A cross-compiler macro for identifying the system is big endian.
 */
#define GTA_BIG_ENDIAN (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

/**
 * A cross-compiler macro for identifying the system is little endian.
 */
#define GTA_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

/**
 * A cross-compiler macro for identifying the system is 32-bit.
 */
#define GTA_32_BIT (__SIZEOF_POINTER__ == 4)

/**
 * A cross-compiler macro for identifying the system is 64-bit.
 */
#define GTA_64_BIT (__SIZEOF_POINTER__ == 8)

/**
 * A cross-compiler macro for identifying the system is x86.
 */
#if defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(__i586__) || defined(__i586) || defined(__i686__) || defined(__i686) || defined(_M_IX86) || defined(__X86__) || defined(_X86_) || defined(__THW_INTEL__) || defined(__I86__) || defined(__INTEL__) || defined(__386)
#define GTA_X86 1
#else
#define GTA_X86 0
#endif

/**
 * A cross-compiler macro for identifying the system is x86_64.
 */
#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(__amd64) || defined(_M_X64) || defined(_M_AMD64)
#define GTA_X86_64 1
#else
#define GTA_X86_64 0
#endif

/**
 * A cross-compiler macro for identifying the system is ARM.
 */
#if defined(__arm__) || defined(__arm) || defined(__thumb__) || defined(__thumb) || defined(_M_ARM) || defined(_M_ARMT) || defined(__TARGET_ARCH_ARM) || defined(__TARGET_ARCH_THUMB) || defined(__TARGET_ARCH_ARM64) || defined(__TARGET_ARCH_THUMB)
#define GTA_ARM 1
#else
#define GTA_ARM 0
#endif

/**
 * A cross-compiler macro for identifying the system is ARM64.
 */
#if defined(__aarch64__) || defined(__aarch64) || defined(__arm64__) || defined(__arm64) || defined(__arm64)
#define GTA_ARM64 1
#else
#define GTA_ARM64 0
#endif

/**
 * A cross-compiler macro for identifying the system is IA64.
 */
#if defined(__ia64__) || defined(_IA64) || defined(__IA64__) || defined(_M_IA64) || defined(__itanium__)
#define GTA_IA64 1
#else
#define GTA_IA64 0
#endif

/**
 * Macros for identifying the AST type.
 *
 * @{
 */
#define GTA_AST_IS_ARRAY(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_array_vtable)
#define GTA_AST_IS_ASSIGN(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_assign_vtable)
#define GTA_AST_IS_BINARY(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_binary_vtable)
#define GTA_AST_IS_BLOCK(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_block_vtable)
#define GTA_AST_IS_BOOLEAN(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_boolean_vtable)
#define GTA_AST_IS_BREAK(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_break_vtable)
#define GTA_AST_IS_CAST(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_cast_vtable)
#define GTA_AST_IS_CONTINUE(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_continue_vtable)
#define GTA_AST_IS_DO_WHILE(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_do_while_vtable)
#define GTA_AST_IS_FLOAT(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_float_vtable)
#define GTA_AST_IS_FOR(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_for_vtable)
#define GTA_AST_IS_FUNCTION(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_function_vtable)
#define GTA_AST_IS_FUNCTION_CALL(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_function_call_vtable)
#define GTA_AST_IS_IDENTIFIER(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_identifier_vtable)
#define GTA_AST_IS_IF_ELSE(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_if_else_vtable)
#define GTA_AST_IS_INDEX(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_index_vtable)
#define GTA_AST_IS_INTEGER(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_integer_vtable)
#define GTA_AST_IS_LIBRARY(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_library_vtable)
#define GTA_AST_IS_MAP(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_map_vtable)
#define GTA_AST_IS_PERIOD(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_period_vtable)
#define GTA_AST_IS_PRINT(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_print_vtable)
#define GTA_AST_IS_RANGED_FOR(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_ranged_for_vtable)
#define GTA_AST_IS_RETURN(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_return_vtable)
#define GTA_AST_IS_SLICE(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_slice_vtable)
#define GTA_AST_IS_STRING(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_string_vtable)
#define GTA_AST_IS_TERNARY(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_ternary_vtable)
#define GTA_AST_IS_UNARY(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_unary_vtable)
#define GTA_AST_IS_USE(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_use_vtable)
#define GTA_AST_IS_WHILE(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_while_vtable)

#define GTA_AST_IS_NUMERIC(X) (GTA_AST_IS_INTEGER(X) || GTA_AST_IS_FLOAT(X))
#define GTA_AST_IS_PRIMITIVE(X) (GTA_AST_IS_NUMERIC(X) || GTA_AST_IS_STRING(X) || GTA_AST_IS_BOOLEAN(X))

/**
 * @}
 */

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_MACROS_H
