#include "nsvector.h"


#define _NS_VECTOR2( _V, _op, _x, _y )\
	( _V )->x _op ( _x );\
	( _V )->y _op ( _y );\
	return ( _V )

#define _NS_VECTOR3( _V, _op, _x, _y, _z )\
	( _V )->x _op ( _x );\
	( _V )->y _op ( _y );\
	( _V )->z _op ( _z );\
	return ( _V )

#define _NS_VECTOR4( _V, _op, _x, _y, _z, _w )\
	( _V )->x _op ( _x );\
	( _V )->y _op ( _y );\
	( _V )->z _op ( _z );\
	( _V )->w _op ( _w );\
	return ( _V )


#define _NS_VECTOR2_INIT( V, x, y )\
	ns_assert( NULL != ( V ) );\
	_NS_VECTOR2( ( V ), =, ( x ), ( y ) )

#define _NS_VECTOR3_INIT( V, x, y, z )\
	ns_assert( NULL != ( V ) );\
	_NS_VECTOR3( ( V ), =, ( x ), ( y ), ( z ) )\

#define _NS_VECTOR4_INIT( V, x, y, z, w )\
	ns_assert( NULL != ( V ) );\
	_NS_VECTOR4( ( V ), =, ( x ), ( y ), ( z ), ( w ) )\

NsVector2d* ns_vector2d( NsVector2d *V, nsdouble x, nsdouble y )
	{  _NS_VECTOR2_INIT( V, x, y );  }

NsVector3d* ns_vector3d( NsVector3d *V, nsdouble x, nsdouble y, nsdouble z )
	{  _NS_VECTOR3_INIT( V, x, y, z );  }

NsVector4d* ns_vector4d( NsVector4d *V, nsdouble x, nsdouble y, nsdouble z, nsdouble w )
	{  _NS_VECTOR4_INIT( V, x, y, z, w );  }

NsVector2f* ns_vector2f( NsVector2f *V, nsfloat x, nsfloat y )
	{  _NS_VECTOR2_INIT( V, x, y );  }

NsVector3f* ns_vector3f( NsVector3f *V, nsfloat x, nsfloat y, nsfloat z )
	{  _NS_VECTOR3_INIT( V, x, y, z );  }

NsVector4f* ns_vector4f( NsVector4f *V, nsfloat x, nsfloat y, nsfloat z, nsfloat w )
	{  _NS_VECTOR4_INIT( V, x, y, z, w );  }

NsVector2l* ns_vector2l( NsVector2l *V, nslong x, nslong y )
	{  _NS_VECTOR2_INIT( V, x, y );  }

NsVector3l* ns_vector3l( NsVector3l *V, nslong x, nslong y, nslong z )
	{  _NS_VECTOR3_INIT( V, x, y, z );  }

NsVector4l* ns_vector4l( NsVector4l *V, nslong x, nslong y, nslong z, nslong w )
	{  _NS_VECTOR4_INIT( V, x, y, z, w );  }

NsVector2i* ns_vector2i( NsVector2i *V, nsint x, nsint y )
	{  _NS_VECTOR2_INIT( V, x, y );  }

NsVector3i* ns_vector3i( NsVector3i *V, nsint x, nsint y, nsint z )
	{  _NS_VECTOR3_INIT( V, x, y, z );  }

NsVector4i* ns_vector4i( NsVector4i *V, nsint x, nsint y, nsint z, nsint w )
	{  _NS_VECTOR4_INIT( V, x, y, z, w );  }

NsVector2s* ns_vector2s( NsVector2s *V, nsshort x, nsshort y )
	{  _NS_VECTOR2_INIT( V, x, y );  }

NsVector3s* ns_vector3s( NsVector3s *V, nsshort x, nsshort y, nsshort z )
	{  _NS_VECTOR3_INIT( V, x, y, z );  }

NsVector4s* ns_vector4s( NsVector4s *V, nsshort x, nsshort y, nsshort z, nsshort w )
	{  _NS_VECTOR4_INIT( V, x, y, z, w );  }


#define _NS_VECTOR_ARRAY( V, type )\
	ns_assert( NULL != ( V ) );\
	return ( type * )( V )

nsdouble* ns_vector2d_array( NsVector2d *V )
	{  _NS_VECTOR_ARRAY( V, nsdouble );  }

nsdouble* ns_vector3d_array( NsVector3d *V )
	{  _NS_VECTOR_ARRAY( V, nsdouble );  }

nsdouble* ns_vector4d_array( NsVector4d *V )
	{  _NS_VECTOR_ARRAY( V, nsdouble );  }

nsfloat* ns_vector2f_array( NsVector2f *V )
	{  _NS_VECTOR_ARRAY( V, nsfloat );  }

nsfloat* ns_vector3f_array( NsVector3f *V )
	{  _NS_VECTOR_ARRAY( V, nsfloat );  }

nsfloat* ns_vector4f_array( NsVector4f *V )
	{  _NS_VECTOR_ARRAY( V, nsfloat );  }

nslong* ns_vector2l_array( NsVector2l *V )
	{  _NS_VECTOR_ARRAY( V, nslong );  }

nslong* ns_vector3l_array( NsVector3l *V )
	{  _NS_VECTOR_ARRAY( V, nslong );  }

nslong* ns_vector4l_array( NsVector4l *V )
	{  _NS_VECTOR_ARRAY( V, nslong );  }

nsint* ns_vector2i_array( NsVector2i *V )
	{  _NS_VECTOR_ARRAY( V, nsint );  }

nsint* ns_vector3i_array( NsVector3i *V )
	{  _NS_VECTOR_ARRAY( V, nsint );  }

nsint* ns_vector4i_array( NsVector4i *V )
	{  _NS_VECTOR_ARRAY( V, nsint );  }

nsshort* ns_vector2s_array( NsVector2s *V )
	{  _NS_VECTOR_ARRAY( V, nsshort );  }

nsshort* ns_vector3s_array( NsVector3s *V )
	{  _NS_VECTOR_ARRAY( V, nsshort );  }

nsshort* ns_vector4s_array( NsVector4s *V )
	{  _NS_VECTOR_ARRAY( V, nsshort );  }

const nsdouble* ns_vector2d_const_array( const NsVector2d *V )
	{  _NS_VECTOR_ARRAY( V, const nsdouble );  }

const nsdouble* ns_vector3d_const_array( const NsVector3d *V )
	{  _NS_VECTOR_ARRAY( V, const nsdouble );  }

const nsdouble* ns_vector4d_const_array( const NsVector4d *V )
	{  _NS_VECTOR_ARRAY( V, const nsdouble );  }

const nsfloat* ns_vector2f_const_array( const NsVector2f *V )
	{  _NS_VECTOR_ARRAY( V, const nsfloat );  }

const nsfloat* ns_vector3f_const_array( const NsVector3f *V )
	{  _NS_VECTOR_ARRAY( V, const nsfloat );  }

const nsfloat* ns_vector4f_const_array( const NsVector4f *V )
	{  _NS_VECTOR_ARRAY( V, const nsfloat );  }

const nslong* ns_vector2l_const_array( const NsVector2l *V )
	{  _NS_VECTOR_ARRAY( V, const nslong );  }

const nslong* ns_vector3l_const_array( const NsVector3l *V )
	{  _NS_VECTOR_ARRAY( V, const nslong );  }

const nslong* ns_vector4l_const_array( const NsVector4l *V )
	{  _NS_VECTOR_ARRAY( V, const nslong );  }

const nsint* ns_vector2i_const_array( const NsVector2i *V )
	{  _NS_VECTOR_ARRAY( V, const nsint );  }

const nsint* ns_vector3i_const_array( const NsVector3i *V )
	{  _NS_VECTOR_ARRAY( V, const nsint );  }

const nsint* ns_vector4i_const_array( const NsVector4i *V )
	{  _NS_VECTOR_ARRAY( V, const nsint );  }

const nsshort* ns_vector2s_const_array( const NsVector2s *V )
	{  _NS_VECTOR_ARRAY( V, const nsshort );  }

const nsshort* ns_vector3s_const_array( const NsVector3s *V )
	{  _NS_VECTOR_ARRAY( V, const nsshort );  }

const nsshort* ns_vector4s_const_array( const NsVector4s *V )
	{  _NS_VECTOR_ARRAY( V, const nsshort );  }


#define _NS_VECTOR2_ADD( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR2( (C), =, (A)->x + (B)->x, (A)->y + (B)->y )

#define _NS_VECTOR3_ADD( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR3( (C), =, (A)->x + (B)->x, (A)->y + (B)->y, (A)->z + (B)->z )

#define _NS_VECTOR4_ADD( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR4( (C), =, (A)->x + (B)->x, (A)->y + (B)->y, (A)->z + (B)->z, (A)->w + (B)->w )

NsVector2d* ns_vector2d_add( NsVector2d *C, const NsVector2d *A, const NsVector2d *B )
   {  _NS_VECTOR2_ADD( C, A, B );  }

NsVector3d* ns_vector3d_add( NsVector3d *C, const NsVector3d *A, const NsVector3d *B )
   {  _NS_VECTOR3_ADD( C, A, B );  }

NsVector4d* ns_vector4d_add( NsVector4d *C, const NsVector4d *A, const NsVector4d *B )
   {  _NS_VECTOR4_ADD( C, A, B );  }

NsVector2f* ns_vector2f_add( NsVector2f *C, const NsVector2f *A, const NsVector2f *B )
   {  _NS_VECTOR2_ADD( C, A, B );  }

NsVector3f* ns_vector3f_add( NsVector3f *C, const NsVector3f *A, const NsVector3f *B )
   {  _NS_VECTOR3_ADD( C, A, B );  }

NsVector4f* ns_vector4f_add( NsVector4f *C, const NsVector4f *A, const NsVector4f *B )
   {  _NS_VECTOR4_ADD( C, A, B );  }

NsVector2l* ns_vector2l_add( NsVector2l *C, const NsVector2l *A, const NsVector2l *B )
   {  _NS_VECTOR2_ADD( C, A, B );  }

NsVector3l* ns_vector3l_add( NsVector3l *C, const NsVector3l *A, const NsVector3l *B )
   {  _NS_VECTOR3_ADD( C, A, B );  }

NsVector4l* ns_vector4l_add( NsVector4l *C, const NsVector4l *A, const NsVector4l *B )
   {  _NS_VECTOR4_ADD( C, A, B );  }

NsVector2i* ns_vector2i_add( NsVector2i *C, const NsVector2i *A, const NsVector2i *B )
   {  _NS_VECTOR2_ADD( C, A, B );  }

NsVector3i* ns_vector3i_add( NsVector3i *C, const NsVector3i *A, const NsVector3i *B )
   {  _NS_VECTOR3_ADD( C, A, B );  }

NsVector4i* ns_vector4i_add( NsVector4i *C, const NsVector4i *A, const NsVector4i *B )
   {  _NS_VECTOR4_ADD( C, A, B );  }

NsVector2s* ns_vector2s_add( NsVector2s *C, const NsVector2s *A, const NsVector2s *B )
   {  _NS_VECTOR2_ADD( C, A, B );  }

NsVector3s* ns_vector3s_add( NsVector3s *C, const NsVector3s *A, const NsVector3s *B )
   {  _NS_VECTOR3_ADD( C, A, B );  }

NsVector4s* ns_vector4s_add( NsVector4s *C, const NsVector4s *A, const NsVector4s *B )
   {  _NS_VECTOR4_ADD( C, A, B );  }


#define _NS_VECTOR2_CMPD_ADD( A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	_NS_VECTOR2( (A), =, (A)->x + (B)->x, (A)->y + (B)->y )

#define _NS_VECTOR3_CMPD_ADD( A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	_NS_VECTOR3( (A), =, (A)->x + (B)->x, (A)->y + (B)->y, (A)->z + (B)->z )

#define _NS_VECTOR4_CMPD_ADD( A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	_NS_VECTOR4( (A), =, (A)->x + (B)->x, (A)->y + (B)->y, (A)->z + (B)->z, (A)->w + (B)->w )

NsVector2d* ns_vector2d_cmpd_add( NsVector2d *A, const NsVector2d *B )
   {  _NS_VECTOR2_CMPD_ADD( A, B );  }

NsVector3d* ns_vector3d_cmpd_add( NsVector3d *A, const NsVector3d *B )
   {  _NS_VECTOR3_CMPD_ADD( A, B );  }

NsVector4d* ns_vector4d_cmpd_add( NsVector4d *A, const NsVector4d *B )
   {  _NS_VECTOR4_CMPD_ADD( A, B );  }

NsVector2f* ns_vector2f_cmpd_add( NsVector2f *A, const NsVector2f *B )
   {  _NS_VECTOR2_CMPD_ADD( A, B );  }

NsVector3f* ns_vector3f_cmpd_add( NsVector3f *A, const NsVector3f *B )
   {  _NS_VECTOR3_CMPD_ADD( A, B );  }

NsVector4f* ns_vector4f_cmpd_add( NsVector4f *A, const NsVector4f *B )
   {  _NS_VECTOR4_CMPD_ADD( A, B );  }

NsVector2l* ns_vector2l_cmpd_add( NsVector2l *A, const NsVector2l *B )
   {  _NS_VECTOR2_CMPD_ADD( A, B );  }

NsVector3l* ns_vector3l_cmpd_add( NsVector3l *A, const NsVector3l *B )
   {  _NS_VECTOR3_CMPD_ADD( A, B );  }

NsVector4l* ns_vector4l_cmpd_add( NsVector4l *A, const NsVector4l *B )
   {  _NS_VECTOR4_CMPD_ADD( A, B );  }

NsVector2i* ns_vector2i_cmpd_add( NsVector2i *A, const NsVector2i *B )
   {  _NS_VECTOR2_CMPD_ADD( A, B );  }

NsVector3i* ns_vector3i_cmpd_add( NsVector3i *A, const NsVector3i *B )
   {  _NS_VECTOR3_CMPD_ADD( A, B );  }

NsVector4i* ns_vector4i_cmpd_add( NsVector4i *A, const NsVector4i *B )
	{  _NS_VECTOR4_CMPD_ADD( A, B );  }

