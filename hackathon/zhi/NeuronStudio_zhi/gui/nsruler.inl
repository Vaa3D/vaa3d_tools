#include "nsruler.h"


#define NS_RULER_COLOR              0x00808080

#define NS_RULER_CTRL_WIDTH         7
#define NS_RULER_CTRL_HALF_WIDTH    3
#define NS_RULER_CTRL_HEIGHT        NS_RULER_CTRL_WIDTH
#define NS_RULER_CTRL_HALF_HEIGHT   NS_RULER_CTRL_HALF_WIDTH

#define NS_RULER_CTRL_TOLERANCE     NS_RULER_CTRL_WIDTH

#define NS_RULER_LINE_TOLERANCE     25.0f


static Vector2i s_LastFfrMouseCoord;


static const nschar* s_FreeFormRulerCursors[ vNUM_FFR_CURSORS ] = 
   {
   "CUR_RULER",
   "CUR_RULER_OVER_CTRL",
   "CUR_RULER_OVER_LINE"
   };


void CreateFreeFormRuler( NsRuler *ruler, HWND hOwner, HDC hDC )
   {
   ns_assert( NULL != ruler );
   
   ruler->hOwner        = hOwner;
   ruler->hDC           = hDC;
   ruler->hPen          = CreatePen( PS_COSMETIC | PS_SOLID, 1, NS_RULER_COLOR );
   ruler->mode          = NS_RULER_MODE_FIRST_CONTROL;
   ruler->active_control = NULL;
   ruler->cursor        = NS_RULER_CURSOR_STANDARD;

   if( ! dllInit( &ruler->controls ) )
      return ( NS_FALSE );
      
   SetFreeFormRulerLength( ruler );

   return NS_TRUE;
   }


void DestroyFreeFormRuler( NsRuler *ruler )
   {
   ns_assert( NULL != ruler );

   EraseFreeFormRuler( ruler );

   DeleteObject( ruler->hPen );
   dllDestroy( &ruler->controls );

   ClearMainStatusBarDist();
   }


void _FreeFormRulerXOR( NsRuler *ruler )
   {
   HDC                  hDC;
   HGDIOBJ              hOldPen;
   nsint                 hOldROP;
   nslistiter  previous;
   nslistiter  current;
   nslistiter  end;
   nsint                 x;
   nsint                 y;


   ns_assert( NULL != ruler );
   
   hDC = ruler->hDC;

   /* XOR mode ensures ruler visible over any color and
      that it can be easily "erased" by simply drawing it
      again in the same place. */

   hOldROP = SetROP2( hDC, R2_XORPEN );  
   hOldPen = SelectObject( hDC, ruler->hPen );

   /* Draw all the line sections. */

   previous = NULL;
   current  = dllibegin( &ruler->controls );
   end      = dlliend( &ruler->controls );

   for( ; current != end; dlliplus( current ) )
      {
      /* Draw small crosshairs over point. */

      x = current->point.x;
      y = current->point.y;

      MoveToEx( hDC, x - NS_RULER_CTRL_HALF_WIDTH, y, NULL );
      LineTo( hDC, x - NS_RULER_CTRL_HALF_WIDTH + NS_RULER_CTRL_WIDTH, y );
      MoveToEx( hDC, x, y - NS_RULER_CTRL_HALF_HEIGHT, NULL );
      LineTo( hDC, x, y - NS_RULER_CTRL_HALF_HEIGHT + NS_RULER_CTRL_HEIGHT ); 
      
      if( NULL != previous )
         {
         MoveToEx( hDC, x, y, NULL );
         LineTo( hDC, previous->point.x, previous->point.y );
         }

      previous = current;

      }/* for( all points ) */ 
   
   SetROP2( hDC, hOldROP );
   SelectObject( hDC, hOldPen );
   }


void DrawFreeFormRuler( NsRuler *ruler )
   {
   _FreeFormRulerXOR( ruler );
   }


void EraseFreeFormRuler( NsRuler *ruler )
   {
   _FreeFormRulerXOR( ruler );
   }


nsint _FreeFormRulerPackFunc( void *controlNode, void *vpPoint )
   {
   ns_assert( NULL != controlNode );
   ns_assert( NULL != vpPoint );

   ( ( FFRCTRLLISTNode* )controlNode )->point = *( ( Vector2i* )vpPoint );

   return ( 0 );
   }


