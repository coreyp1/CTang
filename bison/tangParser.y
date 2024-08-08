// Declarations can be found here:
// https://www.gnu.org/software/bison/manual/html_node/Decl-Summary.html
// Defines can be found here:
// https://www.gnu.org/software/bison/manual/html_node/_0025define-Summary.html

// Minimum version requirement.
// https://www.gnu.org/software/bison/manual/bison.html#index-_0025require
%require "3.8.2"

// Create a .h file with the proper definitions for the lexer.
// https://www.gnu.org/software/bison/manual/bison.html#index-_0025defines-2
%defines

// "Namespace" the exported symbols.
// https://www.gnu.org/software/bison/manual/html_node/_0025define-Summary.html#index-_0025define-api_002eprefix
%define api.prefix {GTA_Parser_}

// Use union-based values (the default).
// https://www.gnu.org/software/bison/manual/bison.html#index-_0025define-api_002evalue_002etype-union
%define api.value.type union

// Define the location type.
// https://www.gnu.org/software/bison/manual/html_node/Location-Type.html
%define api.location.type {GTA_PARSER_LTYPE}

// Use runtime assertions to verify that variant objects are constructed and
// destroyed properly.
// https://www.gnu.org/software/bison/manual/bison.html#index-_0025define-parse_002eassert
%define parse.assert

// Define the YYDEBUG macro so that debugging facilities are compiled.
// https://www.gnu.org/software/bison/manual/bison.html#Tracing
// https://www.gnu.org/software/bison/manual/bison.html#index-_0025define-parse_002etrace
%define parse.trace

// Provide a more helpful error message.
// https://www.gnu.org/software/bison/manual/bison.html#index-_0025define-parse_002eerror-verbose
%define parse.error verbose

// https://www.gnu.org/software/bison/manual/bison.html#Pure-Calling
%define api.pure

// Additional arguments that yylex() should accept.
// https://www.gnu.org/software/bison/manual/bison.html#index-_0025lex_002dparam-3
// YYSTYPE * yylval_param, YYLTYPE * yylloc_param , yyscan_t yyscanner
//%lex-param { YYSTYPE * yyval_param }
//%lex-param { YYLTYPE * yylloc_param }
%lex-param { yyscan_t scanner }

// Additional arguments that yyparse() should accept.
// https://www.gnu.org/software/bison/manual/bison.html#index-_0025parse_002dparam-3
%parse-param { yyscan_t * scanner }
%parse-param { GTA_Ast_Node * * ast }
%parse-param { GTA_Parser_Error * parseError }

// Add a prefix to token names
// %define api.token.prefix {TOKEN_}
// Because the prefix will be added to a suffix, do not include any whitespace
// between the prefix and the closing '}'
%define api.token.prefix {GTA_PARSER_}

// https://www.gnu.org/software/bison/manual/bison.html#index-_0025token
%token EOF_ 0 "end of code"
%token <int64_t> INTEGER "integer literal"
%token <long double> FLOAT "float literal"
%token <bool> BOOLEAN "boolean literal"
%token <GTA_Parser_Unicode_String> STRING "string literal"
%token <GTA_Parser_Unicode_String> TEMPLATESTRING "template string"
%token STRINGERROR "Malformed String"
%token <GTA_Parser_Unicode_String> IDENTIFIER "identifier"
%token ASSIGN "="
%token PLUS "+"
%token MINUS "-"
%token MULTIPLY "*"
%token DIVIDE "/"
%token MODULO "%"
%token EXCLAMATIONPOINT "!"
%token LPAREN "("
%token RPAREN ")"
%token LESSTHAN "<"
%token LESSTHANEQUAL "<="
%token GREATERTHAN ">"
%token GREATERTHANEQUAL ">="
%token EQUALCOMPARE "=="
%token NOTEQUAL "!="
%token AND "&&"
%token OR "||"
%token LBRACE "{"
%token RBRACE "}"
%token LBRACKET "["
%token RBRACKET "]"
%token IF "if"
%token ELSE "else"
%token DO "do"
%token WHILE "while"
%token FOR "for"
%token AS "as"
%token NULL_ "null"
%token CASTINT "int"
%token CASTFLOAT "float"
%token CASTBOOLEAN "boolean"
%token CASTSTRING "string"
%token USE "use"
%token GLOBAL "global"
%token FUNCTION "function"
%token RETURN "return"
%token BREAK "break"
%token CONTINUE "continue"
%token PRINT "print"
%token QUESTIONMARK "?"
%token COLON ":"
%token SEMICOLON ";"
%token COMMA ","
%token PERIOD "."
%token QUICKPRINTBEGIN "<%="
%token <GTA_Parser_Unicode_String> QUICKPRINTBEGINANDSTRING "template string followed by <%="
%token QUICKPRINTEND "<%= %> closing tag"
%token UNEXPECTEDSCRIPTEND "%>"
%token MEMORYERROR "Out of Memory/Memory Allocation Error"
%token OCTAL_OUT_OF_BOUNDS "Octal literal out of bounds"

