#include "app_lib.h"

#include <std/nsio.h>
#include <std/nsascii.h>
#include <std/nsmemory.h>


extern void ____ns_progress_update( nsfloat percent, NsIO *io );
extern nsint ____ns_progress_cancelled( NsIO *io );


eERROR_TYPE g_Error = 0;


const nschar* g_ErrorStrings[] = 
   {
   "No errors",
   "Out of memory ( RAM )",
   "Could not open a file",
   "Could not read all or part of a file",
   "Could not write all or part of a file",
   "A file was initially or became invalid",
   "A file was a format not currently supported",
   "The end of a file was unexpectedly reached",
   "Operating system error",
   "Unsupported pixel format",
   "Unsupported compression type",
   "Multiple pixel formats",
   "Multiple dimensions",
   "Maximum size exceeded",
   "Invalid directory",
	"Sequences with 3D files not currently supported"
   };


HINSTANCE g_Instance = NULL;

/* Column-major matrix:

  0  4   8  12
  1  5   9  13
  2  6  10  14
  3  7  11  15
*/


void IdentityMatrix44( nsfloat m[16] )
   {
   m[ 0] = 1.0f; m[ 4] = 0.0f; m[ 8] = 0.0f; m[12] = 0.0f;
   m[ 1] = 0.0f; m[ 5] = 1.0f; m[ 9] = 0.0f; m[13] = 0.0f;
   m[ 2] = 0.0f; m[ 6] = 0.0f; m[10] = 1.0f; m[14] = 0.0f;
   m[ 3] = 0.0f; m[ 7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;   
   }


void ConstructImage( Image* image )
   {
   ns_assert( NULL != image );

   image->width  = 0;
   image->height = 0;
   image->pixels = NULL;
   }


void DestructImage( Image* image )
   {
   ns_assert( NULL != image );

   ns_free( image->pixels );

   image->width  = 0;
   image->height = 0;
   image->pixels = NULL;
   }


eIMAGE_FILE_TYPE GetImageFileType( const nschar* fileName )
   {
   nschar extension[ _MAX_EXT + 1 ];


   _splitpath( fileName, NULL, NULL, NULL, extension );
   ns_ascii_strlwr( extension ); 

   if( 0 == ns_ascii_strcmp( extension, ".tif" ) )
      return eIMAGE_FILE_TIFF;
   
   if( 0 == ns_ascii_strcmp( extension, ".tiff" ) )
      return eIMAGE_FILE_TIFF;

	if( 0 == ns_ascii_strcmp( extension, ".raw" ) )
		return eIMAGE_FILE_RAW;

	if( 0 == ns_ascii_strcmp( extension, ".lsm" ) )
		return IMAGE_FILE_LSM;

   return eIMAGE_FILE_UNSUPPORTED;
   }


eERROR_TYPE _TranslateTiffErrorCode( const TiffErrorType tiffError )
   {
   eERROR_TYPE error;


   ns_assert( TIFF_NO_ERROR != tiffError );

   switch( tiffError )
      {
      case TIFF_ERROR_FILE_OPEN          : error = eERROR_FILE_OPEN;     break;
      case TIFF_ERROR_FILE_READ          : error = eERROR_FILE_READ;     break;
      case TIFF_ERROR_FILE_WRITE         : error = eERROR_FILE_WRITE;    break;
      case TIFF_ERROR_FILE_SEEK          : error = eERROR_EOF;           break;
      case TIFF_ERROR_FILE_INVALID       : error = eERROR_FILE_INVALID;  break;
      case TIFF_ERROR_VERSION            : error = eERROR_FILE_INVALID;  break;
      case TIFF_ERROR_OUT_OF_MEMORY      : error = eERROR_OUT_OF_MEMORY; break;
      case TIFF_ERROR_COMPRESSED         : error = eERROR_COMPRESSION;   break;
      case TIFF_ERROR_UNSUPPORTED        : error = eERROR_PIXEL_FORMAT;  break;
		case TIFF_ERROR_NOT_ENOUGH_SPACE   : error = eERROR_MULTIPLE_2D_3D_FILES; break;

      default:
         ns_assert( 0 );
      }

   return error;
   }



extern void _do_workspace_set_lsm_info( const TiffLsmInfo *lsm_info );
extern void _do_workspace_add_image_extra_info( const nschar *title, const nschar *info );


eERROR_TYPE _ReadTiffImage
	(
	Image         *image,
	nspointer      buffer,
	nssize         bytes,
	const nschar  *fileName,
	NsIO          *io,
	nsboolean      is_lsm,
	nsulong       *max_intensity
	)
   {
	TiffFile       tiff;
	TiffErrorType  error;


	image->pixels = NULL;

	if( is_lsm )
		tiff_file_construct_lsm( &tiff, io );
	else
		tiff_file_construct( &tiff, io );

	if( NULL == buffer )
		{
		if( TIFF_NO_ERROR !=
			( error = tiff_file_read(
							&tiff,
							fileName,
							NULL != io->progress ? ____ns_progress_cancelled : NULL,
							NULL != io->progress ? ____ns_progress_update : NULL,
							io
							) ) )
			{
			tiff_file_destruct( &tiff, io );
			return ( g_Error = _TranslateTiffErrorCode( error ) );
			}

		if( NULL != io->progress && ____ns_progress_cancelled( io ) )
			{
			tiff_file_destruct( &tiff, io );
			return eNO_ERROR;
			}

		ns_assert( NULL != tiff.pixels );

		image->pixels = tiff.pixels;
		tiff.pixels   = NULL;
		}
	else
		{
		if( TIFF_NO_ERROR !=
			( error = tiff_file_read_ex(
							&tiff,
							fileName,
							buffer,
							bytes,
							NULL != io->progress ? ____ns_progress_cancelled : NULL,
							NULL != io->progress ? ____ns_progress_update : NULL,
							io
							) ) )
			{
			tiff_file_destruct( &tiff, io );
			return ( g_Error = _TranslateTiffErrorCode( error ) );
			}

		if( NULL != io->progress && ____ns_progress_cancelled( io ) )
			{
			tiff_file_destruct( &tiff, io );
			return eNO_ERROR;
			}

		ns_assert( NULL == tiff.pixels );
		}

	_do_workspace_add_image_extra_info( "DOCUMENT NAME:\n", tiff.document_name );
	_do_workspace_add_image_extra_info( "IMAGE DESCRIPTION:\n", tiff.image_description );
	_do_workspace_add_image_extra_info( "MAKE:\n", tiff.make );
	_do_workspace_add_image_extra_info( "MODEL:\n", tiff.model );
	_do_workspace_add_image_extra_info( "PAGE NAME:\n", tiff.page_name );
	_do_workspace_add_image_extra_info( "SOFTWARE:\n", tiff.software );
	_do_workspace_add_image_extra_info( "DATE/TIME:\n", tiff.date_time );
	_do_workspace_add_image_extra_info( "ARTIST:\n", tiff.artist );
	_do_workspace_add_image_extra_info( "HOST COMPUTER:\n", tiff.host_computer );

	if( is_lsm && tiff.is_lsm && NULL != tiff.lsm_info )
		{
		nschar info[64];

		ns_snprint( info, sizeof( info ), NS_FMT_INT, ( nsint )tiff.lsm_info->dimension_x );
		_do_workspace_add_image_extra_info( "Image Width (X): ", info );

		ns_snprint( info, sizeof( info ), NS_FMT_INT, ( nsint )tiff.lsm_info->dimension_y );
		_do_workspace_add_image_extra_info( "Image Height (Y): ", info );

		ns_snprint( info, sizeof( info ), NS_FMT_INT, ( nsint )tiff.lsm_info->dimension_z );
		_do_workspace_add_image_extra_info( "Image Length (Z): ", info );

		ns_snprint( info, sizeof( info ), NS_FMT_INT, ( nsint )tiff.lsm_info->dimension_channels );
		_do_workspace_add_image_extra_info( "# Of Channels: ", info );

		ns_snprint( info, sizeof( info ), NS_FMT_INT, ( nsint )tiff.lsm_info->dimension_time );
		_do_workspace_add_image_extra_info( "# Of Time Series: ", info );

		switch( ( nsint )tiff.lsm_info->s_data_type )
			{
			case 1: ns_snprint( info, sizeof( info ), NS_FMT_STRING, "\"8-bit unsigned integer\""      ); break;
			case 2: ns_snprint( info, sizeof( info ), NS_FMT_STRING, "\"12-bit unsigned integer\""     ); break;
			case 5: ns_snprint( info, sizeof( info ), NS_FMT_STRING, "\"32-bit float\""                ); break;
			case 0: ns_snprint( info, sizeof( info ), NS_FMT_STRING, "\"varies depending on channel\"" ); break;
			}
		_do_workspace_add_image_extra_info( "Channel Format: ", info );

		ns_snprint( info, sizeof( info ), NS_FMT_INT, ( nsint )tiff.lsm_info->thumbnail_x );
		_do_workspace_add_image_extra_info( "Thumbnail Width (X): ", info );

		ns_snprint( info, sizeof( info ), NS_FMT_INT, ( nsint )tiff.lsm_info->thumbnail_y );
		_do_workspace_add_image_extra_info( "Thumbnail Height (Y): ", info );

		ns_snprint( info, sizeof( info ), "%.6f", tiff.lsm_info->voxel_size_x );
		_do_workspace_add_image_extra_info( "Voxel Width (X): ", info );

		ns_snprint( info, sizeof( info ), "%.6f", tiff.lsm_info->voxel_size_y );
		_do_workspace_add_image_extra_info( "Voxel Height (Y): ", info );

		ns_snprint( info, sizeof( info ), "%.6f", tiff.lsm_info->voxel_size_z );
		_do_workspace_add_image_extra_info( "Voxel Length (Z): ", info );

		_do_workspace_set_lsm_info( tiff.lsm_info );
		}

   /* NOTE: Only grayscale currently supported. */
   ns_assert( 8 == tiff.pixel_info.luminance_bits ||
          12 == tiff.pixel_info.luminance_bits ||
          16 == tiff.pixel_info.luminance_bits );

	if( ( nsulong )tiff.max_intensity > *max_intensity )
		*max_intensity = ( nsulong )tiff.max_intensity;

	/* Only 2D images supported! */
	//ns_assert( 1 == tiff.length );

   image->bitsPerPixel = tiff.pixel_info.luminance_bits;
   image->width        = tiff.width;
   image->height       = tiff.height;
	image->length       = tiff.length;
   image->saBoundary   = 1;

	tiff_file_destruct( &tiff, io );

   return eNO_ERROR;
   }


eERROR_TYPE _TranslateRawErrorCode( const RawErrorType raw_error )
   {
   eERROR_TYPE error;

   ns_assert( RAW_NO_ERROR != raw_error );

   switch( raw_error )
      {
      case RAW_ERROR_FILE_OPEN     : error = eERROR_FILE_OPEN;     break;
      case RAW_ERROR_FILE_READ     : error = eERROR_FILE_READ;     break;
      case RAW_ERROR_FILE_WRITE    : error = eERROR_FILE_WRITE;    break;
      case RAW_ERROR_FILE_SEEK     : error = eERROR_EOF;           break;
      case RAW_ERROR_OUT_OF_MEMORY : error = eERROR_OUT_OF_MEMORY; break;
		case RAW_ERROR_NOT_ENOUGH_SPACE : error = eERROR_MULTIPLE_2D_3D_FILES; break;

      default:
         ns_assert( 0 );
      }

   return error;
   }


nssize ____raw_header;
RawPixelType ____raw_type;
nssize ____raw_width;
nssize ____raw_height;
nssize ____raw_length;
nsint ____raw_endian;


eERROR_TYPE _read_raw_image
	(
	Image         *image,
	nspointer      buffer,
	nssize         bytes,
	const nschar  *fileName,
	NsIO          *io,
	nsulong       *max_intensity
	)
   {
	RawErrorType  error;
	unsigned int  raw_max_intensity;


	image->pixels = NULL;

/*TEMP??? Do we need to free the pixels on an error??? */

	if( NULL == buffer )
		{
		if( RAW_NO_ERROR !=
			( error = raw_file_read(
					fileName,
					NULL,
					0,
					____raw_type,
					____raw_width,
					____raw_height,
					____raw_length,
					____raw_endian,
					&image->pixels,
					&raw_max_intensity,
					NULL != io->progress ? ____ns_progress_cancelled : NULL,
					NULL != io->progress ? ____ns_progress_update : NULL,
					io
					 ) ) )
			{
			ns_assert( NULL == image->pixels );
			return ( g_Error = _TranslateRawErrorCode( error ) );
			}

		if( NULL != io->progress && ____ns_progress_cancelled( io ) )
			{
			ns_assert( NULL == image->pixels );
			return eNO_ERROR;
			}

		ns_assert( NULL != image->pixels );
		}
	else
		{
		if( RAW_NO_ERROR !=
			( error = raw_file_read_ex(
					fileName,
					NULL,
					0,
					____raw_type,
					____raw_width,
					____raw_height,
					____raw_length,
					____raw_endian,
					buffer,
					bytes,
					&raw_max_intensity,
					NULL != io->progress ? ____ns_progress_cancelled : NULL,
					NULL != io->progress ? ____ns_progress_update : NULL,
					io
					) ) )
			{
			ns_assert( NULL == image->pixels );
			return ( g_Error = _TranslateRawErrorCode( error ) );
			}

		if( NULL != io->progress && ____ns_progress_cancelled( io ) )
			{
			ns_assert( NULL == image->pixels );
			return eNO_ERROR;
			}

		ns_assert( NULL == image->pixels );
		}

	switch( ____raw_type )
		{
		case RAW_PIXEL_LUM8:
			image->bitsPerPixel = 8;
			break;

		case RAW_PIXEL_LUM12:
			image->bitsPerPixel = 12;
			break;

		case RAW_PIXEL_LUM16:
			image->bitsPerPixel = 16;
			break;
		}

   image->width        = ____raw_width;
   image->height       = ____raw_height;
	image->length       = ____raw_length;
   image->saBoundary   = 1;

	if( ( nsulong )raw_max_intensity > *max_intensity )
		*max_intensity = ( nsulong )raw_max_intensity;

   return eNO_ERROR;
   }



eERROR_TYPE _DoReadImage
	(
	Image         *image,
	nspointer      buffer,
	nssize         bytes,
	const nschar  *fileName,
	NsProgress    *progress,
	nsulong       *max_intensity
	)
	{
	NsIO         io;
	NsFile       file;
	eERROR_TYPE  error;
	nsboolean    is_lsm;


	ns_file_construct( &file );

	io.progress = progress;
	io.file     = &file;

	error = eNO_ERROR;

   ns_assert( NULL != image );
   ns_assert( NULL != fileName );

	is_lsm = NS_FALSE;

   switch( GetImageFileType( fileName ) )
      {
		case IMAGE_FILE_LSM:
			is_lsm = NS_TRUE;
      case eIMAGE_FILE_TIFF:
         error = _ReadTiffImage( image, buffer, bytes, fileName, &io, is_lsm, max_intensity );
         break;

		case eIMAGE_FILE_RAW:
			error = _read_raw_image( image, buffer, bytes, fileName, &io, max_intensity );
			break;

      default:
         error = g_Error = eERROR_FILE_FORMAT;
      }

	ns_file_destruct( &file );

   return error;
	}


eERROR_TYPE ReadImage3D( Image* image, const nschar *file, NsProgress *progress, nsulong *max_intensity )
   {  return _DoReadImage( image, NULL, 0, file, progress, max_intensity );  }


eERROR_TYPE ReadImage2D( Image *image, const nschar *file, nspointer buffer, nssize bytes, nsulong *max_intensity )
	{  return _DoReadImage( image, buffer, bytes, file, NULL, max_intensity );  }


Vector3i* MakeVector3i( Vector3i* v, nslong x, nslong y, nslong z )
   {
   v->x = x;
   v->y = y;
   v->z = z;

   return v;
   }


Vector2i* MakeVector2i( Vector2i* v, nslong x, nslong y )
   {
   v->x = x;
   v->y = y;

   return v;
   }




Vector2i* AddVector2i( const Vector2i* A, const Vector2i* B, Vector2i* C )
   {
   C->x = A->x + B->x;
   C->y = A->y + B->y;

   return ( C );
   }


Vector2i* SubVector2i( const Vector2i* A, const Vector2i* B, Vector2i* C )
   {
   C->x = A->x - B->x;
   C->y = A->y - B->y;

   return ( C );
   }



Vector3f* AddVector3f( const Vector3f* A, const Vector3f* B, Vector3f* C )
   {
   C->x = A->x + B->x;
   C->y = A->y + B->y;
   C->z = A->z + B->z;

   return C;
   }

Vector3f* SubVector3f( const Vector3f* A, const Vector3f* B, Vector3f* C )
   {
   C->x = A->x - B->x;
   C->y = A->y - B->y;
   C->z = A->z - B->z;

   return C;
   }


nsfloat MagnitudeVector3f( const Vector3f* V )
   {  return ( nsfloat )sqrt( V->x*V->x + V->y*V->y + V->z*V->z );  }


Vector3f* NormalizeVector3f( Vector3f* V )
   {
   nsfloat mag = MagnitudeVector3f( V );
   
   V->x /= mag;
   V->y /= mag;
   V->z /= mag;

   return V;
   }


Vector3f* CrossProductVector3f( const Vector3f* A, const Vector3f* B, Vector3f* C )
   {
   C->x = A->y*B->z - A->z*B->y;
   C->y = A->z*B->x - A->x*B->z;
   C->z = A->x*B->y - A->y*B->x;

   return C;
   }


nsfloat DotProductVector3f( const Vector3f* A, const Vector3f* B )
   {  return A->x*B->x + A->y*B->y + A->z*B->z;  }


Vector3f* ScaleVector3f( Vector3f* V, const nsfloat scalar )
   {
   V->x *= scalar;
   V->y *= scalar;
   V->z *= scalar;

   return V;
   }



RECT* MakeRectangle( RECT *r, nslong left, nslong top, nslong right, nslong bottom )
   {
   r->left   = ( LONG )left;
   r->top    = ( LONG )top;
   r->right  = ( LONG )right;
   r->bottom = ( LONG )bottom;

   return r;
   }


void _DrawRaisedGraphicsBorder3D
   (
   HDC         graphics,
   const RECT  *rect,
   HPEN        hilitePen,
   HPEN        shadowPen,
   HPEN        darkShadowPen,
   nsint         isThin
   )
   {
   SelectObject( graphics, hilitePen );
  
   MoveToEx( graphics, rect->left, rect->top, NULL );
   LineTo( graphics, rect->right - 1, rect->top );
   MoveToEx( graphics, rect->left, rect->top, NULL );
   LineTo( graphics, rect->left, rect->bottom - 1 );

   SelectObject( graphics, ( isThin ) ? shadowPen : darkShadowPen );

   MoveToEx( graphics, rect->left, rect->bottom - 1, NULL );
   LineTo( graphics, rect->right, rect->bottom - 1 );
   MoveToEx( graphics, rect->right - 1, rect->top, NULL );
   LineTo( graphics, rect->right - 1, rect->bottom );

   if( ! isThin )
      {
      SelectObject( graphics, shadowPen );

      MoveToEx( graphics, rect->left + 1, rect->bottom - 2, NULL );
      LineTo( graphics, rect->right - 1, rect->bottom - 2 );
      MoveToEx( graphics, rect->right - 2, rect->top + 1, NULL );
      LineTo( graphics, rect->right - 2, rect->bottom - 1 );
      }

   }/* _DrawRaisedUIGraphicsBorder3D() */


void _DrawSunkenGraphicsBorder3D
   (
   HDC         graphics,
   const RECT  *rect,
   HPEN        hilitePen,
   HPEN        shadowPen,
   HPEN        darkShadowPen,
   nsint         isThin
   )
   {
   SelectObject( graphics, shadowPen );

   MoveToEx( graphics, rect->left, rect->top, NULL );
   LineTo( graphics, rect->right - 1, rect->top );
   MoveToEx( graphics, rect->left, rect->top, NULL );
   LineTo( graphics, rect->left, rect->bottom - 1 );

   SelectObject( graphics, hilitePen );

   MoveToEx( graphics, rect->left, rect->bottom - 1, NULL );
   LineTo( graphics, rect->right, rect->bottom - 1 );
   MoveToEx( graphics, rect->right - 1, rect->top, NULL );
   LineTo( graphics, rect->right - 1, rect->bottom );

   if( ! isThin )
      {
      SelectObject( graphics, darkShadowPen );
 
      MoveToEx( graphics, rect->left + 1, rect->top + 1, NULL );
      LineTo( graphics, rect->right - 2, rect->top + 1 );
      MoveToEx( graphics, rect->left + 1, rect->top + 1, NULL );
      LineTo( graphics, rect->left + 1, rect->bottom - 2 );
      }
   }


void ColorThinGraphicsBorder3D( HDC graphics,
                                  const RECT *rect,
                                  nsuint hiliteColor,
                                  nsuint shadowColor,
                                  e3D_BORDER_TYPE type
                                )
   {
   HPEN hilitePen;
   HPEN shadowPen;
   HPEN oldPen;

 
   ns_assert( NULL != rect );

   hilitePen = CreatePen( PS_SOLID, 1, hiliteColor );
   shadowPen = CreatePen( PS_SOLID, 1, shadowColor );

   oldPen = ( HPEN )GetCurrentObject( graphics, OBJ_PEN );

   switch( type )
      {
      case e3D_BORDER_RAISED:
         _DrawRaisedGraphicsBorder3D( graphics, rect, hilitePen, shadowPen, NULL, 1 );
         break;

      case e3D_BORDER_SUNKEN:
         _DrawSunkenGraphicsBorder3D( graphics, rect, hilitePen, shadowPen, NULL, 1 );
         break;
      }

   SelectObject( graphics, oldPen );

   DeleteObject( hilitePen );
   DeleteObject( shadowPen );
   }



void ColorGraphicsBorder( HDC graphics, const RECT* rect, nsuint width, nsuint color )
   {
   nsuint i;
   HBRUSH hBrush;
   RECT rc; 
  

   ns_assert( NULL != rect );
   rc = *rect;

    hBrush = CreateSolidBrush( color );

   for( i = 0; i < width; ++i )
       {
       FrameRect( graphics, &rc, hBrush );
       rc.left += 1;
       rc.top +=1;
       rc.right -= 1;
       rc.bottom -= 1;
       }

   DeleteObject( hBrush );
   }


SIZE* MakeSize( SIZE* size, nslong cx, nslong cy )
   {
   ns_assert( NULL != size );

   size->cx = ( LONG )cx;
   size->cy = ( LONG )cy;

   return size;
   }


POINT* MakePoint( POINT* pt, nslong x, nslong y )
   {
   ns_assert( NULL != pt );

   pt->x = ( LONG )x;
   pt->y = ( LONG )y;

   return pt;
   }


static HWND s_FileOpenNameDialogOwner;
/*
UINT_PTR CALLBACK _FileOpenNameDialogHookProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   if( WM_INITDIALOG == uMsg )
      CenterWindow( GetParent( hDlg ), s_FileOpenNameDialogOwner );

   return 0;
   }
*/


static int ____last_selected_filter = 1;

nsint FileOpenNameDialog
   (
   HWND           owner, 
   const nschar*  title,
   nschar*        path,
   const nsuint   maxPath,
   const nschar*  filter,
   const nschar*  directory,
   const nschar*  extension
   )
   {
   OPENFILENAME  ofn;
	BOOL          ret;


   ns_assert( NULL != path );

	s_FileOpenNameDialogOwner = owner;

   ns_memset( &ofn, 0, sizeof( OPENFILENAME ) );

   ofn.lStructSize     = sizeof( OPENFILENAME );
   ofn.hwndOwner       = owner;
   ofn.lpstrFile       = path;
   ofn.nMaxFile        = maxPath;
   ofn.lpstrTitle      = title;
   ofn.lpstrFilter     = filter;
   ofn.nFilterIndex    = ____last_selected_filter;
   ofn.lpstrInitialDir = directory;
   ofn.lpstrDefExt     = extension;
   ofn.Flags           = OFN_READONLY | OFN_PATHMUSTEXIST /*| OFN_ENABLEHOOK*/ | OFN_EXPLORER;
   //ofn.lpfnHook        = _FileOpenNameDialogHookProc;

   ret = GetOpenFileName( &ofn );

   if( ret )
      ____last_selected_filter = ofn.nFilterIndex;

	return ret;
   }


nsint CreateMemoryGraphics( MemoryGraphics* graphics, HDC hDC, nsuint width, nsuint height )
   {
   ns_assert( NULL != graphics );

   graphics->hDC = NULL;
   graphics->hBuffer = NULL;
   graphics->hOriginalBuffer = NULL;

   graphics->hDC              = CreateCompatibleDC( hDC );
   graphics->hBuffer          = CreateCompatibleBitmap( hDC, width, height );
   graphics->hOriginalBuffer  = SelectObject( graphics->hDC, graphics->hBuffer );
   graphics->originalSwapMode = SetStretchBltMode( graphics->hDC, COLORONCOLOR );
   graphics->width            = width;
   graphics->height           = height;

   if( NULL == graphics->hDC || NULL == graphics->hBuffer )
      {
		if( NULL == graphics->hDC )
			ns_println( "DEVICE CONTEXT CREATION ERROR!" );

		if( NULL == graphics->hBuffer )
			ns_println( "BITMAP CREATION ERROR!" );

      g_Error = eERROR_OPER_SYSTEM;
      DestroyMemoryGraphics( graphics );
      return 0;
      }

   return 1;
   }


void DestroyMemoryGraphics( MemoryGraphics* graphics )
   {
   ns_assert( NULL != graphics );

   if( NULL != graphics->hDC )
      {
      if( 0 != graphics->originalSwapMode )
         SetStretchBltMode( graphics->hDC, graphics->originalSwapMode );

      if( NULL != graphics->hOriginalBuffer )
         SelectObject( graphics->hDC, graphics->hOriginalBuffer );

      if( NULL != graphics->hBuffer )
         DeleteObject( graphics->hBuffer );

		DeleteDC( graphics->hDC );
      }

   graphics->hDC              = NULL;
   graphics->hBuffer          = NULL;
   graphics->hOriginalBuffer  = NULL;
   graphics->originalSwapMode = 0;
   graphics->width            = 0;
   graphics->height           = 0;
   }


extern nsboolean ____workspace_show_tiles;

NS_COMPILE_TIME_SIZE_EQUAL( RGBQUAD, NsVector4ub );

void DrawImageIntoMemoryGraphics( MemoryGraphics* graphics, const NsImage* image, 
                                  Palette* palette )
   {
	RECT rc;

   ns_assert( NULL != graphics );
   ns_assert( NULL != image );
   ns_assert( NULL != palette );

   //SelectObject( graphics->hDC, graphics->hOriginalBuffer );

	ns_assert( NS_PIXEL_LUM_U8 == ns_image_pixel_type( image ) );
	ns_assert( sizeof( LONG ) == ns_image_row_align( image ) );
   //ns_assert( 8 == image->bitsPerPixel );

   palette->h.biSize          = sizeof( BITMAPINFOHEADER );
   palette->h.biWidth         = ( nsint )ns_image_width( image );
   palette->h.biHeight        = -( ( nsint )ns_image_height( image ) );
   palette->h.biPlanes        = 1;
   palette->h.biBitCount      = 8;
   palette->h.biCompression   = BI_RGB;
   palette->h.biSizeImage     = 0;
   palette->h.biXPelsPerMeter = 0;
   palette->h.biYPelsPerMeter = 0;
   palette->h.biClrUsed       = 0;
   palette->h.biClrImportant  = 0;

	SetDIBits( 
		graphics->hDC,
		graphics->hBuffer,
		0,
		( nsuint )ns_image_height( image ),
		ns_image_pixels( image ),
		( BITMAPINFO* )palette,
		DIB_RGB_COLORS
		);

	if( ____workspace_show_tiles )
		{
		rc.left = rc.top = 0;
		rc.right = ( nsint )ns_image_width( image );
		rc.bottom = ( nsint )ns_image_height( image );
		FrameRect( graphics->hDC, &rc, GetStockObject( WHITE_BRUSH ) );
		}

   //SelectObject( graphics->hDC, graphics->hBuffer );
   }



typedef struct tagProgress
   {
   HWND        owner;
   HWND        hWnd;
	HWND        hBar;
   const nschar  *title;
   nsint          wasCancelled;
   void        ( *function )( void* );
   void        *inArgs;
   void        *outArgs;
   nsfloat        percent;
	eERROR_TYPE  error;
   }
   ProgressDlg;


static ProgressDlg s_Progress;




nsboolean ____progress_slider_running;
HANDLE ____progress_slider_handle;
nsint ____progress_slider_mode = 0;
nsint ____progress_slider_direction = 0;
nsint ____progress_slider_left = 0;
#define _PROGRESS_SLIDER_WIDTH  32
#define _PROGRESS_SLIDER_SPEED  4
#define _PROGRESS_SLIDER_SLEEP  20


void progress_enable_slider_mode( nsboolean yes_or_no )
	{  ____progress_slider_mode = yes_or_no;  }


void _PaintProgressBar( HWND hBar, nsfloat percent )
   {
   HDC hDC;
   RECT  progress;
   //RECT     empty;
   RECT     client;


	hDC = GetDC( hBar );

   GetClientRect( hBar, &client );

   progress = client;
   //empty    = client;

	if( ____progress_slider_mode )
		{
		progress.left  = ____progress_slider_left;
		progress.right = progress.left + _PROGRESS_SLIDER_WIDTH;
		}
	else
		{
		progress.right = ( LONG )(
				( nslong )progress.left + 
									( nslong )( ( client.right - client.left ) * ( percent / 100 ) + .5f )
				);

		//empty.left = progress.right;
		}

   //if( 0 < ( empty.right - empty.left ) )
     // FillRect( hDC, &empty, ( HBRUSH )( COLOR_3DFACE + 1 ) );

	FillRect( hDC, &client, ( HBRUSH )( COLOR_3DFACE + 1 ) );

   if( progress.right - progress.left >= 4 )
		{
		DrawEdge( hDC, &progress, EDGE_RAISED, BF_RECT );

		progress.left   += 3;
		progress.right  -= 3;
		progress.top    += 3;
		progress.bottom -= 3;

		if( progress.right > progress.left && progress.bottom > progress.top )
			FillRect( hDC, &progress, ( HBRUSH )( COLOR_ACTIVECAPTION + 1 ) );
		}

	ReleaseDC( hBar, hDC );
   }



void SlideProgressBar( void *args )
	{
	HWND bar;
	RECT rc;


	bar = GetDlgItem( s_Progress.hWnd, IDS_PROGRESS_BAR );
	GetClientRect( bar, &rc );

	while( ____progress_slider_running )
		{
		_PaintProgressBar( bar, 0.0f );

		if( ____progress_slider_direction )
			____progress_slider_left -= _PROGRESS_SLIDER_SPEED;
		else
			____progress_slider_left += _PROGRESS_SLIDER_SPEED;

		if( ____progress_slider_left < 0 )
			{
			____progress_slider_left = 0;
			____progress_slider_direction = ! ____progress_slider_direction;
			}
		else if( ____progress_slider_left + _PROGRESS_SLIDER_WIDTH > rc.right )
			{
			____progress_slider_left = rc.right - _PROGRESS_SLIDER_WIDTH;
			____progress_slider_direction = ! ____progress_slider_direction;
			}

		if( ____progress_slider_running )
			Sleep( _PROGRESS_SLIDER_SLEEP );
		}

	_endthread();
	}



INT_PTR CALLBACK _ProgressDialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_PAINT:
			//ShowWindow( hWnd, SW_HIDE );
			_PaintProgressBar( GetDlgItem( hWnd, IDS_PROGRESS_BAR ), s_Progress.percent );
         return FALSE;

      case WM_INITDIALOG:

         s_Progress.hWnd = hWnd;
			s_Progress.hBar = GetDlgItem( hWnd, IDS_PROGRESS_BAR );
			ns_assert( NULL != s_Progress.hBar );

         if( NULL != s_Progress.title )
            {
				//SetWindowText( hWnd, s_Progress.title );
				}

         //CenterWindow( hWnd, s_Progress.owner );

			//ns_println( "beginning progress thread..." );

         if( -1 == _beginthread( s_Progress.function, 0, &s_Progress ) )
            EndDialog( hWnd, -1 );

			if( ____progress_slider_mode )
				{
				____progress_slider_left    = 0;
				____progress_slider_running = NS_TRUE;

				____progress_slider_handle = ( HANDLE )_beginthread( SlideProgressBar, 0, NULL );
				}

         return FALSE;

      case WM_COMMAND:
         if( IDCANCEL == LOWORD( wParam ) )
            s_Progress.wasCancelled = 1;
         return TRUE;
      }

   return FALSE;
   }


