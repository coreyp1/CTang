/*
 * Generate a Reentrant C Scanner.
 * https://www.cs.virginia.edu/~cr4bd/flex-manual/Code_002dLevel-And-API-Options.html
 * https://www.cs.virginia.edu/~cr4bd/flex-manual/Cxx.html#Cxx
 */
%option reentrant

/*
 * Generate a Bison bridge.
 * https://www.cs.virginia.edu/~cr4bd/flex-manual/Bison-Bridge.html#Bison-Bridge
 */
%option bison-bridge bison-locations

/*
 * Use "gta_flex_" as a name prefix rather than "yy".
 * https://www.cs.virginia.edu/~cr4bd/flex-manual/Code_002dLevel-And-API-Options.html
 */
%option prefix="gta_flex"

/*
 * Get rid of yywrap.
 * https://www.cs.virginia.edu/~cr4bd/flex-manual/Options-Affecting-Scanner-Behavior.html
 */
%option noyywrap

/*
 * Cause the scanner to error (rather than supply an empty default rule)
 * when no match is made.  This is handy for finding hols in our rules.
 * https://www.cs.virginia.edu/~cr4bd/flex-manual/Debugging-Options.html
 */
%option nodefault

/*
 * Don't include code for the interactive scanner.  We won't use it.
 * https://www.cs.virginia.edu/~cr4bd/flex-manual/Options-Affecting-Scanner-Behavior.html
 */
%option never-interactive

/*
 * Use a stack to maintain the lexer state.
 * https://www.cs.virginia.edu/~cr4bd/flex-manual/Scanner-Options.html#Scanner-Options
 */
%option stack

%{
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <cutil/memory.h>

#include "tang/tangScanner.h"
#include "tangParser.h"
#include "tang/unicodeString.h"

// yyterminate() is not a function, but a macro.
// The macro does not return the correct type.  It returns an int,
// But the Bison 3 variants requires the return of a Parser::symbol_type.
// Redefine the yyterminate() macro so that it returns the correct type,
// consistent with our use of the Bison variants.
// This macro will be used in Scanner::get_next_token() method (YY_DECL).
// See yyterminate() in http://web.mit.edu/gnu/doc/html/flex_2.html
#define yyterminate() return EOF

// Use the YY_USER_ACTION macro to add some housekeeping code.
// Specifically, we use it to update our location information to keep
// it consistent with the lexer location.
// https://www.cs.virginia.edu/~cr4bd/flex-manual/Misc-Macros.html
// yyleng is the length of the currently matched token.
// yytext is a C-string containing the matched text.
// https://www.cs.virginia.edu/~cr4bd/flex-manual/User-Values.html#User-Values
#define YY_USER_ACTION {                     \
  yylloc->first_line = yylloc->last_line;    \
  yylloc->first_column = yylloc->last_column;\
  for (int i = 0; i < yyleng; ++i) {         \
    if (yytext[i] == '\n') {                 \
      yylloc->last_column = 0;               \
      ++yylloc->last_line;                   \
    }                                        \
    else {                                   \
      ++yylloc->last_column;                 \
    }                                        \
  }                                          \
}

#define CLEANUP_BUFFER        \
  if (stringBuffer) {         \
    gcu_free(stringBuffer);   \
    stringBuffer = 0;         \
    stringBufferLength = 0;   \
    stringBufferReset = true; \
 }


static bool add_to_string(char * * buffer, size_t * bufferLen, char const * append, size_t appendLen, bool * reset) {
  // If reset is true, then we need to clear the buffer and start over.
  if (reset) {
    *buffer = 0;
    *bufferLen = 0;
    reset = false;
  }

  // If we are starting over, then this is a simple allocation and copy.
  if (!*buffer) {
    // Allocate a new buffer.
    *buffer = gcu_malloc(appendLen + 1);
    if (!*buffer) {
      return false;
    }
    memcpy(*buffer, append, appendLen);
    (*buffer)[appendLen] = 0;
    *bufferLen = appendLen;
    return true;
  }

  // Otherwise, we need to resize the buffer and append the new string.
  void * newPointer = gcu_realloc(*buffer, *bufferLen + appendLen + 1);
  if (newPointer) {
    // The resize succeeded.  Copy the appended string to the original.
    memcpy(&(*buffer)[*bufferLen], append, appendLen);
    (*buffer)[*bufferLen + appendLen] = 0;
    *buffer = newPointer;
    *bufferLen += appendLen;
    return true;
  }
  return false;
}


