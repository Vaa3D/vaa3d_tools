/**************************************************************************

Rayburst Sampling Algorithm
This code is used to cast rays inside a 3D volumetric dataset.

Copyright (C) 2006 Computational Neurobiology and Imaging Center
Mount Sinani School of Medicine, New York NY
www.mssm.edu/cnic

Software Development: Douglas Ehlenberger and Alfredo Rodriguez

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

Please send any questions or comments to douglas.ehlenberger@mssm.edu

***************************************************************************/

#include "srb.h"


#define _SRB_ARRAY_LENGTH( array )\
   ( sizeof( (array) ) / sizeof( (array)[ 0 ] ) )


#define _SRB_MIN3( a, b, c )\
	( ( a < b ) ? ( ( a < c ) ? 0 : 2 ) : ( ( b < c ) ? 1 : 2 ) )


#define _SRB_DISTANCE_SQUARED( x1, y1, z1, x2, y2, z2 )\
	( ( (x2) - (x1) ) * ( (x2) - (x1) ) +\
	  ( (y2) - (y1) ) * ( (y2) - (y1) ) +\
	  ( (z2) - (z1) ) * ( (z2) - (z1) )   )


/* Helper macros to properly index vertex
	and triangle arrays. */

#define _SRB_VERTEX( vertices, at )\
	( (vertices) + ( 3 * (at) ) )

#define _SRB_TRIANGLE( triangles, at )\
	( (triangles) + ( 3 * (at) ) )


/* Forward declarations. */

struct _SRBVERTEX;
typedef struct _SRBVERTEX SRBVERTEX;

struct _SRBEDGE;
typedef struct _SRBEDGE SRBEDGE;

struct _SRBTRIANGLE;
typedef struct _SRBTRIANGLE SRBTRIANGLE;


struct _SRBVERTEX
	{
	double      x;
	double      y;
	double      z;
	int         index;
	SRBVERTEX  *next;
	};


struct _SRBEDGE
	{
	SRBVERTEX    *V1;
	SRBVERTEX    *V2;
	SRBTRIANGLE  *T1;
	SRBTRIANGLE  *T2;
	SRBVERTEX    *M;
	SRBEDGE      *E1;
	SRBEDGE      *E2;
	SRBEDGE      *O;
	int           iteration;
	SRBEDGE      *next;
	};


#define _SRB_EDGE_MIDPOINT( E, _x, _y, _z )\
	(_x) = ( (E)->V1->x + (E)->V2->x ) / 2.0;\
	(_y) = ( (E)->V1->y + (E)->V2->y ) / 2.0;\
	(_z) = ( (E)->V1->z + (E)->V2->z ) / 2.0


#define _SRB_EDGE_FIND_CHILD( P, V1, V2, E )\
	( (E) = _SRB_IS_EDGE( (P)->E1, (V1), (V2) ) ? (P)->E1 : (P)->E2 )


/* NOTE: OK to use address since they are unique. */
#define _SRB_IS_EDGE( E, _V1, _V2 )\
	( ( (E)->V1 == (_V1) && (E)->V2 == (_V2) ) || ( (E)->V1 == (_V2) && (E)->V2 == (_V1) ) )


struct _SRBTRIANGLE
	{
	SRBEDGE      *E1;
	SRBEDGE      *E2;
	SRBEDGE      *E3;
	SRBVERTEX    *V1;
	SRBVERTEX    *V2;
	SRBVERTEX    *V3;
	int           iteration;
	SRBTRIANGLE  *next;
	};


#define _SRB_TRIANGLE_CENTER( T, _x, _y, _z )\
	(_x) = ( (T)->V1->x + (T)->V2->x + (T)->V3->x ) / 3.0;\
	(_y) = ( (T)->V1->y + (T)->V2->y + (T)->V3->y ) / 3.0;\
	(_z) = ( (T)->V1->z + (T)->V2->z + (T)->V3->z ) / 3.0


typedef struct _SRBRAY
	{
	double  ox;
	double  oy;
	double  oz;
	double  dx;
	double  dy;
	double  dz;
	double  ex;
	double  ey;
	double  ez;
	}
	SRBRAY;


typedef struct _SRBPYRAMID
	{
	double  ox;
	double  oy;
	double  oz;
	double  ax;
	double  ay;
	double  az;
	double  bx;
	double  by;
	double  bz;
	double  cx;
	double  cy;
	double  cz;
	double  volume;
	double  face_area;
	}
	SRBPYRAMID;


typedef struct _SRB
	{
	SRBVERTEX    *vertices;
	SRBEDGE      *edges;
	SRBTRIANGLE  *triangles;
	int           iteration;
	SRBEDGE      *tail;

	/* Dummy headers. Avoid unnecessary allocation. */
	SRBVERTEX    _vertex;
	SRBEDGE      _edge;
	SRBTRIANGLE  _triangle;
	}
	SRB;


static SRBVERTEX* _srb_vertex_new
	(
	SRB     *srb,
	double   x,
	double   y,
	double   z
	)
	{
	SRBVERTEX *V;

	if( NULL == ( V = malloc( sizeof( SRBVERTEX ) ) ) )
		return NULL;

	V->x = x;
	V->y = y;
	V->z = z;

	V->next = srb->vertices->next;
	srb->vertices->next = V;

	return V;
	}


static SRBVERTEX* _srb_vertex_new_at
	(
	SRB     *srb,
	int      at,
	double   x,
	double   y,
	double   z
	)
	{
	SRBVERTEX  *V, *head;
	int         i;


	if( NULL == ( V = malloc( sizeof( SRBVERTEX ) ) ) )
		return NULL;

	V->x = x;
	V->y = y;
	V->z = z;

	head = srb->vertices;
	for( i = 0; i < at; ++i )
		head = head->next;

	V->next    = head->next;
	head->next = V;

	return V;
	}


