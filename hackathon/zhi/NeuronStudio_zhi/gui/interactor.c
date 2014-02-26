#include "interactor.h"


void InitInteractor( Interactor *interactor )
   {
	interactor->imode        = INTERACTOR_MODE_ROTATE;
	interactor->idtouch      = 0.01f;
	interactor->iatouch      = 0.1f;
	interactor->ixpos        = 0.0f;
	interactor->iypos        = 0.0f;
	interactor->izpos        = 0.0f;
	interactor->iorientation = 0.0f;
	interactor->itilt        = 0.0f;
	interactor->iprevx       = 0;
	interactor->iprevy       = 0;
	interactor->interacting  = 0;

	interactor->imatrix[0]   = 1.0f;
	interactor->imatrix[1]   = 0.0f;
	interactor->imatrix[2]   = 0.0f;
	interactor->imatrix[3]   = 0.0f;
	interactor->imatrix[4]   = 0.0f;
	interactor->imatrix[5]   = 1.0f;
	interactor->imatrix[6]   = 0.0f;
	interactor->imatrix[7]   = 0.0f;
	interactor->imatrix[8]   = 0.0f;
	interactor->imatrix[9]   = 0.0f;
	interactor->imatrix[10]   = 1.0f;
	interactor->imatrix[11]   = 0.0f;
	interactor->imatrix[12]   = 0.0f;
	interactor->imatrix[13]   = 0.0f;
	interactor->imatrix[14]   = 0.0f;
	interactor->imatrix[15]   = 1.0f;

	interactor->iwinxdim = 0;
	interactor->iwinydim = 0;
   }


void SetInteractorTranslationalTouch( Interactor *interactor, nsfloat touch )
	{  interactor->idtouch = touch;  }


void SetInteractorRotationalTouch( Interactor *interactor, nsfloat touch )
	{  interactor->iatouch = touch;  }


nsint GetInteractorMode( Interactor *interactor )
	{  return interactor->imode;  }


void SetInteractorMode( Interactor *interactor, nsint mode )
	{  interactor->imode = mode;  }


void SetInteractorWindowSize( Interactor *interactor, nsint xdim, nsint ydim)
	{
	interactor->iwinxdim = xdim;
	interactor->iwinydim = ydim;
	}


void StartInteractor( Interactor *interactor, HWND hWnd, nsint x, nsint y )
   {
   interactor->iprevx      = x;
   interactor->iprevy      = y;
   interactor->interacting = 1;
   //interactor->imode       = INTERACTOR_MODE_ROTATE;
	interactor->ioldmode = interactor->imode;

   SetCapture( hWnd );
   }


nsint UpdateInteractor( Interactor *interactor, nsint x, nsint y )
   {
	nsint     deltax, deltay;
	nsfloat   tilt, orientation, roll;
	nsfloat   right[3], up[3], front[3];
	nsint     middlex, middley;
	nsdouble  alpha, beta;
 

	if( interactor->interacting ) /* prevents misuse */
		{
		deltax = x - interactor->iprevx;
		deltay = y - interactor->iprevy;

		switch( interactor->imode )
			{
			case INTERACTOR_MODE_ROTATE:
				tilt        = ( -interactor->iatouch * deltay );
				orientation = (  interactor->iatouch * deltax );

				glPushMatrix();
				glLoadMatrixf( interactor->imatrix );

				right[0] = interactor->imatrix[0];
				right[1] = interactor->imatrix[4];
				right[2] = interactor->imatrix[8];
				up[0]    = interactor->imatrix[1];
				up[1]    = interactor->imatrix[5];
				up[2]    = interactor->imatrix[9];

				glRotatef( tilt, right[0], right[1], right[2] );
				glRotatef( orientation, up[0], up[1], up[2] );
				glGetFloatv( GL_MODELVIEW_MATRIX, interactor->imatrix );
				glPopMatrix();

				break;

			case INTERACTOR_MODE_ROLL:
				middlex = interactor->iwinxdim / 2;
				middley = -interactor->iwinydim / 2;

				// compute angle of rotation based on movement around center of window
				if( interactor->iprevx == middlex || x == middlex )
					roll = 0.0f;
				else
					{
					alpha = ns_atan2( ( interactor->iprevy - middley ), ( interactor->iprevx - middlex ) );
					beta  = ns_atan2( ( y - middley ), ( x - middlex ) );
					roll = ( nsfloat )( ( alpha - beta ) * 180.0 / 3.141593 );
					}

				glPushMatrix();
				glLoadMatrixf( interactor->imatrix );

				front[0] = interactor->imatrix[2];
				front[1] = interactor->imatrix[6];
				front[2] = interactor->imatrix[10];

				glRotatef( -roll, front[0], front[1], front[2] );
				glGetFloatv( GL_MODELVIEW_MATRIX, interactor->imatrix );
				glPopMatrix();

				break;

			case INTERACTOR_MODE_MOVE:
				interactor->ixpos -= ( interactor->idtouch * deltax );
				interactor->iypos -= ( interactor->idtouch * deltay );
				break;

			case INTERACTOR_MODE_ZOOM:
				interactor->izpos += ( interactor->idtouch * deltay );
				break;
			}

		interactor->iprevx = x;
		interactor->iprevy = y;

		return 1;
		}

	return 0;
   }


void EndInteractor( Interactor *interactor )
   {
	interactor->interacting = 0;
	//interactor->imode       = 0;
	interactor->imode       = interactor->ioldmode;

	ReleaseCapture();
   }


void InteractorViewTransformation( Interactor *interactor )
   {
	/* translate to new position */
	glTranslatef( -interactor->ixpos, -interactor->iypos, -interactor->izpos );

	/* rotate on its axis */
	/*
	glRotatef( interactor->itilt, 1.0f, 0.0f, 0.0f );
	glRotatef( interactor->iorientation, 0.0f, 1.0f, 0.0f );
	*/
	glMultMatrixf( interactor->imatrix );
   }
