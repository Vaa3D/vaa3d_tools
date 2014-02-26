

#define __NUM_FILE_TYPES  2
#define __FILE_TYPE_TIFF  0
#define __FILE_TYPE_RAW   1


nsboolean ____saving_projections;


const nschar* __file_filters[ __NUM_FILE_TYPES ] =
	{
	"TIFF (*.tif;*.tiff)",
	"RAW (*.raw)"
	};

#define GetFileFilter( i )\
	__file_filters[(i)]


const nschar* __file_extensions[ __NUM_FILE_TYPES ] =
	{
	".tif",
	".raw"
	}; 

#define GetFileExtension( i )\
	__file_extensions[(i)]



#define __NUM_BIT_DEPTHS  3
#define __BIT_DEPTH_8     0
#define __BIT_DEPTH_12    1
#define __BIT_DEPTH_16    2

NS_PRIVATE const nschar* __bit_depth_names[ __NUM_BIT_DEPTHS ] = { "8-bit", "12-bit", "16-bit" };
NS_PRIVATE nsuint __bit_depth_values[ __NUM_BIT_DEPTHS ] = { 8, 12, 16 };



static HWND             s_VI_hDlg;
static nschar            s_VI_LastDir[ NS_PATH_SIZE ]    = { '\0' };
static nschar            s_VI_CurrDir[ NS_PATH_SIZE ];
static nschar            s_VI_LastPrefix[ NS_PATH_SIZE ] = { '\0' };
static nschar            s_VI_CurrPrefix[ NS_PATH_SIZE ];
static nsint       s_VI_LastFileType            = __FILE_TYPE_TIFF;
static nsint       s_VI_CurrFileType; 
static nsboolean            s_VI_LastDoPadNumber         = NS_TRUE;
static nsboolean            s_VI_CurrDoPadNumber;
static nsboolean            s_VI_LastSaveAsSeries = NS_TRUE;
static nsboolean            s_VI_CurrSaveAsSeries;
static nssize            s_VI_NumIntegratedFiles;
static nsint       ____current_bit_depth, ____actual_bit_depth;


nschar* GetIntegrationLastDir( void )
   {
   return ( s_VI_LastDir );
   }



/* Returns a pointer to the first non-whitespace character. */

const nschar* _ElimIntegrationStrWhitespace( const nschar* string )
   {
   const nschar *start;
   const nschar *end;


   /* Eliminate leading whitespace. */

   for( start = string; '\0' != *start; ++start )
      if( ' ' != *start )
         break;

   /* Eliminate trailing whitespace. */

   for( end = ( string + ns_ascii_strlen( string ) ) - 1; end > string; --end )
      if( ' ' != *end )
         break;

   ++end;
   *( (nschar*)end ) = '\0';

   return ( start );

   }//_ElimIntegrationStrWhitespace()


nsboolean MakeIntegrationFileName( const nschar* prefix, const nschar* extension, nsboolean doPadNumber, 
                               nsint fileNumber, nsint maxFileNumber, nschar* fileName )
   {
   const nschar  *prefixPtr;
   nssize   numChars;
   nssize   padValue;
   char    number[ 64 ];

  
//   vASSERT( vNULL != prefix, vASSERT_POINTER );
  // vASSERT( vNULL != extension, vASSERT_POINTER );
   //vASSERT( vNULL != fileName, vASSERT_POINTER );
   //vASSERT( fileNumber <= maxFileNumber, vASSERT_PARAMETER );

   prefixPtr = _ElimIntegrationStrWhitespace( prefix );

   /* NOTE: Extension assumed to have no whitespace! */

   numChars = ns_ascii_strlen( prefixPtr ) + ns_ascii_strlen( extension );

   if( doPadNumber )
      {
      ns_sprint( number, "%d", maxFileNumber );
      padValue = ns_ascii_strlen( number ) + 1; /* +1 for leading 0. */
      numChars += padValue; 
      }
   else
      {
      ns_sprint( number, "%d", fileNumber );
      numChars += ns_ascii_strlen( number );
      }

   if( numChars > NS_PATH_SIZE - 1 )
      return ( NS_FALSE );

   if( doPadNumber )
      ns_sprint( fileName, "%s%0*d%s", prefixPtr, padValue, fileNumber, extension );
   else
      ns_sprint( fileName, "%s%d%s", prefixPtr, fileNumber, extension );

   return ( NS_TRUE );

   }