static SRBVERTEX* _srb_vertex_at( SRB *srb, int at )
	{
	SRBVERTEX  *V;
	int         i;


	V = srb->vertices->next;

	for( i = 0; i < at; ++i )
		V = V->next;

	return V;
	}


static SRBEDGE* _srb_edge_new
	(
	SRB        *srb,
	SRBVERTEX  *V1,
	SRBVERTEX  *V2
	)
	{
	SRBEDGE *E;

	if( NULL == ( E = malloc( sizeof( SRBEDGE ) ) ) )
		return NULL;

	E->V1 = V1;
	E->V2 = V2;
	E->T1 = NULL;
	E->T2 = NULL;
	E->M  = NULL;
	E->E1 = NULL;
	E->E2 = NULL;
	E->O  = NULL;

	E->iteration = srb->iteration;

	E->next = srb->edges->next;
	srb->edges->next = E;

	return E;
	}


static SRBEDGE* _srb_edge_new_at_tail( SRB *srb, SRBVERTEX *V1, SRBVERTEX *V2 )
	{
	SRBEDGE *E;

	if( NULL == ( E = _srb_edge_new( srb, V1, V2 ) ) )
		return NULL;

	/* Splice out from front. */
	srb->edges->next = E->next;

	if( NULL == srb->tail )
		{
		/* Splice back into front if first one. */
		E->next = srb->edges->next;
		srb->edges->next = E;
		}
	else
		{
		E->next = srb->tail->next;
		srb->tail->next = E;
		}

	srb->tail = E;

	return E;
	}


static SRBEDGE* _srb_edge_find( SRB *srb, SRBVERTEX *V1, SRBVERTEX *V2 )
	{
	SRBEDGE *E = srb->edges->next;

	while( NULL != E )
		{
		if( _SRB_IS_EDGE( E, V1, V2 ) )
			return E;

		E = E->next;
		}

	return NULL;
	}


static SRBEDGE* _srb_edge_new_unique( SRB *srb, SRBVERTEX *V1, SRBVERTEX *V2 )
	{
	SRBEDGE *E;

	if( NULL == ( E = _srb_edge_find( srb, V1, V2 ) ) )
		return _srb_edge_new( srb, V1, V2 );
	else
		return E;
	}


static void _srb_edge_set_triangle( SRBEDGE *E, SRBTRIANGLE *T )
	{
	if( NULL == E->T1 )
		E->T1 = T;
	else if( NULL == E->T2 )
		E->T2 = T;
	}


static SRBTRIANGLE* _srb_triangle_new
	(
	SRB        *srb,
	SRBEDGE    *E1,
	SRBEDGE    *E2,
	SRBEDGE    *E3,
	SRBVERTEX  *V1,
	SRBVERTEX  *V2,
	SRBVERTEX  *V3
	)
	{
	SRBTRIANGLE *T;

	if( NULL == ( T = malloc( sizeof( SRBTRIANGLE ) ) ) )
		return NULL;

	T->E1 = E1;
	T->E2 = E2;
	T->E3 = E3;
	T->V1 = V1;
	T->V2 = V2;
	T->V3 = V3;

	T->iteration = srb->iteration;

	T->next = srb->triangles->next;
	srb->triangles->next = T;

	return T;
	}


static void _srb_init( SRB *srb, SRBPARAMS *params )
	{
	srb->_vertex.next   = NULL;
	srb->vertices       = &srb->_vertex;
	srb->_edge.next     = NULL;
	srb->edges          = &srb->_edge;
	srb->_triangle.next = NULL;
	srb->triangles      = &srb->_triangle;

	srb->iteration = 0;

	/* Convert the origin from voxel space to image space. */
	params->origin_x = params->origin_x / params->voxel_width;
	params->origin_y = params->origin_y / params->voxel_height;
	params->origin_z = params->origin_z / params->voxel_length;
	}


static int _srb_init_2d
	(
	SRB        *srb,
	SRBPARAMS  *params,
	double     *vertices,
	int         vertex_count
	)
	{
	int         i, j;
	double     *v;
	SRBVERTEX  *V, *V1, *V2;
	SRBEDGE    *E1, *E2;


	_srb_init( srb, params );
	srb->tail = NULL;

	for( i = 0; i < vertex_count; ++i )
		{
		v = _SRB_VERTEX( vertices, i );

		if( NULL == _srb_vertex_new_at( srb, i, v[0], v[1], v[2] ) )
			return 0;
		}

	V = srb->vertices->next;

	while( NULL != V )
		{
		V1 = V;
		V2 = NULL != V->next ? V->next : srb->vertices->next;

		if( NULL == _srb_edge_new_at_tail( srb, V1, V2 ) )
			return 0;

		V = V->next;
		}

	/* Pair up opposite edges. */

	E1 = srb->edges->next;

	for( i = 0; i < vertex_count / 2; ++i )
		{
		E2 = E1;

		for( j = 0; j < vertex_count / 2; ++j )
			E2 = E2->next;

		E1->O = E2;
		E2->O = E1;

		E1 = E1->next;
		}

	return 1;
	}


