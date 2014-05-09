#ifndef __PROGRESS_H__
#define __PROGRESS_H__


/* <public> */

/* Returns 1 if cancelled, else 0. */
int IsProgressCancelled( void );


void SetProgressTitle( const char *title );

void SetProgressMessage( const char *message );


/* Percentage is clipped to the range [ 0, 100 ] */
void SetProgressPercent( float percent );



/* <private> */
extern void* g_Progress;




typedef struct _Progress
	{
	const char  *title;
	const char  *text;
	float        percent;
	}
	Progress;


int progress_was_cancelled( const Progress *progress );

const char* progress_get_title( const Progress *progress );
void progress_set_title( Progress *progress, const char *title );

const char* progress_get_text( const Progress *progress );
void progress_set_text( Progress *progress, const char *text );

float progress_get_percent_complete( const Progress *progress );
void progress_set_percent_complete( Progress *progress, float percent );

void progress_next_step( Progress *progress, unsigned step_num );


#endif/* __PROGRESS_H__ */