nsboolean ____progress_dialog_running;

#include "statusbar.h"

eERROR_TYPE ProgressDialog( HWND owner,
                            const nschar *title,
                            void ( *function )( void* ),
                            void *inArgs,
                            void *outArgs,
                            nsint *wasCancelled
                            )
   {
   nsint retValue;

	if( ____progress_dialog_running )
		return eNO_ERROR;

	____progress_dialog_running = NS_TRUE;

   ns_assert( NULL != function );
   ns_assert( NULL != wasCancelled );

//ns_println( "beginning progress..." );

   s_Progress.owner        = owner;
   s_Progress.title        = title;
   s_Progress.wasCancelled = 0;
   s_Progress.function     = function;
   s_Progress.inArgs       = inArgs;
   s_Progress.outArgs      = outArgs;
   s_Progress.percent      = 0;
	s_Progress.error        = eNO_ERROR;

   *wasCancelled = 0;

	if( NULL != owner )
		{
		retValue = ( nsint )DialogBox( g_Instance, "PROGRESS_DIALOG", owner, _ProgressDialogProc );
		//ns_println( "end progress..." );
		//if( -1 == retValue )
		//	return ( g_Error = eERROR_OPER_SYSTEM );
		}
	else
		{
		( s_Progress.function )( &s_Progress );
		retValue = s_Progress.error;
		}

   *wasCancelled = s_Progress.wasCancelled;

SetStatusBarText( "", STATUS_BAR_MSG );

	____progress_dialog_running = NS_FALSE;
   return ( eERROR_TYPE )retValue;
   }