static int _srb_init_3d
	(
	SRB        *srb,
	SRBPARAMS  *params,
	double     *vertices,
	int         vertex_count,
	int        *triangles,
	int         triangle_count
	)
	{
	int           i;
	double       *v;
	int          *t;
	SRBVERTEX    *V1, *V2, *V3;
	SRBEDGE      *E1, *E2, *E3;
	SRBTRIANGLE  *T;


	_srb_init( srb, params );

	for( i = 0; i < vertex_count; ++i )
		{
		v = _SRB_VERTEX( vertices, i );

		if( NULL == _srb_vertex_new_at( srb, i, v[0], v[1], v[2] ) )
			return 0;
		}

	for( i = 0; i < triangle_count; ++i )
		{
		t = _SRB_VERTEX( triangles, i );

		V1 = _srb_vertex_at( srb, t[0] );
		V2 = _srb_vertex_at( srb, t[1] );
		V3 = _srb_vertex_at( srb, t[2] );

		if( NULL == ( E1 = _srb_edge_new_unique( srb, V1, V2 ) ) ||
			 NULL == ( E2 = _srb_edge_new_unique( srb, V2, V3 ) ) ||
			 NULL == ( E3 = _srb_edge_new_unique( srb, V3, V1 ) )    )
				return 0;

		if( NULL == ( T = _srb_triangle_new( srb, E1, E2, E3, V1, V2, V3 ) ) )
			return 0;

		_srb_edge_set_triangle( E1, T );
		_srb_edge_set_triangle( E2, T );
		_srb_edge_set_triangle( E3, T );
		}

	return 1;
	}


static _srb_finalize( SRB *srb )
	{
	SRBVERTEX    *V;
	SRBEDGE      *E;
	SRBTRIANGLE  *T;
	void         *next;


	V = srb->vertices->next;

	while( NULL != V )
		{
		next = V->next;
		free( V );
		V = next;
		}

	E = srb->edges->next;

	while( NULL != E )
		{
		next = E->next;
		free( E );
		E = next;
		}

	T = srb->triangles->next;

	while( NULL != T )
		{
		next = T->next;
		free( T );
		T = next;
		}
	}


static void _srb_set_midpoint_2d( SRBEDGE *E, SRBVERTEX *V, SRBPARAMS *params )
	{
	double  v1x, v1y, v1z, v2x, v2y, v2z;
	double  m;


	/* Find direction vectors. */

	v1x = E->V1->x - params->origin_x;
	v1y = E->V1->y - params->origin_y;
	v1z = E->V1->z - params->origin_z;

	v2x = E->V2->x - params->origin_x;
	v2y = E->V2->y - params->origin_y;
	v2z = E->V2->z - params->origin_z;

	/* Normalize them. */

	m = sqrt( v1x * v1x + v1y * v1y + v1z * v1z );
	v1x /= m;
	v1y /= m;
	v1z /= m;

	m = sqrt( v2x * v2x + v2y * v2y + v2z * v2z );
	v2x /= m;
	v2y /= m;
	v2z /= m;

	/* Just add the two vectors and we get the right direction. */
	V->x = v1x + v2x;
	V->y = v1y + v2y;
	V->z = v1z + v2z;
	}


static int _srb_tesselate_2d( SRB *srb, SRBEDGE *E, SRBPARAMS *params )
	{
/*  
                 
     () E->O->V1             () E->V2
      \                       \
       \                       \
        \                       \
         \  E->O->E1             \  E->E2
          \                       \
           \                       \
           () E->O->M     ()        () E->M
             \          origin       \  
              \                       \
               \                       \
                \                       \  E->E1
                 \  E->O->E2             \
                  \                       \
                   \                       \
                   () E->O->V2             () E->V1
*/

	if( NULL == E->M )
		{
		if( NULL == ( E->M = _srb_vertex_new( srb, 0.0, 0.0, 0.0 ) ) )
			return 0;

		_srb_set_midpoint_2d( E, E->M, params );

		if( NULL == ( E->E1 = _srb_edge_new_at_tail( srb, E->V1, E->M ) ) )
			return 0;

		if( NULL == ( E->E2 = _srb_edge_new_at_tail( srb, E->M, E->V2 ) ) )
			return 0;
		}

	if( NULL != E->O->M )
		{
		/* NOTE: Pair up the edges so that on the next iteration a line
			drawn between their midpoints always passes through the origin. */

		E->E1->O    = E->O->E1;
		E->O->E1->O = E->E1;

		E->E2->O    = E->O->E2;
		E->O->E2->O = E->E2;
		}

	return 1;
	}


