#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <math.h>

#include "basic_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"

#include "NeuronStructExplorer.h"
#include "NeuronStructUtilities.h"

#include "neuron_separator_plugin.h"


using namespace std;

#define MAX_NODE_NUM_ON_TAIL2HEAD_PATH 100000

neuronSeparator::neuronSeparator() // Need this constructor to put crucialNodes in stack instead of heap, otherwise the addresses of its elements may change.
{
	crucialNodes.resize(500);
}

QVector< QVector<V3DLONG> > neuronSeparator::childIndexTable(NeuronTree& nt)
{
	QVector< QVector<V3DLONG> > childs; // indices of the childs of a given parent index
	V3DLONG neuronNum = nt.listNeuron.size();
	childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

	for (V3DLONG i=0; i<neuronNum; ++i)
	{
		V3DLONG par = nt.listNeuron[i].pn; // pn = parent id
		if (par < 0) continue;
		childs[nt.hashNeuron.value(par)].push_back(i); // value(par) = index of par
	}

	return childs;
}

QVector< QVector<V3DLONG> > neuronSeparator::mkChildTableScratch(QList<NeuronSWC>& inputSWC)
{
	// This method creates nodeLoc-childLoc table.

	QVector< QVector<V3DLONG> > childs;
	long int neuronNum = inputSWC.size();
	childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

	for (size_t i=0; i<neuronNum; ++i)
	{
		long int paID = inputSWC[i].parent;
		if (paID == -1) continue;
		for (size_t j=0; j<neuronNum; ++j)
		{
			if (inputSWC[j].n == paID) childs[j].push_back(i);
		}
	}

	return childs;
}

vector<long int> neuronSeparator::mkPaTableScratch(QList<NeuronSWC>& inputSWC)
{
	// This method creates nodeLoc-paLoc table.

	long int neuronNum = inputSWC.size();
	vector<long int> pa(neuronNum, 0);

	for (size_t i=0; i<neuronNum; ++i)
	{
		long int paID = inputSWC[i].parent;
		if (paID == -1) 
		{
			pa[i] = -1;
			continue;
		}

		for (size_t j=0; j<neuronNum; ++j)
		{
			if (inputSWC[j].n == paID) pa[i] = j;
		}
	}

	return pa;
}

map<long int, size_t> neuronSeparator::hashScratchMap(QList<NeuronSWC>& inputSWC)
{
	// This method creates nodeID-nodeLoc table. 

	map<long int, size_t> hashTable;

	int inputSWCsize = inputSWC.size();
	for (int i=0; i<inputSWCsize; ++i)
	{
		int ID = inputSWC[i].n;
		for (int j=0; j<inputSWCsize; ++j)
		{
			if (inputSWC[j].n == ID)
			{
				hashTable[ID] = j;
				break;
			}
		}
	}

	return hashTable;
}

void neuronSeparator::getMergedPath(QList<NeuronSWC>& somaPath, QHash<long int, bool>& locLabel, QList< QList<NeuronSWC> >& paths, NeuronTree& nt)
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
				locLabel[nodeHash.value(pathIt->n)] = true; // labeling nodes that have been included in the somaPath 'true', avoiding repeated inclusion 
			}
			else if (locLabel[nodeHash.value(pathIt->n)] == true) continue;
		}
	}
}

void neuronSeparator::backwardPath(QList<NeuronSWC>& tracedSWC, NeuronTree& nt, NeuronSWC& start, NeuronSWC& end)
{
	// This method traces a path backward; from end to start.

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
		if (inputList.at(parentLoc).parent == -1) 
		{
			tracedSWC.push_back(inputList.at(parentLoc));
			break;
		}
	}
}

QList<NeuronSWC> neuronSeparator::findPath(QVector< QVector<V3DLONG> >& childList, NeuronTree& nt, long int wishedSomaID, long int excludedSomaID)
{
	map<int, size_t> node2locMap;
	map<int, vector<size_t>> node2childLocMap;
	NeuronStructUtil::node2loc_node2childLocMap(nt.listNeuron, node2locMap, node2childLocMap);

	QList<NeuronSWC> nodeList = nt.listNeuron;
	QList<NeuronSWC>::iterator startIt_wished = nodeList.begin();
	QList<NeuronSWC>::iterator startIt_excluded = nodeList.begin();
	for (size_t i=0; i<nodeList.size(); ++i)
	{
		if (nodeList[i].n == wishedSomaID) startIt_wished += ptrdiff_t(i);
		if (nodeList[i].n == excludedSomaID) startIt_excluded += ptrdiff_t(i);
		if (startIt_wished->n*startIt_excluded->n == wishedSomaID*excludedSomaID) break; // if both startIt_wished and startIt_excluded are found, stop iterating
	}
	
	long int latestAncestorLoc = findLatestCommonAncestor(this->circle, nt, childList, *startIt_wished, *startIt_excluded);
	somaNode somaNodeWished;
	//cout << "-- latest ancestor location: " << latestAncestorLoc << endl;
	NeuronSWC latestAncestor = *(nodeList.begin() + ptrdiff_t(latestAncestorLoc));

	if (this->branchAncestor == true)
	{
		QList<NeuronSWC> path1, path2;
		//NeuronStructUtil::upstreamPath(nt.listNeuron, path1, latestAncestor, *startIt_excluded, node2locMap);
		//NeuronStructUtil::upstreamPath(nt.listNeuron, path2, latestAncestor, *startIt_wished, node2locMap);
		backwardPath(path1, nt, *startIt_excluded, latestAncestor);
		backwardPath(path2, nt, *startIt_wished, latestAncestor);
		//cout << path1.size() << " " << path2.size() << endl;
		for (QList<NeuronSWC>::iterator it=path2.begin(); it!=path2.end()-1; ++it) path1.push_back(*it);

		NeuronTree somaTreePartial;
		somaTreePartial.listNeuron = path1;
		QString name = this->outputPath + "/" + QString::number(wishedSomaID) + "_" + QString::number(excludedSomaID) + ".swc";
		writeSWC_file(name, somaTreePartial);
		return path1;
	}
	else 
	{
		if (this->forward == 1)
		{
			QList<NeuronSWC> path;
			//NeuronStructUtil::upstreamPath(nt.listNeuron, path, *startIt_wished, *startIt_excluded, node2locMap);
			backwardPath(path, nt, *startIt_excluded, *startIt_wished);
			
			NeuronTree somaTreePartial;
			somaTreePartial.listNeuron = path;
			QString name = this->outputPath + "/" + QString::number(wishedSomaID) + "_" + QString::number(excludedSomaID) + ".swc";
			writeSWC_file(name, somaTreePartial);
			return path;
		}
		else if (forward == 0)
		{
			QList<NeuronSWC> path;
			//NeuronStructUtil::upstreamPath(nt.listNeuron, path, *startIt_excluded, *startIt_wished, node2locMap);
			backwardPath(path, nt, *startIt_wished, *startIt_excluded);
			
			NeuronTree somaTreePartial;
			somaTreePartial.listNeuron = path;
			QString name = this->outputPath + "/" + QString::number(wishedSomaID) + "_" + QString::number(excludedSomaID) + ".swc";
			writeSWC_file(name, somaTreePartial);
			return path;
		}
	}

	QList<NeuronSWC> dummy;
	return dummy;
}

long int neuronSeparator::findLatestCommonAncestor(bool& circle, NeuronTree& nt, QVector< QVector<V3DLONG> >& childList, NeuronSWC& wishedSoma, NeuronSWC& excludedSoma)
{
	// This method identifies the location of the latest common ancestor of 2 given nodes. (A common ancester is not necessarily a soma, could be a branching point.)

	this->forward = 1;
	if (circle == false)
	{
		QList<NeuronSWC> nodeList = nt.listNeuron;
		vector<ptrdiff_t> ancestors;
		ancestors.clear();
		size_t ancestorLoc;
		ptrdiff_t neuronLoc = nt.hashNeuron.value(wishedSoma.n);

		QList<NeuronSWC>::iterator checkIt = nodeList.begin() + neuronLoc; // this is for the following while loop initiation
		do
		{
			checkIt = nodeList.begin() + ptrdiff_t(nt.hashNeuron.value(checkIt->parent));
			ancestorLoc = nt.hashNeuron.value(checkIt->parent);
			
			if (childList.at(ancestorLoc).size() > 1) ancestors.push_back(ancestorLoc);
			//cout << checkIt->parent << "_" << childList.at(ancestorLoc).size() << " ";
			
			if (checkIt->n == excludedSoma.n)
			{
				this->branchAncestor = false;
				this->forward = 0;
				return ancestorLoc;
			}
		} while (checkIt->parent != -1);
		
		checkIt = nodeList.begin() + ptrdiff_t(nt.hashNeuron.value(excludedSoma.n));
		do
		{
			checkIt = nodeList.begin() + ptrdiff_t(nt.hashNeuron.value(checkIt->parent));
			for (size_t i=0; i<ancestors.size(); ++i) 
			{
				if (nt.hashNeuron.value(checkIt->n) == ancestors[i]) 
				{
					this->forward = 2;
					this->branchAncestor = true;	
					return ancestors[i];
				}
				if (nt.hashNeuron.value(checkIt->n) == nt.hashNeuron.value(wishedSoma.n))
				{
					this->branchAncestor = false;
					return nt.hashNeuron.value(checkIt->n);
				}
			}
		} while (checkIt->parent != -1);
	}
	else if (circle == true)
	{
		// This part is meant to be implemented for tracing algorithms that allow circle to occur.
	}
}

