#include "nsmatrix.h"


#define _NS_MATRIX33( _M, _op,\
							_m11, _m12, _m13,\
							_m21, _m22, _m23,\
							_m31, _m32, _m33\
						 )\
   ( _M )->m11 _op ( _m11 ); ( _M )->m12 _op ( _m12 ); ( _M )->m13 _op ( _m13 );\
   ( _M )->m21 _op ( _m21 ); ( _M )->m22 _op ( _m22 ); ( _M )->m23 _op ( _m23 );\
   ( _M )->m31 _op ( _m31 ); ( _M )->m32 _op ( _m32 ); ( _M )->m33 _op ( _m33 );\
	return ( _M )	

#define _NS_MATRIX44( _M, _op,\
							_m11, _m12, _m13, _m14,\
							_m21, _m22, _m23, _m24,\
							_m31, _m32, _m33, _m34,\
							_m41, _m42, _m43, _m44\
						 )\
   ( _M )->m11 _op ( _m11 ); ( _M )->m12 _op ( _m12 ); ( _M )->m13 _op ( _m13 ); ( _M )->m14 _op ( _m14 );\
   ( _M )->m21 _op ( _m21 ); ( _M )->m22 _op ( _m22 ); ( _M )->m23 _op ( _m23 ); ( _M )->m24 _op ( _m24 );\
   ( _M )->m31 _op ( _m31 ); ( _M )->m32 _op ( _m32 ); ( _M )->m33 _op ( _m33 ); ( _M )->m34 _op ( _m34 );\
   ( _M )->m41 _op ( _m41 ); ( _M )->m42 _op ( _m42 ); ( _M )->m43 _op ( _m43 ); ( _M )->m44 _op ( _m44 );\
	return ( _M )


#define _NS_MATRIX33_ROW_MAJOR( M, v )\
	ns_assert( NULL != (M) );\
	ns_assert( NULL != (v) );\
	_NS_MATRIX33( (M), =,\
					 (v)[0], (v)[1], (v)[2],\
					 (v)[3], (v)[4], (v)[5],\
					 (v)[6], (v)[7], (v)[8] )

#define _NS_MATRIX44_ROW_MAJOR( M, v )\
	ns_assert( NULL != (M));\
	ns_assert( NULL != (v));\
	_NS_MATRIX44( (M), =,\
					 (v)[ 0], (v)[ 1], (v)[ 2], (v)[ 3],\
					 (v)[ 4], (v)[ 5], (v)[ 6], (v)[ 7],\
					 (v)[ 8], (v)[ 9], (v)[10], (v)[11],\
					 (v)[12], (v)[13], (v)[14], (v)[15] )

NsMatrix33d* ns_matrix33d_row_major( NsMatrix33d *M, const nsdouble *values )
	{  _NS_MATRIX33_ROW_MAJOR( M, values );  }

NsMatrix44d* ns_matrix44d_row_major( NsMatrix44d *M, const nsdouble *values )
	{  _NS_MATRIX44_ROW_MAJOR( M, values );  }

NsMatrix33f* ns_matrix33f_row_major( NsMatrix33f *M, const nsfloat *values )
	{  _NS_MATRIX33_ROW_MAJOR( M, values );  }

NsMatrix44f* ns_matrix44f_row_major( NsMatrix44f *M, const nsfloat *values )
	{  _NS_MATRIX44_ROW_MAJOR( M, values );  }

NsMatrix33l* ns_matrix33l_row_major( NsMatrix33l *M, const nslong *values )
	{  _NS_MATRIX33_ROW_MAJOR( M, values );  }

NsMatrix44l* ns_matrix44l_row_major( NsMatrix44l *M, const nslong *values )
	{  _NS_MATRIX44_ROW_MAJOR( M, values );  }


#define _NS_MATRIX33_COL_MAJOR( M, v )\
	ns_assert( NULL != (M) );\
	ns_assert( NULL != (v) );\
	_NS_MATRIX33( (M), =,\
					 (v)[0], (v)[3], (v)[6],\
					 (v)[1], (v)[4], (v)[7],\
					 (v)[2], (v)[5], (v)[8] )

#define _NS_MATRIX44_COL_MAJOR( M, v )\
	ns_assert( NULL != (M));\
	ns_assert( NULL != (v));\
	_NS_MATRIX44( (M), =,\
					 (v)[ 0], (v)[ 4], (v)[ 8], (v)[12],\
					 (v)[ 1], (v)[ 5], (v)[ 9], (v)[13],\
					 (v)[ 2], (v)[ 6], (v)[10], (v)[14],\
					 (v)[ 3], (v)[ 7], (v)[11], (v)[15] )

NsMatrix33d* ns_matrix33d_col_major( NsMatrix33d *M, const nsdouble *values )
	{  _NS_MATRIX33_COL_MAJOR( M, values );  }

NsMatrix44d* ns_matrix44d_col_major( NsMatrix44d *M, const nsdouble *values )
	{  _NS_MATRIX44_COL_MAJOR( M, values );  }

