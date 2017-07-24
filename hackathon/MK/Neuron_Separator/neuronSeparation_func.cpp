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

void neuronSeparator::somaPathTree(QList<NeuronSWC>& somaPath, QHash<long int, bool>& locLabel, QList< QList<NeuronSWC> >& paths, NeuronTree& nt)
{
	QList<NeuronSWC> nodeList = nt.listNeuron;
	QHash<int, int> nodeHash = nt.hashNeuron;
	for (QList<NeuronSWC>::iterator it=nodeList.begin(); it!=nodeList.end(); ++it) locLabel[nodeHash.value(it->n)] = false;
	//cout << locLabel.size() << endl;

	for (QList< QList<NeuronSWC> >::iterator it=paths.begin(); it!=paths.end(); ++it)
	{
		for (QList<NeuronSWC>::iterator pathIt=it->begin(); pathIt!=it->end(); ++pathIt)
		{
			if (locLabel[nodeHash.value(pathIt->n)] == false) 
			{
				somaPath.push_back(*pathIt);
				locLabel[nodeHash.value(pathIt->n)] = true;
			}
			else if (locLabel[nodeHash.value(pathIt->n)] == false) continue;
		}
	}
}

void neuronSeparator::findPath(QVector< QVector<V3DLONG> >& childList, NeuronTree& nt, long int wishedSomaID, long int excludedSomaID, QList< QList<NeuronSWC> >& outputPathList)
{
	QList<NeuronSWC> nodeList = nt.listNeuron;
	QList<NeuronSWC>::iterator startIt_wished = nodeList.begin();
	QList<NeuronSWC>::iterator startIt_excluded = nodeList.begin();
	for (size_t i=0; i<nodeList.size(); ++i)
	{
		if (nodeList[i].n == wishedSomaID) startIt_wished += ptrdiff_t(i);
		if (nodeList[i].n == excludedSomaID) startIt_excluded += ptrdiff_t(i);
		if (startIt_wished->n*startIt_excluded->n == wishedSomaID*excludedSomaID) break;
	}
	
	long int latestAncestorLoc = findLatestAncestor(this->circle, nt, childList, *startIt_wished, *startIt_excluded);
	cout << "-- latest ancestor location: " << latestAncestorLoc << endl;
	NeuronSWC latestAncestor = *(nodeList.begin() + ptrdiff_t(latestAncestorLoc));

	if (latestAncestorLoc != 0)
	{
		this->forward = 2;
		QList<NeuronSWC> path, path1, path2;
		backwardPath(path1, nt, *startIt_excluded, latestAncestor);
		backwardPath(path2, nt, *startIt_wished, latestAncestor);
		//cout << path1.size() << " " << path2.size() << endl;
		for (QList<NeuronSWC>::iterator it=path2.begin(); it!=path2.end()-1; ++it) path1.push_back(*it);
		//cout << path1.size() << endl;
		outputPathList.push_back(path1);
	}
	else 
	{
		if (this->forward == 1)
		{
			QList<NeuronSWC> path;
			backwardPath(path, nt, *startIt_excluded, *startIt_wished);
			outputPathList.push_back(path);
			path.clear();
		}
		else if (forward == 0)
		{
			QList<NeuronSWC> path;
			backwardPath(path, nt, *startIt_wished, *startIt_excluded);
			outputPathList.push_back(path);
			path.clear();
		}
	}

	return;
}

void neuronSeparator::buildSomaTree()
{
	this->somaSWCTree = readSWC_file("somasTree.swc");
	QList<NeuronSWC> somaNodeList = this->somaSWCTree.listNeuron;
	QHash<int, int> somaIDloc = this->somaSWCTree.hashNeuron;
	
	NeuronSWC startNode;
	QVector< QVector<V3DLONG> > somaChildsTable = childIndexTable(this->somaSWCTree);
	//cout << "test" << endl;
	for (QVector< QVector<V3DLONG> >::iterator it=somaChildsTable.begin(); it!=somaChildsTable.end(); ++it) 
	{
		
			
	}


}