void neuronSeparator::buildSomaTree()
{
	// This method builds soma tree, identify the hierarchy, and its braching profile.

	NeuronTree somaTree;
	somaTree.listNeuron = this->somaPath;
	QVector< QVector<V3DLONG> > somaChildsTable;
	vector<long int> somaPaTable;
	somaChildsTable = mkChildTableScratch(this->somaPath);
	somaPaTable = mkPaTableScratch(this->somaPath);
	for (vector<long int>::iterator it=somaPaTable.begin(); it!=somaPaTable.end(); ++it)
	{
		if (*it == 0) 
		{
			size_t loc = size_t(it - somaPaTable.begin());
			this->somaPath[loc].parent = -1;
			break;
		}
	}
	//for (QVector< QVector<V3DLONG> >::iterator it=somaChildsTable.begin(); it!=somaChildsTable.end(); ++it) cout << it->size() << " ";
	//cout << endl << endl;
	//for (vector<long int>::iterator it=somaPaTable.begin(); it!=somaPaTable.end(); ++it) cout << *it << " ";

	vector< vector<size_t> > tails2headList;
	vector<size_t> tail2head;
	for (QVector< QVector<V3DLONG> >::iterator it=somaChildsTable.begin(); it!=somaChildsTable.end(); ++it) 
	{
		if (it->size() == 0)
		{
			size_t loc = size_t(it - somaChildsTable.begin());
			tail2head.push_back(loc);
			size_t paLoc;
			do
			{
				paLoc = somaPaTable[loc];
				if (somaChildsTable[paLoc].size() > 1) // identifying branch points on soma path
				{
					tail2head.push_back(paLoc);
				}
				else if (somaChildsTable[paLoc].size() == 1)
				{
					for (size_t j=0; j<somaIDs.size(); ++j)
					{
						if (this->somaPath[paLoc].n == this->somaIDs[j]) 
						{
							tail2head.push_back(paLoc);
							break;
						}
					}
				}
				loc = paLoc;
			} while (this->somaPath[loc].parent != -1);

			//reverse(tail2head.begin(), tail2head.end());
			tails2headList.push_back(tail2head);
			tail2head.clear();
		}
	}

	cout << endl << "===== Analyzing soma path =====" << endl;
	for (vector< vector<size_t> >::iterator itList=tails2headList.begin(); itList!=tails2headList.end(); ++itList)
	{
		cout << "  -- tail to head path " << (itList-tails2headList.begin()+1) << ": ";
		for (vector<size_t>::iterator it=itList->begin(); it!=itList->end(); ++it) cout << *it << " ";
		
		cout << endl;
	}
	cout << endl;

	this->crucialNodes.clear();
	size_t nodeCount = 0;
	for (vector< vector<size_t> >::iterator itList=tails2headList.begin(); itList!=tails2headList.end(); ++itList)
	{
		for (vector<size_t>::iterator it=itList->begin(); it!=itList->end(); ++it)
		{
			if (this->crucialNodeHash[*it] == 0)
			{
				somaNode crucialNode;
				this->crucialNodes.push_back(crucialNode);
				this->crucialNodes[nodeCount].node = somaPath[*it];
				this->crucialNodeHash[*it] = &(crucialNodes[nodeCount]);
				++nodeCount;
				//cout << "location on the soma path: " << *it << endl;
				//cout << " - memory address: " << crucialNodeHash[*it] << endl;
				//cout << " - ID: " << crucialNodeHash[*it]->node.n << endl << endl;
			}
		}
	}

	somaNode* curSomaNodePtr;
	somaNode* paSomaNodePtr;
	vector<size_t> handled;
	handled.clear();
	for (vector< vector<size_t> >::iterator itList=tails2headList.begin(); itList!=tails2headList.end(); ++itList)
	{
		for (vector<size_t>::iterator it=itList->begin(); it!=itList->end()-1; ++it)
		{
			bool check = false;
			for (vector<size_t>::iterator checkIt=handled.begin(); checkIt!=handled.end(); ++checkIt)
			{
				if (*checkIt == *it)
				{
					check = true;
					break;
				}
			}
			if (check == true) continue;

			if (check == false)
			{
				curSomaNodePtr = this->crucialNodeHash[*it];
				curSomaNodePtr->parent.push_back(this->crucialNodeHash[*(it+1)]);
				paSomaNodePtr = this->crucialNodeHash[*(it+1)];
				paSomaNodePtr->childrenSomas.push_back(curSomaNodePtr);

				handled.push_back(*it);

				//cout << "loc and mem addr: " << *it << " " << curSomaNodePtr << " / " << *(it+1) << " " << paSomaNodePtr << endl;
				//cout << "  -- parent ID: " << curSomaNodePtr->node.n << " children ID: " << paSomaNodePtr->node.n << endl;
			}
		}
	}

	int pathSize = tails2headList[0].size();
	size_t headLoc = tails2headList[0][pathSize-1];
	this->somaTreePtr = this->crucialNodeHash[headLoc];
	this->somaTreePtr->headSoma = true;
	this->somaTreePtr->soma = true;

	int levelCount = 1;
	int childrenCount;
	vector<somaNode*> curLevelPtr, nextLevelPtr;
	curLevelPtr.push_back(this->somaTreePtr);
	do
	{
		childrenCount = 0;
		int childrenSize = 0;
		nextLevelPtr.clear();
		for (vector<somaNode*>::iterator it=curLevelPtr.begin(); it!=curLevelPtr.end(); ++it) 
		{
			(*it)->level = levelCount;
			childrenSize = (*it)->childrenSomas.size();
			//cout << "children size: " << childrenSize << endl;
			if (childrenSize > 1)
			{
				for (size_t i=0; i<this->somaIDs.size(); ++i)
				{
					if (this->somaIDs[i] == (*it)->node.n)
					{
						(*it)->middleSoma = true;
						(*it)->soma = true;
					}
					break;
				}
				(*it)->branch = true;
			}
			else if (childrenSize == 1 && (*it)->parent.size() > 0) 
			{
				(*it)->middleSoma = true;
				(*it)->soma = true;
			}
			else if (childrenSize == 0) 
			{
				(*it)->tailSoma = true;
				(*it)->soma = true;
			}
			childrenCount = childrenCount + childrenSize;

			for (size_t j=0; j<childrenSize; ++j) nextLevelPtr.push_back((*it)->childrenSomas[j]);
		}	
		curLevelPtr = nextLevelPtr;
		++levelCount;
	} while (childrenCount > 0);	
}

