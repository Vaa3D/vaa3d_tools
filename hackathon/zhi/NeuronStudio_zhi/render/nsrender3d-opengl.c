#include "nsrender3d-opengl.h"


NS_PRIVATE void _ns_render3d_enable_anti_aliasing( NsRenderState *state )
	{
	GLboolean  line_smooth;
	GLboolean  blend_enabled;
	GLint      blend_src;
	GLint      blend_dest;


	glGetBooleanv( GL_LINE_SMOOTH, &line_smooth );
	glGetBooleanv( GL_BLEND, &blend_enabled );
	glGetIntegerv( GL_BLEND_SRC, &blend_src );
	glGetIntegerv( GL_BLEND_DST, &blend_dest );

	state->variables.blend_enabled = ( nsboolean )blend_enabled;
	state->variables.blend_src     = ( nsint )blend_src;
	state->variables.blend_dest    = ( nsint )blend_dest;
	state->variables.line_smooth   = ( nsboolean )line_smooth;

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_LINE_SMOOTH );
	}


NS_PRIVATE void _ns_render3d_restore_anti_aliasing( NsRenderState *state )
	{
	glBlendFunc( ( GLenum )state->variables.blend_src, ( GLenum )state->variables.blend_dest );

	if( ! state->variables.blend_enabled )
		glDisable( GL_BLEND );

	if( ! state->variables.line_smooth )
		glDisable( GL_LINE_SMOOTH );
	}


NS_PRIVATE void _ns_render3d_enable_wire_frame( NsRenderState *state )
	{
	GLint polygon_mode[2];

	glGetIntegerv( GL_POLYGON_MODE, polygon_mode );

	state->variables.polygon_mode_front = ( nsint )polygon_mode[0];
	state->variables.polygon_mode_back  = ( nsint )polygon_mode[1];

	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	}


NS_PRIVATE void _ns_render3d_restore_wire_frame( NsRenderState *state )
	{
	glPolygonMode( GL_FRONT, ( GLenum )state->variables.polygon_mode_front );
	glPolygonMode( GL_BACK, ( GLenum )state->variables.polygon_mode_back );
	}


NS_PRIVATE void _ns_render3d_disable_lighting( NsRenderState *state )
	{
	GLboolean lighting;

	glGetBooleanv( GL_LIGHTING, &lighting );
	state->variables.lighting = ( nsboolean )lighting;

	glDisable( GL_LIGHTING );
	}


NS_PRIVATE void _ns_render3d_enable_lighting( NsRenderState *state )
	{
	if( state->variables.lighting )
		glEnable( GL_LIGHTING );
	}


NS_PRIVATE void _ns_render3d_set_line_size( NsRenderState *state )
	{
	GLfloat line_size;

	glGetFloatv( GL_LINE_WIDTH, &line_size );
	state->variables.line_size = line_size;

	glLineWidth( ( GLfloat )state->constants.line_size );
	}


NS_PRIVATE void _ns_render3d_restore_line_size( NsRenderState *state )
	{  glLineWidth( ( GLfloat )state->variables.line_size );  }


NS_PRIVATE void _ns_render3d_set_point_size( NsRenderState *state )
	{
	GLfloat point_size;

	glGetFloatv( GL_POINT_SIZE, &point_size );
	state->variables.point_size = point_size;

	glPointSize( ( GLfloat )state->constants.point_size );
	}


NS_PRIVATE void _ns_render3d_restore_point_size( NsRenderState *state )
	{  glPointSize( ( GLfloat )state->variables.point_size );  }




void ns_render3d_pre_images( NsRenderState *state )
	{
	/*TODO*/
	NS_USE_VARIABLE( state );
	}


void ns_render3d_image
	(
	const NsVector3i  *V,
	const NsImage     *image_xy,
	const NsImage     *image_zy,
	const NsImage     *image_xz,
	NsRenderState     *state
	)
	{
	/*TODO*/
	NS_USE_VARIABLE( V );
	NS_USE_VARIABLE( image_xy );
	NS_USE_VARIABLE( image_zy );
	NS_USE_VARIABLE( image_xz );
	NS_USE_VARIABLE( state );
	}