nsboolean _FreeFormRulerAddPoint( NsRuler *ruler, nsint x, nsint y, nsboolean toFront )
   {
   Vector2i  point;
   nsint      errorCode;


   ns_assert( NULL != ruler );

   point.x = x;
   point.y = y;

   if( toFront )
      errorCode = ns_list_push_front( &ruler->controls,
                                    &point,
                                    _FreeFormRulerPackFunc,
                                    dllNodeBytes( &ruler->controls )
                                  );
   else
      errorCode = ns_list_push_back( &ruler->controls,
                                   &point,
                                   _FreeFormRulerPackFunc,
                                   dllNodeBytes( &ruler->controls )
                                 );
   
   return ( ( 0 == errorCode ) ? NS_TRUE : NS_FALSE );
   }


nsfloat _GetFreeFormRulerLength( NsRuler *ruler )
   {
   nslistiter  previous;
   nslistiter  current;
   nslistiter  end;
   Vector2i             V1;
   Vector2i             V2;
   nsfloat               voxelSpacingX;
   nsfloat               voxelSpacingY;
   nsfloat               lengthX;
   nsfloat               lengthY;
   nsfloat               length;


   length = 0.0f;

   ns_assert( ControlFlagIsSet( vBIT_WORKSPACE_OPEN ), vASSERT_PROGRAM_STATE );

   GetViewDependentVoxelSpacing( GetWorkspace(), 
                                 GetPaintingView(), 
                                 &voxelSpacingX, 
                                 &voxelSpacingY 
                               );

   previous = NULL;
   current  = dllibegin( &ruler->controls );
   end      = dlliend( &ruler->controls );

   for( ; current != end; dlliplus( current ) )
      {
      if( NULL != previous )
         {
         /* Convert client to workspace coordinates and multiply by
            voxel spacing to get the actual physical length. */

         V1 = ClientToWorkspaceCoord( current->point.x, current->point.y );
         V2 = ClientToWorkspaceCoord( previous->point.x, previous->point.y );

         lengthX = ( V1.x - V2.x ) * voxelSpacingX;
         lengthY = ( V1.y - V2.y ) * voxelSpacingY;

         length += ( nsfloat )sqrt( ( lengthX * lengthX ) + ( lengthY * lengthY ) );

         }/* if( previous ) */

      previous = current;

      }/* for( points ) */

   return length;
   }


nsfloat _DistanceSquaredToLine( nslistiter start, nslistiter end,
                               nsint x, nsint y )
   {
   nsint    x0,y0,x1,y1,x2,y2,f,g;
   nsfloat  t,x,y;


   x0 = x;
   y0 = y;
   x1 = start->point.x;
   y1 = start->point.y;
   x2 = end->point.x;
   y2 = end->point.y;

   f = x2 - x1;
   g = y2 - y1;

   /* Zero length vector, just return -1 indicating error. */

   if( 0 == f && 0 == g ) 
      return ( -1.0f );

   t = ( f * ( x0 - x1 ) + g * ( y0 - y1 ) ) / ( ( nsfloat )( f*f + g*g ) );

   /* The point does not intersect the line. */

   if( t < 0 || t > 1 )
      return ( -1.0f );

   x = x1 + f * t;
   y = y1 + g * t;

   return ( ( x - x0 )*( x - x0 ) + ( y - y0 )*( y - y0 ) );
   }


void _SetFfrCursor( NsRuler *ruler, NS_RULER_CURSOR_TYPE type )
   {
   HCURSOR hCursor;
  

   ns_assert( ( nsint )type < NS_RULER_CURSOR_SENTINEL, vASSERT_PARAMETER );

   if( type != ruler->cursor )
      {
      ruler->cursor = type;

      hCursor = LoadCursor( GetProgramInstance(), s_FreeFormRulerCursors[ type ] );

      SetClassLong( ruler->hOwner, GCL_HCURSOR, ( LONG )hCursor );
      SetCursor( hCursor );
      }
   }