typedef struct _NsSaveImageNames
	{
	nschar  proj_xy[ NS_PATH_SIZE ];
	nschar  proj_zy[ NS_PATH_SIZE ];
	nschar  proj_xz[ NS_PATH_SIZE ];
	nschar  series_first[ NS_PATH_SIZE ];
	nschar  series_last[ NS_PATH_SIZE ];
	nschar  volume[ NS_PATH_SIZE ];
	nsint   series_first_num;
	nsint   series_last_num;
	}
	NsSaveImageNames;


NsSaveImageNames *____save_image_names = NULL;


nschar ____fileNames[ NS_PATH_SIZE * 3 + 32 ];


void _DisplayIntegrationFileNames( HWND hDlg )
   {
	const nschar  *extension, *slash;
	nssize         length;


	if( NULL != ____save_image_names )
		{
		____save_image_names->proj_xy[0]       = NS_ASCII_NULL;
		____save_image_names->proj_zy[0]       = NS_ASCII_NULL;
		____save_image_names->proj_xz[0]       = NS_ASCII_NULL;
		____save_image_names->series_first[0]  = NS_ASCII_NULL;
		____save_image_names->series_last[0]   = NS_ASCII_NULL;
		____save_image_names->volume[0]        = NS_ASCII_NULL;
		____save_image_names->series_first_num = 0;
		____save_image_names->series_last_num  = 0;

		slash  = "";
		length = ns_ascii_strlen( s_VI_CurrDir );

		if( ( 0 < length ) && ( NS_UNICHAR_PATH != s_VI_CurrDir[ length - 1 ] ) )
			slash = NS_STRING_PATH;
		}

	if( ____saving_projections )
		{
		extension = GetFileExtension( s_VI_CurrFileType );

		ns_snprint(
			____fileNames,
			NS_ARRAY_LENGTH( ____fileNames ),
			"%s_xy%s  %s_zy%s  %s_xz%s",
			s_VI_CurrPrefix,
			extension,
			s_VI_CurrPrefix,
			extension,
			s_VI_CurrPrefix,
			extension
			);

		if( NULL != ____save_image_names )
			{
			ns_snprint( ____save_image_names->proj_xy, NS_PATH_SIZE, "%s%s%s_xy%s", s_VI_CurrDir, slash, s_VI_CurrPrefix, extension );
			ns_snprint( ____save_image_names->proj_zy, NS_PATH_SIZE, "%s%s%s_zy%s", s_VI_CurrDir, slash, s_VI_CurrPrefix, extension );
			ns_snprint( ____save_image_names->proj_xz, NS_PATH_SIZE, "%s%s%s_xz%s", s_VI_CurrDir, slash, s_VI_CurrPrefix, extension );
			}

		SetDlgItemText( hDlg, IDS_VI_NAMES, ____fileNames );
		}
	else
		{
		/* Make the first and last file name. 
			i.e. file number "1" and file number "total # of files". */

		if( s_VI_CurrSaveAsSeries )
			{
			if( MakeIntegrationFileName(
					s_VI_CurrPrefix,
					GetFileExtension( s_VI_CurrFileType ),
					s_VI_CurrDoPadNumber,
					1,
					( nsint )s_VI_NumIntegratedFiles,
					____fileNames
					) )
				{
				if( NULL != ____save_image_names )
					{
					____save_image_names->series_first_num = 1;
					ns_snprint( ____save_image_names->series_first, NS_PATH_SIZE, "%s%s%s", s_VI_CurrDir, slash, ____fileNames );
					}

				ns_ascii_strcat( ____fileNames, " ... " );

				length = ns_ascii_strlen( ____fileNames );

				if( MakeIntegrationFileName(
						s_VI_CurrPrefix,
						GetFileExtension( s_VI_CurrFileType ),
						s_VI_CurrDoPadNumber,
						( nsint )s_VI_NumIntegratedFiles,
						( nsint )s_VI_NumIntegratedFiles,
						____fileNames + length
						) )
					{
					if( NULL != ____save_image_names )
						{
						____save_image_names->series_last_num = s_VI_NumIntegratedFiles;
						ns_snprint( ____save_image_names->series_last, NS_PATH_SIZE, "%s%s%s", s_VI_CurrDir, slash, ____fileNames + length );
						}

					SetDlgItemText( hDlg, IDS_VI_NAMES, ____fileNames );
					}
				}
			}
		else
			{
			ns_snprint( ____fileNames, NS_ARRAY_LENGTH( ____fileNames ), "%s%s", s_VI_CurrPrefix, GetFileExtension( s_VI_CurrFileType ) );

			if( NULL != ____save_image_names )
				ns_snprint( ____save_image_names->volume, NS_PATH_SIZE, "%s%s%s", s_VI_CurrDir, slash, ____fileNames );

			SetDlgItemText( hDlg, IDS_VI_NAMES, ____fileNames );
			}
		}
   }