void neuronSeparator::breakPathMorph2(const NeuronTree& originalTree)
{
	map<int, size_t> nodeLocMap;
	map<int, vector<size_t>> node2childLocMap;
	map<int, vector<size_t>> branch2childLocMap;
	NeuronStructUtil::node2loc_node2childLocMap(originalTree.listNeuron, nodeLocMap, node2childLocMap);
	for (map<int, vector<size_t>>::iterator it = node2childLocMap.begin(); it != node2childLocMap.end(); ++it)
		if (it->second.size() > 1) branch2childLocMap.insert(pair<int, vector<size_t>>(it->first, it->second));
	
	map<int, size_t> somaNodeLocMap;
	map<int, vector<size_t>> somaNode2childLocMap;
	map<int, vector<size_t>> somaBranch2childLocMap;
	NeuronStructUtil::node2loc_node2childLocMap(this->somaPath, somaNodeLocMap, somaNode2childLocMap);
	
	cout << endl << "===== Identifying nodes to break =====" << endl;
	cout << "crucial node number: " << this->crucialNodes.size() << endl;

	vector<somaNode*> curLevelPtr, nextLevelPtr;
	curLevelPtr.push_back(this->somaTreePtr);
	this->brokenSomaPath = this->somaPath;

	NeuronSWC head, tail;
	int childrenCount;
	long int segmentHeadID;
	long int segmentTailID;
	do
	{
		childrenCount = 0;
		int childrenSize = 0;
		nextLevelPtr.clear();
		for (vector<somaNode*>::iterator it = curLevelPtr.begin(); it != curLevelPtr.end(); ++it) // parent crucial-node loop
		{
			segmentHeadID = (*it)->node.n;
			childrenSize = (*it)->childrenSomas.size();
			childrenCount = childrenCount + childrenSize;
			cout << endl << endl << "children size of " << segmentHeadID << ": " << childrenSize << " -> ";

			somaNode** childrenAddr = new somaNode*[childrenSize];
			int childCount = 0;
			this->branchSomaMerge = false;
			for (vector<somaNode*>::iterator listIt = ((*it)->childrenSomas).begin(); listIt != ((*it)->childrenSomas).end(); ++listIt)
			{
				childrenAddr[childCount] = *listIt;
				nextLevelPtr.push_back(childrenAddr[childCount]);
				if (abs(segmentHeadID - (*listIt)->node.n) <= 5) this->branchSomaMerge = true;
				cout << childrenAddr[childCount]->node.n << " ";
				++childCount;
			} cout << endl;

			//if (branchCutHash[(*it)->node.n] == true) continue;

			for (size_t i = 0; i<childrenSize; ++i) // children crucial-node loop
			{
				segmentTailID = childrenAddr[i]->node.n;
				cout << "--\n Head: " << segmentHeadID << endl << " Tail: " << segmentTailID << endl;

				if ((*it)->soma == true) // parent crucial node
				{
					if (childrenAddr[i]->soma == true)
					{
						QList<NeuronSWC> pathAnalyze;
						head = this->inputSWCTree.listNeuron.at(nodeLocMap.at(segmentHeadID));
						tail = this->inputSWCTree.listNeuron.at(nodeLocMap.at(segmentTailID));
						NeuronStructUtil::upstreamPath(this->inputSWCTree.listNeuron, pathAnalyze, head, tail, nodeLocMap);
						reverse(pathAnalyze.begin(), pathAnalyze.end());
						//backwardPath(pathAnalyze, this->inputSWCTree, tail, head); // extract the path of given head node and tail node
						pathAnalyze.pop_front();
						pathAnalyze.pop_back();
						cout << "Excluded somas at the 2 ends..";
						cout << "Path type: soma to soma, path size: " << pathAnalyze.size() << endl;

						double radAngleMax = 0;
						int twistedBranchID = 0;
						deque<vector<float>> upstreamMaxQue;
						deque<vector<float>> downstreamMaxQue;
						for (QList<NeuronSWC>::iterator it = pathAnalyze.begin(); it != pathAnalyze.end(); ++it)
						{
							if (branch2childLocMap.find(it->n) != branch2childLocMap.end())
							{
								vector<NeuronSWC> upstream;
								NeuronStructUtil::upstreamPath(this->somaPath, upstream, *it, somaNodeLocMap);
								reverse(upstream.begin(), upstream.end());

								vector<NeuronSWC> downstream;
								downstream.push_back(*it);
								if (somaNode2childLocMap.find(it->n) == somaNode2childLocMap.end())
								{
									upstream.clear();
									downstream.clear();
									continue;
								}
								int childID = this->somaPath.at(*somaNode2childLocMap.at(it->n).begin()).n;
								cout << "| ";
								while (downstream.size() < 10)
								{
									cout << childID << " ";
									downstream.push_back(this->somaPath.at(somaNodeLocMap.at(childID)));
									if (somaNode2childLocMap.find(childID) == somaNode2childLocMap.end()) break;
									childID = this->somaPath.at(*somaNode2childLocMap.at(childID).begin()).n;
								}

								if (upstream.size() <= 3 || downstream.size() <= 3)
								{
									cout << endl;
									continue;
								}

								vector<float> vector1(3);
								vector1[0] = upstream.front().x - upstream.back().x;
								vector1[1] = upstream.front().y - upstream.back().y;
								vector1[2] = upstream.front().z - upstream.back().z;
								vector<float> vector2(3);
								vector2[0] = downstream.back().x - downstream.front().x;
								vector2[1] = downstream.back().y - downstream.front().y;
								vector2[2] = downstream.back().z - downstream.front().z;
								double radAngle = NeuronStructExplorer::getRadAngle(vector1, vector2);
								cout << " -- " << it->n << ": " << radAngle << endl;

								if (radAngle > radAngleMax)
								{
									radAngleMax = radAngle;
									twistedBranchID = it->n;
									upstreamMaxQue.clear();
									downstreamMaxQue.clear();
									vector<float> locVec(3);

									for (vector<NeuronSWC>::iterator vecIt = upstream.begin(); vecIt != upstream.end(); ++vecIt)
									{
										locVec[0] = vecIt->x;
										locVec[1] = vecIt->y;
										locVec[2] = vecIt->z;
										upstreamMaxQue.push_back(locVec);
									}
									locVec[0] = it->x;
									locVec[1] = it->y;
									locVec[2] = it->z;
									upstreamMaxQue.push_front(locVec);

									for (vector<NeuronSWC>::iterator vecIt = downstream.begin(); vecIt != downstream.end(); ++vecIt)
									{
										locVec[0] = vecIt->x;
										locVec[1] = vecIt->y;
										locVec[2] = vecIt->z;
										downstreamMaxQue.push_back(locVec);
									}
									locVec[0] = it->x;
									locVec[1] = it->y;
									locVec[2] = it->z;
									downstreamMaxQue.push_front(locVec);
								}
								upstream.clear();
								downstream.clear();
							}
						}

						if (radAngleMax >= 0.5)
						{
							vector<vector<float>> upstreamMax;
							upstreamMax.insert(upstreamMax.begin(), upstreamMaxQue.begin(), upstreamMaxQue.end());
							vector<vector<float>> downstreamMax;
							downstreamMax.insert(downstreamMax.begin(), downstreamMaxQue.begin(), downstreamMaxQue.end());
							double upTurning = NeuronStructExplorer::selfTurningRadAngleSum(upstreamMax);
							double downTurning = NeuronStructExplorer::selfTurningRadAngleSum(downstreamMax);

							int nodeCutID;
							if (upTurning > downTurning) nodeCutID = originalTree.listNeuron.at(nodeLocMap.at(twistedBranchID)).parent;
							else if (downTurning > upTurning) nodeCutID = originalTree.listNeuron.at(*node2childLocMap.at(twistedBranchID).begin()).n;

							this->nodeToBeCutID.push_back(nodeCutID);
							cout << " ID of node to be cut: " << nodeCutID << endl;
						}	
						else
						{
							int nodeCutID = 0;

							vector<vector<int>> consecutive1s_vector;
							vector<int> consecutive1s;
							consecutive1s_vector.push_back(consecutive1s);
							for (QList<NeuronSWC>::iterator rIt = pathAnalyze.begin(); rIt != pathAnalyze.end(); ++rIt)
							{
								if (rIt->radius == 1) consecutive1s_vector.back().push_back(rIt->n);
								else
								{
									consecutive1s.clear();
									consecutive1s_vector.push_back(consecutive1s);
								}
							}

							vector<int> consecutive1sMax;
							consecutive1sMax.clear();
							for (vector<vector<int>>::iterator conIt = consecutive1s_vector.begin(); conIt != consecutive1s_vector.end(); ++conIt)
								if (conIt->size() > consecutive1sMax.size()) consecutive1sMax = *conIt;
							if (consecutive1sMax.size() >= 3) nodeCutID = *(consecutive1sMax.begin() + ptrdiff_t(consecutive1sMax.size() / 2));
							else
							{
								nodeCutID = this->pathScissor(pathAnalyze);
								if (nodeCutID == 0)
								{
									ptrdiff_t pathMiddle = ptrdiff_t(pathAnalyze.size() / 2);
									nodeCutID = (pathAnalyze.begin() + pathMiddle)->n;
								}
							}

							deque<int> branchCheckRangeIDs;
							branchCheckRangeIDs.push_back(nodeCutID);
							int bParentID = this->somaPath.at(somaNodeLocMap.at(nodeCutID)).parent;
							int bChildID = this->somaPath.at(*somaNode2childLocMap.at(nodeCutID).begin()).n;
							int rangeCount = 0;
							bool up = false, down = false;
							int branchRouteID = 0;
							while (rangeCount <= 10)
							{
								++rangeCount;

								if (branch2childLocMap.find(bParentID) != branch2childLocMap.end())
								{
									for (QList<NeuronSWC>::iterator withinIt = pathAnalyze.begin(); withinIt != pathAnalyze.end() - 1; ++withinIt)
									{
										if (withinIt->n == bParentID)
										{
											branchCheckRangeIDs.push_front(bParentID);
											up = true;

											branchRouteID = bParentID;
											vector<size_t> childLocs = node2childLocMap.at(branchRouteID);
											for (vector<size_t>::iterator branchChildIt = childLocs.begin(); branchChildIt != childLocs.end(); ++branchChildIt)
											{
												if (somaNodeLocMap.find(inputSWCTree.listNeuron.at(*branchChildIt).n) == somaNodeLocMap.end())
												{
													int branchCount = 0;
													branchRouteID = inputSWCTree.listNeuron.at(*branchChildIt).n;
													while (branchCount <= 9)
													{
														++branchCount;
														if (node2childLocMap.find(branchRouteID) == node2childLocMap.end()) break;
														branchRouteID = this->inputSWCTree.listNeuron.at(*node2childLocMap.at(branchRouteID).begin()).n;
													}
													break;
												}
											}
											break;
										}
									}
								}
								else if (branch2childLocMap.find(bChildID) != branch2childLocMap.end())
								{
									for (QList<NeuronSWC>::iterator withinIt = pathAnalyze.begin(); withinIt != pathAnalyze.end() - 1; ++withinIt)
									{
										if (withinIt->n == bChildID)
										{
											branchCheckRangeIDs.push_back(bChildID);
											down = true;

											branchRouteID = bChildID;
											vector<size_t> childLocs = node2childLocMap.at(branchRouteID);
											for (vector<size_t>::iterator branchChildIt = childLocs.begin(); branchChildIt != childLocs.end(); ++branchChildIt)
											{
												if (somaNodeLocMap.find(inputSWCTree.listNeuron.at(*branchChildIt).n) == somaNodeLocMap.end())
												{
													int branchCount = 0;
													branchRouteID = inputSWCTree.listNeuron.at(*branchChildIt).n;
													while (branchCount <= 9)
													{
														++branchCount;
														if (node2childLocMap.find(branchRouteID) == node2childLocMap.end()) break;
														branchRouteID = this->inputSWCTree.listNeuron.at(*node2childLocMap.at(branchRouteID).begin()).n;
													}
													break;
												}
											}
										}
									}
								}

								if (somaNodeLocMap.find(bParentID) != somaNodeLocMap.end())
									bParentID = this->somaPath.at(somaNodeLocMap.at(bParentID)).parent;
								if (somaNode2childLocMap.find(bChildID) != somaNode2childLocMap.end())
									bChildID = this->somaPath.at(*somaNode2childLocMap.at(bChildID).begin()).n;
							}

							if (up)
							{
								vector<float> somaRouteVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())), this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.back())));
								vector<float> branchVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())), this->inputSWCTree.listNeuron.at(nodeLocMap.at(branchRouteID)));
								cout << somaRouteVec.at(0) * branchVec.at(0) + somaRouteVec.at(1) * branchVec.at(1) + somaRouteVec.at(2) * branchVec.at(2) << endl;
								if (somaRouteVec.at(0) * branchVec.at(0) + somaRouteVec.at(1) * branchVec.at(1) + somaRouteVec.at(2) * branchVec.at(2) < 0)
								{
									nodeCutID = this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())).parent;
								}
								cout << "up: " << branchCheckRangeIDs.front() << " " << branchRouteID << endl;
							}
							else if (down)
							{
								vector<float> somaRouteVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())), this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.back())));
								vector<float> branchVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.back())), this->inputSWCTree.listNeuron.at(nodeLocMap.at(branchRouteID)));
								if (somaRouteVec.at(0) * branchVec.at(0) + somaRouteVec.at(1) * branchVec.at(1) + somaRouteVec.at(2) * branchVec.at(2) < 0)
									nodeCutID = this->somaPath.at(*somaNode2childLocMap.at(branchCheckRangeIDs.back()).begin()).n;
								cout << "down: " << branchCheckRangeIDs.back() << " " << branchRouteID << endl;
							}

							this->nodeToBeCutID.push_back(nodeCutID);
							cout << " ID of node to be cut: " << nodeCutID << endl;
						}
					}
					else if ((childrenAddr[i]->branch == true) && (childrenAddr[i]->soma == false)) // pure branch
					{
						QList<NeuronSWC> pathAnalyze;
						head = this->inputSWCTree.listNeuron.at(nodeLocMap.at(segmentHeadID));
						tail = this->inputSWCTree.listNeuron.at(nodeLocMap.at(segmentTailID));
						NeuronStructUtil::upstreamPath(this->inputSWCTree.listNeuron, pathAnalyze, head, tail, nodeLocMap);
						reverse(pathAnalyze.begin(), pathAnalyze.end());
						//backwardPath(pathAnalyze, this->inputSWCTree, tail, head); // extract the path of given head node and tail node
						cout << "Path type: soma to branch, path size: " << pathAnalyze.size() << endl;
						if (pathAnalyze.size() <= 20)
						{
							cout << " -- segment too short to be cut, move to the next crucial child node.." << endl;
							continue;
						}

						double radAngleMax = 0;
						int twistedBranchID = 0;
						deque<vector<float>> upstreamMaxQue;
						deque<vector<float>> downstreamMaxQue;
						for (QList<NeuronSWC>::iterator it = pathAnalyze.begin(); it != pathAnalyze.end(); ++it)
						{
							if (branch2childLocMap.find(it->n) != branch2childLocMap.end())
							{
								vector<NeuronSWC> upstream;
								NeuronStructUtil::upstreamPath(this->somaPath, upstream, *it, somaNodeLocMap);
								reverse(upstream.begin(), upstream.end());

								vector<NeuronSWC> downstream;
								downstream.push_back(*it);
								int childID = this->somaPath.at(*somaNode2childLocMap.at(it->n).begin()).n;
								cout << "| ";
								while (downstream.size() < 10)
								{
									cout << childID << " ";
									downstream.push_back(this->somaPath.at(somaNodeLocMap.at(childID)));
									if (somaNode2childLocMap.find(childID) == somaNode2childLocMap.end()) break;
									childID = this->somaPath.at(*somaNode2childLocMap.at(childID).begin()).n;
								}

								if (upstream.size() <= 3 || downstream.size() <= 3)
								{
									cout << endl;
									continue;
								}

								vector<float> vector1(3);
								vector1[0] = upstream.front().x - upstream.back().x;
								vector1[1] = upstream.front().y - upstream.back().y;
								vector1[2] = upstream.front().z - upstream.back().z;
								vector<float> vector2(3);
								vector2[0] = downstream.back().x - downstream.front().x;
								vector2[1] = downstream.back().y - downstream.front().y;
								vector2[2] = downstream.back().z - downstream.front().z;
								double radAngle = NeuronStructExplorer::getRadAngle(vector1, vector2);
								cout << " -- " << it->n << ": " << radAngle << endl;
								

								if (radAngle > radAngleMax)
								{
									radAngleMax = radAngle;
									twistedBranchID = it->n;
									upstreamMaxQue.clear();
									downstreamMaxQue.clear();
									vector<float> locVec(3);

									for (vector<NeuronSWC>::iterator vecIt = upstream.begin(); vecIt != upstream.end(); ++vecIt)
									{
										locVec[0] = vecIt->x;
										locVec[1] = vecIt->y;
										locVec[2] = vecIt->z;
										upstreamMaxQue.push_back(locVec);
									}
									locVec[0] = it->x;
									locVec[1] = it->y;
									locVec[2] = it->z;
									upstreamMaxQue.push_front(locVec);

									for (vector<NeuronSWC>::iterator vecIt = downstream.begin(); vecIt != downstream.end(); ++vecIt)
									{
										locVec[0] = vecIt->x;
										locVec[1] = vecIt->y;
										locVec[2] = vecIt->z;
										downstreamMaxQue.push_back(locVec);
									}
									locVec[0] = it->x;
									locVec[1] = it->y;
									locVec[2] = it->z;
									downstreamMaxQue.push_front(locVec);
								}
								upstream.clear();
								downstream.clear();
							}
						}

						if (radAngleMax >= 0.5)
						{
							vector<vector<float>> upstreamMax;
							upstreamMax.insert(upstreamMax.begin(), upstreamMaxQue.begin(), upstreamMaxQue.end());
							vector<vector<float>> downstreamMax;
							downstreamMax.insert(downstreamMax.begin(), downstreamMaxQue.begin(), downstreamMaxQue.end());
							double upTurning = NeuronStructExplorer::selfTurningRadAngleSum(upstreamMax);
							double downTurning = NeuronStructExplorer::selfTurningRadAngleSum(downstreamMax);
							//cout << upTurning << " " << downTurning << endl;

							int nodeCutID;
							if (upTurning > downTurning) nodeCutID = originalTree.listNeuron.at(nodeLocMap.at(twistedBranchID)).parent;
							else if (downTurning > upTurning) nodeCutID = this->somaPath.at(*somaNode2childLocMap.at(twistedBranchID).begin()).n;

							this->nodeToBeCutID.push_back(nodeCutID);
							cout << " ID of node to be cut: " << nodeCutID << endl;
						}
						else
						{
							int nodeCutID = 0;

							vector<vector<int>> consecutive1s_vector;
							vector<int> consecutive1s;
							consecutive1s_vector.push_back(consecutive1s);
							for (QList<NeuronSWC>::iterator rIt = pathAnalyze.begin(); rIt != pathAnalyze.end(); ++rIt)
							{
								if (rIt->radius == 1) consecutive1s_vector.back().push_back(rIt->n);
								else
								{
									consecutive1s.clear();
									consecutive1s_vector.push_back(consecutive1s);
								}
							}

							vector<int> consecutive1sMax;
							consecutive1sMax.clear();
							for (vector<vector<int>>::iterator conIt = consecutive1s_vector.begin(); conIt != consecutive1s_vector.end(); ++conIt)
								if (conIt->size() > consecutive1sMax.size()) consecutive1sMax = *conIt;
							if (consecutive1sMax.size() >= 3) nodeCutID = *(consecutive1sMax.begin() + ptrdiff_t(consecutive1sMax.size() / 2));
							else
							{
								nodeCutID = this->pathScissor(pathAnalyze);
								if (nodeCutID == 0)
								{
									ptrdiff_t pathMiddle = ptrdiff_t(pathAnalyze.size() / 2);
									nodeCutID = (pathAnalyze.begin() + pathMiddle)->n;
								}
							}

							deque<int> branchCheckRangeIDs;
							branchCheckRangeIDs.push_back(nodeCutID);
							int bParentID = this->somaPath.at(somaNodeLocMap.at(nodeCutID)).parent;
							int bChildID = this->somaPath.at(*somaNode2childLocMap.at(nodeCutID).begin()).n;
							int rangeCount = 0;
							bool up = false, down = false;
							int branchRouteID = 0;
							while (rangeCount <= 10)
							{
								++rangeCount;

								if (branch2childLocMap.find(bParentID) != branch2childLocMap.end())
								{
									for (QList<NeuronSWC>::iterator withinIt = pathAnalyze.begin(); withinIt != pathAnalyze.end() - 1; ++withinIt)
									{
										if (withinIt->n == bParentID)
										{
											branchCheckRangeIDs.push_front(bParentID);
											up = true;

											branchRouteID = bParentID;
											vector<size_t> childLocs = node2childLocMap.at(branchRouteID);
											for (vector<size_t>::iterator branchChildIt = childLocs.begin(); branchChildIt != childLocs.end(); ++branchChildIt)
											{
												if (somaNodeLocMap.find(inputSWCTree.listNeuron.at(*branchChildIt).n) == somaNodeLocMap.end())
												{
													int branchCount = 0;
													branchRouteID = inputSWCTree.listNeuron.at(*branchChildIt).n;
													while (branchCount <= 9)
													{
														++branchCount;
														if (node2childLocMap.find(branchRouteID) == node2childLocMap.end()) break;
														branchRouteID = this->inputSWCTree.listNeuron.at(*node2childLocMap.at(branchRouteID).begin()).n;
													}
													break;
												}
											}
											break;
										}
									}
								}
								else if (branch2childLocMap.find(bChildID) != branch2childLocMap.end())
								{
									for (QList<NeuronSWC>::iterator withinIt = pathAnalyze.begin(); withinIt != pathAnalyze.end() - 1; ++withinIt)
									{
										if (withinIt->n == bChildID)
										{
											branchCheckRangeIDs.push_back(bChildID);
											down = true;

											branchRouteID = bChildID;
											vector<size_t> childLocs = node2childLocMap.at(branchRouteID);
											for (vector<size_t>::iterator branchChildIt = childLocs.begin(); branchChildIt != childLocs.end(); ++branchChildIt)
											{
												if (somaNodeLocMap.find(inputSWCTree.listNeuron.at(*branchChildIt).n) == somaNodeLocMap.end())
												{
													int branchCount = 0;
													branchRouteID = inputSWCTree.listNeuron.at(*branchChildIt).n;
													while (branchCount <= 9)
													{
														++branchCount;
														if (node2childLocMap.find(branchRouteID) == node2childLocMap.end()) break;
														branchRouteID = this->inputSWCTree.listNeuron.at(*node2childLocMap.at(branchRouteID).begin()).n;
													}
													break;
												}
											}
										}
									}
								}

								if (somaNodeLocMap.find(bParentID) != somaNodeLocMap.end())
									bParentID = this->somaPath.at(somaNodeLocMap.at(bParentID)).parent;
								if (somaNode2childLocMap.find(bChildID) != somaNode2childLocMap.end())
									bChildID = this->somaPath.at(*somaNode2childLocMap.at(bChildID).begin()).n;
							}

							if (up)
							{
								vector<float> somaRouteVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())), this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.back())));
								vector<float> branchVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())), this->inputSWCTree.listNeuron.at(nodeLocMap.at(branchRouteID)));
								cout << somaRouteVec.at(0) * branchVec.at(0) + somaRouteVec.at(1) * branchVec.at(1) + somaRouteVec.at(2) * branchVec.at(2) << endl;
								if (somaRouteVec.at(0) * branchVec.at(0) + somaRouteVec.at(1) * branchVec.at(1) + somaRouteVec.at(2) * branchVec.at(2) < 0)
								{
									nodeCutID = this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())).parent;
								}
								cout << "up: " << branchCheckRangeIDs.front() << " " << branchRouteID << endl;
							}
							else if (down)
							{
								vector<float> somaRouteVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())), this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.back())));
								vector<float> branchVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.back())), this->inputSWCTree.listNeuron.at(nodeLocMap.at(branchRouteID)));
								if (somaRouteVec.at(0) * branchVec.at(0) + somaRouteVec.at(1) * branchVec.at(1) + somaRouteVec.at(2) * branchVec.at(2) < 0)
									nodeCutID = this->somaPath.at(*somaNode2childLocMap.at(branchCheckRangeIDs.back()).begin()).n;
								cout << "down: " << branchCheckRangeIDs.back() << " " << branchRouteID << endl;
							}

							this->nodeToBeCutID.push_back(nodeCutID);
							cout << " ID of node to be cut: " << nodeCutID << endl;
						}
					}
				}
				else if ((*it)->branch == true && (*it)->soma == false)
				{
					if (childrenAddr[i]->soma == true)
					{
						QList<NeuronSWC> pathAnalyze;
						head = this->inputSWCTree.listNeuron.at(nodeLocMap.at(segmentHeadID));
						tail = this->inputSWCTree.listNeuron.at(nodeLocMap.at(segmentTailID));
						NeuronStructUtil::upstreamPath(this->inputSWCTree.listNeuron, pathAnalyze, head, tail, nodeLocMap);
						reverse(pathAnalyze.begin(), pathAnalyze.end());
						//backwardPath(pathAnalyze, this->inputSWCTree, tail, head); // extract the path of given head node and tail node
						cout << "Path type: branch to soma, path size: " << pathAnalyze.size() << endl;
						if (pathAnalyze.size() <= 20)
						{
							cout << " -- segment too short to be cut, move to the next crucial child node.." << endl;
							this->branchSomaMerge = true;
							continue;
						}
						
						if (this->branchSomaMerge)
						{
							for (int counti = 0; counti < 5; ++counti) pathAnalyze.pop_back();
							this->branchSomaMerge = false;
							cout << pathAnalyze.size() << endl;
						}
						
						double radAngleMax = 0;
						int twistedBranchID = 0;
						deque<vector<float>> upstreamMaxQue;
						deque<vector<float>> downstreamMaxQue;
						for (QList<NeuronSWC>::iterator it = pathAnalyze.begin(); it != pathAnalyze.end(); ++it)
						{
							if (branch2childLocMap.find(it->n) != branch2childLocMap.end())
							{
								vector<NeuronSWC> upstream;
								NeuronStructUtil::upstreamPath(this->somaPath, upstream, *it, somaNodeLocMap);
								reverse(upstream.begin(), upstream.end());

								vector<NeuronSWC> downstream;
								downstream.push_back(*it);
								if (somaNode2childLocMap.find(it->n) == somaNode2childLocMap.end())
								{
									upstream.clear();
									downstream.clear();
									continue;
								}
								int childID = this->somaPath.at(*somaNode2childLocMap.at(it->n).begin()).n;
								cout << "| ";
								while (downstream.size() < 10)
								{
									cout << childID << " ";
									downstream.push_back(this->somaPath.at(somaNodeLocMap.at(childID)));
									if (somaNode2childLocMap.find(childID) == somaNode2childLocMap.end()) break;
									childID = this->somaPath.at(*somaNode2childLocMap.at(childID).begin()).n;
								}

								if (upstream.size() <= 3 || downstream.size() <= 3)
								{
									cout << endl;
									continue;
								}

								vector<float> vector1(3);
								vector1[0] = upstream.front().x - upstream.back().x;
								vector1[1] = upstream.front().y - upstream.back().y;
								vector1[2] = upstream.front().z - upstream.back().z;
								vector<float> vector2(3);
								vector2[0] = downstream.back().x - downstream.front().x;
								vector2[1] = downstream.back().y - downstream.front().y;
								vector2[2] = downstream.back().z - downstream.front().z;
								double radAngle = NeuronStructExplorer::getRadAngle(vector1, vector2);
								cout << " -- " << it->n << ": " << radAngle << endl;

								if (radAngle > radAngleMax)
								{
									radAngleMax = radAngle;
									twistedBranchID = it->n;
									upstreamMaxQue.clear();
									downstreamMaxQue.clear();
									vector<float> locVec(3);
					
									for (vector<NeuronSWC>::iterator vecIt = upstream.begin(); vecIt != upstream.end(); ++vecIt)
									{
										locVec[0] = vecIt->x;
										locVec[1] = vecIt->y;
										locVec[2] = vecIt->z;
										upstreamMaxQue.push_back(locVec);
									}
									locVec[0] = it->x;
									locVec[1] = it->y;
									locVec[2] = it->z;
									upstreamMaxQue.push_front(locVec);
								
									for (vector<NeuronSWC>::iterator vecIt = downstream.begin(); vecIt != downstream.end(); ++vecIt)
									{
										locVec[0] = vecIt->x;
										locVec[1] = vecIt->y;
										locVec[2] = vecIt->z;
										downstreamMaxQue.push_back(locVec);
									}
									locVec[0] = it->x;
									locVec[1] = it->y;
									locVec[2] = it->z;
									downstreamMaxQue.push_front(locVec);
								}
								upstream.clear();
								downstream.clear();
							}
						}
						if (upstreamMaxQue.empty() || downstreamMaxQue.empty()) continue;

						if (radAngleMax >= 0.5)
						{
							vector<vector<float>> upstreamMax;
							upstreamMax.insert(upstreamMax.begin(), upstreamMaxQue.begin(), upstreamMaxQue.end());
							vector<vector<float>> downstreamMax;
							downstreamMax.insert(downstreamMax.begin(), downstreamMaxQue.begin(), downstreamMaxQue.end());
							double upTurning = NeuronStructExplorer::selfTurningRadAngleSum(upstreamMax);
							double downTurning = NeuronStructExplorer::selfTurningRadAngleSum(downstreamMax);
							
							int nodeCutID;
							if (upTurning > downTurning) nodeCutID = originalTree.listNeuron.at(nodeLocMap.at(twistedBranchID)).parent;
							else if (downTurning > upTurning) nodeCutID = originalTree.listNeuron.at(*node2childLocMap.at(twistedBranchID).begin()).n;
							
							this->nodeToBeCutID.push_back(nodeCutID);
							cout << " ID of node to be cut: " << nodeCutID << endl;
						}
						else
						{
							int nodeCutID = 0;

							vector<vector<int>> consecutive1s_vector;
							vector<int> consecutive1s;
							consecutive1s_vector.push_back(consecutive1s);
							for (QList<NeuronSWC>::iterator rIt = pathAnalyze.begin(); rIt != pathAnalyze.end(); ++rIt)
							{
								if (rIt->radius == 1) consecutive1s_vector.back().push_back(rIt->n);
								else
								{
									consecutive1s.clear();
									consecutive1s_vector.push_back(consecutive1s);
								}
							}

							vector<int> consecutive1sMax;
							consecutive1sMax.clear();
							for (vector<vector<int>>::iterator conIt = consecutive1s_vector.begin(); conIt != consecutive1s_vector.end(); ++conIt)
								if (conIt->size() > consecutive1sMax.size()) consecutive1sMax = *conIt;
							if (consecutive1sMax.size() >= 3) nodeCutID = *(consecutive1sMax.begin() + ptrdiff_t(consecutive1sMax.size() / 2));
							else
							{
								nodeCutID = this->pathScissor(pathAnalyze);
								if (nodeCutID == 0)
								{
									ptrdiff_t pathMiddle = ptrdiff_t(pathAnalyze.size() / 2);
									nodeCutID = (pathAnalyze.begin() + pathMiddle)->n;
								}
							}
							
							deque<int> branchCheckRangeIDs;
							branchCheckRangeIDs.push_back(nodeCutID);
							int bParentID = this->somaPath.at(somaNodeLocMap.at(nodeCutID)).parent;
							int bChildID = this->somaPath.at(*somaNode2childLocMap.at(nodeCutID).begin()).n;
							int rangeCount = 0;
							bool up = false, down = false;
							int branchRouteID = 0;
							while (rangeCount <= 10)
							{
								++rangeCount;

								if (branch2childLocMap.find(bParentID) != branch2childLocMap.end())
								{
									for (QList<NeuronSWC>::iterator withinIt = pathAnalyze.begin(); withinIt != pathAnalyze.end() - 1; ++withinIt)
									{
										if (withinIt->n == bParentID)
										{
											branchCheckRangeIDs.push_front(bParentID);
											up = true;

											branchRouteID = bParentID;
											vector<size_t> childLocs = node2childLocMap.at(branchRouteID);
											cout << childLocs.size() << endl;
											for (vector<size_t>::iterator branchChildIt = childLocs.begin(); branchChildIt != childLocs.end(); ++branchChildIt)
											{
												if (somaNodeLocMap.find(inputSWCTree.listNeuron.at(*branchChildIt).n) == somaNodeLocMap.end())
												{
													int branchCount = 0;
													branchRouteID = inputSWCTree.listNeuron.at(*branchChildIt).n;
													while (branchCount <= 9)
													{
														++branchCount;
														if (node2childLocMap.find(branchRouteID) == node2childLocMap.end()) break;
														branchRouteID = this->inputSWCTree.listNeuron.at(*node2childLocMap.at(branchRouteID).begin()).n;
													}
													break;
												}
											}
											break;
										}
									}
								}
								else if (branch2childLocMap.find(bChildID) != branch2childLocMap.end())
								{
									for (QList<NeuronSWC>::iterator withinIt = pathAnalyze.begin(); withinIt != pathAnalyze.end() - 1; ++withinIt)
									{
										if (withinIt->n == bChildID)
										{
											branchCheckRangeIDs.push_back(bChildID);
											down = true;

											branchRouteID = bChildID;
											vector<size_t> childLocs = node2childLocMap.at(branchRouteID);
											for (vector<size_t>::iterator branchChildIt = childLocs.begin(); branchChildIt != childLocs.end(); ++branchChildIt)
											{
												if (somaNodeLocMap.find(inputSWCTree.listNeuron.at(*branchChildIt).n) == somaNodeLocMap.end())
												{
													int branchCount = 0;
													branchRouteID = inputSWCTree.listNeuron.at(*branchChildIt).n;
													while (branchCount <= 9)
													{
														++branchCount;
														if (node2childLocMap.find(branchRouteID) == node2childLocMap.end()) break;
														branchRouteID = this->inputSWCTree.listNeuron.at(*node2childLocMap.at(branchRouteID).begin()).n;
													}
													break;
												}
											}
										}
									}	
								}

								if (somaNodeLocMap.find(bParentID) != somaNodeLocMap.end())
									bParentID = this->somaPath.at(somaNodeLocMap.at(bParentID)).parent;
								if (somaNode2childLocMap.find(bChildID) != somaNode2childLocMap.end())
									bChildID = this->somaPath.at(*somaNode2childLocMap.at(bChildID).begin()).n;
							}
							
							if (up)
							{
								vector<float> somaRouteVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())), this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.back())));
								vector<float> branchVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())), this->inputSWCTree.listNeuron.at(nodeLocMap.at(branchRouteID)));
								if (somaRouteVec.at(0) * branchVec.at(0) + somaRouteVec.at(1) * branchVec.at(1) + somaRouteVec.at(2) * branchVec.at(2) < 0)
								{
									nodeCutID = this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())).parent;
								}
								cout << "up: " << branchCheckRangeIDs.front() << " " << branchRouteID << endl;
							}
							else if (down)
							{
								vector<float> somaRouteVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())), this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.back())));
								vector<float> branchVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.back())), this->inputSWCTree.listNeuron.at(nodeLocMap.at(branchRouteID)));
								if (somaRouteVec.at(0) * branchVec.at(0) + somaRouteVec.at(1) * branchVec.at(1) + somaRouteVec.at(2) * branchVec.at(2) < 0)
									nodeCutID = this->somaPath.at(*somaNode2childLocMap.at(branchCheckRangeIDs.back()).begin()).n;
								cout << "down: " << branchCheckRangeIDs.back() << " " << branchRouteID << endl;
							}

							this->nodeToBeCutID.push_back(nodeCutID);
							cout << " ID of node to be cut: " << nodeCutID << endl;
						}
					}
					else if (childrenAddr[i]->soma == false && childrenAddr[i]->branch == true)
					{
						QList<NeuronSWC> pathAnalyze;
						head = this->inputSWCTree.listNeuron.at(nodeLocMap.at(segmentHeadID));
						tail = this->inputSWCTree.listNeuron.at(nodeLocMap.at(segmentTailID));
						NeuronStructUtil::upstreamPath(this->inputSWCTree.listNeuron, pathAnalyze, head, tail, nodeLocMap);
						reverse(pathAnalyze.begin(), pathAnalyze.end());
						//backwardPath(pathAnalyze, this->inputSWCTree, tail, head); // extract the path of given head node and tail node
						cout << "Path type: branch to branch, path size: " << pathAnalyze.size() << endl;
						if (pathAnalyze.size() <= 10)
						{
							cout << " -- segment too short to be cut, move to the next crucial child node.." << endl;
							this->branchSomaMerge = true;
							continue;
						}

						double radAngleMax = 0;
						int twistedBranchID = 0;
						deque<vector<float>> upstreamMaxQue;
						deque<vector<float>> downstreamMaxQue;
						for (QList<NeuronSWC>::iterator it = pathAnalyze.begin(); it != pathAnalyze.end(); ++it)
						{
							if (branch2childLocMap.find(it->n) != branch2childLocMap.end())
							{
								vector<NeuronSWC> upstream;
								NeuronStructUtil::upstreamPath(somaPath, upstream, *it, somaNodeLocMap);
								reverse(upstream.begin(), upstream.end());

								vector<NeuronSWC> downstream;
								downstream.push_back(*it);
								if (somaNode2childLocMap.find(it->n) == somaNode2childLocMap.end())
								{
									upstream.clear();
									downstream.clear();
									continue;
								}
								int childID = this->somaPath.at(*somaNode2childLocMap.at(it->n).begin()).n;
								cout << "| ";
								while (downstream.size() < 10)
								{
									cout << childID << " ";
									downstream.push_back(this->somaPath.at(somaNodeLocMap.at(childID)));
									if (somaNode2childLocMap.find(childID) == somaNode2childLocMap.end()) break;
									childID = this->somaPath.at(*somaNode2childLocMap.at(childID).begin()).n;
								}

								if (upstream.size() <= 3 || downstream.size() <= 3)
								{
									cout << endl;
									continue;
								}

								vector<float> vector1(3);
								vector1[0] = upstream.front().x - upstream.back().x;
								vector1[1] = upstream.front().y - upstream.back().y;
								vector1[2] = upstream.front().z - upstream.back().z;
								vector<float> vector2(3);
								vector2[0] = downstream.back().x - downstream.front().x;
								vector2[1] = downstream.back().y - downstream.front().y;
								vector2[2] = downstream.back().z - downstream.front().z;
								double radAngle = NeuronStructExplorer::getRadAngle(vector1, vector2);
								cout << " -- " << it->n << ": " << radAngle << endl;

								if (radAngle > radAngleMax)
								{
									radAngleMax = radAngle;
									twistedBranchID = it->n;
									upstreamMaxQue.clear();
									downstreamMaxQue.clear();
									vector<float> locVec(3);

									for (vector<NeuronSWC>::iterator vecIt = upstream.begin(); vecIt != upstream.end(); ++vecIt)
									{
										locVec[0] = vecIt->x;
										locVec[1] = vecIt->y;
										locVec[2] = vecIt->z;
										upstreamMaxQue.push_back(locVec);
									}
									locVec[0] = it->x;
									locVec[1] = it->y;
									locVec[2] = it->z;
									upstreamMaxQue.push_front(locVec);

									for (vector<NeuronSWC>::iterator vecIt = downstream.begin(); vecIt != downstream.end(); ++vecIt)
									{
										locVec[0] = vecIt->x;
										locVec[1] = vecIt->y;
										locVec[2] = vecIt->z;
										downstreamMaxQue.push_back(locVec);
									}
									locVec[0] = it->x;
									locVec[1] = it->y;
									locVec[2] = it->z;
									downstreamMaxQue.push_front(locVec);
								}
								upstream.clear();
								downstream.clear();
							}
						}
						if (upstreamMaxQue.empty() || downstreamMaxQue.empty()) continue;

						if (radAngleMax >= 0.5)
						{
							vector<vector<float>> upstreamMax;
							upstreamMax.insert(upstreamMax.begin(), upstreamMaxQue.begin(), upstreamMaxQue.end());
							vector<vector<float>> downstreamMax;
							downstreamMax.insert(downstreamMax.begin(), downstreamMaxQue.begin(), downstreamMaxQue.end());
							double upTurning = NeuronStructExplorer::selfTurningRadAngleSum(upstreamMax);
							double downTurning = NeuronStructExplorer::selfTurningRadAngleSum(downstreamMax);

							int nodeCutID;
							if (upTurning > downTurning) nodeCutID = originalTree.listNeuron.at(nodeLocMap.at(twistedBranchID)).parent;
							else if (downTurning > upTurning) nodeCutID = originalTree.listNeuron.at(*node2childLocMap.at(twistedBranchID).begin()).n;

							this->nodeToBeCutID.push_back(nodeCutID);
							cout << " ID of node to be cut: " << nodeCutID << endl;
						}
						else
						{
							int nodeCutID = 0;

							vector<vector<int>> consecutive1s_vector;
							vector<int> consecutive1s;
							consecutive1s_vector.push_back(consecutive1s);
							for (QList<NeuronSWC>::iterator rIt = pathAnalyze.begin(); rIt != pathAnalyze.end(); ++rIt)
							{
								if (rIt->radius == 1) consecutive1s_vector.back().push_back(rIt->n);
								else
								{
									consecutive1s.clear();
									consecutive1s_vector.push_back(consecutive1s);
								}
							}

							vector<int> consecutive1sMax;
							consecutive1sMax.clear();
							for (vector<vector<int>>::iterator conIt = consecutive1s_vector.begin(); conIt != consecutive1s_vector.end(); ++conIt)
								if (conIt->size() > consecutive1sMax.size()) consecutive1sMax = *conIt;
							if (consecutive1sMax.size() >= 3) nodeCutID = *(consecutive1sMax.begin() + ptrdiff_t(consecutive1sMax.size() / 2));
							else
							{
								nodeCutID = this->pathScissor(pathAnalyze);
								if (nodeCutID == 0)
								{
									ptrdiff_t pathMiddle = ptrdiff_t(pathAnalyze.size() / 2);
									nodeCutID = (pathAnalyze.begin() + pathMiddle)->n;
								}
							}

							deque<int> branchCheckRangeIDs;
							branchCheckRangeIDs.push_back(nodeCutID);
							int bParentID = this->somaPath.at(somaNodeLocMap.at(nodeCutID)).parent;
							int bChildID = this->somaPath.at(*somaNode2childLocMap.at(nodeCutID).begin()).n;
							int rangeCount = 0;
							bool up = false, down = false;
							int branchRouteID = 0;
							while (rangeCount <= 10)
							{
								++rangeCount;

								if (branch2childLocMap.find(bParentID) != branch2childLocMap.end())
								{
									for (QList<NeuronSWC>::iterator withinIt = pathAnalyze.begin(); withinIt != pathAnalyze.end() - 1; ++withinIt)
									{
										if (withinIt->n == bParentID)
										{
											branchCheckRangeIDs.push_front(bParentID);
											up = true;

											branchRouteID = bParentID;
											vector<size_t> childLocs = node2childLocMap.at(branchRouteID);
											cout << childLocs.size() << endl;
											for (vector<size_t>::iterator branchChildIt = childLocs.begin(); branchChildIt != childLocs.end(); ++branchChildIt)
											{
												if (somaNodeLocMap.find(inputSWCTree.listNeuron.at(*branchChildIt).n) == somaNodeLocMap.end())
												{
													int branchCount = 0;
													branchRouteID = inputSWCTree.listNeuron.at(*branchChildIt).n;
													while (branchCount <= 9)
													{
														++branchCount;
														if (node2childLocMap.find(branchRouteID) == node2childLocMap.end()) break;
														branchRouteID = this->inputSWCTree.listNeuron.at(*node2childLocMap.at(branchRouteID).begin()).n;
													}
													break;
												}
											}
											break;
										}
									}
								}
								else if (branch2childLocMap.find(bChildID) != branch2childLocMap.end())
								{
									for (QList<NeuronSWC>::iterator withinIt = pathAnalyze.begin(); withinIt != pathAnalyze.end() - 1; ++withinIt)
									{
										if (withinIt->n == bChildID)
										{
											branchCheckRangeIDs.push_back(bChildID);
											down = true;

											branchRouteID = bChildID;
											vector<size_t> childLocs = node2childLocMap.at(branchRouteID);
											for (vector<size_t>::iterator branchChildIt = childLocs.begin(); branchChildIt != childLocs.end(); ++branchChildIt)
											{
												if (somaNodeLocMap.find(inputSWCTree.listNeuron.at(*branchChildIt).n) == somaNodeLocMap.end())
												{
													int branchCount = 0;
													branchRouteID = inputSWCTree.listNeuron.at(*branchChildIt).n;
													while (branchCount <= 9)
													{
														++branchCount;
														if (node2childLocMap.find(branchRouteID) == node2childLocMap.end()) break;
														branchRouteID = this->inputSWCTree.listNeuron.at(*node2childLocMap.at(branchRouteID).begin()).n;
													}
													break;
												}
											}
										}
									}
								}

								if (somaNodeLocMap.find(bParentID) != somaNodeLocMap.end())
									bParentID = this->somaPath.at(somaNodeLocMap.at(bParentID)).parent;
								if (somaNode2childLocMap.find(bChildID) != somaNode2childLocMap.end())
									bChildID = this->somaPath.at(*somaNode2childLocMap.at(bChildID).begin()).n;
							}

							if (up)
							{
								vector<float> somaRouteVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())), this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.back())));
								vector<float> branchVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())), this->inputSWCTree.listNeuron.at(nodeLocMap.at(branchRouteID)));
								if (somaRouteVec.at(0) * branchVec.at(0) + somaRouteVec.at(1) * branchVec.at(1) + somaRouteVec.at(2) * branchVec.at(2) < 0)
								{
									nodeCutID = this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())).parent;
								}
								cout << "up: " << branchCheckRangeIDs.front() << " " << branchRouteID << endl;
							}
							else if (down)
							{
								vector<float> somaRouteVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.front())), this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.back())));
								vector<float> branchVec = NeuronStructExplorer::getVector_NeuronSWC(this->somaPath.at(somaNodeLocMap.at(branchCheckRangeIDs.back())), this->inputSWCTree.listNeuron.at(nodeLocMap.at(branchRouteID)));
								if (somaRouteVec.at(0) * branchVec.at(0) + somaRouteVec.at(1) * branchVec.at(1) + somaRouteVec.at(2) * branchVec.at(2) < 0)
									nodeCutID = this->somaPath.at(*somaNode2childLocMap.at(branchCheckRangeIDs.back()).begin()).n;
								cout << "down: " << branchCheckRangeIDs.back() << " " << branchRouteID << endl;
							}

							this->nodeToBeCutID.push_back(nodeCutID);
							cout << " ID of node to be cut: " << nodeCutID << endl;
						}
					}
				}
			}

			delete[] childrenAddr;
		}
		cout << "  --  Finished determining branch/path break" << endl;

		curLevelPtr.clear();
		for (vector<somaNode*>::iterator levelIt = nextLevelPtr.begin(); levelIt != nextLevelPtr.end(); ++levelIt)
			curLevelPtr.push_back(*levelIt);
	} while (childrenCount > 0);
	cout << "====================================================" << endl << endl;
}

