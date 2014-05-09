
#include <image/raw.h>

static const nschar   *s_Raw_FileName;
static nsint64         s_Raw_Bytes;
static nssize          s_Raw_Width;
static nssize          s_Raw_Height;
static nssize          s_Raw_Length;
static nssize          s_Raw_NumChannels;
static nssize          s_Raw_BitsPerChannel;
static nsint           s_Raw_EndianType;
static nssize          s_Raw_HeaderBytes;
static nsboolean       ____raw_allow_length;


void _InitRawOptionsDialog( HWND hDlg )
   {
   nschar    file[ _MAX_FNAME ];
   nschar    ext[ _MAX_EXT ];
   nschar    buffer[ _MAX_PATH ];
	NsStat64  st;
   nsint     k;
 

   _splitpath( s_Raw_FileName, NULL, NULL, file, ext );
   ns_sprint( buffer, "%s%s", file, ext );
   SetDlgItemText( hDlg, IDS_RAW_FILENAME, buffer );

	ns_stat64( s_Raw_FileName, &st );

   s_Raw_Bytes = st.st_size;
   ns_sprint( buffer, "%I64d bytes", s_Raw_Bytes );

   SetDlgItemText( hDlg, IDS_RAW_FILESIZE, buffer );

   s_Raw_Width  = 0;
   s_Raw_Height = 0;
   s_Raw_Length = 0;

   s_Raw_HeaderBytes = 0;

   ns_sprint( buffer, "0" );
   SetDlgItemText( hDlg, IDE_RAW_HEADER_SIZE, buffer );

   s_Raw_BitsPerChannel = 8;

   SendMessage( GetDlgItem( hDlg, IDR_RAW_CHANNEL_DEPTH_8BIT ), BM_SETCHECK, BST_CHECKED, 0 );
   SendMessage( GetDlgItem( hDlg, IDR_RAW_CHANNEL_DEPTH_16BIT ), BM_SETCHECK, BST_UNCHECKED, 0 );

   s_Raw_EndianType = RAW_LITTLE_ENDIAN;

   SendMessage( GetDlgItem( hDlg, IDR_RAW_CHANNEL_ENDIAN_LITTLE ), BM_SETCHECK, BST_CHECKED, 0 );
   SendMessage( GetDlgItem( hDlg, IDR_RAW_CHANNEL_ENDIAN_BIG ), BM_SETCHECK, BST_UNCHECKED, 0 );

   EnableWindow( GetDlgItem( hDlg, IDR_RAW_CHANNEL_ENDIAN_LITTLE ), FALSE );
   EnableWindow( GetDlgItem( hDlg, IDR_RAW_CHANNEL_ENDIAN_BIG ), FALSE );

   for( k = 1; k <= 4; ++k )
      {
      ns_sprint( buffer, "%d", k );
      SendMessage( GetDlgItem( hDlg, IDC_RAW_CHANNEL_COUNT ), CB_ADDSTRING, 0, ( LPARAM )buffer );
      }

   s_Raw_NumChannels = 1;
   
   SendMessage( GetDlgItem( hDlg, IDC_RAW_CHANNEL_COUNT ), CB_SELECTSTRING, -1, ( LPARAM )"1" );

	if( ! ____raw_allow_length )
		{
		SetDlgItemText( hDlg, IDE_RAW_PIXEL_LENGTH, "1" );
		EnableWindow( GetDlgItem( hDlg, IDE_RAW_PIXEL_LENGTH ), FALSE );
		s_Raw_Length = 1;
		}
   }


typedef enum
   {
   _RAW_OPTIONS_NO_ERROR,
   _RAW_OPTIONS_ERROR_IMAGE_FORMAT,
   _RAW_OPTIONS_ERROR_IMAGE_TOO_SMALL,
   _RAW_OPTIONS_ERROR_IMAGE_TOO_BIG,
   _RAW_OPTIONS_ERROR_WIDTH_ZERO,
   _RAW_OPTIONS_ERROR_HEIGHT_ZERO,
   _RAW_OPTIONS_ERROR_LENGTH_ZERO
   }
   _RAW_OPTIONS_ERROR_TYPE;


