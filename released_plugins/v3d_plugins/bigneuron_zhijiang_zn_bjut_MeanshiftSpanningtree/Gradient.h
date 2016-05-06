#include <v3d_interface.h>
class Gradient
{
public:
	double gradient_x;
	double gradient_y;
	double gradient_z;
	double gradient;
	double angle_x;
	double angle_y;
	double angle_z;
	V3DLONG x;
	V3DLONG y;
	V3DLONG z;
public:
	Gradient()
	{
		gradient_x=0;
		gradient_y=0;
		gradient_z=0;
		gradient=0;
		angle_x=0;
		angle_y=0;
		angle_z=0;
	};
};