#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "tz_string.h"
#include "tz_iarray.h"
#include "tz_error.h"
#include "tz_darray.h"

void strtrim(char *str)
{
  if(str==NULL)
    return;

  int length = strlen(str);
  int start,end,i;
  start = 0;
  end = length;
  for(i=0;i<length;i++)
    if(isspace(str[i]))
      start++;
    else
      break;

  for(i=length-1;i>=0;i--)
    if(isspace(str[i]))
      end--;
    else
      break;

  if(end<length)
    str[end] = '\0';

  if(start>0)
    memmove(str,str+start,end-start+1);  
}

void strrmspc(char *str)
{
  if(str==NULL)
    return;

  int i,pos,length;
  length = strlen(str);
  pos = 0;
  for(i=0;i<length;i++) {
    if(!isspace(str[i])) {
       str[pos] = str[i];
       pos++;
    }
  }
  str[pos] = '\0';
}

char* strsplit(char *str, char delim, int pos)
{
  if (pos == 0) {
    return NULL;
  }
  
  char *tail = NULL;
  if (pos > 0) {
    tail = strchr(str, delim);
    pos--;
    while ((tail != NULL) && (pos > 0)) {
      tail = strchr(tail + 1, delim);
      pos--;
    }
  } else {
    int len = strlen(str);
    int i;
    for (i = len - 1; i >= 0; i--) {
      if (str[i] == delim) {
	pos++;
      }
      if (pos >= 0) {
	break;
      }
    }
    if (i < 0) {
      tail = NULL;
    } else {
      tail = str + i;
    }
  }

  if (tail != NULL) {
    tail[0] = '\0';
    tail++;
  }

  return tail;
}

BOOL String_Ends_With(const char *str, const char *end)
{
  if ((str == NULL) || (end == NULL)) {
    return FALSE;
  }

  int n1 = strlen(str);
  int n2 = strlen(end);
  if (n1 < n2) {
    return FALSE;
  }

  if (n2 == 0) {
    return TRUE;
  }

  if (strcmp(str+(n1-n2), end) == 0) {
    return TRUE;
  }

  return FALSE;
}

BOOL String_Starts_With(const char *str, const char *start)
{
  if ((str == NULL) || (start == NULL)) {
    return FALSE;
  }

  int n1 = strlen(str);
  int n2 = strlen(start);
  if (n1 < n2) {
    return FALSE;
  }

  if (n2 == 0) {
    return TRUE;
  }

  if (strncmp(str, start, n2) == 0) {
    return TRUE;
  }

  return FALSE;
}

void fprint_space(FILE  *fp, int n)
{
  if (n > 0) {
    int i;
    for (i = 0; i < n; i++) {
      fprintf(fp, "%c", ' ');
    }
  }
}

int String_First_Integer(const char *str)
{
  int index = 0;
  while(str[index] != '\0') {
    if (isdigit(str[index++])) {
      break;
    }
  }

  index--;
  if (str[index] == '\0') {
    return -1;
  }

  return strtol(str + index, NULL, 10);
}

int String_Last_Integer(const char *str)
{
  if (str == NULL) {
    return -1;
  }

  int len = strlen(str);
  if (len == 0) {
    return -1;
  }

  int index = len;
  int number_found = 0;
  for (index = len - 1; index >= 0; index--) {
    if (isdigit(str[index])) {
      if (number_found == 0) {
	number_found = 1;
      }
    } else {
      if (number_found == 1) {
	break;
      }
    }
  }

  if (number_found == 0) {
    return -1;
  }

  return String_First_Integer(str + index + 1);
}

BOOL Is_Integer(const char *str)
{
  if (!isdigit(*str)) {
    if ((*str == '+') || (*str == '-')) {
      if (!isdigit(str[1])) {
	return FALSE;
      }
    } else {
      return FALSE;
    }
  }

  str++;
  while (*str) {
    if (!isdigit(*str)) {
      return FALSE;
    }
    str++;
  }

  return TRUE;
}

