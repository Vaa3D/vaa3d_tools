//
//=======================================================================
// Copyright 2011 Institute PICB.
// Authors: Hang Xiao
// Data : March 20, 2011
//=======================================================================
//


#include "../../component_tree.h"
#include "../../myalgorithms.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main(int argc, char * argv[])
{
	ComponentTree tree;
	tree.load(argv[1]);
	cout<<"---------------Component Tree-------------------------"<<endl;
	cout<<"width = "<<tree.width()<<"\theight = "<<tree.height()<<"\tdepth = "<<tree.depth()<<endl;
	cout<<"minSize = "<<tree.getMinThresh()<<"\tmaxSize = "<<tree.getMaxThresh()<<"\tsingleSize = "<<tree.getSingleThresh()<<endl;
	cout<<"total node : "<<tree.nodeNum()<<"\tleaf node : "<<tree.leafNum()<<"\tpixles num : "<<tree.pixelNum()<<endl;
	cout<<endl;
	return 0;
}