%}

/*
 * States used for different parsing modes.  It is useful when needing to
 * create micro-parsers, such as for comments or parsing C-style double-quoted
 * strings.
 * Approach modeled after http://dinosaur.compilertools.net/flex/manpage.html
 *   (search for "C-style")
 * Also given here: https://www.cs.virginia.edu/~cr4bd/flex-manual/Start-Conditions.html
 */
%x SCRIPT TEMPLATE QUICKPRINT doubleQuoteString singleLineComment multiLineComment

%%
  // Include any initialization code here.  These lines *must* be indented,
  // otherwise, flex will try to interpret them as rules rather than code.

  // Holds the contents of a double-quoted string as it is being parsed.
  char * stringBuffer = 0;
  size_t stringBufferLength = 0;
  bool stringBufferReset = true;

  // Identifies the type of string being recognized.
  GTA_String_Type type = GTA_UNICODE_STRING_TYPE_TRUSTED;

  // Flag variable to track whether or not the EOF has been seen, for use when
  // parsing templates.
  bool eofSeen = false;

<TEMPLATE>{
  "<%=" {
    yy_push_state(QUICKPRINT, yyscanner);
    if (stringBufferLength) {
      yylval->GTA_PARSER_QUICKPRINTBEGINANDSTRING = (GTA_Parser_Unicode_String){
        .str = stringBuffer,
        .len = stringBufferLength,
        .type = type,
      };
      stringBufferReset = true;
      return GTA_PARSER_QUICKPRINTBEGINANDSTRING;
      // return g_tang_make_QUICKPRINTBEGINANDSTRING(stringBuffer, location);
    }
    return GTA_PARSER_QUICKPRINTBEGIN;
    // return g_tang_make_QUICKPRINTBEGIN(location);
  }
  "<%" {
    yy_push_state(SCRIPT, yyscanner);
    if (stringBufferLength) {
      yylval->GTA_PARSER_TEMPLATESTRING = (GTA_Parser_Unicode_String){
        .str = stringBuffer,
        .len = stringBufferLength,
        .type = type,
      };
      stringBufferReset = true;
      return GTA_PARSER_TEMPLATESTRING;
      // return g_tang_make_TEMPLATESTRING(stringBuffer, location);
    }
  }
  [^<]+ {
    if (!add_to_string(&stringBuffer, &stringBufferLength, yytext, yyleng, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
      // return g_tang_make_MEMORYERROR("Error allocating string memory.", location);
    }
  }
  "<" {
    if (!add_to_string(&stringBuffer, &stringBufferLength, yytext, yyleng, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
      // return g_tang_make_MEMORYERROR("Error allocating string memory.", location);
    }
  }
  <<EOF>> {
    if (!eofSeen && stringBufferLength) {
      eofSeen = true;
      yylval->GTA_PARSER_TEMPLATESTRING = (GTA_Parser_Unicode_String){
        .str = stringBuffer,
        .len = stringBufferLength,
        .type = type,
      };
      stringBufferReset = true;
      return GTA_PARSER_TEMPLATESTRING;
      // return g_tang_make_TEMPLATESTRING(stringBuffer, location);
    }
    yyterminate();
  }
}

<INITIAL,SCRIPT,QUICKPRINT,singleLineComment,multiLineComment>{
  <<EOF>> {
    yyterminate();
  }
}

<INITIAL,SCRIPT>{
  "%>" {
    // Exit out of SCRIPT mode and return to TEMPLATE mode.
    // Error if we weren't actually in TEMPLATE mode to begin with.
    if (yyg->yy_start_stack_ptr < 1) {
      // Can't pop the stack.  This is an error.
      CLEANUP_BUFFER
      return GTA_PARSER_UNEXPECTEDSCRIPTEND;
      // return g_tang_make_UNEXPECTEDSCRIPTEND(location);
    }
    yy_pop_state(yyscanner);
    stringBufferReset = true;
  }
}

<QUICKPRINT>{
  "%>" {
    // Exit out of QUICKPRINT mode and return to TEMPLATE mode.
    yy_pop_state(yyscanner);
    stringBufferReset = true;
    return GTA_PARSER_QUICKPRINTEND;
    // return g_tang_make_QUICKPRINTEND(location);
  }
}

<INITIAL,SCRIPT,QUICKPRINT>{
  \/\/ {
    // Beginning of a single line comment.
    yy_push_state(singleLineComment, yyscanner);
  }
  \/\* {
    // Beginning of a multi line comment.
    yy_push_state(multiLineComment, yyscanner);
  }
  %\" {
    // " This is to fix the highlighting when editing the flex file.
    // The start of a new, double-quoted string.
    stringBufferReset = true;
    // TODO: This should be a "print" type.
    type = GTA_UNICODE_STRING_TYPE_PERCENT;
    yy_push_state(doubleQuoteString, yyscanner);
  }
  \!\" {
    // " This is to fix the highlighting when editing the flex file.
    // The start of a new, double-quoted string.
    stringBufferReset = true;
    // TODO: This should be a "print" type.
    type = GTA_UNICODE_STRING_TYPE_UNTRUSTED;
    yy_push_state(doubleQuoteString, yyscanner);
  }
  \" {
    // The start of a new, double-quoted string.
    stringBufferReset = true;
    // TODO: This should be a "print" type.
    type = GTA_UNICODE_STRING_TYPE_TRUSTED;
    yy_push_state(doubleQuoteString, yyscanner);
  }
  if {
    // " This is to fix the highlighting when editing the flex file.
    return GTA_PARSER_IF;
    // return g_tang_make_IF(location);
  }
  else {
    return GTA_PARSER_ELSE;
    // return g_tang_make_ELSE(location);
  }
  do {
    return GTA_PARSER_DO;
    // return g_tang_make_DO(location);
  }
  while {
    return GTA_PARSER_WHILE;
    // return g_tang_make_WHILE(location);
  }
  for {
    return GTA_PARSER_FOR;
    // return g_tang_make_FOR(location);
  }
  null {
    return GTA_PARSER_NULL_;
    // return g_tang_make_NULL(location);
  }
  int {
    return GTA_PARSER_CASTINT;
    // return g_tang_make_CASTINT(location);
  }
  float {
    return GTA_PARSER_CASTFLOAT;
    // return g_tang_make_CASTFLOAT(location);
  }
  bool {
    return GTA_PARSER_CASTBOOLEAN;
    // return g_tang_make_CASTBOOLEAN(location);
  }
  string {
    return GTA_PARSER_CASTSTRING;
    // return g_tang_make_CASTSTRING(location);
  }
  true {
    yylval->GTA_PARSER_BOOLEAN = true;
    return GTA_PARSER_BOOLEAN;
    // return g_tang_make_BOOLEAN(true, location);
  }
  false {
    yylval->GTA_PARSER_BOOLEAN = false;
    return GTA_PARSER_BOOLEAN;
    // return g_tang_make_BOOLEAN(false, location);
  }
  use {
    return GTA_PARSER_USE;
    // return g_tang_make_USE(location);
  }
  function {
    return GTA_PARSER_FUNCTION;
    // return g_tang_make_FUNCTION(location);
  }
  return {
    return GTA_PARSER_RETURN;
    // return g_tang_make_RETURN(location);
  }
  break {
    return GTA_PARSER_BREAK;
    // return g_tang_make_BREAK(location);
  }
  continue {
    return GTA_PARSER_CONTINUE;
    // return g_tang_make_CONTINUE(location);
  }
  print {
    return GTA_PARSER_PRINT;
    // return g_tang_make_PRINT(location);
  }
  [0-9]+ {
    yylval->GTA_PARSER_INTEGER = strtoll(yytext, 0, 10);
    return GTA_PARSER_INTEGER;
    // int64_t number = strtoll(yytext, 0, 10);
    // return g_tang_make_INTEGER(number, location);
  }
  ([0-9]+\.[0-9]*)|(\.[0-9]+) {
    yylval->GTA_PARSER_FLOAT = strtold(yytext, 0);
    return GTA_PARSER_FLOAT;
    // long double number = strtold(yytext, 0);
    // return g_tang_make_FLOAT(number, location);
  }
  = {
    return GTA_PARSER_ASSIGN;
    // return g_tang_make_ASSIGN(location);
  }
  \+ {
    return GTA_PARSER_PLUS;
    // return g_tang_make_PLUS(location);
  }
  \- {
    return GTA_PARSER_MINUS;
    // return g_tang_make_MINUS(location);
  }
  \* {
    return GTA_PARSER_MULTIPLY;
    // return g_tang_make_MULTIPLY(location);
  }
  \/ {
    return GTA_PARSER_DIVIDE;
    // return g_tang_make_DIVIDE(location);
  }
  \% {
    return GTA_PARSER_MODULO;
    // return g_tang_make_MODULO(location);
  }
  \( {
    return GTA_PARSER_LPAREN;
    // return g_tang_make_LPAREN(location);
  }
  \) {
    return GTA_PARSER_RPAREN;
    // return g_tang_make_RPAREN(location);
  }
  \{ {
    return GTA_PARSER_LBRACE;
    // return g_tang_make_LBRACE(location);
  }
  \} {
    return GTA_PARSER_RBRACE;
    // return g_tang_make_RBRACE(location);
  }
  \[ {
    return GTA_PARSER_LBRACKET;
    // return g_tang_make_LBRACKET(location);
  }
  \] {
    return GTA_PARSER_RBRACKET;
    // return g_tang_make_RBRACKET(location);
  }
  \! {
    return GTA_PARSER_EXCLAMATIONPOINT;
    // return g_tang_make_EXCLAMATIONPOINT(location);
  }
  \< {
    return GTA_PARSER_LESSTHAN;
    // return g_tang_make_LESSTHAN(location);
  }
  \<= {
    return GTA_PARSER_LESSTHANEQUAL;
    // return g_tang_make_LESSTHANEQUAL(location);
  }
  \> {
    return GTA_PARSER_GREATERTHAN;
    // return g_tang_make_GREATERTHAN(location);
  }
  \>= {
    return GTA_PARSER_GREATERTHANEQUAL;
    // return g_tang_make_GREATERTHANEQUAL(location);
  }
  == {
    return GTA_PARSER_EQUALCOMPARE;
    // return g_tang_make_EQUALCOMPARE(location);
  }
  != {
    return GTA_PARSER_NOTEQUAL;
    // return g_tang_make_NOTEQUAL(location);
  }
  && {
    return GTA_PARSER_AND;
    // return g_tang_make_AND(location);
  }
  \|\| {
    return GTA_PARSER_OR;
    // return g_tang_make_OR(location);
  }
  as {
    return GTA_PARSER_AS;
    // return g_tang_make_AS(location);
  }
  \? {
    return GTA_PARSER_QUESTIONMARK;
    // return g_tang_make_QUESTIONMARK(location);
  }
  : {
    return GTA_PARSER_COLON;
    // return g_tang_make_COLON(location);
  }
  ; {
    return GTA_PARSER_SEMICOLON;
    // return g_tang_make_SEMICOLON(location);
  }
  , {
    return GTA_PARSER_COMMA;
    // return g_tang_make_COMMA(location);
  }
  [a-zA-Z_][a-zA-Z0-9_]* {
    if (!add_to_string(&stringBuffer, &stringBufferLength, yytext, yyleng, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
    }
    yylval->GTA_PARSER_IDENTIFIER = (GTA_Parser_Unicode_String){
      .str = stringBuffer,
      .len = yyleng,
      .type = GTA_UNICODE_STRING_TYPE_TRUSTED,
    };
    stringBufferReset = true;
    return GTA_PARSER_IDENTIFIER;
    // return g_tang_make_IDENTIFIER(yytext, location);
  }
  \. {
    return GTA_PARSER_PERIOD;
    // return g_tang_make_PERIOD(location);
  }
  . {}
  \n {}
}

