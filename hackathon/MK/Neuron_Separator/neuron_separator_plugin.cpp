/* Neuron_Separator_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-7-12 : by MK
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_separator_plugin.h"
#include <QFileInfo>
#include <iostream>
#include <ctime>
#include <fstream>
#include <iomanip>

#include "NeuronStructExplorer.h"
#include "NeuronStructUtilities.h"

using namespace std;
Q_EXPORT_PLUGIN2(Neuron_Separator, neuronSeparator);
 
QStringList neuronSeparator::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList neuronSeparator::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void neuronSeparator::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by MK, 2017-7-12"));
	}
}

bool neuronSeparator::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("separate_neuron"))
	{
		vector<char*> infiles, inparas, outfiles;
		if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
		if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
		if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
		
		QString swc_circle = inparas.at(0);
		QString somaRange;
		//if (inparas.at(1)) somaRange = inparas.at(1);
		QString inputSWCfile = infiles.at(0);
		QString inputSomas = infiles.at(1);
		this->outputPath = outfiles[0];

		if (swc_circle == "y") this->circle = true;
		else if (swc_circle == "n")	this->circle = false;

		vector<long int> targetSoma;
		if (inparas.size() > 1)
		{
			for (size_t i = 1; i < inparas.size(); ++i)
			{
				QString inputSomaID = inparas.at(i);
				targetSoma.push_back(inputSomaID.toLong());
			}
		}
		//cout << "Target Soma(s): ";
		//for (vector<long int>::iterator it=targetSoma.begin(); it!=targetSoma.end(); ++it) cout << *it << " ";
		//cout << endl;
		
		this->inputSWCTree = readSWC_file(inputSWCfile);
		QList<NeuronSWC> nodeList = inputSWCTree.listNeuron;
		QList<ImageMarker> somas = readMarker_file(inputSomas);
		this->childsTable = childIndexTable(inputSWCTree);

		map<string, vector<int>> nodeTileMap;
		NeuronStructUtil::nodeTileMapGen(inputSWCTree, nodeTileMap, 30);
		map<int, size_t> nodeLocMap;
		map<int, vector<size_t>> node2childLocMap;
		NeuronStructUtil::node2loc_node2childLocMap(inputSWCTree.listNeuron, nodeLocMap, node2childLocMap);
		
		for (QList<ImageMarker>::iterator it = somas.begin(); it != somas.end(); ++it)  // collecting all soma locations
		{
			string keyLabel = NeuronStructUtil::getNodeTileKey(*it, 30);
			vector<int> tileIDs = nodeTileMap.at(keyLabel);
			float distSqr = 1000;
			int somaID;
			for (vector<int>::iterator idIt = tileIDs.begin(); idIt != tileIDs.end(); ++idIt)
			{
				float thisDistSqr = ((it->x - 1) - inputSWCTree.listNeuron.at(nodeLocMap.at(*idIt)).x) * ((it->x - 1) - inputSWCTree.listNeuron.at(nodeLocMap.at(*idIt)).x) +
									((it->y - 1) - inputSWCTree.listNeuron.at(nodeLocMap.at(*idIt)).y) * ((it->y - 1) - inputSWCTree.listNeuron.at(nodeLocMap.at(*idIt)).y) +
									((it->z - 1) - inputSWCTree.listNeuron.at(nodeLocMap.at(*idIt)).z) * ((it->z - 1) - inputSWCTree.listNeuron.at(nodeLocMap.at(*idIt)).z);
				if (thisDistSqr < distSqr)
				{
					distSqr = thisDistSqr;
					somaID = *idIt;
				}
			}
			this->somaIDs.push_back(somaID);

			/*for (QList<NeuronSWC>::iterator nodeIt = nodeList.begin(); nodeIt != nodeList.end(); ++nodeIt)
			{
                if (abs(long(nodeIt->x - it->x - 1)) < 4 && abs(long(nodeIt->y - it->y - 1)) < 4 && abs(long(nodeIt->z - it->z - 1)) < 4)
				{
					this->somaIDs.push_back(nodeIt->n);
					break;
				}
			}*/
		}
        cout << "number of somas: " << this->somaIDs.size() << endl;

		// ------------ Identify the path of any pair of given soma locations and merge the paths -------------
		long int wishedID, excludedID;
		for (size_t i=0; i<this->somaIDs.size()-1; ++i) 
		{
			wishedID = this->somaIDs[i];
			for (size_t j=i+1; j<this->somaIDs.size(); ++j)
			{
				excludedID = this->somaIDs[j];
				QList<NeuronSWC> path = findPath(childsTable, inputSWCTree, wishedID, excludedID); // identify the path connecting any given 2 nodes
				this->paths.push_back(path);
				NeuronSWC wishedSoma = nodeList.at(inputSWCTree.hashNeuron.value(wishedID));
				NeuronSWC excludedSoma = nodeList.at(inputSWCTree.hashNeuron.value(excludedID));
				//breakPathMorph(this->inputSWCTree, paths[pathCount], this->childsTable, wishedSoma, excludedSoma); 
			}
		}

		getMergedPath(this->somaPath, this->locLabel, this->paths, this->inputSWCTree); // put individual soma path together
		NeuronTree pathTree;
		pathTree.listNeuron = this->somaPath;
		QString pathTreeFileName = this->outputPath + "/somasTree.swc";
		writeSWC_file(pathTreeFileName, pathTree);
		// -------- END of [Identify the path of any pair of given soma locations and merge the paths] --------
		

		// ------------- Build soma tree, determine hierarchy, and cut the path -------------
		buildSomaTree(); // <- This is the function for the purpose. The rest of the codes in this section is for examination.

		int childrenCount = 0;
		vector<somaNode*> curLevelPtr, nextLevelPtr;
		curLevelPtr.push_back(this->somaTreePtr);
		if (targetSoma.size() == 0)
		{
			long int segmentHeadID;
			long int segmentTailID;
			do
			{
				childrenCount = 0;
				int childrenSize = 0;
				nextLevelPtr.clear();
				for (vector<somaNode*>::iterator checkIt=curLevelPtr.begin(); checkIt!=curLevelPtr.end(); ++checkIt) 
				{
					segmentHeadID = (*checkIt)->node.n;
					childrenSize = (*checkIt)->childrenSomas.size();
					childrenCount = childrenCount + childrenSize;

					cout << " - ID: " << (*checkIt)->node.n << endl;
					cout << " - level: " << (*checkIt)->level << endl;
					cout << " - children size: " << childrenSize << " -> ";
					for (vector<somaNode*>::iterator checkChi=(*checkIt)->childrenSomas.begin(); checkChi!=(*checkIt)->childrenSomas.end(); ++checkChi)
						cout << (*checkChi)->node.n << " ";
					cout << endl;
					cout << " - is branch? " << (*checkIt)->branch << endl;
					cout << " - is soma? " << (*checkIt)->soma << endl;
					if ((*checkIt)->soma)
					{
						if ((*checkIt)->headSoma) cout << "   - head" << endl;
						else if ((*checkIt)->middleSoma) cout << "   - middle" << endl;
						else if ((*checkIt)->tailSoma) cout << "   - tail" << endl;
					}
					cout << endl;

					for (size_t j=0; j<childrenSize; ++j) 
					{
						segmentTailID = (*checkIt)->childrenSomas[j]->node.n;
						nextLevelPtr.push_back((*checkIt)->childrenSomas[j]);
					}
				}	
				curLevelPtr = nextLevelPtr;		
			} while (childrenCount > 0);
			cout << "================================" << endl;
		}
		// ------- END of [Build soma tree and determine hierarchy, and cut the path] -------
		

		// ------ Break nodes on input SWC ------
		//breakPathMorph(); // -> Identify the IDs of nodes to be cut.
		breakPathMorph2(inputSWCTree);

		QHash<int, int> inputSWCHash = this->inputSWCTree.hashNeuron;
		this->brokenInputSWC = this->inputSWCTree.listNeuron;
		cout << "IDs of nodes to be cut: ";
		for (vector<long int>::iterator cutIt=this->nodeToBeCutID.begin(); cutIt!=this->nodeToBeCutID.end(); ++cutIt)
		{
			cout << *cutIt << " ";
			this->brokenInputSWC[inputSWCHash[*cutIt]].parent = -1;
		}
		cout << endl << endl;
		NeuronTree brokenWholeTree;
		brokenWholeTree.listNeuron = this->brokenInputSWC;
		QString brokenName = this->outputPath + "/brokenWholeSWC.swc";
		writeSWC_file(brokenName, brokenWholeTree);
		cout << endl;

		QList<NeuronSWC> extracted;
		
		map<int, QList<NeuronSWC>> partitionedNeuronMap;
		vector<int> partitionedIDs;
		for (vector<long int>::iterator extIt = this->somaIDs.begin(); extIt != this->somaIDs.end(); ++extIt)
		{
			long int ID = *extIt;
			//cout << "soma ID: " << ID << " ";
			size_t loc = inputSWCHash[ID];
			NeuronSWC startNode = this->inputSWCTree.listNeuron[loc];
			NeuronStructUtil::wholeSingleTree_extract(this->brokenInputSWC, extracted, startNode);
			extracted = swcTrace(this->brokenInputSWC, ID, startNode);
			//cout << extracted.size() << endl;
			//for (QList<NeuronSWC>::iterator it = extracted.begin(); it != extracted.end(); ++it) it->type = typeCount;
			//NeuronTree extTree;
			//extTree.listNeuron = extracted;
			partitionedNeuronMap.insert({ ID, extracted });
			partitionedIDs.push_back(ID);

			//QString name = this->outputPath + "/extracted_" + QString::number(ID) + ".swc";
			//writeSWC_file(name, extTree);
			extracted.clear();
		}

		cout << "test1" << endl;
		set<vector<int>> markerMergeGroup;
		while (partitionedIDs.size() > 0)
		{
			vector<int> newSet;
			newSet.push_back(*partitionedIDs.begin());
			partitionedIDs.erase(partitionedIDs.begin());
			bool merge = true;
		
			vector<int>::iterator groupIt = newSet.begin();
			while (groupIt != newSet.end())
			{
				NeuronSWC groupedNode = this->inputSWCTree.listNeuron.at(nodeLocMap.at(*groupIt));
				vector<ptrdiff_t> delLocs;
				for (vector<int>::iterator it1 = partitionedIDs.begin(); it1 != partitionedIDs.end(); ++it1)
				{
					NeuronSWC restNode = this->inputSWCTree.listNeuron.at(nodeLocMap.at(*it1));
					if ((restNode.x - groupedNode.x) * (restNode.x - groupedNode.x) + (restNode.y - groupedNode.y) * (restNode.y - groupedNode.y) + (restNode.z - groupedNode.z) * (restNode.z - groupedNode.z) <= 900)
					{
						newSet.push_back(*it1);
						groupIt = newSet.begin();
						delLocs.push_back(it1 - partitionedIDs.begin());
					}
				}
				sort(delLocs.rbegin(), delLocs.rend());
				for (vector<ptrdiff_t>::iterator delIt = delLocs.begin(); delIt != delLocs.end(); ++delIt) partitionedIDs.erase(partitionedIDs.begin() + *delIt);

				++groupIt;
			}
			markerMergeGroup.insert(newSet);
		}

		cout << "test2" << endl;
		int typeCount = 0;
		for (set<vector<int>>::iterator setIt = markerMergeGroup.begin(); setIt != markerMergeGroup.end(); ++setIt)
		{
			cout << "soma ID: " ;
			QList<NeuronSWC> mergedList;
			vector<int> thisGroup = *setIt;
			sort(thisGroup.begin(), thisGroup.end());
			for (vector<int>::iterator markerIt = thisGroup.begin(); markerIt != thisGroup.end(); ++markerIt)
			{	
				cout << *markerIt << " ";
				mergedList.append(partitionedNeuronMap.at(*markerIt));
			}
			cout << endl;
			for (QList<NeuronSWC>::iterator nodeIt = mergedList.begin(); nodeIt != mergedList.end(); ++nodeIt) nodeIt->type = typeCount;
			QString name = this->outputPath + "/extracted_" + QString::number(*thisGroup.begin()) + ".swc";
			NeuronTree newTree;
			newTree.listNeuron = mergedList;
			writeSWC_file(name, newTree);
			++typeCount;
		}
		cout << endl;

		this->crucialNodes.clear();
		
		/*long int somaID = 1;
		NeuronSWC startNode = this->brokenInputSWC[inputSWCHash[somaID]];
		QList<NeuronSWC> traced = swcTrace(this->brokenInputSWC, somaID, startNode);
		NeuronTree extTree;
		extTree.listNeuron = traced;
		QString name = "test_extracted_" + QString::number(somaID) + ".swc";
		writeSWC_file(name, extTree);*/

		/*unordered_map<long int, size_t> brokenSomaHash = hashScratch(this->brokenSomaPath);
		NeuronSWC testNode = this->brokenSomaPath[brokenSomaHash[2161]];
		QList<NeuronSWC> tracedTest = swcTrace(this->brokenSomaPath, 2161, testNode);
		NeuronTree testTree;
		testTree.listNeuron = tracedTest;
		QString name = "testTraced.swc";
		writeSWC_file(name, testTree);*/

		return true;
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		cout << 
			"\n-------------------------------\n" << 
			"Usage: vaa3d_msvc.exe /x neuron_separator /f separate_neuron /i <input swc file name> <soma location marker file> /p [swc_circle] /o <output directory> \n" << 
			"\n - swc_circle: the parameter allows or prohibits circling strcture to happen; can only be 'y' or 'n'. \n-------------------------------" << endl;
	}
	else return false;

	return true;
}