void ns_render3d_post_images( NsRenderState *state )
	{
	/*TODO*/
	NS_USE_VARIABLE( state );
	}




void ns_render3d_pre_texts( NsRenderState *state )
	{
	/*TODO*/
	NS_USE_VARIABLE( state );
	}


void ns_render3d_text
	(
	const NsVector3f  *V,
	const nschar      *text,
	NsRenderState     *state
	)
	{
	/*TODO*/
	NS_USE_VARIABLE( V );
	NS_USE_VARIABLE( text );
	NS_USE_VARIABLE( state );
	}


void ns_render3d_post_texts( NsRenderState *state )
	{
	/*TODO*/
	NS_USE_VARIABLE( state );
	}




NS_PRIVATE GLuint _ns_render3d_cube_faces[ 24 ] =
   {
   0, 2, 3, 1,
   1, 3, 7, 5,
   5, 7, 6, 4,
   4, 6, 2, 0,
   4, 0, 1, 5,
   2, 6, 7, 3
   };

NS_PRIVATE GLfloat _ns_render3d_cube_vertices[ 8 * 3 ];

NS_PRIVATE GLfloat _ns_render3d_cube_normals[ 8 * 3 ] =
	{
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f
	};

NS_PRIVATE nsboolean _ns_render3d_cube_normalized = NS_FALSE;


NS_PRIVATE void _ns_render3d_cube_normalize( void )
	{
	nssize      i;
	NsVector3f  V;


	ns_assert( ! _ns_render3d_cube_normalized );

	for( i = 0; i < 8; ++i )
		{
		V.x = _ns_render3d_cube_normals[i*3+0];
		V.y = _ns_render3d_cube_normals[i*3+1];
		V.z = _ns_render3d_cube_normals[i*3+2];

		ns_vector3f_norm( &V );

		_ns_render3d_cube_normals[i*3+0] = V.x;
		_ns_render3d_cube_normals[i*3+1] = V.y;
		_ns_render3d_cube_normals[i*3+2] = V.z;
		}

	_ns_render3d_cube_normalized = NS_TRUE;
	}


NS_PRIVATE void _ns_render3d_pre_cubes
	(
	NsRenderState  *state,
	nsboolean       is_solid,
	nsboolean       use_lighting,
	nsboolean       unit_lines
	)
	{
	GLfloat line_size;

	if( ! is_solid )
		{
		if( ! use_lighting )
			_ns_render3d_disable_lighting( state );

		glGetFloatv( GL_LINE_WIDTH, &line_size );
		state->variables.line_size = line_size;

		glLineWidth( ( GLfloat )( unit_lines ? 1.0f : state->constants.line_size ) );

		_ns_render3d_enable_wire_frame( state );

		if( state->constants.anti_aliasing )
			_ns_render3d_enable_anti_aliasing( state );
		}

	if( state->constants.spines_soft_corners )
		{
		glEnableClientState( GL_VERTEX_ARRAY );
		glVertexPointer( 3, GL_FLOAT, 0, _ns_render3d_cube_vertices );

		if( use_lighting )
			{
			if( ! _ns_render3d_cube_normalized )
				_ns_render3d_cube_normalize();

			glEnableClientState( GL_NORMAL_ARRAY );
			glNormalPointer( GL_FLOAT, 0, _ns_render3d_cube_normals );
			}
		}
	}