NsMatrix33f* ns_matrix33f_col_major( NsMatrix33f *M, const nsfloat *values )
	{  _NS_MATRIX33_COL_MAJOR( M, values );  }

NsMatrix44f* ns_matrix44f_col_major( NsMatrix44f *M, const nsfloat *values )
	{  _NS_MATRIX44_COL_MAJOR( M, values );  }

NsMatrix33l* ns_matrix33l_col_major( NsMatrix33l *M, const nslong *values )
	{  _NS_MATRIX33_COL_MAJOR( M, values );  }

NsMatrix44l* ns_matrix44l_col_major( NsMatrix44l *M, const nslong *values )
	{  _NS_MATRIX44_COL_MAJOR( M, values );  }


#define _NS_MATRIX_ARRAY( M, type )\
	ns_assert( NULL != ( M ) );\
	return ( type * )( M )

nsdouble* ns_matrix33d_array( NsMatrix33d *M )
	{  _NS_MATRIX_ARRAY( M, nsdouble );  }

nsdouble* ns_matrix44d_array( NsMatrix44d *M )
	{  _NS_MATRIX_ARRAY( M, nsdouble );  }

nsfloat* ns_matrix33f_array( NsMatrix33f *M )
	{  _NS_MATRIX_ARRAY( M, nsfloat );  }

nsfloat* ns_matrix44f_array( NsMatrix44f *M )
	{  _NS_MATRIX_ARRAY( M, nsfloat );  }

nslong* ns_matrix33l_array( NsMatrix33l *M )
	{  _NS_MATRIX_ARRAY( M, nslong );  }

nslong* ns_matrix44l_array( NsMatrix44l *M )
	{  _NS_MATRIX_ARRAY( M, nslong );  }

const nsdouble* ns_matrix33d_const_array( const NsMatrix33d *M )
	{  _NS_MATRIX_ARRAY( M, const nsdouble );  }

const nsdouble* ns_matrix44d_const_array( const NsMatrix44d *M )
	{  _NS_MATRIX_ARRAY( M, const nsdouble );  }

const nsfloat* ns_matrix33f_const_array( const NsMatrix33f *M )
	{  _NS_MATRIX_ARRAY( M, const nsfloat );  }

const nsfloat* ns_matrix44f_const_array( const NsMatrix44f *M )
	{  _NS_MATRIX_ARRAY( M, const nsfloat );  }

const nslong* ns_matrix33l_const_array( const NsMatrix33l *M )
	{  _NS_MATRIX_ARRAY( M, const nslong );  }

const nslong* ns_matrix44l_const_array( const NsMatrix44l *M )
	{  _NS_MATRIX_ARRAY( M, const nslong );  }


#define _NS_MATRIX33_ZERO( M, type )\
	ns_assert( NULL != (M) );\
	_NS_MATRIX33( (M), =,\
					 (type)0, (type)0, (type)0,\
					 (type)0, (type)0, (type)0,\
					 (type)0, (type)0, (type)0 )

#define _NS_MATRIX44_ZERO( M, type )\
	ns_assert( NULL != (M) );\
	_NS_MATRIX44( (M), =,\
					 (type)0, (type)0, (type)0, (type)0,\
					 (type)0, (type)0, (type)0, (type)0,\
					 (type)0, (type)0, (type)0, (type)0,\
					 (type)0, (type)0, (type)0, (type)0 )

NsMatrix33d* ns_matrix33d_zero( NsMatrix33d *M )
	{  _NS_MATRIX33_ZERO( M, nsdouble );  }

NsMatrix44d* ns_matrix44d_zero( NsMatrix44d *M )
	{  _NS_MATRIX44_ZERO( M, nsdouble );  }

NsMatrix33f* ns_matrix33f_zero( NsMatrix33f *M )
	{  _NS_MATRIX33_ZERO( M, nsfloat );  }

NsMatrix44f* ns_matrix44f_zero( NsMatrix44f *M )
	{  _NS_MATRIX44_ZERO( M, nsfloat );  }

NsMatrix33l* ns_matrix33l_zero( NsMatrix33l *M )
	{  _NS_MATRIX33_ZERO( M, nslong );  }

NsMatrix44l* ns_matrix44l_zero( NsMatrix44l *M )
	{  _NS_MATRIX44_ZERO( M, nslong );  }


#define _NS_MATRIX33_IDENT( M, type )\
	ns_assert( NULL != (M) );\
	_NS_MATRIX33( (M), =,\
					 (type)1, (type)0, (type)0,\
					 (type)0, (type)1, (type)0,\
					 (type)0, (type)0, (type)1 )

#define _NS_MATRIX44_IDENT( M, type )\
	ns_assert( NULL != (M) );\
	_NS_MATRIX44( (M), =,\
					 (type)1, (type)0, (type)0, (type)0,\
					 (type)0, (type)1, (type)0, (type)0,\
					 (type)0, (type)0, (type)1, (type)0,\
					 (type)0, (type)0, (type)0, (type)1 )

