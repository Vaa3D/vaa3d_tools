#include "neuron_separator_plugin.h"
#include <iostream>
#include <math.h>
#include "basic_surf_objs.h"
#include <vector>
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
#include <algorithm>
#include <vector>

using namespace std;

QVector< QVector<V3DLONG> > neuronSeparator::childIndexTable(NeuronTree& nt)
{
	QVector< QVector<V3DLONG> > childs;

	V3DLONG neuronNum = nt.listNeuron.size();
	childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
	V3DLONG* flag = new V3DLONG[neuronNum];
	double* segment_length = new double[neuronNum];
	V3DLONG* parent_id = new V3DLONG[neuronNum];

	for (V3DLONG i=0; i<neuronNum; i++)
	{
		flag[i] = 1;
		segment_length[i] = 100000.00;
		parent_id[i] = -1;
		V3DLONG par = nt.listNeuron[i].pn;
		if (par < 0) continue;
		childs[nt.hashNeuron.value(par)].push_back(i);
	}

	return childs;
}

void neuronSeparator::findPath(QVector< QVector<V3DLONG> >& childList, NeuronTree& nt, long int wishedSomaID, long int excludedSomaID, QList< QList<NeuronSWC> >& outputPathList)
{
	this->forward = true;
	QList<NeuronSWC> nodeList = nt.listNeuron;
	QList<NeuronSWC>::iterator startIt_wished = nodeList.begin();
	QList<NeuronSWC>::iterator startIt_excluded = nodeList.begin();
	for (size_t i=0; i<nodeList.size(); ++i)
	{
		if (nodeList[i].n == wishedSomaID) startIt_wished += ptrdiff_t(i);
		if (nodeList[i].n == excludedSomaID) startIt_excluded += ptrdiff_t(i);
		if (startIt_wished->n*startIt_excluded->n == wishedSomaID*excludedSomaID) break;
	}
	
	long int latestAncestorLoc = findLatestAncestor(this->circle, nt, childList, *startIt_wished, *startIt_excluded, forward);
	cout << "-- latest ancestor location: " << latestAncestorLoc << endl;
	NeuronSWC latestAncestor = *(nodeList.begin() + ptrdiff_t(latestAncestorLoc));

	if (latestAncestorLoc != 0)
	{
		QList<NeuronSWC> path, path1, path2;
		backwardPath(path1, nt, *startIt_excluded, latestAncestor);
		backwardPath(path2, nt, *startIt_wished, latestAncestor);
		//cout << path1.size() << " " << path2.size() << endl;
		for (QList<NeuronSWC>::iterator it=path2.begin(); it!=path2.end()-1; ++it) path1.push_back(*it);
		//cout << path1.size() << endl;
		outputPathList.push_back(path1);
		downwardBFS(path, nt, *startIt_wished, *startIt_excluded, childList);
		outputPathList.push_back(path);
	}
	else 
	{
		if (forward == true)
		{
			QList<NeuronSWC> path;
			backwardPath(path, nt, *startIt_excluded, *startIt_wished);
			outputPathList.push_back(path);
			path.clear();
			downwardBFS(path, nt, *startIt_wished, *startIt_excluded, childList);
			outputPathList.push_back(path);
		}
		else if (forward == false)
		{
			QList<NeuronSWC> path;
			backwardPath(path, nt, *startIt_wished, *startIt_excluded);
			outputPathList.push_back(path);
			path.clear();
			downwardBFS(path, nt, *startIt_excluded, *startIt_wished, childList);
			outputPathList.push_back(path);
		}
	}

	return;
}

void neuronSeparator::breakPathMorph(NeuronTree& nt, QList<NeuronSWC>& path, QVector< QVector<V3DLONG> >& childList, NeuronSWC wishedSoma, NeuronSWC excludedSoma)
{
	vector<branchNodeProfile> branches;
	NeuronSWC head;
	if (this->forward == true) 
	{
		SortSWC(path, path, wishedSoma.n, VOID);
		head = wishedSoma;
	}
	else if (this->forward == false) 
	{
		SortSWC(path, path, excludedSoma.n, VOID);
		head = excludedSoma;
	}

	QHash<int, int> IDloc = nt.hashNeuron;
	for (QList<NeuronSWC>::iterator it=path.begin(); it!=path.end(); ++it)
	{
		if (childList.at(IDloc.value(it->n)).size() > 1) 
		{
			branchNodeProfile branch;
			branch.brachingNum = childList.at(IDloc.value(it->n)).size();
			branch.branchingNode = *it;
			double distance = (head.x-it->x)*(head.x-it->x) + (head.y-it->y)*(head.y-it->y) + (head.z-it->z)*(head.z-it->z);
			distance = sqrt(distance);
			branch.dist = distance;
			branches.push_back(branch);
		}
	}

	
}