NsVector2s* ns_vector2s_cmpd_add( NsVector2s *A, const NsVector2s *B )
   {  _NS_VECTOR2_CMPD_ADD( A, B );  }

NsVector3s* ns_vector3s_cmpd_add( NsVector3s *A, const NsVector3s *B )
   {  _NS_VECTOR3_CMPD_ADD( A, B );  }

NsVector4s* ns_vector4s_cmpd_add( NsVector4s *A, const NsVector4s *B )
   {  _NS_VECTOR4_CMPD_ADD( A, B );  }


#define _NS_VECTOR2_ADD_COMPS( C, A, _x, _y )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR2( (C), =, (A)->x + (_x), (A)->y + (_y) )

#define _NS_VECTOR3_ADD_COMPS( C, A, _x, _y, _z )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR3( (C), =, (A)->x + (_x), (A)->y + (_y), (A)->z + (_z) )

#define _NS_VECTOR4_ADD_COMPS( C, A, _x, _y, _z, _w )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR4( (C), =, (A)->x + (_x), (A)->y + (_y), (A)->z + (_z), (A)->w + (_w) )

NsVector2d* ns_vector2d_add_xy( NsVector2d *C, const NsVector2d *A, nsdouble x, nsdouble y )
   {  _NS_VECTOR2_ADD_COMPS( C, A, x, y );  }

NsVector3d* ns_vector3d_add_xyz( NsVector3d *C, const NsVector3d *A, nsdouble x, nsdouble y, nsdouble z )
   {  _NS_VECTOR3_ADD_COMPS( C, A, x, y, z );  }

NsVector4d* ns_vector4d_add_xyzw( NsVector4d *C, const NsVector4d *A, nsdouble x, nsdouble y, nsdouble z, nsdouble w )
   {  _NS_VECTOR4_ADD_COMPS( C, A, x, y, z, w );  }

NsVector2f* ns_vector2f_add_xy( NsVector2f *C, const NsVector2f *A, nsfloat x, nsfloat y )
   {  _NS_VECTOR2_ADD_COMPS( C, A, x, y );  }

NsVector3f* ns_vector3f_add_xyz( NsVector3f *C, const NsVector3f *A, nsfloat x, nsfloat y, nsfloat z )
   {  _NS_VECTOR3_ADD_COMPS( C, A, x, y, z );  }

NsVector4f* ns_vector4f_add_xyzw( NsVector4f *C, const NsVector4f *A, nsfloat x, nsfloat y, nsfloat z, nsfloat w )
   {  _NS_VECTOR4_ADD_COMPS( C, A, x, y, z, w );  }

NsVector2l* ns_vector2l_add_xy( NsVector2l *C, const NsVector2l *A, nslong x, nslong y )
   {  _NS_VECTOR2_ADD_COMPS( C, A, x, y );  }

NsVector3l* ns_vector3l_add_xyz( NsVector3l *C, const NsVector3l *A, nslong x, nslong y, nslong z )
   {  _NS_VECTOR3_ADD_COMPS( C, A, x, y, z );  }

NsVector4l* ns_vector4l_add_xyzw( NsVector4l *C, const NsVector4l *A, nslong x, nslong y, nslong z, nslong w )
   {  _NS_VECTOR4_ADD_COMPS( C, A, x, y, z, w );  }

NsVector2i* ns_vector2i_add_xy( NsVector2i *C, const NsVector2i *A, nsint x, nsint y )
   {  _NS_VECTOR2_ADD_COMPS( C, A, x, y );  }

NsVector3i* ns_vector3i_add_xyz( NsVector3i *C, const NsVector3i *A, nsint x, nsint y, nsint z )
   {  _NS_VECTOR3_ADD_COMPS( C, A, x, y, z );  }

NsVector4i* ns_vector4i_add_xyzw( NsVector4i *C, const NsVector4i *A, nsint x, nsint y, nsint z, nsint w )
   {  _NS_VECTOR4_ADD_COMPS( C, A, x, y, z, w );  }

NsVector2s* ns_vector2s_add_xy( NsVector2s *C, const NsVector2s *A, nsshort x, nsshort y )
   {  _NS_VECTOR2_ADD_COMPS( C, A, x, y );  }

NsVector3s* ns_vector3s_add_xyz( NsVector3s *C, const NsVector3s *A, nsshort x, nsshort y, nsshort z )
   {  _NS_VECTOR3_ADD_COMPS( C, A, x, y, z );  }

NsVector4s* ns_vector4s_add_xyzw( NsVector4s *C, const NsVector4s *A, nsshort x, nsshort y, nsshort z, nsshort w )
   {  _NS_VECTOR4_ADD_COMPS( C, A, x, y, z, w );  }


#define _NS_VECTOR2_CMPD_ADD_COMPS( V, _x, _y )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR2( (V), =, (V)->x + (_x), (V)->y + (_y) )

#define _NS_VECTOR3_CMPD_ADD_COMPS( V, _x, _y, _z )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR3( (V), =, (V)->x + (_x), (V)->y + (_y), (V)->z + (_z) )

#define _NS_VECTOR4_CMPD_ADD_COMPS( V, _x, _y, _z, _w )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR4( (V), =, (V)->x + (_x), (V)->y + (_y), (V)->z + (_z), (V)->w + (_w) )

NsVector2d* ns_vector2d_cmpd_add_xy( NsVector2d *V, nsdouble x, nsdouble y )
   {  _NS_VECTOR2_CMPD_ADD_COMPS( V, x, y );  }

NsVector3d* ns_vector3d_cmpd_add_xyz( NsVector3d *V, nsdouble x, nsdouble y, nsdouble z )
   {  _NS_VECTOR3_CMPD_ADD_COMPS( V, x, y, z );  }

NsVector4d* ns_vector4d_cmpd_add_xyzw( NsVector4d *V, nsdouble x, nsdouble y, nsdouble z, nsdouble w )
   {  _NS_VECTOR4_CMPD_ADD_COMPS( V, x, y, z, w );  }

NsVector2f* ns_vector2f_cmpd_add_xy( NsVector2f *V, nsfloat x, nsfloat y )
   {  _NS_VECTOR2_CMPD_ADD_COMPS( V, x, y );  }

NsVector3f* ns_vector3f_cmpd_add_xyz( NsVector3f *V, nsfloat x, nsfloat y, nsfloat z )
   {  _NS_VECTOR3_CMPD_ADD_COMPS( V, x, y, z );  }

NsVector4f* ns_vector4f_cmpd_add_xyzw( NsVector4f *V, nsfloat x, nsfloat y, nsfloat z, nsfloat w )
   {  _NS_VECTOR4_CMPD_ADD_COMPS( V, x, y, z, w );  }

NsVector2l* ns_vector2l_cmpd_add_xy( NsVector2l *V, nslong x, nslong y )
   {  _NS_VECTOR2_CMPD_ADD_COMPS( V, x, y );  }

NsVector3l* ns_vector3l_cmpd_add_xyz( NsVector3l *V, nslong x, nslong y, nslong z )
   {  _NS_VECTOR3_CMPD_ADD_COMPS( V, x, y, z );  }

NsVector4l* ns_vector4l_cmpd_add_xyzw( NsVector4l *V, nslong x, nslong y, nslong z, nslong w )
   {  _NS_VECTOR4_CMPD_ADD_COMPS( V, x, y, z, w );  }

NsVector2i* ns_vector2i_cmpd_add_xy( NsVector2i *V, nsint x, nsint y )
   {  _NS_VECTOR2_CMPD_ADD_COMPS( V, x, y );  }

NsVector3i* ns_vector3i_cmpd_add_xyz( NsVector3i *V, nsint x, nsint y, nsint z )
   {  _NS_VECTOR3_CMPD_ADD_COMPS( V, x, y, z );  }

NsVector4i* ns_vector4i_cmpd_add_xyzw( NsVector4i *V, nsint x, nsint y, nsint z, nsint w )
   {  _NS_VECTOR4_CMPD_ADD_COMPS( V, x, y, z, w );  }

NsVector2s* ns_vector2s_cmpd_add_xy( NsVector2s *V, nsshort x, nsshort y )
   {  _NS_VECTOR2_CMPD_ADD_COMPS( V, x, y );  }

NsVector3s* ns_vector3s_cmpd_add_xyz( NsVector3s *V, nsshort x, nsshort y, nsshort z )
   {  _NS_VECTOR3_CMPD_ADD_COMPS( V, x, y, z );  }

NsVector4s* ns_vector4s_cmpd_add_xyzw( NsVector4s *V, nsshort x, nsshort y, nsshort z, nsshort w )
   {  _NS_VECTOR4_CMPD_ADD_COMPS( V, x, y, z, w );  }


#define _NS_VECTOR2_SUB( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR2( (C), =, (A)->x - (B)->x, (A)->y - (B)->y )

#define _NS_VECTOR3_SUB( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR3( (C), =, (A)->x - (B)->x, (A)->y - (B)->y, (A)->z - (B)->z )

#define _NS_VECTOR4_SUB( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR4( (C), =, (A)->x - (B)->x, (A)->y - (B)->y, (A)->z - (B)->z, (A)->w - (B)->w )

NsVector2d* ns_vector2d_sub( NsVector2d *C, const NsVector2d *A, const NsVector2d *B )
   {  _NS_VECTOR2_SUB( C, A, B );  }

NsVector3d* ns_vector3d_sub( NsVector3d *C, const NsVector3d *A, const NsVector3d *B )
   {  _NS_VECTOR3_SUB( C, A, B );  }

NsVector4d* ns_vector4d_sub( NsVector4d *C, const NsVector4d *A, const NsVector4d *B )
   {  _NS_VECTOR4_SUB( C, A, B );  }

NsVector2f* ns_vector2f_sub( NsVector2f *C, const NsVector2f *A, const NsVector2f *B )
   {  _NS_VECTOR2_SUB( C, A, B );  }

NsVector3f* ns_vector3f_sub( NsVector3f *C, const NsVector3f *A, const NsVector3f *B )
   {  _NS_VECTOR3_SUB( C, A, B );  }

NsVector4f* ns_vector4f_sub( NsVector4f *C, const NsVector4f *A, const NsVector4f *B )
   {  _NS_VECTOR4_SUB( C, A, B );  }

NsVector2l* ns_vector2l_sub( NsVector2l *C, const NsVector2l *A, const NsVector2l *B )
   {  _NS_VECTOR2_SUB( C, A, B );  }

NsVector3l* ns_vector3l_sub( NsVector3l *C, const NsVector3l *A, const NsVector3l *B )
   {  _NS_VECTOR3_SUB( C, A, B );  }

NsVector4l* ns_vector4l_sub( NsVector4l *C, const NsVector4l *A, const NsVector4l *B )
   {  _NS_VECTOR4_SUB( C, A, B );  }

NsVector2i* ns_vector2i_sub( NsVector2i *C, const NsVector2i *A, const NsVector2i *B )
   {  _NS_VECTOR2_SUB( C, A, B );  }

NsVector3i* ns_vector3i_sub( NsVector3i *C, const NsVector3i *A, const NsVector3i *B )
   {  _NS_VECTOR3_SUB( C, A, B );  }

NsVector4i* ns_vector4i_sub( NsVector4i *C, const NsVector4i *A, const NsVector4i *B )
   {  _NS_VECTOR4_SUB( C, A, B );  }

NsVector2s* ns_vector2s_sub( NsVector2s *C, const NsVector2s *A, const NsVector2s *B )
   {  _NS_VECTOR2_SUB( C, A, B );  }

NsVector3s* ns_vector3s_sub( NsVector3s *C, const NsVector3s *A, const NsVector3s *B )
   {  _NS_VECTOR3_SUB( C, A, B );  }

NsVector4s* ns_vector4s_sub( NsVector4s *C, const NsVector4s *A, const NsVector4s *B )
   {  _NS_VECTOR4_SUB( C, A, B );  }


#define _NS_VECTOR2_CMPD_SUB( A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	_NS_VECTOR2( (A), =, (A)->x - (B)->x, (A)->y - (B)->y )

#define _NS_VECTOR3_CMPD_SUB( A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	_NS_VECTOR3( (A), =, (A)->x - (B)->x, (A)->y - (B)->y, (A)->z - (B)->z )

#define _NS_VECTOR4_CMPD_SUB( A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	_NS_VECTOR4( (A), =, (A)->x - (B)->x, (A)->y - (B)->y, (A)->z - (B)->z, (A)->w - (B)->w )

NsVector2d* ns_vector2d_cmpd_sub( NsVector2d *A, const NsVector2d *B )
   {  _NS_VECTOR2_CMPD_SUB( A, B );  }

NsVector3d* ns_vector3d_cmpd_sub( NsVector3d *A, const NsVector3d *B )
   {  _NS_VECTOR3_CMPD_SUB( A, B );  }

NsVector4d* ns_vector4d_cmpd_sub( NsVector4d *A, const NsVector4d *B )
   {  _NS_VECTOR4_CMPD_SUB( A, B );  }

NsVector2f* ns_vector2f_cmpd_sub( NsVector2f *A, const NsVector2f *B )
   {  _NS_VECTOR2_CMPD_SUB( A, B );  }

NsVector3f* ns_vector3f_cmpd_sub( NsVector3f *A, const NsVector3f *B )
   {  _NS_VECTOR3_CMPD_SUB( A, B );  }

NsVector4f* ns_vector4f_cmpd_sub( NsVector4f *A, const NsVector4f *B )
   {  _NS_VECTOR4_CMPD_SUB( A, B );  }

NsVector2l* ns_vector2l_cmpd_sub( NsVector2l *A, const NsVector2l *B )
   {  _NS_VECTOR2_CMPD_SUB( A, B );  }

NsVector3l* ns_vector3l_cmpd_sub( NsVector3l *A, const NsVector3l *B )
   {  _NS_VECTOR3_CMPD_SUB( A, B );  }

NsVector4l* ns_vector4l_cmpd_sub( NsVector4l *A, const NsVector4l *B )
   {  _NS_VECTOR4_CMPD_SUB( A, B );  }

