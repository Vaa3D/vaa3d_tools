#include "SimScope.h"
#include <iostream>
#include "VirtualVolume.h"

using namespace std;

void scopeSimulator::testFunc(int num) 
{
	cout << "test " << num << endl;
}

void scopeSimulator::paramShotFromController(LocationSimple nextLoc, float x, float y)
{
	cout << x << " " << y << endl;

}


void scopeSimulator::fakeScopeCrop()
{
    wholeImgDim[0] = this->data1d->getDIM_H();
    wholeImgDim[1] = this->data1d->getDIM_V();
    wholeImgDim[2] = this->data1d->getDIM_D();
    wholeImgDim[3] = this->data1d->getDIM_C();

}