void neuronSeparator::breakPathMorph()
{
	cout << endl << "===== Identifying nodes to break =====" << endl;
	cout << "crucial node number: " << this->crucialNodes.size() << endl;

	QHash<int, int> IDloc = this->inputSWCTree.hashNeuron;
	vector<somaNode*> curLevelPtr, nextLevelPtr;
	curLevelPtr.push_back(this->somaTreePtr);
	this->brokenSomaPath = this->somaPath;
	map<long int, size_t> somaPathHash = hashScratchMap(this->somaPath);
	map<long int, size_t> branchCutHash;
	QVector< QVector<V3DLONG> > somaPathChildTable = mkChildTableScratch(this->somaPath);

	NeuronSWC head, tail;
	int childrenCount;
	long int segmentHeadID;
	long int segmentTailID;
	do
	{	
		childrenCount = 0;
		int childrenSize = 0;
		nextLevelPtr.clear();
		for (vector<somaNode*>::iterator it=curLevelPtr.begin(); it!=curLevelPtr.end(); ++it) // parent crucial-node loop
		{
			segmentHeadID = (*it)->node.n;
			childrenSize = (*it)->childrenSomas.size();
			childrenCount = childrenCount + childrenSize;
			cout << endl << endl << "children size of " << segmentHeadID << ": " << childrenSize << " -> ";
			
			somaNode** childrenAddr = new somaNode*[childrenSize];
			int childCount = 0;
			for (vector<somaNode*>::iterator listIt=((*it)->childrenSomas).begin(); listIt!=((*it)->childrenSomas).end(); ++listIt)
			{
				childrenAddr[childCount] = *listIt;
				nextLevelPtr.push_back(childrenAddr[childCount]);
				cout << childrenAddr[childCount]->node.n << " ";
				++childCount;
			} cout << endl;	
			
			if (branchCutHash[(*it)->node.n] == true) continue;

			for (size_t i=0; i<childrenSize; ++i) // children crucial-node loop
			{
				segmentTailID = childrenAddr[i]->node.n;
				cout << "--\n Head: " << segmentHeadID << endl << " Tail: " << segmentTailID << endl;
				
				if ((*it)->soma == true) // parent crucial node
				{
					if (childrenAddr[i]->soma == true)
					{
						QList<NeuronSWC> pathAnalyze;
						head = this->inputSWCTree.listNeuron[IDloc[segmentHeadID]];
						tail = this->inputSWCTree.listNeuron[IDloc[segmentTailID]];
						backwardPath(pathAnalyze, this->inputSWCTree, tail, head); // extract the path of given head node and tail node
						//QVector< QVector<V3DLONG> > segmentChildTable = mkChildTableScratch(pathAnalyze);
						cout << " path size: " << pathAnalyze.size() << endl << "--" << endl;

						long int ID = pathScissor(pathAnalyze);
						if (ID > 0)
						{
							this->nodeToBeCutID.push_back(ID);
							size_t location = somaPathHash[ID];
							this->brokenSomaPath[location].parent = -1;
						}
						//size_t location = somaPathHash[ID];
					}
					else if ((childrenAddr[i]->branch == true) && (childrenAddr[i]->soma == false))
					{
						bool allSoma = true;
						for (vector<somaNode*>::iterator somaIt=childrenAddr[i]->childrenSomas.begin(); 
							somaIt!=childrenAddr[i]->childrenSomas.end(); ++somaIt)
						{
							if ((*somaIt)->soma == false)
							{
								allSoma = false;
								break;
							}
						}

						if (allSoma) 
						{
							long int ID = childrenAddr[i]->node.n;
							size_t brLoc = somaPathHash[ID];
							QVector<V3DLONG> nodeAfterBr = somaPathChildTable[brLoc];

							int nodeAfterBrSize = nodeAfterBr.size();
							for (int nodei=0; nodei<nodeAfterBrSize; ++nodei) 
							{
								this->brokenSomaPath[nodeAfterBr[nodei]].parent = -1;
								this->nodeToBeCutID.push_back(brokenSomaPath[nodeAfterBr[nodei]].n);
							}
							branchCutHash[ID] = true;
							cout << " branch to be cut ID: " << ID << endl;
						}
					}
				}
				else if ((*it)->branch==true && (*it)->soma==false)
				{
					if (childrenAddr[i]->soma == true)
					{
						QList<NeuronSWC> pathAnalyze;
						head = this->inputSWCTree.listNeuron[IDloc[segmentHeadID]];
						tail = this->inputSWCTree.listNeuron[IDloc[segmentTailID]];
						backwardPath(pathAnalyze, this->inputSWCTree, tail, head); // extract the path of given head node and tail node
						//QVector< QVector<V3DLONG> > segmentChildTable = mkChildTableScratch(pathAnalyze);
						cout << "path size: " << pathAnalyze.size() << endl;

						long int ID = pathScissor(pathAnalyze);
						if (ID > 0)
						{
							this->nodeToBeCutID.push_back(ID);
							size_t location = somaPathHash[ID];
							this->brokenSomaPath[location].parent = -1;
						}
					}
				}
			}
			
			delete [] childrenAddr;
		}
		cout << "  --  Finished determining branch/path break" << endl;
		
		curLevelPtr.clear();
		for (vector<somaNode*>::iterator levelIt=nextLevelPtr.begin(); levelIt!=nextLevelPtr.end(); ++levelIt)
			curLevelPtr.push_back(*levelIt);
	} while (childrenCount > 0);
	cout << "====================================================" << endl << endl;

	QString name = "brokenSomaPath_test.swc";
	NeuronTree brokenSomaTree;
	brokenSomaTree.listNeuron = this->brokenSomaPath;
	writeSWC_file(name, brokenSomaTree);
}