NsVector2i* ns_vector2i_cmpd_sub( NsVector2i *A, const NsVector2i *B )
   {  _NS_VECTOR2_CMPD_SUB( A, B );  }

NsVector3i* ns_vector3i_cmpd_sub( NsVector3i *A, const NsVector3i *B )
   {  _NS_VECTOR3_CMPD_SUB( A, B );  }

NsVector4i* ns_vector4i_cmpd_sub( NsVector4i *A, const NsVector4i *B )
   {  _NS_VECTOR4_CMPD_SUB( A, B );  }

NsVector2s* ns_vector2s_cmpd_sub( NsVector2s *A, const NsVector2s *B )
   {  _NS_VECTOR2_CMPD_SUB( A, B );  }

NsVector3s* ns_vector3s_cmpd_sub( NsVector3s *A, const NsVector3s *B )
   {  _NS_VECTOR3_CMPD_SUB( A, B );  }

NsVector4s* ns_vector4s_cmpd_sub( NsVector4s *A, const NsVector4s *B )
   {  _NS_VECTOR4_CMPD_SUB( A, B );  }


#define _NS_VECTOR2_SUB_COMPS( C, A, _x, _y )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR2( (C), =, (A)->x - (_x), (A)->y - (_y) )

#define _NS_VECTOR3_SUB_COMPS( C, A, _x, _y, _z )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR3( (C), =, (A)->x - (_x), (A)->y - (_y), (A)->z - (_z) )

#define _NS_VECTOR4_SUB_COMPS( C, A, _x, _y, _z, _w )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR4( (C), =, (A)->x - (_x), (A)->y - (_y), (A)->z - (_z), (A)->w - (_w) )

NsVector2d* ns_vector2d_sub_xy( NsVector2d *C, const NsVector2d *A, nsdouble x, nsdouble y )
   {  _NS_VECTOR2_SUB_COMPS( C, A, x, y );  }

NsVector3d* ns_vector3d_sub_xyz( NsVector3d *C, const NsVector3d *A, nsdouble x, nsdouble y, nsdouble z )
   {  _NS_VECTOR3_SUB_COMPS( C, A, x, y, z );  }

NsVector4d* ns_vector4d_sub_xyzw( NsVector4d *C, const NsVector4d *A, nsdouble x, nsdouble y, nsdouble z, nsdouble w )
   {  _NS_VECTOR4_SUB_COMPS( C, A, x, y, z, w );  }

NsVector2f* ns_vector2f_sub_xy( NsVector2f *C, const NsVector2f *A, nsfloat x, nsfloat y )
   {  _NS_VECTOR2_SUB_COMPS( C, A, x, y );  }

NsVector3f* ns_vector3f_sub_xyz( NsVector3f *C, const NsVector3f *A, nsfloat x, nsfloat y, nsfloat z )
   {  _NS_VECTOR3_SUB_COMPS( C, A, x, y, z );  }

NsVector4f* ns_vector4f_sub_xyzw( NsVector4f *C, const NsVector4f *A, nsfloat x, nsfloat y, nsfloat z, nsfloat w )
   {  _NS_VECTOR4_SUB_COMPS( C, A, x, y, z, w );  }

NsVector2l* ns_vector2l_sub_xy( NsVector2l *C, const NsVector2l *A, nslong x, nslong y )
   {  _NS_VECTOR2_SUB_COMPS( C, A, x, y );  }

NsVector3l* ns_vector3l_sub_xyz( NsVector3l *C, const NsVector3l *A, nslong x, nslong y, nslong z )
   {  _NS_VECTOR3_SUB_COMPS( C, A, x, y, z );  }

NsVector4l* ns_vector4l_sub_xyzw( NsVector4l *C, const NsVector4l *A, nslong x, nslong y, nslong z, nslong w )
   {  _NS_VECTOR4_SUB_COMPS( C, A, x, y, z, w );  }

NsVector2i* ns_vector2i_sub_xy( NsVector2i *C, const NsVector2i *A, nsint x, nsint y )
   {  _NS_VECTOR2_SUB_COMPS( C, A, x, y );  }

NsVector3i* ns_vector3i_sub_xyz( NsVector3i *C, const NsVector3i *A, nsint x, nsint y, nsint z )
   {  _NS_VECTOR3_SUB_COMPS( C, A, x, y, z );  }

NsVector4i* ns_vector4i_sub_xyzw( NsVector4i *C, const NsVector4i *A, nsint x, nsint y, nsint z, nsint w )
   {  _NS_VECTOR4_SUB_COMPS( C, A, x, y, z, w );  }

NsVector2s* ns_vector2s_sub_xy( NsVector2s *C, const NsVector2s *A, nsshort x, nsshort y )
   {  _NS_VECTOR2_SUB_COMPS( C, A, x, y );  }

NsVector3s* ns_vector3s_sub_xyz( NsVector3s *C, const NsVector3s *A, nsshort x, nsshort y, nsshort z )
   {  _NS_VECTOR3_SUB_COMPS( C, A, x, y, z );  }

NsVector4s* ns_vector4s_sub_xyzw( NsVector4s *C, const NsVector4s *A, nsshort x, nsshort y, nsshort z, nsshort w )
   {  _NS_VECTOR4_SUB_COMPS( C, A, x, y, z, w );  }


#define _NS_VECTOR2_CMPD_SUB_COMPS( V, _x, _y )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR2( (V), =, (V)->x - (_x), (V)->y - (_y) )

#define _NS_VECTOR3_CMPD_SUB_COMPS( V, _x, _y, _z )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR3( (V), =, (V)->x - (_x), (V)->y - (_y), (V)->z - (_z) )

#define _NS_VECTOR4_CMPD_SUB_COMPS( V, _x, _y, _z, _w )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR4( (V), =, (V)->x - (_x), (V)->y - (_y), (V)->z - (_z), (V)->w - (_w) )

NsVector2d* ns_vector2d_cmpd_sub_xy( NsVector2d *V, nsdouble x, nsdouble y )
   {  _NS_VECTOR2_CMPD_SUB_COMPS( V, x, y );  }

NsVector3d* ns_vector3d_cmpd_sub_xyz( NsVector3d *V, nsdouble x, nsdouble y, nsdouble z )
   {  _NS_VECTOR3_CMPD_SUB_COMPS( V, x, y, z );  }

NsVector4d* ns_vector4d_cmpd_sub_xyzw( NsVector4d *V, nsdouble x, nsdouble y, nsdouble z, nsdouble w )
   {  _NS_VECTOR4_CMPD_SUB_COMPS( V, x, y, z, w );  }

NsVector2f* ns_vector2f_cmpd_sub_xy( NsVector2f *V, nsfloat x, nsfloat y )
   {  _NS_VECTOR2_CMPD_SUB_COMPS( V, x, y );  }

NsVector3f* ns_vector3f_cmpd_sub_xyz( NsVector3f *V, nsfloat x, nsfloat y, nsfloat z )
   {  _NS_VECTOR3_CMPD_SUB_COMPS( V, x, y, z );  }

NsVector4f* ns_vector4f_cmpd_sub_xyzw( NsVector4f *V, nsfloat x, nsfloat y, nsfloat z, nsfloat w )
   {  _NS_VECTOR4_CMPD_SUB_COMPS( V, x, y, z, w );  }

NsVector2l* ns_vector2l_cmpd_sub_xy( NsVector2l *V, nslong x, nslong y )
   {  _NS_VECTOR2_CMPD_SUB_COMPS( V, x, y );  }

NsVector3l* ns_vector3l_cmpd_sub_xyz( NsVector3l *V, nslong x, nslong y, nslong z )
   {  _NS_VECTOR3_CMPD_SUB_COMPS( V, x, y, z );  }

NsVector4l* ns_vector4l_cmpd_sub_xyzw( NsVector4l *V, nslong x, nslong y, nslong z, nslong w )
   {  _NS_VECTOR4_CMPD_SUB_COMPS( V, x, y, z, w );  }

NsVector2i* ns_vector2i_cmpd_sub_xy( NsVector2i *V, nsint x, nsint y )
   {  _NS_VECTOR2_CMPD_SUB_COMPS( V, x, y );  }

NsVector3i* ns_vector3i_cmpd_sub_xyz( NsVector3i *V, nsint x, nsint y, nsint z )
   {  _NS_VECTOR3_CMPD_SUB_COMPS( V, x, y, z );  }

NsVector4i* ns_vector4i_cmpd_sub_xyzw( NsVector4i *V, nsint x, nsint y, nsint z, nsint w )
   {  _NS_VECTOR4_CMPD_SUB_COMPS( V, x, y, z, w );  }

NsVector2s* ns_vector2s_cmpd_sub_xy( NsVector2s *V, nsshort x, nsshort y )
   {  _NS_VECTOR2_CMPD_SUB_COMPS( V, x, y );  }

NsVector3s* ns_vector3s_cmpd_sub_xyz( NsVector3s *V, nsshort x, nsshort y, nsshort z )
   {  _NS_VECTOR3_CMPD_SUB_COMPS( V, x, y, z );  }

NsVector4s* ns_vector4s_cmpd_sub_xyzw( NsVector4s *V, nsshort x, nsshort y, nsshort z, nsshort w )
   {  _NS_VECTOR4_CMPD_SUB_COMPS( V, x, y, z, w );  }


#define _NS_VECTOR3_MUL( M, A, B )\
	M->m11 = A->x*B->x; M->m12 = A->x*B->y; M->m13 = A->x*B->z;\
	M->m21 = A->y*B->x; M->m22 = A->y*B->y; M->m23 = A->y*B->z;\
	M->m31 = A->z*B->x; M->m32 = A->z*B->y; M->m33 = A->z*B->z;\
	return M

#define _NS_VECTOR4_MUL( M, A, B )\
	M->m11 = A->x*B->x; M->m12 = A->x*B->y; M->m13 = A->x*B->z; M->m14 = A->x*B->w;\
	M->m21 = A->y*B->x; M->m22 = A->y*B->y; M->m23 = A->y*B->z; M->m24 = A->y*B->w;\
	M->m31 = A->z*B->x; M->m32 = A->z*B->y; M->m33 = A->z*B->z; M->m34 = A->z*B->w;\
	M->m41 = A->w*B->x; M->m42 = A->w*B->y; M->m43 = A->w*B->z; M->m44 = A->w*B->w;\
	return M

NsMatrix33d* ns_vector3d_mul( NsMatrix33d *M, const NsVector3d *A, const NsVector3d *B )
	{  _NS_VECTOR3_MUL( M, A, B );  }

NsMatrix44d* ns_vector4d_mul( NsMatrix44d *M, const NsVector4d *A, const NsVector4d *B )
	{  _NS_VECTOR4_MUL( M, A, B );  }

NsMatrix33f* ns_vector3f_mul( NsMatrix33f *M, const NsVector3f *A, const NsVector3f *B )
	{  _NS_VECTOR3_MUL( M, A, B );  }

NsMatrix44f* ns_vector4f_mul( NsMatrix44f *M, const NsVector4f *A, const NsVector4f *B )
	{  _NS_VECTOR4_MUL( M, A, B );  }

NsMatrix33l* ns_vector3l_mul( NsMatrix33l *M, const NsVector3l *A, const NsVector3l *B )
	{  _NS_VECTOR3_MUL( M, A, B );  }

NsMatrix44l* ns_vector4l_mul( NsMatrix44l *M, const NsVector4l *A, const NsVector4l *B )
	{  _NS_VECTOR4_MUL( M, A, B );  }


#define _NS_VECTOR2_SCALE( C, A, s )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR2( (C), =, (A)->x * (s), (A)->y * (s) )

#define _NS_VECTOR3_SCALE( C, A, s )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR3( (C), =, (A)->x * (s), (A)->y * (s), (A)->z * (s) )

#define _NS_VECTOR4_SCALE( C, A, s )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR4( (C), =, (A)->x * (s), (A)->y * (s), (A)->z * (s), (A)->w * (s) )

NsVector2d* ns_vector2d_scale( NsVector2d *C, const NsVector2d *A, nsdouble s )
	{  _NS_VECTOR2_SCALE( C, A, s );  }

NsVector3d* ns_vector3d_scale( NsVector3d *C, const NsVector3d *A, nsdouble s )
	{  _NS_VECTOR3_SCALE( C, A, s );  }

NsVector4d* ns_vector4d_scale( NsVector4d *C, const NsVector4d *A, nsdouble s )
	{  _NS_VECTOR4_SCALE( C, A, s );  }

NsVector2f* ns_vector2f_scale( NsVector2f *C, const NsVector2f *A, nsfloat s )
	{  _NS_VECTOR2_SCALE( C, A, s );  }

NsVector3f* ns_vector3f_scale( NsVector3f *C, const NsVector3f *A, nsfloat s )
	{  _NS_VECTOR3_SCALE( C, A, s );  }

NsVector4f* ns_vector4f_scale( NsVector4f *C, const NsVector4f *A, nsfloat s )
	{  _NS_VECTOR4_SCALE( C, A, s );  }

NsVector2l* ns_vector2l_scale( NsVector2l *C, const NsVector2l *A, nslong s )
	{  _NS_VECTOR2_SCALE( C, A, s );  }

NsVector3l* ns_vector3l_scale( NsVector3l *C, const NsVector3l *A, nslong s )
	{  _NS_VECTOR3_SCALE( C, A, s );  }

NsVector4l* ns_vector4l_scale( NsVector4l *C, const NsVector4l *A, nslong s )
	{  _NS_VECTOR4_SCALE( C, A, s );  }

NsVector2i* ns_vector2i_scale( NsVector2i *C, const NsVector2i *A, nsint s )
	{  _NS_VECTOR2_SCALE( C, A, s );  }

NsVector3i* ns_vector3i_scale( NsVector3i *C, const NsVector3i *A, nsint s )
	{  _NS_VECTOR3_SCALE( C, A, s );  }

NsVector4i* ns_vector4i_scale( NsVector4i *C, const NsVector4i *A, nsint s )
	{  _NS_VECTOR4_SCALE( C, A, s );  }