NsMatrix33d* ns_matrix33d_ident( NsMatrix33d *M )
	{  _NS_MATRIX33_IDENT( M, nsdouble );  }

NsMatrix44d* ns_matrix44d_ident( NsMatrix44d *M )
	{  _NS_MATRIX44_IDENT( M, nsdouble );  }

NsMatrix33f* ns_matrix33f_ident( NsMatrix33f *M )
	{  _NS_MATRIX33_IDENT( M, nsfloat );  }

NsMatrix44f* ns_matrix44f_ident( NsMatrix44f *M )
	{  _NS_MATRIX44_IDENT( M, nsfloat );  }

NsMatrix33l* ns_matrix33l_ident( NsMatrix33l *M )
	{  _NS_MATRIX33_IDENT( M, nslong );  }

NsMatrix44l* ns_matrix44l_ident( NsMatrix44l *M )
	{  _NS_MATRIX44_IDENT( M, nslong );  }


#define _NS_MATRIX33_TRANS( M, type )\
	ns_assert( NULL != (M) );\
   NS_SWAP( type, (M)->m12, (M)->m21 );\
   NS_SWAP( type, (M)->m13, (M)->m31 );\
   NS_SWAP( type, (M)->m23, (M)->m32 );\
	return (M)

#define _NS_MATRIX44_TRANS( M, type )\
	ns_assert( NULL != (M) );\
   NS_SWAP( type, (M)->m12, (M)->m21 );\
   NS_SWAP( type, (M)->m13, (M)->m31 );\
   NS_SWAP( type, (M)->m14, (M)->m41 );\
   NS_SWAP( type, (M)->m23, (M)->m32 );\
   NS_SWAP( type, (M)->m24, (M)->m42 );\
   NS_SWAP( type, (M)->m34, (M)->m43 );\
	return (M)

NsMatrix33d* ns_matrix33d_trans( NsMatrix33d *M )
	{  _NS_MATRIX33_TRANS( M, nsdouble );  }

NsMatrix44d* ns_matrix44d_trans( NsMatrix44d *M )
	{  _NS_MATRIX44_TRANS( M, nsdouble );  }

NsMatrix33f* ns_matrix33f_trans( NsMatrix33f *M )
	{  _NS_MATRIX33_TRANS( M, nsfloat );  }

NsMatrix44f* ns_matrix44f_trans( NsMatrix44f *M )
	{  _NS_MATRIX44_TRANS( M, nsfloat );  }

NsMatrix33l* ns_matrix33l_trans( NsMatrix33l *M )
	{  _NS_MATRIX33_TRANS( M, nslong );  }

NsMatrix44l* ns_matrix44l_trans( NsMatrix44l *M )
	{  _NS_MATRIX44_TRANS( M, nslong );  }


#define _NS_MATRIX33_ADD( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
	_NS_MATRIX33( (C), =,\
					 (A)->m11 + (B)->m11, (A)->m12 + (B)->m12, (A)->m13 + (B)->m13,\
					 (A)->m21 + (B)->m21, (A)->m22 + (B)->m22, (A)->m23 + (B)->m23,\
					 (A)->m31 + (B)->m31, (A)->m32 + (B)->m32, (A)->m33 + (B)->m33 )

#define _NS_MATRIX44_ADD( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
	_NS_MATRIX44( (C), =,\
					 (A)->m11 + (B)->m11, (A)->m12 + (B)->m12, (A)->m13 + (B)->m13, (A)->m14 + (B)->m14,\
					 (A)->m21 + (B)->m21, (A)->m22 + (B)->m22, (A)->m23 + (B)->m23, (A)->m24 + (B)->m24,\
					 (A)->m31 + (B)->m31, (A)->m32 + (B)->m32, (A)->m33 + (B)->m33, (A)->m34 + (B)->m34,\
					 (A)->m41 + (B)->m41, (A)->m42 + (B)->m42, (A)->m43 + (B)->m43, (A)->m44 + (B)->m44 )

NsMatrix33d* ns_matrix33d_add( NsMatrix33d *C, const NsMatrix33d *A, const NsMatrix33d *B )
	{  _NS_MATRIX33_ADD( C, A, B );  }

NsMatrix44d* ns_matrix44d_add( NsMatrix44d *C, const NsMatrix44d *A, const NsMatrix44d *B )
	{  _NS_MATRIX44_ADD( C, A, B );  }

NsMatrix33f* ns_matrix33f_add( NsMatrix33f *C, const NsMatrix33f *A, const NsMatrix33f *B )
	{  _NS_MATRIX33_ADD( C, A, B );  }

NsMatrix44f* ns_matrix44f_add( NsMatrix44f *C, const NsMatrix44f *A, const NsMatrix44f *B )
	{  _NS_MATRIX44_ADD( C, A, B );  }