NS_PRIVATE void _ns_render3d_draw_cube
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	)
	{
	GLfloat *v = _ns_render3d_cube_vertices;

   v[0*3+0]=V1->x; v[0*3+1]=V1->y; v[0*3+2]=V1->z;
   v[1*3+0]=V2->x; v[1*3+1]=V1->y; v[1*3+2]=V1->z;
   v[2*3+0]=V1->x; v[2*3+1]=V2->y; v[2*3+2]=V1->z;
   v[3*3+0]=V2->x; v[3*3+1]=V2->y; v[3*3+2]=V1->z;
   v[4*3+0]=V1->x; v[4*3+1]=V1->y; v[4*3+2]=V2->z;
   v[5*3+0]=V2->x; v[5*3+1]=V1->y; v[5*3+2]=V2->z;
   v[6*3+0]=V1->x; v[6*3+1]=V2->y; v[6*3+2]=V2->z;
   v[7*3+0]=V2->x; v[7*3+1]=V2->y; v[7*3+2]=V2->z;

	if( state->constants.spines_soft_corners )
		glDrawElements(
			GL_QUADS,
			NS_ARRAY_LENGTH( _ns_render3d_cube_faces ),
			GL_UNSIGNED_INT,
			_ns_render3d_cube_faces
			);
	else
		{
		glBegin( GL_QUADS );

		/* TODO: There might be a problem with the calls to glVertex3fv() when running
			a 32-bit version on a 64-bit machine? */

		glNormal3f( 0.0f, 0.0f, -1.0f );
		glVertex3fv( v + 0*3 ); glVertex3fv( v + 2*3 ); glVertex3fv( v + 3*3 ); glVertex3fv( v + 1*3 );

		glNormal3f( -1.0f, 0.0f, 0.0f );
		glVertex3fv( v + 1*3 ); glVertex3fv( v + 3*3 ); glVertex3fv( v + 7*3 ); glVertex3fv( v + 5*3 );

		glNormal3f( 0.0f, 0.0f, 1.0f );
		glVertex3fv( v + 5*3 ); glVertex3fv( v + 7*3 ); glVertex3fv( v + 6*3 ); glVertex3fv( v + 4*3 );

		glNormal3f( 1.0f, 0.0f, 0.0f );
		glVertex3fv( v + 4*3 ); glVertex3fv( v + 6*3 ); glVertex3fv( v + 2*3 ); glVertex3fv( v + 0*3 );

		glNormal3f( 0.0f, -1.0f, 0.0f );
		glVertex3fv( v + 4*3 ); glVertex3fv( v + 0*3 ); glVertex3fv( v + 1*3 ); glVertex3fv( v + 5*3 );

		glNormal3f( 0.0f, 1.0f, 0.0f );
		glVertex3fv( v + 2*3 ); glVertex3fv( v + 6*3 ); glVertex3fv( v + 7*3 ); glVertex3fv( v + 3*3 );

		glEnd();
		}
	}


NS_PRIVATE void _ns_render3d_post_cubes
	(
	NsRenderState  *state,
	nsboolean       is_solid,
	nsboolean       use_lighting
	)
	{
	if( state->constants.spines_soft_corners )
		{
		glDisableClientState( GL_VERTEX_ARRAY );

		if( use_lighting )
			glDisableClientState( GL_NORMAL_ARRAY );
		}

	if( ! is_solid )
		{
		_ns_render3d_restore_wire_frame( state );
		_ns_render3d_restore_line_size( state );

		if( state->constants.anti_aliasing )
			_ns_render3d_restore_anti_aliasing( state );

		if( ! use_lighting )
			_ns_render3d_enable_lighting( state );
		}
	}




void ns_render3d_pre_borders( NsRenderState *state )
	{  _ns_render3d_pre_cubes( state, NS_FALSE, NS_FALSE, NS_TRUE );  }


void ns_render3d_border
	(
	const NsVector3i  *V1i,
	const NsVector3i  *V2i,
	NsRenderState     *state
	)
	{
	NsVector3f V1, V2;

	glColor3f(
		NS_COLOR_GET_RED_F( state->variables.color ),
		NS_COLOR_GET_GREEN_F( state->variables.color ),
		NS_COLOR_GET_BLUE_F( state->variables.color )
		);

	ns_to_voxel_space( V1i, &V1, state->constants.voxel_info );
	ns_to_voxel_space( V2i, &V2, state->constants.voxel_info );

	_ns_render3d_draw_cube( &V1, &V2, state );
	}


void ns_render3d_post_borders( NsRenderState *state )
	{  _ns_render3d_post_cubes( state, NS_FALSE, NS_FALSE );  }




void ns_render3d_pre_lines( NsRenderState *state )
	{
	_ns_render3d_disable_lighting( state );
	_ns_render3d_set_line_size( state );

	if( state->constants.anti_aliasing )
		_ns_render3d_enable_anti_aliasing( state );

	glBegin( GL_LINES );
	}