void _SetFfrIdleCursor( NsRuler *ruler, nsint x, nsint y )
   {
   nslistiter  previous;
   nslistiter  current;
   nslistiter  end;
   nsint                 xdiff;
   nsint                 ydiff;
   nsboolean                canDragRuler;
   nsfloat               distance;


   ns_assert( NULL != ruler );

   canDragRuler = NS_FALSE;

   /* Based on where the cursor is hovering over, set the appropriate 
      cursor. */

   previous = NULL;
   current  = dllibegin( &ruler->controls );
   end      = dlliend( &ruler->controls );

   for( ; current != end; dlliplus( current ) )
      {
      xdiff = current->point.x - x;
      xdiff = NS_ABS( xdiff );

      ydiff = current->point.y - y;
      ydiff = NS_ABS( ydiff );

      if( xdiff < NS_RULER_CTRL_TOLERANCE && ydiff < NS_RULER_CTRL_TOLERANCE )
         { 
         _SetFfrCursor( ruler, NS_RULER_CURSOR_OVER_CONTROL );
         ruler->active_control = current;
         return;
         }

      /* Check if point close enough to any line to enable translation
         of the entire ruler. */

      if( NULL != previous )
         {
         distance = _DistanceSquaredToLine( previous, current, x, y );

         if( 0 <= distance && distance <= NS_RULER_LINE_TOLERANCE )
            canDragRuler = NS_TRUE;
         }

      previous = current;
      
      }/* for( points ) */

   if( canDragRuler )
      {
      _SetFfrCursor( ruler, NS_RULER_CURSOR_OVER_LINE );
      SetVector2i( s_LastFfrMouseCoord, x, y );
      }
   else
      _SetFfrCursor( ruler, NS_RULER_CURSOR_STANDARD );

   ruler->active_control = NULL;
   }


void _TranslateFreeFormRuler( NsRuler *ruler, nsint x, nsint y )
   {
   Vector2i  difference;
   Vector2i  currFfrMouseCoord;


   ns_assert( NULL != ruler );

   SetVector2i( currFfrMouseCoord, x, y );
   SubVector2i( currFfrMouseCoord, s_LastFfrMouseCoord, difference );

   if( ! IsZeroVector2i( difference ) )
      {
      nslistiter  current;
      nslistiter  end;

      current = dllibegin( &ruler->controls );
      end     = dlliend( &ruler->controls );

      /* Offset every point in the ruler. */
      for( ; current != end; dlliplus( current ) )
         AddVector2i( current->point, difference, current->point );  
      }

   s_LastFfrMouseCoord = currFfrMouseCoord;
   }


nsboolean _HandleFfrMouseMove( NsRuler *ruler, nsint x, nsint y )
   {
   ns_assert( NULL != ruler );

   switch( ruler->mode )
      {
      case NS_RULER_MODE_DRAG_CONTROL:
         /* Erase the old ruler, update the active control's point
            and then redraw. */ 

         EraseFreeFormRuler( ruler );

         ns_assert( NULL != ruler->active_control );
         ruler->active_control->point.x = x;
         ruler->active_control->point.y = y;

         DrawFreeFormRuler( ruler );

         /* Calculate and display current length. */
         SetFreeFormRulerLength( ruler );
         break;

      case NS_RULER_MODE_DRAG_RULER:
         /* Erase the ruler, translate all points, and then redraw. */

         EraseFreeFormRuler( ruler );
         _TranslateFreeFormRuler( ruler, x, y );
         DrawFreeFormRuler( ruler );
         break;

      case NS_RULER_MODE_IDLE:  
         _SetFfrIdleCursor( ruler, x, y );     
         break;
      }

   return NS_TRUE;
   }


nsboolean _HandleFfrMouseLButtonDown( NsRuler *ruler, nsint x, nsint y )
   {
   nsboolean ok;
  

   ns_assert( NULL != ruler );

   switch( ruler->mode )
      {
      case NS_RULER_MODE_FIRST_CONTROL:
         ns_assert( 0 == ns_list_size( &ruler->controls ), vASSERT_PROGRAM_STATE );

         /* Add 2 points and begin dragging of the second point. */

         ok = ( nsboolean )( ( _FreeFormRulerAddPoint( ruler, x, y, NS_FALSE )
                   &&
                _FreeFormRulerAddPoint( ruler, x, y, NS_FALSE )
              ) );

         if( ok )
            {
            ruler->active_control = dllirbegin( &ruler->controls );
            ruler->mode          = NS_RULER_MODE_DRAG_CONTROL;

            DrawFreeFormRuler( ruler );          
            SetCapture( ruler->hOwner );

            SetFreeFormRulerLength( ruler );
            }
         else
            dllClear( &ruler->controls );

         return ( ok );

      case NS_RULER_MODE_IDLE:
         switch( ruler->cursor )
            {
            case NS_RULER_CURSOR_OVER_CONTROL:
               ns_assert( NULL != ruler->active_control );
               ruler->mode = NS_RULER_MODE_DRAG_CONTROL;
               SetCapture( ruler->hOwner );
               break;
            
            case NS_RULER_CURSOR_OVER_LINE:
               ruler->mode = NS_RULER_MODE_DRAG_RULER;
               SetCapture( ruler->hOwner );
               break;
            
            case NS_RULER_CURSOR_STANDARD:
               /* User didn't click on anything, so clear all previous points. */

               ruler->mode = NS_RULER_MODE_FIRST_CONTROL;
               EraseFreeFormRuler( ruler );
               dllClear( &ruler->controls );

               SetFreeFormRulerLength( ruler );

               /* The function calls itself to pass the coordinates. Recursive
                  call is OK since we changed the mode. i.e. no infinite recursion! */

               return _HandleFfrMouseLButtonDown( ruler, x, y );

            default:
               ns_assert( 0, vASSERT_PROGRAM_STATE );;

            }/* switch( cursor ) */
         break;

      }/* switch( mode ) */

   return NS_TRUE;
   }
   