NsMatrix33l* ns_matrix33l_add( NsMatrix33l *C, const NsMatrix33l *A, const NsMatrix33l *B )
	{  _NS_MATRIX33_ADD( C, A, B );  }

NsMatrix44l* ns_matrix44l_add( NsMatrix44l *C, const NsMatrix44l *A, const NsMatrix44l *B )
	{  _NS_MATRIX44_ADD( C, A, B );  }


#define _NS_MATRIX33_CMPD_ADD( A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	_NS_MATRIX33( (A), +=,\
					 (B)->m11, (B)->m12, (B)->m13,\
					 (B)->m21, (B)->m22, (B)->m23,\
					 (B)->m31, (B)->m32, (B)->m33 )

#define _NS_MATRIX44_CMPD_ADD( A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	_NS_MATRIX44( (A), +=,\
					 (B)->m11, (B)->m12, (B)->m13, (B)->m14,\
					 (B)->m21, (B)->m22, (B)->m23, (B)->m24,\
					 (B)->m31, (B)->m32, (B)->m33, (B)->m34,\
					 (B)->m41, (B)->m42, (B)->m43, (B)->m44 )

NsMatrix33d* ns_matrix33d_cmpd_add( NsMatrix33d *A, const NsMatrix33d *B )
	{  _NS_MATRIX33_CMPD_ADD( A, B );  }

NsMatrix44d* ns_matrix44d_cmpd_add( NsMatrix44d *A, const NsMatrix44d *B )
	{  _NS_MATRIX44_CMPD_ADD( A, B );  }

NsMatrix33f* ns_matrix33f_cmpd_add( NsMatrix33f *A, const NsMatrix33f *B )
	{  _NS_MATRIX33_CMPD_ADD( A, B );  }

NsMatrix44f* ns_matrix44f_cmpd_add( NsMatrix44f *A, const NsMatrix44f *B )
	{  _NS_MATRIX44_CMPD_ADD( A, B );  }

NsMatrix33l* ns_matrix33l_cmpd_add( NsMatrix33l *A, const NsMatrix33l *B )
	{  _NS_MATRIX33_CMPD_ADD( A, B );  }

NsMatrix44l* ns_matrix44l_cmpd_add( NsMatrix44l *A, const NsMatrix44l *B )
	{  _NS_MATRIX44_CMPD_ADD( A, B );  }


#define _NS_MATRIX33_SUB( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
	_NS_MATRIX33( (C), =,\
					 (A)->m11 - (B)->m11, (A)->m12 - (B)->m12, (A)->m13 - (B)->m13,\
					 (A)->m21 - (B)->m21, (A)->m22 - (B)->m22, (A)->m23 - (B)->m23,\
					 (A)->m31 - (B)->m31, (A)->m32 - (B)->m32, (A)->m33 - (B)->m33 )

#define _NS_MATRIX44_SUB( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
	_NS_MATRIX44( (C), =,\
					 (A)->m11 - (B)->m11, (A)->m12 - (B)->m12, (A)->m13 - (B)->m13, (A)->m14 - (B)->m14,\
					 (A)->m21 - (B)->m21, (A)->m22 - (B)->m22, (A)->m23 - (B)->m23, (A)->m24 - (B)->m24,\
					 (A)->m31 - (B)->m31, (A)->m32 - (B)->m32, (A)->m33 - (B)->m33, (A)->m34 - (B)->m34,\
					 (A)->m41 - (B)->m41, (A)->m42 - (B)->m42, (A)->m43 - (B)->m43, (A)->m44 - (B)->m44 )

NsMatrix33d* ns_matrix33d_sub( NsMatrix33d *C, const NsMatrix33d *A, const NsMatrix33d *B )
	{  _NS_MATRIX33_SUB( C, A, B );  }

NsMatrix44d* ns_matrix44d_sub( NsMatrix44d *C, const NsMatrix44d *A, const NsMatrix44d *B )
	{  _NS_MATRIX44_SUB( C, A, B );  }

NsMatrix33f* ns_matrix33f_sub( NsMatrix33f *C, const NsMatrix33f *A, const NsMatrix33f *B )
	{  _NS_MATRIX33_SUB( C, A, B );  }

NsMatrix44f* ns_matrix44f_sub( NsMatrix44f *C, const NsMatrix44f *A, const NsMatrix44f *B )
	{  _NS_MATRIX44_SUB( C, A, B );  }

NsMatrix33l* ns_matrix33l_sub( NsMatrix33l *C, const NsMatrix33l *A, const NsMatrix33l *B )
	{  _NS_MATRIX33_SUB( C, A, B );  }

NsMatrix44l* ns_matrix44l_sub( NsMatrix44l *C, const NsMatrix44l *A, const NsMatrix44l *B )
	{  _NS_MATRIX44_SUB( C, A, B );  }


#define _NS_MATRIX33_CMPD_SUB( A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	_NS_MATRIX33( (A), -=,\
					 (B)->m11, (B)->m12, (B)->m13,\
					 (B)->m21, (B)->m22, (B)->m23,\
					 (B)->m31, (B)->m32, (B)->m33 )