void ns_render3d_line
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	)
	{
	glColor3f(
		NS_COLOR_GET_RED_F( state->variables.color ),
		NS_COLOR_GET_GREEN_F( state->variables.color ),
		NS_COLOR_GET_BLUE_F( state->variables.color )
		);

	glVertex3f( V1->x, V1->y, V1->z );
	glVertex3f( V2->x, V2->y, V2->z );
	}


void ns_render3d_post_lines( NsRenderState *state )
	{
	glEnd();

	_ns_render3d_restore_line_size( state );

	if( state->constants.anti_aliasing )
		_ns_render3d_restore_anti_aliasing( state );

	_ns_render3d_enable_lighting( state );
	}




void ns_render3d_pre_points( NsRenderState *state )
	{
	_ns_render3d_disable_lighting( state );
	_ns_render3d_set_point_size( state );

	glBegin( GL_POINTS );
	}


void ns_render3d_point
	(
	const NsVector3b  *N,
	const NsVector3f  *P,
	NsRenderState     *state
	)
	{
	glColor3f(
		NS_COLOR_GET_RED_F( state->variables.color ),
		NS_COLOR_GET_GREEN_F( state->variables.color ),
		NS_COLOR_GET_BLUE_F( state->variables.color )
		);

	glNormal3b( N->x, N->y, N->z );
	glVertex3f( P->x, P->y, P->z );
	}


void ns_render3d_post_points( NsRenderState *state )
	{
	glEnd();

	_ns_render3d_restore_point_size( state );
	_ns_render3d_enable_lighting( state );
	}




NS_PRIVATE void _ns_render3d_draw_sphere( nsdouble radius, nsint complexity )
   {
   nsdouble  phi, theta, delta, x, y, z, p;
   nsint     i, j;


   delta = NS_PI_D / 2.0 / ( nsdouble )complexity;

   for( i = 0, theta = 0.0; i < 4 * complexity; i++, theta += delta )
      {
      glBegin( GL_TRIANGLE_STRIP );
       
      glNormal3d( 0.0, 1.0, 0.0 );
      glVertex3d( 0.0, radius, 0.0 );

      for( j = 1, phi = delta; j < 2 * complexity; j++, phi += delta )
         {
         y = ns_cos( phi );
         p = ns_sin( phi );
         x = p * ns_cos( theta );
         z = p * -ns_sin( theta );

         glNormal3d( x, y, z );
         glVertex3d( radius * x, radius * y, radius * z );

         x = p * ns_cos( theta + delta );
         z = p * -ns_sin( theta + delta );

         glNormal3d( x, y, z );
         glVertex3d( radius * x, radius * y, radius * z );
         }

      glNormal3d( 0.0, -1.0, 0.0 );
      glVertex3d( 0.0, -radius, 0.0 );
       
      glEnd();
      }
   }


void ns_render3d_pre_spheres( NsRenderState *state )
	{
	if( ! state->variables.filled )
		_ns_render3d_enable_wire_frame( state );

	if( ! state->variables.filled && state->constants.anti_aliasing )
		_ns_render3d_enable_anti_aliasing( state );

	state->variables.display_lists = glGenLists( 1 );

	glNewList( state->variables.display_lists, GL_COMPILE );
	_ns_render3d_draw_sphere( 1.0, state->constants.polygon_complexity );
	glEndList();
	}


void ns_render3d_sphere
	(
	const NsVector3f  *P,
	nsfloat            radius,
	NsRenderState     *state
	)
	{
   glPushMatrix();

	glTranslatef( P->x, P->y, P->z );
   glScalef( radius, radius, radius );

	glColor4ub(
		state->variables.color.x,
		state->variables.color.y,
		state->variables.color.z,
		state->variables.color.w
		);

	glCallList( state->variables.display_lists );

	glPopMatrix();
	}


void ns_render3d_post_spheres( NsRenderState *state )
	{
	if( ! state->variables.filled )
		_ns_render3d_restore_wire_frame( state );

	if( ! state->variables.filled && state->constants.anti_aliasing )
		_ns_render3d_restore_anti_aliasing( state );

	glDeleteLists( state->variables.display_lists, 1 );
	state->variables.display_lists = 0;
	}




