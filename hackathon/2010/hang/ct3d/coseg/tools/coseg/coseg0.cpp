//
//=======================================================================
// Copyright 2011 Institute PICB.
// Authors: Hang Xiao
// Data : March 20, 2011
//=======================================================================
//


#include "../../CT3D/cell_track.h"
#include <vector>
#include <string>
#include <ctime>
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
	time_t start, end;
	time(&start);
	CellTrack cell_track;
	vector<string> tree_files;
	//vector<char*> tree_files;
	for(int i = 1; i < argc; i++)
	{
		tree_files.push_back(argv[i]);
	}
	cell_track.setMethod(0);
	cell_track.createFromTrees(tree_files);
	cell_track.exportImages((char*)"coseg0_out");
	time(&end);
	cout<<"time elapsed (s) : "<<difftime(end, start)<<endl;
}
