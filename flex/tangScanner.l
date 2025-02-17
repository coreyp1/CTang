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
  if (*reset) {
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
    memcpy(&((char *)newPointer)[*bufferLen], append, appendLen);
    ((char *)newPointer)[*bufferLen + appendLen] = 0;
    *buffer = newPointer;
    *bufferLen += appendLen;
    return true;
  }
  return false;
}

static size_t gta_flex_get_state_stack_depth(void * yyscanner);

%}

/*
 * States used for different parsing modes.  It is useful when needing to
 * create micro-parsers, such as for comments or parsing C-style double-quoted
 * strings.
 * Approach modeled after http://dinosaur.compilertools.net/flex/manpage.html
 *   (search for "C-style")
 * Also given here: https://www.cs.virginia.edu/~cr4bd/flex-manual/Start-Conditions.html
 */
%x SCRIPT TEMPLATE QUICKPRINT doubleQuoteString singleLineComment multiLineComment DATE DATEInsideExpression

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
    }
    return GTA_PARSER_QUICKPRINTBEGIN;
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
    }
  }
  [^<]+ {
    if (!add_to_string(&stringBuffer, &stringBufferLength, yytext, yyleng, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
    }
    stringBufferReset = false;
  }
  "<" {
    if (!add_to_string(&stringBuffer, &stringBufferLength, yytext, yyleng, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
    }
    stringBufferReset = false;
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
    }
    yyterminate();
  }
}

<INITIAL,SCRIPT,QUICKPRINT,singleLineComment,multiLineComment,DATE,DATEInsideExpression>{
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
  }
}

<INITIAL,SCRIPT,QUICKPRINT>{
  if {
    // " This is to fix the highlighting when editing the flex file.
    return GTA_PARSER_IF;
  }
  else {
    return GTA_PARSER_ELSE;
  }
  do {
    return GTA_PARSER_DO;
  }
  while {
    return GTA_PARSER_WHILE;
  }
  for {
    return GTA_PARSER_FOR;
  }
  use {
    return GTA_PARSER_USE;
  }
  global {
    return GTA_PARSER_GLOBAL;
  }
  function {
    return GTA_PARSER_FUNCTION;
  }
  return {
    return GTA_PARSER_RETURN;
  }
  break {
    return GTA_PARSER_BREAK;
  }
  continue {
    return GTA_PARSER_CONTINUE;
  }
  \{ {
    return GTA_PARSER_LBRACE;
  }
  \} {
    return GTA_PARSER_RBRACE;
  }
  as {
    return GTA_PARSER_AS;
  }
  ; {
    return GTA_PARSER_SEMICOLON;
  }
}

<INITIAL,SCRIPT,QUICKPRINT,DATEInsideExpression>{
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
    type = GTA_UNICODE_STRING_TYPE_HTML;
    yy_push_state(doubleQuoteString, yyscanner);
  }
  \" {
    // " This is to fix the highlighting when editing the flex file.
    // The start of a new, double-quoted string.
    stringBufferReset = true;
    // TODO: This should be a "print" type.
    type = GTA_UNICODE_STRING_TYPE_TRUSTED;
    yy_push_state(doubleQuoteString, yyscanner);
  }
  null {
    return GTA_PARSER_NULL_;
  }
  int {
    return GTA_PARSER_CASTINT;
  }
  float {
    return GTA_PARSER_CASTFLOAT;
  }
  bool {
    return GTA_PARSER_CASTBOOLEAN;
  }
  string {
    return GTA_PARSER_CASTSTRING;
  }
  true {
    yylval->GTA_PARSER_BOOLEAN = true;
    return GTA_PARSER_BOOLEAN;
  }
  false {
    yylval->GTA_PARSER_BOOLEAN = false;
    return GTA_PARSER_BOOLEAN;
  }
  print {
    return GTA_PARSER_PRINT;
  }
  [0-9]+ {
    yylval->GTA_PARSER_INTEGER = strtoll(yytext, 0, 10);
    return GTA_PARSER_INTEGER;
  }
  ([0-9]+\.[0-9]*)|(\.[0-9]+) {
    yylval->GTA_PARSER_FLOAT = strtold(yytext, 0);
    return GTA_PARSER_FLOAT;
  }
  = {
    return GTA_PARSER_ASSIGN;
  }
  \+ {
    return GTA_PARSER_PLUS;
  }
  \- {
    return GTA_PARSER_MINUS;
  }
  \* {
    return GTA_PARSER_MULTIPLY;
  }
  \/ {
    return GTA_PARSER_DIVIDE;
  }
  \% {
    return GTA_PARSER_MODULO;
  }
  \( {
    if (gta_flex_get_state_stack_depth(yyscanner) && (yy_top_state(yyscanner) == DATEInsideExpression)) {
      yy_push_state(DATEInsideExpression, yyscanner);
    }
    return GTA_PARSER_LPAREN;
  }
  \) {
    if (gta_flex_get_state_stack_depth(yyscanner) && (yy_top_state(yyscanner) == DATEInsideExpression)) {
      yy_pop_state(yyscanner);
    }
    return GTA_PARSER_RPAREN;
  }
  \[ {
    return GTA_PARSER_LBRACKET;
  }
  \] {
    return GTA_PARSER_RBRACKET;
  }
  \! {
    return GTA_PARSER_EXCLAMATIONPOINT;
  }
  \< {
    return GTA_PARSER_LESSTHAN;
  }
  \<= {
    return GTA_PARSER_LESSTHANEQUAL;
  }
  \> {
    return GTA_PARSER_GREATERTHAN;
  }
  \>= {
    return GTA_PARSER_GREATERTHANEQUAL;
  }
  == {
    return GTA_PARSER_EQUALCOMPARE;
  }
  != {
    return GTA_PARSER_NOTEQUAL;
  }
  && {
    return GTA_PARSER_AND;
  }
  \|\| {
    return GTA_PARSER_OR;
  }
  \? {
    return GTA_PARSER_QUESTIONMARK;
  }
  : {
    return GTA_PARSER_COLON;
  }
  , {
    return GTA_PARSER_COMMA;
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
  }
  \. {
    return GTA_PARSER_PERIOD;
  }
  @ {
    yy_push_state(DATE, yyscanner);
    return GTA_PARSER_AT;
  }
  [[:blank:]]+ {
    // Ignore whitespace.
  }
  \n {
    // Ignore newlines.
  }
  . {
    return GTA_PARSER_SYNTAXERROR;
  }
}

