/**
 * @file
 * Declare the GTA_Scanner used to tokenize a Tang script.
 */

#ifndef GTA_TANGSCANNER_H
#define GTA_TANGSCANNER_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define YYSTYPE GTA_PARSER_STYPE
#define YYLTYPE GTA_PARSER_LTYPE

#include "tangParser.h"

// Our scanner will populate an internal buffer so that strings can be properly
// interpreted (and null-terminated) by the parser.  The buffer will be passed
// back to the parser as the tokens (IDENTIFIER, STRING, etc.) are recognized.
//
// In the event of a parse error, the parser will need to be able to clean up
// any memory that was allocated for the buffer, hence passing in this pointer
// by reference.
#undef YY_DECL
#define YY_DECL int gta_scanner_get_next_token(GTA_PARSER_STYPE * yylval_param, GTA_PARSER_LTYPE * yylloc_param , yyscan_t yyscanner)

// The actual declaration of the scanner function.
YY_DECL;

// #define YY_DECL Tang::TangParser::symbol_type Tang::TangScanner::get_next_token()

// Now, the normal header contents.

// #include "tangParser.h"


    /**
     * Helper function to set the scanner to template parsing mode.
     */
    // void setModeTemplate() {
      // Formula is taken from the BEGIN macro in tangScanner.cpp.
      // (2) is the value that Flex has assigned the TEMPLATE rule type from
      // tpl.l.
      // this->yy_start = 1 + 2 * (2);
    // }

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GTA_TANGSCANNER_H

