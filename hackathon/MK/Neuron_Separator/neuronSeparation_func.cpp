#include "neuron_separator_plugin.h"
#include <iostream>
#include <math.h>
#include "basic_surf_objs.h"
#include <vector>
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
#include <algorithm>
#include <vector>

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
		backwardPath(path1, nt, *startIt_excluded, latestAncestor);
		backwardPath(path2, nt, *startIt_wished, latestAncestor);
		//cout << path1.size() << " " << path2.size() << endl;
		for (QList<NeuronSWC>::iterator it=path2.begin(); it!=path2.end()-1; ++it) path1.push_back(*it);
		//cout << path1.size() << endl;

		NeuronTree somaTreePartial;
		somaTreePartial.listNeuron = path1;
		QString name = QString::number(wishedSomaID) + "_" + QString::number(excludedSomaID) + ".swc";
		writeSWC_file(name, somaTreePartial);
		return path1;
	}
	else 
	{
		if (this->forward == 1)
		{
			QList<NeuronSWC> path;
			backwardPath(path, nt, *startIt_excluded, *startIt_wished);
			
			NeuronTree somaTreePartial;
			somaTreePartial.listNeuron = path;
			QString name = QString::number(wishedSomaID) + "_" + QString::number(excludedSomaID) + ".swc";
			writeSWC_file(name, somaTreePartial);
			return path;
		}
		else if (forward == 0)
		{
			QList<NeuronSWC> path;
			backwardPath(path, nt, *startIt_wished, *startIt_excluded);
			
			NeuronTree somaTreePartial;
			somaTreePartial.listNeuron = path;
			QString name = QString::number(wishedSomaID) + "_" + QString::number(excludedSomaID) + ".swc";
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

void neuronSeparator::breakPathMorph(somaNode* headSomaPtr)
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

	for (int i=0; i<segSize; ++i)
	{
		double distCheck;
		double distance1 = (head.x-segment[i].x)*(head.x-segment[i].x) + (head.y-segment[i].y)*(head.y-segment[i].y) + (head.z-segment[i].z)*(head.z-segment[i].z);
		double distance2 = (tail.x-segment[i].x)*(tail.x-segment[i].x) + (tail.y-segment[i].y)*(tail.y-segment[i].y) + (tail.z-segment[i].z)*(tail.z-segment[i].z);
		double nodeNorm[3];
		nodeCheck nodeScanned;

		if (distance1 > distance2) 
		{
			nodeScanned.closerToHead = false;
			distCheck = sqrt(distance2);
			nodeNorm[0] = (segment[i].x-tail.x) / distCheck;
			nodeNorm[1] = (segment[i].y-tail.y) / distCheck;
			nodeNorm[2] = (segment[i].z-tail.z) / distCheck;
		}
		else 
		{
			nodeScanned.closerToHead = true;
			distCheck = sqrt(distance1);
			nodeNorm[0] = (segment[i].x-head.x) / distCheck;
			nodeNorm[1] = (segment[i].y-head.y) / distCheck;
			nodeNorm[2] = (segment[i].z-head.z) / distCheck;
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

	for (int i=1; i<segSize-1; ++i)
	{
		//cout << nodePathScreen[i].somaBranchNorm[0] << " " << nodePathScreen[i].somaBranchNorm[1] << " " << nodePathScreen[i].somaBranchNorm[2] << endl;
		
		//nodeCheck child = nodePathScreen[nodePathScreen[i].childLocOnPath];
		if (nodePathScreen[i].closerToHead == true)
		{
			nodePathScreen[i].dirIndex = (nodePathScreen[i+1].node.x-nodePathScreen[i].node.x) * nodePathScreen[i].somaBranchNorm[0] + 
										 (nodePathScreen[i+1].node.y-nodePathScreen[i].node.y) * nodePathScreen[i].somaBranchNorm[1] +
										 (nodePathScreen[i+1].node.z-nodePathScreen[i].node.z) * nodePathScreen[i].somaBranchNorm[2];
		}
		else if (nodePathScreen[i].closerToHead == false)
		{
			nodePathScreen[i].dirIndex = (nodePathScreen[i-1].node.x-nodePathScreen[i].node.x) * nodePathScreen[i].somaBranchNorm[0] + 
										 (nodePathScreen[i-1].node.y-nodePathScreen[i].node.y) * nodePathScreen[i].somaBranchNorm[1] +
										 (nodePathScreen[i-1].node.z-nodePathScreen[i].node.z) * nodePathScreen[i].somaBranchNorm[2];
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
		cout << " no node to be cut" << endl;
		return 0;
	}
	cout << " ID of node to be cut: " << nodeCutID << endl;
	
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
	
	return;
}

QList<NeuronSWC> neuronSeparator::swcTrace(QList<NeuronSWC>& list, long int startID, NeuronSWC& startNode)
{	
	long parent, id;
	QList<NeuronSWC> traced;
	NeuronSWC root;
	if (startNode.parent == -1) downward(traced, list, startNode);
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
	
	return tracedSorted;
}