long int neuronSeparator::pathScissor(QList<NeuronSWC>& segment)
{
	long int nodeCutID;

	int segSize = segment.size();
	NeuronSWC head, tail;
	head = segment[0];
	tail = segment[segSize - 1];

	QVector< QVector<V3DLONG> > segmentChildTable = mkChildTableScratch(segment);
	nodeCheck* nodePathScreen = new nodeCheck[segSize];

	for (int i = 0; i < segSize; ++i)
	{
		double distCheck;
		double distance1 = (head.x - segment[i].x) * (head.x - segment[i].x) + (head.y - segment[i].y) * (head.y - segment[i].y) + (head.z - segment[i].z) * (head.z - segment[i].z);
		double distance2 = (tail.x - segment[i].x) * (tail.x - segment[i].x) + (tail.y - segment[i].y) * (tail.y - segment[i].y) + (tail.z - segment[i].z) * (tail.z - segment[i].z);
		double nodeNorm[3];
		nodeCheck nodeScanned;

		if (distance1 > distance2) 
		{
			nodeScanned.closerToHead = false;
			distCheck = sqrt(distance2);
			nodeNorm[0] = (segment[i].x - tail.x) / distCheck;
			nodeNorm[1] = (segment[i].y - tail.y) / distCheck;
			nodeNorm[2] = (segment[i].z - tail.z) / distCheck;
		}
		else 
		{
			nodeScanned.closerToHead = true;
			distCheck = sqrt(distance1);
			nodeNorm[0] = (segment[i].x - head.x) / distCheck;
			nodeNorm[1] = (segment[i].y - head.y) / distCheck;
			nodeNorm[2] = (segment[i].z - head.z) / distCheck;
		}
						
		nodeScanned.node = segment[i];
		nodeScanned.dist = distCheck;
		nodeScanned.somaBranchNorm[0] = nodeNorm[0];
		nodeScanned.somaBranchNorm[1] = nodeNorm[1];
		nodeScanned.somaBranchNorm[2] = nodeNorm[2];
		nodeScanned.locOnPath = i;
		nodeScanned.childLocOnPath = segmentChildTable[nodeScanned.locOnPath][0];
		nodePathScreen[i] = nodeScanned;
	}

	for (int i = 1; i < segSize-1; ++i)
	{
		//cout << nodePathScreen[i].somaBranchNorm[0] << " " << nodePathScreen[i].somaBranchNorm[1] << " " << nodePathScreen[i].somaBranchNorm[2] << endl;
		
		//nodeCheck child = nodePathScreen[nodePathScreen[i].childLocOnPath];
		if (nodePathScreen[i].closerToHead == true)
		{
			nodePathScreen[i].dirIndex = (nodePathScreen[i + 1].node.x - nodePathScreen[i].node.x) * nodePathScreen[i].somaBranchNorm[0] +
										 (nodePathScreen[i + 1].node.y - nodePathScreen[i].node.y) * nodePathScreen[i].somaBranchNorm[1] +
										 (nodePathScreen[i + 1].node.z - nodePathScreen[i].node.z) * nodePathScreen[i].somaBranchNorm[2];
		}
		else if (nodePathScreen[i].closerToHead == false)
		{
			nodePathScreen[i].dirIndex = (nodePathScreen[i - 1].node.x - nodePathScreen[i].node.x) * nodePathScreen[i].somaBranchNorm[0] +
										 (nodePathScreen[i - 1].node.y - nodePathScreen[i].node.y) * nodePathScreen[i].somaBranchNorm[1] +
										 (nodePathScreen[i - 1].node.z - nodePathScreen[i].node.z) * nodePathScreen[i].somaBranchNorm[2];
		}
		//cout << nodePathScreen[i].dirIndex << " ";
	}

	double minIndex = 0;
	for (int i=1; i<segSize-1; ++i)
	{
		if (nodePathScreen[i].dirIndex < minIndex)
		{
			minIndex = nodePathScreen[i].dirIndex;
			nodeCutID = nodePathScreen[i].node.n;
		}
	}
	if (minIndex == 0) 
	{
		cout << " no node to be cut by geometry scissor." << endl;
		return 0;
	}
	//cout << " ID of node to be cut: " << nodeCutID << endl;
	
	return nodeCutID;
}

