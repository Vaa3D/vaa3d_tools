/*
 * q_neurontree_segmentation.h
 *
 *  Created on: Nov 13, 2009
 *      Author: Lei Qu
 */

#ifndef __Q_NEURONTREE_SEGMENTATION_H__
#define __Q_NEURONTREE_SEGMENTATION_H__

#include <stdio.h>
#include <vector>
using namespace std;

#include "../basic_c_fun/basic_surf_objs.h"

struct NeuronDualDirLinkElement
{
	long			l_id;			//id of current node (as in swc file)
	long			l_parent_id;	//id of parent node  (as in swc file)
	long 			l_parent_ind;	//index of parent node in listNeuron of NeuronTree (every node can only has one parent)
	vector<long> 	vec_child_ind;	//index array of child nodes in listNeuron of NeuronTree (node may has multi children)

	NeuronDualDirLinkElement() {l_id=l_parent_id=l_parent_ind=0;vec_child_ind.clear();}
};

//This function search down the neuron tree for segments/branches from root node in a recursive way
//
//The searched neuron segments/branches satisfy:
//(1). the two end node of segment is either root node or end node or joint node;
//(2). any node (except the two end one) within a segment is not root, end or joint node;
//
//Parameters:
//nt_neurontree:				input neuron tree for segmentation (output of readSWC_file)
//vecvec_neuronbranchnode_ind:	output branches along with their node index
//
bool q_neurontree2branches_recursively(
		const NeuronTree &nt_neurontree,
		vector< vector<long> > &vecvec_neuronbranchnode_ind);

//Recursion function that search down the neuron tree
//Since this function only search DOWN the neuron tree, make sure you start from the ROOT node if you want get all neuron segments/branches
bool q_neurontree2branches_recurfunc(
		const vector<NeuronDualDirLinkElement> &vec_neurondualdir,
		const long l_startnode_ind,
		vector< vector<long> > &vecvec_neuronbranchnode_ind);


#endif
