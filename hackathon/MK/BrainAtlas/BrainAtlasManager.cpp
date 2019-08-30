#include <sstream>

#include <boost/algorithm/string.hpp>

#include "NeuronStructUtilities.h"

#include "BrainAtlasManager.h"

BrainAtlasManager::BrainAtlasManager(QWidget* parent, V3DPluginCallback2* callback) : regionListUI(new Ui::Dialog), thisCallback(callback), QDialog(parent)
{	
	regionListUI->setupUi(this);

	QString brgFilePath = "./BrainAtlas/brgs";
	QDir inputDir(brgFilePath);
	inputDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	QStringList brgFileList = inputDir.entryList();

	QProgressDialog* iniProgressBarPtr = new QProgressDialog(this);
	iniProgressBarPtr->setWindowTitle("Brain atlas initialization");
	iniProgressBarPtr->setMinimumWidth(400);
	iniProgressBarPtr->setRange(0, 100);
	iniProgressBarPtr->setModal(true);
	
	if (!iniProgressBarPtr->isVisible()) iniProgressBarPtr->show();
	iniProgressBarPtr->setLabelText("Loading each brain region information...");
	qApp->processEvents();
	int testi = 0;
	for (QStringList::iterator it = brgFileList.begin(); it != brgFileList.end(); ++it)
	{
		float portion = float(it - brgFileList.begin()) / brgFileList.size();
		int percentage = int(portion * 100);
		
		string regionFileFullName = (brgFilePath + "/" + *it).toStdString();
		brainRegion thisRegion;
		thisRegion.readBrainRegion_file(regionFileFullName);
		this->regionMap.insert(pair<string, brainRegion>(thisRegion.name, thisRegion));

		iniProgressBarPtr->setValue(percentage);
	}
	iniProgressBarPtr->setValue(100);
	if (iniProgressBarPtr->isVisible()) iniProgressBarPtr->close();

	regionListUI->tableWidget->installEventFilter(this);
	regionListUI->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Display") << tr("Region Name"));
	for (boost::container::flat_map<string, brainRegion>::iterator it = this->regionMap.begin(); it != this->regionMap.end(); ++it)
	{
		int regionCount = int(it - this->regionMap.begin());

		/*QWidget* checkBoxWidget = new QWidget;
		QCheckBox* checkBox = new QCheckBox;
		QHBoxLayout* layoutCheckBox = new QHBoxLayout(checkBoxWidget);
		layoutCheckBox->addWidget(checkBox);
		layoutCheckBox->setAlignment(Qt::AlignCenter);
		layoutCheckBox->setContentsMargins(0, 0, 0, 0);
		checkBox->setChecked(false);
		regionListUI->tableWidget->setCellWidget(regionCount, 0, checkBoxWidget);*/

		QTableWidgetItem* displayItemPtr = new QTableWidgetItem;
		displayItemPtr->setCheckState(Qt::Unchecked);
		displayItemPtr->setTextAlignment(Qt::AlignCenter);
		regionListUI->tableWidget->setItem(regionCount, 0, displayItemPtr);

		QTableWidgetItem* nameItemPtr = new QTableWidgetItem;
		QString regionNameQ = QString::fromStdString(it->first);
		nameItemPtr->setText(regionNameQ);
		nameItemPtr->setTextAlignment(Qt::AlignCenter);
		regionListUI->tableWidget->setItem(regionCount, 1, nameItemPtr);
	}

	this->curWin = this->thisCallback->currentImageWindow();
	this->cur3DViewer = this->thisCallback->find3DViewerByName(this->thisCallback->getImageName(this->curWin));
	
	qDebug() << this->thisCallback->getImageName(this->curWin);

	this->show();
	this->scanCheckBoxes_list();
	
}

void BrainAtlasManager::scanCheckBoxes_list()
{
	for (int i = 0; i < regionListUI->tableWidget->rowCount(); ++i)
	{
		string regionName = regionListUI->tableWidget->item(i, 1)->text().toStdString();
		if (regionListUI->tableWidget->item(i, 0)->checkState() == Qt::Checked)
		{
			if (this->loadedRegions.find(regionName) != this->loadedRegions.end())
			{
				if (!this->surfaceStatus.at(regionName))
				{
					this->thisCallback->displaySWC(this->cur3DViewer, this->surface2indexMap.at(regionName));
					this->surfaceStatus[regionName] = true;
				}
			}
			else
			{
				this->loadedRegions.insert(regionName);
				QString swcPathQ = "./BrainAtlas/brain_regionSurfaces/" + QString::fromStdString(regionName) + ".swc";
				string swcPath = swcPathQ.toStdString();
				const char* swcPathC = swcPath.c_str();
				int index = this->thisCallback->setSWC_noDecompose(this->cur3DViewer, swcPathC);
				this->surface2indexMap.insert({ regionName, index });
				this->surfaceStatus.insert({ regionName, true });
			}
		}
		else if (regionListUI->tableWidget->item(i, 0)->checkState() == Qt::Unchecked)
		{			
			if (this->loadedRegions.find(regionName) != this->loadedRegions.end())
			{
				if (this->surfaceStatus.at(regionName))
				{
					this->thisCallback->hideSWC(this->cur3DViewer, this->surface2indexMap.at(regionName));
					this->surfaceStatus[regionName] = false;
				}
			}
		}
	}
	QTimer::singleShot(500, this, SLOT(scanCheckBoxes_list()));
}