nsboolean _save_image_check_for_overwrite( HWND hdlg )
	{
	NsStat     s;
	nsint      series_num;
	NsString   msg;
	nsboolean  check;
	nsboolean  ret;
	NsError    error;


	ns_string_construct( &msg );

	check = NS_FALSE;

	if( NULL != ____save_image_names )
		{
		if( ____saving_projections )
			{
			const nschar *question = " already exists.\nDo you want to replace it?";

			if( NS_SUCCESS( ns_stat( ____save_image_names->proj_xy, &s ), error ) )
				{
				check = NS_TRUE;
				ns_string_append( &msg, ____save_image_names->proj_xy );
				}

			if( NS_SUCCESS( ns_stat( ____save_image_names->proj_zy, &s ), error ) )
				{
				if( check )
					{
					ns_string_append( &msg, " and\n" );
					question = " already exist.\nDo you want to replace them?";
					}

				check = NS_TRUE;
				ns_string_append( &msg, ____save_image_names->proj_zy );
				}

			if( NS_SUCCESS( ns_stat( ____save_image_names->proj_xz, &s ), error ) )
				{
				if( check )
					{
					ns_string_append( &msg, " and\n" );
					question = " already exist.\nDo you want to replace them?";
					}

				check = NS_TRUE;
				ns_string_append( &msg, ____save_image_names->proj_xz );
				}

			if( check )
				ns_string_append( &msg, question );
			}
		else
			{
			if( s_VI_CurrSaveAsSeries )
				{
				for( series_num = ____save_image_names->series_first_num;
					  series_num <= ____save_image_names->series_last_num;
					  ++series_num )
					if( MakeIntegrationFileName(
							s_VI_CurrPrefix,
							GetFileExtension( s_VI_CurrFileType ),
							s_VI_CurrDoPadNumber,
							series_num,
							( nsint )s_VI_NumIntegratedFiles,
							____fileNames
							) )
						if( NS_SUCCESS( ns_stat( ____fileNames, &s ), error ) )
							{
							check = NS_TRUE;

							ns_string_append( &msg, "One or more files in the series from\n" );
							ns_string_append( &msg, ____save_image_names->series_first );
							ns_string_append( &msg, " to\n" );
							ns_string_append( &msg, ____save_image_names->series_last );
							ns_string_append( &msg, " already exists.\nDo you want to replace them?" );

							break;
							}
				}
			else
				{
				if( NS_SUCCESS( ns_stat( ____save_image_names->volume, &s ), error ) )
					{
					check = NS_TRUE;
					ns_string_append( &msg, ____save_image_names->volume );
					ns_string_append( &msg, " already exists.\nDo you want to replace it?" );
					}
				}
			}
		}

	ret = NS_TRUE;

	if( check )
		if( IDNO == MessageBox( hdlg, ns_string_get( &msg ), "NeuronStudio", MB_YESNO | MB_ICONWARNING ) )
			ret = NS_FALSE;

	ns_string_destruct( &msg );

	return ret;
	}