// Any %type declarations of non-terminals.
// https://www.gnu.org/software/bison/manual/bison.html#index-_0025type
%type <GTA_Ast_Node *> program
%type <GTA_Ast_Node *> expression
%type <GTA_Ast_Node *> libraryExpression
// vector<GTA_Ast_Node *>
%type <GCU_Vector64 *> statements
// vector<const char *>
%type <GCU_Vector64 *> functionDeclarationArguments
// vector<GTA_Ast_Node *>
%type <GCU_Vector64 *> expressionList
// vector<std::pair<const char *, GTA_Ast_Node *>>
%type <GCU_Vector64 *> mapList
%type <GTA_Ast_Node *> statement
%type <GTA_Ast_Node *> codeBlock
%type <GTA_Ast_Node *> openStatement
%type <GTA_Ast_Node *> closedStatement
%type <GTA_Ast_Node *> optionalExpression
%type <GTA_Ast_Node *> slice

// Precedence rules.
// For guidance, see:
// https://efxa.org/2014/05/17/techniques-for-resolving-common-grammar-conflicts-in-parsers/
// Notice that the order is reversed from:
// https://en.cppreference.com/w/cpp/language/operator_precedence
// Here, rules are in order of lowest to highest precedence.
%right "=" "?" ":"
%left "||"
%left "&&"
%left "==" "!="
%left "<" "<=" ">" ">="
%left "+" "-"
%left "*" "/" "%"
%right UMINUS AS "!"
%left "(" ")" "[" "]" "."

// Destructors allow us to clean up memory when an error is encountered.
// https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
%destructor {
  gcu_free((void *)$$.str);
} IDENTIFIER STRING TEMPLATESTRING QUICKPRINTBEGINANDSTRING
%destructor {
  gcu_vector64_destroy($$);
} statements functionDeclarationArguments expressionList mapList
%destructor {
  gta_ast_node_destroy($$);
} expression libraryExpression statement codeBlock openStatement closedStatement optionalExpression slice


// Code sections.
// https://www.gnu.org/software/bison/manual/bison.html#g_t_0025code-Summary
// `requires` will be included in the .h file.
%code requires {
#include <stdint.h>
#include <cutil/hash.h>
#include <tang/ast/astNode.h>
#include <tang/location.h>
#include <tang/unicodeString.h>

/* An opaque pointer. */
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#define YY_DECL int gta_scanner_get_next_token(YYSTYPE * yylval, YYLTYPE * yylloc, yyscan_t yyscanner)

typedef const char * GTA_Parser_Error;

/**
 * For "string" expressions, we need to store the string type and the string
 */
typedef struct {
  const char * str;
  size_t len;
  GTA_String_Type type;
} GTA_Parser_Unicode_String;

}