nsboolean _HandleFfrMouseLButtonUp( NsRuler *ruler, nsint x, nsint y )
   { 
   ns_assert( NULL != ruler );

   switch( ruler->mode )
      {
      case NS_RULER_MODE_DRAG_CONTROL:
         ruler->active_control = NULL;
         ruler->mode          = NS_RULER_MODE_IDLE;
         ReleaseCapture();
         break;

      case NS_RULER_MODE_DRAG_RULER: 
         ruler->mode = NS_RULER_MODE_IDLE;
         ReleaseCapture();
         break;
      }

   return NS_TRUE;
   }


nsboolean _HandleFfrMouseLButtonDblClk( NsRuler *ruler, nsint x, nsint y )
   {
   nsboolean ok = NS_TRUE;
  

   ns_assert( NULL != ruler );

   switch( ruler->mode )
      {
      case NS_RULER_MODE_IDLE:
         if( NS_RULER_CURSOR_OVER_CONTROL == ruler->cursor )
            {
            nsboolean atEndPoint = NS_FALSE;
            nsboolean toFront;


            ns_assert( NULL != ruler->active_control );

            /* If user double clicks on and end point, push a new point
               and begin dragging of that point. */

            if( ruler->active_control == dllibegin( &ruler->controls ) )
               {
               atEndPoint = NS_TRUE;
               toFront    = NS_TRUE;
               }
            else if( ruler->active_control == dllirbegin( &ruler->controls ) )
               {
               atEndPoint = NS_TRUE;
               toFront    = NS_FALSE;
               }

            if( atEndPoint )
               {
               EraseFreeFormRuler( ruler );
               ok = _FreeFormRulerAddPoint( ruler, x, y, toFront );
               DrawFreeFormRuler( ruler );
               
               if( ok )
                  {
                  ruler->active_control = 
                     ( toFront ) ? dllibegin( &ruler->controls ) : dllirbegin( &ruler->controls );
                  ruler->mode = NS_RULER_MODE_DRAG_CONTROL;
                  SetCapture( ruler->hOwner );
                  }
               }
            }
         break;
      }

   return ok;
   }


void ns_ruler_on_mouse
   ( 
   NsRuler           *ruler,
   nsint              x, 
   nsint              y,
   NsRulerMouseType   type
   )
   {
   ns_assert( NULL != ruler );

   switch( type )
      {
      case NS_RULER_MOUSE_MOVE:
         return _ns_ruler_on_mouse_move( ruler, x, y );

      case NS_RULER_MOUSE_LBUTTONDOWN:
         return _ns_ruler_on_mouse_lbuttondown( ruler, x, y );

      case NS_RULER_MOUSE_LBUTTONUP:
         return _ns_ruler_on_mouse_lbuttonup( ruler, x, y );

      case NS_RULER_MOUSE_LBUTTONDBLCLK:
         return _ns_ruler_on_mouse_lbuttondblclk( ruler, x, y ); 

      default:
         ns_assert_not_reached();
      }

   return NS_FALSE;       
   }



/*
void SetFreeFormRulerLength( NsRuler *ruler )
   {
   vCHAR buffer[ 128 ];

 
   if( 0 == ns_list_size( &ruler->controls ) )
      sprintf( buffer, "???" );
   else
      {
      ns_assert( ControlFlagIsSet( vBIT_WORKSPACE_OPEN ), vASSERT_PROGRAM_STATE );

      sprintf( buffer, 
               "%.4f %s", 
               _GetFreeFormRulerLength( ruler ),
               GetVoxelUnitsShortString( GetWorkspaceVoxelUnits( GetWorkspace() ) )  
             );
      }

   SetMainStatusBarDist( buffer );    
   }
*/

