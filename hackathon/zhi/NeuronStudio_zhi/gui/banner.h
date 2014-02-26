#ifndef __BANNER_H__
#define __BANNER_H__

#include <windows.h>
#include <commctrl.h>


void IntroBanner
   ( 
   HINSTANCE    hInstance,
   char        *bmpResourceName,
   int          hasBorder,
   int          hasStatus,
   const char  *statusText,
   int          seconds
   );


#endif/* __BANNER_H__ */