void _InitIntegrationDialog( HWND hDlg )
   {
   HWND  hComboBox;
   nsint  k;


   s_VI_hDlg = hDlg;

   /* Set up the drow down( combo ) boxes. */

   /* File types */

   hComboBox = GetDlgItem( hDlg, IDC_VI_SAVE_AS );

   for( k = 0; k < __NUM_FILE_TYPES; ++k )
      SendMessage( hComboBox,
                   CB_ADDSTRING,
                   0, 
                   ( LPARAM )GetFileFilter( ( nsint )k ) 
                 );
   
   s_VI_CurrFileType = s_VI_LastFileType;

   SendMessage( hComboBox, 
                CB_SELECTSTRING, 
                -1,
                ( LPARAM )GetFileFilter( s_VI_CurrFileType )
              );

   hComboBox = GetDlgItem( hDlg, IDC_VI_BIT_DEPTH );

   for( k = 0; k < __NUM_BIT_DEPTHS; ++k )
      SendMessage( hComboBox,
                   CB_ADDSTRING, 
                   0, 
                   ( LPARAM )__bit_depth_names[ ( nsint )k ]
                 );

   SendMessage( hComboBox, 
                CB_SELECTSTRING, 
                -1,
                ( LPARAM )__bit_depth_names[ ____current_bit_depth ]
              );

   /* Try to initalize the directory path. */

   if( 0 == ns_ascii_strlen( s_VI_LastDir ) )
      {
      //IniSetSection( vINI_SECTION_PATHS );
      //IniSetKey( vINI_KEY_INTEGRATION_PATH );
      //IniReadStringVariable( "\0", NS_PATH_SIZE, s_VI_LastDir );

      //if( 0 == ns_ascii_strlen( s_VI_LastDir ) )
         ns_getcwd( s_VI_LastDir, NS_PATH_SIZE );
      }

   ns_ascii_strcpy( s_VI_CurrDir, s_VI_LastDir );

   /* Set current variables to the values that were inputted last. */

   ns_ascii_strcpy( s_VI_CurrPrefix, s_VI_LastPrefix );

   s_VI_CurrDoPadNumber  = s_VI_LastDoPadNumber;
	s_VI_CurrSaveAsSeries = s_VI_LastSaveAsSeries;

   /* Show 'em in the dialog box. */

   SendMessage( GetDlgItem( hDlg, IDE_VI_DESTINATION ), EM_SETLIMITTEXT,
                NS_PATH_SIZE - 1, 0 );
   SendMessage( GetDlgItem( hDlg, IDE_VI_PREFIX ), EM_SETLIMITTEXT,
                NS_PATH_SIZE - 1, 0 );

   SetDlgItemText( hDlg, IDE_VI_DESTINATION, s_VI_CurrDir );
   SetDlgItemText( hDlg, IDE_VI_PREFIX, s_VI_CurrPrefix );
   /* SetDlgItemText( hDlg, IDS_VI_TYPE_DESCRIBE, GetVolumeIntegrationDescStr( s_VI_CurrFuncType ) ); */

   SendMessage( GetDlgItem( hDlg, IDC_VI_PAD_NUMBER ), 
                BM_SETCHECK,
               ( WPARAM )( ( s_VI_CurrDoPadNumber ) ? BST_CHECKED : BST_UNCHECKED ), 
               0 
              );


   SendMessage( GetDlgItem( hDlg, IDC_VI_SAVE_AS_SERIES ), 
                BM_SETCHECK,
               ( WPARAM )( ( s_VI_CurrSaveAsSeries ) ? BST_CHECKED : BST_UNCHECKED ), 
               0 
              );

	EnableWindow( GetDlgItem( hDlg, IDC_VI_PAD_NUMBER ), s_VI_CurrSaveAsSeries );

   _DisplayIntegrationFileNames( hDlg );

   }/* _InitIntegrationDialog() */


nsboolean _save_image_confirm_bit_depth( HWND dlg )
	{
	nsboolean      ret;
	const nschar  *size_info;
	nschar         msg[256];


	ret = NS_TRUE;

	if( ! ____saving_projections && ____current_bit_depth != ____actual_bit_depth )
		{
		if( ____current_bit_depth < ____actual_bit_depth )
			{
			size_info = ( NS_BITS_TO_BYTES( ____current_bit_depth ) != NS_BITS_TO_BYTES( ____actual_bit_depth ) )
								?
							" a smaller file size, but with" : "";

			ns_snprint(
				msg,
				sizeof( msg ),
				"Decreasing bit depth from " NS_FMT_UINT " bits to " NS_FMT_UINT " bits.\n\n"
				"This will result in" NS_FMT_STRING " a loss of information.\n"
				"Are you sure you want to proceed?\n",
				__bit_depth_values[ ____actual_bit_depth ],
				__bit_depth_values[ ____current_bit_depth ],
				size_info
				);
			}
		else
			{
			size_info = ( NS_BITS_TO_BYTES( ____current_bit_depth ) != NS_BITS_TO_BYTES( ____actual_bit_depth ) )
								?
							" a larger file size with" : "";

			ns_snprint(
				msg,
				sizeof( msg ),
				"Increasing bit depth from " NS_FMT_UINT " bits to " NS_FMT_UINT " bits.\n\n"
				"This will result in" NS_FMT_STRING " no gain in information.\n"
				"Are you sure you want to proceed?\n",
				__bit_depth_values[ ____actual_bit_depth ],
				__bit_depth_values[ ____current_bit_depth ],
				size_info
				);
			}

		ret = ( IDYES == MessageBox( dlg, msg, "NeuronStudio", MB_YESNO | MB_ICONQUESTION ) );
		}

	return ret;
	}