static int _srb_tesselate_3d( SRB *srb, SRBTRIANGLE *T )
	{
	SRBEDGE      *E1, *E2, *E3, *E4, *E5, *E6, *E7, *E8, *E9;
	SRBTRIANGLE  *T1, *T2, *T3, *T4;
	double        x, y, z;

/*                   T->V2
                      ()
                      /\
                     /  \
                    /    \
                 E5/  T2  \E4
                  /        \
                 /          \
       T->E2->M()-----E6-----()T->E1->M
               /\            /\
              /  \    T4    /  \
             /    \        /    \
          E8/  T3  \E7  E2/  T1  \E1
           /        \    /        \
          /          \  /          \
         /-----E9-----\/-----E3-----\
   T->V3()            ()            ()T->V1
                   T->E3->M
*/
	      
	/* If necessary create up to 3 new midpoint vertices and up to
		6 new outside edges. */
	if( NULL == T->E1->M )
		{
		_SRB_EDGE_MIDPOINT( T->E1, x, y, z );

		if( NULL == ( T->E1->M = _srb_vertex_new( srb, x, y, z ) ) )
			return 0;

		if( NULL == ( T->E1->E1 = _srb_edge_new( srb, T->V1, T->E1->M ) ) )
			return 0;

		if( NULL == ( T->E1->E2 = _srb_edge_new( srb, T->E1->M, T->V2 ) ) )
			return 0;

		E1 = T->E1->E1;
		E4 = T->E1->E2;
		}
	else
		{
		_SRB_EDGE_FIND_CHILD( T->E1, T->V1, T->E1->M, E1 );
		_SRB_EDGE_FIND_CHILD( T->E1, T->E1->M, T->V2, E4 );
		}

	if( NULL == T->E2->M )
		{
		_SRB_EDGE_MIDPOINT( T->E2, x, y, z );

		if( NULL == ( T->E2->M = _srb_vertex_new( srb, x, y, z ) ) )
			return 0;

		if( NULL == ( T->E2->E1 = _srb_edge_new( srb, T->V2, T->E2->M ) ) )
			return 0;

		if( NULL == ( T->E2->E2 = _srb_edge_new( srb, T->E2->M, T->V3 ) ) )
			return 0;

		E5 = T->E2->E1;
		E8 = T->E2->E2;
		}
	else
		{
		_SRB_EDGE_FIND_CHILD( T->E2, T->V2, T->E2->M, E5 );
		_SRB_EDGE_FIND_CHILD( T->E2, T->E2->M, T->V3, E8 );
		}

	if( NULL == T->E3->M )
		{
		_SRB_EDGE_MIDPOINT( T->E3, x, y, z );

		if( NULL == ( T->E3->M = _srb_vertex_new( srb, x, y, z ) ) )
			return 0;

		if( NULL == ( T->E3->E1 = _srb_edge_new( srb, T->V3, T->E3->M ) ) )
			return 0;

		if( NULL == ( T->E3->E2 = _srb_edge_new( srb, T->E3->M, T->V1 ) ) )
			return 0;

		E9 = T->E3->E1;
		E3 = T->E3->E2;
		}
	else
		{
		_SRB_EDGE_FIND_CHILD( T->E3, T->V3, T->E3->M, E9 );
		_SRB_EDGE_FIND_CHILD( T->E3, T->E3->M, T->V1, E3 );
		}

	/* Always create 3 new edges for the middle triangle. */
	if( NULL == ( E6 = _srb_edge_new( srb, T->E1->M, T->E2->M ) ) )
		return 0;

	if( NULL == ( E7 = _srb_edge_new( srb, T->E2->M, T->E3->M ) ) )
		return 0;

	if( NULL == ( E2 = _srb_edge_new( srb, T->E3->M, T->E1->M ) ) )
		return 0;

	/* Create 4 new triangles from this original one. */
	if( NULL == ( T1 = _srb_triangle_new( srb, E1, E2, E3, T->V1, T->E1->M, T->E3->M ) ) )
		return 0;

	if( NULL == ( T2 = _srb_triangle_new( srb, E4, E5, E6, T->E1->M, T->V2, T->E2->M ) ) )
		return 0;

	if( NULL == ( T3 = _srb_triangle_new( srb, E7, E8, E9, T->E3->M, T->E2->M, T->V3 ) ) )
		return 0;

	if( NULL == ( T4 = _srb_triangle_new( srb, E6, E7, E2, T->E1->M, T->E2->M, T->E3->M ) ) )
		return 0;

	_srb_edge_set_triangle( E1, T1 );
	_srb_edge_set_triangle( E2, T1 );
	_srb_edge_set_triangle( E2, T4 );
	_srb_edge_set_triangle( E3, T1 );

	_srb_edge_set_triangle( E4, T2 );
	_srb_edge_set_triangle( E5, T2 );
	_srb_edge_set_triangle( E6, T2 );
	_srb_edge_set_triangle( E6, T4 );

	_srb_edge_set_triangle( E7, T3 );
	_srb_edge_set_triangle( E7, T4 );
	_srb_edge_set_triangle( E8, T3 );
	_srb_edge_set_triangle( E9, T3 );

	return 1;
	}


static void _srb_mesh_delete( SRBMESH *mesh )
	{
	free( mesh->vertices );
	free( mesh->triangles );
	free( mesh );
	}


static SRBMESH* _srb_mesh_new( int vertex_count, int triangle_count )
	{
	SRBMESH *mesh;

	if( NULL == ( mesh = malloc( sizeof( SRBMESH ) ) ) )
		return NULL;

	mesh->vertices  = malloc( sizeof( double ) * 3 * vertex_count );
	mesh->triangles = malloc( sizeof( int ) * 3 * triangle_count );

	if( NULL == mesh->vertices || NULL == mesh->triangles )
		{
		_srb_mesh_delete( mesh );
		return NULL;
		}

	mesh->vertex_count   = vertex_count;
	mesh->triangle_count = triangle_count;

	mesh->volume       = 0.0;
	mesh->surface_area = 0.0;

	return mesh;
	}


static void _srb_fan_delete( SRBFAN *fan )
	{
	free( fan->vertices );
	free( fan );
	}


static SRBFAN* _srb_fan_new( int vertex_count )
	{
	SRBFAN *fan;

	if( NULL == ( fan = malloc( sizeof( SRBFAN ) ) ) )
		return NULL;

	fan->vertices = malloc( sizeof( double ) * 3 * vertex_count );

	if( NULL == fan->vertices )
		{
		_srb_fan_delete( fan );
		return NULL;
		}

	fan->vertex_count = vertex_count;

	fan->diameter = 0.0;
	fan->area     = 0.0;

	return fan;
	}


static void _srb_pyramid_face_area( SRBPYRAMID *triangle )
	{
	double nx, ny, nz;
	double oax, oay, oaz;
	double obx, oby, obz;


	/* Compute face vectors. */
	oax = triangle->ax - triangle->ox;
	oay = triangle->ay - triangle->oy;
	oaz = triangle->az - triangle->oz;

	obx = triangle->bx - triangle->ox;
	oby = triangle->by - triangle->oy;
	obz = triangle->bz - triangle->oz;

	/* Compute normal. */
	nx = oay * obz - oby * oaz;
	ny = oaz * obx - oax * obz;
	nz = oax * oby - obx * oay;

	/* Compute surface area. */
	triangle->face_area = 0.5 * sqrt( nx * nx + ny * ny + nz * nz );
	}


