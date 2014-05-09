/*****************************************************************
    filtermgr.h:    This code controls the excution of an image
                    filter on some image. It allows for the filter
                    to be multithreaded and also for in-place
                    computation so that no additional copies of
                    the data are created.
******************************************************************/
#ifndef FILTERMGR_H
#define FILTERMGR_H

#include <nsconfigure.h>
#include <std/nsmemory.h>
#include <std/nsthread.h>
#include <std/nsprogress.h>

NS_DECLS_BEGIN


#define FILTER_BLUR     1
#define FILTER_MEDIAN   2
#define FILTER_BLUR26   3
#define FILTER_INVERT   4
#define FILTER_ZBLUR    5
#define FILTER_12to16   6


typedef struct tagFILTERDEFINITION
	{
	nsuchar     *image;
	nslong       imagexdim;
	nslong       imageydim;
	nslong       imagezdim;
	nslong       threads;
	nsint        filtertype;
	NsProgress  *progress;
	nslong       region_x;
	nslong       region_y;
	nslong       region_z;
	nslong       region_width;
	nslong       region_height;
	nslong       region_length;
	}
	FILTERDEFINITION;


typedef struct tagFILTERDEFINITION16
	{
	nsushort    *image;
	nslong       imagexdim;
	nslong       imageydim;
	nslong       imagezdim;
	nslong       threads;
	nsint        filtertype;
	nsint        is12bit;
	NsProgress  *progress;
	nslong       region_x;
	nslong       region_y;
	nslong       region_z;
	nslong       region_width;
	nslong       region_height;
	nslong       region_length;
	}
	FILTERDEFINITION16;


// these return zero if memory fails
NS_IMPEXP nsint RunFilter( FILTERDEFINITION *fd );
NS_IMPEXP nsint RunFilter_16bit( FILTERDEFINITION16 *fd );

NS_DECLS_END

#endif
