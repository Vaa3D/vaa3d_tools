#ifndef __NS_MATH_MATRIX_H__
#define __NS_MATH_MATRIX_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsmath.h>
#include <math/nsvectordecls.h>
#include <math/nsmatrixdecls.h>

NS_DECLS_BEGIN

NS_IMPEXP NsMatrix33d* ns_matrix33d_row_major( NsMatrix33d *M, const nsdouble *values );
NS_IMPEXP NsMatrix44d* ns_matrix44d_row_major( NsMatrix44d *M, const nsdouble *values );

NS_IMPEXP NsMatrix33f* ns_matrix33f_row_major( NsMatrix33f *M, const nsfloat *values );
NS_IMPEXP NsMatrix44f* ns_matrix44f_row_major( NsMatrix44f *M, const nsfloat *values );

NS_IMPEXP NsMatrix33l* ns_matrix33l_row_major( NsMatrix33l *M, const nslong *values );
NS_IMPEXP NsMatrix44l* ns_matrix44l_row_major( NsMatrix44l *M, const nslong *values );


NS_IMPEXP NsMatrix33d* ns_matrix33d_col_major( NsMatrix33d *M, const nsdouble *values );
NS_IMPEXP NsMatrix44d* ns_matrix44d_col_major( NsMatrix44d *M, const nsdouble *values );

NS_IMPEXP NsMatrix33f* ns_matrix33f_col_major( NsMatrix33f *M, const nsfloat *values );
NS_IMPEXP NsMatrix44f* ns_matrix44f_col_major( NsMatrix44f *M, const nsfloat *values );

NS_IMPEXP NsMatrix33l* ns_matrix33l_col_major( NsMatrix33l *M, const nslong *values );
NS_IMPEXP NsMatrix44l* ns_matrix44l_col_major( NsMatrix44l *M, const nslong *values );


NS_IMPEXP nsdouble* ns_matrix33d_array( NsMatrix33d *M );
NS_IMPEXP nsdouble* ns_matrix44d_array( NsMatrix44d *M );

NS_IMPEXP nsfloat* ns_matrix33f_array( NsMatrix33f *M );
NS_IMPEXP nsfloat* ns_matrix44f_array( NsMatrix44f *M );

NS_IMPEXP nslong* ns_matrix33l_array( NsMatrix33l *M );
NS_IMPEXP nslong* ns_matrix44l_array( NsMatrix44l *M );


NS_IMPEXP const nsdouble* ns_matrix33d_const_array( const NsMatrix33d *M );
NS_IMPEXP const nsdouble* ns_matrix44d_const_array( const NsMatrix44d *M );

NS_IMPEXP const nsfloat* ns_matrix33f_const_array( const NsMatrix33f *M );
NS_IMPEXP const nsfloat* ns_matrix44f_const_array( const NsMatrix44f *M );

NS_IMPEXP const nslong* ns_matrix33l_const_array( const NsMatrix33l *M );
NS_IMPEXP const nslong* ns_matrix44l_const_array( const NsMatrix44l *M );


/* M = 0
	Returns M */
NS_IMPEXP NsMatrix33d* ns_matrix33d_zero( NsMatrix33d *M );
NS_IMPEXP NsMatrix44d* ns_matrix44d_zero( NsMatrix44d *M );

NS_IMPEXP NsMatrix33f* ns_matrix33f_zero( NsMatrix33f *M );
NS_IMPEXP NsMatrix44f* ns_matrix44f_zero( NsMatrix44f *M );

NS_IMPEXP NsMatrix33l* ns_matrix33l_zero( NsMatrix33l *M );
NS_IMPEXP NsMatrix44l* ns_matrix44l_zero( NsMatrix44l *M );


/* M = I
	Returns M */
NS_IMPEXP NsMatrix33d* ns_matrix33d_ident( NsMatrix33d *M );
NS_IMPEXP NsMatrix44d* ns_matrix44d_ident( NsMatrix44d *M );