static void _srb_pyramid_volume_and_surface_area( SRBPYRAMID *pyramid )
	{
	double  nx, ny, nz;
	double  acx, acy, acz;
	double  abx, aby, abz;
	double  aox, aoy, aoz;


	/* Compute base vectors. */
	acx = pyramid->cx - pyramid->ax;
	acy = pyramid->cy - pyramid->ay;
	acz = pyramid->cz - pyramid->az;

	abx = pyramid->bx - pyramid->ax;
	aby = pyramid->by - pyramid->ay;
	abz = pyramid->bz - pyramid->az;

	/* Compute normal. */
	nx = acy * abz - aby * acz;
	ny = abx * acz - acx * abz;
	nz = acx * aby - abx * acy;

	/* Compute 'a' to 'o' vector. */
	aox = pyramid->ox - pyramid->ax;
	aoy = pyramid->oy - pyramid->ay;
	aoz = pyramid->oz - pyramid->az;

	/* Compute surface area. */
	pyramid->face_area = 0.5 * sqrt( nx * nx + ny * ny + nz * nz );

	/* Compute volume. */
	pyramid->volume = ( 1.0 / 6.0 ) * ( aox * nx + aoy * ny + aoz * nz );

	/* If winding of the base triangle is known this test
		may be eliminated. */
	if( pyramid->volume < 0.0 )
		pyramid->volume = -pyramid->volume;
	}


static SRBMESH* _srb_to_mesh( SRB *srb, SRBPARAMS *params )
	{
	SRBMESH      *mesh;
	SRBVERTEX    *V;
	SRBTRIANGLE  *T;
	int           vertex_count;
	int           triangle_count;
	double       *v;
	int          *t;
	int           i;
	SRBPYRAMID    P;


	vertex_count = 0;
	V            = srb->vertices->next;

	while( NULL != V )
		{
		/* Convert the point from image space to voxel space. */
		V->x = V->x * params->voxel_width;
		V->y = V->y * params->voxel_height;
		V->z = V->z * params->voxel_length;

		V->index = vertex_count++;
		V = V->next;
		}

	triangle_count = 0;
	T              = srb->triangles->next;

	while( NULL != T )
		{
		if( T->iteration == srb->iteration )
			++triangle_count;

		T = T->next;
		}

	if( NULL == ( mesh = _srb_mesh_new( vertex_count, triangle_count ) ) )
		return NULL;

	V = srb->vertices->next;

	while( NULL != V )
		{
		v = _SRB_VERTEX( mesh->vertices, V->index );

		v[0] = V->x;
		v[1] = V->y;
		v[2] = V->z;

		V = V->next;
		}

	/* Convert the origin back to voxel space. */
	params->origin_x = params->origin_x * params->voxel_width;
	params->origin_y = params->origin_y * params->voxel_height;
	params->origin_z = params->origin_z * params->voxel_length;

	P.ox = params->origin_x;
	P.oy = params->origin_y;
	P.oz = params->origin_z;

	i = 0;
	T = srb->triangles->next;

	while( NULL != T )
		{
		if( T->iteration == srb->iteration )
			{
			t = _SRB_TRIANGLE( mesh->triangles, i );
			++i;

			t[0] = T->V1->index;
			t[1] = T->V2->index;
			t[2] = T->V3->index;

			P.ax = T->V1->x;
			P.ay = T->V1->y;
			P.az = T->V1->z;
			P.bx = T->V2->x;
			P.by = T->V2->y;
			P.bz = T->V2->z;
			P.cx = T->V3->x;
			P.cy = T->V3->y;
			P.cz = T->V3->z;

			_srb_pyramid_volume_and_surface_area( &P );

			mesh->volume       += P.volume;
			mesh->surface_area += P.face_area;
			}

		T = T->next;
		}

	return mesh;
	}


static SRBFAN* _srb_to_fan( SRB *srb, SRBPARAMS *params )
	{
	SRBFAN      *fan;
	SRBVERTEX   *V;
	SRBEDGE     *E;
	SRBVERTEX   *D1, *D2;
	int          vertex_count;
	double      *v;
	int          i;
	SRBPYRAMID   P;
	double       min_diameter;
	double       diameter;


	vertex_count = 0;
	V            = srb->vertices->next;

	while( NULL != V )
		{
		/* Convert the point from image space to voxel space. */
		V->x = V->x * params->voxel_width;
		V->y = V->y * params->voxel_height;
		V->z = V->z * params->voxel_length;

		++vertex_count;
		V = V->next;
		}

	if( NULL == ( fan = _srb_fan_new( vertex_count ) ) )
		return NULL;

	/* Convert the origin back to voxel space. */
	params->origin_x = params->origin_x * params->voxel_width;
	params->origin_y = params->origin_y * params->voxel_height;
	params->origin_z = params->origin_z * params->voxel_length;

	/* NOTE: Need to go through the edges since they are in
		order if we want to generate a proper triangle fan. */

	i = 0;
	E = srb->edges->next;

	min_diameter = -1.0;

	while( NULL != E )
		{
		if( E->iteration == srb->iteration )
			{
			v = _SRB_VERTEX( fan->vertices, i );

			v[0] = E->V1->x;
			v[1] = E->V1->y;
			v[2] = E->V1->z;

			E->V1->index = i;

			/* NOTE: Using distance squared to avoid square root. */
			diameter = _SRB_DISTANCE_SQUARED(
							E->V1->x,
							E->V1->y,
							E->V1->z,
							E->O->V2->x,
							E->O->V2->y,
							E->O->V2->z
							);

			if( min_diameter < 0.0 || diameter < min_diameter )
				{
				min_diameter = diameter;

				D1 = E->V1;
				D2 = E->O->V2;
				}

			++i;
			}

		E = E->next;
		}

	fan->diameter = sqrt( min_diameter );

	P.ox = params->origin_x;
	P.oy = params->origin_y;
	P.oz = params->origin_z;

	E = srb->edges->next;

	while( NULL != E )
		{
		if( E->iteration == srb->iteration )
			{
			P.ax = E->V1->x;
			P.ay = E->V1->y;
			P.az = E->V1->z;
			P.bx = E->V2->x;
			P.by = E->V2->y;
			P.bz = E->V2->z;

			_srb_pyramid_face_area( &P );

			fan->area += P.face_area;
			}

		E = E->next;
		}

	return fan;
	}


