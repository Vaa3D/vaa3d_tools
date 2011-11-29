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
	tree.printTree();
	//tree.printReverseAlphaMapping();

	//char graph_file[255]="";
	//strcat(graph_file, argv[1]);
	//strcat(graph_file, ".dot");
	//tree.saveGraph(graph_file);
}
