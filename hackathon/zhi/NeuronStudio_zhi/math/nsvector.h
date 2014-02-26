#ifndef __NS_MATH_VECTOR_H__
#define __NS_MATH_VECTOR_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nsmath.h>
#include <math/nsvectordecls.h>
#include <math/nsmatrixdecls.h>
#include <math/nsaabboxdecls.h>

NS_DECLS_BEGIN

typedef enum
	{
	NS_VECTOR_UNIT_I,  /* 1, 0, 0, 0 */
	NS_VECTOR_UNIT_J,	/* 0, 1, 0, 0 */
	NS_VECTOR_UNIT_K,  /* 0, 0, 1, 0 */
	NS_VECTOR_UNIT_L   /* 0, 0, 0, 1 */
	}
	NsVectorUnitType;


/* Purposely tried to keep them short. */
#define NS_VS_X  1
#define NS_VS_Y  2
#define NS_VS_Z  3
#define NS_VS_W  4

/* The parameters for these macros are defined above. */
#define NS_VECTOR_SWIZZLE1( s1 )\
	(((nsuint)(s1)))
#define NS_VECTOR_SWIZZLE2( s1, s2 )\
	(((nsuint)(s1))|(((nsuint)(s2))<<3))
#define NS_VECTOR_SWIZZLE3( s1, s2, s3 )\
	(((nsuint)(s1))|(((nsuint)(s2))<<3)|(((nsuint)(s3))<<6))
#define NS_VECTOR_SWIZZLE4( s1, s2, s3, s4 )\
	(((nsuint)(s1))|(((nsuint)(s2))<<3)|(((nsuint)(s3))<<6)|(((nsuint)(s4))<<9))


/* Returns V */
NS_IMPEXP NsVector2d* ns_vector2d( NsVector2d *V, nsdouble x, nsdouble y );
NS_IMPEXP NsVector3d* ns_vector3d( NsVector3d *V, nsdouble x, nsdouble y, nsdouble z );
NS_IMPEXP NsVector4d* ns_vector4d( NsVector4d *V, nsdouble x, nsdouble y, nsdouble z, nsdouble w );

NS_IMPEXP NsVector2f* ns_vector2f( NsVector2f *V, nsfloat x, nsfloat y );
NS_IMPEXP NsVector3f* ns_vector3f( NsVector3f *V, nsfloat x, nsfloat y, nsfloat z );
NS_IMPEXP NsVector4f* ns_vector4f( NsVector4f *V, nsfloat x, nsfloat y, nsfloat z, nsfloat w );

NS_IMPEXP NsVector2l* ns_vector2l( NsVector2l *V, nslong x, nslong y );
NS_IMPEXP NsVector3l* ns_vector3l( NsVector3l *V, nslong x, nslong y, nslong z );
NS_IMPEXP NsVector4l* ns_vector4l( NsVector4l *V, nslong x, nslong y, nslong z, nslong w );

NS_IMPEXP NsVector2i* ns_vector2i( NsVector2i *V, nsint x, nsint y );
NS_IMPEXP NsVector3i* ns_vector3i( NsVector3i *V, nsint x, nsint y, nsint z );
NS_IMPEXP NsVector4i* ns_vector4i( NsVector4i *V, nsint x, nsint y, nsint z, nsint w );

NS_IMPEXP NsVector2s* ns_vector2s( NsVector2s *V, nsshort x, nsshort y );
NS_IMPEXP NsVector3s* ns_vector3s( NsVector3s *V, nsshort x, nsshort y, nsshort z );
NS_IMPEXP NsVector4s* ns_vector4s( NsVector4s *V, nsshort x, nsshort y, nsshort z, nsshort w );


/* Returns V as an array. */
NS_IMPEXP nsdouble* ns_vector2d_array( NsVector2d *V );
NS_IMPEXP nsdouble* ns_vector3d_array( NsVector3d *V );
NS_IMPEXP nsdouble* ns_vector4d_array( NsVector4d *V );

NS_IMPEXP nsfloat* ns_vector2f_array( NsVector2f *V );
NS_IMPEXP nsfloat* ns_vector3f_array( NsVector3f *V );
NS_IMPEXP nsfloat* ns_vector4f_array( NsVector4f *V );

NS_IMPEXP nslong* ns_vector2l_array( NsVector2l *V );
NS_IMPEXP nslong* ns_vector3l_array( NsVector3l *V );
NS_IMPEXP nslong* ns_vector4l_array( NsVector4l *V );

NS_IMPEXP nsint* ns_vector2i_array( NsVector2i *V );
NS_IMPEXP nsint* ns_vector3i_array( NsVector3i *V );
NS_IMPEXP nsint* ns_vector4i_array( NsVector4i *V );

NS_IMPEXP nsshort* ns_vector2s_array( NsVector2s *V );
NS_IMPEXP nsshort* ns_vector3s_array( NsVector3s *V );
NS_IMPEXP nsshort* ns_vector4s_array( NsVector4s *V );


NS_IMPEXP const nsdouble* ns_vector2d_const_array( const NsVector2d *V );
NS_IMPEXP const nsdouble* ns_vector3d_const_array( const NsVector3d *V );
NS_IMPEXP const nsdouble* ns_vector4d_const_array( const NsVector4d *V );

NS_IMPEXP const nsfloat* ns_vector2f_const_array( const NsVector2f *V );
NS_IMPEXP const nsfloat* ns_vector3f_const_array( const NsVector3f *V );
NS_IMPEXP const nsfloat* ns_vector4f_const_array( const NsVector4f *V );

NS_IMPEXP const nslong* ns_vector2l_const_array( const NsVector2l *V );
NS_IMPEXP const nslong* ns_vector3l_const_array( const NsVector3l *V );
NS_IMPEXP const nslong* ns_vector4l_const_array( const NsVector4l *V );

NS_IMPEXP const nsint* ns_vector2i_const_array( const NsVector2i *V );
NS_IMPEXP const nsint* ns_vector3i_const_array( const NsVector3i *V );
NS_IMPEXP const nsint* ns_vector4i_const_array( const NsVector4i *V );

NS_IMPEXP const nsshort* ns_vector2s_const_array( const NsVector2s *V );
NS_IMPEXP const nsshort* ns_vector3s_const_array( const NsVector3s *V );
NS_IMPEXP const nsshort* ns_vector4s_const_array( const NsVector4s *V );


/* C = A + B
   Returns C */
NS_IMPEXP NsVector2d* ns_vector2d_add( NsVector2d *C, const NsVector2d *A, const NsVector2d *B );
NS_IMPEXP NsVector3d* ns_vector3d_add( NsVector3d *C, const NsVector3d *A, const NsVector3d *B );
NS_IMPEXP NsVector4d* ns_vector4d_add( NsVector4d *C, const NsVector4d *A, const NsVector4d *B );

