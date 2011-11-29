#include <v3d_interface.h>
#include <v3d_basicdatatype.h>
//#include "../basic_c_fun/basic_landmark.h"
//#include <math.h>
//#include <stdlib.h>

class v3d_utils
{
public:
	static unsigned char* doubleArrayToCharArray(double* data1dD, V3DLONG numVoxels, ImagePixelType dataType);
	static double* channelToDoubleArray(Image4DSimple* inputImage, int channel);
	static LandmarkList getConnectedComponents(double* data1dD, V3DLONG szx, V3DLONG szy, V3DLONG szz, double threshold);
	static LandmarkList getConnectedComponents2D(double* data1dD, V3DLONG szx, V3DLONG szy, V3DLONG szz, double threshold);
	static void rescaleForDisplay(double* input, double* output, V3DLONG numVoxels, ImagePixelType dataType);
;

};