#define _NS_MATRIX44_CMPD_SUB( A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	_NS_MATRIX44( (A), -=,\
					 (B)->m11, (B)->m12, (B)->m13, (B)->m14,\
					 (B)->m21, (B)->m22, (B)->m23, (B)->m24,\
					 (B)->m31, (B)->m32, (B)->m33, (B)->m34,\
					 (B)->m41, (B)->m42, (B)->m43, (B)->m44 )

NsMatrix33d* ns_matrix33d_cmpd_sub( NsMatrix33d *A, const NsMatrix33d *B )
	{  _NS_MATRIX33_CMPD_SUB( A, B );  }

NsMatrix44d* ns_matrix44d_cmpd_sub( NsMatrix44d *A, const NsMatrix44d *B )
	{  _NS_MATRIX44_CMPD_SUB( A, B );  }

NsMatrix33f* ns_matrix33f_cmpd_sub( NsMatrix33f *A, const NsMatrix33f *B )
	{  _NS_MATRIX33_CMPD_SUB( A, B );  }

NsMatrix44f* ns_matrix44f_cmpd_sub( NsMatrix44f *A, const NsMatrix44f *B )
	{  _NS_MATRIX44_CMPD_SUB( A, B );  }

NsMatrix33l* ns_matrix33l_cmpd_sub( NsMatrix33l *A, const NsMatrix33l *B )
	{  _NS_MATRIX33_CMPD_SUB( A, B );  }

NsMatrix44l* ns_matrix44l_cmpd_sub( NsMatrix44l *A, const NsMatrix44l *B )
	{  _NS_MATRIX44_CMPD_SUB( A, B );  }


#define _NS_MATRIX33_MUL( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
   ns_assert( (C) != (A) && (C) != (B) );\
	_NS_MATRIX33( (C), =,\
					 (A)->m11*(B)->m11 + (A)->m12*(B)->m21 + (A)->m13*(B)->m31,\
					 (A)->m11*(B)->m12 + (A)->m12*(B)->m22 + (A)->m13*(B)->m32,\
					 (A)->m11*(B)->m13 + (A)->m12*(B)->m23 + (A)->m13*(B)->m33,\
					 (A)->m21*(B)->m11 + (A)->m22*(B)->m21 + (A)->m23*(B)->m31,\
					 (A)->m21*(B)->m12 + (A)->m22*(B)->m22 + (A)->m23*(B)->m32,\
					 (A)->m21*(B)->m13 + (A)->m22*(B)->m23 + (A)->m23*(B)->m33,\
					 (A)->m31*(B)->m11 + (A)->m32*(B)->m21 + (A)->m33*(B)->m31,\
					 (A)->m31*(B)->m12 + (A)->m32*(B)->m22 + (A)->m33*(B)->m32,\
					 (A)->m31*(B)->m13 + (A)->m32*(B)->m23 + (A)->m33*(B)->m33 )

#define _NS_MATRIX44_MUL( C, A, B )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (B) );\
	ns_assert( NULL != (C) );\
   ns_assert( (C) != (A) && (C) != (B) );\
	_NS_MATRIX44( (C), =,\
					 (A)->m11*(B)->m11 + (A)->m12*(B)->m21 + (A)->m13*(B)->m31 + (A)->m14*(B)->m41,\
					 (A)->m11*(B)->m12 + (A)->m12*(B)->m22 + (A)->m13*(B)->m32 + (A)->m14*(B)->m42,\
					 (A)->m11*(B)->m13 + (A)->m12*(B)->m23 + (A)->m13*(B)->m33 + (A)->m14*(B)->m43,\
					 (A)->m11*(B)->m14 + (A)->m12*(B)->m24 + (A)->m13*(B)->m34 + (A)->m14*(B)->m44,\
				    (A)->m21*(B)->m11 + (A)->m22*(B)->m21 + (A)->m23*(B)->m31 + (A)->m24*(B)->m41,\
					 (A)->m21*(B)->m12 + (A)->m22*(B)->m22 + (A)->m23*(B)->m32 + (A)->m24*(B)->m42,\
					 (A)->m21*(B)->m13 + (A)->m22*(B)->m23 + (A)->m23*(B)->m33 + (A)->m24*(B)->m43,\
					 (A)->m21*(B)->m14 + (A)->m22*(B)->m24 + (A)->m23*(B)->m34 + (A)->m24*(B)->m44,\
					 (A)->m31*(B)->m11 + (A)->m32*(B)->m21 + (A)->m33*(B)->m31 + (A)->m34*(B)->m41,\
					 (A)->m31*(B)->m12 + (A)->m32*(B)->m22 + (A)->m33*(B)->m32 + (A)->m34*(B)->m42,\
					 (A)->m31*(B)->m13 + (A)->m32*(B)->m23 + (A)->m33*(B)->m33 + (A)->m34*(B)->m43,\
					 (A)->m31*(B)->m14 + (A)->m32*(B)->m24 + (A)->m33*(B)->m34 + (A)->m34*(B)->m44,\
					 (A)->m41*(B)->m11 + (A)->m42*(B)->m21 + (A)->m43*(B)->m31 + (A)->m44*(B)->m41,\
					 (A)->m41*(B)->m12 + (A)->m42*(B)->m22 + (A)->m43*(B)->m32 + (A)->m44*(B)->m42,\
					 (A)->m41*(B)->m13 + (A)->m42*(B)->m23 + (A)->m43*(B)->m33 + (A)->m44*(B)->m43,\
					 (A)->m41*(B)->m14 + (A)->m42*(B)->m24 + (A)->m43*(B)->m34 + (A)->m44*(B)->m44 )

