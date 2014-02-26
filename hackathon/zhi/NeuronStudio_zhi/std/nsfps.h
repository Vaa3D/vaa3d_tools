#ifndef __NS_STD_FPS_H__
#define __NS_STD_FPS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsbytearray.h>
#include <std/nstime.h>

NS_DECLS_BEGIN

typedef struct _NsFramesPerSecond
	{
	nssize        index;
	nssize        interval;
	nstimer      *times;
	NsByteArray   buffer;
	}
	NsFramesPerSecond;


#define NS_FRAMES_PER_SECOND_DEFAULT_INTERVAL  100

/* 'interval' is the amount of frames to store rendering times. */
NS_IMPEXP NsError ns_frames_per_second_construct( NsFramesPerSecond *fps, nssize interval );
NS_IMPEXP void ns_frames_per_second_destruct( NsFramesPerSecond *fps );

NS_IMPEXP nsdouble ns_frames_per_second( NsFramesPerSecond *fps );

NS_DECLS_END

#endif/* __NS_STD_FPS_H__ */
