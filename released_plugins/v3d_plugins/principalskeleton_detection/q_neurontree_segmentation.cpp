/*
 * q_neurontree_segmentation.cpp
 *
 *  Created on: Nov 13, 2009
 *      Author: Lei Qu
 */

#include "q_neurontree_segmentation.h"


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
		vector< vector<long> > &vecvec_neuronbranchnode_ind)
{
	//check parameters
	if(nt_neurontree.listNeuron.size()<=0)
	{
		printf("ERROR: q_neurontree2branches_recursively: input vector is empty.\n");
		return false;
	}
	if(vecvec_neuronbranchnode_ind.size())
	{
		printf("WARNNING: q_neurontree2branches_recursively: output vector is not null, original contents are cleared!\n");
		vecvec_neuronbranchnode_ind.clear();
	}

	//reorgnize the input neuron tree into dual-directional link
	//find the root node index in the same time
	long root_ind=-1;
	long l_nnode=nt_neurontree.listNeuron.size();
	vector<NeuronDualDirLinkElement> vec_neurondualdirlink(l_nnode,NeuronDualDirLinkElement());

	for(long i=0;i<l_nnode;i++)
	{
		vec_neurondualdirlink[i].l_id=nt_neurontree.listNeuron[i].n;
		vec_neurondualdirlink[i].l_parent_id=nt_neurontree.listNeuron[i].pn;

		vec_neurondualdirlink[i].l_parent_ind=nt_neurontree.hashNeuron[nt_neurontree.listNeuron[i].pn];
		if(vec_neurondualdirlink[i].l_parent_id==-1)
		{
			root_ind=i;
			continue;
		}
		vec_neurondualdirlink[vec_neurondualdirlink[i].l_parent_ind].vec_child_ind.push_back(i);
	}

	//search neuron segment/branch recursively from the root point
	if(!q_neurontree2branches_recurfunc(vec_neurondualdirlink,root_ind,vecvec_neuronbranchnode_ind))
	{
		printf("ERROR: q_neurontree2branches_recurfunc() return false!");
		return false;
	}

	return true;
}

//Recursion function that search down the neuron tree
//Since this function only search DOWN the neuron tree, make sure you start from the ROOT node if you want get all neuron segments/branches
bool q_neurontree2branches_recurfunc(
		const vector<NeuronDualDirLinkElement> &vec_neurondualdir,
		const long l_startnode_ind,
		vector< vector<long> > &vecvec_neuronbranchnode_ind)
{
	//check parameters
	if(vec_neurondualdir.size()<=0)
	{
		printf("ERROR: q_neurontree2branches_recurfunc: input vector is empty.\n");
		return false;
	}
	if(l_startnode_ind<0 || l_startnode_ind>=vec_neurondualdir.size())
	{
		printf("ERROR: q_neurontree2branches_recurfunc: input startnode index is invalid.\n");
		return false;
	}

	vector<long> vec_curbranch_ind;
	long l_curnode_ind=l_startnode_ind;

	//push the parent node into current branch (if current node is not root node)
	if(vec_neurondualdir[l_curnode_ind].l_parent_id!=-1)
	{
		vec_curbranch_ind.push_back(vec_neurondualdir[l_curnode_ind].l_parent_ind);
	}
	//push current node into current branch
	vec_curbranch_ind.push_back(l_curnode_ind);
	//search down the branch recursively
	do
	{
		if(vec_neurondualdir[l_curnode_ind].vec_child_ind.size()==1)		//in the middle of branch --> continue search down
		{
			l_curnode_ind=vec_neurondualdir[l_curnode_ind].vec_child_ind[0];
			vec_curbranch_ind.push_back(l_curnode_ind);
		}
		else if(vec_neurondualdir[l_curnode_ind].vec_child_ind.size()>1)	//is the joint point --> recursively search down every children
		{
			long l_nchild=vec_neurondualdir[l_curnode_ind].vec_child_ind.size();

			for(long i=0;i<l_nchild;i++)
			{
				long l_start_ind_new=vec_neurondualdir[l_curnode_ind].vec_child_ind[i];
				q_neurontree2branches_recurfunc(vec_neurondualdir,l_start_ind_new,vecvec_neuronbranchnode_ind);
			}
			break;
		}
		else if(vec_neurondualdir[l_curnode_ind].vec_child_ind.size()==0)	//is the end of neuron tree --> break
		{
			break;
		}
	}while(1);

	//push current branch into vec_branch
	vecvec_neuronbranchnode_ind.push_back(vec_curbranch_ind);

	return true;
}
