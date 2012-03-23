#include <cmath>
#include <iostream>

#include "stackutil.h"
#include "mapview.h"

using namespace std;

int main(int argc, char ** argv)
{
	int L = 14;//log(8)/log(2.0);
	int M = 38;//log(8)/log(2.0);
	int N = 3;//log(8)/log(2.0);
	int l = 512;//log(256)/log(2.0);
	int m = 256;//log(128)/log(2.0);
	int n = 64;//log(64)/log(2.0);
	string prefix = "test";
	cout<<"prefix = "<<prefix<<endl;
	cout<<"L = "<<L<<endl;
	cout<<"M = "<<M<<endl;
	cout<<"N = "<<N<<endl;
	cout<<"l = "<<l<<endl;
	cout<<"m = "<<m<<endl;
	cout<<"n = "<<n<<endl;

	ImageMapView mapview;
	mapview.setPara(prefix, L, M, N, l, m, n);

	unsigned char * outimg1d = 0; 
	long x0 = 0, y0 = 0, z0 = 0;
	V3DLONG outsz[4] = {512, 256, 64, 1};
	mapview.getImage(0, outimg1d, x0, y0, z0, outsz[0], outsz[1], outsz[2]);
	if(outimg1d) saveImage("test_out.raw", outimg1d, outsz, V3D_UINT8);
    
    if (outimg1d) {delete []outimg1d; outimg1d=0;}
}