NsVector2s* ns_vector2s_scale( NsVector2s *C, const NsVector2s *A, nsshort s )
	{  _NS_VECTOR2_SCALE( C, A, s );  }

NsVector3s* ns_vector3s_scale( NsVector3s *C, const NsVector3s *A, nsshort s )
	{  _NS_VECTOR3_SCALE( C, A, s );  }

NsVector4s* ns_vector4s_scale( NsVector4s *C, const NsVector4s *A, nsshort s )
	{  _NS_VECTOR4_SCALE( C, A, s );  }


#define _NS_VECTOR2_CMPD_SCALE( V, s )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR2( (V), =, (V)->x * (s), (V)->y * (s) )

#define _NS_VECTOR3_CMPD_SCALE( V, s )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR3( (V), =, (V)->x * (s), (V)->y * (s), (V)->z * (s) )

#define _NS_VECTOR4_CMPD_SCALE( V, s )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR4( (V), =, (V)->x * (s), (V)->y * (s), (V)->z * (s), (V)->w * (s) )

NsVector2d* ns_vector2d_cmpd_scale( NsVector2d *V, nsdouble s )
   {  _NS_VECTOR2_CMPD_SCALE( V, s );  }

NsVector3d* ns_vector3d_cmpd_scale( NsVector3d *V, nsdouble s )
   {  _NS_VECTOR3_CMPD_SCALE( V, s );  }

NsVector4d* ns_vector4d_cmpd_scale( NsVector4d *V, nsdouble s )
   {  _NS_VECTOR4_CMPD_SCALE( V, s );  }

NsVector2f* ns_vector2f_cmpd_scale( NsVector2f *V, nsfloat s )
   {  _NS_VECTOR2_CMPD_SCALE( V, s );  }

NsVector3f* ns_vector3f_cmpd_scale( NsVector3f *V, nsfloat s )
   {  _NS_VECTOR3_CMPD_SCALE( V, s );  }

NsVector4f* ns_vector4f_cmpd_scale( NsVector4f *V, nsfloat s )
   {  _NS_VECTOR4_CMPD_SCALE( V, s );  }

NsVector2l* ns_vector2l_cmpd_scale( NsVector2l *V, nslong s )
   {  _NS_VECTOR2_CMPD_SCALE( V, s );  }

NsVector3l* ns_vector3l_cmpd_scale( NsVector3l *V, nslong s )
   {  _NS_VECTOR3_CMPD_SCALE( V, s );  }

NsVector4l* ns_vector4l_cmpd_scale( NsVector4l *V, nslong s )
   {  _NS_VECTOR4_CMPD_SCALE( V, s );  }

NsVector2i* ns_vector2i_cmpd_scale( NsVector2i *V, nsint s )
   {  _NS_VECTOR2_CMPD_SCALE( V, s );  }

NsVector3i* ns_vector3i_cmpd_scale( NsVector3i *V, nsint s )
   {  _NS_VECTOR3_CMPD_SCALE( V, s );  }

NsVector4i* ns_vector4i_cmpd_scale( NsVector4i *V, nsint s )
   {  _NS_VECTOR4_CMPD_SCALE( V, s );  }

NsVector2s* ns_vector2s_cmpd_scale( NsVector2s *V, nsshort s )
   {  _NS_VECTOR2_CMPD_SCALE( V, s );  }

NsVector3s* ns_vector3s_cmpd_scale( NsVector3s *V, nsshort s )
   {  _NS_VECTOR3_CMPD_SCALE( V, s );  }

NsVector4s* ns_vector4s_cmpd_scale( NsVector4s *V, nsshort s )
   {  _NS_VECTOR4_CMPD_SCALE( V, s );  }


#define _NS_VECTOR2_NON_UNI_SCALE( C, A, S )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR2( (C), =, (A)->x * (S)->x, (A)->y * (S)->y )

#define _NS_VECTOR3_NON_UNI_SCALE( C, A, S )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR3( (C), =, (A)->x * (S)->x, (A)->y * (S)->y, (A)->z * (S)->z )

#define _NS_VECTOR4_NON_UNI_SCALE( C, A, S )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR4( (C), =, (A)->x * (S)->x, (A)->y * (S)->y, (A)->z * (S)->z, (A)->w * (S)->w )

NsVector2d* ns_vector2d_non_uni_scale( NsVector2d *C, const NsVector2d *A, const NsVector2d *S )
	{  _NS_VECTOR2_NON_UNI_SCALE( C, A, S );  }

NsVector3d* ns_vector3d_non_uni_scale( NsVector3d *C, const NsVector3d *A, const NsVector3d *S )
	{  _NS_VECTOR3_NON_UNI_SCALE( C, A, S );  }

NsVector4d* ns_vector4d_non_uni_scale( NsVector4d *C, const NsVector4d *A, const NsVector4d *S )
	{  _NS_VECTOR4_NON_UNI_SCALE( C, A, S );  }

NsVector2f* ns_vector2f_non_uni_scale( NsVector2f *C, const NsVector2f *A, const NsVector2f *S )
	{  _NS_VECTOR2_NON_UNI_SCALE( C, A, S );  }

NsVector3f* ns_vector3f_non_uni_scale( NsVector3f *C, const NsVector3f *A, const NsVector3f *S )
	{  _NS_VECTOR3_NON_UNI_SCALE( C, A, S );  }

NsVector4f* ns_vector4f_non_uni_scale( NsVector4f *C, const NsVector4f *A, const NsVector4f *S )
	{  _NS_VECTOR4_NON_UNI_SCALE( C, A, S );  }

NsVector2l* ns_vector2l_non_uni_scale( NsVector2l *C, const NsVector2l *A, const NsVector2l *S )
	{  _NS_VECTOR2_NON_UNI_SCALE( C, A, S );  }

NsVector3l* ns_vector3l_non_uni_scale( NsVector3l *C, const NsVector3l *A, const NsVector3l *S )
	{  _NS_VECTOR3_NON_UNI_SCALE( C, A, S );  }

NsVector4l* ns_vector4l_non_uni_scale( NsVector4l *C, const NsVector4l *A, const NsVector4l *S )
	{  _NS_VECTOR4_NON_UNI_SCALE( C, A, S );  }

NsVector2i* ns_vector2i_non_uni_scale( NsVector2i *C, const NsVector2i *A, const NsVector2i *S )
	{  _NS_VECTOR2_NON_UNI_SCALE( C, A, S );  }

NsVector3i* ns_vector3i_non_uni_scale( NsVector3i *C, const NsVector3i *A, const NsVector3i *S )
	{  _NS_VECTOR3_NON_UNI_SCALE( C, A, S );  }

NsVector4i* ns_vector4i_non_uni_scale( NsVector4i *C, const NsVector4i *A, const NsVector4i *S )
	{  _NS_VECTOR4_NON_UNI_SCALE( C, A, S );  }

NsVector2s* ns_vector2s_non_uni_scale( NsVector2s *C, const NsVector2s *A, const NsVector2s *S )
	{  _NS_VECTOR2_NON_UNI_SCALE( C, A, S );  }

NsVector3s* ns_vector3s_non_uni_scale( NsVector3s *C, const NsVector3s *A, const NsVector3s *S )
	{  _NS_VECTOR3_NON_UNI_SCALE( C, A, S );  }

NsVector4s* ns_vector4s_non_uni_scale( NsVector4s *C, const NsVector4s *A, const NsVector4s *S )
	{  _NS_VECTOR4_NON_UNI_SCALE( C, A, S );  }


#define _NS_VECTOR2_CMPD_NON_UNI_SCALE( V, S )\
	ns_assert( NULL != (V) );\
	ns_assert( NULL != (S) );\
	_NS_VECTOR2( (V), =, (V)->x * (S)->x, (V)->y * (S)->y )

#define _NS_VECTOR3_CMPD_NON_UNI_SCALE( V, S )\
	ns_assert( NULL != (V) );\
	ns_assert( NULL != (S) );\
	_NS_VECTOR3( (V), =, (V)->x * (S)->x, (V)->y * (S)->y, (V)->z * (S)->z )

#define _NS_VECTOR4_CMPD_NON_UNI_SCALE( V, S )\
	ns_assert( NULL != (V) );\
	ns_assert( NULL != (S) );\
	_NS_VECTOR4( (V), =, (V)->x * (S)->x, (V)->y * (S)->y, (V)->z * (S)->z, (V)->w * (S)->w )

NsVector2d* ns_vector2d_cmpd_non_uni_scale( NsVector2d *V, const NsVector2d *S )
   {  _NS_VECTOR2_CMPD_NON_UNI_SCALE( V, S );  }

NsVector3d* ns_vector3d_cmpd_non_uni_scale( NsVector3d *V, const NsVector3d *S )
   {  _NS_VECTOR3_CMPD_NON_UNI_SCALE( V, S );  }

NsVector4d* ns_vector4d_cmpd_non_uni_scale( NsVector4d *V, const NsVector4d *S )
   {  _NS_VECTOR4_CMPD_NON_UNI_SCALE( V, S );  }

NsVector2f* ns_vector2f_cmpd_non_uni_scale( NsVector2f *V, const NsVector2f *S )
   {  _NS_VECTOR2_CMPD_NON_UNI_SCALE( V, S );  }

NsVector3f* ns_vector3f_cmpd_non_uni_scale( NsVector3f *V, const NsVector3f *S )
   {  _NS_VECTOR3_CMPD_NON_UNI_SCALE( V, S );  }

NsVector4f* ns_vector4f_cmpd_non_uni_scale( NsVector4f *V, const NsVector4f *S )
   {  _NS_VECTOR4_CMPD_NON_UNI_SCALE( V, S );  }

NsVector2l* ns_vector2l_cmpd_non_uni_scale( NsVector2l *V, const NsVector2l *S )
   {  _NS_VECTOR2_CMPD_NON_UNI_SCALE( V, S );  }

NsVector3l* ns_vector3l_cmpd_non_uni_scale( NsVector3l *V, const NsVector3l *S )
   {  _NS_VECTOR3_CMPD_NON_UNI_SCALE( V, S );  }

NsVector4l* ns_vector4l_cmpd_non_uni_scale( NsVector4l *V, const NsVector4l *S )
   {  _NS_VECTOR4_CMPD_NON_UNI_SCALE( V, S );  }

NsVector2i* ns_vector2i_cmpd_non_uni_scale( NsVector2i *V, const NsVector2i *S )
   {  _NS_VECTOR2_CMPD_NON_UNI_SCALE( V, S );  }

NsVector3i* ns_vector3i_cmpd_non_uni_scale( NsVector3i *V, const NsVector3i *S )
   {  _NS_VECTOR3_CMPD_NON_UNI_SCALE( V, S );  }

NsVector4i* ns_vector4i_cmpd_non_uni_scale( NsVector4i *V, const NsVector4i *S )
   {  _NS_VECTOR4_CMPD_NON_UNI_SCALE( V, S );  }

NsVector2s* ns_vector2s_cmpd_non_uni_scale( NsVector2s *V, const NsVector2s *S )
   {  _NS_VECTOR2_CMPD_NON_UNI_SCALE( V, S );  }

NsVector3s* ns_vector3s_cmpd_non_uni_scale( NsVector3s *V, const NsVector3s *S )
   {  _NS_VECTOR3_CMPD_NON_UNI_SCALE( V, S );  }

NsVector4s* ns_vector4s_cmpd_non_uni_scale( NsVector4s *V, const NsVector4s *S )
   {  _NS_VECTOR4_CMPD_NON_UNI_SCALE( V, S );  }


#define _NS_VECTOR2_SCALE_COMPS( C, A, sx, sy )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR2( (C), =, (A)->x * (sx), (A)->y * (sy) )

#define _NS_VECTOR3_SCALE_COMPS( C, A, sx, sy, sz )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR3( (C), =, (A)->x * (sx), (A)->y * (sy), (A)->z * (sz) )

#define _NS_VECTOR4_SCALE_COMPS( C, A, sx, sy, sz, sw )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR4( (C), =, (A)->x * (sx), (A)->y * (sy), (A)->z * (sz), (A)->w * (sw) )

NsVector2d* ns_vector2d_scale_xy( NsVector2d *C, const NsVector2d *A, nsdouble sx, nsdouble sy )
	{  _NS_VECTOR2_SCALE_COMPS( C, A, sx, sy );  }

NsVector3d* ns_vector3d_scale_xyz( NsVector3d *C, const NsVector3d *A, nsdouble sx, nsdouble sy, nsdouble sz )
	{  _NS_VECTOR3_SCALE_COMPS( C, A, sx, sy, sz );  }

NsVector4d* ns_vector4d_scale_xyzw( NsVector4d *C, const NsVector4d *A, nsdouble sx, nsdouble sy, nsdouble sz, nsdouble sw )
	{  _NS_VECTOR4_SCALE_COMPS( C, A, sx, sy, sz, sw );  }

NsVector2f* ns_vector2f_scale_xy( NsVector2f *C, const NsVector2f *A, nsfloat sx, nsfloat sy )
	{  _NS_VECTOR2_SCALE_COMPS( C, A, sx, sy );  }

NsVector3f* ns_vector3f_scale_xyz( NsVector3f *C, const NsVector3f *A, nsfloat sx, nsfloat sy, nsfloat sz )
	{  _NS_VECTOR3_SCALE_COMPS( C, A, sx, sy, sz );  }

NsVector4f* ns_vector4f_scale_xyzw( NsVector4f *C, const NsVector4f *A, nsfloat sx, nsfloat sy, nsfloat sz, nsfloat sw )
	{  _NS_VECTOR4_SCALE_COMPS( C, A, sx, sy, sz, sw );  }

NsVector2l* ns_vector2l_scale_xy( NsVector2l *C, const NsVector2l *A, nslong sx, nslong sy )
	{  _NS_VECTOR2_SCALE_COMPS( C, A, sx, sy );  }

NsVector3l* ns_vector3l_scale_xyz( NsVector3l *C, const NsVector3l *A, nslong sx, nslong sy, nslong sz )
	{  _NS_VECTOR3_SCALE_COMPS( C, A, sx, sy, sz );  }