NS_IMPEXP NsMatrix33f* ns_matrix33f_ident( NsMatrix33f *M );
NS_IMPEXP NsMatrix44f* ns_matrix44f_ident( NsMatrix44f *M );

NS_IMPEXP NsMatrix33l* ns_matrix33l_ident( NsMatrix33l *M );
NS_IMPEXP NsMatrix44l* ns_matrix44l_ident( NsMatrix44l *M );


/* M = M^T
	Returns M */
NS_IMPEXP NsMatrix33d* ns_matrix33d_trans( NsMatrix33d *M );
NS_IMPEXP NsMatrix44d* ns_matrix44d_trans( NsMatrix44d *M );

NS_IMPEXP NsMatrix33f* ns_matrix33f_trans( NsMatrix33f *M );
NS_IMPEXP NsMatrix44f* ns_matrix44f_trans( NsMatrix44f *M );

NS_IMPEXP NsMatrix33l* ns_matrix33l_trans( NsMatrix33l *M );
NS_IMPEXP NsMatrix44l* ns_matrix44l_trans( NsMatrix44l *M );


/* C = A + B
	Returns C */
NS_IMPEXP NsMatrix33d* ns_matrix33d_add( NsMatrix33d *C, const NsMatrix33d *A, const NsMatrix33d *B );
NS_IMPEXP NsMatrix44d* ns_matrix44d_add( NsMatrix44d *C, const NsMatrix44d *A, const NsMatrix44d *B );

NS_IMPEXP NsMatrix33f* ns_matrix33f_add( NsMatrix33f *C, const NsMatrix33f *A, const NsMatrix33f *B );
NS_IMPEXP NsMatrix44f* ns_matrix44f_add( NsMatrix44f *C, const NsMatrix44f *A, const NsMatrix44f *B );

NS_IMPEXP NsMatrix33l* ns_matrix33l_add( NsMatrix33l *C, const NsMatrix33l *A, const NsMatrix33l *B );
NS_IMPEXP NsMatrix44l* ns_matrix44l_add( NsMatrix44l *C, const NsMatrix44l *A, const NsMatrix44l *B );


/* A += B
	Returns A */
NS_IMPEXP NsMatrix33d* ns_matrix33d_cmpd_add( NsMatrix33d *A, const NsMatrix33d *B );
NS_IMPEXP NsMatrix44d* ns_matrix44d_cmpd_add( NsMatrix44d *A, const NsMatrix44d *B );

NS_IMPEXP NsMatrix33f* ns_matrix33f_cmpd_add( NsMatrix33f *A, const NsMatrix33f *B );
NS_IMPEXP NsMatrix44f* ns_matrix44f_cmpd_add( NsMatrix44f *A, const NsMatrix44f *B );

NS_IMPEXP NsMatrix33l* ns_matrix33l_cmpd_add( NsMatrix33l *A, const NsMatrix33l *B );
NS_IMPEXP NsMatrix44l* ns_matrix44l_cmpd_add( NsMatrix44l *A, const NsMatrix44l *B );


/* C = A - B
	Returns C */
NS_IMPEXP NsMatrix33d* ns_matrix33d_sub( NsMatrix33d *C, const NsMatrix33d *A, const NsMatrix33d *B );
NS_IMPEXP NsMatrix44d* ns_matrix44d_sub( NsMatrix44d *C, const NsMatrix44d *A, const NsMatrix44d *B );

NS_IMPEXP NsMatrix33f* ns_matrix33f_sub( NsMatrix33f *C, const NsMatrix33f *A, const NsMatrix33f *B );
NS_IMPEXP NsMatrix44f* ns_matrix44f_sub( NsMatrix44f *C, const NsMatrix44f *A, const NsMatrix44f *B );

NS_IMPEXP NsMatrix33l* ns_matrix33l_sub( NsMatrix33l *C, const NsMatrix33l *A, const NsMatrix33l *B );
NS_IMPEXP NsMatrix44l* ns_matrix44l_sub( NsMatrix44l *C, const NsMatrix44l *A, const NsMatrix44l *B );


