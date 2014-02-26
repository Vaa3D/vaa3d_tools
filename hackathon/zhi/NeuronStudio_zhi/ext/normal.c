#include "normal.h"


void ComputeImageNormal( nsfloat *corners, nsfloat *normal )
	{
	nsfloat dx, dy, dz, mag;

	// compute change in x
	dx = corners[0]-corners[1]+corners[2]-corners[3] +
		  corners[4]-corners[5]+corners[6]-corners[7];

	// compute change in y
	dy = corners[2]-corners[0]+corners[3]-corners[1] +
		  corners[6]-corners[4]+corners[7]-corners[5];

	// compute change in z
	dz = corners[4]-corners[0]+corners[5]-corners[1] +
		  corners[6]-corners[2]+corners[7]-corners[3];

	// normalize
	mag = ns_sqrtf(dx*dx+dy*dy+dz*dz);

	// NOTE: to prevent division by zero...
	if( mag >= 1.0f )
		{
		mag = 1.0f / mag;

		normal[0] = dx * mag;
		normal[1] = dy * mag;
		normal[2] = dz * mag;
		}
	else
		{
		normal[0] = 0.0f;
		normal[1] = 0.0f;
		normal[2] = 1.0f;
		}
	}
