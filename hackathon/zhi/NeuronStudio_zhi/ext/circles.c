#include "circles.h"


NS_PRIVATE NsVector3d   circle_up;
NS_PRIVATE NsVector3d   circle_right;
NS_PRIVATE GLboolean    circle_lighting;
NS_PRIVATE GLfloat      circle_line_width;
NS_PRIVATE NsVector3d  *circle_rights;
NS_PRIVATE NsVector3d  *circle_ups;
NS_PRIVATE nsint        circle_k;


void CirclesInit( void )
	{
	circle_rights = NULL;
	circle_ups    = NULL;
	}


void CirclesFinalize( void )
	{
	ns_free( circle_rights );
	ns_free( circle_ups );

	circle_rights = NULL;
	circle_ups    = NULL;
	}


#define CIRCLES_ALLOC  128


NS_PRIVATE void CirclesAlloc( void )
	{
	CirclesFinalize();
	CirclesInit();

	circle_rights = ns_new_array( NsVector3d, CIRCLES_ALLOC );
	circle_ups    = ns_new_array( NsVector3d, CIRCLES_ALLOC );
	}


void CirclesBegin( nsint complexity, nssize line_width )
	{
	nsfloat   modelview[16];
   nsdouble  theta, delta;
   nsint     i;


	glGetBooleanv( GL_LIGHTING, &circle_lighting );
	glDisable( GL_LIGHTING );

	glGetFloatv( GL_LINE_WIDTH, &circle_line_width );
	glLineWidth( ( GLfloat )line_width );

	glGetFloatv( GL_MODELVIEW_MATRIX, modelview );

	circle_right.x = modelview[0];
	circle_right.y = modelview[4];
	circle_right.z = modelview[8];
	circle_up.x    = modelview[1];
	circle_up.y    = modelview[5];
	circle_up.z    = modelview[9];

	if( NULL == circle_rights || NULL == circle_ups )
		CirclesAlloc();
	
	if( NULL != circle_rights && NULL != circle_ups )
		{
		delta = NS_PI_D / 2.0 / ( nsdouble )complexity;

		for( circle_k = 0, i = 0, theta = 0.0; i < 4 * complexity; circle_k += 2, i++, theta += delta )
			{
			ns_assert( circle_k + 1 < CIRCLES_ALLOC );

			ns_vector3d_scale( circle_rights + circle_k, &circle_right, ns_cos( theta ) );
			ns_vector3d_scale( circle_ups + circle_k, &circle_up, ns_sin( theta ) ); 

			ns_vector3d_scale( circle_rights + circle_k + 1, &circle_right, ns_cos( theta + delta ) );
			ns_vector3d_scale( circle_ups + circle_k + 1, &circle_up, ns_sin( theta + delta ) );
			}
		}
	}


void CirclesEnd( void )
	{
	if( circle_lighting )
		glEnable( GL_LIGHTING );

	glLineWidth( circle_line_width );
	}


void CircleRender( const NsVector3d *P, nsfloat radius, const nsuchar *rgba )
	{
	nsdouble    *v;
	NsVector3d   V, R, U, T;
   nsint        i;


	if( NULL == circle_rights || NULL == circle_ups )
		return;

	v = ns_vector3d_array( &V );

	glBegin( GL_LINES );
	glColor4ubv( rgba );

	for( i = 0; i < circle_k; ++i )
		{
		ns_vector3d_add(
			&V,
			P,
			ns_vector3d_add(
				&T,
				ns_vector3d_scale( &R, circle_rights + i, radius ),
				ns_vector3d_scale( &U, circle_ups + i, radius )
				)
			);

		glVertex3dv( v );
		}

	glEnd();
	}
