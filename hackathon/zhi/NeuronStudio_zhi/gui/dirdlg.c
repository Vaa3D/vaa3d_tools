#include "dirdlg.h"


static DIRSEARCHNAME     s_DSN;
static HWND              s_DSN_DriveComboBox;
static HWND              s_DSN_DirListBox;
static HWND              s_DSN_ToolTip;
static HHOOK             s_DSN_Hook;
static int               s_DSN_OriginalDrive;
static int               s_DSN_SelectedDrive;
static char              s_DSN_cwd[ _MAX_PATH ];
static char              s_DSN_OriginalDir[ _MAX_PATH ];
static char              s_DSN_LastWorkingDir[ _MAX_PATH ] = {'\0'};
static char              s_DSN_SelDirName[ _MAX_DIR ];
static char              s_DSN_ToolTipBuffer[ _MAX_DIR ];
static HIMAGELIST        s_DSN_ImageList;
static BOOL              s_DSN_ReturnDirName;
static unsigned          s_DSN_AveCharWidth = 8;
static int               s_DSN_EditItem;
static HWND              s_DSN_EditLabel;
static char              s_DSN_Mkdir[ _MAX_PATH ];


#define DSN_COL_WIDTH_DIR_NAMES  240


void _DirSearchCenterDlg( HWND hDlg )
   {
   RECT rcParent;
   RECT rcChild;
   int  midX;
   int  midY;


   GetWindowRect( s_DSN.hwndOwner, &rcParent );
   GetWindowRect( hDlg, &rcChild );

   midX = rcParent.left + ( rcParent.right - rcParent.left )/2 - 
                          ( rcChild.right - rcChild.left )/2;

   midY = rcParent.top  + ( rcParent.bottom - rcParent.top )/2 - 
                          ( rcChild.bottom - rcChild.top )/2;

   SetWindowPos( hDlg, NULL, midX, midY, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

   }//_DirSearchCenterDlg()


void _DirSearchSetDriveText()
   {
   int             numItems;
   char            drive1[ 16 ] = {'\0'};  
   char            drive2[ 16 ] = {'\0'};
   COMBOBOXEXITEM  cbei;

   
   /* Find the index of the selected drive in the combo box...  */

   sprintf( drive1, "%c:", 'a' + s_DSN_SelectedDrive - 1 );

   numItems = ( int )SendMessage( s_DSN_DriveComboBox, CB_GETCOUNT, 0, 0 );

   cbei.pszText    = drive2;
   cbei.cchTextMax = 16;
   cbei.mask       = CBEIF_TEXT;

   for( cbei.iItem = 0; cbei.iItem < numItems; ++cbei.iItem )
      {
      SendMessage( s_DSN_DriveComboBox, CBEM_GETITEM, 0, ( LPARAM )&cbei );

      if( 0 == strcmp( drive1, drive2 ) )
         {
         SendMessage( s_DSN_DriveComboBox, CB_SETCURSEL, cbei.iItem, 0 );
         return;
         }

      }//for()

   }//_DirSearchSetDriveText()


void _DirSearchGetDriveText( char *drive )
   {
   int index;


   index = ( int )SendMessage( s_DSN_DriveComboBox, CB_GETCURSEL, 0, 0 );

   SendMessage( s_DSN_DriveComboBox, CB_GETLBTEXT, index, ( LPARAM )drive );

   }//_DirSearchGetDriveText()


void _DirSearchInitDrives( HWND hDlg )
   {
   char             *drive;
   unsigned         length;
   char             drives[ 128 ];
   char             buffer[ 16 ];
   COMBOBOXEXITEM   cbei;


   SendMessage( s_DSN_DriveComboBox, CBEM_SETIMAGELIST, 0, ( LPARAM )s_DSN_ImageList );

   memset( &cbei, 0, sizeof( COMBOBOXEXITEM ) );
   cbei.mask       = CBEIF_IMAGE | CBEIF_TEXT | CBEIF_SELECTEDIMAGE;
   cbei.cchTextMax = 16;
    
   s_DSN_SelectedDrive = _getdrive();
 
   GetLogicalDriveStrings( 128, drives );

   drive  = drives;
   length = ( unsigned )strlen( drive );

   while( 0 < length )
      {
      sprintf( buffer, "%c:", tolower( drive[0] ) );

      cbei.pszText = buffer;

      switch( GetDriveType( drive ) )
         {
         case DRIVE_REMOVABLE   : cbei.iImage = cbei.iSelectedImage = 1; break;
         case DRIVE_RAMDISK     :
         case DRIVE_UNKNOWN     :
         case DRIVE_NO_ROOT_DIR :  
         case DRIVE_REMOTE      :
         case DRIVE_FIXED       : cbei.iImage = cbei.iSelectedImage = 2; break;
         case DRIVE_CDROM       : cbei.iImage = cbei.iSelectedImage = 3; break;

         }//switch()

      SendMessage( s_DSN_DriveComboBox, CBEM_INSERTITEM, 0, ( LPARAM )&cbei );
     
      ++cbei.iItem;

      drive  = drive + length + 1;
      length = ( unsigned )strlen( drive );

      }//while()

   }//_DirSearchInitDrives()


void _DirSearchSetDirText( HWND hDlg )
   {
   char   path[ _MAX_PATH ];
   char  *lock;
   char  *ptr;
   int    length;
   int    numChars;
   int    dirCount;

   
   dirCount = 0;
   length   = ( int )strlen( s_DSN_cwd );

   if( 48 < length )
      {
      sprintf( path, "%c:\\...", s_DSN_cwd[0] );

      lock     = NULL;
      numChars = 0;
      ptr      = s_DSN_cwd + length - 1;
      
      while( 1 )
         {
         while( '\\' != *ptr )
            {
            --ptr;
            ++numChars;
            }

         ++dirCount;

         if( numChars < 40 )
            lock = ptr--;
         else if( 1 == dirCount )
            {
            lock = ptr;
            if( ptr == ( s_DSN_cwd + 2 ) ) 
               path[2] = '\0';
            break;
            }
         else
            break;

         }//while()

      if( NULL != lock )
         strcat( path, lock );

      SetDlgItemText( hDlg, IDS_DSN_DIR, path );
      }
   else
      SetDlgItemText( hDlg, IDS_DSN_DIR, s_DSN_cwd );

   }//_DirSearchSetDirText()


void _DirSearchSetList( HWND hDlg )
   {
   HANDLE           handle;
   WIN32_FIND_DATA  info;
   LVITEM           lvitem;
   int              width;
   int              maxWidth = -1;


   /* Clear the selected directory since 
      we moved into a new one... */
   
   s_DSN_SelDirName[0] = '\0';

   /* Update current directory... */

   _getcwd( s_DSN_cwd, _MAX_PATH );

   _DirSearchSetDirText( hDlg );

   ListView_DeleteAllItems( s_DSN_DirListBox );

   /* Begin search... */

   handle = FindFirstFile( "*", &info );

   if( INVALID_HANDLE_VALUE != handle )
      {
      memset( &lvitem, 0, sizeof( LVITEM ) );
      lvitem.mask       = LVIF_TEXT | LVIF_IMAGE;
      lvitem.cchTextMax = _MAX_PATH;

      do
         {
         if( ( info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && 
              0 != strcmp( ".", info.cFileName ) )
            {
            width = ( int )strlen( info.cFileName );

            if( maxWidth < width )
               width = maxWidth;

            /* If hidden directroy, set "transparent" image... */

            if( info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )
               lvitem.iImage = ( 0 == strcmp( "..", info.cFileName ) ) ? 5 : 6;
            else
               lvitem.iImage = ( 0 == strcmp( "..", info.cFileName ) ) ? 5 : 0;

            lvitem.pszText = info.cFileName;
            ListView_InsertItem( s_DSN_DirListBox, &lvitem );
            ++lvitem.iItem;
            }
         }
      while( FindNextFile( handle, &info ) );

      FindClose( handle );

      //if( -1 != maxWidth )
        // ListView_SetColumnWidth( s_DSN_DirListBox, -1, maxWidth * s_DSN_AveCharWidth );

      }//if( handle )

   }//_DirSearchSetList()


void _DirSearchChangeDrive( HWND hDlg )
   {
   char drive[ 16 ];
   int  idrive;


   _DirSearchGetDriveText( drive );

   idrive = drive[0] - 'a' + 1;

   /* If user selects the drive we're already in, do nothing... */

   if( idrive == _getdrive() )
      return;

   /* Restore the working directory before changing drives... */

   _chdir( s_DSN_OriginalDir );
              
   if( 0 != _chdrive( idrive ) )
      {
      if( 'a' != drive[0] )
         MessageBox( hDlg, "Drive not accessible.", "NeuronStudio",//s_DSN.lpstrTitle,  
                     MB_OK | MB_ICONERROR );
         
      /* Re-set the box... */

      _DirSearchSetDriveText();
      _chdir( s_DSN_cwd );
      }
   else
      {
      s_DSN_SelectedDrive = idrive;  

      /* Save the original directory on this drive... */

      _getcwd( s_DSN_OriginalDir, _MAX_PATH );

      /* If we're at the drive of the last working dir,
         then change to the last working dir... */

      if( drive[0] == tolower( s_DSN_LastWorkingDir[0] ) )
         _chdir( s_DSN_LastWorkingDir );
     
      _DirSearchSetList( hDlg );
      }

   }//_DirSearchChangeDrive()


void _DirSearchDoChdir( HWND hDlg, const char *directory )
	{
   if( 0 == _chdir( directory ) )
      _DirSearchSetList( hDlg );
	}


void _DirSearchChangeDir( HWND hDlg )
   {
   int  index;
   char directory[ _MAX_DIR ];


   index = ListView_GetSelectionMark( s_DSN_DirListBox );

   ListView_GetItemText( s_DSN_DirListBox, index, 0, directory, _MAX_DIR );

	_DirSearchDoChdir( hDlg, directory );

   }//_DirSearchChangeDir()


void _DirSearchSetSelDir( void )
   { 
   int index;

  
   index = ListView_GetSelectionMark( s_DSN_DirListBox );

   if( -1 != index )
      {
      ListView_SetItemState( s_DSN_DirListBox, index, LVIS_SELECTED, LVIS_SELECTED );
      ListView_GetItemText( s_DSN_DirListBox, index, 0, s_DSN_SelDirName, _MAX_DIR );
      }

   }//_DirSearchSetSelDir()


void _DirSearchInitImageList( void )
   {
   HBITMAP hbmp;
   HBITMAP mask;
   BITMAP  bmp;


   hbmp = LoadBitmap( s_DSN.hInstance, "BMP_DIR_SEARCH" );
   mask = LoadBitmap( s_DSN.hInstance, "BMP_MASK_DIR_SEARCH" );

   GetObject( hbmp, sizeof( BITMAP ), &bmp );

   s_DSN_ImageList = ImageList_Create( bmp.bmWidth / 7,
                                       bmp.bmHeight,
                                       ILC_COLOR24 | ILC_MASK,
                                       7, 1 );

   ImageList_Add( s_DSN_ImageList, hbmp, mask );

   ListView_SetImageList( s_DSN_DirListBox, s_DSN_ImageList, LVSIL_SMALL ); 

   DeleteObject( hbmp );
   DeleteObject( mask );

   }//_DirSearchInitImageList()


void _DirSearchInitList( void )
   {
   LVCOLUMN    lvc;

  
   memset( &lvc, 0, sizeof( LVCOLUMN ) );
   lvc.mask = LVCF_FMT | LVCF_WIDTH;
   lvc.fmt  = LVCFMT_LEFT; 
   lvc.cx   = DSN_COL_WIDTH_DIR_NAMES; 

   ListView_InsertColumn( s_DSN_DirListBox, 0, &lvc );

   }//_DirSearchInitList()


void _DirSearchSetInitialDir( void )
   {
   int         drive;
   const char *path;


   /* Set the starting directory for the dialog... */

   if( NULL != s_DSN.lpstrInitialDir )
      path = s_DSN.lpstrInitialDir;
   else
      path = s_DSN_LastWorkingDir;

   drive = path[0];

   if( 0 != drive )
      {
      drive = tolower( drive );

      /* Save the original directory on this drive... */

      drive = drive - 'a' + 1;
 
      if( drive != _getdrive() )
         _chdrive( drive );

      _getcwd( s_DSN_OriginalDir, _MAX_PATH );

      _chdir( path );
      }
   else
      _getcwd( s_DSN_OriginalDir, _MAX_PATH );

   }//_DirSearchSetInitialDir()


LRESULT CALLBACK _DirSearchToolTipHookProc( int nCode, WPARAM wParam, LPARAM lParam )
   {
   static POINT s_DSN_CursorPos;
   MSG *msg; 


   msg = ( MSG* )lParam; 
    
   if( nCode < 0 ) 
      return CallNextHookEx( s_DSN_Hook, nCode, wParam, lParam ); 
 
    switch( msg->message ) 
       { 
       case WM_MOUSEMOVE:
          {
          POINT cursorPos;
          MSG   relay; 
  
          
          relay     = *msg;
          
          GetCursorPos( &cursorPos );

          /* Hide tool tip when cursor moving around list box... */
         
          if( cursorPos.x != s_DSN_CursorPos.x || cursorPos.y != s_DSN_CursorPos.y )
             SendMessage( s_DSN_ToolTip, TTM_POP, 0, 0 );

          GetCursorPos( &s_DSN_CursorPos );

          SendMessage( s_DSN_ToolTip, TTM_RELAYEVENT, 0, ( LPARAM )&relay ); 
          } 
          break; 

       }//switch() 

   return CallNextHookEx( s_DSN_Hook, nCode, wParam, lParam ); 

   } //_DirSearchToolTipHookProc()


void _DirSearchCreateToolTip( HWND hDlg )
   {
   TOOLINFO ti; 


   s_DSN_ToolTip = CreateWindowEx( 0, 
                                   TOOLTIPS_CLASS,
                                   NULL, 
                                   TTS_ALWAYSTIP, 
                                   CW_USEDEFAULT, 
                                   CW_USEDEFAULT, 
                                   CW_USEDEFAULT, 
                                   CW_USEDEFAULT, 
                                   hDlg,
                                   NULL,
                                   s_DSN.hInstance, 
                                   NULL
                                 ); 

  
   s_DSN_Hook = SetWindowsHookEx( WH_GETMESSAGE, 
                                  _DirSearchToolTipHookProc, 
                                  NULL, 
                                  GetCurrentThreadId()
                                ); 

   ti.cbSize   = sizeof( TOOLINFO ); 
   ti.uFlags   = TTF_IDISHWND; 
   ti.hwnd     = hDlg;
   ti.uId      = ( UINT )s_DSN_DirListBox; 
   ti.hinst    = 0; 
   ti.lpszText = LPSTR_TEXTCALLBACK; 

   SendMessage( s_DSN_ToolTip, TTM_ADDTOOL, 0, ( LPARAM )&ti ); 
    
   }//_DirSearchCreateToolTip()


void _DirSearchHandleToolTip( HWND hDlg, LPNMTTDISPINFO di )
   {
   HDC   hDC;
   POINT cursorPt;
   RECT  boxRect;
   RECT  itemRect;
   SIZE  strSize;
   int   k;
   int   count;
	int   done = 0;


   GetCursorPos( &cursorPt );

   GetWindowRect( s_DSN_DirListBox, &boxRect );

   count = ListView_GetItemCount( s_DSN_DirListBox );

   for( k = 0; k < count; ++k )
      {
      ListView_GetItemRect( s_DSN_DirListBox, k, &itemRect, LVIR_BOUNDS );

      if( cursorPt.x >= boxRect.left + itemRect.left  && 
          cursorPt.x <= boxRect.left + itemRect.right && 
          cursorPt.y >= boxRect.top + itemRect.top  && 
          cursorPt.y <= boxRect.top + itemRect.bottom )
         {
         ListView_GetItemText( s_DSN_DirListBox, k, 0, s_DSN_ToolTipBuffer, _MAX_DIR );
         
         /* Only show tool tip if the text width exceeds
            the column width... */

         hDC = GetDC( s_DSN_DirListBox );

         GetTextExtentPoint32( hDC, 
                               s_DSN_ToolTipBuffer, 
                               ( int )strlen( s_DSN_ToolTipBuffer ),
                               &strSize 
                             );

         ReleaseDC( s_DSN_DirListBox, hDC );

         if( DSN_COL_WIDTH_DIR_NAMES < strSize.cx )
            {
            di->hinst    = NULL;
            di->lpszText = s_DSN_ToolTipBuffer;
            }

			done = 1;
         break;

         }//if( in bounds )

      }//for()

/*
	if( ! done )
		{
		GetWindowRect( GetDlgItem( hDlg, IDB_DSN_MKDIR ), &boxRect );

		if( cursorPt.x >= boxRect.left && cursorPt.x < boxRect.right && 
			 cursorPt.y >= boxRect.top  && cursorPt.y < boxRect.bottom )
			{
			di->hinst    = NULL;
			di->lpszText = "Create New Folder";

			done = 1;
			}
		}

	if( ! done )
		{
		GetWindowRect( GetDlgItem( hDlg, IDB_DSN_CDPARENT ), &boxRect );

		if( cursorPt.x >= boxRect.left && cursorPt.x < boxRect.right && 
			 cursorPt.y >= boxRect.top  && cursorPt.y < boxRect.bottom )
			{
			di->hinst    = NULL;
			di->lpszText = "Up One Level";

			done = 1;
			}
		}
*/

   }//_DirSearchHandleToolTip()


void _DirSearchInitButtons( HWND hDlg )
	{
	SendMessage( GetDlgItem( hDlg, IDB_DSN_MKDIR ),
					 BM_SETIMAGE,
					 IMAGE_ICON,
					 ( LPARAM )LoadIcon( s_DSN.hInstance, "ICON_MKDIR" )
				  );

	SendMessage( GetDlgItem( hDlg, IDB_DSN_CDPARENT ),
					 BM_SETIMAGE,
					 IMAGE_ICON,
					 ( LPARAM )LoadIcon( s_DSN.hInstance, "ICON_PARENTCD" )
				  );
	}


void _DirSearchMkdir( HWND hDlg )
	{
   LVITEM lvitem;

	s_DSN_EditItem = ListView_GetItemCount( s_DSN_DirListBox );

	memset( &lvitem, 0, sizeof( LVITEM ) );
	lvitem.mask       = LVIF_TEXT | LVIF_IMAGE;
	lvitem.cchTextMax = _MAX_PATH;
	lvitem.iImage     = 0;
	lvitem.pszText    = "";
	lvitem.iItem      = s_DSN_EditItem;

	ListView_InsertItem( s_DSN_DirListBox, &lvitem );

	SetFocus( s_DSN_DirListBox );
	s_DSN_EditLabel = ListView_EditLabel( s_DSN_DirListBox, s_DSN_EditItem );

	SendMessage( s_DSN_EditLabel, EM_LIMITTEXT, _MAX_PATH, 0 );
	SetWindowText( s_DSN_EditLabel, "New Folder" );
	SendMessage( s_DSN_EditLabel, EM_SETSEL, 0, -1 );
	}


typedef struct __dir_search_hack
	{
	NMHDR hdr;
	LVITEM item;
	}
	_dir_search_hack;


void _DirSearchDoMkdir( HWND hDlg, const char *directory )
	{
   LVITEM lvitem;

	if( NULL == directory )
		{
		ListView_DeleteItem( s_DSN_DirListBox, s_DSN_EditItem );
		return;
		}

	if( 0 == strlen( directory ) )
		{
		ListView_DeleteItem( s_DSN_DirListBox, s_DSN_EditItem );
		MessageBox( hDlg, "Please specify a folder name", "NeuronStudio", MB_OK | MB_ICONERROR ); 
		return;
		}

	if( -1 == _mkdir( directory ) )
		{
		ListView_DeleteItem( s_DSN_DirListBox, s_DSN_EditItem );

		switch( errno )
			{
			case EEXIST:
				MessageBox( hDlg, "A directory with the specified name already exists", "NeuronStudio", MB_OK | MB_ICONERROR );
				break;

			case ENOENT:
				MessageBox( hDlg, "Invalid path specification", "NeuronStudio", MB_OK | MB_ICONERROR );
				break;
			}
		return;
		}

	memset( &lvitem, 0, sizeof( LVITEM ) );
	lvitem.mask       = LVIF_TEXT;
	lvitem.cchTextMax = _MAX_PATH;
	lvitem.pszText    = ( char* )directory;
	lvitem.iItem      = s_DSN_EditItem;

	ListView_SetItem( s_DSN_DirListBox, &lvitem );
	}


BOOL _DirSearchDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_NOTIFY:
			if( ( ( NMHDR* )lParam )->code == LVN_ENDLABELEDIT )
				_DirSearchDoMkdir( hDlg, ( ( _dir_search_hack* )lParam )->item.pszText );
         else if( ( ( NMHDR* )lParam )->code == TTN_NEEDTEXT )
            _DirSearchHandleToolTip( hDlg, ( LPNMTTDISPINFO )lParam );
         else if( IDL_DSN_DIRS == ( ( NMHDR* )lParam )->idFrom )
            {
            switch( ( ( NMHDR* )lParam )->code )
               {
               case NM_DBLCLK:
                  _DirSearchChangeDir( hDlg );
                  break;
           
               case NM_CLICK:
                  _DirSearchSetSelDir();
                  break;
               }
            }

         return ( FALSE );
 
      
      case WM_COMMAND:

         switch( LOWORD( wParam ) )
            {
				case IDB_DSN_CDPARENT:
					_DirSearchDoChdir( hDlg, ".." );
					break;

				case IDB_DSN_MKDIR:
					_DirSearchMkdir( hDlg );
					break;

            case IDC_DSN_DRIVES: 
               if( CBN_SELCHANGE == HIWORD( wParam ) )
                  _DirSearchChangeDrive( hDlg );
               break;

            case IDB_DSN_OK:
               s_DSN_ReturnDirName = TRUE;
               EndDialog( hDlg, 0 );
               break;

            case IDB_DSN_CANCEL:
               s_DSN_ReturnDirName = FALSE;
               EndDialog( hDlg, 0 );
               break;

            }//switch()

         return ( TRUE );


      case WM_INITDIALOG:

         _DirSearchSetInitialDir();

         s_DSN_SelDirName[0] = '\0';

         SetWindowText( hDlg, 
                        ( NULL != s_DSN.lpstrTitle ) ? 
                        s_DSN.lpstrTitle : "Choose Directory" 
                      );
         
         s_DSN_DriveComboBox = GetDlgItem( hDlg, IDC_DSN_DRIVES );
         s_DSN_DirListBox    = GetDlgItem( hDlg, IDL_DSN_DIRS );
 
         _DirSearchCenterDlg( hDlg );

         _DirSearchInitImageList();
         _DirSearchInitList();
         _DirSearchInitDrives( hDlg );

         _DirSearchSetDriveText();
         _DirSearchSetList( hDlg );

         _DirSearchCreateToolTip( hDlg );

			_DirSearchInitButtons( hDlg );

         return ( FALSE );

        
      case WM_CLOSE:

         s_DSN_ReturnDirName = FALSE;

         EndDialog( hDlg, 0 );

         return ( TRUE );

      }//switch()

   return ( FALSE );

   }//_DirSearchDlgProc()