nsint ProgressDialogWasCancelled( void *dialog )
   {
   ns_assert( NULL != dialog );
   return ( ( ProgressDlg* )dialog )->wasCancelled;
   }


void EndProgressDialog( void *dialog, const eERROR_TYPE error )
   {
   ns_assert( NULL != dialog );

	/* If in slider mode, join the thread (i.e. wait for it to terminate) before
		terminating the progress dialog. */
	if( ____progress_slider_mode )
		{
		____progress_slider_running = NS_FALSE;
		WaitForSingleObject( ____progress_slider_handle, INFINITE );
		}
	
	(( ProgressDlg*)dialog )->error = error;
   EndDialog( (( ProgressDlg*)dialog )->hWnd, error );
   }


void GetProgressDialogArgs( void *dialog, void** inArgs, void** outArgs )
   {
   ns_assert( NULL != dialog );
   ns_assert( NULL != inArgs );
   ns_assert( NULL != outArgs );

   *inArgs  = (( ProgressDlg*)dialog )->inArgs;
   *outArgs = (( ProgressDlg*)dialog )->outArgs;
   }


void UpdateProgressDialog( void *dialog, const nsfloat percentDone )
   {
   nsfloat clippedPercent;

   ns_assert( NULL != dialog );

   clippedPercent = percentDone;

   if( clippedPercent < 0.0f )
      clippedPercent = 0.0f;
   else if( 100.0f < clippedPercent )
      clippedPercent = 100.0f;

   ((ProgressDlg*)dialog)->percent = clippedPercent;

   _PaintProgressBar( GetDlgItem( ((ProgressDlg*)dialog)->hWnd, IDS_PROGRESS_BAR ),
                      clippedPercent );

	//InvalidateRect( (( ProgressDlg*)dialog )->hBar, NULL, FALSE );
   }