BOOL Is_Float(const char *str)
{
  int ndot = 0;
  if (!isdigit(*str)) {
    if ((*str == '+') || (*str == '-')) {
      if (str[1] == '.') {
	str++;
	ndot++;
      } else {
	if (!isdigit(str[1])) {
	  return FALSE;
	}
      }
    } else if (*str == '.') {
      ndot++;
    } else {
      return FALSE;
    }
  }

  str++;
  while (*str) {
    if (!isdigit(*str)) {
      if (*str == '.') {
	if (ndot == 0) {
	  ndot++;
	} else {
	  return FALSE;
	}
      } else {
	return FALSE;
      }
    }
    str++;
  }

  return TRUE;
}

BOOL Is_Space(const char *str)
{
  while (*str) {
    if (!tz_isspace(*str)) {
      return FALSE;
    }
    str++;
  }

  return TRUE;
}

int tz_isspace(char c)
{
  if (c > 0) {
    return isspace(c);
  } else {
    return 1;
  }
}

int tz_issemicolon(char c)
{
  return (c == ';');
}

int tz_iscoma(char c)
{
  return (c == ',');
}

int tz_islinebreak(char c)
{
  return ((c == '\r') || (c == '\n'));
}

int tz_isdlm(char c)
{
  return tz_isspace(c) || tz_iscoma(c) || tz_issemicolon(c) || 
    tz_islinebreak(c);
}

int Read_Word(FILE *fp, char *str, int n)
{
  return Read_Word_D(fp, str, n, tz_isspace);
}

int Read_Word_D(FILE *fp, char *str, int n, int (*is_dlm) (char))
{
  if (n < 0) {
    return 0;
  }

  if (feof(fp)) {
    return 0;
  }

  char c;
  fread(&c, sizeof(char), 1, fp);

  while (is_dlm(c)) {
    if (feof(fp)) {
      return 0;
    } else {
      fread(&c, sizeof(char), 1, fp);
    }
  }

  int index = 0;

  str[index++] = c;
  
  while (!feof(fp)) {
    if ((n > 0) && (index == n)) { /* maximum length reached */
      break;
    }

    fread(&c, sizeof(char), 1, fp);
    if (!is_dlm(c)) {
      str[index++] = c;
    } else  {
      fseek(fp, SEEK_CUR, -1);
      break;
    }

    if (index >= MAX_WORD_LENGTH - 1) {
      break;
    }
  }
  
  str[index] = '\0';

  return index;  
}

int Count_Word_D(char *str, int (*is_dlm) (char))
{
  if (is_dlm == NULL) {
    is_dlm = tz_isspace;
  }

  int n = 0;
  int status = 0;
  while (*str) {
    switch (status) {
    case 0:
      if (!is_dlm(*str)) {
	status = 1;
      }
      break;
    case 1:
      if (is_dlm(*str)) {
	n++;
	status = 0;
      }
      break;
    default:
      break;
    }
    str++;
  }

  if (status == 1) {
    n++;
  }

  return n;
}

int Count_Word_P(char *str, int (*is_dlm) (char), 
		 int (*is_pattern)(const char *))
{
  if (is_dlm == NULL) {
    is_dlm = tz_isspace;
  }

  char buffer[100];

  int i = 0;
  int n = 0;
  int status = 0;
  while (*str) {
    switch (status) {
    case 0:
      if (!is_dlm(*str)) { /* word begins */
	buffer[i++] = *str;
	status = 1;
      }
      break;
    case 1:
      if (is_dlm(*str)) { /* word ends */
	buffer[i] = '\0';
	strtrim(buffer);
	if (is_pattern(buffer)) {
	  n++;
	}
	i = 0;
	status = 0;
      } else { /* word continues */
	buffer[i++] = *str;
      }
      break;
    default:
      break;
    }

    str++;
  }

  if (status == 1) {
    buffer[i] = '\0';
    strtrim(buffer);
    if (is_pattern(buffer)) {
      n++;
    }
  }

  return n;  
}

int Count_Number_D(char *str, int (*is_dlm) (char))
{
  return Count_Word_P(str, is_dlm, Is_Float);
}

int Count_Integer_D(char *str, int (*is_dlm) (char))
{
  return Count_Word_P(str, is_dlm, Is_Integer);
}

