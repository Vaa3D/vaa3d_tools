#include "BrainAtlasControlPanel.h"
#include "BrainAtlas_define.h"

#include <fstream>
#include <sstream>

BrainAtlasControlPanel::BrainAtlasControlPanel(QWidget* parent, V3DPluginCallback2* callback) : currentUIptr(new Ui::Dialog), thisCallback(callback), QDialog(parent)
{
	currentUIptr->setupUi(this);

	QString brgFilePath;
	if (!DEBUG) brgFilePath = ".\\BrainAtlas\\brgs";
	else brgFilePath = "D:\\AllenVaa3D_2013_Qt486\\v3d_external\\bin\\BrainAtlas\\debug";
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
	//qApp->processEvents();
	for (QStringList::iterator it = brgFileList.begin(); it != brgFileList.end(); ++it)
	{
		float portion = float(it - brgFileList.begin()) / brgFileList.size();
		int percentage = int(portion * 100);

		string regionFileFullName = (brgFilePath + "\\" + *it).toStdString();
		brainRegion thisRegion;
		thisRegion.readBrainRegion_file(regionFileFullName);
		this->regionMap.insert(pair<string, brainRegion>(thisRegion.name, thisRegion));

		iniProgressBarPtr->setValue(percentage);
	}
	iniProgressBarPtr->setValue(100);
	
	this->regionTypeMapGen(iniProgressBarPtr);
	
	if (iniProgressBarPtr->isVisible()) iniProgressBarPtr->close();

	currentUIptr->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Display") << tr("Region Name"));
	for (boost::container::flat_map<string, brainRegion>::iterator it = this->regionMap.begin(); it != this->regionMap.end(); ++it)
	{
		int regionCount = int(it - this->regionMap.begin());

		QTableWidgetItem* displayItemPtr = new QTableWidgetItem;
		displayItemPtr->setCheckState(Qt::Unchecked);
		displayItemPtr->setTextAlignment(Qt::AlignCenter);
		currentUIptr->tableWidget->setItem(regionCount, 0, displayItemPtr);

		QTableWidgetItem* nameItemPtr = new QTableWidgetItem;
		QString regionNameQ = QString::fromStdString(it->first);
		nameItemPtr->setText(regionNameQ);
		nameItemPtr->setTextAlignment(Qt::AlignCenter);
		currentUIptr->tableWidget->setItem(regionCount, 1, nameItemPtr);

		this->region2UIindexMap.insert({ regionNameQ.toStdString(), regionCount });
	}

	this->curWin = this->thisCallback->currentImageWindow();
	this->cur3DViewer = this->thisCallback->find3DViewerByName(this->thisCallback->getImageName(this->curWin));
	this->thisCallback->setBrainAtlasStatus(this->cur3DViewer, true);

	this->connect(currentUIptr->tableWidget, SIGNAL(cellChanged(int, int)), this, SLOT(regionSelected(int, int)));

	this->refresh = false;

	string versionString = to_string(MAINVERSION_NUM) + "." + to_string(SUBVERSION_NUM) + "." + to_string(PATCHVERSION_NUM);
	QString windowTitleQ = "Brain Atlas v" + QString::fromStdString(versionString);
	this->setWindowTitle(windowTitleQ);

	this->show();
}

void BrainAtlasControlPanel::regionTypeMapGen(QProgressDialog* iniProgressBarPtr)
{
	iniProgressBarPtr->setLabelText("Labeling each region color...");
	
	string idValue2regionNamePath;
	if (!DEBUG) idValue2regionNamePath = ".\\BrainAtlas\\idValue2regionName.txt";
	else idValue2regionNamePath = "D:\\AllenVaa3D_2013_Qt486\\v3d_external\\bin\\BrainAtlas\\idValue2regionName.txt";

	ifstream inputFile(idValue2regionNamePath);
	string line;
	string buffer;
	vector<string> lineSplit;
	map<int, string> idValue2nameMap;
	boost::container::flat_set<int> valueSet;
	map<int, int> idValue2assignedMap;
	int assignedI = 0;
	if (inputFile.is_open())
	{
		while (getline(inputFile, line))
		{
			stringstream ss(line);
			while (ss >> buffer) lineSplit.push_back(buffer);

			++assignedI;
			int typeValue = assignedI % 20;
			this->regionTypeMap.insert(pair<string, int>(lineSplit.at(1), typeValue));

			lineSplit.clear();

			float portion = float(assignedI) / 1327;
			int percentage = int(portion * 100);
			iniProgressBarPtr->setValue(percentage);
		}

		inputFile.close();
	}
	
	iniProgressBarPtr->setValue(100);
}