void ns_render3d_pre_aabboxes( NsRenderState *state )
	{
	_ns_render3d_disable_lighting( state );
	_ns_render3d_set_line_size( state );

	if( state->constants.anti_aliasing )
		_ns_render3d_enable_anti_aliasing( state );

	glColor3f(
		NS_COLOR_GET_RED_F( state->variables.color ),
		NS_COLOR_GET_GREEN_F( state->variables.color ),
		NS_COLOR_GET_BLUE_F( state->variables.color )
		);

	glBegin( GL_LINES );
	}


NS_PRIVATE void _ns_render3d_aabbox_line
	(
	nspointer          not_used,
	const NsVector3d  *V1,
	const NsVector3d  *V2
	)
	{
	NS_USE_VARIABLE( not_used );

	glVertex3dv( ns_vector3d_const_array( V1 ) );
	glVertex3dv( ns_vector3d_const_array( V2 ) );
	}


void ns_render3d_aabbox
	(
	const NsAABBox3d  *B,
	NsRenderState     *state
	)
	{
	NS_USE_VARIABLE( state );
	ns_aabbox3d_render( B, _ns_render3d_aabbox_line, NULL );
	}


void ns_render3d_post_aabboxes( NsRenderState *state )
	{
	glEnd();

	_ns_render3d_restore_line_size( state );

	if( state->constants.anti_aliasing )
		_ns_render3d_restore_anti_aliasing( state );

	_ns_render3d_enable_lighting( state );
	}




NS_PRIVATE void _ns_render3d_draw_cylinder
	(
	nsdouble  radius1,
	nsdouble  radius2,
	nsdouble  height,
	nsint     complexity
	)
	{
   nsdouble  theta, x, y, z, delta, step;
   nsint     i, j;


   delta = NS_PI_D / 2.0 / ( nsdouble )complexity;
   y     = height;
   theta = 0.0;
   step  = height / ( nsdouble )complexity;

   glBegin( GL_TRIANGLE_STRIP );

   for( j = 0; j < complexity; ++j )
      {
      for( i = 0; i < 4 * complexity + 1; i++, theta += delta )
         {
         x = ns_cos( theta );
         z = -ns_sin( theta );

         glNormal3d( x, 0.0, z );
         glVertex3d( radius1 * x, y, radius1 * z );
         glVertex3d( radius2 * x, y - step, radius2 * z );
         }

      y -= step;
      }

   glEnd();
	}


void ns_render3d_pre_frustums( NsRenderState *state )
	{
	if( ! state->variables.filled )
		_ns_render3d_enable_wire_frame( state );

	if( ! state->variables.filled && state->constants.anti_aliasing )
		_ns_render3d_enable_anti_aliasing( state );

	state->variables.display_lists = glGenLists( 1 );

	glNewList( state->variables.display_lists, GL_COMPILE );
		_ns_render3d_draw_cylinder( 1.0, 1.0, 1.0, state->constants.polygon_complexity );
	glEndList();
	}


void ns_render3d_frustum
	(
	const NsVector3f  *P1,
	nsfloat            r1,
	const NsVector3f  *P2,
	nsfloat            r2,
	NsRenderState     *state
	)
	{
	nsfloat dx, dy, dz, dd, M[16];

	dx = P2->x - P1->x;
	dy = P2->y - P1->y;
	dz = P2->z - P1->z;
	dd = ns_sqrtf( dx * dx + dy * dy + dz * dz );

	if( 0.0f < dd )
		{
		glPushMatrix();

		/* Move it and rotate it into position. */
		glTranslatef( P1->x, P1->y, P1->z );
		glRotatef( -ns_atan2f( dz, dx ) * 180.0f / NS_PI_F, 0.0f, 1.0f, 0.0f );
		glRotatef( -ns_acosf( dy / dd ) * 180.0f / NS_PI_F, 0.0f, 0.0f, 1.0f );

		glColor4ub(
			state->variables.color.x,
			state->variables.color.y,
			state->variables.color.z,
			state->variables.color.w
			);

		/*  Tapering matrix: Remember OpenGL expects column major matrix!

		|  r1     0         0    0  |
		|   0  dd*r1/r2     0    0  |
		|   0     0        r1    0  |
		|   0   r1/r2 - 1   0    1  |
		*/
		M[ 0] = r1;   M[ 4] = 0.0f;           M[ 8] = 0.0f; M[12] = 0.0f;
		M[ 1] = 0.0f; M[ 5] = dd * r1 / r2;   M[ 9] = 0.0f; M[13] = 0.0f;
		M[ 2] = 0.0f; M[ 6] = 0.0f;           M[10] = r1;   M[14] = 0.0f;
		M[ 3] = 0.0f; M[ 7] = r1 / r2 - 1.0f; M[11] = 0.0f; M[15] = 1.0f;

		glPushMatrix();
			glMultMatrixf( M );
			glCallList( state->variables.display_lists );
		glPopMatrix();

		glPopMatrix();
		}
	}


