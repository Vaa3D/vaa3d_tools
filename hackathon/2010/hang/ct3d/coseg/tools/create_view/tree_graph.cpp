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
#include <iostream>
using namespace std;
int main(int argc, char * argv[])
{
	if(argc > 3 || argc < 2)
	{
		cout<<"usage : tree_graph <component_tree_file> [output_file]"<<endl;
		return 1;
	}
	ComponentTree tree;
	tree.load(argv[1]);
	//tree.printTree();
	//tree.printReverseAlphaMapping();
	if(argc == 3) tree.saveGraph(argv[2]);
	else
	{
		char graph_file[255] = "";
		strcat(graph_file, argv[1]);
		strcat(graph_file, ".dot");
		tree.saveGraph(graph_file);
	}
}