int _RetrieveRawOptions( HWND hDlg )
   {
   nsint        temp;
   nschar       buffer[ 256 ] = { '\0' };
   nsint64      bytes;
   NsPixelType  pixelType;


	GetDlgItemText( hDlg, IDE_RAW_PIXEL_WIDTH, buffer, sizeof( buffer ) );
	s_Raw_Width = ns_atoi( buffer );
	GetDlgItemText( hDlg, IDE_RAW_PIXEL_HEIGHT, buffer, sizeof( buffer ) );
	s_Raw_Height = ns_atoi( buffer );
	GetDlgItemText( hDlg, IDE_RAW_PIXEL_LENGTH, buffer, sizeof( buffer ) );
	s_Raw_Length = ns_atoi( buffer );

   temp = ( nsint )SendMessage( GetDlgItem( hDlg, IDC_RAW_CHANNEL_COUNT ), CB_GETCURSEL, 0, 0 );
   ns_assert( ( nsint )CB_ERR != temp );
   s_Raw_NumChannels = temp + 1; /* zero based! */
   ns_assert( 1 <= s_Raw_NumChannels && s_Raw_NumChannels <= 4 );

   temp = ( nsint )SendMessage( GetDlgItem( hDlg, IDR_RAW_CHANNEL_DEPTH_8BIT ), BM_GETCHECK, 0, 0 );
   if( ( nsint )BST_CHECKED == temp )
      {
      s_Raw_BitsPerChannel = 8;
      ns_assert( BST_UNCHECKED ==
               SendMessage( GetDlgItem( hDlg, IDR_RAW_CHANNEL_DEPTH_16BIT ), BM_GETCHECK, 0, 0 )
             );
      }
   else
      {
      s_Raw_BitsPerChannel = 16;
      ns_assert( BST_UNCHECKED ==
               SendMessage( GetDlgItem( hDlg, IDR_RAW_CHANNEL_DEPTH_8BIT ), BM_GETCHECK, 0, 0 )
             );
      }

   if( 8 < s_Raw_BitsPerChannel )
      {
      temp = ( nsint )SendMessage( GetDlgItem( hDlg, IDR_RAW_CHANNEL_ENDIAN_LITTLE ), BM_GETCHECK, 0, 0 );
      if( ( nsint )BST_CHECKED == temp )
         {
         s_Raw_EndianType = RAW_LITTLE_ENDIAN;
         ns_assert( BST_UNCHECKED ==
                  SendMessage( GetDlgItem( hDlg, IDR_RAW_CHANNEL_ENDIAN_BIG ), BM_GETCHECK, 0, 0 )
                );
         }
      else
         {
         s_Raw_EndianType = RAW_BIG_ENDIAN;
         ns_assert( BST_UNCHECKED ==
                  SendMessage( GetDlgItem( hDlg, IDR_RAW_CHANNEL_ENDIAN_LITTLE ), BM_GETCHECK, 0, 0 )
                );
         }
      }

   GetDlgItemText( hDlg, IDE_RAW_HEADER_SIZE, buffer, sizeof( buffer ) );
   s_Raw_HeaderBytes = ns_atoi( buffer );

   pixelType = NS_PIXEL_UNSUPPORTED;
   
   switch( s_Raw_NumChannels )
      {
      case 1:
         if( 8 == s_Raw_BitsPerChannel )
            pixelType = NS_PIXEL_LUM_U8;
         else if( 16 == s_Raw_BitsPerChannel )
            pixelType = NS_PIXEL_LUM_U16;
         break;
      }

   if( NS_PIXEL_UNSUPPORTED == pixelType )
      return _RAW_OPTIONS_ERROR_IMAGE_FORMAT;

   if( 0 == s_Raw_Width )
      return _RAW_OPTIONS_ERROR_WIDTH_ZERO;
   if( 0 == s_Raw_Height )
      return _RAW_OPTIONS_ERROR_HEIGHT_ZERO;
   if( 0 == s_Raw_Length )
      return _RAW_OPTIONS_ERROR_LENGTH_ZERO;

   bytes  = s_Raw_NumChannels;
   bytes *= s_Raw_BitsPerChannel / 8;
   bytes *= s_Raw_Width;
   bytes *= s_Raw_Height;
   bytes *= s_Raw_Length;
   bytes += s_Raw_HeaderBytes;

   if( bytes < s_Raw_Bytes )
      return _RAW_OPTIONS_ERROR_IMAGE_TOO_SMALL;
   else if( s_Raw_Bytes < bytes )
      return _RAW_OPTIONS_ERROR_IMAGE_TOO_BIG;

   return _RAW_OPTIONS_NO_ERROR;
	}


void _RawUnrecoverableErrorBox( HWND hDlg, _RAW_OPTIONS_ERROR_TYPE error )
	{
   nschar *msg;

	switch( error )
		{
		case _RAW_OPTIONS_ERROR_IMAGE_FORMAT:
			msg = "Unsupported image format.";
			break;

		case _RAW_OPTIONS_ERROR_IMAGE_TOO_BIG:
			msg = "Specified image is larger than the file.";
			break;

		case _RAW_OPTIONS_ERROR_WIDTH_ZERO:
			msg = "Width cannot be zero.";
			break;

		case _RAW_OPTIONS_ERROR_HEIGHT_ZERO:
			msg = "Height cannot be zero.";
			break;

		case _RAW_OPTIONS_ERROR_LENGTH_ZERO:
			msg = "Length cannot be zero.";
			break;

      default:
         ns_assert_not_reached();
		}

   MessageBox( hDlg, msg, "NeuronStudio", MB_OK | MB_ICONERROR );
	}