void neuronSeparator::breakSomaPathMorph()
{
	vector<branchNodeProfile> branches;
	QList<NeuronSWC> nodeList = this->inputSWCTree.listNeuron;
	QHash<int, int> IDloc = this->inputSWCTree.hashNeuron;




}

void neuronSeparator::breakPathMorph(NeuronTree& nt, QList<NeuronSWC>& path, QVector< QVector<V3DLONG> >& childList, NeuronSWC wishedSoma, NeuronSWC excludedSoma)
{
	vector<branchNodeProfile> branches;
	QList<NeuronSWC> nodeList = nt.listNeuron;
	NeuronSWC head, tale;
	cout << "forward: " << forward << endl;
	if (this->forward == 1) 
	{
		reverse(path.begin(), path.end());
		head = wishedSoma;
		tale = excludedSoma;
	}
	else if (this->forward == 0) 
	{
		head = excludedSoma;
		tale = wishedSoma;
		//cout << forward << endl;
		//SortSWC(path, path, excludedSoma.n, VOID);
		//head = excludedSoma;
	}

	QHash<int, int> IDloc = nt.hashNeuron;
	for (QList<NeuronSWC>::iterator it=path.begin(); it!=path.end(); ++it)
	{
		if (childList.at(IDloc.value(it->n)).size() > 1) 
		{
			branchNodeProfile branch;
			branch.loc = IDloc.value(it->n);
			branch.branchingNum = childList.at(IDloc.value(it->n)).size();
			branch.branchingNode = *it;
			double distance1 = (head.x-it->x)*(head.x-it->x) + (head.y-it->y)*(head.y-it->y) + (head.z-it->z)*(head.z-it->z);
			double distance2 = (tale.x-it->x)*(tale.x-it->x) + (tale.y-it->y)*(tale.y-it->y) + (tale.z-it->z)*(tale.z-it->z);
			if (distance1 > distance2) 
			{
				branch.dist = sqrt(distance2);
				branch.head = false;
				branch.somaBranchNorm[0] = (branch.branchingNode.x-tale.x) / branch.dist;
				branch.somaBranchNorm[1] = (branch.branchingNode.y-tale.y) / branch.dist;
				branch.somaBranchNorm[2] = (branch.branchingNode.z-tale.z) / branch.dist;
				branch.soma = excludedSoma;
			}
			else 
			{
				branch.dist = sqrt(distance1);
				branch.head = true;
				branch.somaBranchNorm[0] = (branch.branchingNode.x-head.x) / branch.dist;
				branch.somaBranchNorm[1] = (branch.branchingNode.y-head.y) / branch.dist;
				branch.somaBranchNorm[2] = (branch.branchingNode.z-head.z) / branch.dist;
				branch.soma = wishedSoma;
			}
			branches.push_back(branch);
		}
	}
	
	for (vector<branchNodeProfile>::iterator it=branches.begin(); it!=branches.end(); ++it)
	{
		cout << it->somaBranchNorm[0] << " " << it->somaBranchNorm[1] << " " << it->somaBranchNorm[2] << endl;
		cout << it->loc << " " << it->branchingNum << endl; 
		for (size_t i=0; i<it->branchingNum; ++i)
		{
			//cout << childList.at(it->loc)[i] << endl; // branch's child location 
			for (size_t j=0; j<path.size(); ++j)
			{
				//cout << IDloc.value(path[j].n) << " ";
				if (IDloc.value(path[j].n) == childList.at(it->loc)[i])
				{
					cout << IDloc.value(path.at(j).n) << " " << childList.at(it->loc)[i] << endl << endl;
					it->childLoc = childList.at(it->loc).at(i);
					if (it->head == true)
					{
						it->dirIndex = ((it+1)->branchingNode.x-it->branchingNode.x) * it->somaBranchNorm[0] + 
									   ((it+1)->branchingNode.y-it->branchingNode.y) * it->somaBranchNorm[1] +
									   ((it+1)->branchingNode.z-it->branchingNode.z) * it->somaBranchNorm[2];
					}
					else if (it->head == false)
					{
						it->dirIndex = ((it-1)->branchingNode.x-it->branchingNode.x) * it->somaBranchNorm[0] + 
									   ((it-1)->branchingNode.y-it->branchingNode.y) * it->somaBranchNorm[1] +
									   ((it-1)->branchingNode.z-it->branchingNode.z) * it->somaBranchNorm[2];
					}
				}
			}
		}
	}

	for (vector<branchNodeProfile>::iterator it=branches.begin(); it!=branches.end(); ++it) cout << it->branchingNode.n << " " << it->dirIndex << endl;

	QList<NeuronSWC> newList = nodeList;
	for (vector<branchNodeProfile>::iterator it=branches.end()-2; it!=branches.begin()-1; --it)
	{
		if (it->dirIndex < 0)
		{
			if (it->head == true) continue;
			newList[it->childLoc].parent = -1;
			cout << "branch node ID: " << it->branchingNode.n << " cut location: " << it->childLoc << endl;
		}
	}
	for (QList<NeuronSWC>::iterator it=newList.begin(); it!=newList.end(); ++it)
	{
		if (it->parent == -1) this->extractBFS(this->extractedNeuron, newList, *it);
		break;
	}
}

