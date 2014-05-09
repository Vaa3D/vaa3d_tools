#ifndef __NS_MATH_DRAGRECT_DECLS_H__
#define __NS_MATH_DRAGRECT_DECLS_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>

NS_DECLS_BEGIN

typedef struct _NsDragRect
	{
	nsint  start_x, start_y;
	nsint  end_x, end_y;
	nsint  min_x, min_y;
	nsint  max_x, max_y;
	nsint  visible;
	nsint  tolerance;
	}
	NsDragRect;

NS_DECLS_END

#endif/* __NS_MATH_DRAGRECT_DECLS_H__ */