int _RawRecoverableErrorBox( HWND hDlg, _RAW_OPTIONS_ERROR_TYPE error )
	{
   int ok = 0;

	switch( error )
		{
		case _RAW_OPTIONS_ERROR_IMAGE_TOO_SMALL:
         if( IDYES == MessageBox( hDlg, "Specified image is smaller than the file. Proceed?",
                                  "NeuronStudio", MB_YESNO | MB_ICONQUESTION ) )
            ok = 1;
			break;

		default:
			ns_assert_not_reached();
		}

   return ok;
	}


INT_PTR CALLBACK _RawOptionsDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
			SetFocus( GetDlgItem( hDlg, IDE_RAW_PIXEL_WIDTH ) );
         _InitRawOptionsDialog( hDlg );
         CenterWindow( hDlg, s_MainWindow.hWnd );
         return FALSE;

      case WM_COMMAND:
         switch( LOWORD( wParam ) )
            {
            case IDR_RAW_CHANNEL_DEPTH_8BIT:
               EnableWindow( GetDlgItem( hDlg, IDR_RAW_CHANNEL_ENDIAN_LITTLE ), FALSE );
               EnableWindow( GetDlgItem( hDlg, IDR_RAW_CHANNEL_ENDIAN_BIG ), FALSE );               
               break;

            case IDR_RAW_CHANNEL_DEPTH_16BIT:
               EnableWindow( GetDlgItem( hDlg, IDR_RAW_CHANNEL_ENDIAN_LITTLE ), TRUE );
               EnableWindow( GetDlgItem( hDlg, IDR_RAW_CHANNEL_ENDIAN_BIG ), TRUE );               
               break;

            case IDOK:
               {
               _RAW_OPTIONS_ERROR_TYPE error = ( _RAW_OPTIONS_ERROR_TYPE )_RetrieveRawOptions( hDlg );

               switch( error )
                  {
                  case _RAW_OPTIONS_NO_ERROR:
                     EndDialog( hDlg, 1 );
                     break;

                  case _RAW_OPTIONS_ERROR_IMAGE_FORMAT:
                     _RawUnrecoverableErrorBox( hDlg, error );
                     break;
 
                  case _RAW_OPTIONS_ERROR_IMAGE_TOO_SMALL:
                     if( _RawRecoverableErrorBox( hDlg, error ) )
                        EndDialog( hDlg, 1 );
                     break;

                  case _RAW_OPTIONS_ERROR_IMAGE_TOO_BIG:
                     _RawUnrecoverableErrorBox( hDlg, error );
                     break;

                  case _RAW_OPTIONS_ERROR_WIDTH_ZERO:
                     _RawUnrecoverableErrorBox( hDlg, error );
                     break;

                  case _RAW_OPTIONS_ERROR_HEIGHT_ZERO:
                     _RawUnrecoverableErrorBox( hDlg, error );
                     break;

                  case _RAW_OPTIONS_ERROR_LENGTH_ZERO:
                     _RawUnrecoverableErrorBox( hDlg, error );
                     break;
                  }
               }
               break;

            case IDCANCEL:
               EndDialog( hDlg, 0 );
               break;
            }
         return TRUE;

      case WM_CLOSE:
         EndDialog( hDlg, 0 );
         return TRUE;
      }
   
   return FALSE;
	}


extern nssize ____raw_header;
extern RawPixelType ____raw_type;
extern nssize ____raw_width;
extern nssize ____raw_height;
extern nssize ____raw_length;
extern nsint ____raw_endian;


nsboolean RawOptionsDialog( HWND hOwner, const nschar *fileName )
   {
   nsint retValue;

   ns_assert( NULL != fileName );

   s_Raw_FileName = fileName;

   retValue = ( nsint )DialogBox( g_Instance,
                         "DLG_RAW_OPTIONS",
                         hOwner,
                         _RawOptionsDlgProc
                       );

   if( 1 == retValue )
      {
		____raw_width  = s_Raw_Width;
		____raw_height = s_Raw_Height;
		____raw_length = s_Raw_Length;
		____raw_endian = s_Raw_EndianType;
		____raw_header = s_Raw_HeaderBytes;

		ns_assert( 1 == s_Raw_NumChannels );

		switch( s_Raw_BitsPerChannel )
			{
			case 8:
				____raw_type = RAW_PIXEL_LUM8;
				break;

			case 16:
				____raw_type = RAW_PIXEL_LUM16;
				break;
			}

      return NS_TRUE;
      }

   return NS_FALSE;
   }