NS_IMPEXP NsVector2f* ns_vector2f_add( NsVector2f *C, const NsVector2f *A, const NsVector2f *B );
NS_IMPEXP NsVector3f* ns_vector3f_add( NsVector3f *C, const NsVector3f *A, const NsVector3f *B );
NS_IMPEXP NsVector4f* ns_vector4f_add( NsVector4f *C, const NsVector4f *A, const NsVector4f *B );

NS_IMPEXP NsVector2l* ns_vector2l_add( NsVector2l *C, const NsVector2l *A, const NsVector2l *B );
NS_IMPEXP NsVector3l* ns_vector3l_add( NsVector3l *C, const NsVector3l *A, const NsVector3l *B );
NS_IMPEXP NsVector4l* ns_vector4l_add( NsVector4l *C, const NsVector4l *A, const NsVector4l *B );

NS_IMPEXP NsVector2i* ns_vector2i_add( NsVector2i *C, const NsVector2i *A, const NsVector2i *B );
NS_IMPEXP NsVector3i* ns_vector3i_add( NsVector3i *C, const NsVector3i *A, const NsVector3i *B );
NS_IMPEXP NsVector4i* ns_vector4i_add( NsVector4i *C, const NsVector4i *A, const NsVector4i *B );

NS_IMPEXP NsVector2s* ns_vector2s_add( NsVector2s *C, const NsVector2s *A, const NsVector2s *B );
NS_IMPEXP NsVector3s* ns_vector3s_add( NsVector3s *C, const NsVector3s *A, const NsVector3s *B );
NS_IMPEXP NsVector4s* ns_vector4s_add( NsVector4s *C, const NsVector4s *A, const NsVector4s *B );


/* A += B
	Returns A */
NS_IMPEXP NsVector2d* ns_vector2d_cmpd_add( NsVector2d *A, const NsVector2d *B );
NS_IMPEXP NsVector3d* ns_vector3d_cmpd_add( NsVector3d *A, const NsVector3d *B );
NS_IMPEXP NsVector4d* ns_vector4d_cmpd_add( NsVector4d *A, const NsVector4d *B );

NS_IMPEXP NsVector2f* ns_vector2f_cmpd_add( NsVector2f *A, const NsVector2f *B );
NS_IMPEXP NsVector3f* ns_vector3f_cmpd_add( NsVector3f *A, const NsVector3f *B );
NS_IMPEXP NsVector4f* ns_vector4f_cmpd_add( NsVector4f *A, const NsVector4f *B );

NS_IMPEXP NsVector2l* ns_vector2l_cmpd_add( NsVector2l *A, const NsVector2l *B );
NS_IMPEXP NsVector3l* ns_vector3l_cmpd_add( NsVector3l *A, const NsVector3l *B );
NS_IMPEXP NsVector4l* ns_vector4l_cmpd_add( NsVector4l *A, const NsVector4l *B );

NS_IMPEXP NsVector2i* ns_vector2i_cmpd_add( NsVector2i *A, const NsVector2i *B );
NS_IMPEXP NsVector3i* ns_vector3i_cmpd_add( NsVector3i *A, const NsVector3i *B );
NS_IMPEXP NsVector4i* ns_vector4i_cmpd_add( NsVector4i *A, const NsVector4i *B );

NS_IMPEXP NsVector2s* ns_vector2s_cmpd_add( NsVector2s *A, const NsVector2s *B );
NS_IMPEXP NsVector3s* ns_vector3s_cmpd_add( NsVector3s *A, const NsVector3s *B );
NS_IMPEXP NsVector4s* ns_vector4s_cmpd_add( NsVector4s *A, const NsVector4s *B );


/* C = A - B  
   Returns C */
NS_IMPEXP NsVector2d* ns_vector2d_sub( NsVector2d *C, const NsVector2d *A, const NsVector2d *B );
NS_IMPEXP NsVector3d* ns_vector3d_sub( NsVector3d *C, const NsVector3d *A, const NsVector3d *B );
NS_IMPEXP NsVector4d* ns_vector4d_sub( NsVector4d *C, const NsVector4d *A, const NsVector4d *B );

NS_IMPEXP NsVector2f* ns_vector2f_sub( NsVector2f *C, const NsVector2f *A, const NsVector2f *B );
NS_IMPEXP NsVector3f* ns_vector3f_sub( NsVector3f *C, const NsVector3f *A, const NsVector3f *B );
NS_IMPEXP NsVector4f* ns_vector4f_sub( NsVector4f *C, const NsVector4f *A, const NsVector4f *B );

NS_IMPEXP NsVector2l* ns_vector2l_sub( NsVector2l *C, const NsVector2l *A, const NsVector2l *B );
NS_IMPEXP NsVector3l* ns_vector3l_sub( NsVector3l *C, const NsVector3l *A, const NsVector3l *B );
NS_IMPEXP NsVector4l* ns_vector4l_sub( NsVector4l *C, const NsVector4l *A, const NsVector4l *B );

NS_IMPEXP NsVector2i* ns_vector2i_sub( NsVector2i *C, const NsVector2i *A, const NsVector2i *B );
NS_IMPEXP NsVector3i* ns_vector3i_sub( NsVector3i *C, const NsVector3i *A, const NsVector3i *B );
NS_IMPEXP NsVector4i* ns_vector4i_sub( NsVector4i *C, const NsVector4i *A, const NsVector4i *B );

NS_IMPEXP NsVector2s* ns_vector2s_sub( NsVector2s *C, const NsVector2s *A, const NsVector2s *B );
NS_IMPEXP NsVector3s* ns_vector3s_sub( NsVector3s *C, const NsVector3s *A, const NsVector3s *B );
NS_IMPEXP NsVector4s* ns_vector4s_sub( NsVector4s *C, const NsVector4s *A, const NsVector4s *B );


/* A -= B
	Returns A */
NS_IMPEXP NsVector2d* ns_vector2d_cmpd_sub( NsVector2d *A, const NsVector2d *B );
NS_IMPEXP NsVector3d* ns_vector3d_cmpd_sub( NsVector3d *A, const NsVector3d *B );
NS_IMPEXP NsVector4d* ns_vector4d_cmpd_sub( NsVector4d *A, const NsVector4d *B );

NS_IMPEXP NsVector2f* ns_vector2f_cmpd_sub( NsVector2f *A, const NsVector2f *B );
NS_IMPEXP NsVector3f* ns_vector3f_cmpd_sub( NsVector3f *A, const NsVector3f *B );
NS_IMPEXP NsVector4f* ns_vector4f_cmpd_sub( NsVector4f *A, const NsVector4f *B );

NS_IMPEXP NsVector2l* ns_vector2l_cmpd_sub( NsVector2l *A, const NsVector2l *B );
NS_IMPEXP NsVector3l* ns_vector3l_cmpd_sub( NsVector3l *A, const NsVector3l *B );
NS_IMPEXP NsVector4l* ns_vector4l_cmpd_sub( NsVector4l *A, const NsVector4l *B );