// `top` will be included at the top of the .c file, but not in .h.
%code top {
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <cutil/memory.h>
#include "tangParser.h"
#include <tang/macros.h>
#include <tang/tangScanner.h>
#include <tang/ast/astNodeAll.h>

static GTA_Parser_Error ErrorOutOfMemory = "Out of memory/Memory allocation error";
// static GTA_Parser_Error ErrorOctalOutOfBounds = true;
// static GTA_Parser_Error ErrorStringError = true;
// static GTA_Parser_Error ErrorUnexpectedScriptEnd = true;

void GTA_Parser_error(GTA_PARSER_LTYPE * yylloc, yyscan_t * scanner, GTA_Ast_Node * * ast, GTA_Parser_Error * parseError, const char * yymsg);

// We must provide the yylex() function.
// yylex() arguments are defined in the bison .y file.
// It is conceivable that a programmer may want to have multiple compilers in
// the same project.  Each compiler will need its own yylex() function.
// Because yylex() is only defined and used in this file, we can set its
// linkage as "internal", by declaring the function "static", which allows
// each compiler's yylex() to not interfere with that of another linked file.
// https://en.cppreference.com/w/cpp/language/storage_duration
static int GTA_Parser_lex(GTA_PARSER_STYPE * yylval_param, GTA_PARSER_LTYPE * yylloc_param , yyscan_t yyscanner) {
  return gta_scanner_get_next_token(yylval_param, yylloc_param, yyscanner);
}


// Helper cleanup function for vector64 of ast nodes.
static void vector64_ast_node_cleanup(GCU_Vector64 * vector) {
  for (size_t i = 0; i < vector->count; ++i) {
    gta_ast_node_destroy((GTA_Ast_Node *)vector->data[i].p);
  }
}

// Helper cleanup function for vector64 of map pairs.
static void vector64_map_pair_cleanup(GCU_Vector64 * vector) {
  for (size_t i = 0; i < vector->count; ++i) {
    gta_ast_node_destroy((GTA_Ast_Node *)((GTA_Ast_Node_Map_Pair *)vector->data[i].p)->key);
    gta_ast_node_destroy((GTA_Ast_Node *)((GTA_Ast_Node_Map_Pair *)vector->data[i].p)->value);
    gcu_free((void *)vector->data[i].p);
  }
}

#define LOCATION(A, B)              \
  GTA_PARSER_LTYPE location = {     \
    .first_line = A.first_line,     \
    .first_column = A.first_column, \
    .last_line = B.last_line,       \
    .last_column = B.last_column,   \
  }

#define VERIFY(Z)   \
  if (*parseError) { \
    Z = 0;          \
    break;          \
  }

#define VERIFY1(A,Z)      \
  if (*parseError || !A) { \
    Z = 0;                \
    break;                \
  }

#define VERIFY2(A,B,Z)          \
  if (*parseError || !A || !B) { \
    Z = 0;                      \
    break;                      \
  }

#define VERIFY3(A,B,C,Z)              \
  if (*parseError || !A || !B || !C) { \
    Z = 0;                            \
    break;                            \
  }

#define VERIFY4(A,B,C,D,Z)                  \
  if (*parseError || !A || !B || !C || !D) { \
    Z = 0;                                  \
    break;                                  \
  }

#define BINARY_TEMPLATE(X,A,AA,B,BB,Z)     \
  VERIFY2(A,B,Z);                          \
  LOCATION(AA, BB);                        \
  Z = (GTA_Ast_Node *)gta_ast_node_binary_create(A, B, X, location); \
  if (!Z) {                                \
    parseError = &ErrorOutOfMemory;        \
    break;                                 \
  }

#define UNARY_TEMPLATE(X,AA,B,BB,Z)        \
  VERIFY1(B,Z);                            \
  LOCATION(AA, BB);                        \
  Z = (GTA_Ast_Node *)gta_ast_node_unary_create(B, X, location); \
  if (!Z) {                                \
    parseError = &ErrorOutOfMemory;        \
  }

#define CAST_TEMPLATE(X,A,AA,BB,Z)         \
  VERIFY1(A,Z);                            \
  LOCATION(AA, BB);                        \
  Z = (GTA_Ast_Node *)gta_ast_node_cast_create(A, X, location); \
  if (!Z) {                                \
    parseError = &ErrorOutOfMemory;        \
    break;                                 \
  }


// End of Code top section.
}
// The grammar start symbol (non-terminal).
// https://www.gnu.org/software/bison/manual/bison.html#index-_0025start
%start program


%%
// Grammar Section.
// https://efxa.org/2014/05/17/techniques-for-resolving-common-grammar-conflicts-in-parsers/
// https://stackoverflow.com/a/12732388/3821565

// `program` represents every possible syntactically-valid program.
program
  : expression
    {
      *ast = (GTA_Ast_Node *)$1;
    }
  | statements
    {
      // Verify that there have been no memory errors.
      VERIFY1($1,$$);

      *ast = (GTA_Ast_Node *)gta_ast_node_block_create($1, @1);
      if (!*ast) {
        parseError = &ErrorOutOfMemory;
      }
    }
  | EOF_
    {}
  ;

// `functionDeclarationArguments` is a comma-separated list of variable names
// given as part of a function declaration.
functionDeclarationArguments
  : %empty
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      $$ = gcu_vector64_create(0);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
      $$->cleanup = vector64_ast_node_cleanup;
    }
  | IDENTIFIER
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      // Create the identifier object.
      const char * identifier = $1.str;
      GTA_Ast_Node * parameter = (GTA_Ast_Node *)gta_ast_node_identifier_create(identifier, @1);
      if (!parameter) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
        break;
      }

      // Create a vector to hold the identifiers.
      $$ = gcu_vector64_create(1);
      if (!$$) {
        gta_ast_node_destroy(parameter);
        parseError = &ErrorOutOfMemory;
        break;
      }
      $$->cleanup = vector64_ast_node_cleanup;

      gcu_vector64_append($$, GCU_TYPE64_P(parameter));
    }
  | functionDeclarationArguments "," IDENTIFIER
    {
      // Verify that there have been no memory errors.
      VERIFY1($1,$$);

      const char * identifier = $3.str;
      GTA_Ast_Node * parameter = (GTA_Ast_Node *)gta_ast_node_identifier_create(identifier, @3);
      if (!parameter) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
        break;
      }

      if (!gcu_vector64_append($1, GCU_TYPE64_P(parameter))) {
        gta_ast_node_destroy(parameter);
        $$ = 0;
        parseError = &ErrorOutOfMemory;
        break;
      }
      $$ = $1;
    }
  ;