NsVector4l* ns_vector4l_scale_xyzw( NsVector4l *C, const NsVector4l *A, nslong sx, nslong sy, nslong sz, nslong sw )
	{  _NS_VECTOR4_SCALE_COMPS( C, A, sx, sy, sz, sw );  }

NsVector2i* ns_vector2i_scale_xy( NsVector2i *C, const NsVector2i *A, nsint sx, nsint sy )
	{  _NS_VECTOR2_SCALE_COMPS( C, A, sx, sy );  }

NsVector3i* ns_vector3i_scale_xyz( NsVector3i *C, const NsVector3i *A, nsint sx, nsint sy, nsint sz )
	{  _NS_VECTOR3_SCALE_COMPS( C, A, sx, sy, sz );  }

NsVector4i* ns_vector4i_scale_xyzw( NsVector4i *C, const NsVector4i *A, nsint sx, nsint sy, nsint sz, nsint sw )
	{  _NS_VECTOR4_SCALE_COMPS( C, A, sx, sy, sz, sw );  }

NsVector2s* ns_vector2s_scale_xy( NsVector2s *C, const NsVector2s *A, nsshort sx, nsshort sy )
	{  _NS_VECTOR2_SCALE_COMPS( C, A, sx, sy );  }

NsVector3s* ns_vector3s_scale_xyz( NsVector3s *C, const NsVector3s *A, nsshort sx, nsshort sy, nsshort sz )
	{  _NS_VECTOR3_SCALE_COMPS( C, A, sx, sy, sz );  }

NsVector4s* ns_vector4s_scale_xyzw( NsVector4s *C, const NsVector4s *A, nsshort sx, nsshort sy, nsshort sz, nsshort sw )
	{  _NS_VECTOR4_SCALE_COMPS( C, A, sx, sy, sz, sw );  }


#define _NS_VECTOR2_CMPD_SCALE_COMPS( V, sx, sy )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR2( (V), =, (V)->x * (sx), (V)->y * (sy) )

#define _NS_VECTOR3_CMPD_SCALE_COMPS( V, sx, sy, sz )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR3( (V), =, (V)->x * (sx), (V)->y * (sy), (V)->z * (sz) )

#define _NS_VECTOR4_CMPD_SCALE_COMPS( V, sx, sy, sz, sw )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR4( (V), =, (V)->x * (sx), (V)->y * (sy), (V)->z * (sz), (V)->w * (sw) )

NsVector2d* ns_vector2d_cmpd_scale_xy( NsVector2d *V, nsdouble sx, nsdouble sy )
   {  _NS_VECTOR2_CMPD_SCALE_COMPS( V, sx, sy );  }

NsVector3d* ns_vector3d_cmpd_scale_xyz( NsVector3d *V, nsdouble sx, nsdouble sy, nsdouble sz )
   {  _NS_VECTOR3_CMPD_SCALE_COMPS( V, sx, sy, sz );  }

NsVector4d* ns_vector4d_cmpd_scale_xyzw( NsVector4d *V, nsdouble sx, nsdouble sy, nsdouble sz, nsdouble sw )
   {  _NS_VECTOR4_CMPD_SCALE_COMPS( V, sx, sy, sz, sw );  }

NsVector2f* ns_vector2f_cmpd_scale_xy( NsVector2f *V, nsfloat sx, nsfloat sy )
   {  _NS_VECTOR2_CMPD_SCALE_COMPS( V, sx, sy );  }

NsVector3f* ns_vector3f_cmpd_scale_xyz( NsVector3f *V, nsfloat sx, nsfloat sy, nsfloat sz )
   {  _NS_VECTOR3_CMPD_SCALE_COMPS( V, sx, sy, sz );  }

NsVector4f* ns_vector4f_cmpd_scale_xyzw( NsVector4f *V, nsfloat sx, nsfloat sy, nsfloat sz, nsfloat sw )
   {  _NS_VECTOR4_CMPD_SCALE_COMPS( V, sx, sy, sz, sw );  }

NsVector2l* ns_vector2l_cmpd_scale_xy( NsVector2l *V, nslong sx, nslong sy )
   {  _NS_VECTOR2_CMPD_SCALE_COMPS( V, sx, sy );  }

NsVector3l* ns_vector3l_cmpd_scale_xyz( NsVector3l *V, nslong sx, nslong sy, nslong sz )
   {  _NS_VECTOR3_CMPD_SCALE_COMPS( V, sx, sy, sz );  }

NsVector4l* ns_vector4l_cmpd_scale_xyzw( NsVector4l *V, nslong sx, nslong sy, nslong sz, nslong sw )
   {  _NS_VECTOR4_CMPD_SCALE_COMPS( V, sx, sy, sz, sw );  }

NsVector2i* ns_vector2i_cmpd_scale_xy( NsVector2i *V, nsint sx, nsint sy )
   {  _NS_VECTOR2_CMPD_SCALE_COMPS( V, sx, sy );  }

NsVector3i* ns_vector3i_cmpd_scale_xyz( NsVector3i *V, nsint sx, nsint sy, nsint sz )
   {  _NS_VECTOR3_CMPD_SCALE_COMPS( V, sx, sy, sz );  }

NsVector4i* ns_vector4i_cmpd_scale_xyzw( NsVector4i *V, nsint sx, nsint sy, nsint sz, nsint sw )
   {  _NS_VECTOR4_CMPD_SCALE_COMPS( V, sx, sy, sz, sw );  }

NsVector2s* ns_vector2s_cmpd_scale_xy( NsVector2s *V, nsshort sx, nsshort sy )
   {  _NS_VECTOR2_CMPD_SCALE_COMPS( V, sx, sy );  }

NsVector3s* ns_vector3s_cmpd_scale_xyz( NsVector3s *V, nsshort sx, nsshort sy, nsshort sz )
   {  _NS_VECTOR3_CMPD_SCALE_COMPS( V, sx, sy, sz );  }

NsVector4s* ns_vector4s_cmpd_scale_xyzw( NsVector4s *V, nsshort sx, nsshort sy, nsshort sz, nsshort sw )
   {  _NS_VECTOR4_CMPD_SCALE_COMPS( V, sx, sy, sz, sw );  }


#define _NS_VECTOR2_REV( C, A )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR2( (C), =, -(A)->x, -(A)->y )

#define _NS_VECTOR3_REV( C, A )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR3( (C), =, -(A)->x, -(A)->y, -(A)->z )

#define _NS_VECTOR4_REV( C, A )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR4( (C), =, -(A)->x, -(A)->y, -(A)->z, -(A)->w )

NsVector2d* ns_vector2d_rev( NsVector2d *C, const NsVector2d *A )
   {  _NS_VECTOR2_REV( C, A );  }

NsVector3d* ns_vector3d_rev( NsVector3d *C, const NsVector3d *A )
   {  _NS_VECTOR3_REV( C, A );  }

NsVector4d* ns_vector4d_rev( NsVector4d *C, const NsVector4d *A )
   {  _NS_VECTOR4_REV( C, A );  }

NsVector2f* ns_vector2f_rev( NsVector2f *C, const NsVector2f *A )
   {  _NS_VECTOR2_REV( C, A );  }

NsVector3f* ns_vector3f_rev( NsVector3f *C, const NsVector3f *A )
   {  _NS_VECTOR3_REV( C, A );  }

NsVector4f* ns_vector4f_rev( NsVector4f *C, const NsVector4f *A )
   {  _NS_VECTOR4_REV( C, A );  }

NsVector2l* ns_vector2l_rev( NsVector2l *C, const NsVector2l *A )
   {  _NS_VECTOR2_REV( C, A );  }

NsVector3l* ns_vector3l_rev( NsVector3l *C, const NsVector3l *A )
   {  _NS_VECTOR3_REV( C, A );  }

NsVector4l* ns_vector4l_rev( NsVector4l *C, const NsVector4l *A )
   {  _NS_VECTOR4_REV( C, A );  }

NsVector2i* ns_vector2i_rev( NsVector2i *C, const NsVector2i *A )
   {  _NS_VECTOR2_REV( C, A );  }

NsVector3i* ns_vector3i_rev( NsVector3i *C, const NsVector3i *A )
   {  _NS_VECTOR3_REV( C, A );  }

NsVector4i* ns_vector4i_rev( NsVector4i *C, const NsVector4i *A )
   {  _NS_VECTOR4_REV( C, A );  }

NsVector2s* ns_vector2s_rev( NsVector2s *C, const NsVector2s *A )
   {  _NS_VECTOR2_REV( C, A );  }

NsVector3s* ns_vector3s_rev( NsVector3s *C, const NsVector3s *A )
   {  _NS_VECTOR3_REV( C, A );  }

NsVector4s* ns_vector4s_rev( NsVector4s *C, const NsVector4s *A )
   {  _NS_VECTOR4_REV( C, A );  }


#define _NS_VECTOR2_CMPD_REV( V )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR2( (V), =, -(V)->x, -(V)->y )

#define _NS_VECTOR3_CMPD_REV( V )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR3( (V), =, -(V)->x, -(V)->y, -(V)->z )

#define _NS_VECTOR4_CMPD_REV( V )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR4( (V), =, -(V)->x, -(V)->y, -(V)->z, -(V)->w )

NsVector2d* ns_vector2d_cmpd_rev( NsVector2d *V )
   {  _NS_VECTOR2_CMPD_REV( V );  }

NsVector3d* ns_vector3d_cmpd_rev( NsVector3d *V )
   {  _NS_VECTOR3_CMPD_REV( V );  }

NsVector4d* ns_vector4d_cmpd_rev( NsVector4d *V )
   {  _NS_VECTOR4_CMPD_REV( V );  }

NsVector2f* ns_vector2f_cmpd_rev( NsVector2f *V )
   {  _NS_VECTOR2_CMPD_REV( V );  }

NsVector3f* ns_vector3f_cmpd_rev( NsVector3f *V )
   {  _NS_VECTOR3_CMPD_REV( V );  }

NsVector4f* ns_vector4f_cmpd_rev( NsVector4f *V )
   {  _NS_VECTOR4_CMPD_REV( V );  }

NsVector2l* ns_vector2l_cmpd_rev( NsVector2l *V )
   {  _NS_VECTOR2_CMPD_REV( V );  }

NsVector3l* ns_vector3l_cmpd_rev( NsVector3l *V )
   {  _NS_VECTOR3_CMPD_REV( V );  }

NsVector4l* ns_vector4l_cmpd_rev( NsVector4l *V )
   {  _NS_VECTOR4_CMPD_REV( V );  }

NsVector2i* ns_vector2i_cmpd_rev( NsVector2i *V )
   {  _NS_VECTOR2_CMPD_REV( V );  }

NsVector3i* ns_vector3i_cmpd_rev( NsVector3i *V )
   {  _NS_VECTOR3_CMPD_REV( V );  }

NsVector4i* ns_vector4i_cmpd_rev( NsVector4i *V )
   {  _NS_VECTOR4_CMPD_REV( V );  }

NsVector2s* ns_vector2s_cmpd_rev( NsVector2s *V )
   {  _NS_VECTOR2_CMPD_REV( V );  }

NsVector3s* ns_vector3s_cmpd_rev( NsVector3s *V )
   {  _NS_VECTOR3_CMPD_REV( V );  }

NsVector4s* ns_vector4s_cmpd_rev( NsVector4s *V )
   {  _NS_VECTOR4_CMPD_REV( V );  }


#define _NS_VECTOR2_ZERO( V, type )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR2( (V), =, (type)0, (type)0 )

#define _NS_VECTOR3_ZERO( V, type )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR3( (V), =, (type)0, (type)0, (type)0 )

#define _NS_VECTOR4_ZERO( V, type )\
	ns_assert( NULL != (V) );\
	_NS_VECTOR4( (V), =, (type)0, (type)0, (type)0, (type)0 )

NsVector2d* ns_vector2d_zero( NsVector2d *V )
   {  _NS_VECTOR2_ZERO( V, nsdouble );  }

NsVector3d* ns_vector3d_zero( NsVector3d *V )
   {  _NS_VECTOR3_ZERO( V, nsdouble );  }

NsVector4d* ns_vector4d_zero( NsVector4d *V )
   {  _NS_VECTOR4_ZERO( V, nsdouble );  }

NsVector2f* ns_vector2f_zero( NsVector2f *V )
   {  _NS_VECTOR2_ZERO( V, nsfloat );  }

NsVector3f* ns_vector3f_zero( NsVector3f *V )
   {  _NS_VECTOR3_ZERO( V, nsfloat );  }

NsVector4f* ns_vector4f_zero( NsVector4f *V )
   {  _NS_VECTOR4_ZERO( V, nsfloat );  }

NsVector2l* ns_vector2l_zero( NsVector2l *V )
   {  _NS_VECTOR2_ZERO( V, nslong );  }

NsVector3l* ns_vector3l_zero( NsVector3l *V )
   {  _NS_VECTOR3_ZERO( V, nslong );  }

NsVector4l* ns_vector4l_zero( NsVector4l *V )
   {  _NS_VECTOR4_ZERO( V, nslong );  }

NsVector2i* ns_vector2i_zero( NsVector2i *V )
   {  _NS_VECTOR2_ZERO( V, nsint );  }

NsVector3i* ns_vector3i_zero( NsVector3i *V )
   {  _NS_VECTOR3_ZERO( V, nsint );  }

NsVector4i* ns_vector4i_zero( NsVector4i *V )
   {  _NS_VECTOR4_ZERO( V, nsint );  }

NsVector2s* ns_vector2s_zero( NsVector2s *V )
   {  _NS_VECTOR2_ZERO( V, nsshort );  }

NsVector3s* ns_vector3s_zero( NsVector3s *V )
   {  _NS_VECTOR3_ZERO( V, nsshort );  }

NsVector4s* ns_vector4s_zero( NsVector4s *V )
   {  _NS_VECTOR4_ZERO( V, nsshort );  }


#define _NS_VECTOR_NORM( V, d, t, type )\
	ns_assert( NULL != (V) );\
	return ns_vector##d##t##_cmpd_scale( (V), (type)1 / ns_vector##d##t##_mag( (V) ) )