void neuronSeparator::downward(QList<NeuronSWC>& tracedSWC, QList<NeuronSWC>& inputList, NeuronSWC& start)
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
	
	if (start.parent == -1) tracedSWC.push_front(start);

	return;
}

QList<NeuronSWC> neuronSeparator::swcTrace(QList<NeuronSWC>& list, long int startID, NeuronSWC& startNode)
{	
	map<int, size_t> node2locMap;
	map<int, vector<size_t>> node2childLocMap;
	NeuronStructUtil::node2loc_node2childLocMap(list, node2locMap, node2childLocMap);

	long parent, id;
	QList<NeuronSWC> traced;
	NeuronSWC root;
	//if (startNode.parent == -1) downward(traced, list, startNode);
	if (startNode.parent == -1) NeuronStructUtil::downstream_subTreeExtract(list, traced, startNode, node2locMap, node2childLocMap);
	else
	{
		long parent = startNode.parent, tempParent;
		NeuronSWC up1LevelNode;
		int level_count = 0;
		do 
		{
			++level_count;
			tempParent = parent;
			for (size_t i=0; i<list.size(); ++i)
			{
				if (list[i].n == parent)
				{
					up1LevelNode = list[i];
					parent = up1LevelNode.parent;
					break;
				}
			}
			if (parent == -1) root = up1LevelNode;
			downward(traced, list, up1LevelNode);
			//if (level_count % 100 == 0) cout << ". ";
		} while (parent != tempParent);
	}
	//cout << endl;
	//traced.push_back(startNode);
	traced.push_back(root);
	list.push_back(startNode);
	//cout << traced.size() << endl;

	QList<NeuronSWC> tracedSorted;
	//SortSWC(traced, tracedSorted, VOID, VOID);
	
	//return tracedSorted;
	return traced;
}
