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
	//tree.printTree();
	//tree.printReverseAlphaMapping();
	//vector<ComponentTree::Node*> nodes = tree.root()->getBreadthFirstNodes();
	vector<ComponentTree::Node*> nodes = tree.root()->getPostOrderNodes();
	for(int i = 0; i < nodes.size(); i++)
	{
		cout<<nodes[i]->getLabel()<<"\t";
	}
	cout<<endl;
	vector<ComponentTree::Node*>::iterator it = nodes.begin();
	while(it != nodes.end())
	{
		ComponentTree::Node* node = *it;
		cout<<node->getChilds().size()<<"\t";
		it++;
	}
}
