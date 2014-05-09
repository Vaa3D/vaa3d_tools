#ifndef __APP_DISPLAY_H__
#define __APP_DISPLAY_H__
/*============================================================================
   Module  : app_display.h
   Author  : Doug Ehlenberger © 2002
   Purpose : Define display window controls.
   Date    : Last modified on November 18, 2002.
=============================================================================*/
#include "app_lib.h"
#include "app_data.h"


/* IMPORTANT!!!

   COORDINATE SYSTEMS( for the workspace and the window( client ) ) 


   NOTE: Global coordinates are a right hand coordinate system rotated 
         180 degrees around x-axis. 

         ( '+' indicates positive axis if "traveling" from the origin 
           out along the axis. )

             / +Z
            /
           /
          /--------- +X
          |
          |
          |
          | +Y


   In forward view:


          ----------  +X              ----------  +X
          |                           |
          |                           |
          |                           |
          | +Y                        | +Y

         client( window )            global


   In side view:

          ----------  +X              ----------- +Z  
          |                           |
          |                           |
          |                           |
          | +Y                        | +Y

         client                     global


   In top view:

          ----------  +X              ----------- +X
          |                           |
          |                           |
          |                           |
          | +Y                        | -Z

         client                     global

   NOTE: The -Z for the top view!! This could be avoided by assuming the view was
   from the bottom but it is more intuitive to view the volume from the top. The 
   current view will have to be known and certain operations will have to change 
   when in top view.

   The images top left corner is assumed to be fixed at 0,0,0 in the global
   coordinate system. A viewport is assumed to be moving over the global 
   coordinate system whose top left corner is known.
*/


/* Set workspace message. Sent to set the windows handle
   to the workspace. wParam = 0, lParam = the handle or 
   zero to clear the window. */

#define mMSG_USER_SetWorkspace  ( WM_USER + 1 )


/* Update message. Sent when a display window makes a change
   to some of the data. wParam = 0, lParam = one of the eDISPLAY_TYPES. */

#define mMSG_USER_Update        ( WM_USER + 2 )


/* The window will fit the image. wParam and lParam should be zero. */

#define mMSG_USER_FitImage      ( WM_USER + 4 )


/* The window will set its view to the passed type. lParam = one of the eDISPLAY_TYPES. */

#define mMSG_USER_SetDisplayType   ( WM_USER + 5 )


/* lParam = thickness */
#define mMSG_USER_SetPenThickness ( WM_USER + 6 )



HWND CreateDisplayWindow
   ( 
   HWND  hParent,
   int   ID, 
   int   index,
   int   x, 
   int   y, 
   int   width, 
   int   height,
   int   display
   );


#endif/* __APP_DISPLAY_H__ */