<DATE>{
  [se][pn]?[yMwdhms] {
    yylval->GTA_PARSER_DATE_CLOSE_RELATIVE = (GTA_Parser_Date){
      .str = yytext,
      .len = yyleng,
    };
    return GTA_PARSER_DATE_CLOSE_RELATIVE;
  }
  now {
    return GTA_PARSER_DATE_NOW;
  }
  today {
    return GTA_PARSER_DATE_TODAY;
  }
  tomorrow {
    return GTA_PARSER_DATE_TOMORROW;
  }
  yesterday {
    return GTA_PARSER_DATE_YESTERDAY;
  }
  \+ {
    return GTA_PARSER_DATE_PLUS;
  }
  \( {
    yy_push_state(DATEInsideExpression, yyscanner);
  }
  [yMwdhms] {
    yylval->GTA_PARSER_DATE_UNIT = (GTA_Parser_Date){
      .str = yytext,
      .len = yyleng,
    };
    return GTA_PARSER_DATE_UNIT;
  }
  \d+(-\d+(-\d+(T\d+:\d+(:\d+)?)?)?)? {
    yylval->GTA_PARSER_DATE_ABSOLUTE = (GTA_Parser_Date){
      .str = yytext,
      .len = yyleng,
    };
    return GTA_PARSER_DATE_ABSOLUTE;
  }
  Z {
    return GTA_PARSER_DATE_TIMEZONE_Z;
  }
  T[+-]?\d+(:\d+(:\d+(.\d+)?)?)? {
    yylval->GTA_PARSER_DATE_TIMEZONE_T = (GTA_Parser_Date){
      .str = yytext,
      .len = yyleng,
    };
    return GTA_PARSER_DATE_TIMEZONE_T;
  }
  (\[[^\]]+\]) {
    yylval->GTA_PARSER_DATE_TIMEZONE = (GTA_Parser_Date){
      .str = yytext,
      .len = yyleng,
    };
    return GTA_PARSER_DATE_TIMEZONE;
  }
  . {
    yy_pop_state(yyscanner);
  }
  \n {
    yy_pop_state(yyscanner);
  }
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
    stringBufferReset = false;
  }
  \\[xX][a-fA-F0-9]{1,2} {
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
    stringBufferReset = false;
  }
  \\[0-9]+ {
    // Generate Error - Bad escape sequence.
    // Something like "\48" or "\07777777"
    CLEANUP_BUFFER
    return GTA_PARSER_STRINGERROR;
  }
  \n|\\n {
    // Matches either an explicit or escaped newline.
    if (!add_to_string(&stringBuffer, &stringBufferLength, "\n", 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
    }
    stringBufferReset = false;
  }
  \t|\\t {
    // Matches either an explicit or escaped tab.
    if (!add_to_string(&stringBuffer, &stringBufferLength, "\t", 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
    }
    stringBufferReset = false;
  }
  \r|\\r {
    // Matches either an explicit or escaped carriage return.
    if (!add_to_string(&stringBuffer, &stringBufferLength, "\r", 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
    }
    stringBufferReset = false;
  }
  \b|\\b {
    // Matches either an explicit or escaped bell.
    if (!add_to_string(&stringBuffer, &stringBufferLength, "\b", 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
    }
    stringBufferReset = false;
  }
  \f|\\f {
    // Matches either an explicit or escaped form feed.
    if (!add_to_string(&stringBuffer, &stringBufferLength, "\f", 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
    }
    stringBufferReset = false;
  }
  \\(.|\n) {
    // Any other escaped character.
    char buff[2] = {yytext[1], 0};
    if (!add_to_string(&stringBuffer, &stringBufferLength, buff, 1, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
    }
    stringBufferReset = false;
  }
  [^\\\n\"]+ {
    // "] <-- to fix the highlighting in Vim.
    // Any other character in the string.
    if (!add_to_string(&stringBuffer, &stringBufferLength, yytext, yyleng, &stringBufferReset)) {
      CLEANUP_BUFFER
      return GTA_PARSER_MEMORYERROR;
    }
    stringBufferReset = false;
  }
  \\ {
    // Because we specified escape characters, we also need to specify what to
    // do if we see only a single backslash at the end of the input.
    CLEANUP_BUFFER
    return GTA_PARSER_STRINGERROR;
  }
}

%%

// Tang helper function to set the initial state to TEMPLATE.
void gta_flex_set_state(int state, void * yyscanner) {
  yy_push_state(state, yyscanner);
}

// Tang helper function to get the depth of the state stack.
size_t gta_flex_get_state_stack_depth(void * yyscanner) {
  struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	return yyg->yy_start_stack_ptr;
}
