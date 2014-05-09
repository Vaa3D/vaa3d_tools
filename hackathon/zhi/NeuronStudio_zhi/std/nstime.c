#include "nstime.h"


#ifdef NS_OS_WINDOWS
	#include "nstime-os-windows.inl"
#else
	#include "nstime-null.inl"
#endif
