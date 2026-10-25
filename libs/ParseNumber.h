#ifndef __PARSENUMBER_H__
#define __PARSENUMBER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

/*
 * Parse an unsigned integer from a string and validate that its value
 * does not exceed the maximum supported value.
 *
 * Converts the input string to an unsigned 64-bit integer using base-10
 * notation. Rejects empty strings, invalid characters, conversion errors,
 * and values that exceed the specified maximum.
 *
 * Parameters:
 *   pszValue  Null-terminated string containing the integer to parse.
 *   pValue    Receives the parsed integer on success.
 *   maxValue  Maximum permitted value.
 *
 * Returns:
 *   TRUE if the entire input string represents a valid unsigned integer
 *   within the permitted range; otherwise, FALSE.
 */
BOOLEAN __fastcall ParseUnsignedInteger( PCTSTR pszIn, PUINT64 puiOut );

#ifdef __cplusplus
}
#endif

#endif