double _srb_interpolate_3d( SRBPARAMS *params, double x, double y, double z )
	{
    long     lox, loy, loz;
	double  i0, i1, i2, i3, i4, i5, i6, i7;
	double  i01, i23, i45, i67, i0123, i4567, value;
    long     imw, imh, iml, imwh;


	imw  = params->image_width;
	imh  = params->image_height;
	iml  = params->image_length;
	imwh = imw * imh;

	/* Check that point is inside image boundaries. */
	if( x < 0.0 || x > ( imw - 1 ) ||
		 y < 0.0 || y > ( imh - 1 ) ||
		 z < 0.0 || z > ( iml - 1 )   )
		return 0.0;

	/* Get point boundaries. */
    lox = ( long    )x;
    loy = ( long )y;
    loz = ( long )z;

	/****************************************************************
	*  Get intensity at boundaries. Since these are integers there is
	*  not need to add 0.5 to find correct pixel domain.
	*             i1---i5     z
	*            /    /      /
	*          i0---i4 |     --x
	*           |   | i7    |
	*          i2--i6/      y
	*
	*****************************************************************/
	/* Get corner values. */
	i0 = ( double )params->image[ imwh * loz + imw * loy + lox ];
	i1 = ( double )params->image[ imwh * ( loz + 1 ) + imw * loy + lox ];
	i2 = ( double )params->image[ imwh * loz + imw * ( loy + 1 ) + lox ];
	i3 = ( double )params->image[ imwh * ( loz + 1 ) + imw * ( loy + 1 ) + lox ];
	i4 = ( double )params->image[ imwh * loz + imw * loy + ( lox + 1 ) ];
	i5 = ( double )params->image[ imwh * ( loz + 1 ) + imw * loy + ( lox + 1 ) ];
	i6 = ( double )params->image[ imwh * loz + imw * ( loy + 1 ) + ( lox + 1 ) ];
	i7 = ( double )params->image[ imwh * ( loz + 1 ) + imw * ( loy + 1 ) + ( lox + 1 ) ];

	/* Interpolate across z. */
	i01 = ( z - loz ) * ( i1 - i0 ) + i0;
	i23 = ( z - loz ) * ( i3 - i2 ) + i2;
	i45 = ( z - loz ) * ( i5 - i4 ) + i4;
	i67 = ( z - loz ) * ( i7 - i6 ) + i6;

	/* Interpolate across y. */
	i0123 = ( y - loy ) * ( i23 - i01 ) + i01;
	i4567 = ( y - loy ) * ( i67 - i45 ) + i45;

	/* Interpolate across x. */
	value = ( x - lox ) * ( i4567 - i0123 ) + i0123;

	/* Return that value. */
	return value;
	}


double _srb_interpolate_2d( SRBPARAMS *params, double x, double y, double z, int dir )
	{
    long     lox, loy, loz;
	double  i0, i1, i2, i3, i4, i5, i6;
	double  i01, i23, i45, i02, i46, value;
    long     imw, imh, iml, imwh;


	imw  = params->image_width;
	imh  = params->image_height;
	iml  = params->image_length;
	imwh = imw * imh;

	/* Check that point is inside image boundaries. */
	if( x < 0.0 || x > ( imw - 1 ) ||
		 y < 0.0 || y > ( imh - 1 ) ||
		 z < 0.0 || z > ( iml - 1 )   )
		return 0.0;

	/* Get point boundaries. */
    lox = ( long     )x;
    loy = ( long )y;
    loz = ( long )z;

	/****************************************************************
	*  Get intensity at boundaries. Since these are integers there is
	*  not need to add 0.5 to find correct pixel domain.
	*             i1---i5     z
	*            /    /      /
	*          i0---i4 |     --x
	*           |   | i7    |
	*          i2--i6/      y
	*
	*****************************************************************/
	switch( dir )
		{
		case 0:
			/* Get corner values. */
			i0 = ( double )params->image[ imwh * loz + imw * loy + lox ];
			i1 = ( double )params->image[ imwh * ( loz + 1 ) + imw * loy + lox ];
			i2 = ( double )params->image[ imwh * loz + imw * ( loy + 1 ) + lox ];
			i3 = ( double )params->image[ imwh * ( loz + 1 ) + imw * ( loy + 1 ) + lox ];

			/* Interpolate accross z. */
			i01 = ( z - loz ) * ( i1 - i0 ) + i0;
			i23 = ( z - loz ) * ( i3 - i2 ) + i2;

			/* Interpolate accross y. */
			value = ( y - loy ) * ( i23 - i01 ) + i01;
			break;

		case 1:
			/* Get corner values. */
			i0 = ( double )params->image[ imwh * loz + imw * loy + lox ];
			i1 = ( double )params->image[ imwh * ( loz + 1 ) + imw * loy + lox ];
			i4 = ( double )params->image[ imwh * loz + imw * loy + ( lox + 1 ) ];
			i5 = ( double )params->image[ imwh * ( loz + 1 ) + imw * loy + ( lox + 1 ) ];

			/* Interpolate accross z. */
			i01 = ( z - loz ) * ( i1 - i0 ) + i0;
			i45 = ( z - loz ) * ( i5 - i4 ) + i4;

			/* Interpolate across x. */
			value = ( x - lox ) * ( i45 - i01 ) + i01;
			break;

		case 2:
			/* Get corner values. */
			i0 = ( double )params->image[ imwh * loz + imw * loy + lox ];
			i2 = ( double )params->image[ imwh * loz + imw * ( loy + 1 ) + lox ];
			i4 = ( double )params->image[ imwh * loz + imw * loy + ( lox + 1 ) ];
			i6 = ( double )params->image[ imwh * loz + imw * ( loy + 1 ) + ( lox + 1 ) ];

			/* Interpolate accross y. */
			i02 = ( y - loy ) * ( i2 - i0 ) + i0;
			i46 = ( y - loy ) * ( i6 - i4 ) + i4;

			/* Interpolate accross x. */
			value = ( x - lox ) * ( i46 - i02 ) + i02;
			break;
		}

	/* Return that value. */
	return value;
	}