NsMatrix33d* ns_matrix33d_mul( NsMatrix33d *C, const NsMatrix33d *A, const NsMatrix33d *B )
	{  _NS_MATRIX33_MUL( C, A, B );  }

NsMatrix44d* ns_matrix44d_mul( NsMatrix44d *C, const NsMatrix44d *A, const NsMatrix44d *B )
	{  _NS_MATRIX44_MUL( C, A, B );  }

NsMatrix33f* ns_matrix33f_mul( NsMatrix33f *C, const NsMatrix33f *A, const NsMatrix33f *B )
	{  _NS_MATRIX33_MUL( C, A, B );  }

NsMatrix44f* ns_matrix44f_mul( NsMatrix44f *C, const NsMatrix44f *A, const NsMatrix44f *B )
	{  _NS_MATRIX44_MUL( C, A, B );  }

NsMatrix33l* ns_matrix33l_mul( NsMatrix33l *C, const NsMatrix33l *A, const NsMatrix33l *B )
	{  _NS_MATRIX33_MUL( C, A, B );  }

NsMatrix44l* ns_matrix44l_mul( NsMatrix44l *C, const NsMatrix44l *A, const NsMatrix44l *B )
	{  _NS_MATRIX44_MUL( C, A, B );  }


#define _NS_MATRIX33_SCALE( C, A, s )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_MATRIX33( (C), =,\
					 (A)->m11*s, (A)->m12*s, (A)->m13*s,\
					 (A)->m21*s, (A)->m22*s, (A)->m23*s,\
					 (A)->m31*s, (A)->m32*s, (A)->m33*s )

#define _NS_MATRIX44_SCALE( C, A, s )\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	_NS_MATRIX44( (C), =,\
					 (A)->m11*s, (A)->m12*s, (A)->m13*s, (A)->m14*s,\
					 (A)->m21*s, (A)->m22*s, (A)->m23*s, (A)->m24*s,\
					 (A)->m31*s, (A)->m32*s, (A)->m33*s, (A)->m34*s,\
					 (A)->m41*s, (A)->m42*s, (A)->m43*s, (A)->m44*s )

NsMatrix33d* ns_matrix33d_scale( NsMatrix33d *C, const NsMatrix33d *A, nsdouble s )
	{  _NS_MATRIX33_SCALE( C, A, s );  }

NsMatrix44d* ns_matrix44d_scale( NsMatrix44d *C, const NsMatrix44d *A, nsdouble s )
	{  _NS_MATRIX44_SCALE( C, A, s );  }

NsMatrix33f* ns_matrix33f_scale( NsMatrix33f *C, const NsMatrix33f *A, nsfloat s )
	{  _NS_MATRIX33_SCALE( C, A, s );  }

NsMatrix44f* ns_matrix44f_scale( NsMatrix44f *C, const NsMatrix44f *A, nsfloat s )
	{  _NS_MATRIX44_SCALE( C, A, s );  }

NsMatrix33l* ns_matrix33l_scale( NsMatrix33l *C, const NsMatrix33l *A, nslong s )
	{  _NS_MATRIX33_SCALE( C, A, s );  }

NsMatrix44l* ns_matrix44l_scale( NsMatrix44l *C, const NsMatrix44l *A, nslong s )
	{  _NS_MATRIX44_SCALE( C, A, s );  }


#define _NS_MATRIX33_CMPD_SCALE( M, s )\
	ns_assert( NULL != (M) );\
	_NS_MATRIX33( (M), *=,\
					 (s), (s), (s),\
					 (s), (s), (s),\
					 (s), (s), (s) )

#define _NS_MATRIX44_CMPD_SCALE( M, s )\
	ns_assert( NULL != (M) );\
	_NS_MATRIX44( (M), *=,\
					 (s), (s), (s), (s),\
					 (s), (s), (s), (s),\
					 (s), (s), (s), (s),\
					 (s), (s), (s), (s) )

NsMatrix33d* ns_matrix33d_cmpd_scale( NsMatrix33d *M, nsdouble s )
	{  _NS_MATRIX33_CMPD_SCALE( M, s );  }