void _DirSearchFinalizeName( void )
   {
   int  sdnLength;
   int  cwdLength;


   /* Save this working directory for the next
      time the dialog box is opened... */

   strcpy( s_DSN_LastWorkingDir, s_DSN_cwd );

   /* Concat the selected, i.e. clicked once, directory
      name if necessary... */

   sdnLength = ( int )strlen( s_DSN_SelDirName );

   if( 0 < sdnLength )
      {
      cwdLength = ( int )strlen( s_DSN_cwd );

      /* Append a '\' character if necessary... */
 
      if( '\\' != s_DSN_cwd[ cwdLength - 1 ] )
         strcat( s_DSN_cwd, "\\" );

      /* If selected is parent, just go up one level... */

      if( 0 == strcmp( "..", s_DSN_SelDirName ) )
         {
         _chdir( ".." );
         _getcwd( s_DSN_cwd, _MAX_PATH );
         }
      else
         strcat( s_DSN_cwd, s_DSN_SelDirName );

      }//if( length )

   strncpy( s_DSN.lpstrDir, s_DSN_cwd, s_DSN.nMaxDir ); 
 
   }//_DirSearchFinalizeName()


BOOL GetDirSearchName( DIRSEARCHNAME *dsn )
   {
   INITCOMMONCONTROLSEX  iccx;

   
   s_DSN = *dsn;
  
   s_DSN_OriginalDrive = _getdrive();

   iccx.dwSize = sizeof( INITCOMMONCONTROLSEX );
   iccx.dwICC  = ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_USEREX_CLASSES;
   InitCommonControlsEx( &iccx );

   DialogBox( s_DSN.hInstance,
              "DLG_DIR_SEARCH",
              s_DSN.hwndOwner,
              ( DLGPROC )_DirSearchDlgProc
            );

	UnhookWindowsHookEx( s_DSN_Hook );

   ImageList_Destroy( s_DSN_ImageList );

   if( s_DSN_ReturnDirName )
      _DirSearchFinalizeName();

   /* The dialog box should not change any of
      the working directories in any drives and
      the current drive should be the same before
      and after the dialog box is called... */

   _chdir( s_DSN_OriginalDir );

   if( s_DSN_OriginalDrive != _getdrive() )
      _chdrive( s_DSN_OriginalDrive );

   return ( s_DSN_ReturnDirName );

   }//GetDirSearchName()
