#include "nsprogress.h"


NsProgress ns_progress_null = { NULL, NULL, NULL, NULL };


NS_IMPEXP NsProgress* ns_progress
	(
	NsProgress  *progress,
	nsboolean   ( *cancelled )( NsProgress *progress ),
	void        ( *update )( NsProgress *progress, nsfloat percent ),
	void        ( *set_title )( NsProgress *progress, const nschar *title ),
	void        ( *set_description )( NsProgress *progress, const nschar *description ),
	nspointer   user_data
	)
	{
	ns_assert( NULL != progress );

	progress->cancelled       = cancelled;
	progress->update          = update;
	progress->set_title       = set_title;
	progress->set_description = set_description;
	progress->user_data       = user_data;

	return progress;
	}


nsboolean ns_progress_cancelled( const NsProgress *progress )
	{
	return ( NULL != progress && NULL != progress->cancelled ) ?
			 ( progress->cancelled )( progress->user_data ) : NS_FALSE;
	}


void ns_progress_begin( NsProgress *progress )
	{
	if( NULL != progress )
		{
		progress->last_percent = 0.0f;
		progress->curr_percent = 0.0f;
		progress->curr_iter    = 0.0f;

		ns_progress_update( progress, NS_PROGRESS_BEGIN );
		}
	}


void ns_progress_end( NsProgress *progress )
	{
	if( NULL != progress )
		ns_progress_update( progress, NS_PROGRESS_END );
	}


void ns_progress_update( NsProgress *progress, nsfloat percent )
	{
	if( NULL != progress && NULL != progress->update )
		( progress->update )( progress->user_data, percent );
	}


void ns_progress_num_iters( NsProgress *progress, nssize num_iters )
	{
	if( NULL != progress )
		progress->num_iters = ( nsfloat )num_iters;
	}


void ns_progress_next_iter( NsProgress *progress )
	{
	if( NULL != progress )
		{
		progress->curr_iter += 1.0f;

		progress->curr_percent = 100.0f * progress->curr_iter / progress->num_iters;
		progress->curr_percent = ns_floorf( progress->curr_percent );

		if( progress->last_percent < progress->curr_percent )
			{
			progress->last_percent = progress->curr_percent;
			ns_progress_update( progress, progress->curr_percent );
			}
		}
	}


void ns_progress_set_title( NsProgress *progress, const nschar *title )
	{
ns_println( "%s", title );
	if( NULL != progress && NULL != progress->set_title )
		( progress->set_title )( progress->user_data, title );
	}


void ns_progress_set_description( NsProgress *progress, const nschar *description )
	{
	if( NULL != progress && NULL != progress->set_description )
		( progress->set_description )( progress->user_data, description );
	}


nspointer ns_progress_get_user_data( const NsProgress *progress )
	{  return NULL != progress ? ( nspointer )( progress->user_data ) : NULL;  }


void ns_progress_set_user_data( NsProgress *progress, nspointer user_data )
	{
	if( NULL != progress )
		progress->user_data = user_data;
	}


NS_VALUE_GET_IMPLEMENT( ns_value_get_progress, NsProgress* );
NS_VALUE_SET_IMPLEMENT( ns_value_set_progress, NsProgress* );
NS_VALUE_ARG_IMPLEMENT( ns_value_arg_progress, NsProgress* );

NsError ns_value_register_progress( void )
	{
	return ns_value_register(
				NS_VALUE_PROGRESS,
				sizeof( NsProgress* ),
				NULL,
				NULL,
				ns_value_arg_progress
				);
	}
