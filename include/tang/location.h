/**
 * @file
 *
 * This file defines the location types used by the lexers and parsers.
 */

#ifndef GTA_PARSER_LOCATION_H
#define GTA_PARSER_LOCATION_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct GTA_PARSER_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} GTA_PARSER_LTYPE;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // GTA_PARSER_LOCATION_H