void _HandleIntegrationDlgCommand( HWND hDlg, WPARAM wParam, LPARAM lParam )
   {
   switch( LOWORD( wParam ) ) 
      {
      case IDE_VI_PREFIX:
         if( EN_UPDATE == HIWORD( wParam ) )
            {
            GetDlgItemText( hDlg, IDE_VI_PREFIX, s_VI_CurrPrefix, NS_PATH_SIZE - 1 );
            _DisplayIntegrationFileNames( hDlg );
            }
         break;

      case IDE_VI_DESTINATION:
         if( EN_UPDATE == HIWORD( wParam ) ) 
            GetDlgItemText( hDlg, IDE_VI_DESTINATION, s_VI_CurrDir, NS_PATH_SIZE - 1 );
         break;

      case IDC_VI_SAVE_AS:
         if( CBN_SELCHANGE == HIWORD( wParam ) )
            {
            nsint index = ( nsint )SendMessage( GetDlgItem( hDlg, IDC_VI_SAVE_AS ), CB_GETCURSEL, 0, 0 );
            //vASSERT( index < __NUM_FILE_TYPES, vASSERT_INDEX );
            s_VI_CurrFileType = ( nsint )index;
            _DisplayIntegrationFileNames( hDlg );
            }
         break;

      case IDC_VI_BIT_DEPTH:
         if( CBN_SELCHANGE == HIWORD( wParam ) )
            {
            nsint index = ( nsint )SendMessage( GetDlgItem( hDlg, IDC_VI_BIT_DEPTH ), CB_GETCURSEL, 0, 0 );
            ____current_bit_depth = ( nsint )index;
				//ns_println( "The bit depth is now %s", __bit_depth_names[ ____current_bit_depth ] );
            }
         break;

      case IDB_VI_BROWSE:
         {
         DIRSEARCHNAME dsn;
          
         dsn.hwndOwner       = hDlg;
         dsn.hInstance       = g_Instance;
         dsn.lpstrDir        = s_VI_CurrDir;
         dsn.nMaxDir         = NS_PATH_SIZE;
         dsn.lpstrInitialDir = s_VI_CurrDir;
         dsn.lpstrTitle      = NULL;

         if( GetDirSearchName( &dsn ) )
            SetDlgItemText( hDlg, IDE_VI_DESTINATION, s_VI_CurrDir );
         }
         break;


      case IDC_VI_PAD_NUMBER:
         if( BN_CLICKED == HIWORD( wParam ) )
            {
            s_VI_CurrDoPadNumber = ( nsboolean )( ! s_VI_CurrDoPadNumber );
            _DisplayIntegrationFileNames( hDlg );
            }
         break;

      case IDC_VI_SAVE_AS_SERIES:
         if( BN_CLICKED == HIWORD( wParam ) )
            {
            s_VI_CurrSaveAsSeries = ( nsboolean )( ! s_VI_CurrSaveAsSeries );

				EnableWindow( GetDlgItem( hDlg, IDC_VI_PAD_NUMBER ), s_VI_CurrSaveAsSeries );
            _DisplayIntegrationFileNames( hDlg );
            }
         break;


      case IDOK:
         ns_ascii_strcpy( s_VI_LastDir, s_VI_CurrDir );
         ns_ascii_strcpy( s_VI_LastPrefix, s_VI_CurrPrefix );

         s_VI_LastFileType     = s_VI_CurrFileType;
         s_VI_LastDoPadNumber  = s_VI_CurrDoPadNumber;
			s_VI_LastSaveAsSeries = s_VI_CurrSaveAsSeries;

			if( _save_image_check_for_overwrite( hDlg ) && _save_image_confirm_bit_depth( hDlg ) )
				EndDialog( hDlg, 1 );
         break;

      case IDCANCEL:
         EndDialog( hDlg, 0 );
         break;

      }/* switch( id ) */

   }/* _HandleIntegrationDlgCommand() */