NsMatrix44d* ns_matrix44d_cmpd_scale( NsMatrix44d *M, nsdouble s )
	{  _NS_MATRIX44_CMPD_SCALE( M, s );  }

NsMatrix33f* ns_matrix33f_cmpd_scale( NsMatrix33f *M, nsfloat s )
	{  _NS_MATRIX33_CMPD_SCALE( M, s );  }

NsMatrix44f* ns_matrix44f_cmpd_scale( NsMatrix44f *M, nsfloat s )
	{  _NS_MATRIX44_CMPD_SCALE( M, s );  }

NsMatrix33l* ns_matrix33l_cmpd_scale( NsMatrix33l *M, nslong s )
	{  _NS_MATRIX33_CMPD_SCALE( M, s );  }

NsMatrix44l* ns_matrix44l_cmpd_scale( NsMatrix44l *M, nslong s )
	{  _NS_MATRIX44_CMPD_SCALE( M, s );  }


#define _NS_MATRIX33_MUL_VECTOR3( C, M, A )\
	ns_assert( NULL != (M) );\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	(C)->x = (M)->m11*(A)->x + (M)->m12*(A)->y + (M)->m13*(A)->z;\
	(C)->y = (M)->m21*(A)->x + (M)->m22*(A)->y + (M)->m23*(A)->z;\
	(C)->z = (M)->m31*(A)->x + (M)->m32*(A)->y + (M)->m33*(A)->z;\
	return (C)

NsVector3d* ns_matrix33d_mul_vector3d
	(
	NsVector3d         *C,
	const NsMatrix33d  *M,
	const NsVector3d   *A
	)
	{  _NS_MATRIX33_MUL_VECTOR3( C, M, A );  }

NsVector3f* ns_matrix33f_mul_vector3f
	(
	NsVector3f         *C,
	const NsMatrix33f  *M,
	const NsVector3f   *A
	)
	{  _NS_MATRIX33_MUL_VECTOR3( C, M, A );  }

NsVector3l* ns_matrix33l_mul_vector3l
	(
	NsVector3l         *C,
	const NsMatrix33l  *M,
	const NsVector3l   *A
	)
	{  _NS_MATRIX33_MUL_VECTOR3( C, M, A );  }


#define _NS_MATRIX44_MUL_VECTOR3( C, M, A )\
	ns_assert( NULL != (M) );\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	(C)->x = (M)->m11*(A)->x + (M)->m12*(A)->y + (M)->m13*(A)->z + (M)->m14;\
	(C)->y = (M)->m21*(A)->x + (M)->m22*(A)->y + (M)->m23*(A)->z + (M)->m24;\
	(C)->z = (M)->m31*(A)->x + (M)->m32*(A)->y + (M)->m33*(A)->z + (M)->m34;\
	return (C)

NsVector3d* ns_matrix44d_mul_vector3d
	(
	NsVector3d         *C,
	const NsMatrix44d  *M,
	const NsVector3d   *A
	)
	{  _NS_MATRIX44_MUL_VECTOR3( C, M, A );  }

NsVector3f* ns_matrix44f_mul_vector3f
	(
	NsVector3f         *C,
	const NsMatrix44f  *M,
	const NsVector3f   *A
	)
	{  _NS_MATRIX44_MUL_VECTOR3( C, M, A );  }

NsVector3l* ns_matrix44l_mul_vector3l
	(
	NsVector3l         *C,
	const NsMatrix44l  *M,
	const NsVector3l   *A
	)
	{  _NS_MATRIX44_MUL_VECTOR3( C, M, A );  }


#define _NS_MATRIX44_MUL_VECTOR4( C, M, A )\
	ns_assert( NULL != (M) );\
	ns_assert( NULL != (A) );\
	ns_assert( NULL != (C) );\
	(C)->x = (M)->m11*(A)->x + (M)->m12*(A)->y + (M)->m13*(A)->z + (M)->m14*(A)->w;\
	(C)->y = (M)->m21*(A)->x + (M)->m22*(A)->y + (M)->m23*(A)->z + (M)->m24*(A)->w;\
	(C)->z = (M)->m31*(A)->x + (M)->m32*(A)->y + (M)->m33*(A)->z + (M)->m34*(A)->w;\
	(C)->w = (M)->m41*(A)->x + (M)->m42*(A)->y + (M)->m43*(A)->z + (M)->m44*(A)->w;\
	return (C)

NsVector4d* ns_matrix44d_mul_vector4d
	(
	NsVector4d         *C,
	const NsMatrix44d  *M,
	const NsVector4d   *A
	)
	{  _NS_MATRIX44_MUL_VECTOR4( C, M, A );  }

NsVector4f* ns_matrix44f_mul_vector4f
	(
	NsVector4f         *C,
	const NsMatrix44f  *M,
	const NsVector4f   *A
	)
	{  _NS_MATRIX44_MUL_VECTOR4( C, M, A );  }