NS_IMPEXP NsVector2i* ns_vector2i_cmpd_sub( NsVector2i *A, const NsVector2i *B );
NS_IMPEXP NsVector3i* ns_vector3i_cmpd_sub( NsVector3i *A, const NsVector3i *B );
NS_IMPEXP NsVector4i* ns_vector4i_cmpd_sub( NsVector4i *A, const NsVector4i *B );

NS_IMPEXP NsVector2s* ns_vector2s_cmpd_sub( NsVector2s *A, const NsVector2s *B );
NS_IMPEXP NsVector3s* ns_vector3s_cmpd_sub( NsVector3s *A, const NsVector3s *B );
NS_IMPEXP NsVector4s* ns_vector4s_cmpd_sub( NsVector4s *A, const NsVector4s *B );


/* M = A * B
	Returns M */
NS_IMPEXP NsMatrix33d* ns_vector3d_mul( NsMatrix33d *M, const NsVector3d *A, const NsVector3d *B );
NS_IMPEXP NsMatrix44d* ns_vector4d_mul( NsMatrix44d *M, const NsVector4d *A, const NsVector4d *B );

NS_IMPEXP NsMatrix33f* ns_vector3f_mul( NsMatrix33f *M, const NsVector3f *A, const NsVector3f *B );
NS_IMPEXP NsMatrix44f* ns_vector4f_mul( NsMatrix44f *M, const NsVector4f *A, const NsVector4f *B );

NS_IMPEXP NsMatrix33l* ns_vector3l_mul( NsMatrix33l *M, const NsVector3l *A, const NsVector3l *B );
NS_IMPEXP NsMatrix44l* ns_vector4l_mul( NsMatrix44l *M, const NsVector4l *A, const NsVector4l *B );


/* C = A * s
	Returns C */
NS_IMPEXP NsVector2d* ns_vector2d_scale( NsVector2d *C, const NsVector2d *A, nsdouble s );
NS_IMPEXP NsVector3d* ns_vector3d_scale( NsVector3d *C, const NsVector3d *A, nsdouble s );
NS_IMPEXP NsVector4d* ns_vector4d_scale( NsVector4d *C, const NsVector4d *A, nsdouble s );

NS_IMPEXP NsVector2f* ns_vector2f_scale( NsVector2f *C, const NsVector2f *A, nsfloat s );
NS_IMPEXP NsVector3f* ns_vector3f_scale( NsVector3f *C, const NsVector3f *A, nsfloat s );
NS_IMPEXP NsVector4f* ns_vector4f_scale( NsVector4f *C, const NsVector4f *A, nsfloat s );

NS_IMPEXP NsVector2l* ns_vector2l_scale( NsVector2l *C, const NsVector2l *A, nslong s );
NS_IMPEXP NsVector3l* ns_vector3l_scale( NsVector3l *C, const NsVector3l *A, nslong s );
NS_IMPEXP NsVector4l* ns_vector4l_scale( NsVector4l *C, const NsVector4l *A, nslong s );

NS_IMPEXP NsVector2i* ns_vector2i_scale( NsVector2i *C, const NsVector2i *A, nsint s );
NS_IMPEXP NsVector3i* ns_vector3i_scale( NsVector3i *C, const NsVector3i *A, nsint s );
NS_IMPEXP NsVector4i* ns_vector4i_scale( NsVector4i *C, const NsVector4i *A, nsint s );

NS_IMPEXP NsVector2s* ns_vector2s_scale( NsVector2s *C, const NsVector2s *A, nsshort s );
NS_IMPEXP NsVector3s* ns_vector3s_scale( NsVector3s *C, const NsVector3s *A, nsshort s );
NS_IMPEXP NsVector4s* ns_vector4s_scale( NsVector4s *C, const NsVector4s *A, nsshort s );


/* V *= s
   Returns V */
NS_IMPEXP NsVector2d* ns_vector2d_cmpd_scale( NsVector2d *V, nsdouble s );
NS_IMPEXP NsVector3d* ns_vector3d_cmpd_scale( NsVector3d *V, nsdouble s );
NS_IMPEXP NsVector4d* ns_vector4d_cmpd_scale( NsVector4d *V, nsdouble s );

NS_IMPEXP NsVector2f* ns_vector2f_cmpd_scale( NsVector2f *V, nsfloat s );
NS_IMPEXP NsVector3f* ns_vector3f_cmpd_scale( NsVector3f *V, nsfloat s );
NS_IMPEXP NsVector4f* ns_vector4f_cmpd_scale( NsVector4f *V, nsfloat s );

NS_IMPEXP NsVector2l* ns_vector2l_cmpd_scale( NsVector2l *V, nslong s );
NS_IMPEXP NsVector3l* ns_vector3l_cmpd_scale( NsVector3l *V, nslong s );
NS_IMPEXP NsVector4l* ns_vector4l_cmpd_scale( NsVector4l *V, nslong s );

NS_IMPEXP NsVector2i* ns_vector2i_cmpd_scale( NsVector2i *V, nsint s );
NS_IMPEXP NsVector3i* ns_vector3i_cmpd_scale( NsVector3i *V, nsint s );
NS_IMPEXP NsVector4i* ns_vector4i_cmpd_scale( NsVector4i *V, nsint s );

NS_IMPEXP NsVector2s* ns_vector2s_cmpd_scale( NsVector2s *V, nsshort s );
NS_IMPEXP NsVector3s* ns_vector3s_cmpd_scale( NsVector3s *V, nsshort s );
NS_IMPEXP NsVector4s* ns_vector4s_cmpd_scale( NsVector4s *V, nsshort s );


/* C->components[i] = A->components[i] * S->components[i] 
	returns C */
NS_IMPEXP NsVector2d* ns_vector2d_non_uni_scale( NsVector2d *C, const NsVector2d *A, const NsVector2d *S );
NS_IMPEXP NsVector3d* ns_vector3d_non_uni_scale( NsVector3d *C, const NsVector3d *A, const NsVector3d *S );
NS_IMPEXP NsVector4d* ns_vector4d_non_uni_scale( NsVector4d *C, const NsVector4d *A, const NsVector4d *S );

NS_IMPEXP NsVector2f* ns_vector2f_non_uni_scale( NsVector2f *C, const NsVector2f *A, const NsVector2f *S );
NS_IMPEXP NsVector3f* ns_vector3f_non_uni_scale( NsVector3f *C, const NsVector3f *A, const NsVector3f *S );
NS_IMPEXP NsVector4f* ns_vector4f_non_uni_scale( NsVector4f *C, const NsVector4f *A, const NsVector4f *S );

NS_IMPEXP NsVector2l* ns_vector2l_non_uni_scale( NsVector2l *C, const NsVector2l *A, const NsVector2l *S );
NS_IMPEXP NsVector3l* ns_vector3l_non_uni_scale( NsVector3l *C, const NsVector3l *A, const NsVector3l *S );
NS_IMPEXP NsVector4l* ns_vector4l_non_uni_scale( NsVector4l *C, const NsVector4l *A, const NsVector4l *S );