// `expressionList` is a comma-separated list of expressions given as part
// of a function call or an array declaration.
expressionList
  : %empty
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      $$ = gcu_vector64_create(0);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
      $$->cleanup = vector64_ast_node_cleanup;
    }
  | expression
    {
      // Verify that there have been no memory errors.
      VERIFY1($1,$$);

      $$ = gcu_vector64_create(1);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
      $$->cleanup = vector64_ast_node_cleanup;
      gcu_vector64_append($$, GCU_TYPE64_P($1));
    }
  | expressionList "," expression
    {
      // Verify that there have been no memory errors.
      VERIFY2($1,$3,$$);

      if (!gcu_vector64_append($1, GCU_TYPE64_P($3))) {
        $$ = 0;
        parseError = &ErrorOutOfMemory;
        break;
      }
      $$ = $1;
    }
  ;

// `mapList` is a comma-separated list of expressions given as part of a
// map declaration.
mapList
  : IDENTIFIER ":" expression
    {
      // Verify that there have been no memory errors.
      VERIFY1($3,$$);

      GTA_Unicode_String * key_unicode_string = gta_unicode_string_create_and_adopt($1.str, $1.len, $1.type);
      if (!key_unicode_string) {
        parseError = &ErrorOutOfMemory;
        gcu_free((void *)$1.str);
        break;
      }

      GTA_Ast_Node * key = (GTA_Ast_Node *)gta_ast_node_string_create(key_unicode_string, @1);
      if (!key) {
        gta_unicode_string_destroy(key_unicode_string);
        parseError = &ErrorOutOfMemory;
        break;
      }

      // Base case.  Create a vector to hold additional entries.
      $$ = gcu_vector64_create(32);
      if (!$$) {
        gta_ast_node_destroy(key);
        parseError = &ErrorOutOfMemory;
        break;
      }
      $$->cleanup = vector64_map_pair_cleanup;

      // Create the pair.
      GTA_Ast_Node_Map_Pair * pair = gcu_malloc(sizeof(GTA_Ast_Node_Map_Pair));
      if (!pair) {
        gta_ast_node_destroy(key);
        gcu_vector64_destroy($$);
        parseError = &ErrorOutOfMemory;
        $$ = 0;
        break;
      }

      // Populate all the info for this pair.
      pair->key = key;
      pair->value = $3;
      if (!gcu_vector64_append($$, GCU_TYPE64_P((void *)pair))) {
        gcu_free(pair);
        gta_ast_node_destroy(key);
        gcu_vector64_destroy($$);
        parseError = &ErrorOutOfMemory;
        $$ = 0;
        break;
      }
    }
  | mapList "," IDENTIFIER ":" expression
    {
      // Verify that there have been no memory errors.
      VERIFY2($1,$5,$$)

      GTA_Unicode_String * key_unicode_string = gta_unicode_string_create_and_adopt($3.str, $3.len, $3.type);
      if (!key_unicode_string) {
        parseError = &ErrorOutOfMemory;
        gcu_free((void *)$3.str);
        break;
      }

      GTA_Ast_Node * key = (GTA_Ast_Node *)gta_ast_node_string_create(key_unicode_string, @3);
      if (!key) {
        gta_unicode_string_destroy(key_unicode_string);
        parseError = &ErrorOutOfMemory;
        break;
      }

      // Create the pair.
      GTA_Ast_Node_Map_Pair * pair = gcu_malloc(sizeof(GTA_Ast_Node_Map_Pair));
      if (!pair) {
        gta_ast_node_destroy(key);
        parseError = &ErrorOutOfMemory;
        $$ = 0;
        break;
      }

      // Populate all the info for this pair.
      pair->key = key;
      pair->value = $5;
      if (!gcu_vector64_append($1, GCU_TYPE64_P(pair))) {
        gta_ast_node_destroy(key);
        gcu_free(pair);
        parseError = &ErrorOutOfMemory;
        $$ = 0;
        break;
      }

      $$ = $1;
    }
  | mapList ","
  ;

// `statements` represent a sequence of `statement` expressions.
statements
  : statement
    {
      // Verify that there have been no memory errors.
      VERIFY1($1,$$);

      // Base case.  Create a vector to hold additional entries.
      $$ = gcu_vector64_create(1);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
      $$->cleanup = vector64_ast_node_cleanup;

      // Space already reserved.
      gcu_vector64_append($$, GCU_TYPE64_P($1));
    }
  | statements statement
    {
      // Verify that there have been no memory errors.
      VERIFY2($1,$2,$$)

      if (!gcu_vector64_append($1, GCU_TYPE64_P($2))) {
        parseError = &ErrorOutOfMemory;
        $$ = 0;
        break;
      }
      $$ = $1;
    }
  ;

