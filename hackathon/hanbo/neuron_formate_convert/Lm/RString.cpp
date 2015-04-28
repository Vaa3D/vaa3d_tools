
#include "RString.h"

// This function reverse a string s 
void reverse(char s[])
{   
   int c, i, j;
   for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
       c = s[i];
       s[i] = s[j];
       s[j] = c;
   }
}



void itoa1(int n, char s[], int begin)
{
  int i = begin;

  do {
     s[i++] = n %10 + '0';
  } while ((n /=10) > 0);
  s[i] = '\0';

  reverse(s);
}

