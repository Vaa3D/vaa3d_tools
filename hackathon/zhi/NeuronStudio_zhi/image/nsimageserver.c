#include "nsimageserver.h"


/*
G_PRIVATE_FUNC gboolean _g_image_reader_next( GImageReader *image_reader, GError **error )
   {
   const gchar     *file_name;
   GImageFileType   file_type;
   GImageInfo       image_info;


   g_assert( NULL != image_reader );
   g_assert( image_reader->current_file != image_reader->end_files );

   file_name = ( const gchar* )g_list_data( image_reader->current_file );
   file_type = g_image_file_type_by_extension( file_name );

   /* Save the old info first! */
/*
   image_info = image_reader->image_2d->image_info;

   g_object_unref( image_reader->image_2d );
   image_reader->image_2d = g_image_new();

   if( image_reader->length_3d <= image_reader->index_2d )
      {
		if( ! g_image_info_from_file( image_reader->image_2d,
												file_name,
												file_type,
												&image_reader->image_info,
												error
											 ) )
			return FALSE;

      image_reader->length_3d = image_reader->image_2d->image_info.length;
      image_reader->index_2d  = 0;
      }
   else
      image_reader->image_2d->image_info = image_info;

	if( image_reader->enabled )
		if( ! g_image_read_from_file_at( image_reader->image_2d,
													file_name,
													file_type,
													&image_reader->image_info,
													image_reader->index_2d,
													error
												 ) )
			return FALSE;

   return TRUE;
   }


G_PRIVATE_FUNC void _g_image_reader_finalize( GObject *image_reader )
   {
   g_assert( NULL != image_reader );
   g_object_unref( G_IMAGE_READER( image_reader )->image_2d );
   ( G_IMAGE_READER_GET_CLASS( image_reader )->parent_finalize )( image_reader );
   }


G_PRIVATE_FUNC void _g_image_reader_class_init( gpointer klass, gpointer data )
   {
   g_assert( NULL != klass );

   G_IMAGE_READER_CLASS( klass )->parent_finalize = G_OBJECT_CLASS( klass )->finalize;
   G_OBJECT_CLASS( klass )->finalize            = _g_image_reader_finalize;
   }


G_PRIVATE_FUNC void _g_image_reader_init( GTypeInstance *instance, gpointer klass )
   {
   g_assert( NULL != instance );
   G_IMAGE_READER( instance )->image_2d = g_image_new();
   }


/**//*
static GImageReaderClass _g_image_reader_class;

GType g_image_reader_get_type( void )
   {
   static GType _g_image_reader_type = 0;

   if( ! _g_image_reader_type )
      {
      /**//*_g_object_class_init( &_g_image_reader_class, NULL );
      /**//*_g_image_reader_class_init( &_g_image_reader_class, NULL );
      /**//*_g_image_reader_type = sizeof( GImageReader );
      }

   return _g_image_reader_type;
   }


GImageReader* g_image_reader_new( void )
   {
   /**//*GObject *image_reader = g_object_new( G_TYPE_IMAGE_READER, ( const gchar* )&_g_image_reader_class );
   /**//*_g_image_reader_init( ( GTypeInstance* )image_reader, NULL );
   return G_IMAGE_READER( image_reader );
   }


gboolean g_image_reader_create
   (
   GImageReader      *image_reader,
   GList             *first_file,
   GList             *end_files,
   const GImageInfo  *image_info,
	gboolean           enabled,
   GError           **error
   )
   {
   g_assert( NULL != image_reader );
   g_assert( NULL != first_file );
   g_assert( NULL != end_files );

   image_reader->current_file = first_file;
   image_reader->end_files    = end_files;
	image_reader->enabled      = enabled;

   if( NULL != image_info )
      image_reader->image_info = *image_info;

   return ( g_image_reader_has_more( image_reader ) ) ?
          _g_image_reader_next( image_reader, error ) : TRUE;
   }


gboolean g_image_reader_enable( GImageReader *image_reader, GError **error )
	{
	g_assert( NULL != image_reader );

	image_reader->enabled = G_TRUE;

   return ( g_image_reader_has_more( image_reader ) ) ?
          _g_image_reader_next( image_reader, error ) : TRUE;
	}


void g_image_reader_disable( GImageReader *image_reader )
	{
	g_assert( NULL != image_reader );
	image_reader->enabled = G_FALSE;
	}


gboolean g_image_reader_has_more( const GImageReader *image_reader )
   {
   g_assert( NULL != image_reader );
   return ( image_reader->current_file != image_reader->end_files ) ? TRUE : FALSE;
   }


gboolean g_image_reader_advance( GImageReader *image_reader, GError **error )
   {
   g_assert( NULL != image_reader );

   ++image_reader->index_2d;

   if( image_reader->index_2d == image_reader->length_3d )
      image_reader->current_file = g_list_next( image_reader->current_file );

   return ( g_image_reader_has_more( image_reader ) ) ?
          _g_image_reader_next( image_reader, error ) : TRUE;
   }


GImage* g_image_reader_read( GImageReader *image_reader )
   {
   g_assert( NULL != image_reader );
   return image_reader->image_2d;
   }
*/