/* Modified from http://www.gdargaud.net/Hack/SourceCode.html */
char* Read_Param(FILE *fp, const char *var, String_Workspace *sw)
{
  char *var_name = NULL;
  char *comment = NULL;
  char *equal = NULL;

  char *first_quote, *last_quote;
  char *p1, *p2;
  int line=0, len=0, pos=0;
  char *line_space = NULL;

  while ((line_space = Read_Line(fp, sw)) != NULL) {
    line++;
    len = strlen(line_space);
    if (len==0) { /* skip empty line */
      continue;
    }

#if 0
    if (line_space[len-1]=='\n' || line_space[len-1]=='\r') { 
      line_space[--len]='\0'; /* turns a line into a null terminated string */
    }
#endif

    equal = strchr(line_space, '=');  /* search for equal sign */
    pos = strcspn(line_space, ";#!"); /* search for comment */
    comment = (pos==len) ? NULL : line_space+pos;

    if ((equal==NULL) || (comment != NULL && comment <= equal)) {
      continue;	/* Only comment */
    }

    *equal++ = '\0';
    if (comment != NULL) {
      *comment ='\0';
    }

    first_quote = strchr(equal, '"');	/* search for double quote char */
    last_quote = strrchr(equal, '"');

    if (first_quote!=NULL) {
      if (last_quote==NULL) {
	fprintf(stderr, "\nError reading parameter line %d - Missing end quote.", line);
	break;
      }
      *first_quote = *last_quote = '\0';
      equal = first_quote + 1;
    }
		
    /* removes leading/trailing spaces */
    pos = strspn(line_space, " \t");
    if (pos == strlen(line_space)) {
      fprintf(stderr, "\nError reading parameter line %d - Missing variable name.", line);
      break;		/* No function name */
    }

    while ((p1=strrchr(line_space, ' '))!=NULL || 
	   (p2=strrchr(line_space, '\t'))!=NULL) {
      if (p1 != NULL) {
	*p1='\0';
      } else if (p2!=NULL) {
	*p2='\0';
      }
    }

    var_name = line_space + pos;

    pos = strspn(equal, " \t");
    if (pos == strlen(equal)) {
      fprintf(stderr, "\nError reading parameter line %d - Missing value.", line);
      break;
    }
    equal += pos;

    if (strcmp(var_name, var)==0) {		/* Found it */
      return equal;
    }
  } /* process a line */

  return NULL;
}

char* Read_Line(FILE *fp, String_Workspace *sw)
{
  if ((fp == NULL) || feof(fp)) {
    return NULL;
  }

  char c = '\0';
  int index = 0;

  while (!feof(fp)) {
    if (fread(&c, sizeof(char), 1, fp) == 0) {
      if (index == 0) {
	return NULL;
      }
      break;
    }

    if (tz_islinebreak(c)) {
      break;
    }
    if (index >= sw->size) {
      String_Workspace_Grow(sw);
    }
    sw->array[index] = c;
    index++;
  }
  if (sw->array == NULL) {
    String_Workspace_Grow(sw);
  }

  if (index >= sw->size) {
    String_Workspace_Grow(sw);
  }
  sw->array[index] = '\0';

  return sw->array;    
}

static int count_integer(const char *str) 
{
  int n = 0;
  int state = 0;

  while (*str) {
    switch (state) {
    case 0:
      if (isdigit(*str)) {
	n++;
	state = 1;
      }
      break;
    case 1:
      if (!isdigit(*str)) {
	state = 0;
      }
      break;
    default:
      TZ_ERROR(ERROR_DATA_VALUE);
      break;
    }
    str++;
  }  

  return n;
}