NsVector2d* ns_vector2d_norm( NsVector2d *V )
   {  _NS_VECTOR_NORM( V, 2, d, nsdouble );  }

NsVector3d* ns_vector3d_norm( NsVector3d *V )
   {  _NS_VECTOR_NORM( V, 3, d, nsdouble );  }

NsVector4d* ns_vector4d_norm( NsVector4d *V )
   {  _NS_VECTOR_NORM( V, 4, d, nsdouble );  }

NsVector2f* ns_vector2f_norm( NsVector2f *V )
   {  _NS_VECTOR_NORM( V, 2, f, nsfloat );  }

NsVector3f* ns_vector3f_norm( NsVector3f *V )
   {  _NS_VECTOR_NORM( V, 3, f, nsfloat );  }

NsVector4f* ns_vector4f_norm( NsVector4f *V )
   {  _NS_VECTOR_NORM( V, 4, f, nsfloat );  }


#define _NS_VECTOR_MAG( V, t, type, sqrt_func )\
	ns_assert( NULL != (V) );\
	return sqrt_func( ( type )ns_vector##t##_dot( (V), (V) ) )

nsdouble ns_vector2d_mag( const NsVector2d *V )
   {  _NS_VECTOR_MAG( V, 2d, nsdouble, ns_sqrt );  }

nsdouble ns_vector3d_mag( const NsVector3d *V )
   {  _NS_VECTOR_MAG( V, 3d, nsdouble, ns_sqrt );  }

nsdouble ns_vector4d_mag( const NsVector4d *V )
   {  _NS_VECTOR_MAG( V, 4d, nsdouble, ns_sqrt );  }

nsfloat ns_vector2f_mag( const NsVector2f *V )
   {  _NS_VECTOR_MAG( V, 2f, nsfloat, ns_sqrtf );  }

nsfloat ns_vector3f_mag( const NsVector3f *V )
   {  _NS_VECTOR_MAG( V, 3f, nsfloat, ns_sqrtf );  }

nsfloat ns_vector4f_mag( const NsVector4f *V )
   {  _NS_VECTOR_MAG( V, 4f, nsfloat, ns_sqrtf );  }

nsdouble ns_vector2l_mag( const NsVector2l *V )
   {  _NS_VECTOR_MAG( V, 2l, nsdouble, ns_sqrt );  }

nsdouble ns_vector3l_mag( const NsVector3l *V )
   {  _NS_VECTOR_MAG( V, 3l, nsdouble, ns_sqrt );  }

nsdouble ns_vector4l_mag( const NsVector4l *V )
   {  _NS_VECTOR_MAG( V, 4l, nsdouble, ns_sqrt );  }

nsfloat ns_vector2i_mag( const NsVector2i *V )
   {  _NS_VECTOR_MAG( V, 2i, nsfloat, ns_sqrtf );  }

nsfloat ns_vector3i_mag( const NsVector3i *V )
   {  _NS_VECTOR_MAG( V, 3i, nsfloat, ns_sqrtf );  }

nsfloat ns_vector4i_mag( const NsVector4i *V )
   {  _NS_VECTOR_MAG( V, 4i, nsfloat, ns_sqrtf );  }

nsfloat ns_vector2s_mag( const NsVector2s *V )
   {  _NS_VECTOR_MAG( V, 2s, nsfloat, ns_sqrtf );  }

nsfloat ns_vector3s_mag( const NsVector3s *V )
   {  _NS_VECTOR_MAG( V, 3s, nsfloat, ns_sqrtf );  }

nsfloat ns_vector4s_mag( const NsVector4s *V )
   {  _NS_VECTOR_MAG( V, 4s, nsfloat, ns_sqrtf );  }


#define _NS_VECTOR2_DOT( A, B, type )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	return (type)(A)->x*(B)->x + (type)(A)->y*(B)->y

#define _NS_VECTOR3_DOT( A, B, type )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	return (type)(A)->x*(B)->x + (type)(A)->y*(B)->y + (type)(A)->z*(B)->z

#define _NS_VECTOR4_DOT( A, B, type )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	return (type)(A)->x*(B)->x + (type)(A)->y*(B)->y + (type)(A)->z*(B)->z + (type)(A)->w*(B)->w

nsdouble ns_vector2d_dot( const NsVector2d *A, const NsVector2d *B )
   {  _NS_VECTOR2_DOT( A, B, nsdouble );  }

nsdouble ns_vector3d_dot( const NsVector3d *A, const NsVector3d *B )
   {  _NS_VECTOR3_DOT( A, B, nsdouble );  }

nsdouble ns_vector4d_dot( const NsVector4d *A, const NsVector4d *B )
   {  _NS_VECTOR4_DOT( A, B, nsdouble );  }

nsfloat ns_vector2f_dot( const NsVector2f *A, const NsVector2f *B )
   {  _NS_VECTOR2_DOT( A, B, nsfloat );  }

nsfloat ns_vector3f_dot( const NsVector3f *A, const NsVector3f *B )
   {  _NS_VECTOR3_DOT( A, B, nsfloat );  }

nsfloat ns_vector4f_dot( const NsVector4f *A, const NsVector4f *B )
   {  _NS_VECTOR4_DOT( A, B, nsfloat );  }

nslong ns_vector2l_dot( const NsVector2l *A, const NsVector2l *B )
   {  _NS_VECTOR2_DOT( A, B, nslong );  }

nslong ns_vector3l_dot( const NsVector3l *A, const NsVector3l *B )
   {  _NS_VECTOR3_DOT( A, B, nslong );  }

nslong ns_vector4l_dot( const NsVector4l *A, const NsVector4l *B )
   {  _NS_VECTOR4_DOT( A, B, nslong );  }

nslong ns_vector2i_dot( const NsVector2i *A, const NsVector2i *B )
	{  _NS_VECTOR2_DOT( A, B, nslong );  }

nslong ns_vector3i_dot( const NsVector3i *A, const NsVector3i *B )
   {  _NS_VECTOR3_DOT( A, B, nslong );  }

nslong ns_vector4i_dot( const NsVector4i *A, const NsVector4i *B )
   {  _NS_VECTOR4_DOT( A, B, nslong );  }

nslong ns_vector2s_dot( const NsVector2s *A, const NsVector2s *B )
   {  _NS_VECTOR2_DOT( A, B, nslong );  }

nslong ns_vector3s_dot( const NsVector3s *A, const NsVector3s *B )
   {  _NS_VECTOR3_DOT( A, B, nslong );  }

nslong ns_vector4s_dot( const NsVector4s *A, const NsVector4s *B )
   {  _NS_VECTOR4_DOT( A, B, nslong );  }


#define _NS_VECTOR3_CROSS( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
   ns_assert( (C) != (A) && (C) != (B) );\
	_NS_VECTOR3( (C), =, (A)->y*(B)->z - (A)->z*(B)->y, (A)->z*(B)->x - (A)->x*(B)->z, (A)->x*(B)->y - (A)->y*(B)->x )

NsVector3d* ns_vector3d_cross( NsVector3d *C, const NsVector3d *A, const NsVector3d *B )
   {  _NS_VECTOR3_CROSS( C, A, B );  }

NsVector3f* ns_vector3f_cross( NsVector3f *C, const NsVector3f *A, const NsVector3f *B )
   {  _NS_VECTOR3_CROSS( C, A, B );  }

NsVector3l* ns_vector3l_cross( NsVector3l *C, const NsVector3l *A, const NsVector3l *B )
   {  _NS_VECTOR3_CROSS( C, A, B );  }

NsVector3i* ns_vector3i_cross( NsVector3i *C, const NsVector3i *A, const NsVector3i *B )
   {  _NS_VECTOR3_CROSS( C, A, B );  }

NsVector3s* ns_vector3s_cross( NsVector3s *C, const NsVector3s *A, const NsVector3s *B )
   {  _NS_VECTOR3_CROSS( C, A, B );  }



#define _NS_VECTOR_ANGLE( A, B, t, type, acos_func )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	return acos_func(\
				( type )ns_vector##t##_dot( (A), (B) )\
					/\
				( ns_vector##t##_mag( (A) ) * ns_vector##t##_mag( (B) ) )\
				)


nsdouble ns_vector2d_angle( const NsVector2d *A, const NsVector2d *B )
	{  _NS_VECTOR_ANGLE( A, B, 2d, nsdouble, ns_acos );  }

nsdouble ns_vector3d_angle( const NsVector3d *A, const NsVector3d *B )
	{  _NS_VECTOR_ANGLE( A, B, 3d, nsdouble, ns_acos );  }

nsdouble ns_vector4d_angle( const NsVector4d *A, const NsVector4d *B )
	{  _NS_VECTOR_ANGLE( A, B, 4d, nsdouble, ns_acos );  }

nsfloat ns_vector2f_angle( const NsVector2f *A, const NsVector2f *B )
	{  _NS_VECTOR_ANGLE( A, B, 2f, nsfloat, ns_acosf );  }

nsfloat ns_vector3f_angle( const NsVector3f *A, const NsVector3f *B )
	{  _NS_VECTOR_ANGLE( A, B, 3f, nsfloat, ns_acosf );  }

nsfloat ns_vector4f_angle( const NsVector4f *A, const NsVector4f *B )
	{  _NS_VECTOR_ANGLE( A, B, 4f, nsfloat, ns_acosf );  }

nsdouble ns_vector2l_angle( const NsVector2l *A, const NsVector2l *B )
	{  _NS_VECTOR_ANGLE( A, B, 2l, nsdouble, ns_acos );  }

nsdouble ns_vector3l_angle( const NsVector3l *A, const NsVector3l *B )
	{  _NS_VECTOR_ANGLE( A, B, 3l, nsdouble, ns_acos );  }

nsdouble ns_vector4l_angle( const NsVector4l *A, const NsVector4l *B )
	{  _NS_VECTOR_ANGLE( A, B, 4l, nsdouble, ns_acos );  }

nsfloat ns_vector2i_angle( const NsVector2i *A, const NsVector2i *B )
	{  _NS_VECTOR_ANGLE( A, B, 2i, nsfloat, ns_acosf );  }

nsfloat ns_vector3i_angle( const NsVector3i *A, const NsVector3i *B )
	{  _NS_VECTOR_ANGLE( A, B, 3i, nsfloat, ns_acosf );  }

nsfloat ns_vector4i_angle( const NsVector4i *A, const NsVector4i *B )
	{  _NS_VECTOR_ANGLE( A, B, 4i, nsfloat, ns_acosf );  }

nsfloat ns_vector2s_angle( const NsVector2s *A, const NsVector2s *B )
	{  _NS_VECTOR_ANGLE( A, B, 2s, nsfloat, ns_acosf );  }

nsfloat ns_vector3s_angle( const NsVector3s *A, const NsVector3s *B )
	{  _NS_VECTOR_ANGLE( A, B, 3s, nsfloat, ns_acosf );  }

nsfloat ns_vector4s_angle( const NsVector4s *A, const NsVector4s *B )
	{  _NS_VECTOR_ANGLE( A, B, 4s, nsfloat, ns_acosf );  }


NsVector3f* ns_vector3f_invert( NsVector3f* C, const NsVector3f *A )
	{
	ns_assert( NULL != C );
	ns_assert( NULL != A );

	C->x = 1.0f / A->x;
	C->y = 1.0f / A->y;
	C->z = 1.0f / A->z;

	return C;
	}


#define _NS_VECTOR_DISTANCE( A, B, t )\
	NsVector##t C;\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	return ns_vector##t##_mag( ns_vector##t##_sub( &C, (A), (B) ) )

nsdouble ns_vector2d_distance( const NsVector2d *A, const NsVector2d *B )
	{  _NS_VECTOR_DISTANCE( A, B, 2d );  }

nsdouble ns_vector3d_distance( const NsVector3d *A, const NsVector3d *B )
	{  _NS_VECTOR_DISTANCE( A, B, 3d );  }

nsdouble ns_vector4d_distance( const NsVector4d *A, const NsVector4d *B )
	{  _NS_VECTOR_DISTANCE( A, B, 4d );  }

nsfloat ns_vector2f_distance( const NsVector2f *A, const NsVector2f *B )
	{  _NS_VECTOR_DISTANCE( A, B, 2f );  }

nsfloat ns_vector3f_distance( const NsVector3f *A, const NsVector3f *B )
	{  _NS_VECTOR_DISTANCE( A, B, 3f );  }

nsfloat ns_vector4f_distance( const NsVector4f *A, const NsVector4f *B )
	{  _NS_VECTOR_DISTANCE( A, B, 4f );  }

nsdouble ns_vector2l_distance( const NsVector2l *A, const NsVector2l *B )
	{  _NS_VECTOR_DISTANCE( A, B, 2l );  }

nsdouble ns_vector3l_distance( const NsVector3l *A, const NsVector3l *B )
	{  _NS_VECTOR_DISTANCE( A, B, 3l );  }

nsdouble ns_vector4l_distance( const NsVector4l *A, const NsVector4l *B )
	{  _NS_VECTOR_DISTANCE( A, B, 4l );  }

nsfloat ns_vector2i_distance( const NsVector2i *A, const NsVector2i *B )
	{  _NS_VECTOR_DISTANCE( A, B, 2i );  }

nsfloat ns_vector3i_distance( const NsVector3i *A, const NsVector3i *B )
	{  _NS_VECTOR_DISTANCE( A, B, 3i );  }

nsfloat ns_vector4i_distance( const NsVector4i *A, const NsVector4i *B )
	{  _NS_VECTOR_DISTANCE( A, B, 4i );  }

nsfloat ns_vector2s_distance( const NsVector2s *A, const NsVector2s *B )
	{  _NS_VECTOR_DISTANCE( A, B, 2s );  }

nsfloat ns_vector3s_distance( const NsVector3s *A, const NsVector3s *B )
	{  _NS_VECTOR_DISTANCE( A, B, 3s );  }

nsfloat ns_vector4s_distance( const NsVector4s *A, const NsVector4s *B )
	{  _NS_VECTOR_DISTANCE( A, B, 4s );  }


/* djb2 hash algorithm. */
#define _NS_VECTOR_HASH( V, d, type )\
	nssize  i;\
	nsuint  hash;\
	nschar  key[ d * sizeof( type ) ];\
	for( i = 0; i < d; ++i )\
		( ( type* )(key) )[i] = ( ( type* )(V) )[i];\
	hash = 5381;\
	for( i = 0; i < d * sizeof( type ); ++i )\
		hash = ( ( hash << 5 ) + hash ) + key[i];\
	return hash

nsuint ns_vector2d_hash( const NsVector2d *V )
	{  _NS_VECTOR_HASH( V, 2, nsdouble );  }

nsuint ns_vector3d_hash( const NsVector3d *V )
	{  _NS_VECTOR_HASH( V, 3, nsdouble );  }

nsuint ns_vector4d_hash( const NsVector4d *V )
	{  _NS_VECTOR_HASH( V, 4, nsdouble );  }

nsuint ns_vector2f_hash( const NsVector2f *V )
	{  _NS_VECTOR_HASH( V, 2, nsfloat );  }

nsuint ns_vector3f_hash( const NsVector3f *V )
	{  _NS_VECTOR_HASH( V, 3, nsfloat );  }

nsuint ns_vector4f_hash( const NsVector4f *V )
	{  _NS_VECTOR_HASH( V, 4, nsfloat );  }

nsuint ns_vector2l_hash( const NsVector2l *V )
	{  _NS_VECTOR_HASH( V, 2, nslong );  }

nsuint ns_vector3l_hash( const NsVector3l *V )
	{  _NS_VECTOR_HASH( V, 3, nslong );  }

nsuint ns_vector4l_hash( const NsVector4l *V )
	{  _NS_VECTOR_HASH( V, 4, nslong );  }

nsuint ns_vector2i_hash( const NsVector2i *V )
	{  _NS_VECTOR_HASH( V, 2, nsint );  }

nsuint ns_vector3i_hash( const NsVector3i *V )
	{  _NS_VECTOR_HASH( V, 3, nsint );  }

nsuint ns_vector4i_hash( const NsVector4i *V )
	{  _NS_VECTOR_HASH( V, 4, nsint );  }

nsuint ns_vector2s_hash( const NsVector2s *V )
	{  _NS_VECTOR_HASH( V, 2, nsshort );  }

nsuint ns_vector3s_hash( const NsVector3s *V )
	{  _NS_VECTOR_HASH( V, 3, nsshort );  }

nsuint ns_vector4s_hash( const NsVector4s *V )
	{  _NS_VECTOR_HASH( V, 4, nsshort );  }


#define _NS_VECTOR3_ABS( C, A )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_VECTOR3( (C), =, NS_ABS( (A)->x ), NS_ABS( (A)->y ), NS_ABS( (A)->z ) )

NsVector3i* ns_vector3i_abs( NsVector3i *C, const NsVector3i *A )
   {  _NS_VECTOR3_ABS( C, A );  }


#define _NS_VECTOR_2_TO_2( S, D, type )\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (D) );\
	_NS_VECTOR2( (D), =, (type)(S)->x, (type)(S)->y )

#define _NS_VECTOR_2_TO_3( S, D, type )\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (D) );\
	_NS_VECTOR3( (D), =, (type)(S)->x, (type)(S)->y, (type)0 )

#define _NS_VECTOR_2_TO_4( S, D, type )\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (D) );\
	_NS_VECTOR4( (D), =, (type)(S)->x, (type)(S)->y, (type)0, (type)0 )

#define _NS_VECTOR_3_TO_2( S, D, type )\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (D) );\
	_NS_VECTOR2( (D), =, (type)(S)->x, (type)(S)->y )

#define _NS_VECTOR_3_TO_3( S, D, type )\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (D) );\
	_NS_VECTOR3( (D), =, (type)(S)->x, (type)(S)->y, (type)(S)->z )

#define _NS_VECTOR_3_TO_4( S, D, type )\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (D) );\
	_NS_VECTOR4( (D), =, (type)(S)->x, (type)(S)->y, (type)(S)->z, (type)0 )

#define _NS_VECTOR_4_TO_2( S, D, type )\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (D) );\
	_NS_VECTOR2( (D), =, (type)(S)->x, (type)(S)->y )

#define _NS_VECTOR_4_TO_3( S, D, type )\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (D) );\
	_NS_VECTOR3( (D), =, (type)(S)->x, (type)(S)->y, (type)(S)->z )

#define _NS_VECTOR_4_TO_4( S, D, type )\
	ns_assert( NULL != (S) );\
	ns_assert( NULL != (D) );\
	_NS_VECTOR4( (D), =, (type)(S)->x, (type)(S)->y, (type)(S)->z, (type)(S)->w )


NsVector3d* ns_vector2d_to_3d( const NsVector2d *S, NsVector3d *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector2d_to_4d( const NsVector2d *S, NsVector4d *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsdouble );  }

NsVector2f* ns_vector2d_to_2f( const NsVector2d *S, NsVector2f *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector2d_to_3f( const NsVector2d *S, NsVector3f *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector2d_to_4f( const NsVector2d *S, NsVector4f *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsfloat );  }

NsVector2l* ns_vector2d_to_2l( const NsVector2d *S, NsVector2l *D )
	{  _NS_VECTOR_2_TO_2( S, D, nslong );  }

NsVector3l* ns_vector2d_to_3l( const NsVector2d *S, NsVector3l *D )
	{  _NS_VECTOR_2_TO_3( S, D, nslong );  }

NsVector4l* ns_vector2d_to_4l( const NsVector2d *S, NsVector4l *D )
	{  _NS_VECTOR_2_TO_4( S, D, nslong );  }

NsVector2i* ns_vector2d_to_2i( const NsVector2d *S, NsVector2i *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsint );  }

NsVector3i* ns_vector2d_to_3i( const NsVector2d *S, NsVector3i *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsint );  }