void SetProgressDialogTitle( void *dialog, const nschar* title )
   {
	SetStatusBarText( title, STATUS_BAR_MSG );
   //ns_assert( NULL != dialog );
//SetDlgItemText( ((ProgressDlg*)dialog)->hWnd, IDS_PROGRESS_MSG, title );
   //SetWindowText( ((ProgressDlg*)dialog)->hWnd, title );
   }

void SetProgressDialogMessage( void *dialog, const nschar* message )
   {
	SetStatusBarText( message, STATUS_BAR_MSG );
   //ns_assert( NULL != dialog );
   //SetDlgItemText( ((ProgressDlg*)dialog)->hWnd, IDS_PROGRESS_MSG, message );
   }


void CenterWindow( HWND hChild, HWND hParent )
   {
   RECT  parent;
   RECT  child;
   nslong  width;
   nslong  x;
   nslong  height;
   nslong  y;


   GetWindowRect( hChild, &child );
   GetWindowRect( hParent, &parent );

   width = ( child.right - child.left );

   x = parent.left +
       ( parent.right - parent.left ) / 2 - 
       ( child.right - child.left ) / 2;

   child.left  = ( LONG )x;
   child.right = ( LONG )( x + width );

   height = ( child.bottom - child.top );

   y = parent.top + 
       ( parent.bottom - parent.top ) / 2 - 
       ( child.bottom - child.top ) / 2;

   child.top    = ( LONG )y;
   child.bottom = ( LONG )( y + height );

    SetWindowPos( hChild, 
                   NULL, 
                  child.left,
                  child.top, 
                  0, 0, 
                  SWP_NOZORDER | SWP_NOSIZE
                );
   }