int *String_To_Integer_Array(const char *str, int *array, int *n)
{
  *n = count_integer(str);
  if (array == NULL) {
    array = iarray_malloc(*n);
  }

  int i = 0;
  int state = 0;
  BOOL is_negative = FALSE;

  while (*str) {
    switch (state) {
    case 0:
      if (isdigit(*str)) {
	array[i] = *str - '0';
	state = 1;
      } else if (*str == '-') {
          //array[i] = 0;
        is_negative = TRUE;
          //state = 1;
      } else {
        is_negative = FALSE;
      }
      break;
    case 1:
      if (isdigit(*str)) {
	array[i] = array[i] * 10 + *str - '0';
      } else { /* end of the number */
	if (is_negative) {
	  array[i] = -array[i];
	}
	is_negative = FALSE;
	state = 0;
	i++;
      }
      break;
    default:
      TZ_ERROR(ERROR_DATA_VALUE);
      break;
    }
    str++;
  }

  if (is_negative) {
    if (array[(*n)-1] > 0) {
      array[(*n)-1] *= -1;
    }
  }
  return array;
}

/*
static int count_double(const char *str) 
{
  int n = 0;
  int state = 0;

  while (*str) {
    int hit = isdigit(*str) || (*str == '.');
    switch (state) {
    case 0:
      if (hit) {
	n++;
	state = 1;
      }
      break;
    case 1:
      if (!hit) {
	state = 0;
      }
      break;
    default:
      TZ_ERROR(ERROR_DATA_VALUE);
      break;
    }
    str++;
  }  

  return n;
}
*/

static int count_double(const char *str)
{
  int n = 0;
  int state = 0;

  BOOL has_number = FALSE;

  while (*str) {
    switch (state) {
      case 0:
        if (has_number) {
          n++;
          has_number = FALSE;
        }

        if (isdigit(*str)) {
          state = 2;
          has_number = TRUE;
        } else if (*str == '+' || *str == '-') {
          state = 1;
        } else if ((*str) == '.') {
          state = 8;
        }

        ++str;
        break;
      case 1:
        if (isdigit(*str)) {
          state = 2;
          has_number = TRUE;
          ++str;
        } else if ((*str) == '.') {
          state = 8;
          ++str;
        } else {
          state = 0;
        }
        break;
      case 8:
        if (isdigit(*str)) {
          state = 3;
          has_number = TRUE;
          ++str;
        } else {
          state = 0;
        }
        break;
      case 2:
        if (*str == '.') {
          state = 3;
        } else if (*str == 'e' || *str == 'E') {
          state = 5;
        } else if (!isdigit(*str)) {
          state = 0;
        }
        ++str;
        break;
      case 3:
        if (*str == 'e' || *str == 'E') {
          state = 5;
        } else if (isdigit(*str)) {
          state = 4;
          has_number = TRUE;
        } else {
          state = 0;
        }
        ++str;
        break;
      case 4:
        if (*str == 'e' || *str == 'E') {
          state = 5;
        } else if (!isdigit(*str)) {
          state = 0;
        }
        ++str;
        break;
      case 5:
        if (*str == '+' || *str == '-') {
          state = 6;
        } else if (isdigit(*str)) {
          state = 7;
        } else {
          state = 0;
        }
        ++str;
        break;
      case 6:
        if (isdigit(*str)) {
          state = 7;
        } else {
          state = 0;
        }
        ++str;
        break;
      case 7:
        if (!isdigit(*str)) {
          state =0;
        }
        ++str;
        break;
      default:
        state = 0;
        ++str;
        break;
    }
  }

  if (has_number) {
    ++n;
  }

  return n;
}


