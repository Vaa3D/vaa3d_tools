#ifndef __DIRDLG_H__
#define __DIRDLG_H__

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <ctype.h>
#include <direct.h>
#include <errno.h>
#include "resource.h"
 
/*

BMP_DIR_SEARCH          BITMAP  MOVEABLE PURE   "files.bmp"
BMP_MASK_DIR_SEARCH     BITMAP  MOVEABLE PURE   "filesmask.bmp"


#define IDE_DSN_DIR                     
#define IDL_DSN_DIRS                    
#define IDC_DSN_DRIVES                  
#define IDB_DSN_OK                      
#define IDB_DSN_CANCEL     


DLG_DIR_SEARCH DIALOGEX 0, 0, 186, 130
STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
FONT 8, "MS Sans Serif", 0, 0, 0x1
BEGIN
    DEFPUSHBUTTON   "OK",IDB_DSN_OK,129,7,50,14
    PUSHBUTTON      "Cancel",IDB_DSN_CANCEL,129,24,50,14
    CONTROL         "",IDC_DSN_DRIVES,"ComboBoxEx32",CBS_DROPDOWNLIST | 
                    WS_TABSTOP,128,60,51,113
    EDITTEXT        IDE_DSN_DIR,7,109,172,13,ES_AUTOHSCROLL | ES_READONLY | 
                    NOT WS_BORDER,WS_EX_STATICEDGE
    CONTROL         "List2",IDL_DSN_DIRS,"SysListView32",LVS_REPORT | 
                    LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOCOLUMNHEADER | 
                    WS_TABSTOP,7,7,112,94,WS_EX_CLIENTEDGE
    LTEXT           "Drives",IDC_STATIC,128,49,36,10
END

*/


typedef struct tagDIRSEARCHNAME
   {
   HWND       hwndOwner;
   HINSTANCE  hInstance;
   LPTSTR     lpstrDir;          
   DWORD      nMaxDir;           
   LPCTSTR    lpstrInitialDir;
   LPCTSTR    lpstrTitle;
   }
   DIRSEARCHNAME;


BOOL GetDirSearchName( DIRSEARCHNAME *dsn );


#endif/* __DIRDLG_H__ */
