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

#include <stdint.h>
#include <cutil/float.h>
#include <cutil/vector.h>

/**
 * Declaring the calling convention (for consistency).
 *
 * ARM and ARM64 use the AAPCS calling convention.  The ARM EABI is a
 * standard for ARM architecture processors that defines a common
 * application binary interface for simple, high-performance, and
 * efficient embedded systems with a focus on reducing memory usage.
 *
 * The x86 and x86-64 use the cdecl calling convention.  The cdecl
 * calling convention is the default calling convention for x86 and
 * x86-64 C and C++ programs.  It is used by many C and C++ compilers
 * for the x86 and x86-64 architectures.
 *
 * The Windows API uses the stdcall calling convention.  We want to use
 * the cdecl calling convention for the library, so we need to declare
 * the calling convention for the functions.
 */
#if defined(ARM) || defined(ARM64)
#define GTA_CALL __attribute__((pcs("aapcs")))
#elif defined(_WIN32) || defined(_WIN64)
#define GTA_CALL __cdecl
#else
#define GTA_CALL
#endif

/**
 * A cross-compiler macro for declaring a function as no discard.
 */
#if defined(__cplusplus) && __cplusplus >= 201703L
#define GTA_NO_DISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
#define GTA_NO_DISCARD __attribute__((warn_unused_result))
#else
#define GTA_NO_DISCARD
#endif

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
 * Define the type for integers based on the system architecture.
 */
#if GTA_32_BIT
typedef int32_t GTA_Integer;
typedef uint32_t GTA_UInteger;
#elif GTA_64_BIT
typedef int64_t GTA_Integer;
typedef uint64_t GTA_UInteger;
#else
#error "Unsupported architecture"
#endif

/**
 * Define the type for floats based on the system architecture.
 */
#if GTA_32_BIT
typedef GCU_float32_t GTA_Float;
#elif GTA_64_BIT
typedef GCU_float64_t GTA_Float;
#else
#error "Unsupported architecture"
#endif

/**
 * Define the bytecode vector type based on the system architecture.
 */
#if GTA_32_BIT
typedef GCU_Vector32 GTA_Bytecode_Vector;
#define GTA_VECTORX_CREATE gcu_vector32_create
#define GTA_VECTORX_DESTROY gcu_vector32_destroy
#define GTA_VECTORX_APPEND gcu_vector32_append
#define GTA_VECTORX_COUNT gcu_vector32_count
#define GTA_TYPEX_UI(X) (X).ui32
#define GTA_TYPEX_I(X) (X).i32
#define GTA_TYPEX_F(X) (X).f32
#define GTA_TYPEX_P(X) (X).p
#define GTA_TYPEX_MAKE_UI(X) GCU_TYPE32_UI32(X)
#define GTA_TYPEX_MAKE_I(X) GCU_TYPE32_I32(X)
#define GTA_TYPEX_MAKE_F(X) GCU_TYPE32_F32(X)
#define GTA_TYPEX_MAKE_P(X) GCU_TYPE32_P(X)
#elif GTA_64_BIT
typedef GCU_Vector64 GTA_VectorX;
typedef GCU_Type64_Union GTA_TypeX_Union;
#define GTA_VECTORX_CREATE gcu_vector64_create
#define GTA_VECTORX_DESTROY gcu_vector64_destroy
#define GTA_VECTORX_APPEND gcu_vector64_append
#define GTA_VECTORX_COUNT gcu_vector64_count
#define GTA_TYPEX_UI(X) (X).ui64
#define GTA_TYPEX_I(X) (X).i64
#define GTA_TYPEX_F(X) (X).f
#define GTA_TYPEX_P(X) (X).p
#define GTA_TYPEX_MAKE_UI(X) GCU_TYPE64_UI64(X)
#define GTA_TYPEX_MAKE_I(X) GCU_TYPE64_I64(X)
#define GTA_TYPEX_MAKE_F(X) GCU_TYPE64_F(X)
#define GTA_TYPEX_MAKE_P(X) GCU_TYPE64_P(X)
#else
#error "Unsupported architecture"
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
#define GTA_AST_IS_PARSE_ERROR(X) (((GTA_Ast_Node *) X)->vtable == &gta_ast_node_parse_error_vtable)
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

/**
 * Macros for identifying the computed value type.
 * @{
 */
#define GTA_COMPUTED_VALUE_IS_NULL(X) (((GTA_Computed_Value *) X)->vtable == &gta_computed_value_null_vtable)
#define GTA_COMPUTED_VALUE_IS_ERROR(X) (((GTA_Computed_Value *) X)->vtable == &gta_computed_value_error_vtable
#define GTA_COMPUTED_VALUE_IS_BOOLEAN(X) (((GTA_Computed_Value *) X)->vtable == &gta_computed_value_boolean_vtable)
#define GTA_COMPUTED_VALUE_IS_INTEGER(X) (((GTA_Computed_Value *) X)->vtable == &gta_computed_value_integer_vtable)
#define GTA_COMPUTED_VALUE_IS_FLOAT(X) (((GTA_Computed_Value *) X)->vtable == &gta_computed_value_float_vtable)
#define GTA_COMPUTED_VALUE_IS_STRING(X) (((GTA_Computed_Value *) X)->vtable == &gta_computed_value_string_vtable)
#define GTA_COMPUTED_VALUE_IS_ARRAY(X) (((GTA_Computed_Value *) X)->vtable == &gta_computed_value_array_vtable)
#define GTA_COMPUTED_VALUE_IS_MAP(X) (((GTA_Computed_Value *) X)->vtable == &gta_computed_value_map_vtable)
#define GTA_COMPUTED_VALUE_IS_FUNCTION(X) (((GTA_Computed_Value *) X)->vtable == &gta_computed_value_function_vtable)
#define GTA_COMPUTED_VALUE_IS_LIBRARY(X) (((GTA_Computed_Value *) X)->vtable == &gta_computed_value_library_vtable)
/**
 * @}
 */

/**
 * Macros for dealing with x86 and x86-64 assembly.
 */
#define GTA_X86_RAX

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //GTA_MACROS_H
