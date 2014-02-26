#include "progress.h"
#include "app_lib.h"


void *g_Progress = NULL;



int IsProgressCancelled( void )
   {  return ProgressDialogWasCancelled( g_Progress );  }


void SetProgressTitle( const char *title )
   {  SetProgressDialogTitle( g_Progress, title );  }


void SetProgressMessage( const char *message )
   {  SetProgressDialogMessage( g_Progress, message );  }


void SetProgressPercent( float percent )
   {  UpdateProgressDialog( g_Progress, percent );  }







int progress_was_cancelled( const Progress *progress )
	{  return IsProgressCancelled();  }


const char* progress_get_title( const Progress *progress )
	{  return progress->title;  }


void progress_set_title( Progress *progress, const char *title )
	{
	progress->title = title;
	SetProgressTitle( title );
	}


const char* progress_get_text( const Progress *progress )
	{  return progress->text;  }


void progress_set_text( Progress *progress, const char *text )
	{
	progress->text = text;
	SetProgressMessage( text ); 
	}


float progress_get_percent_complete( const Progress *progress )
	{  return progress->percent;  }


void progress_set_percent_complete( Progress *progress, float percent )
	{
	progress->percent = percent;
	SetProgressPercent( percent );
	}


void progress_next_step( Progress *progress, unsigned step_num )
	{
	
	}