#define _MAX_INTEGER_DIGITS  64


nsint ParseVolumeFileListName
   ( 
   const nschar*   fileName,
   nschar               *prefix, 
   const nsuint          prefixChars,    /* Size of the prefix buffer. */
   nschar               *extension, 
   const nsuint          extensionChars  /* Size of the extension buffer. */
   )
   {
   nschar  number[ _MAX_INTEGER_DIGITS ];
   nsint   fileNameIndex;
   nsint   prefixIndex;
   nsint   extensionIndex;
   nsint   numberIndex;
   nsint   maxPrefixIndex;
   nsint   maxExtensionIndex;
   nsint   maxNumberIndex;


   fileNameIndex     = ( ( nsint )ns_ascii_strlen( fileName ) ) - 1;
   prefixIndex       = 0;
   extensionIndex    = 0;
   numberIndex       = 0;
  
   /* NOTE: Leave room for the '\0' character! */

   maxPrefixIndex    = ( ( nsint )prefixChars ) - 1;
   maxExtensionIndex = ( ( nsint )extensionChars ) - 1;
   maxNumberIndex    = ( ( nsint )_MAX_INTEGER_DIGITS ) - 1;

   /* Get optional extension characters. This routine currently assumes
      the extension is any trailing non-digit characters. */

   while( ( 0 <= fileNameIndex ) && 
          ( extensionIndex < maxExtensionIndex ) &&
          ( ! isdigit( fileName[ fileNameIndex ] ) ) 
        )
      extension[ extensionIndex++ ] = fileName[ fileNameIndex-- ];
      
   /* Get the number part of the file name. */

   while( ( 0 <= fileNameIndex ) &&
          ( numberIndex < maxNumberIndex ) &&
          ( isdigit( fileName[ fileNameIndex ] ) )
        )
      number[ numberIndex++ ] = fileName[ fileNameIndex-- ];
      
   /* All remaining characters are assumed to be prefix characters. */

   while( ( 0 <= fileNameIndex ) &&
          ( prefixIndex < maxPrefixIndex )
        )
      prefix[ prefixIndex++ ] = fileName[ fileNameIndex-- ];
      
   prefix[ prefixIndex ]       = '\0';
   extension[ extensionIndex ] = '\0';
   number[ numberIndex ]       = '\0';

   /* Have to reverse the strings since the file name was 
      traversed backwards. */
      
   ns_ascii_strrev( prefix );
   ns_ascii_strrev( extension );
   ns_ascii_strrev( number );

   return ( 0 < ns_ascii_strlen( number ) ) ? atoi( number ) : -1;

   }/* ParseVolumeFileListName() */