NS_IMPEXP NsVector2i* ns_vector2i_non_uni_scale( NsVector2i *C, const NsVector2i *A, const NsVector2i *S );
NS_IMPEXP NsVector3i* ns_vector3i_non_uni_scale( NsVector3i *C, const NsVector3i *A, const NsVector3i *S );
NS_IMPEXP NsVector4i* ns_vector4i_non_uni_scale( NsVector4i *C, const NsVector4i *A, const NsVector4i *S );

NS_IMPEXP NsVector2s* ns_vector2s_non_uni_scale( NsVector2s *C, const NsVector2s *A, const NsVector2s *S );
NS_IMPEXP NsVector3s* ns_vector3s_non_uni_scale( NsVector3s *C, const NsVector3s *A, const NsVector3s *S );
NS_IMPEXP NsVector4s* ns_vector4s_non_uni_scale( NsVector4s *C, const NsVector4s *A, const NsVector4s *S );


/* V->components[i] * S->components[i]
   Returns V */
NS_IMPEXP NsVector2d* ns_vector2d_cmpd_non_uni_scale( NsVector2d *V, const NsVector2d *S );
NS_IMPEXP NsVector3d* ns_vector3d_cmpd_non_uni_scale( NsVector3d *V, const NsVector3d *S );
NS_IMPEXP NsVector4d* ns_vector4d_cmpd_non_uni_scale( NsVector4d *V, const NsVector4d *S );

NS_IMPEXP NsVector2f* ns_vector2f_cmpd_non_uni_scale( NsVector2f *V, const NsVector2f *S );
NS_IMPEXP NsVector3f* ns_vector3f_cmpd_non_uni_scale( NsVector3f *V, const NsVector3f *S );
NS_IMPEXP NsVector4f* ns_vector4f_cmpd_non_uni_scale( NsVector4f *V, const NsVector4f *S );

NS_IMPEXP NsVector2l* ns_vector2l_cmpd_non_uni_scale( NsVector2l *V, const NsVector2l *S );
NS_IMPEXP NsVector3l* ns_vector3l_cmpd_non_uni_scale( NsVector3l *V, const NsVector3l *S );
NS_IMPEXP NsVector4l* ns_vector4l_cmpd_non_uni_scale( NsVector4l *V, const NsVector4l *S );

NS_IMPEXP NsVector2i* ns_vector2i_cmpd_non_uni_scale( NsVector2i *V, const NsVector2i *S );
NS_IMPEXP NsVector3i* ns_vector3i_cmpd_non_uni_scale( NsVector3i *V, const NsVector3i *S );
NS_IMPEXP NsVector4i* ns_vector4i_cmpd_non_uni_scale( NsVector4i *V, const NsVector4i *S );

NS_IMPEXP NsVector2s* ns_vector2s_cmpd_non_uni_scale( NsVector2s *V, const NsVector2s *S );
NS_IMPEXP NsVector3s* ns_vector3s_cmpd_non_uni_scale( NsVector3s *V, const NsVector3s *S );
NS_IMPEXP NsVector4s* ns_vector4s_cmpd_non_uni_scale( NsVector4s *V, const NsVector4s *S );


/* C = -A
   Returns C */
NS_IMPEXP NsVector2d* ns_vector2d_rev( NsVector2d *C, const NsVector2d *A );
NS_IMPEXP NsVector3d* ns_vector3d_rev( NsVector3d *C, const NsVector3d *A );
NS_IMPEXP NsVector4d* ns_vector4d_rev( NsVector4d *C, const NsVector4d *A );

NS_IMPEXP NsVector2f* ns_vector2f_rev( NsVector2f *C, const NsVector2f *A );
NS_IMPEXP NsVector3f* ns_vector3f_rev( NsVector3f *C, const NsVector3f *A );
NS_IMPEXP NsVector4f* ns_vector4f_rev( NsVector4f *C, const NsVector4f *A );

NS_IMPEXP NsVector2l* ns_vector2l_rev( NsVector2l *C, const NsVector2l *A );
NS_IMPEXP NsVector3l* ns_vector3l_rev( NsVector3l *C, const NsVector3l *A );
NS_IMPEXP NsVector4l* ns_vector4l_rev( NsVector4l *C, const NsVector4l *A );

NS_IMPEXP NsVector2i* ns_vector2i_rev( NsVector2i *C, const NsVector2i *A );
NS_IMPEXP NsVector3i* ns_vector3i_rev( NsVector3i *C, const NsVector3i *A );
NS_IMPEXP NsVector4i* ns_vector4i_rev( NsVector4i *C, const NsVector4i *A );

NS_IMPEXP NsVector2s* ns_vector2s_rev( NsVector2s *C, const NsVector2s *A );
NS_IMPEXP NsVector3s* ns_vector3s_rev( NsVector3s *C, const NsVector3s *A );
NS_IMPEXP NsVector4s* ns_vector4s_rev( NsVector4s *C, const NsVector4s *A );


/* V = -V
   Returns V */
NS_IMPEXP NsVector2d* ns_vector2d_cmpd_rev( NsVector2d *V );
NS_IMPEXP NsVector3d* ns_vector3d_cmpd_rev( NsVector3d *V );
NS_IMPEXP NsVector4d* ns_vector4d_cmpd_rev( NsVector4d *V );

NS_IMPEXP NsVector2f* ns_vector2f_cmpd_rev( NsVector2f *V );
NS_IMPEXP NsVector3f* ns_vector3f_cmpd_rev( NsVector3f *V );
NS_IMPEXP NsVector4f* ns_vector4f_cmpd_rev( NsVector4f *V );

NS_IMPEXP NsVector2l* ns_vector2l_cmpd_rev( NsVector2l *V );
NS_IMPEXP NsVector3l* ns_vector3l_cmpd_rev( NsVector3l *V );
NS_IMPEXP NsVector4l* ns_vector4l_cmpd_rev( NsVector4l *V );

NS_IMPEXP NsVector2i* ns_vector2i_cmpd_rev( NsVector2i *V );
NS_IMPEXP NsVector3i* ns_vector3i_cmpd_rev( NsVector3i *V );
NS_IMPEXP NsVector4i* ns_vector4i_cmpd_rev( NsVector4i *V );

NS_IMPEXP NsVector2s* ns_vector2s_cmpd_rev( NsVector2s *V );
NS_IMPEXP NsVector3s* ns_vector3s_cmpd_rev( NsVector3s *V );
NS_IMPEXP NsVector4s* ns_vector4s_cmpd_rev( NsVector4s *V );


/* V = 0
   Returns V */
NS_IMPEXP NsVector2d* ns_vector2d_zero( NsVector2d *V );
NS_IMPEXP NsVector3d* ns_vector3d_zero( NsVector3d *V );
NS_IMPEXP NsVector4d* ns_vector4d_zero( NsVector4d *V );

NS_IMPEXP NsVector2f* ns_vector2f_zero( NsVector2f *V );
NS_IMPEXP NsVector3f* ns_vector3f_zero( NsVector3f *V );
NS_IMPEXP NsVector4f* ns_vector4f_zero( NsVector4f *V );