NsVector4l* ns_matrix44l_mul_vector4l
	(
	NsVector4l         *C,
	const NsMatrix44l  *M,
	const NsVector4l   *A
	)
	{  _NS_MATRIX44_MUL_VECTOR4( C, M, A );  }


#define _NS_MATRIX44_TRANSLATION( M, x, y, z, type )\
	ns_assert( NULL != (M) );\
	_NS_MATRIX44( (M), =,\
					 (type)1, (type)0, (type)0, (x),\
					 (type)0, (type)1, (type)0, (y),\
					 (type)0, (type)0, (type)1, (z),\
					 (type)0, (type)0, (type)0, (type)1 )

NsMatrix44d* ns_matrix44d_translation( NsMatrix44d *M, nsdouble x, nsdouble y, nsdouble z )
   {  _NS_MATRIX44_TRANSLATION( M, x, y, z, nsdouble );  }

NsMatrix44f* ns_matrix44f_translation( NsMatrix44f *M, nsfloat x, nsfloat y, nsfloat z )
   {  _NS_MATRIX44_TRANSLATION( M, x, y, z, nsfloat );  }


#define _NS_MATRIX44_SCALING( M, x, y, z, type )\
	ns_assert( NULL != (M) );\
	_NS_MATRIX44( (M), =,\
					 (x),     (type)0, (type)0, (type)0,\
					 (type)0, (y),     (type)0, (type)0,\
					 (type)0, (type)0, (z),     (type)0,\
					 (type)0, (type)0, (type)0, (type)1 )

NsMatrix44d* ns_matrix44d_scaling( NsMatrix44d *M, nsdouble x, nsdouble y, nsdouble z )
   {  _NS_MATRIX44_SCALING( M, x, y, z, nsdouble );  }

NsMatrix44f* ns_matrix44f_scaling( NsMatrix44f *M, nsfloat x, nsfloat y, nsfloat z )
   {  _NS_MATRIX44_SCALING( M, x, y, z, nsfloat );  }


#define _NS_MATRIX44_ROTATION( M, radians, type, sin_func, cos_func )\
	type s;\
	type c;\
	ns_assert( NULL != (M) );\
	s = sin_func( (radians) );\
	c = cos_func( (radians) )


#define _NS_MATRIX44_ROTATION_X( M, radians, type, zero, one, sin_func, cos_func )\
	_NS_MATRIX44_ROTATION( M, radians, type, sin_func, cos_func );\
	_NS_MATRIX44( (M), =,\
					 one,  zero, zero, zero,\
					 zero, c,    -s,   zero,\
					 zero, s,    c,    zero,\
					 zero, zero, zero, one )

NsMatrix44d* ns_matrix44d_rotation_x( NsMatrix44d *M, nsdouble radians )
	{  _NS_MATRIX44_ROTATION_X( M, radians, nsdouble, 0.0, 1.0, ns_sin, ns_cos );  }

NsMatrix44f* ns_matrix44f_rotation_x( NsMatrix44f *M, nsfloat radians )
	{  _NS_MATRIX44_ROTATION_X( M, radians, nsfloat, 0.0f, 1.0f, ns_sinf, ns_cosf );  }


#define _NS_MATRIX44_ROTATION_Y( M, radians, type, zero, one, sin_func, cos_func )\
	_NS_MATRIX44_ROTATION( M, radians, type, sin_func, cos_func );\
	_NS_MATRIX44( (M), =,\
					 c,    zero, s,    zero,\
					 zero, one,  zero, zero,\
					 -s,   zero, c,    zero,\
					 zero, zero, zero, one )

NsMatrix44d* ns_matrix44d_rotation_y( NsMatrix44d *M, nsdouble radians )
	{  _NS_MATRIX44_ROTATION_Y( M, radians, nsdouble, 0.0, 1.0, ns_sin, ns_cos );  }

NsMatrix44f* ns_matrix44f_rotation_y( NsMatrix44f *M, nsfloat radians )
	{  _NS_MATRIX44_ROTATION_Y( M, radians, nsfloat, 0.0f, 1.0f, ns_sinf, ns_cosf );  }


#define _NS_MATRIX44_ROTATION_Z( M, radians, type, zero, one, sin_func, cos_func )\
	_NS_MATRIX44_ROTATION( M, radians, type, sin_func, cos_func );\
	_NS_MATRIX44( (M), =,\
					 c,    -s,   zero, zero,\
					 s,    c,    zero, zero,\
					 zero, zero, one,  zero,\
					 zero, zero, zero, one )

NsMatrix44d* ns_matrix44d_rotation_z( NsMatrix44d *M, nsdouble radians )
	{  _NS_MATRIX44_ROTATION_Z( M, radians, nsdouble, 0.0, 1.0, ns_sin, ns_cos );  }

NsMatrix44f* ns_matrix44f_rotation_z( NsMatrix44f *M, nsfloat radians )
	{  _NS_MATRIX44_ROTATION_Z( M, radians, nsfloat, 0.0f, 1.0f, ns_sinf, ns_cosf );  }