const nschar* FindFileNameInPath( const nschar* path, const nschar dirMark )
   {
   const nschar *ptr = path + ( ns_ascii_strlen( path ) - 1 );

   while( path <= ptr )
      {
      if( dirMark == *ptr )
         break;

      --ptr;
      }

   return ( ++ptr );

   }/* FindFileNameInPath() */


void CenterRectangleHoriz( RECT* src, const RECT* dest )
   {
   nslong  width;
   nslong  x;

   width = src->right - src->left;

   x = dest->left +
       ( dest->right - dest->left ) / 2 - 
       ( src->right - src->left ) / 2;

   src->left  = ( LONG )x;
   src->right = ( LONG )( x + width );

   }/* CenterRectangleHoriz() */


void CenterRectangleVert( RECT* src, const RECT* dest )
   {
   nslong  height;
   nslong  y;

   height = ( src->bottom - src->top );

   y = dest->top + 
       ( dest->bottom - dest->top ) / 2 - 
       ( src->bottom - src->top ) / 2;

   src->top    = ( LONG )y;
   src->bottom = ( LONG )( y + height );

   }/* CenterRectangleVert() */



void _CenterRectangle( RECT* src, const RECT* dest )
   {
   nslong  width;
   nslong  x;
   nslong  height;
   nslong  y;


   width = src->right - src->left;

   x = dest->left +
       ( dest->right - dest->left ) / 2 - 
       ( src->right - src->left ) / 2;

   src->left  = ( LONG )x;
   src->right = ( LONG )( x + width );

   height = ( src->bottom - src->top );

   y = dest->top + 
       ( dest->bottom - dest->top ) / 2 - 
       ( src->bottom - src->top ) / 2;

   src->top    = ( LONG )y;
   src->bottom = ( LONG )( y + height );
   }