<singleLineComment>{
  \n {
    yy_pop_state(yyscanner);
  }
  .+ {}
}

<multiLineComment>{
  \*\/ {
    yy_pop_state(yyscanner);
  }
  [^*]+ {}
  \* {}
}

<doubleQuoteString>{
  <<EOF>> {
    return GTA_PARSER_STRINGERROR;
    // return g_tang_make_STRINGERROR(location);
  }
  \" {
    // " This is to fix the highlighting in Vim.
    // Found the end of a double-quoted string.
    yy_pop_state(yyscanner);
    if (!stringBufferLength) {
      if (!add_to_string(&stringBuffer, &stringBufferLength, "", 0, &stringBufferReset)) {
        CLEANUP_BUFFER
        return GTA_PARSER_MEMORYERROR;
      }
    }
    yylval->GTA_PARSER_STRING = (GTA_Parser_Unicode_String){
      .str = stringBuffer,
      .len = stringBufferLength,
      .type = type,
    };
    stringBufferReset = true;
    return GTA_PARSER_STRING;
    // return g_tang_make_STRING({stringBuffer, type}, location);
  }
  \\[0-7]{1,3} {
    // Interpret an escaped octal number.
    // The octal numer is 1-3 digits long, and has a maximum value of 511,
    // which is too large for a single byte.  If this happens, we will
    // generate an error.
    uint32_t result = yytext[1] - '0';
    if (yytext[2]) {
      result <<= 3;
      result += yytext[2] - '0';
      if (yytext[3]) {
        result <<= 3;
        result += yytext[3] - '0';
      }
    }
    if (result > 255) {
      // Generate Error - Bad escape sequence.
      return GTA_PARSER_OCTAL_OUT_OF_BOUNDS;
    }
    char buff[2] = {(unsigned char)result, 0};
    if (!add_to_string(&stringBuffer, &stringBufferLength, buff, 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
    }
  }
  \\x[a-fA-F0-9]{1,2} {
    // Match a hexadecimal char.
    unsigned char result = 0;
    for (int i = 2; i < yyleng; ++i) {
      result <<= 4;
      if (yytext[i] >= 'a' && yytext[i] <= 'f') {
        result += 10 + yytext[i] - 'a';
      }
      else if (yytext[i] >= 'A' && yytext[i] <= 'F') {
        result += 10 + yytext[i] - 'A';
      }
      else {
        result += yytext[i] - '0';
      }
    }
    char buff[2] = {result, 0};
    if (!add_to_string(&stringBuffer, &stringBufferLength, buff, 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
    }
  }
  \\[0-9]+ {
    // Generate Error - Bad escape sequence.
    // Something like "\48" or "\07777777"
    CLEANUP_BUFFER
    return GTA_PARSER_STRINGERROR;
    // return g_tang_make_STRINGERROR(location);
  }
  \n|\\n {
    // Matches either an explicit or escaped newline.
    if (!add_to_string(&stringBuffer, &stringBufferLength, "\n", 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
      // return g_tang_make_MEMORYERROR("Error allocating string memory.", location);
    }
  }
  \t|\\t {
    // Matches either an explicit or escaped tab.
    if (!add_to_string(&stringBuffer, &stringBufferLength, "\t", 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
      // return g_tang_make_MEMORYERROR("Error allocating string memory.", location);
    }
  }
  \r|\\r {
    // Matches either an explicit or escaped carriage return.
    if (!add_to_string(&stringBuffer, &stringBufferLength, "\r", 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
      // return g_tang_make_MEMORYERROR("Error allocating string memory.", location);
    }
  }
  \b|\\b {
    // Matches either an explicit or escaped bell.
    if (!add_to_string(&stringBuffer, &stringBufferLength, "\b", 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
      // return g_tang_make_MEMORYERROR("Error allocating string memory.", location);
    }
  }
  \f|\\f {
    // Matches either an explicit or escaped form feed.
    if (!add_to_string(&stringBuffer, &stringBufferLength, "\f", 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
      // return g_tang_make_MEMORYERROR("Error allocating string memory.", location);
    }
  }
  \\(.|\n) {
    // Any other escaped character.
    char buff[2] = {yytext[1], 0};
    if (!add_to_string(&stringBuffer, &stringBufferLength, buff, 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
      // return g_tang_make_MEMORYERROR("Error allocating string memory.", location);
    }
  }
  [^\\\n\"]+ {
    // "] <-- to fix the highlighting in Vim.
    // Any other character in the string.
    if (!add_to_string(&stringBuffer, &stringBufferLength, yytext, yyleng, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
      // return g_tang_make_MEMORYERROR("Error allocating string memory.", location);
    }
  }
  \\ {
    // Because we specified escape characters, we also need to specify what to
    // do if we see only a single backslash at the end of the input.
    CLEANUP_BUFFER
    return GTA_PARSER_STRINGERROR;
    // return g_tang_make_STRINGERROR(location);
  }
}

%%