double *String_To_Double_Array(const char *str, double *array, int *n)
{
  *n = count_double(str);
  if (array == NULL) {
    array = darray_malloc(*n);
  }

  int i = 0;
  int state = 0;

  char *numstr = strdup(str);
  char *numstr_end = numstr;
  char *numstr_head = numstr;

  BOOL has_number = FALSE;

  while (*str) {
    switch (state) {
      case 0:
        if (has_number) {
#ifdef _DEBUG_2
          printf("%s\n", numstr);
#endif

          array[i++] = atof(numstr);
          has_number = FALSE;
        }

        numstr = numstr_end;

        if (isdigit(*str)) {
          state = 2;
          has_number = TRUE;
        } else if (*str == '+' || *str == '-') {
          state = 1;
        } else if ((*str) == '.') {
          state = 8;
        }

        ++str;
        ++numstr_end;
        break;
      case 1:
        if (isdigit(*str)) {
          state = 2;
          has_number = TRUE;
          ++str;
          ++numstr_end;
        } else if ((*str) == '.') {
          state = 8;
          ++str;
          ++numstr_end;
        } else {
          state = 0;
        }
        break;
      case 8:
        if (isdigit(*str)) {
          state = 3;
          has_number = TRUE;
          ++str;
          ++numstr_end;
        } else {
          state = 0;
        }
        break;
      case 2:
        if (*str == '.') {
          state = 3;
        } else if (*str == 'e' || *str == 'E') {
          state = 5;
        } else if (!isdigit(*str)) {
          *numstr_end = '\0';
          state = 0;
        }
        ++str;
        ++numstr_end;
        break;
      case 3:
        if (*str == 'e' || *str == 'E') {
          state = 5;
        } else if (isdigit(*str)) {
          state = 4;
          has_number = TRUE;
        } else {
          *numstr_end = '\0';
          state = 0;
        }
        ++str;
        ++numstr_end;
        break;
      case 4:
        if (*str == 'e' || *str == 'E') {
          state = 5;
        } else if (!isdigit(*str)) {
          *numstr_end = '\0';
          state = 0;
        }
        ++str;
        ++numstr_end;
        break;
      case 5:
        if (*str == '+' || *str == '-') {
          state = 6;
        } else if (isdigit(*str)) {
          state = 7;
        } else {
          *numstr_end = '\0';
          state = 0;
        }
        ++numstr_end;
        ++str;
        break;
      case 6:
        if (isdigit(*str)) {
          state = 7;
        } else {
          *numstr_end = '\0';
          state = 0;
        }
        ++numstr_end;
        ++str;
        break;
      case 7:
        if (!isdigit(*str)) {
          *numstr_end = '\0';
          state =0;
        }
        ++numstr_end;
        ++str;
        break;
      default:
        state = 0;
        *numstr_end = '\0';
        ++numstr_end;
        ++str;
        break;
    }
  }

  if (has_number) {
    array[i] = atof(numstr);
  }

  free(numstr_head);

  return array;
}

#if 0
double *String_To_Double_Array(const char *str, double *array, int *n)
{
  *n = count_double(str);
  if (array == NULL) {
    array = darray_malloc(*n);
  }

  int i = 0;
  int state = 0;
  int sig = 1;
  BOOL is_negative = FALSE;

  while (*str) {
    switch (state) {
    case 0:
      if (isdigit(*str)) {
	array[i] = *str - '0';
        if (is_negative) {
          array[i] = -array[i];
        }
	//if (*str != '.') {
	//  array[i] -= '0';
	//}
	state = 1;
      } else if (*str == '.') {
	array[i] = 0.0;
	state = 2;
      } else {
        is_negative = (*str == '-');
      }
      break;
    case 1:
      if (isdigit(*str)) {
	array[i] = fabs(array[i]) * 10 + *str - '0';
        if (is_negative) {
          array[i] = -array[i];
        }
      } else if (*str == '.') {
	state = 2;
      } else {
        is_negative = (*str == '-');
	state = 0;
	i++;
      }
      break;
    case 2:
      if (isdigit(*str)) {
	sig *= 10;
	array[i] = fabs(array[i]) + (double) (*str - '0') / sig;
        if (is_negative) {
          array[i] = -array[i];
        }
      } else {
	sig = 1;
	state = 0;
        is_negative = (*str == '-');
	i++;
      }
      break;
    default:
      TZ_ERROR(ERROR_DATA_VALUE);
      break;
    }
    str++;
  }

  return array;
}
#endif

int File_Line_Number(const char *path, BOOL count_empty)
{
  FILE *fp = GUARDED_FOPEN(path, "r");
  int n = 0;
  String_Workspace *sw = New_String_Workspace();
  char *line;
  while ((line = Read_Line(fp, sw)) != NULL) {
    if (Is_Space(line)) {
      if (count_empty) {
	n++;
      }
    } else {
      n++;
    }
  }
  Kill_String_Workspace(sw);
  fclose(fp);

  return n;
}

int tz_isletter(char c)
{
  return ((c <= 'z') && (c >= 'a')) || ((c <= 'Z') && (c >= 'A'));
}