nsfloat ScaleAndCenterRectangle( RECT* src, const RECT* dest )
   {
   nsfloat aspect;
   nsuint dwidth;
   nsuint dheight;
   nsuint swidth;
   nsuint sheight;


   swidth = src->right - src->left;
   sheight = src->bottom - src->top;
  
   dwidth = dest->right - dest->left;
   dheight = dest->bottom - dest->top;


   /* The aspect should be picked by the smaller of the two dimensions
      of the destination rectangle. */

   if( dheight < dwidth )
      aspect = ( ( nsfloat )dheight ) / ( ( nsfloat )sheight );
   else
      aspect = ( ( nsfloat )dwidth ) / ( ( nsfloat )swidth );

   ScaleRectangle( src, aspect );
   _CenterRectangle( src, dest );

   return aspect;
   }


void ScaleRectangle( RECT* rect, nsfloat scalar )
   {
   rect->left   = ( LONG )( nslong )( rect->left * scalar );
   rect->top    = ( LONG )( nslong )( rect->top * scalar );
   rect->right  = ( LONG )( nslong )( rect->right * scalar );
   rect->bottom = ( LONG )( nslong )( rect->bottom * scalar );
   }


void TranslateRectangle( RECT* rect, const nslong amount, const nsint vertically )
   {
   if( vertically )
      {
      rect->top    += ( LONG )amount;
      rect->bottom += ( LONG )amount;
      }
   else
      {
      rect->left  += ( LONG )amount;
      rect->right += ( LONG )amount;
      }
   }



typedef struct tagMENUELEMSTATE
   {
   nsint id;
   nsint enabled;
   } 
   MENUELEMSTATE;

typedef struct tagMENUEVENT
   {
   MENUELEMSTATE  *elemStates;
   nsint             numElems;
   void           ( *hook )( void );
   }
   MENUEVENT;


static MENUELEMSTATE s_OnFileOpen[] =
   {
   { IDM_FILE_OPEN,  0 },
   { IDM_FILE_CLOSE, 1 }
   };


static MENUELEMSTATE s_OnFileClose[] =
   {
   { IDM_FILE_OPEN,  1 },
   { IDM_FILE_CLOSE, 0 }
   };


static MENUEVENT s_MenuEvents[ mNUM_MENU_EVENTS ] = 
   {
   { s_OnFileOpen, sizeof( s_OnFileOpen )/sizeof( MENUELEMSTATE ), NULL },
   { s_OnFileClose, sizeof( s_OnFileClose )/sizeof( MENUELEMSTATE ), NULL }
   };


void OnMenuEvent( const eMENU_EVENT type, HWND hOwner )
   {
   MENUEVENT  event;
   nsint        k;
 

   event = s_MenuEvents[ type ];

   for( k = 0; k < event.numElems; ++k )
      SetMenuItemEnabled( hOwner, event.elemStates[ k ].id, event.elemStates[ k ].enabled );

   if( NULL != event.hook )
      ( event.hook )();
   }


nsboolean GetMenuItemEnabled( HWND hWnd, int id )
	{
   HMENU         hMenu;
   MENUITEMINFO  info;


   hMenu = GetMenu( hWnd );

   info.cbSize = sizeof( MENUITEMINFO );
   info.fMask  = MIIM_STATE;

   GetMenuItemInfo( hMenu, id, FALSE, &info );

   return ( info.fState & MFS_ENABLED ) ? 1 : 0;
	}


void SetMenuItemEnabled( HWND hWnd, nsint id, nsint enabled )
   {
   HMENU         hMenu;
   MENUITEMINFO  info;


   hMenu = GetMenu( hWnd );

   info.cbSize = sizeof( MENUITEMINFO );
   info.fMask  = MIIM_STATE;

   GetMenuItemInfo( hMenu, id, FALSE, &info );

   if( enabled )
      {
      info.fState &= ~MFS_DISABLED;
      info.fState |= MFS_ENABLED;
      }
   else
      {
      info.fState &= ~MFS_ENABLED;
      info.fState |= MFS_DISABLED;
      }

   SetMenuItemInfo( hMenu, id, FALSE, &info );

   }/* SetMenuItemEnabled() */


void SetMenuItemChecked( HWND hWnd, nsint id, nsint checked )
   {
   HMENU         hMenu;
   MENUITEMINFO  info;


   hMenu = GetMenu( hWnd );

   info.cbSize = sizeof( MENUITEMINFO );
   info.fMask  = MIIM_STATE;

   GetMenuItemInfo( hMenu, id, FALSE, &info );

   if( checked )
      {
      info.fState &= ~MFS_UNCHECKED;
      info.fState |= MFS_CHECKED;
      }
   else
      {
      info.fState &= ~MFS_CHECKED;
      info.fState |= MFS_UNCHECKED;
      }

   SetMenuItemInfo( hMenu, id, FALSE, &info );
   }


nsint IsMenuItemChecked( HWND hWnd, nsint id )
   {
   HMENU         hMenu;
   MENUITEMINFO  info;


   hMenu = GetMenu( hWnd );

   info.cbSize = sizeof( MENUITEMINFO );
   info.fMask  = MIIM_STATE;

   GetMenuItemInfo( hMenu, id, FALSE, &info );

   return ( info.fState & MFS_CHECKED ) ? 1 : 0;
   }



void _DoEnableMenuItems( HMENU hMenu, nsint enabled )
   {
   nsint           numItems;
   nsint           i;
   MENUITEMINFO  info;
   HMENU         hSubMenu;


   info.cbSize = sizeof( MENUITEMINFO );
   numItems    = GetMenuItemCount( hMenu );
  
   for( i = 0; i < numItems; ++i )
      {
      info.fMask = MIIM_SUBMENU | MIIM_STATE;
      GetMenuItemInfo( hMenu, i, TRUE, &info );
      
      hSubMenu = info.hSubMenu;

      info.fMask = MIIM_STATE;

      if( enabled )
         {
         info.fState &= ~MFS_DISABLED;
         info.fState |= MFS_ENABLED;
         }
      else
         {
         info.fState &= ~MFS_ENABLED;
         info.fState |= MFS_DISABLED;
         }

      SetMenuItemInfo( hMenu, i, TRUE, &info );

      if( NULL != hSubMenu )
         _DoEnableMenuItems( hSubMenu, enabled );
      }
   }


