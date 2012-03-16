#include <cmath>
#include <iostream>

#include "stackutil.h"
#include "mapview.h"

using namespace std;

int main(int argc, char ** argv)
{
	int L = log(8)/log(2.0);
	int M = log(8)/log(2.0);
	int N = log(8)/log(2.0);
	int l = log(256)/log(2.0);
	int m = log(128)/log(2.0);
	int n = log(64)/log(2.0);
	string prefix = "test";
	cout<<"prefix = "<<prefix<<endl;
	cout<<"L = "<<L<<endl;
	cout<<"M = "<<M<<endl;
	cout<<"N = "<<N<<endl;
	cout<<"l = "<<l<<endl;
	cout<<"m = "<<m<<endl;
	cout<<"n = "<<n<<endl;

	ImageMapView mapview;
	mapview.setPara(prefix, L+l, M+m, N+n, l, m, n);

	unsigned char * outimg1d = 0; 
	long x0 = 10, y0 = 20, z0 = 65;
	V3DLONG outsz[4] = {100, 280, 51, 1};
	mapview.getImage(2, outimg1d, x0, y0, z0, outsz[0], outsz[1], outsz[2]);
	if(outimg1d) saveImage("test_out.raw", outimg1d, outsz, V3D_UINT8);
    
    if (outimg1d) {delete []outimg1d; outimg1d=0;}
}