NS_IMPEXP NsVector2l* ns_vector2l_zero( NsVector2l *V );
NS_IMPEXP NsVector3l* ns_vector3l_zero( NsVector3l *V );
NS_IMPEXP NsVector4l* ns_vector4l_zero( NsVector4l *V );

NS_IMPEXP NsVector2i* ns_vector2i_zero( NsVector2i *V );
NS_IMPEXP NsVector3i* ns_vector3i_zero( NsVector3i *V );
NS_IMPEXP NsVector4i* ns_vector4i_zero( NsVector4i *V );

NS_IMPEXP NsVector2s* ns_vector2s_zero( NsVector2s *V );
NS_IMPEXP NsVector3s* ns_vector3s_zero( NsVector3s *V );
NS_IMPEXP NsVector4s* ns_vector4s_zero( NsVector4s *V );


/* V = V / ||V||
   Returns V  */
NS_IMPEXP NsVector2d* ns_vector2d_norm( NsVector2d *V );
NS_IMPEXP NsVector3d* ns_vector3d_norm( NsVector3d *V );
NS_IMPEXP NsVector4d* ns_vector4d_norm( NsVector4d *V );

NS_IMPEXP NsVector2f* ns_vector2f_norm( NsVector2f *V );
NS_IMPEXP NsVector3f* ns_vector3f_norm( NsVector3f *V );
NS_IMPEXP NsVector4f* ns_vector4f_norm( NsVector4f *V );


/* Returns ||V|| */
NS_IMPEXP nsdouble ns_vector2d_mag( const NsVector2d *V );
NS_IMPEXP nsdouble ns_vector3d_mag( const NsVector3d *V );
NS_IMPEXP nsdouble ns_vector4d_mag( const NsVector4d *V );

NS_IMPEXP nsfloat ns_vector2f_mag( const NsVector2f *V );
NS_IMPEXP nsfloat ns_vector3f_mag( const NsVector3f *V );
NS_IMPEXP nsfloat ns_vector4f_mag( const NsVector4f *V );

NS_IMPEXP nsdouble ns_vector2l_mag( const NsVector2l *V );
NS_IMPEXP nsdouble ns_vector3l_mag( const NsVector3l *V );
NS_IMPEXP nsdouble ns_vector4l_mag( const NsVector4l *V );

NS_IMPEXP nsfloat ns_vector2i_mag( const NsVector2i *V );
NS_IMPEXP nsfloat ns_vector3i_mag( const NsVector3i *V );
NS_IMPEXP nsfloat ns_vector4i_mag( const NsVector4i *V );

NS_IMPEXP nsfloat ns_vector2s_mag( const NsVector2s *V );
NS_IMPEXP nsfloat ns_vector3s_mag( const NsVector3s *V );
NS_IMPEXP nsfloat ns_vector4s_mag( const NsVector4s *V );


/* Returns A * B */
NS_IMPEXP nsdouble ns_vector2d_dot( const NsVector2d *A, const NsVector2d *B );
NS_IMPEXP nsdouble ns_vector3d_dot( const NsVector3d *A, const NsVector3d *B );
NS_IMPEXP nsdouble ns_vector4d_dot( const NsVector4d *A, const NsVector4d *B );

NS_IMPEXP nsfloat ns_vector2f_dot( const NsVector2f *A, const NsVector2f *B );
NS_IMPEXP nsfloat ns_vector3f_dot( const NsVector3f *A, const NsVector3f *B );
NS_IMPEXP nsfloat ns_vector4f_dot( const NsVector4f *A, const NsVector4f *B );

NS_IMPEXP nslong ns_vector2l_dot( const NsVector2l *A, const NsVector2l *B );
NS_IMPEXP nslong ns_vector3l_dot( const NsVector3l *A, const NsVector3l *B );
NS_IMPEXP nslong ns_vector4l_dot( const NsVector4l *A, const NsVector4l *B );

NS_IMPEXP nslong ns_vector2i_dot( const NsVector2i *A, const NsVector2i *B );
NS_IMPEXP nslong ns_vector3i_dot( const NsVector3i *A, const NsVector3i *B );
NS_IMPEXP nslong ns_vector4i_dot( const NsVector4i *A, const NsVector4i *B );

NS_IMPEXP nslong ns_vector2s_dot( const NsVector2s *A, const NsVector2s *B );
NS_IMPEXP nslong ns_vector3s_dot( const NsVector3s *A, const NsVector3s *B );
NS_IMPEXP nslong ns_vector4s_dot( const NsVector4s *A, const NsVector4s *B );


/* C = A x B 
   Returns C */
NS_IMPEXP NsVector3d* ns_vector3d_cross( NsVector3d *C, const NsVector3d *A, const NsVector3d *B );
NS_IMPEXP NsVector3f* ns_vector3f_cross( NsVector3f *C, const NsVector3f *A, const NsVector3f *B );
NS_IMPEXP NsVector3l* ns_vector3l_cross( NsVector3l *C, const NsVector3l *A, const NsVector3l *B );
NS_IMPEXP NsVector3i* ns_vector3i_cross( NsVector3i *C, const NsVector3i *A, const NsVector3i *B );
NS_IMPEXP NsVector3s* ns_vector3s_cross( NsVector3s *C, const NsVector3s *A, const NsVector3s *B );


/* Return the angle, in radians, between the vectors.
	cos( angle ) = ( A * B ) / ( ||A||||B|| ) */
NS_IMPEXP nsdouble ns_vector2d_angle( const NsVector2d *A, const NsVector2d *B );
NS_IMPEXP nsdouble ns_vector3d_angle( const NsVector3d *A, const NsVector3d *B );
NS_IMPEXP nsdouble ns_vector4d_angle( const NsVector4d *A, const NsVector4d *B );

NS_IMPEXP nsfloat ns_vector2f_angle( const NsVector2f *A, const NsVector2f *B );
NS_IMPEXP nsfloat ns_vector3f_angle( const NsVector3f *A, const NsVector3f *B );
NS_IMPEXP nsfloat ns_vector4f_angle( const NsVector4f *A, const NsVector4f *B );

NS_IMPEXP nsdouble ns_vector2l_angle( const NsVector2l *A, const NsVector2l *B );
NS_IMPEXP nsdouble ns_vector3l_angle( const NsVector3l *A, const NsVector3l *B );
NS_IMPEXP nsdouble ns_vector4l_angle( const NsVector4l *A, const NsVector4l *B );

NS_IMPEXP nsfloat ns_vector2i_angle( const NsVector2i *A, const NsVector2i *B );
NS_IMPEXP nsfloat ns_vector3i_angle( const NsVector3i *A, const NsVector3i *B );
NS_IMPEXP nsfloat ns_vector4i_angle( const NsVector4i *A, const NsVector4i *B );

