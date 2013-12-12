/**@file tz_string.h
 * @brief string processing
 * @author Ting Zhao
 */

#ifndef _TZ_STRING_H_
#define _TZ_STRING_H_

#include <stdio.h>
#include "tz_cdefs.h"
#include "tz_string_defs.h"

__BEGIN_DECLS


/**@addtogroup string_ String utilities (tz_string.h)
 * @{
 */

/**@brief remove spaces at the ends of a string 
 *
 * strtrim() remove spaces at the beginning and end of a string.  
 */
void strtrim(char *str);

/**@brief remove spaces in a string 
 * 
 * strrmspc() removes all spaces in a string.
 */
void strrmspc(char *str);

/**@brief Split a string
 *
 * strsplit() splits <str> at the <pos>th occurence of <delim>. If <pos> is
 * negative, its counted backwards. It returns the string next to the
 * located <delim>. The location will become the end of <str>.
 * If there is no <delim>, it returns NULL.
 */
char* strsplit(char *str, char delim, int pos);

/**@brief test if a char is a space*/
int tz_isspace(char c);

int tz_issemicolon(char c);
int tz_iscoma(char c);
int tz_islinebreak(char c);
int tz_isdlm(char c);

int tz_isletter(char c);

/**@brief get the first integer in a string 
 *
 * String_First_Integer() returns the first integer in \a str. It ignores all 
 * signs though, e.g. '-123' returns 123. It returns -1 if no number is found.
 */
int String_First_Integer(const char *str);

/**@brief get the last integer in a string 
 *
 * String_Last_Integer() returns the last integer in \a str. Its behavior is
 * similar to String_First_Integer() while dealing with signs and 
 * no number found.
 */
int String_Last_Integer(const char *str);

BOOL Is_Integer(const char *str);
BOOL Is_Float(const char *str);
BOOL Is_Space(const char *str);

/**@brief Test if a string ends with a given string
 *
 * String_Ends_With() returns TRUE if and only if the ending characters of 
 * <str> matches <end> exactly. It returns FALSE if either <str> or <end> is
 * NULL.
 */
BOOL String_Ends_With(const char *str, const char *end);

/**@brief Test if a string starts with a given string
 *
 * String_Starts_With() returns TRUE if and only if the starting characters of 
 * <str> matches <start> exactly. It returns FALSE if either <str> or <start> is
 * NULL.
 */
BOOL String_Starts_With(const char *str, const char *start);

/**@brief output spaces  
 *
 * fprint_space() prints \a n spaces in \a fp.
 */
void fprint_space(FILE  *fp, int n);

/**@brief read a word from a stream
 *
 * Read_Word() reads a word from fp into str. It returns 0 if there is no word
 * found, otherwise it returns the length of the word. Here a word is defined as
 * a string consisting of non-space characters. Read_Word() will read the first
 * word from the start of the steam <fp>. <n> is the maximum length of the word
 * to read. If <n> is 0, the function simply reads the word as long as 
 * possible. The caller needs to give <str> enough space.
 *
 * Read_Word_D() allows the user to specify a function to determine if a char
 * is a delimiter.
 */
int Read_Word(FILE *fp, char *str, int n);
int Read_Word_D(FILE *fp, char *str, int n, int (*is_dlm) (char));

int Count_Word_D(char *str, int (*is_dlm) (char));
int Count_Word_P(char *str, int (*is_dlm) (char), 
		 int (*is_pattern)(const char *));
int Count_Number_D(char *str, int (*is_dlm) (char));
int Count_Integer_D(char *str, int (*is_dlm) (char));

/**@brief Turn string into an integer array.
 *
 * String_To_Integer_Array() extracts integers in <str> and returns them as an
 * integer array. <array> provides the storage space for the integer array if it
 * is not NULL. <n> stores the number of integers extracts and can not be NULL.
 */
int* String_To_Integer_Array(const char *str, int *array, int *n);

/**@brief Turn string into an double array.
 *
 * String_To_Double_Array() extracts doubles in <str> and returns them as an
 * double array. <array> provides the storage space for the double array if it
 * is not NULL. <n> stores the number of doubles extracts and can not be NULL.
 */
double* String_To_Double_Array(const char *str, double *array, int *n);

/**@brief read a parameter
 *
 * Read_Param() reads a parameter with with name <var> and returns a string that
 * has its value to be converted into a desired type. The returned pointer is
 * owned by \a sw.
 */
char* Read_Param(FILE *fp, const char* var, String_Workspace *sw);

/**@brief read a line
 *
 * Read_Line() returns a line from fp. The returned pointer is owned by \a sw.
 */
char* Read_Line(FILE *fp, String_Workspace *sw);

/**@Count number of lines in a file
 *
 * File_Line_Number() returns the number of lines in the file <path>. The empty
 * lines are included if <count_empty> is TRUE. Otherwise they are ignored.
 */
int File_Line_Number(const char *path, BOOL count_empty);

/**@}*/

/*static int count_double(const char *str);*/
__END_DECLS

#endif