long int neuronSeparator::findLatestAncestor(bool& circle, NeuronTree& nt, QVector< QVector<V3DLONG> >& childList, NeuronSWC& wishedSoma, NeuronSWC& excludedSoma, bool& forward)
{
	long int latestAncestorLoc = 0;
	if (circle == false)
	{
		ptrdiff_t neuronLoc = nt.hashNeuron.value(wishedSoma.n);
		QList<NeuronSWC> nodeList = nt.listNeuron;
		QList<NeuronSWC>::iterator checkIt = nodeList.begin() + neuronLoc;
		vector<ptrdiff_t> ancestors;
		size_t ancestorLoc;
		while (checkIt->parent != -1)
		{
			checkIt = nodeList.begin() + ptrdiff_t(nt.hashNeuron.value(checkIt->parent));
			ancestorLoc = nt.hashNeuron.value(checkIt->parent);
			
			if (childList.at(ancestorLoc).size() > 1) ancestors.push_back(ancestorLoc);
			//cout << checkIt->parent << "_" << childList.at(ancestorLoc).size() << " ";
			
			if (checkIt->n == excludedSoma.n)
			{
				forward = false;
				return 0;
			}
		}
		//cout << " -- " << ancestors.size() << " ancestors" << endl;
		
		checkIt = nodeList.begin() + ptrdiff_t(nt.hashNeuron.value(excludedSoma.n));
		while (checkIt->parent != -1)
		{
			checkIt = nodeList.begin() + ptrdiff_t(nt.hashNeuron.value(checkIt->parent));
			for (size_t i=0; i<ancestors.size(); ++i) 
			{
				if (nt.hashNeuron.value(checkIt->n) == ancestors[i]) return ancestors[i];
			}
		}
	}
	else if (circle == true)
	{


	}

	return latestAncestorLoc;
}

void neuronSeparator::backwardPath(QList<NeuronSWC>& tracedSWC, NeuronTree& nt, NeuronSWC& start, NeuronSWC& end)
{
	QList<NeuronSWC> inputList = nt.listNeuron;

	vector<size_t> loc;
	long parent, id, child;
	long int parentLoc;
	QList<long int> childrenLoc;  // both parents and children are represented in the index number on the listNeuron.
	QList<NeuronSWC> parentNode;
	QList<NeuronSWC> childNodes;
	parentLoc = nt.hashNeuron.value(start.parent);
	tracedSWC.push_back(start);

	NeuronSWC currNode;
	NeuronSWC lastNode;
	while (inputList.at(parentLoc).n != end.parent)
	{
		currNode = inputList.at(parentLoc);
		tracedSWC.push_back(currNode);
		lastNode = currNode;
		parentLoc = nt.hashNeuron.value(currNode.parent);
		if (inputList.at(parentLoc).parent == -1) break;
	}
}

void neuronSeparator::downwardBFS(QList<NeuronSWC>& tracedSWC, NeuronTree& nt, NeuronSWC& start, NeuronSWC& end, QVector< QVector<V3DLONG> >& childList)
{
	QList<NeuronSWC> inputList = nt.listNeuron;

	vector<size_t> loc;
	long parent, id, child;
	QList<long int> parentsLoc;
	QList<long int> childrenLoc;  // both parents and children are represented in the index number on the listNeuron.
	QList<NeuronSWC> parentNode;
	QList<NeuronSWC> childNodes;
	parentsLoc.push_back(nt.hashNeuron.value(start.n));
	tracedSWC.push_back(start);

	// childList index number = node's parent's location
	// chilsList content = node's children's location
	ptrdiff_t order;
	do 
	{
		childrenLoc.clear();
		childNodes.clear();
		for (size_t i=0; i<parentsLoc.size(); ++i)
		{
			if (childList[parentsLoc[i]].size() == 0) continue;
			else 
			{
				for (size_t j=0; j<childList[parentsLoc[i]].size(); ++j) 
				{
					childrenLoc.push_back(childList[parentsLoc[i]][j]);
					tracedSWC.push_back(inputList.at(childList[parentsLoc[i]][j]));
				}
			}
		}
		parentsLoc = childrenLoc;

		for (size_t k=0; k<childrenLoc.size(); ++k)
		{
			if (inputList.at(childrenLoc[k]).n == end.n) return;
		}
	} while (childrenLoc.size() > 0);
	
	return;
}