NS_IMPEXP nsfloat ns_vector2s_angle( const NsVector2s *A, const NsVector2s *B );
NS_IMPEXP nsfloat ns_vector3s_angle( const NsVector3s *A, const NsVector3s *B );
NS_IMPEXP nsfloat ns_vector4s_angle( const NsVector4s *A, const NsVector4s *B );


/* C = 1/A
	Returns C */
NS_IMPEXP NsVector3f* ns_vector3f_invert( NsVector3f* C, const NsVector3f *A );


/* Return the linear distance between 2 points, i.e. vectors.
	Equivalent to mag( sub( A - B ) ) */
NS_IMPEXP nsdouble ns_vector2d_distance( const NsVector2d *A, const NsVector2d *B );
NS_IMPEXP nsdouble ns_vector3d_distance( const NsVector3d *A, const NsVector3d *B );
NS_IMPEXP nsdouble ns_vector4d_distance( const NsVector4d *A, const NsVector4d *B );

NS_IMPEXP nsfloat ns_vector2f_distance( const NsVector2f *A, const NsVector2f *B );
NS_IMPEXP nsfloat ns_vector3f_distance( const NsVector3f *A, const NsVector3f *B );
NS_IMPEXP nsfloat ns_vector4f_distance( const NsVector4f *A, const NsVector4f *B );

NS_IMPEXP nsdouble ns_vector2l_distance( const NsVector2l *A, const NsVector2l *B );
NS_IMPEXP nsdouble ns_vector3l_distance( const NsVector3l *A, const NsVector3l *B );
NS_IMPEXP nsdouble ns_vector4l_distance( const NsVector4l *A, const NsVector4l *B );

NS_IMPEXP nsfloat ns_vector2i_distance( const NsVector2i *A, const NsVector2i *B );
NS_IMPEXP nsfloat ns_vector3i_distance( const NsVector3i *A, const NsVector3i *B );
NS_IMPEXP nsfloat ns_vector4i_distance( const NsVector4i *A, const NsVector4i *B );

NS_IMPEXP nsfloat ns_vector2s_distance( const NsVector2s *A, const NsVector2s *B );
NS_IMPEXP nsfloat ns_vector3s_distance( const NsVector3s *A, const NsVector3s *B );
NS_IMPEXP nsfloat ns_vector4s_distance( const NsVector4s *A, const NsVector4s *B );


/* Returns hash value for the vector. */

NS_IMPEXP nsuint ns_vector2d_hash( const NsVector2d *V );
NS_IMPEXP nsuint ns_vector3d_hash( const NsVector3d *V );
NS_IMPEXP nsuint ns_vector4d_hash( const NsVector4d *V );

NS_IMPEXP nsuint ns_vector2f_hash( const NsVector2f *V );
NS_IMPEXP nsuint ns_vector3f_hash( const NsVector3f *V );
NS_IMPEXP nsuint ns_vector4f_hash( const NsVector4f *V );

NS_IMPEXP nsuint ns_vector2l_hash( const NsVector2l *V );
NS_IMPEXP nsuint ns_vector3l_hash( const NsVector3l *V );
NS_IMPEXP nsuint ns_vector4l_hash( const NsVector4l *V );

NS_IMPEXP nsuint ns_vector2i_hash( const NsVector2i *V );
NS_IMPEXP nsuint ns_vector3i_hash( const NsVector3i *V );
NS_IMPEXP nsuint ns_vector4i_hash( const NsVector4i *V );

NS_IMPEXP nsuint ns_vector2s_hash( const NsVector2s *V );
NS_IMPEXP nsuint ns_vector3s_hash( const NsVector3s *V );
NS_IMPEXP nsuint ns_vector4s_hash( const NsVector4s *V );


/* C = |A|
   Returns C */
NS_IMPEXP NsVector3i* ns_vector3i_abs( NsVector3i *C, const NsVector3i *A );


