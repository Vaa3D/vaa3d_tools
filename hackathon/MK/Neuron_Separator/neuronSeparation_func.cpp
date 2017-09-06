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
	QVector< QVector<V3DLONG> > childs; // indices of the childs of a given parent index
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

	for (size_t i=0; i<somaIDs.size(); ++i)
	{
		long int paID;
		NeuronSWC* nodePtr;
		for (size_t j=0; j<this->somaPath.size(); ++j)
		{
			if (this->somaPath[j].n == this->somaIDs[i])
			{
				nodePtr = &(this->somaPath[j]);
				paID = this->somaPath[j].parent;
				break;
			}
		}
		if (paID == -1) break;

		bool paWithin = false;
		for (size_t j=0; j<this->somaPath.size(); ++j)
		{
			if (this->somaPath[j].n == paID)
			{
				paWithin = true;
				break;
			}
		}

		if (paWithin == false) nodePtr->parent = -1;
	}

	NeuronTree somaTree;
	somaTree.listNeuron = this->somaPath;
	QVector< QVector<V3DLONG> > somaChildsTable;
	vector<long int> somaPaTable;
	somaChildsTable = mkChildTableScratch(this->somaPath);
	somaPaTable = mkPaTableScratch(this->somaPath);
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
				if (somaChildsTable[paLoc].size() > 1) tail2head.push_back(paLoc);
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
				paSomaNodePtr->childrenSomas.push_back(this->crucialNodeHash[*it]);

				handled.push_back(*it);
			}
		}
	}
	/*somaNode* testPtr = crucialNodeHash[244];
	cout << "child address: "; 
	for (vector<somaNode*>::iterator it=testPtr->childrenSomas.begin(); it!=testPtr->childrenSomas.end(); ++it) cout << *it << " ";
	cout << endl;*/

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

	NeuronSWC head, tail;
	int childrenCount;
	long int segmentHeadID;
	long int segmentTailID;
	vector<node_to_be_cut> nodeScreen;
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
			cout << endl << "children size of " << segmentHeadID << ": " << childrenSize << " ->";
			
			somaNode** childrenAddr = new somaNode*[childrenSize];
			int childCount = 0;
			vector<long int> childrenList;
			childrenList.clear();
			for (vector<somaNode*>::iterator listIt=((*it)->childrenSomas).begin(); listIt!=((*it)->childrenSomas).end(); ++listIt)
			{
				//cout << (*listIt)->node.n << " ";
				childrenList.push_back((*listIt)->node.n);
				childrenAddr[childCount] = *listIt;
				nextLevelPtr.push_back(childrenAddr[childCount]);
				cout << childrenAddr[childCount]->node.n << " ";
				++childCount;
			} cout << endl;	

			for (size_t i=0; i<childrenSize; ++i) // children crucial-node loop
			{
				segmentTailID = childrenAddr[i]->node.n;
				cout << "Head: " << segmentHeadID << endl << "Tail: " << segmentTailID << endl;
				
				if ((*it)->soma == true) // parent crucial node
				{
					if (childrenAddr[i]->soma == true)
					{
						QList<NeuronSWC> pathAnalyze;
						head = this->inputSWCTree.listNeuron[IDloc[segmentHeadID]];
						tail = this->inputSWCTree.listNeuron[IDloc[segmentTailID]];
						backwardPath(pathAnalyze, this->inputSWCTree, tail, head); // extract the path of given head node and tail node
						QVector< QVector<V3DLONG> > segmentChildTable = mkChildTableScratch(pathAnalyze);

						/*nodeScreen.clear();
						for (QList<NeuronSWC>::iterator pathIt=pathAnalyze.begin(); pathIt!=pathAnalyze.end(); ++pathIt)
						{
							double distCheck;
							double distance1 = (head.x-pathIt->x)*(head.x-pathIt->x) + (head.y-pathIt->y)*(head.y-pathIt->y) + (head.z-pathIt->z)*(head.z-pathIt->z);
							double distance2 = (tail.x-pathIt->x)*(tail.x-pathIt->x) + (tail.y-pathIt->y)*(tail.y-pathIt->y) + (tail.z-pathIt->z)*(tail.z-pathIt->z);
							double nodeNorm[3];
							node_to_be_cut nodeScanned;

							if (distance1 > distance2) 
							{
								nodeScanned.closerToHead = false;
								distCheck = sqrt(distance2);
								nodeNorm[0] = (pathIt->x-tail.x) / distCheck;
								nodeNorm[1] = (pathIt->y-tail.y) / distCheck;
								nodeNorm[2] = (pathIt->z-tail.z) / distCheck;
							}
							else 
							{
								nodeScanned.closerToHead = true;
								distCheck = sqrt(distance1);
								nodeNorm[0] = (pathIt->x-head.x) / distCheck;
								nodeNorm[1] = (pathIt->y-head.y) / distCheck;
								nodeNorm[2] = (pathIt->z-head.z) / distCheck;
							}
						
							nodeScanned.node = *pathIt;
							nodeScanned.dist = distCheck;
							nodeScanned.somaBranchNorm[0] = nodeNorm[0];
							nodeScanned.somaBranchNorm[1] = nodeNorm[1];
							nodeScanned.somaBranchNorm[2] = nodeNorm[2];
							nodeScanned.locOnPath = int(ptrdiff_t(pathIt - pathAnalyze.begin()));
							nodeScanned.childLocOnPath = segmentChildTable[nodeScanned.locOnPath][0];
							nodeScreen.push_back(nodeScanned);
						}*/
						//pathScissor(pathAnalyze, nodeScreen);
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
							this->nodeToBeCutID.push_back(childrenAddr[i]->node.n);
							cout << "cut ID: " << childrenAddr[i]->node.n << endl << endl;
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
						QVector< QVector<V3DLONG> > segmentChildTable = mkChildTableScratch(pathAnalyze);
						cout << "path size: " << pathAnalyze.size() << endl << "--" << endl;

						/*nodeScreen.clear();
						for (QList<NeuronSWC>::iterator pathIt=pathAnalyze.begin(); pathIt!=pathAnalyze.end(); ++pathIt)
						{
							double distCheck;
							double distance1 = (head.x-pathIt->x)*(head.x-pathIt->x) + (head.y-pathIt->y)*(head.y-pathIt->y) + (head.z-pathIt->z)*(head.z-pathIt->z);
							double distance2 = (tail.x-pathIt->x)*(tail.x-pathIt->x) + (tail.y-pathIt->y)*(tail.y-pathIt->y) + (tail.z-pathIt->z)*(tail.z-pathIt->z);
							double nodeNorm[3];
							node_to_be_cut nodeScanned;

							if (distance1 > distance2) 
							{
								nodeScanned.closerToHead = false;
								distCheck = sqrt(distance2);
								nodeNorm[0] = (pathIt->x-tail.x) / distCheck;
								nodeNorm[1] = (pathIt->y-tail.y) / distCheck;
								nodeNorm[2] = (pathIt->z-tail.z) / distCheck;
							}
							else 
							{
								nodeScanned.closerToHead = true;
								distCheck = sqrt(distance1);
								nodeNorm[0] = (pathIt->x-head.x) / distCheck;
								nodeNorm[1] = (pathIt->y-head.y) / distCheck;
								nodeNorm[2] = (pathIt->z-head.z) / distCheck;
							}
						
							nodeScanned.node = *pathIt;
							nodeScanned.dist = distCheck;
							nodeScanned.somaBranchNorm[0] = nodeNorm[0];
							nodeScanned.somaBranchNorm[1] = nodeNorm[1];
							nodeScanned.somaBranchNorm[2] = nodeNorm[2];
							nodeScanned.locOnPath = int(ptrdiff_t(pathIt - pathAnalyze.begin()));
							nodeScanned.childLocOnPath = segmentChildTable[nodeScanned.locOnPath][0];
							nodeScreen.push_back(nodeScanned);
						}*/
						//pathScissor(pathAnalyze, nodeScreen);
					}
				}
			}
			delete [] childrenAddr;
		}
		

		/*cout << "number of curLevel Ptr: " << curLevelPtr.size() << "\n -> ";
		for (vector<somaNode*>::iterator testIt=curLevelPtr.begin(); testIt!=curLevelPtr.end(); ++testIt)
			cout << (*testIt)->node.n << " ";
		cout << endl;
		
		cout << "number of nextLevel Ptr: " << nextLevelPtr.size() << "\n -> ";
		for (vector<somaNode*>::iterator testIt=nextLevelPtr.begin(); testIt!=nextLevelPtr.end(); ++testIt)
			cout << (*testIt)->node.n << " ";
		cout << endl;*/
		
		curLevelPtr.clear();
		for (vector<somaNode*>::iterator levelIt=nextLevelPtr.begin(); levelIt!=nextLevelPtr.end(); ++levelIt)
			curLevelPtr.push_back(*levelIt);
		
		/*cout << "number of new curLevel Ptr: " << curLevelPtr.size() << "\n -> ";
		for (vector<somaNode*>::iterator testIt=curLevelPtr.begin(); testIt!=curLevelPtr.end(); ++testIt)
			cout << (*testIt)->node.n << " ";
		cout << endl << endl;*/
						
	} while (childrenCount > 0);
}