NsVector4i* ns_vector2d_to_4i( const NsVector2d *S, NsVector4i *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsint );  }

NsVector2s* ns_vector2d_to_2s( const NsVector2d *S, NsVector2s *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector2d_to_3s( const NsVector2d *S, NsVector3s *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector2d_to_4s( const NsVector2d *S, NsVector4s *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsshort );  }


NsVector2d* ns_vector3d_to_2d( const NsVector3d *S, NsVector2d *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsdouble );  }

NsVector4d* ns_vector3d_to_4d( const NsVector3d *S, NsVector4d *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsdouble );  }

NsVector2f* ns_vector3d_to_2f( const NsVector3d *S, NsVector2f *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector3d_to_3f( const NsVector3d *S, NsVector3f *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector3d_to_4f( const NsVector3d *S, NsVector4f *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsfloat );  }

NsVector2l* ns_vector3d_to_2l( const NsVector3d *S, NsVector2l *D )
	{  _NS_VECTOR_3_TO_2( S, D, nslong );  }

NsVector3l* ns_vector3d_to_3l( const NsVector3d *S, NsVector3l *D )
	{  _NS_VECTOR_3_TO_3( S, D, nslong );  }

NsVector4l* ns_vector3d_to_4l( const NsVector3d *S, NsVector4l *D )
	{  _NS_VECTOR_3_TO_4( S, D, nslong );  }

NsVector2i* ns_vector3d_to_2i( const NsVector3d *S, NsVector2i *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsint );  }

NsVector3i* ns_vector3d_to_3i( const NsVector3d *S, NsVector3i *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsint );  }

NsVector4i* ns_vector3d_to_4i( const NsVector3d *S, NsVector4i *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsint );  }

NsVector2s* ns_vector3d_to_2s( const NsVector3d *S, NsVector2s *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector3d_to_3s( const NsVector3d *S, NsVector3s *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector3d_to_4s( const NsVector3d *S, NsVector4s *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsshort );  }


NsVector2d* ns_vector4d_to_2d( const NsVector4d *S, NsVector2d *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector4d_to_3d( const NsVector4d *S, NsVector3d *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsdouble );  }

NsVector2f* ns_vector4d_to_2f( const NsVector4d *S, NsVector2f *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector4d_to_3f( const NsVector4d *S, NsVector3f *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector4d_to_4f( const NsVector4d *S, NsVector4f *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsfloat );  }

NsVector2l* ns_vector4d_to_2l( const NsVector4d *S, NsVector2l *D )
	{  _NS_VECTOR_4_TO_2( S, D, nslong );  }

NsVector3l* ns_vector4d_to_3l( const NsVector4d *S, NsVector3l *D )
	{  _NS_VECTOR_4_TO_3( S, D, nslong );  }

NsVector4l* ns_vector4d_to_4l( const NsVector4d *S, NsVector4l *D )
	{  _NS_VECTOR_4_TO_4( S, D, nslong );  }

NsVector2i* ns_vector4d_to_2i( const NsVector4d *S, NsVector2i *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsint );  }

NsVector3i* ns_vector4d_to_3i( const NsVector4d *S, NsVector3i *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsint );  }

NsVector4i* ns_vector4d_to_4i( const NsVector4d *S, NsVector4i *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsint );  }

NsVector2s* ns_vector4d_to_2s( const NsVector4d *S, NsVector2s *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector4d_to_3s( const NsVector4d *S, NsVector3s *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector4d_to_4s( const NsVector4d *S, NsVector4s *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsshort );  }


NsVector3f* ns_vector2f_to_3f( const NsVector2f *S, NsVector3f *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector2f_to_4f( const NsVector2f *S, NsVector4f *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsfloat );  }

NsVector2d* ns_vector2f_to_2d( const NsVector2f *S, NsVector2d *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector2f_to_3d( const NsVector2f *S, NsVector3d *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector2f_to_4d( const NsVector2f *S, NsVector4d *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsdouble );  }

NsVector2l* ns_vector2f_to_2l( const NsVector2f *S, NsVector2l *D )
	{  _NS_VECTOR_2_TO_2( S, D, nslong );  }

NsVector3l* ns_vector2f_to_3l( const NsVector2f *S, NsVector3l *D )
	{  _NS_VECTOR_2_TO_3( S, D, nslong );  }

NsVector4l* ns_vector2f_to_4l( const NsVector2f *S, NsVector4l *D )
	{  _NS_VECTOR_2_TO_4( S, D, nslong );  }

NsVector2i* ns_vector2f_to_2i( const NsVector2f *S, NsVector2i *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsint );  }

NsVector3i* ns_vector2f_to_3i( const NsVector2f *S, NsVector3i *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsint );  }

NsVector4i* ns_vector2f_to_4i( const NsVector2f *S, NsVector4i *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsint );  }

NsVector2s* ns_vector2f_to_2s( const NsVector2f *S, NsVector2s *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector2f_to_3s( const NsVector2f *S, NsVector3s *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector2f_to_4s( const NsVector2f *S, NsVector4s *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsshort );  }


NsVector2f* ns_vector3f_to_2f( const NsVector3f *S, NsVector2f *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsfloat );  }

NsVector4f* ns_vector3f_to_4f( const NsVector3f *S, NsVector4f *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsfloat );  }

NsVector2d* ns_vector3f_to_2d( const NsVector3f *S, NsVector2d *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector3f_to_3d( const NsVector3f *S, NsVector3d *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector3f_to_4d( const NsVector3f *S, NsVector4d *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsdouble );  }


NsVector2l* ns_vector3f_to_2l( const NsVector3f *S, NsVector2l *D )
	{  _NS_VECTOR_3_TO_2( S, D, nslong );  }

NsVector3l* ns_vector3f_to_3l( const NsVector3f *S, NsVector3l *D )
	{  _NS_VECTOR_3_TO_3( S, D, nslong );  }

NsVector4l* ns_vector3f_to_4l( const NsVector3f *S, NsVector4l *D )
	{  _NS_VECTOR_3_TO_4( S, D, nslong );  }

NsVector2i* ns_vector3f_to_2i( const NsVector3f *S, NsVector2i *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsint );  }

NsVector3i* ns_vector3f_to_3i( const NsVector3f *S, NsVector3i *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsint );  }

NsVector4i* ns_vector3f_to_4i( const NsVector3f *S, NsVector4i *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsint );  }

NsVector2s* ns_vector3f_to_2s( const NsVector3f *S, NsVector2s *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector3f_to_3s( const NsVector3f *S, NsVector3s *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector3f_to_4s( const NsVector3f *S, NsVector4s *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsshort );  }


NsVector2f* ns_vector4f_to_2f( const NsVector4f *S, NsVector2f *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector4f_to_3f( const NsVector4f *S, NsVector3f *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsfloat );  }

NsVector2d* ns_vector4f_to_2d( const NsVector4f *S, NsVector2d *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector4f_to_3d( const NsVector4f *S, NsVector3d *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector4f_to_4d( const NsVector4f *S, NsVector4d *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsdouble );  }

NsVector2l* ns_vector4f_to_2l( const NsVector4f *S, NsVector2l *D )
	{  _NS_VECTOR_4_TO_2( S, D, nslong );  }

NsVector3l* ns_vector4f_to_3l( const NsVector4f *S, NsVector3l *D )
	{  _NS_VECTOR_4_TO_3( S, D, nslong );  }

NsVector4l* ns_vector4f_to_4l( const NsVector4f *S, NsVector4l *D )
	{  _NS_VECTOR_4_TO_4( S, D, nslong );  }

NsVector2i* ns_vector4f_to_2i( const NsVector4f *S, NsVector2i *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsint );  }

NsVector3i* ns_vector4f_to_3i( const NsVector4f *S, NsVector3i *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsint );  }

NsVector4i* ns_vector4f_to_4i( const NsVector4f *S, NsVector4i *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsint );  }