void ns_render3d_post_frustums( NsRenderState *state )
	{
	if( ! state->variables.filled )
		_ns_render3d_restore_wire_frame( state );

	if( ! state->variables.filled && state->constants.anti_aliasing )
		_ns_render3d_restore_anti_aliasing( state );

	glDeleteLists( state->variables.display_lists, 1 );
	state->variables.display_lists = 0;
	}




void ns_render3d_pre_triangles( NsRenderState *state )
	{
	if( ! state->variables.filled )
		_ns_render3d_enable_wire_frame( state );

	if( ! state->variables.filled && state->constants.anti_aliasing )
		_ns_render3d_enable_anti_aliasing( state );

	glBegin( GL_TRIANGLES );
	}


void ns_render3d_triangle
	(
	const NsVector3f  *N1,
	const NsVector3f  *P1,
	const NsVector3f  *N2,
	const NsVector3f  *P2,
	const NsVector3f  *N3,
	const NsVector3f  *P3,
	NsRenderState     *state
	)
	{
	glColor3f(
		NS_COLOR_GET_RED_F( state->variables.color ),
		NS_COLOR_GET_GREEN_F( state->variables.color ),
		NS_COLOR_GET_BLUE_F( state->variables.color )
		);

	glNormal3f( N1->x, N1->y, N1->z );
	glVertex3f( P1->x, P1->y, P1->z );
	glNormal3f( N2->x, N2->y, N2->z );
	glVertex3f( P2->x, P2->y, P2->z );
	glNormal3f( N3->x, N3->y, N3->z );
	glVertex3f( P3->x, P3->y, P3->z );
	}


void ns_render3d_post_triangles( NsRenderState *state )
	{
	glEnd();

	if( ! state->variables.filled )
		_ns_render3d_restore_wire_frame( state );

	if( ! state->variables.filled && state->constants.anti_aliasing )
		_ns_render3d_restore_anti_aliasing( state );
	}




void ns_render3d_pre_ellipses( NsRenderState *state )
	{  ns_render3d_pre_spheres( state );  }


void ns_render3d_ellipse
	(
	const NsVector3f  *V,
	const NsVector3f  *R,
	NsRenderState     *state
	)
	{
   glPushMatrix();

	glTranslatef( V->x, V->y, V->z );
   glScalef( R->x, R->y, R->z );

	glColor4ub(
		state->variables.color.x,
		state->variables.color.y,
		state->variables.color.z,
		state->variables.color.w
		);

	glCallList( state->variables.display_lists );

	glPopMatrix();
	}


void ns_render3d_post_ellipses( NsRenderState *state )
	{  ns_render3d_post_spheres( state );  }




void ns_render3d_pre_rectangles( NsRenderState *state )
	{  _ns_render3d_pre_cubes( state, state->variables.filled, state->constants.lighting, NS_FALSE );  }


void ns_render3d_rectangle
	(
	const NsVector3f  *V1,
	const NsVector3f  *V2,
	NsRenderState     *state
	)
	{
	glColor3f(
		NS_COLOR_GET_RED_F( state->variables.color ),
		NS_COLOR_GET_GREEN_F( state->variables.color ),
		NS_COLOR_GET_BLUE_F( state->variables.color )
		);

	_ns_render3d_draw_cube( V1, V2, state );
	}


void ns_render3d_post_rectangles( NsRenderState *state )
	{  _ns_render3d_post_cubes( state, state->variables.filled, state->constants.lighting );  }