/* A -= B
	Returns A */
NS_IMPEXP NsMatrix33d* ns_matrix33d_cmpd_sub( NsMatrix33d *A, const NsMatrix33d *B );
NS_IMPEXP NsMatrix44d* ns_matrix44d_cmpd_sub( NsMatrix44d *A, const NsMatrix44d *B );

NS_IMPEXP NsMatrix33f* ns_matrix33f_cmpd_sub( NsMatrix33f *A, const NsMatrix33f *B );
NS_IMPEXP NsMatrix44f* ns_matrix44f_cmpd_sub( NsMatrix44f *A, const NsMatrix44f *B );

NS_IMPEXP NsMatrix33l* ns_matrix33l_cmpd_sub( NsMatrix33l *A, const NsMatrix33l *B );
NS_IMPEXP NsMatrix44l* ns_matrix44l_cmpd_sub( NsMatrix44l *A, const NsMatrix44l *B );


/* C = A * B
	Returns C */
NS_IMPEXP NsMatrix33d* ns_matrix33d_mul( NsMatrix33d *C, const NsMatrix33d *A, const NsMatrix33d *B );
NS_IMPEXP NsMatrix44d* ns_matrix44d_mul( NsMatrix44d *C, const NsMatrix44d *A, const NsMatrix44d *B );

NS_IMPEXP NsMatrix33f* ns_matrix33f_mul( NsMatrix33f *C, const NsMatrix33f *A, const NsMatrix33f *B );
NS_IMPEXP NsMatrix44f* ns_matrix44f_mul( NsMatrix44f *C, const NsMatrix44f *A, const NsMatrix44f *B );

NS_IMPEXP NsMatrix33l* ns_matrix33l_mul( NsMatrix33l *C, const NsMatrix33l *A, const NsMatrix33l *B );
NS_IMPEXP NsMatrix44l* ns_matrix44l_mul( NsMatrix44l *C, const NsMatrix44l *A, const NsMatrix44l *B );


/* C = A * s
	Returns C */
NS_IMPEXP NsMatrix33d* ns_matrix33d_scale( NsMatrix33d *C, const NsMatrix33d *A, nsdouble s );
NS_IMPEXP NsMatrix44d* ns_matrix44d_scale( NsMatrix44d *C, const NsMatrix44d *A, nsdouble s );

NS_IMPEXP NsMatrix33f* ns_matrix33f_scale( NsMatrix33f *C, const NsMatrix33f *A, nsfloat s );
NS_IMPEXP NsMatrix44f* ns_matrix44f_scale( NsMatrix44f *C, const NsMatrix44f *A, nsfloat s );

NS_IMPEXP NsMatrix33l* ns_matrix33l_scale( NsMatrix33l *C, const NsMatrix33l *A, nslong s );
NS_IMPEXP NsMatrix44l* ns_matrix44l_scale( NsMatrix44l *C, const NsMatrix44l *A, nslong s );


/* C *= scalar
	Returns C */
NS_IMPEXP NsMatrix33d* ns_matrix33d_cmpd_scale( NsMatrix33d *M, nsdouble s );
NS_IMPEXP NsMatrix44d* ns_matrix44d_cmpd_scale( NsMatrix44d *M, nsdouble s );

NS_IMPEXP NsMatrix33f* ns_matrix33f_cmpd_scale( NsMatrix33f *M, nsfloat s );
NS_IMPEXP NsMatrix44f* ns_matrix44f_cmpd_scale( NsMatrix44f *M, nsfloat s );

NS_IMPEXP NsMatrix33l* ns_matrix33l_cmpd_scale( NsMatrix33l *M, nslong s );
NS_IMPEXP NsMatrix44l* ns_matrix44l_cmpd_scale( NsMatrix44l *M, nslong s );