/* NsVector2d conversions */
NS_IMPEXP NsVector3d* ns_vector2d_to_3d( const NsVector2d *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector2d_to_4d( const NsVector2d *S, NsVector4d *D );

NS_IMPEXP NsVector2f* ns_vector2d_to_2f( const NsVector2d *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector2d_to_3f( const NsVector2d *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector2d_to_4f( const NsVector2d *S, NsVector4f *D );

NS_IMPEXP NsVector2l* ns_vector2d_to_2l( const NsVector2d *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector2d_to_3l( const NsVector2d *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector2d_to_4l( const NsVector2d *S, NsVector4l *D );

NS_IMPEXP NsVector2i* ns_vector2d_to_2i( const NsVector2d *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector2d_to_3i( const NsVector2d *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector2d_to_4i( const NsVector2d *S, NsVector4i *D );

NS_IMPEXP NsVector2s* ns_vector2d_to_2s( const NsVector2d *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector2d_to_3s( const NsVector2d *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector2d_to_4s( const NsVector2d *S, NsVector4s *D );


/* NsVector3d conversions */
NS_IMPEXP NsVector2d* ns_vector3d_to_2d( const NsVector3d *S, NsVector2d *D );
NS_IMPEXP NsVector4d* ns_vector3d_to_4d( const NsVector3d *S, NsVector4d *D );

NS_IMPEXP NsVector2f* ns_vector3d_to_2f( const NsVector3d *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector3d_to_3f( const NsVector3d *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector3d_to_4f( const NsVector3d *S, NsVector4f *D );

NS_IMPEXP NsVector2l* ns_vector3d_to_2l( const NsVector3d *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector3d_to_3l( const NsVector3d *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector3d_to_4l( const NsVector3d *S, NsVector4l *D );

NS_IMPEXP NsVector2i* ns_vector3d_to_2i( const NsVector3d *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector3d_to_3i( const NsVector3d *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector3d_to_4i( const NsVector3d *S, NsVector4i *D );

NS_IMPEXP NsVector2s* ns_vector3d_to_2s( const NsVector3d *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector3d_to_3s( const NsVector3d *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector3d_to_4s( const NsVector3d *S, NsVector4s *D );


/* NsVector4d conversions */
NS_IMPEXP NsVector2d* ns_vector4d_to_2d( const NsVector4d *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector4d_to_3d( const NsVector4d *S, NsVector3d *D );

NS_IMPEXP NsVector2f* ns_vector4d_to_2f( const NsVector4d *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector4d_to_3f( const NsVector4d *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector4d_to_4f( const NsVector4d *S, NsVector4f *D );

NS_IMPEXP NsVector2l* ns_vector4d_to_2l( const NsVector4d *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector4d_to_3l( const NsVector4d *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector4d_to_4l( const NsVector4d *S, NsVector4l *D );

NS_IMPEXP NsVector2i* ns_vector4d_to_2i( const NsVector4d *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector4d_to_3i( const NsVector4d *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector4d_to_4i( const NsVector4d *S, NsVector4i *D );

NS_IMPEXP NsVector2s* ns_vector4d_to_2s( const NsVector4d *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector4d_to_3s( const NsVector4d *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector4d_to_4s( const NsVector4d *S, NsVector4s *D );


/* NsVector2f conversions */
NS_IMPEXP NsVector3f* ns_vector2f_to_3f( const NsVector2f *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector2f_to_4f( const NsVector2f *S, NsVector4f *D );

NS_IMPEXP NsVector2d* ns_vector2f_to_2d( const NsVector2f *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector2f_to_3d( const NsVector2f *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector2f_to_4d( const NsVector2f *S, NsVector4d *D );

NS_IMPEXP NsVector2l* ns_vector2f_to_2l( const NsVector2f *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector2f_to_3l( const NsVector2f *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector2f_to_4l( const NsVector2f *S, NsVector4l *D );

NS_IMPEXP NsVector2i* ns_vector2f_to_2i( const NsVector2f *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector2f_to_3i( const NsVector2f *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector2f_to_4i( const NsVector2f *S, NsVector4i *D );

NS_IMPEXP NsVector2s* ns_vector2f_to_2s( const NsVector2f *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector2f_to_3s( const NsVector2f *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector2f_to_4s( const NsVector2f *S, NsVector4s *D );


/* NsVector3f conversions */
NS_IMPEXP NsVector2f* ns_vector3f_to_2f( const NsVector3f *S, NsVector2f *D );
NS_IMPEXP NsVector4f* ns_vector3f_to_4f( const NsVector3f *S, NsVector4f *D );

NS_IMPEXP NsVector2d* ns_vector3f_to_2d( const NsVector3f *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector3f_to_3d( const NsVector3f *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector3f_to_4d( const NsVector3f *S, NsVector4d *D );

NS_IMPEXP NsVector2l* ns_vector3f_to_2l( const NsVector3f *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector3f_to_3l( const NsVector3f *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector3f_to_4l( const NsVector3f *S, NsVector4l *D );

NS_IMPEXP NsVector2i* ns_vector3f_to_2i( const NsVector3f *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector3f_to_3i( const NsVector3f *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector3f_to_4i( const NsVector3f *S, NsVector4i *D );

NS_IMPEXP NsVector2s* ns_vector3f_to_2s( const NsVector3f *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector3f_to_3s( const NsVector3f *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector3f_to_4s( const NsVector3f *S, NsVector4s *D );


/* NsVector4f conversions */
NS_IMPEXP NsVector2f* ns_vector4f_to_2f( const NsVector4f *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector4f_to_3f( const NsVector4f *S, NsVector3f *D );

NS_IMPEXP NsVector2d* ns_vector4f_to_2d( const NsVector4f *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector4f_to_3d( const NsVector4f *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector4f_to_4d( const NsVector4f *S, NsVector4d *D );

NS_IMPEXP NsVector2l* ns_vector4f_to_2l( const NsVector4f *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector4f_to_3l( const NsVector4f *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector4f_to_4l( const NsVector4f *S, NsVector4l *D );

NS_IMPEXP NsVector2i* ns_vector4f_to_2i( const NsVector4f *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector4f_to_3i( const NsVector4f *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector4f_to_4i( const NsVector4f *S, NsVector4i *D );

NS_IMPEXP NsVector2s* ns_vector4f_to_2s( const NsVector4f *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector4f_to_3s( const NsVector4f *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector4f_to_4s( const NsVector4f *S, NsVector4s *D );


/* NsVector2l conversions */
NS_IMPEXP NsVector3l* ns_vector2l_to_3l( const NsVector2l *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector2l_to_4l( const NsVector2l *S, NsVector4l *D );

NS_IMPEXP NsVector2d* ns_vector2l_to_2d( const NsVector2l *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector2l_to_3d( const NsVector2l *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector2l_to_4d( const NsVector2l *S, NsVector4d *D );

NS_IMPEXP NsVector2f* ns_vector2l_to_2f( const NsVector2l *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector2l_to_3f( const NsVector2l *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector2l_to_4f( const NsVector2l *S, NsVector4f *D );

NS_IMPEXP NsVector2i* ns_vector2l_to_2i( const NsVector2l *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector2l_to_3i( const NsVector2l *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector2l_to_4i( const NsVector2l *S, NsVector4i *D );

NS_IMPEXP NsVector2s* ns_vector2l_to_2s( const NsVector2l *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector2l_to_3s( const NsVector2l *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector2l_to_4s( const NsVector2l *S, NsVector4s *D );


/* NsVector3l conversions */
NS_IMPEXP NsVector2l* ns_vector3l_to_2l( const NsVector3l *S, NsVector2l *D );
NS_IMPEXP NsVector4l* ns_vector3l_to_4l( const NsVector3l *S, NsVector4l *D );

NS_IMPEXP NsVector2d* ns_vector3l_to_2d( const NsVector3l *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector3l_to_3d( const NsVector3l *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector3l_to_4d( const NsVector3l *S, NsVector4d *D );

NS_IMPEXP NsVector2f* ns_vector3l_to_2f( const NsVector3l *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector3l_to_3f( const NsVector3l *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector3l_to_4f( const NsVector3l *S, NsVector4f *D );

NS_IMPEXP NsVector2i* ns_vector3l_to_2i( const NsVector3l *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector3l_to_3i( const NsVector3l *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector3l_to_4i( const NsVector3l *S, NsVector4i *D );

NS_IMPEXP NsVector2s* ns_vector3l_to_2s( const NsVector3l *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector3l_to_3s( const NsVector3l *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector3l_to_4s( const NsVector3l *S, NsVector4s *D );


/* NsVector4l conversions */
NS_IMPEXP NsVector2l* ns_vector4l_to_2l( const NsVector4l *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector4l_to_3l( const NsVector4l *S, NsVector3l *D );

NS_IMPEXP NsVector2d* ns_vector4l_to_2d( const NsVector4l *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector4l_to_3d( const NsVector4l *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector4l_to_4d( const NsVector4l *S, NsVector4d *D );

NS_IMPEXP NsVector2f* ns_vector4l_to_2f( const NsVector4l *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector4l_to_3f( const NsVector4l *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector4l_to_4f( const NsVector4l *S, NsVector4f *D );

NS_IMPEXP NsVector2i* ns_vector4l_to_2i( const NsVector4l *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector4l_to_3i( const NsVector4l *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector4l_to_4i( const NsVector4l *S, NsVector4i *D );

NS_IMPEXP NsVector2s* ns_vector4l_to_2s( const NsVector4l *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector4l_to_3s( const NsVector4l *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector4l_to_4s( const NsVector4l *S, NsVector4s *D );


/* NsVector2i conversions */
NS_IMPEXP NsVector3i* ns_vector2i_to_3i( const NsVector2i *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector2i_to_4i( const NsVector2i *S, NsVector4i *D );

NS_IMPEXP NsVector2d* ns_vector2i_to_2d( const NsVector2i *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector2i_to_3d( const NsVector2i *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector2i_to_4d( const NsVector2i *S, NsVector4d *D );

NS_IMPEXP NsVector2f* ns_vector2i_to_2f( const NsVector2i *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector2i_to_3f( const NsVector2i *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector2i_to_4f( const NsVector2i *S, NsVector4f *D );

NS_IMPEXP NsVector2l* ns_vector2i_to_2l( const NsVector2i *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector2i_to_3l( const NsVector2i *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector2i_to_4l( const NsVector2i *S, NsVector4l *D );

NS_IMPEXP NsVector2s* ns_vector2i_to_2s( const NsVector2i *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector2i_to_3s( const NsVector2i *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector2i_to_4s( const NsVector2i *S, NsVector4s *D );


/* NsVector3i conversions */
NS_IMPEXP NsVector2i* ns_vector3i_to_2i( const NsVector3i *S, NsVector2i *D );
NS_IMPEXP NsVector4i* ns_vector3i_to_4i( const NsVector3i *S, NsVector4i *D );

NS_IMPEXP NsVector2d* ns_vector3i_to_2d( const NsVector3i *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector3i_to_3d( const NsVector3i *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector3i_to_4d( const NsVector3i *S, NsVector4d *D );

NS_IMPEXP NsVector2f* ns_vector3i_to_2f( const NsVector3i *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector3i_to_3f( const NsVector3i *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector3i_to_4f( const NsVector3i *S, NsVector4f *D );

NS_IMPEXP NsVector2l* ns_vector3i_to_2l( const NsVector3i *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector3i_to_3l( const NsVector3i *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector3i_to_4l( const NsVector3i *S, NsVector4l *D );

NS_IMPEXP NsVector2s* ns_vector3i_to_2s( const NsVector3i *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector3i_to_3s( const NsVector3i *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector3i_to_4s( const NsVector3i *S, NsVector4s *D );


/* NsVector4i conversions */
NS_IMPEXP NsVector2i* ns_vector4i_to_2i( const NsVector4i *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector4i_to_3i( const NsVector4i *S, NsVector3i *D );

NS_IMPEXP NsVector2d* ns_vector4i_to_2d( const NsVector4i *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector4i_to_3d( const NsVector4i *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector4i_to_4d( const NsVector4i *S, NsVector4d *D );

NS_IMPEXP NsVector2f* ns_vector4i_to_2f( const NsVector4i *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector4i_to_3f( const NsVector4i *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector4i_to_4f( const NsVector4i *S, NsVector4f *D );

NS_IMPEXP NsVector2l* ns_vector4i_to_2l( const NsVector4i *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector4i_to_3l( const NsVector4i *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector4i_to_4l( const NsVector4i *S, NsVector4l *D );

NS_IMPEXP NsVector2s* ns_vector4i_to_2s( const NsVector4i *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector4i_to_3s( const NsVector4i *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector4i_to_4s( const NsVector4i *S, NsVector4s *D );


/* NsVector2s conversions */
NS_IMPEXP NsVector3s* ns_vector2s_to_3s( const NsVector2s *S, NsVector3s *D );
NS_IMPEXP NsVector4s* ns_vector2s_to_4s( const NsVector2s *S, NsVector4s *D );

NS_IMPEXP NsVector2d* ns_vector2s_to_2d( const NsVector2s *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector2s_to_3d( const NsVector2s *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector2s_to_4d( const NsVector2s *S, NsVector4d *D );

NS_IMPEXP NsVector2f* ns_vector2s_to_2f( const NsVector2s *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector2s_to_3f( const NsVector2s *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector2s_to_4f( const NsVector2s *S, NsVector4f *D );

NS_IMPEXP NsVector2l* ns_vector2s_to_2l( const NsVector2s *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector2s_to_3l( const NsVector2s *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector2s_to_4l( const NsVector2s *S, NsVector4l *D );

NS_IMPEXP NsVector2i* ns_vector2s_to_2i( const NsVector2s *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector2s_to_3i( const NsVector2s *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector2s_to_4i( const NsVector2s *S, NsVector4i *D );


/* NsVector3s conversions */
NS_IMPEXP NsVector2s* ns_vector3s_to_2s( const NsVector3s *S, NsVector2s *D );
NS_IMPEXP NsVector4s* ns_vector3s_to_4s( const NsVector3s *S, NsVector4s *D );

NS_IMPEXP NsVector2d* ns_vector3s_to_2d( const NsVector3s *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector3s_to_3d( const NsVector3s *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector3s_to_4d( const NsVector3s *S, NsVector4d *D );

NS_IMPEXP NsVector2f* ns_vector3s_to_2f( const NsVector3s *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector3s_to_3f( const NsVector3s *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector3s_to_4f( const NsVector3s *S, NsVector4f *D );

NS_IMPEXP NsVector2l* ns_vector3s_to_2l( const NsVector3s *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector3s_to_3l( const NsVector3s *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector3s_to_4l( const NsVector3s *S, NsVector4l *D );

NS_IMPEXP NsVector2i* ns_vector3s_to_2i( const NsVector3s *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector3s_to_3i( const NsVector3s *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector3s_to_4i( const NsVector3s *S, NsVector4i *D );


/* NsVector4s conversions */
NS_IMPEXP NsVector2s* ns_vector4s_to_2s( const NsVector4s *S, NsVector2s *D );
NS_IMPEXP NsVector3s* ns_vector4s_to_3s( const NsVector4s *S, NsVector3s *D );

NS_IMPEXP NsVector2d* ns_vector4s_to_2d( const NsVector4s *S, NsVector2d *D );
NS_IMPEXP NsVector3d* ns_vector4s_to_3d( const NsVector4s *S, NsVector3d *D );
NS_IMPEXP NsVector4d* ns_vector4s_to_4d( const NsVector4s *S, NsVector4d *D );

NS_IMPEXP NsVector2f* ns_vector4s_to_2f( const NsVector4s *S, NsVector2f *D );
NS_IMPEXP NsVector3f* ns_vector4s_to_3f( const NsVector4s *S, NsVector3f *D );
NS_IMPEXP NsVector4f* ns_vector4s_to_4f( const NsVector4s *S, NsVector4f *D );

NS_IMPEXP NsVector2l* ns_vector4s_to_2l( const NsVector4s *S, NsVector2l *D );
NS_IMPEXP NsVector3l* ns_vector4s_to_3l( const NsVector4s *S, NsVector3l *D );
NS_IMPEXP NsVector4l* ns_vector4s_to_4l( const NsVector4s *S, NsVector4l *D );

NS_IMPEXP NsVector2i* ns_vector4s_to_2i( const NsVector4s *S, NsVector2i *D );
NS_IMPEXP NsVector3i* ns_vector4s_to_3i( const NsVector4s *S, NsVector3i *D );
NS_IMPEXP NsVector4i* ns_vector4s_to_4i( const NsVector4s *S, NsVector4i *D );

NS_DECLS_END

#endif/* __NS_MATH_VECTOR_H__ */