void _srb_cast_ray( SRBRAY *ray, SRBPARAMS *params )
	{
	double  x, y, z, val;
	double  newx, newy, newz, newval;
	double  i, j, k, tx, ty, tz, tmin;
	int     which;


	/* Set current position of ray. */
	x = ray->ox;
	y = ray->oy;
	z = ray->oz;

	/* Compute first intensity using trilinear interpolation. */
	val = _srb_interpolate_3d( params, x, y, z );

	/* If intensity lower than threshold return origin as end. */
	if( val < params->threshold )
		{
		ray->ex = ray->ox;
		ray->ey = ray->oy;
		ray->ez = ray->oz;

		return;
		}

	while( 1 )
		{
		/* Get next face of exit. */
		i = ( ray->dx < 0.0 ) ? ceil( x - 1.0 ) : floor( x + 1.0 );
		j = ( ray->dy < 0.0 ) ? ceil( y - 1.0 ) : floor( y + 1.0 );
		k = ( ray->dz < 0.0 ) ? ceil( z - 1.0 ) : floor( z + 1.0 );

		/* Get time of intersection. */
		tx = ( i - x ) / ray->dx;
		ty = ( j - y ) / ray->dy;
		tz = ( k - z ) / ray->dz;

		which = _SRB_MIN3( tx, ty, tz );

		/* Interpolate at face. */
		switch( which )
			{
			case 0: tmin = tx; break;
			case 1: tmin = ty; break;
			case 2: tmin = tz; break;
			}

		newx = x + tmin * ray->dx;
		newy = y + tmin * ray->dy;
		newz = z + tmin * ray->dz;

		newval = _srb_interpolate_2d( params, newx, newy, newz, which );

		/* If intensity lower interpolate linearly and break. */
		if( newval < params->threshold )
			{
			ray->ex = ( params->threshold - val ) * ( newx - x ) / ( newval - val ) + x;
			ray->ey = ( params->threshold - val ) * ( newy - y ) / ( newval - val ) + y;
			ray->ez = ( params->threshold - val ) * ( newz - z ) / ( newval - val ) + z;

			return;
			}

		/* Advance position. */
		x   = newx;
		y   = newy;
		z   = newz;
		val = newval;
		}
	}


static void _srb_initial_cast( SRB *srb, SRBPARAMS *params )
	{
	SRBVERTEX  *V;
	SRBRAY      R;


	R.ox = params->origin_x;
	R.oy = params->origin_y;
	R.oz = params->origin_z;

	V = srb->vertices->next;

	while( NULL != V )
		{
		/* First case is special case. The vertices
			position is the ray's direction. */
		R.dx = V->x;
		R.dy = V->y;
		R.dz = V->z;

		_srb_cast_ray( &R, params );

		V->x = R.ex;
		V->y = R.ey;
		V->z = R.ez;

		V = V->next;
		}
	}


static int _srb_cast_2d( SRB *srb, SRBPARAMS *params )
	{
	SRBEDGE  *E;
	SRBRAY    R;


	++(srb->iteration);
	srb->tail = NULL;

	/* First tesselate all the edges. */

	E = srb->edges->next;

	while( NULL != E )
		{
		if( E->iteration == srb->iteration - 1 )
			if( ! _srb_tesselate_2d( srb, E, params ) )
				return 0;

		E = E->next;
		}

	/* Now cast the midpoints. */

	R.ox = params->origin_x;
	R.oy = params->origin_y;
	R.oz = params->origin_z;

	E = srb->edges->next;

	while( NULL != E )
		{
		if( E->iteration == srb->iteration - 1 )
			{
			/* In 2D the midpoint is already a direction vector. */
			R.dx = E->M->x;
			R.dy = E->M->y;
			R.dz = E->M->z;

			_srb_cast_ray( &R, params );

			E->M->x = R.ex;
			E->M->y = R.ey;
			E->M->z = R.ez;
			}

		E = E->next;
		}

	return 1;
	}


static int _srb_cast_3d( SRB *srb, SRBPARAMS *params )
	{
	SRBTRIANGLE  *T;
	SRBEDGE      *E;
	SRBRAY        R;


	++(srb->iteration);

	/* First tesselate all the triangles. */

	T = srb->triangles->next;

	while( NULL != T )
		{
		if( T->iteration == srb->iteration - 1 )
			if( ! _srb_tesselate_3d( srb, T ) )
				return 0;

		T = T->next;
		}

	/* Now cast the midpoints. */

	R.ox = params->origin_x;
	R.oy = params->origin_y;
	R.oz = params->origin_z;

	E = srb->edges->next;

	while( NULL != E )
		{
		if( E->iteration == srb->iteration - 1 )
			{
			/* Convert the midpoint to a direction vector. */
			R.dx = E->M->x - R.ox;
			R.dy = E->M->y - R.oy;
			R.dz = E->M->z - R.oz;

			_srb_cast_ray( &R, params );

			E->M->x = R.ex;
			E->M->y = R.ey;
			E->M->z = R.ez;
			}

		E = E->next;
		}

	return 1;
	}