/* C = M * A
	Returns B */
NS_IMPEXP NsVector3d* ns_matrix33d_mul_vector3d
	(
	NsVector3d         *C,
	const NsMatrix33d  *M,
	const NsVector3d   *A
	);

NS_IMPEXP NsVector3f* ns_matrix33f_mul_vector3f
	(
	NsVector3f         *C,
	const NsMatrix33f  *M,
	const NsVector3f   *A
	);

NS_IMPEXP NsVector3l* ns_matrix33l_mul_vector3l
	(
	NsVector3l         *C,
	const NsMatrix33l  *M,
	const NsVector3l   *A
	);


/* C = M * A where A->w assumed to be 1
	Returns B */
NS_IMPEXP NsVector3d* ns_matrix44d_mul_vector3d
	(
	NsVector3d         *C,
	const NsMatrix44d  *M,
	const NsVector3d   *A
	);

NS_IMPEXP NsVector3f* ns_matrix44f_mul_vector3f
	(
	NsVector3f         *C,
	const NsMatrix44f  *M,
	const NsVector3f   *A
	);

NS_IMPEXP NsVector3l* ns_matrix44l_mul_vector3l
	(
	NsVector3l         *C,
	const NsMatrix44l  *M,
	const NsVector3l   *A
	);


/* C = M * A
	Returns B */
NS_IMPEXP NsVector4d* ns_matrix44d_mul_vector4d
	(
	NsVector4d         *C,
	const NsMatrix44d  *M,
	const NsVector4d   *A
	);

NS_IMPEXP NsVector4f* ns_matrix44f_mul_vector4f
	(
	NsVector4f         *C,
	const NsMatrix44f  *M,
	const NsVector4f   *A
	);

NS_IMPEXP NsVector4l* ns_matrix44l_mul_vector4l
	(
	NsVector4l         *C,
	const NsMatrix44l  *M,
	const NsVector4l   *A
	);


/* Vector translation matrix. */
NS_IMPEXP NsMatrix44d* ns_matrix44d_translation( NsMatrix44d *M, nsdouble x, nsdouble y, nsdouble z );
NS_IMPEXP NsMatrix44f* ns_matrix44f_translation( NsMatrix44f *M, nsfloat x, nsfloat y, nsfloat z );


/* Vector scaling matrix. */
NS_IMPEXP NsMatrix44d* ns_matrix44d_scaling( NsMatrix44d *M, nsdouble x, nsdouble y, nsdouble z );
NS_IMPEXP NsMatrix44f* ns_matrix44f_scaling( NsMatrix44f *M, nsfloat x, nsfloat y, nsfloat z );


/* Vector rotation matrices( xyz axis' ). */
NS_IMPEXP NsMatrix44d* ns_matrix44d_rotation_x( NsMatrix44d *M, nsdouble radians );
NS_IMPEXP NsMatrix44f* ns_matrix44f_rotation_x( NsMatrix44f *M, nsfloat radians );

NS_IMPEXP NsMatrix44d* ns_matrix44d_rotation_y( NsMatrix44d *M, nsdouble radians );
NS_IMPEXP NsMatrix44f* ns_matrix44f_rotation_y( NsMatrix44f *M, nsfloat radians );

NS_IMPEXP NsMatrix44d* ns_matrix44d_rotation_z( NsMatrix44d *M, nsdouble radians );
NS_IMPEXP NsMatrix44f* ns_matrix44f_rotation_z( NsMatrix44f *M, nsfloat radians );


/* Vector rotation matrix( arbitrary axis ). */
NS_IMPEXP NsMatrix44d* ns_matrix44d_rotation_axis( NsMatrix44d *M, const NsVector3d *A, nsdouble radians );
NS_IMPEXP NsMatrix44f* ns_matrix44f_rotation_axis( NsMatrix44f *M, const NsVector3f *A, nsfloat radians );

NS_DECLS_END

#endif/* __NS_MATH_MATRIX_H__ */