NsVector2s* ns_vector4f_to_2s( const NsVector4f *S, NsVector2s *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector4f_to_3s( const NsVector4f *S, NsVector3s *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector4f_to_4s( const NsVector4f *S, NsVector4s *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsshort );  }


NsVector3l* ns_vector2l_to_3l( const NsVector2l *S, NsVector3l *D )
	{  _NS_VECTOR_2_TO_3( S, D, nslong );  }

NsVector4l* ns_vector2l_to_4l( const NsVector2l *S, NsVector4l *D )
	{  _NS_VECTOR_2_TO_4( S, D, nslong );  }

NsVector2d* ns_vector2l_to_2d( const NsVector2l *S, NsVector2d *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector2l_to_3d( const NsVector2l *S, NsVector3d *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector2l_to_4d( const NsVector2l *S, NsVector4d *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsdouble );  }

NsVector2f* ns_vector2l_to_2f( const NsVector2l *S, NsVector2f *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector2l_to_3f( const NsVector2l *S, NsVector3f *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector2l_to_4f( const NsVector2l *S, NsVector4f *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsfloat );  }

NsVector2i* ns_vector2l_to_2i( const NsVector2l *S, NsVector2i *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsint );  }

NsVector3i* ns_vector2l_to_3i( const NsVector2l *S, NsVector3i *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsint );  }

NsVector4i* ns_vector2l_to_4i( const NsVector2l *S, NsVector4i *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsint );  }

NsVector2s* ns_vector2l_to_2s( const NsVector2l *S, NsVector2s *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector2l_to_3s( const NsVector2l *S, NsVector3s *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector2l_to_4s( const NsVector2l *S, NsVector4s *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsshort );  }


NsVector2l* ns_vector3l_to_2l( const NsVector3l *S, NsVector2l *D )
	{  _NS_VECTOR_3_TO_2( S, D, nslong );  }

NsVector4l* ns_vector3l_to_4l( const NsVector3l *S, NsVector4l *D )
	{  _NS_VECTOR_3_TO_4( S, D, nslong );  }

NsVector2d* ns_vector3l_to_2d( const NsVector3l *S, NsVector2d *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector3l_to_3d( const NsVector3l *S, NsVector3d *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector3l_to_4d( const NsVector3l *S, NsVector4d *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsdouble );  }

NsVector2f* ns_vector3l_to_2f( const NsVector3l *S, NsVector2f *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector3l_to_3f( const NsVector3l *S, NsVector3f *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector3l_to_4f( const NsVector3l *S, NsVector4f *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsfloat );  }

NsVector2i* ns_vector3l_to_2i( const NsVector3l *S, NsVector2i *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsint );  }

NsVector3i* ns_vector3l_to_3i( const NsVector3l *S, NsVector3i *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsint );  }

NsVector4i* ns_vector3l_to_4i( const NsVector3l *S, NsVector4i *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsint );  }

NsVector2s* ns_vector3l_to_2s( const NsVector3l *S, NsVector2s *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector3l_to_3s( const NsVector3l *S, NsVector3s *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector3l_to_4s( const NsVector3l *S, NsVector4s *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsshort );  }


NsVector2l* ns_vector4l_to_2l( const NsVector4l *S, NsVector2l *D )
	{  _NS_VECTOR_4_TO_2( S, D, nslong );  }

NsVector3l* ns_vector4l_to_3l( const NsVector4l *S, NsVector3l *D )
	{  _NS_VECTOR_4_TO_3( S, D, nslong );  }

NsVector2d* ns_vector4l_to_2d( const NsVector4l *S, NsVector2d *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector4l_to_3d( const NsVector4l *S, NsVector3d *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector4l_to_4d( const NsVector4l *S, NsVector4d *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsdouble );  }

NsVector2f* ns_vector4l_to_2f( const NsVector4l *S, NsVector2f *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector4l_to_3f( const NsVector4l *S, NsVector3f *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector4l_to_4f( const NsVector4l *S, NsVector4f *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsfloat );  }

NsVector2i* ns_vector4l_to_2i( const NsVector4l *S, NsVector2i *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsint );  }

NsVector3i* ns_vector4l_to_3i( const NsVector4l *S, NsVector3i *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsint );  }

NsVector4i* ns_vector4l_to_4i( const NsVector4l *S, NsVector4i *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsint );  }

NsVector2s* ns_vector4l_to_2s( const NsVector4l *S, NsVector2s *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector4l_to_3s( const NsVector4l *S, NsVector3s *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector4l_to_4s( const NsVector4l *S, NsVector4s *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsshort );  }


NsVector3i* ns_vector2i_to_3i( const NsVector2i *S, NsVector3i *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsint );  }

NsVector4i* ns_vector2i_to_4i( const NsVector2i *S, NsVector4i *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsint );  }

NsVector2d* ns_vector2i_to_2d( const NsVector2i *S, NsVector2d *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector2i_to_3d( const NsVector2i *S, NsVector3d *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector2i_to_4d( const NsVector2i *S, NsVector4d *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsdouble );  }

NsVector2f* ns_vector2i_to_2f( const NsVector2i *S, NsVector2f *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector2i_to_3f( const NsVector2i *S, NsVector3f *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector2i_to_4f( const NsVector2i *S, NsVector4f *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsfloat );  }

NsVector2l* ns_vector2i_to_2l( const NsVector2i *S, NsVector2l *D )
	{  _NS_VECTOR_2_TO_2( S, D, nslong );  }

NsVector3l* ns_vector2i_to_3l( const NsVector2i *S, NsVector3l *D )
	{  _NS_VECTOR_2_TO_3( S, D, nslong );  }

NsVector4l* ns_vector2i_to_4l( const NsVector2i *S, NsVector4l *D )
	{  _NS_VECTOR_2_TO_4( S, D, nslong );  }

NsVector2s* ns_vector2i_to_2s( const NsVector2i *S, NsVector2s *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector2i_to_3s( const NsVector2i *S, NsVector3s *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector2i_to_4s( const NsVector2i *S, NsVector4s *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsshort );  }


NsVector2i* ns_vector3i_to_2i( const NsVector3i *S, NsVector2i *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsint );  }

NsVector4i* ns_vector3i_to_4i( const NsVector3i *S, NsVector4i *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsint );  }

NsVector2d* ns_vector3i_to_2d( const NsVector3i *S, NsVector2d *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector3i_to_3d( const NsVector3i *S, NsVector3d *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector3i_to_4d( const NsVector3i *S, NsVector4d *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsdouble );  }

NsVector2f* ns_vector3i_to_2f( const NsVector3i *S, NsVector2f *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector3i_to_3f( const NsVector3i *S, NsVector3f *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector3i_to_4f( const NsVector3i *S, NsVector4f *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsfloat );  }

NsVector2l* ns_vector3i_to_2l( const NsVector3i *S, NsVector2l *D )
	{  _NS_VECTOR_3_TO_2( S, D, nslong );  }

NsVector3l* ns_vector3i_to_3l( const NsVector3i *S, NsVector3l *D )
	{  _NS_VECTOR_3_TO_3( S, D, nslong );  }

NsVector4l* ns_vector3i_to_4l( const NsVector3i *S, NsVector4l *D )
	{  _NS_VECTOR_3_TO_4( S, D, nslong );  }

NsVector2s* ns_vector3i_to_2s( const NsVector3i *S, NsVector2s *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector3i_to_3s( const NsVector3i *S, NsVector3s *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector3i_to_4s( const NsVector3i *S, NsVector4s *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsshort );  }


NsVector2i* ns_vector4i_to_2i( const NsVector4i *S, NsVector2i *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsint );  }

NsVector3i* ns_vector4i_to_3i( const NsVector4i *S, NsVector3i *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsint );  }

NsVector2d* ns_vector4i_to_2d( const NsVector4i *S, NsVector2d *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector4i_to_3d( const NsVector4i *S, NsVector3d *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector4i_to_4d( const NsVector4i *S, NsVector4d *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsdouble );  }

NsVector2f* ns_vector4i_to_2f( const NsVector4i *S, NsVector2f *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector4i_to_3f( const NsVector4i *S, NsVector3f *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector4i_to_4f( const NsVector4i *S, NsVector4f *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsfloat );  }

NsVector2l* ns_vector4i_to_2l( const NsVector4i *S, NsVector2l *D )
	{  _NS_VECTOR_4_TO_2( S, D, nslong );  }

NsVector3l* ns_vector4i_to_3l( const NsVector4i *S, NsVector3l *D )
	{  _NS_VECTOR_4_TO_3( S, D, nslong );  }

NsVector4l* ns_vector4i_to_4l( const NsVector4i *S, NsVector4l *D )
	{  _NS_VECTOR_4_TO_4( S, D, nslong );  }

NsVector2s* ns_vector4i_to_2s( const NsVector4i *S, NsVector2s *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector4i_to_3s( const NsVector4i *S, NsVector3s *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector4i_to_4s( const NsVector4i *S, NsVector4s *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsshort );  }


NsVector3s* ns_vector2s_to_3s( const NsVector2s *S, NsVector3s *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsshort );  }

NsVector4s* ns_vector2s_to_4s( const NsVector2s *S, NsVector4s *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsshort );  }

NsVector2d* ns_vector2s_to_2d( const NsVector2s *S, NsVector2d *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector2s_to_3d( const NsVector2s *S, NsVector3d *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector2s_to_4d( const NsVector2s *S, NsVector4d *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsdouble );  }

NsVector2f* ns_vector2s_to_2f( const NsVector2s *S, NsVector2f *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector2s_to_3f( const NsVector2s *S, NsVector3f *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector2s_to_4f( const NsVector2s *S, NsVector4f *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsfloat );  }

NsVector2l* ns_vector2s_to_2l( const NsVector2s *S, NsVector2l *D )
	{  _NS_VECTOR_2_TO_2( S, D, nslong );  }

NsVector3l* ns_vector2s_to_3l( const NsVector2s *S, NsVector3l *D )
	{  _NS_VECTOR_2_TO_3( S, D, nslong );  }

NsVector4l* ns_vector2s_to_4l( const NsVector2s *S, NsVector4l *D )
	{  _NS_VECTOR_2_TO_4( S, D, nslong );  }

NsVector2i* ns_vector2s_to_2i( const NsVector2s *S, NsVector2i *D )
	{  _NS_VECTOR_2_TO_2( S, D, nsint );  }

NsVector3i* ns_vector2s_to_3i( const NsVector2s *S, NsVector3i *D )
	{  _NS_VECTOR_2_TO_3( S, D, nsint );  }

NsVector4i* ns_vector2s_to_4i( const NsVector2s *S, NsVector4i *D )
	{  _NS_VECTOR_2_TO_4( S, D, nsint );  }


NsVector2s* ns_vector3s_to_2s( const NsVector3s *S, NsVector2s *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsshort );  }

NsVector4s* ns_vector3s_to_4s( const NsVector3s *S, NsVector4s *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsshort );  }

NsVector2d* ns_vector3s_to_2d( const NsVector3s *S, NsVector2d *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector3s_to_3d( const NsVector3s *S, NsVector3d *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector3s_to_4d( const NsVector3s *S, NsVector4d *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsdouble );  }

NsVector2f* ns_vector3s_to_2f( const NsVector3s *S, NsVector2f *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector3s_to_3f( const NsVector3s *S, NsVector3f *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector3s_to_4f( const NsVector3s *S, NsVector4f *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsfloat );  }

NsVector2l* ns_vector3s_to_2l( const NsVector3s *S, NsVector2l *D )
	{  _NS_VECTOR_3_TO_2( S, D, nslong );  }

NsVector3l* ns_vector3s_to_3l( const NsVector3s *S, NsVector3l *D )
	{  _NS_VECTOR_3_TO_3( S, D, nslong );  }

NsVector4l* ns_vector3s_to_4l( const NsVector3s *S, NsVector4l *D )
	{  _NS_VECTOR_3_TO_4( S, D, nslong );  }

NsVector2i* ns_vector3s_to_2i( const NsVector3s *S, NsVector2i *D )
	{  _NS_VECTOR_3_TO_2( S, D, nsint );  }

NsVector3i* ns_vector3s_to_3i( const NsVector3s *S, NsVector3i *D )
	{  _NS_VECTOR_3_TO_3( S, D, nsint );  }

NsVector4i* ns_vector3s_to_4i( const NsVector3s *S, NsVector4i *D )
	{  _NS_VECTOR_3_TO_4( S, D, nsint );  }


NsVector2s* ns_vector4s_to_2s( const NsVector4s *S, NsVector2s *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsshort );  }

NsVector3s* ns_vector4s_to_3s( const NsVector4s *S, NsVector3s *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsshort );  }

NsVector2d* ns_vector4s_to_2d( const NsVector4s *S, NsVector2d *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsdouble );  }

NsVector3d* ns_vector4s_to_3d( const NsVector4s *S, NsVector3d *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsdouble );  }

NsVector4d* ns_vector4s_to_4d( const NsVector4s *S, NsVector4d *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsdouble );  }

NsVector2f* ns_vector4s_to_2f( const NsVector4s *S, NsVector2f *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsfloat );  }

NsVector3f* ns_vector4s_to_3f( const NsVector4s *S, NsVector3f *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsfloat );  }

NsVector4f* ns_vector4s_to_4f( const NsVector4s *S, NsVector4f *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsfloat );  }

NsVector2l* ns_vector4s_to_2l( const NsVector4s *S, NsVector2l *D )
	{  _NS_VECTOR_4_TO_2( S, D, nslong );  }

NsVector3l* ns_vector4s_to_3l( const NsVector4s *S, NsVector3l *D )
	{  _NS_VECTOR_4_TO_3( S, D, nslong );  }

NsVector4l* ns_vector4s_to_4l( const NsVector4s *S, NsVector4l *D )
	{  _NS_VECTOR_4_TO_4( S, D, nslong );  }

NsVector2i* ns_vector4s_to_2i( const NsVector4s *S, NsVector2i *D )
	{  _NS_VECTOR_4_TO_2( S, D, nsint );  }

NsVector3i* ns_vector4s_to_3i( const NsVector4s *S, NsVector3i *D )
	{  _NS_VECTOR_4_TO_3( S, D, nsint );  }

NsVector4i* ns_vector4s_to_4i( const NsVector4s *S, NsVector4i *D )
	{  _NS_VECTOR_4_TO_4( S, D, nsint );  }