static void _srb_tolerance_cast
	(
	SRB        *srb,
	SRBPARAMS  *params,
	double      x,
	double      y,
	double      z,
	double     *sum_dm,
	double     *sum_dc_minus_dm
	)
	{
	SRBRAY  R;
	double  dc, dm, dc_minus_dm;


	R.ox = params->origin_x;
	R.oy = params->origin_y;
	R.oz = params->origin_z;

	/* Distance to point. */
	dm = sqrt( _SRB_DISTANCE_SQUARED( R.ox, R.oy, R.oz, x, y, z ) );

	R.dx = x - R.ox;
	R.dy = y - R.oy;
	R.dz = z - R.oz;

	_srb_cast_ray( &R, params );

	/* Distance of the casted ray. */
	dc = sqrt( _SRB_DISTANCE_SQUARED( R.ox, R.oy, R.oz, R.ex, R.ey, R.ez ) );

	(*sum_dm) += dm;

	dc_minus_dm = dc - dm;

	if( dc_minus_dm < 0.0 )
		dc_minus_dm = -dc_minus_dm;

	(*sum_dc_minus_dm) += dc_minus_dm;
	}


static int _srb_has_converged
	(
	SRB        *srb,
	SRBPARAMS  *params,
	double      sum_dm,
	double      sum_dc_minus_dm
	)
	{
	double f = ( sum_dc_minus_dm / sum_dm ) * 100.0;

	if( f < 0.0 )
		f = -f;

	return f <= params->fit_percent;
	}


static int _srb_has_converged_3d( SRB *srb, SRBPARAMS *params )
	{
	SRBTRIANGLE  *T;
	double        x, y, z;
	double        sum_dm, sum_dc_minus_dm;


	sum_dm = sum_dc_minus_dm = 0.0;

	T = srb->triangles->next;

	while( NULL != T )
		{
		if( T->iteration == srb->iteration )
			{
			_SRB_TRIANGLE_CENTER( T, x, y, z );
			_srb_tolerance_cast( srb, params, x, y, z, &sum_dm, &sum_dc_minus_dm );
			}

		T = T->next;
		}

	return _srb_has_converged( srb, params, sum_dm, sum_dc_minus_dm );
	}


static int _srb_has_converged_2d( SRB *srb, SRBPARAMS *params )
	{
	SRBEDGE  *E;
	double    x, y, z;
	double    sum_dm, sum_dc_minus_dm;


	sum_dm = sum_dc_minus_dm = 0.0;

	E = srb->edges->next;

	while( NULL != E )
		{
		if( E->iteration == srb->iteration )
			{
			_SRB_EDGE_MIDPOINT( E, x, y, z );
			_srb_tolerance_cast( srb, params, x, y, z, &sum_dm, &sum_dc_minus_dm );
			}

		E = E->next;
		}

	return _srb_has_converged( srb, params, sum_dm, sum_dc_minus_dm );
	}


static double ____srb_octahedron_vertices[ 6 * 3 ] =
	{ 
	1.0, 0.0, 0.0,
	0.0, 1.0, 0.0,
	-1.0, 0.0, 0.0,
	0.0, -1.0, 0.0,
	0.0, 0.0, 1.0,
	0.0, 0.0, -1.0
	};


/* NOTE: These are vertex indices. */
static int ____srb_octahedron_triangles[ 8 * 3 ] =
	{ 
	0, 1, 4,
	1, 2, 4,
	2, 3, 4,
	3, 0, 4,
	1, 0, 5,
	2, 1, 5,
	3, 2, 5,
	0, 3, 5
	};


SRBMESH* srb_run_3d( SRBPARAMS *params )
	{
	SRB       srb;
	SRBMESH  *mesh;
	int       i;


	if( ! _srb_init_3d(
				&srb,
				params,
				____srb_octahedron_vertices,
				_SRB_ARRAY_LENGTH( ____srb_octahedron_vertices ) / 3,
				____srb_octahedron_triangles,
				_SRB_ARRAY_LENGTH( ____srb_octahedron_triangles ) / 3
				) )
		{
		_srb_finalize( &srb );
		return NULL;
		}

	_srb_initial_cast( &srb, params );

	for( i = 1; i <= params->max_iterations; ++i )
		{
		if( ! _srb_cast_3d( &srb, params ) )
			{
			_srb_finalize( &srb );
			return NULL;
			}

		if( _srb_has_converged_3d( &srb, params ) )
			break;
		}

	mesh = _srb_to_mesh( &srb, params );
	_srb_finalize( &srb );

	return mesh;
	}


/* NOTE: Its critical that the vertices come in clockwise or
	counter-clockwise order! */
static double ____srb_quadrilateral_vertices[ 4 * 3 ] =
	{ 
	1.0, 0.0, 0.0,
	0.0, 1.0, 0.0,
	-1.0, 0.0, 0.0,
	0.0, -1.0, 0.0
	};


SRBFAN* srb_run_2d( SRBPARAMS *params )
	{
	SRB      srb;
	SRBFAN  *fan;
	int      i;


	if( ! _srb_init_2d(
				&srb,
				params,
				____srb_quadrilateral_vertices,
				_SRB_ARRAY_LENGTH( ____srb_quadrilateral_vertices ) / 3
				) )
		{
		_srb_finalize( &srb );
		return NULL;
		}

	_srb_initial_cast( &srb, params );

	for( i = 1; i <= params->max_iterations; ++i )
		{
		if( ! _srb_cast_2d( &srb, params ) )
			{
			_srb_finalize( &srb );
			return NULL;
			}

		if( _srb_has_converged_2d( &srb, params ) )
			break;
		}

	fan = _srb_to_fan( &srb, params );
	_srb_finalize( &srb );

	return fan;
	}


void srb_mesh_destroy( SRBMESH *mesh )
	{  _srb_mesh_delete( mesh );  }


void srb_fan_destroy( SRBFAN *fan )
	{  _srb_fan_delete( fan );  }
