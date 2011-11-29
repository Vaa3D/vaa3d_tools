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
#include <string>
using namespace std;

int main(int argc, char * argv[])
{
	int width = 0;
	int height = 0;
	int depth = 0;
	int channels = 1;
	unsigned char * img = NULL;
	img = readtiff(argv[1], &height, &width, &depth, &channels);
	int min_thresh = atoi(argv[2]);
	int max_thresh = atoi(argv[3]);
	int single_thresh = atoi(argv[4]);

	cout<<"width = "<<width<<"  height = "<<height<<"  depth = "<<depth<<"  channels = "<<channels<<endl;
	cout<<"min_thresh = "<<min_thresh<<"  max_thresh = "<<max_thresh<<"  single_thresh = "<<single_thresh<<endl;

	ComponentTree tree;
	tree.create(argv[1], min_thresh, max_thresh, single_thresh);
	if(argc == 5)
	{
		string str_file(argv[1]);
		str_file = str_file.substr(0, str_file.rfind("."));
		str_file.append(".bin.tree");
		tree.save(str_file.c_str());
	}
	else if(argc == 6)
	{
		tree.save(argv[5]);
	}
}
