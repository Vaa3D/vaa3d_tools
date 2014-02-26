#ifndef __NS_MATH_CONV_DECLS_H__
#define __NS_MATH_CONV_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>

NS_DECLS_BEGIN

#ifdef NS_OS_WINDOWS
	#pragma pack( push, 1 )
#endif

typedef struct _NsConv33d
	{
	nsdouble
	k00, k10, k20,
	k01, k11, k21,
	k02, k12, k22;
	}
	NsConv33d;

typedef struct _NsConv33f
	{
	nsfloat
	k00, k10, k20,
	k01, k11, k21,
	k02, k12, k22;
	}
	NsConv33f;

typedef struct _NsConv33l
	{
	nslong
	k00, k10, k20,
	k01, k11, k21,
	k02, k12, k22;
	}
	NsConv33l;

typedef struct _NsConv55d
	{
	nsdouble
	k00, k10, k20, k30, k40,
	k01, k11, k21, k31, k41,
	k02, k12, k22, k32, k42,
	k03, k13, k23, k33, k43,
	k04, k14, k24, k34, k44;
	}
	NsConv55d;

typedef struct _NsConv55f
	{
	nsfloat
	k00, k10, k20, k30, k40,
	k01, k11, k21, k31, k41,
	k02, k12, k22, k32, k42,
	k03, k13, k23, k33, k43,
	k04, k14, k24, k34, k44;
	}
	NsConv55f;

typedef struct _NsConv55l
	{
	nslong
	k00, k10, k20, k30, k40,
	k01, k11, k21, k31, k41,
	k02, k12, k22, k32, k42,
	k03, k13, k23, k33, k43,
	k04, k14, k24, k34, k44;
	}
	NsConv55l;

typedef struct _NsConv333d
	{
	nsdouble
	k000, k100, k200,
	k010, k110, k210,
	k020, k120, k220,
		k001, k101, k201,
		k011, k111, k211,
		k021, k121, k221,
			k002, k102, k202,
			k012, k112, k212,
			k022, k122, k222;
	}
	NsConv333d;

typedef struct _NsConv333f
	{
	nsfloat
	k000, k100, k200,
	k010, k110, k210,
	k020, k120, k220,
		k001, k101, k201,
		k011, k111, k211,
		k021, k121, k221,
			k002, k102, k202,
			k012, k112, k212,
			k022, k122, k222;
	}
	NsConv333f;

typedef struct _NsConv333l
	{
	nslong
	k000, k100, k200,
	k010, k110, k210,
	k020, k120, k220,
		k001, k101, k201,
		k011, k111, k211,
		k021, k121, k221,
			k002, k102, k202,
			k012, k112, k212,
			k022, k122, k222;
	}
	NsConv333l;

typedef struct _NsConv555d
	{
	nsdouble
	k000, k100, k200, k300, k400,
	k010, k110, k210, k310, k410,
	k020, k120, k220, k320, k420,
	k030, k130, k230, k330, k430,
	k040, k140, k240, k340, k440,
		k001, k101, k201, k301, k401,
		k011, k111, k211, k311, k411,
		k021, k121, k221, k321, k421,
		k031, k131, k231, k331, k431,
		k041, k141, k241, k341, k441,
			k002, k102, k202, k302, k402,
			k012, k112, k212, k312, k412,
			k022, k122, k222, k322, k422,
			k032, k132, k232, k332, k432,
			k042, k142, k242, k342, k442,
				k003, k103, k203, k303, k403,
				k013, k113, k213, k313, k413,
				k023, k123, k223, k323, k423,
				k033, k133, k233, k333, k433,
				k043, k143, k243, k343, k443,
					k004, k104, k204, k304, k404,
					k014, k114, k214, k314, k414,
					k024, k124, k224, k324, k424,
					k034, k134, k234, k334, k434,
					k044, k144, k244, k344, k444;
	}
	NsConv555d;