long int neuronSeparator::findLatestAncestor(bool& circle, NeuronTree& nt, QVector< QVector<V3DLONG> >& childList, NeuronSWC& wishedSoma, NeuronSWC& excludedSoma)
{
	long int latestAncestorLoc = 0;
	this->forward = 1;
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
				this->forward = 0;
				return 0;
			}
		}
		if (checkIt->parent == -1) 
		{
			this->forward = 1;
			return 0;

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
		// This part is meant to be implemented for tracing algorithms that allow circle to occur.

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

void neuronSeparator::extractBFS(QList<NeuronSWC>& tracedSWC, QList<NeuronSWC>& inputList, NeuronSWC& start)
{
	vector<size_t> loc;
	long parent, id, child, uncle;
	QList<NeuronSWC> parents;
	QList<NeuronSWC> children;
	parents.push_back(start);

	ptrdiff_t order;
	do 
	{
		loc.clear();
		children.clear();
		for (size_t i=0; i<parents.size(); ++i)
		{
			for (size_t j=0; j<inputList.size(); ++j)
			{
				parent = inputList[j].parent;
				if (parent == parents[i].n) 
				{	
					NeuronSWC outputLine;
					outputLine.n = inputList[j].n;
					outputLine.type = inputList[j].type;
					outputLine.x = inputList[j].x;
					outputLine.y = inputList[j].y;
					outputLine.z = inputList[j].z;
					outputLine.radius = inputList[j].radius;
					outputLine.parent = inputList[j].parent;
					tracedSWC.append(outputLine);
					
					loc.push_back(j);
					children.push_back(outputLine);
					//cout << outputLine.n << " " << outputLine.parent << endl;
				}
			}
		}

		order = 0;
		sort(loc.begin(), loc.end());
		ptrdiff_t ptrLoc = 0;
		for (vector<size_t>::iterator nodeIt=loc.begin(); nodeIt!=loc.end(); ++nodeIt)
		{
			//cout << *nodeIt << " ";
			inputList.erase(inputList.begin() + ptrdiff_t(*nodeIt) - order);
			size_t ptrLoc = ptrdiff_t(*nodeIt) - order;
			//cout << "deleted location: " << ptrLoc << " ";
			++order;
		}
		//cout << endl << endl;
		parents = children;
	} while (children.size() > 0);
	
	return;
}