BOOL CALLBACK _IntegrationDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_COMMAND:
         _HandleIntegrationDlgCommand( hDlg, wParam, lParam );
         return ( TRUE );

      case WM_PAINT:
         //_PaintIntegrationDlgBkColor( hDlg );
         return ( FALSE ); /* i.e. let rest of window repaint. */

      case WM_INITDIALOG:
         CenterWindow( hDlg, s_MainWindow.hWnd );
         _InitIntegrationDialog( hDlg );
         SetFocus( GetDlgItem( hDlg, IDE_VI_PREFIX ) );
         return ( FALSE );

      case WM_CLOSE:
         EndDialog( hDlg, 0 );
         return ( TRUE );
      }
   
   return ( FALSE );

   }/* _IntegrationDlgProc() */


nsboolean SaveImageDialog( /*nschar *destDir, nschar *prefix, nsint *destType, nsint *doPadNumber*/ )
   {
   int retValue;
	
	____saving_projections = NS_FALSE;
	____save_image_names   = ns_new( NsSaveImageNames );

   /* Need to init. a few variables before calling dialog. */

   //vASSERT( vNULL != vws, vASSERT_POINTER );
   //s_VI_Workspace = vws;

   //s_VI_PixelCode          = vws->pixelCode;


   //s_VI_NumIntegratedFiles = ns_image_length( workspace_volume( s_MainWindow.activeWorkspace ) );
	s_VI_NumIntegratedFiles = workspace_visual_roi_length( s_MainWindow.activeWorkspace );

	switch( ns_image_pixel_type( workspace_volume( s_MainWindow.activeWorkspace ) ) )
		{
		case NS_PIXEL_LUM_U8:
			____current_bit_depth = ____actual_bit_depth = __BIT_DEPTH_8;
			break;

		case NS_PIXEL_LUM_U12:
			____current_bit_depth = ____actual_bit_depth = __BIT_DEPTH_12;
			break;

		case NS_PIXEL_LUM_U16:
			____current_bit_depth = ____actual_bit_depth = __BIT_DEPTH_16;
			break;

		default:
			ns_assert_not_reached();
		}


		//____volume_length;//GetNumIntegratedFiles( vws ); 

   //EnableThreadWindows( FALSE );

   retValue = ( int )DialogBox( g_Instance,
                         "DLG_INTEGRATION",
                         s_MainWindow.hWnd,
                         ( DLGPROC )_IntegrationDlgProc
                       );

	ns_delete( ____save_image_names );
	____save_image_names = NULL;

//   EnableThreadWindows( TRUE );

   if( 1 == retValue )
      {
      /* Everything OK, so set up the args structure. */

      //vASSERT( vNULL != args->destDir, vASSERT_POINTER );
      //ns_ascii_strcpy( destDir, s_VI_CurrDir );
      
      //vASSERT( vNULL != args->prefix, vASSERT_POINTER );
      //ns_ascii_strcpy( prefix, s_VI_CurrPrefix );

      //args->workspace         = vws;
      //*destType          = s_VI_CurrFileType;
      //args->funcType          = s_VI_CurrFuncType;
      //args->unitsType         = s_VI_VoxelsArgs.units;
      //args->bkgrndColor.red   = GetRValue( s_VI_ConvBkgrndColor );
      //args->bkgrndColor.green = GetGValue( s_VI_ConvBkgrndColor );
      //args->bkgrndColor.blue  = GetBValue( s_VI_ConvBkgrndColor );
      //*doPadNumber       = s_VI_CurrDoPadNumber;  
      //args->voxelX            = s_VI_VoxelsArgs.spacingX;
      //args->voxelY            = s_VI_VoxelsArgs.spacingY;
      //args->voxelZ            = s_VI_VoxelsArgs.spacingZ;

      return NS_TRUE;
      }

   return NS_FALSE;
   }



nsboolean SaveProjectionsDialog( void )
   {
   int retValue;

	____saving_projections = NS_TRUE;
	____save_image_names   = ns_new( NsSaveImageNames );

   retValue = ( int )DialogBox( g_Instance,
                         "SAVE_PROJECTIONS",
                         s_MainWindow.hWnd,
                         ( DLGPROC )_IntegrationDlgProc
                       );

	ns_delete( ____save_image_names );
	____save_image_names = NULL;

   if( 1 == retValue )
      return NS_TRUE;

   return NS_FALSE;
   }