// `statement` represents an `expression`
statement
  : closedStatement
  | openStatement
  ;

// To avoid the "dangling else" problem:
// https://en.wikipedia.org/wiki/Dangling_else#Avoiding_the_conflict_in_LR_parsers
// These should only contain closedStatements

closedStatement
  : "if" "(" expression ")" closedStatement "else" closedStatement
    {
      // Verify that there have been no memory errors.
      VERIFY3($3,$5,$7,$$);

      LOCATION(@1, @7);
      $$ = (GTA_Ast_Node *)gta_ast_node_if_else_create($3, $5, $7, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "while" "(" expression ")" closedStatement
    {
      // Verify that there have been no memory errors.
      VERIFY2($3,$5,$$);

      LOCATION(@1, @5);
      $$ = (GTA_Ast_Node *)gta_ast_node_while_create($3, $5, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "do" statement "while" "(" expression ")" ";"
    {
      // Verify that there have been no memory errors.
      VERIFY2($2,$5,$$);

      LOCATION(@1, @7);
      $$ = (GTA_Ast_Node *)gta_ast_node_do_while_create($5, $2, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "for" "(" optionalExpression ";" optionalExpression ";" optionalExpression ")" closedStatement
    {
      // Verify that there have been no memory errors.
      VERIFY4($3,$5,$7,$9,$$);

      LOCATION(@1, @9);
      $$ = (GTA_Ast_Node *)gta_ast_node_for_create($3, $5, $7, $9, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "for" "(" IDENTIFIER ":" expression ")" closedStatement
    {
      // Verify that there have been no memory errors.
      VERIFY2($5,$7,$$);

      const char * identifier = $3.str;

      LOCATION(@1, @7);
      $$ = (GTA_Ast_Node *)gta_ast_node_ranged_for_create(identifier, $5, $7, location);
      if (!$$) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "function" IDENTIFIER "(" functionDeclarationArguments ")" codeBlock
    {
      // Verify that there have been no memory errors.
      VERIFY2($4,$6,$$);

      const char * identifier = $2.str;

      LOCATION(@1, @6);
      $$ = (GTA_Ast_Node *)gta_ast_node_function_create(identifier, $4, $6, location);
      if (!$$) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | codeBlock
  | "return" ";"
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      $$ = (GTA_Ast_Node *)gta_ast_node_return_create(0, @1);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "return" expression ";"
    {
      // Verify that there have been no memory errors.
      VERIFY1($2,$$);

      LOCATION(@1, @3);
      $$ = (GTA_Ast_Node *)gta_ast_node_return_create($2, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "break" ";"
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      $$ = (GTA_Ast_Node *)gta_ast_node_break_create(@1);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "continue" ";"
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      $$ = (GTA_Ast_Node *)gta_ast_node_continue_create(@1);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | expression ";"
  | TEMPLATESTRING
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      GTA_Unicode_String * string = gta_unicode_string_create_and_adopt($1.str, $1.len, $1.type);
      if (!string) {
        parseError = &ErrorOutOfMemory;
        gcu_free((void *)$1.str);
        break;
      }
      $$ = (GTA_Ast_Node *)gta_ast_node_string_create(string, @1);
      if (!$$) {
        gta_unicode_string_destroy(string);
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | QUICKPRINTBEGINANDSTRING expression QUICKPRINTEND
    {
      // This rule is for when a template string is followed by a quick print.
      // Verify that there have been no memory errors.
      VERIFY1($2,$$);

      LOCATION(@1, @3);

      GTA_Unicode_String * string = gta_unicode_string_create_and_adopt($1.str, $1.len, $1.type);
      if (!string) {
        parseError = &ErrorOutOfMemory;
        gcu_free((void *)$1.str);
        $$ = 0;
        break;
      }

      GTA_Ast_Node * preceding = (GTA_Ast_Node *)gta_ast_node_string_create(string, @1);
      if (!preceding) {
        gta_unicode_string_destroy(string);
        $$ = 0;
        parseError = &ErrorOutOfMemory;
        break;
      }

      GTA_Ast_Node * print_preceding = (GTA_Ast_Node *)gta_ast_node_print_create(preceding, @1);
      if (!print_preceding) {
        gta_ast_node_destroy(preceding);
        $$ = 0;
        parseError = &ErrorOutOfMemory;
        break;
      }

      GCU_Vector64 * block = gcu_vector64_create(2);
      if (!block) {
        gta_ast_node_destroy(print_preceding);
        $$ = 0;
        parseError = &ErrorOutOfMemory;
        break;
      }
      block->cleanup = vector64_ast_node_cleanup;
      gcu_vector64_append(block, GCU_TYPE64_P((void *)print_preceding));
      gcu_vector64_append(block, GCU_TYPE64_P((void *)$2));

      $$ = (GTA_Ast_Node *)gta_ast_node_block_create(block, location);
      if (!$$) {
        gcu_vector64_destroy(block);
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | QUICKPRINTBEGIN expression QUICKPRINTEND
    {
      // This rule is for when a quick print is not preceded by a template string.
      // Verify that there have been no memory errors.
      VERIFY1($2,$$);

      LOCATION(@1, @3);
      $$ = (GTA_Ast_Node *)gta_ast_node_print_create($2, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "use" IDENTIFIER ";"
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      const char * identifier = $2.str;

      LOCATION(@1, @3);
      GTA_Ast_Node * library = (GTA_Ast_Node *)gta_ast_node_library_create(identifier, location);
      if (!library) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
        break;
      }
      // Copy the identifier.
      const char * identifier_copy = gcu_calloc(strlen(identifier) + 1, sizeof(char));
      if (!identifier_copy) {
        gcu_free((void *)identifier);
        gta_ast_node_destroy(library);
        parseError = &ErrorOutOfMemory;
        break;
      }
      strcpy((char *)identifier_copy, identifier);
      $$ = (GTA_Ast_Node *)gta_ast_node_use_create(identifier_copy, library, location);
      if (!$$) {
        gta_ast_node_destroy(library);
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "use" libraryExpression "as" IDENTIFIER ";"
    {
      // Verify that there have been no memory errors.
      VERIFY1($2,$$);

      const char * identifier = $4.str;

      LOCATION(@1, @5);
      $$ = (GTA_Ast_Node *)gta_ast_node_use_create(identifier, $2, location);
      if (!$$) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
      }
    }
  | "global" IDENTIFIER ";"
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      const char * identifier = $2.str;

      GTA_Ast_Node * name = (GTA_Ast_Node *)gta_ast_node_identifier_create(identifier, @2);
      if (!name) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
        break;
      }

      LOCATION(@1, @3);
      $$ = (GTA_Ast_Node *)gta_ast_node_global_create(name, 0, location);
      if (!$$) {
        gta_ast_node_destroy(name);
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
    | "global" IDENTIFIER "=" expression ";"
    {
      // Verify that there have been no memory errors.
      VERIFY1($4,$$);

      const char * identifier = $2.str;

      GTA_Ast_Node * name = (GTA_Ast_Node *)gta_ast_node_identifier_create(identifier, @2);
      if (!name) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
        break;
      }

      LOCATION(@1, @5);
      $$ = (GTA_Ast_Node *)gta_ast_node_global_create(name, $4, location);
      if (!$$) {
        gta_ast_node_destroy(name);
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  ;

libraryExpression
  : IDENTIFIER
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      const char * identifier = $1.str;

      $$ = (GTA_Ast_Node *)gta_ast_node_library_create(identifier, @1);
      if (!$$) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
      }
    }
  | libraryExpression "." IDENTIFIER
    {
      // Verify that there have been no memory errors.
      VERIFY1($1,$$);

      const char * identifier = $3.str;

      LOCATION(@1, @3);
      $$ = (GTA_Ast_Node *)gta_ast_node_period_create($1, identifier, location);
      if (!$$) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
      }
    }
  ;

// These should only have an openStatement as the last terminal.
openStatement
  : "if" "(" expression ")" statement
    {
      // Verify that there have been no memory errors.
      VERIFY2($3,$5,$$);

      LOCATION(@1, @5);
      $$ = (GTA_Ast_Node *)gta_ast_node_if_else_create($3, $5, 0, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "if" "(" expression ")" closedStatement "else" openStatement
    {
      // Verify that there have been no memory errors.
      VERIFY3($3,$5,$7,$$);

      LOCATION(@1, @7);
      $$ = (GTA_Ast_Node *)gta_ast_node_if_else_create($3, $5, $7, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "while" "(" expression ")" openStatement
    {
      // Verify that there have been no memory errors.
      VERIFY2($3,$5,$$);

      LOCATION(@1, @5);
      $$ = (GTA_Ast_Node *)gta_ast_node_while_create($3, $5, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "for" "(" optionalExpression ";" optionalExpression ";" optionalExpression ")" openStatement
    {
      // Verify that there have been no memory errors.
      VERIFY4($3,$5,$7,$9,$$);

      LOCATION(@1, @9);
      $$ = (GTA_Ast_Node *)gta_ast_node_for_create($3, $5, $7, $9, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "for" "(" IDENTIFIER ":" expression ")" openStatement
    {
      // Verify that there have been no memory errors.
      VERIFY2($5,$7,$$);

      // Copy the identifier.
      const char * identifier = $3.str;

      LOCATION(@1, @7);
      $$ = (GTA_Ast_Node *)gta_ast_node_ranged_for_create(identifier, $5, $7, location);
      if (!$$) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  ;

// `optionalExpression` is an expression that, if not present, will default to
// a null value.
optionalExpression
  : %empty
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      GTA_PARSER_LTYPE location = {
        .first_line = 0,
        .first_column = 0,
        .last_line = 0,
        .last_column = 0,
      };
      $$ = gta_ast_node_create(location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
      }
    }
  | expression
  ;

// `slice` represents a slice operation on a container.
slice
  : expression "[" optionalExpression ":" optionalExpression ":" optionalExpression "]"
    {
      // Verify that there have been no memory errors.
      VERIFY4($1,$3,$5,$7,$$);

      LOCATION(@1, @8);
      $$ = (GTA_Ast_Node *)gta_ast_node_slice_create($1, $3, $5, $7, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | expression "[" optionalExpression ":" optionalExpression "]"
    {
      // Verify that there have been no memory errors.
      VERIFY3($1,$3,$5,$$);

      LOCATION(@1, @6);
      $$ = (GTA_Ast_Node *)gta_ast_node_slice_create($1, $3, $5, 0, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  ;

// `codeBlock` represents a series of statements.
codeBlock
  : "{" "}"
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      LOCATION(@1, @2);
      GTA_Ast_Node * null_val = gta_ast_node_create(location);
      if (!null_val) {
        parseError = &ErrorOutOfMemory;
        break;
      }
      GCU_Vector64 * vector = gcu_vector64_create(1);
      if (!vector) {
        gta_ast_node_destroy(null_val);
        parseError = &ErrorOutOfMemory;
        break;
      }
      vector->cleanup = vector64_ast_node_cleanup;
      gcu_vector64_append(vector, GCU_TYPE64_P((void *)null_val));
      $$ = (GTA_Ast_Node *)gta_ast_node_block_create(vector, location);
      if (!$$) {
        gcu_vector64_destroy(vector);
        parseError = &ErrorOutOfMemory;
      }
    }
  | "{" statements "}"
    {
      // Verify that there have been no memory errors.
      VERIFY1($2,$$);

      LOCATION(@1, @3);
      $$ = (GTA_Ast_Node *)gta_ast_node_block_create($2, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  ;

// `expression` represents a computable value.
expression
  : NULL_
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      $$ = (GTA_Ast_Node *)gta_ast_node_create(@1);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | IDENTIFIER
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      // Copy the identifier.
      const char * identifier = $1.str;

      $$ = (GTA_Ast_Node *)gta_ast_node_identifier_create(identifier, @1);
      if (!$$) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | INTEGER
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      $$ = (GTA_Ast_Node *)gta_ast_node_integer_create($1, @1);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | FLOAT
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      $$ = (GTA_Ast_Node *)gta_ast_node_float_create($1, @1);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | BOOLEAN
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      $$ = (GTA_Ast_Node *)gta_ast_node_boolean_create($1, @1);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
      }
    }
  | STRING
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      GTA_Unicode_String * string = gta_unicode_string_create_and_adopt((const char * const)$1.str, $1.len, $1.type);
      if (!string) {
        parseError = &ErrorOutOfMemory;
        gcu_free((void *)$1.str);
        break;
      }

      $$ = (GTA_Ast_Node *)gta_ast_node_string_create(string, @1);
      if (!$$) {
        gta_unicode_string_destroy(string);
        parseError = &ErrorOutOfMemory;
      }
    }
  | expression "=" expression
    {
      // Verify that there have been no memory errors.
      VERIFY2($1,$3,$$);

      LOCATION(@1, @3);
      $$ = (GTA_Ast_Node *)gta_ast_node_assign_create($1, $3, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
      }
    }
  | expression "+" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_ADD,$1,@1,$3,@3,$$);
    }
  | expression "-" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_SUBTRACT,$1,@1,$3,@3,$$);
    }
  | expression "*" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_MULTIPLY,$1,@1,$3,@3,$$);
    }
  | expression "/" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_DIVIDE,$1,@1,$3,@3,$$);
    }
  | expression "%" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_MODULO,$1,@1,$3,@3,$$);
    }
  | "-" expression %prec UMINUS
    {
      UNARY_TEMPLATE(GTA_UNARY_TYPE_NEGATIVE,@1,$2,@2,$$);
    }
  | "!" expression
    {
      UNARY_TEMPLATE(GTA_UNARY_TYPE_NOT,@1,$2,@2,$$);
    }
  | expression "<" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_LESS_THAN,$1,@1,$3,@3,$$);
    }
  | expression "<=" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_LESS_THAN_EQUAL,$1,@1,$3,@3,$$);
    }
  | expression ">" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_GREATER_THAN,$1,@1,$3,@3,$$);
    }
  | expression ">=" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_GREATER_THAN_EQUAL,$1,@1,$3,@3,$$);
    }
  | expression "==" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_EQUAL,$1,@1,$3,@3,$$);
    }
  | expression "!=" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_NOT_EQUAL,$1,@1,$3,@3,$$);
    }
  | expression "&&" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_AND,$1,@1,$3,@3,$$);
    }
  | expression "||" expression
    {
      BINARY_TEMPLATE(GTA_BINARY_TYPE_OR,$1,@1,$3,@3,$$);
    }
  | slice
  | "(" expression ")"
    {
      // Verify that there have been no memory errors.
      VERIFY1($2,$$);

      $$ = $2;
    }
  | expression "as" "int"
    {
      CAST_TEMPLATE(GTA_CAST_TYPE_INTEGER,$1,@1,@3,$$);
    }
  | expression "as" "float"
    {
      CAST_TEMPLATE(GTA_CAST_TYPE_FLOAT,$1,@1,@3,$$);
    }
  | expression "as" "boolean"
    {
      CAST_TEMPLATE(GTA_CAST_TYPE_BOOLEAN,$1,@1,@3,$$);
    }
  | expression "as" "string"
    {
      CAST_TEMPLATE(GTA_CAST_TYPE_STRING,$1,@1,@3,$$);
    }
  | "print" "(" expression ")"
    {
      // Verify that there have been no memory errors.
      VERIFY1($3,$$);

      LOCATION(@1, @4);
      $$ = (GTA_Ast_Node *)gta_ast_node_print_create($3, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
      }
    }
  | expression "." IDENTIFIER
    {
      // Verify that there have been no memory errors.
      VERIFY1($1,$$);

      const char * identifier = $3.str;

      LOCATION(@1, @3);
      $$ = (GTA_Ast_Node *)gta_ast_node_period_create($1, identifier, location);
      if (!$$) {
        gcu_free((void *)identifier);
        parseError = &ErrorOutOfMemory;
      }
    }
  |  "[" expressionList "]"
    {
      // Verify that there have been no memory errors.
      VERIFY1($2,$$);

      LOCATION(@1, @3);
      $$ = (GTA_Ast_Node *)gta_ast_node_array_create($2, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | "{" ":" "}"
    {
      // Verify that there have been no memory errors.
      VERIFY($$);

      LOCATION(@1, @3);
      GCU_Vector64 * vector = gcu_vector64_create(0);
      if (!vector) {
        parseError = &ErrorOutOfMemory;
        break;
      }
      vector->cleanup = vector64_map_pair_cleanup;
      $$ = (GTA_Ast_Node *)gta_ast_node_map_create(vector, location);
      if (!$$) {
        gcu_vector64_destroy(vector);
        parseError = &ErrorOutOfMemory;
      }
    }
  | "{" mapList "}"
    {
      // Verify that there have been no memory errors.
      VERIFY1($2,$$);

      LOCATION(@1, @3);
      $$ = (GTA_Ast_Node *)gta_ast_node_map_create($2, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
      }
    }
  | expression "[" expression "]"
    {
      // Verify that there have been no memory errors.
      VERIFY2($1,$3,$$);

      LOCATION(@1, @4);
      $$ = (GTA_Ast_Node *)gta_ast_node_index_create($1, $3, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | expression "(" expressionList ")"
    {
      // Verify that there have been no memory errors.
      VERIFY2($1,$3,$$);

      LOCATION(@1, @4);
      $$ = (GTA_Ast_Node *)gta_ast_node_function_call_create($1, $3, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  | expression "?" expression ":" expression
    {
      // Verify that there have been no memory errors.
      VERIFY3($1,$3,$5,$$);

      LOCATION(@1, @5);
      $$ = (GTA_Ast_Node *)gta_ast_node_ternary_create($1, $3, $5, location);
      if (!$$) {
        parseError = &ErrorOutOfMemory;
        break;
      }
    }
  ;

%%

// https://www.gnu.org/software/bison/manual/bison.html#YYERROR
void GTA_Parser_error(GTA_PARSER_LTYPE * yylloc, GTA_MAYBE_UNUSED(yyscan_t * scanner), GTA_Ast_Node * * ast, GTA_Parser_Error * parseError, const char * yymsg) {
  *parseError = yymsg;
  if (*ast) {
    gta_ast_node_destroy(*ast);
    *ast = (GTA_Ast_Node *)gta_ast_node_parse_error_create(yymsg, *yylloc);
  }
}
