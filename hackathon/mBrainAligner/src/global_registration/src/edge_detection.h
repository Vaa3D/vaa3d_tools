#include <vector>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;

class Coord3D_PCM
{
public:
	double x, y, z;
	Coord3D_PCM(double x0, double y0, double z0) { x = x0; y = y0; z = z0; }
	Coord3D_PCM() { x = y = z = 0; }
};

bool edgeContourExtract(unsigned char* img_tar, unsigned char* img_sub, long long* sz_tar, long long* sz_sub,
	vector<Coord3D_PCM>& tar_points, vector<Coord3D_PCM>& sub_points);