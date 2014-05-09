#ifndef __CLRDLG_H__
#define __CLRDLG_H__

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "resource.h"


/* FUNCTION: ChooseColorsDialog()

   Activates a modal color selection dialog box for multiple items.

   PARAMETERS:

   hInstance:
     Instance of the executable passed to WinMain().

   hOwner :
     The parent window of the dialog.

   numItems :
     The number of items for which colors are being selected. 

   itemNames :
     String descriptions of the items, e.g. "background".

   itemColors :
     The current color of the items before the dialog is activated.
     If the function returns non-zero then this array also contains
     the new colors of the items selected by the user.

   defaultColors :
     The default color of the items. i.e. the preferred color.

   selectedItem :
     The initially selected item displayed in the dialog.

   onApply :
     Callback function when the user hits the apply button.
     The current selected item and color are passed to the
     function.

   RETURNS: Non-zero if the user hits the OK button, else zero.
*/


int ChooseColorsDialog
   (
   HINSTANCE       hInstance,
   HWND            hOwner,
   const unsigned  numItems,
   const char      **itemNames,
   COLORREF        *itemColors,
   const COLORREF  *defaultColors,
   const unsigned  selectedItem,
   void            ( *onApply )( const unsigned item, const COLORREF color )
   );


#endif/* __CLRDLG_H__ */