NeuronTree BrainAtlasControlPanel::convertRegion2tree(string regionName)
{
	NeuronTree outputTree;
	int nodeType = this->regionMap.at(regionName).CCFintensity % 16;
	if (nodeType >= 0 && nodeType < 5) nodeType += 16;
	if (!regionName.compare("VISp1")) nodeType += 1;
	for (auto& body : this->regionMap.at(regionName).regionBodies)
	{
		for (auto& slice : body.surfaceCoordSets)
		{
			for (auto& coord : slice.second)
			{
				NeuronSWC node;
				node.x = coord.at(0);
				node.y = coord.at(1);
				node.z = coord.at(2);
				node.type = nodeType;
				node.parent = -1;

				outputTree.listNeuron.append(node);
			}
		}
	}

	return outputTree;
}

void BrainAtlasControlPanel::regionSelected(int row, int col)
{
	string regionName = currentUIptr->tableWidget->item(row, 1)->text().toStdString();
	if (currentUIptr->tableWidget->item(row, 0)->checkState() == Qt::Checked)
	{
		if (this->loadedRegions.find(regionName) != this->loadedRegions.end()) // The region has been loaded.
		{
			if (this->surfaceStatus.find(regionName) != this->surfaceStatus.end()) // Not refreshed.
			{
				if (!this->surfaceStatus.at(regionName))
				{
					this->thisCallback->displaySWC(this->cur3DViewer, this->region2ObjManagerIndexMap.at(regionName));
					this->surfaceStatus[regionName] = true;
				}
			}
			else																   // Has been refreshed.
			{
				int index = this->thisCallback->addRegion_brainAtlas(this->cur3DViewer, this->regionTreeMap.at(regionName));
				this->region2ObjManagerIndexMap.insert({ regionName, index });
				this->surfaceStatus.insert({ regionName, true });
			}
		}
		else
		{
			// The swc of the selected region hasn't been loaded yet.

			this->loadedRegions.insert(regionName);

			NeuronTree selectedRegionTree = this->convertRegion2tree(regionName);
			selectedRegionTree.name = QString::fromStdString(regionName);
			selectedRegionTree.file = "Brg file";
			selectedRegionTree.comment = "brain region";
			selectedRegionTree.color = XYZW(0, 0, 0, 0);
			this->regionTreeMap.insert(pair<string, NeuronTree>(regionName, selectedRegionTree));
			
			int index = this->thisCallback->addRegion_brainAtlas(this->cur3DViewer, selectedRegionTree);
			this->region2ObjManagerIndexMap.insert({ regionName, index });
			this->surfaceStatus.insert({ regionName, true });
		}
	}
	else if (currentUIptr->tableWidget->item(row, 0)->checkState() == Qt::Unchecked)
	{
		if (this->refresh)
		{
			this->surfaceStatus.erase(this->surfaceStatus.find(regionName));
			this->region2ObjManagerIndexMap.erase(this->region2ObjManagerIndexMap.find(regionName));
		}
		else
		{
			if (this->loadedRegions.find(regionName) != this->loadedRegions.end())
			{
				if (this->surfaceStatus.at(regionName))
				{
					this->thisCallback->hideSWC(this->cur3DViewer, this->region2ObjManagerIndexMap.at(regionName));
					this->surfaceStatus[regionName] = false;
				}
			}
		}
	}
}

void BrainAtlasControlPanel::hideRegionFromMouseClick(string regionName)
{
	int row = this->region2UIindexMap.at(regionName);
	if (currentUIptr->tableWidget->item(row, 0)->checkState() == Qt::Checked) currentUIptr->tableWidget->item(row, 0)->setCheckState(Qt::Unchecked);
}

void BrainAtlasControlPanel::scanInvolvedRegions(vector<float> coord)
{
	cout << "Scanning brain regions for (" << coord.at(0) << ", " << coord.at(1) << ", " << coord.at(2) << ")" << endl;

	set<string> regions2show = myScanner.involvedRegionScan(coord, this->regionMap);
	cout << "Determined regions: ";
	for (auto& region : regions2show)
	{
		cout << region << " ";
		int row = this->region2UIindexMap.at(region);
		if (currentUIptr->tableWidget->item(row, 0)->checkState() == Qt::Unchecked) currentUIptr->tableWidget->item(row, 0)->setCheckState(Qt::Checked);
	}
	cout << endl;
}

void BrainAtlasControlPanel::cleanUpRegionRecords()
{
	set<string> regionNames;
	for (auto& region : this->surfaceStatus) regionNames.insert(region.first);

	for (auto& region : regionNames)
	{
		int row = this->region2UIindexMap.at(region);
		if (currentUIptr->tableWidget->item(row, 0)->checkState() == Qt::Checked) currentUIptr->tableWidget->item(row, 0)->setCheckState(Qt::Unchecked);
	}

	this->refresh = false;
}

void BrainAtlasControlPanel::browseFolder()
{
	QObject* signalSender = sender();
	QString objName = signalSender->objectName();

	if (objName == "pushButton_2")
		currentUIptr->lineEdit->setText(QFileDialog::getExistingDirectory(this, tr("Choose folder"), "", QFileDialog::DontUseNativeDialog));
}

void BrainAtlasControlPanel::scanSomaOKclicked()
{
	this->myScanner.somaScanRootPathQ = currentUIptr->lineEdit->text();
	this->myScanner.scanSomas(this->regionMap);
}