void neuronSeparator::pathScissor(QList<NeuronSWC> segment, vector<node_to_be_cut> nodeScreen)
{
	/*for (vector<node_to_be_cut>::iterator it=nodeScreen.begin(); it!=nodeScreen.end(); ++it)
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
	*/
}

void neuronSeparator::breakPathMorph(NeuronTree& nt, QList<NeuronSWC>& path, QVector< QVector<V3DLONG> >& childList, NeuronSWC wishedSoma, NeuronSWC excludedSoma)
{
	// ---------------------- Identify branching node and compute its direction index --------------------


	/*
	
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
	// ------------------- END of [Identify branching node and compute its direction index] --------------------- 

	QList<NeuronSWC> newList = nodeList;
	for (vector<branchNodeProfile>::iterator it=branches.end()-2; it!=branches.begin()-1; --it)
	{
		if (it->dirIndex < 0)
		{
			if (it->head == true) continue;
			newList[it->childLoc].parent = -1;
			branchToBeCut.push_back(*it);
			cout << "branch node ID: " << it->branchingNode.n << " cut location: " << it->childLoc << endl;
		}
	}
	for (QList<NeuronSWC>::iterator it=newList.begin(); it!=newList.end(); ++it)
	{
		if (it->parent == -1) this->extractBFS(this->extractedNeuron, newList, *it);
		break;
	}*/
}

void neuronSeparator::downwardBFS(QList<NeuronSWC>& tracedSWC, NeuronTree& nt, NeuronSWC& start, NeuronSWC& end, QVector< QVector<V3DLONG> >& childList)
{
	// This method is deprecated?

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