void BrainAtlasManager::neuronInvolvedRegionClicked()
{
	QList<string> loadedRegionsQ;
	for (set<string>::iterator regionIt = this->loadedRegions.begin(); regionIt != this->loadedRegions.end(); ++regionIt)
		loadedRegionsQ.append(*regionIt);
	QList<NeuronTree> neurons = this->thisCallback->loadedNeurons(this->cur3DViewer, loadedRegionsQ);
	
	map<string, set<string>> neuron2regionMap;
	set<string> regionSet;
	for (QList<NeuronTree>::iterator it = neurons.begin(); it != neurons.end(); ++it)
	{
		regionSet.clear();
		neuron2regionMap.insert({ it->name.toStdString(), regionSet });

		vector<int> swcBounds = NeuronStructUtil::getSWCboundary<int>(*it);
		string line;
		string buffer;
		vector<string> lineSplit;
		map<string, vector<vector<int>>> regionBoundsMap;
		ifstream inputFile("./BrainAtlas/regionBoundaries.txt");
		if (inputFile.is_open())
		{
			while (getline(inputFile, line))
			{
				stringstream ss(line);
				while (ss >> buffer) lineSplit.push_back(buffer);

				string name = lineSplit.at(0);
				vector<vector<int>> boundaries;
				int count = 0;
				vector<int> currBounds;
				for (vector<string>::iterator it = lineSplit.begin() + 1; it != lineSplit.end(); ++it)
				{
					currBounds.push_back(stoi(*it));
					++count;
					if (count % 6 == 0)
					{
						boundaries.push_back(currBounds);
						count = 0;
						currBounds.clear();
					}
				}

				//for (vector<int>::iterator it1 = swcBounds.begin(); it1 != swcBounds.end(); ++it1) cout << *it1 << " ";
				//cout << endl;
				//cout << lineSplit.at(0) << ": ";
				for (vector<vector<int>>::iterator it = boundaries.begin(); it != boundaries.end(); ++it)
				{
					//cout << it->at(0) << " " << it->at(1) << " " << it->at(2) << " " << it->at(3) << " " << it->at(4) << " " << it->at(5) << endl;

					if (swcBounds.at(0) < it->at(1) && swcBounds.at(1) > it->at(0) &&
						swcBounds.at(2) < it->at(3) && swcBounds.at(3) > it->at(2) &&
						swcBounds.at(4) < it->at(5) && swcBounds.at(5) > it->at(4))
					{
						regionBoundsMap.insert({ lineSplit.at(0), boundaries });
						break;
					}
					else continue;
				}
				//cout << endl;
				//for (vector<string>::iterator it = lineSplit.begin(); it != lineSplit.end(); ++it) cout << *it << " ";
				//cout << endl;
				lineSplit.clear();
				boundaries.clear();
			}
		}
		inputFile.close();

		for (map<string, vector<vector<int>>>::iterator candidateIt = regionBoundsMap.begin(); candidateIt != regionBoundsMap.end(); ++candidateIt) cout << candidateIt->first << " ";
		cout << endl;
		cout << "candidate regions number: " << regionBoundsMap.size() << endl;

		vector<brainRegion> regionList;
		string inputRegionFolder = "./BrainAtlas/brgs/";
		for (map<string, vector<vector<int>>>::iterator candidateIt = regionBoundsMap.begin(); candidateIt != regionBoundsMap.end(); ++candidateIt)
		{
			string inputRegionFullName = inputRegionFolder + candidateIt->first + ".brg";
			cout << candidateIt->first << endl;
			brainRegion region;
			region.name = candidateIt->first;
			region.readBrainRegion_file(inputRegionFullName);
			for (vector<connectedComponent>::iterator compIt = region.regionBodies.begin(); compIt != region.regionBodies.end(); ++compIt)
			{
				//cout << compIt->xMax << " " << compIt->xMin << " " << compIt->yMax << " " << compIt->yMin << " " << compIt->zMax << " " << compIt->zMin << endl;
				//std::system("pause");

				for (QList<NeuronSWC>::iterator nodeIt = (*it).listNeuron.begin(); nodeIt < (*it).listNeuron.end() - 10; nodeIt = nodeIt + 10)
				{
					int nodeX = int(nodeIt->x);
					int nodeY = int(nodeIt->y);
					int nodeZ = int(nodeIt->z);
					//cout << nodeX << " " << nodeY << " " << nodeZ << endl;
					if (nodeX > compIt->xMin && nodeX < compIt->xMax &&
						nodeY > compIt->yMin && nodeY < compIt->yMax &&
						nodeZ > compIt->zMin && nodeZ < compIt->zMax)
					{
						vector<int> xyVec = { int(nodeIt->x), int(nodeIt->y) };
						vector<int> xzVec = { int(nodeIt->x), int(nodeIt->z) };
						vector<int> yzVec = { int(nodeIt->y), int(nodeIt->z) };

						if (compIt->xyProjection.find(xyVec) != compIt->xyProjection.end() && compIt->xzProjection.find(xzVec) != compIt->xzProjection.end() && compIt->yzProjection.find(yzVec) != compIt->yzProjection.end())
						{
							//cout << " " << nodeX << " " << nodeY << " " << nodeZ << endl;
							//regions.insert(it->first);
							regionList.push_back(region);
							goto REGION_FOUND;
						}
					}
				}
			}

		REGION_FOUND:
			continue;
		}
		cout << "--" << endl;

		QList<NeuronSWC> copiedList = it->listNeuron;
		for (vector<brainRegion>::iterator brainRegionIt = regionList.begin(); brainRegionIt != regionList.end(); ++brainRegionIt)
		{
			cout << brainRegionIt->name << ": ";
			for (vector<connectedComponent>::iterator compIt = brainRegionIt->regionBodies.begin(); compIt != brainRegionIt->regionBodies.end(); ++compIt)
			{
				for (QList<NeuronSWC>::iterator nodeIt = copiedList.begin(); nodeIt < copiedList.end() - 10; nodeIt = nodeIt + 10)
				{
					int nodeZ = int(nodeIt->z);
					if (compIt->surfaceCoordSets.find(nodeZ) != compIt->surfaceCoordSets.end())
					{
						int nodeX = int(nodeIt->x);
						int nodeY = int(nodeIt->y);
						vector<int> sliceBound = getSliceBoundaries(compIt->surfaceCoordSets.at(nodeZ));
						//cout << sliceBound.at(0) << " " << sliceBound.at(1) << " " << sliceBound.at(2) << " " << sliceBound.at(3) << endl;

						int test = 0;
						for (int stepi = nodeY; stepi <= 320; ++stepi)
						{
							vector<int> stepVec = { nodeX, stepi, nodeZ };
							if (compIt->surfaceCoordSets.at(nodeZ).find(stepVec) != compIt->surfaceCoordSets.at(nodeZ).end())
							{
								//cout << "Group " << int(compIt - it->regionBodies.begin()) + 1 << " " << compIt->xMin << " " << compIt->xMax << " " << compIt->yMin << " " << compIt->yMax << endl;
								//cout << "   " << nodeX << " " << nodeY << " " << nodeZ << endl;
								++test;
								break;
							}
						}

						for (int stepi = nodeY; stepi >= 1; --stepi)
						{
							vector<int> stepVec = { nodeX, stepi, nodeZ };
							if (compIt->surfaceCoordSets.at(nodeZ).find(stepVec) != compIt->surfaceCoordSets.at(nodeZ).end())
							{
								//cout << "Group " << int(compIt - it->regionBodies.begin()) + 1 << " " << compIt->xMin << " " << compIt->xMax << " " << compIt->yMin << " " << compIt->yMax << endl;
								//cout << "   " << nodeX << " " << nodeY << " " << nodeZ << endl;
								++test;
								break;
							}
						}

						if (test == 2)
						{
							neuron2regionMap.at(it->name.toStdString()).insert(brainRegionIt->name);
							copiedList.erase(nodeIt);
							goto FINAL_REGION;
						}
					}
				}
			}

		FINAL_REGION:
			cout << endl;
			continue;
		}
	}

	QString disp_text = "";
	for (map<string, set<string>>::iterator it = neuron2regionMap.begin(); it != neuron2regionMap.end(); ++it)
	{
		disp_text += "name: " + QString::fromStdString(it->first) + "\n";
		disp_text += "involved brain regions: ";
		cout << it->first << ": ";
		
		for (set<string>::iterator it1 = it->second.begin(); it1 != it->second.end(); ++it1)
		{
			disp_text += QString::fromStdString(*it1) + " ";
			cout << " " << *it1;
		}
		cout << endl << endl;
		disp_text += "\n\n";
	}

	v3d_msg(disp_text);
}