void EnableMenuItems( HWND hWnd, nsint enabled )
   {
   HMENU         hMenu;
   nsint           numItems;
   nsint           i;
   MENUITEMINFO  info;


   hMenu = GetMenu( hWnd );

   info.cbSize = sizeof( MENUITEMINFO );
   info.fMask  = MIIM_SUBMENU;

   numItems = GetMenuItemCount( hMenu );
  
   for( i = 0; i < numItems; ++i )
      {
      GetMenuItemInfo( hMenu, i, TRUE, &info );
     
      if( NULL != info.hSubMenu )
         _DoEnableMenuItems( info.hSubMenu, enabled );
      }
   }


void _DoSetMenuItemsChecked( HMENU hMenu, nsint checked )
   {
   nsint           numItems;
   nsint           i;
   MENUITEMINFO  info;
   HMENU         hSubMenu;


   info.cbSize = sizeof( MENUITEMINFO );
   numItems    = GetMenuItemCount( hMenu );
  
   for( i = 0; i < numItems; ++i )
      {
      info.fMask = MIIM_SUBMENU | MIIM_STATE;
      GetMenuItemInfo( hMenu, i, TRUE, &info );
      
      hSubMenu = info.hSubMenu;

      info.fMask = MIIM_STATE;

      if( checked )
         {
         info.fState &= ~MFS_UNCHECKED;
         info.fState |= MFS_CHECKED;
         }
      else
         {
         info.fState &= ~MFS_CHECKED;
         info.fState |= MFS_UNCHECKED;
         }

      SetMenuItemInfo( hMenu, i, TRUE, &info );

      if( NULL != hSubMenu )
         _DoSetMenuItemsChecked( hSubMenu, checked );
      }
   }


void SetMenuItemsChecked( HWND hWnd, nsint checked )
   {
   HMENU         hMenu;
   nsint           numItems;
   nsint           i;
   MENUITEMINFO  info;


   hMenu = GetMenu( hWnd );

   info.cbSize = sizeof( MENUITEMINFO );
   info.fMask  = MIIM_SUBMENU;

   numItems = GetMenuItemCount( hMenu );
  
   for( i = 0; i < numItems; ++i )
      {
      GetMenuItemInfo( hMenu, i, TRUE, &info );
     
      if( NULL != info.hSubMenu )
         _DoSetMenuItemsChecked( info.hSubMenu, checked );
      }
   }


static const nschar* s_NeuronStudioLicense =
	"This software is provided \"as is\" and without "
	"warranties or representations, either expressed or "
	"implied, as to performance, merchantability, or "
	"fitness for any particular purpose, and the user "
	"assumes all risks when using it. You may not decompile, "
	"disassemble, reverse engineer, or modify the software. "
	"This software cannot be sold, redistributed without prior "
	"written permission, or incorporated into other products "
	"(commercial or otherwise).";


static HWND s_AboutNeuronStudioDialogOwner = NULL;


#include <ext/lock.h>
//extern nschar ____license[];
extern nschar ____executable_path[];

INT_PTR CALLBACK _AboutNeuronStudioDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
   {
   switch( uMsg )
      {
      case WM_INITDIALOG:
			{
			//nsint days_left;
			nschar buffer[ 256 ];

         CenterWindow( hDlg, s_AboutNeuronStudioDialogOwner );

			SendMessage(
				GetDlgItem( hDlg, IDE_LICENSE ),
				EM_SETMARGINS,
				( WPARAM )( EC_LEFTMARGIN | EC_RIGHTMARGIN ),
				( LPARAM )MAKELONG( 2, 2 )
				);

         SetDlgItemText( hDlg, IDE_LICENSE, s_NeuronStudioLicense );
			SetDlgItemText( hDlg, IDS_ABOUT_DATE, __DATE__ );

			ns_snprint(
				buffer,
				NS_ARRAY_LENGTH( buffer ),
				NS_FMT_UINT
				"."
				NS_FMT_UINT
				"."
				NS_MAKE_FMT( NS_FMT_FLAG_ZERO_FILL "2", NS_FMT_TYPE_UINT )
				" "
				NS_FMT_UINT
				"-bit",
				NS_VERSION_MAJOR,
				NS_VERSION_MINOR,
				NS_VERSION_MICRO,
				NS_POINTER_BITS
				);

			SetDlgItemText( hDlg, IDS_ABOUT_VERSION, buffer );

			//days_left = GetLockDays( ____license );
			//ns_snprint( buffer, NS_ARRAY_LENGTH( buffer ), NS_FMT_INT " days remaining", days_left );
			//SetDlgItemText( hDlg, IDS_LICENSE_NUM_DAYS, buffer );

			SetDlgItemText( hDlg, IDE_ABOUT_PATH, ____executable_path );
			SetFocus( GetDlgItem( hDlg, IDE_ABOUT_PATH ) );
			SendMessage( GetDlgItem( hDlg, IDE_ABOUT_PATH ), EM_SETSEL, -1, 0 );
			}
         return FALSE;

      case WM_COMMAND:
         switch( LOWORD( wParam ) )
            {
            case IDE_LICENSE:
               SetFocus( GetDlgItem( hDlg, IDOK ) );
               break;

            case IDOK:
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


void AboutNeuronStudioDialog( HWND hOwner )
   {
   s_AboutNeuronStudioDialogOwner = hOwner;
   DialogBox( g_Instance, "ABOUT_DIALOG", hOwner, _AboutNeuronStudioDlgProc );
   }


void drag_rect_renderer_init( DragRectRenderer *state, HWND wnd )
   {
   state->wnd         = wnd;
   state->dc          = GetDC( state->wnd );
   state->pen         = CreatePen( PS_DOT, 1, 0xFFFFFF );
   state->old_rop     = SetROP2( state->dc, R2_XORPEN );
   state->old_bk_mode = SetBkMode( state->dc, TRANSPARENT );
   state->old_pen     = SelectObject( state->dc, state->pen );
   }

void drag_rect_renderer_finalize( DragRectRenderer *state )
   {
   SetROP2( state->dc, state->old_rop );
   SetBkMode( state->dc, state->old_bk_mode );
   SelectObject( state->dc, state->old_pen );
   DeleteObject( state->pen );
   ReleaseDC( state->wnd, state->dc );
   }

void drag_rect_renderer_line( DragRectRenderer *state, nsint x1, nsint y1, nsint x2, nsint y2 )
   {
   MoveToEx( state->dc, x1, y1, NULL );
   LineTo( state->dc, x2, y2 );
   }