typedef struct _NsConv555f
	{
	nsfloat
	k000, k100, k200, k300, k400,
	k010, k110, k210, k310, k410,
	k020, k120, k220, k320, k420,
	k030, k130, k230, k330, k430,
	k040, k140, k240, k340, k440,
		k001, k101, k201, k301, k401,
		k011, k111, k211, k311, k411,
		k021, k121, k221, k321, k421,
		k031, k131, k231, k331, k431,
		k041, k141, k241, k341, k441,
			k002, k102, k202, k302, k402,
			k012, k112, k212, k312, k412,
			k022, k122, k222, k322, k422,
			k032, k132, k232, k332, k432,
			k042, k142, k242, k342, k442,
				k003, k103, k203, k303, k403,
				k013, k113, k213, k313, k413,
				k023, k123, k223, k323, k423,
				k033, k133, k233, k333, k433,
				k043, k143, k243, k343, k443,
					k004, k104, k204, k304, k404,
					k014, k114, k214, k314, k414,
					k024, k124, k224, k324, k424,
					k034, k134, k234, k334, k434,
					k044, k144, k244, k344, k444;
	}
	NsConv555f;

typedef struct _NsConv555l 
	{
	nslong
	k000, k100, k200, k300, k400,
	k010, k110, k210, k310, k410,
	k020, k120, k220, k320, k420,
	k030, k130, k230, k330, k430,
	k040, k140, k240, k340, k440,
		k001, k101, k201, k301, k401,
		k011, k111, k211, k311, k411,
		k021, k121, k221, k321, k421,
		k031, k131, k231, k331, k431,
		k041, k141, k241, k341, k441,
			k002, k102, k202, k302, k402,
			k012, k112, k212, k312, k412,
			k022, k122, k222, k322, k422,
			k032, k132, k232, k332, k432,
			k042, k142, k242, k342, k442,
				k003, k103, k203, k303, k403,
				k013, k113, k213, k313, k413,
				k023, k123, k223, k323, k423,
				k033, k133, k233, k333, k433,
				k043, k143, k243, k343, k443,
					k004, k104, k204, k304, k404,
					k014, k114, k214, k314, k414,
					k024, k124, k224, k324, k424,
					k034, k134, k234, k334, k434,
					k044, k144, k244, k344, k444;
	}
	NsConv555l;

#ifdef NS_OS_WINDOWS
	#pragma pack( pop )
#endif


NS_COMPILE_TIME_SIZE_CHECK( NsConv33d, 3*3*sizeof( nsdouble ) );
NS_COMPILE_TIME_SIZE_CHECK( NsConv55d, 5*5*sizeof( nsdouble ) );

NS_COMPILE_TIME_SIZE_CHECK( NsConv33f, 3*3*sizeof( nsfloat ) );
NS_COMPILE_TIME_SIZE_CHECK( NsConv55f, 5*5*sizeof( nsfloat ) );

NS_COMPILE_TIME_SIZE_CHECK( NsConv33l, 3*3*sizeof( nslong ) );
NS_COMPILE_TIME_SIZE_CHECK( NsConv55l, 5*5*sizeof( nslong ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv33d, k00,  0*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv33d, k22,  8*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv55d, k00,  0*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv55d, k44, 24*sizeof( nsdouble ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv33f, k00,  0*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv33f, k22,  8*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv55f, k00,  0*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv55f, k44, 24*sizeof( nsfloat ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv33l, k00,  0*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv33l, k22,  8*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv55l, k00,  0*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv55l, k44, 24*sizeof( nslong ) );

NS_COMPILE_TIME_SIZE_CHECK( NsConv333d, 3*3*3*sizeof( nsdouble ) );
NS_COMPILE_TIME_SIZE_CHECK( NsConv555d, 5*5*5*sizeof( nsdouble ) );

NS_COMPILE_TIME_SIZE_CHECK( NsConv333f, 3*3*3*sizeof( nsfloat ) );
NS_COMPILE_TIME_SIZE_CHECK( NsConv555f, 5*5*5*sizeof( nsfloat ) );

NS_COMPILE_TIME_SIZE_CHECK( NsConv333l, 3*3*3*sizeof( nslong ) );
NS_COMPILE_TIME_SIZE_CHECK( NsConv555l, 5*5*5*sizeof( nslong ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv333d, k000,   0*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv333d, k222,  26*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv555d, k000,   0*sizeof( nsdouble ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv555d, k444, 124*sizeof( nsdouble ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv333f, k000,   0*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv333f, k222,  26*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv555f, k000,   0*sizeof( nsfloat ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv555f, k444, 124*sizeof( nsfloat ) );

NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv333l, k000,   0*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv333l, k222,  26*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv555l, k000,   0*sizeof( nslong ) );
NS_COMPILE_TIME_STRUCT_MEMBER_OFFSET_CHECK( NsConv555l, k444, 124*sizeof( nslong ) );

NS_DECLS_END

#endif/* __NS_MATH_CONV_DECLS_H__ */
