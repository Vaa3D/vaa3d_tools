#include <boost/algorithm/string.hpp>

#include "BrainAtlasManager.h"

BrainAtlasManaer::BrainAtlasManaer(QWidget* parent, V3DPluginCallback2* callback) : regionListUI(new Ui::Dialog), thisCallback(callback), QDialog(parent)
{	
	regionListUI->setupUi(this);

	QString brgFilePath = "../../vaa3d_tools/hackathon/MK/BrainAtlas/brgs";
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

	this->show();
	this->scanCheckBoxes_list();
	
}

void BrainAtlasManaer::scanCheckBoxes_list()
{
	
	for (int i = 0; i < regionListUI->tableWidget->rowCount(); ++i)
	{
		if (regionListUI->tableWidget->item(i, 0)->checkState() == Qt::Checked)
		{
			string regionName = regionListUI->tableWidget->item(i, 1)->text().toStdString();
			if (this->regionTreeMap.find(regionName) != this->regionTreeMap.end()) break;

			QString NeuronTreeName = "../../vaa3d_tools/hackathon/MK/BrainAtlas/brain_regionSurfaces/" + QString::fromStdString(regionName) + ".swc";
			NeuronTree thisRegionTree = readSWC_file(NeuronTreeName);
			this->regionTreeMap.insert(pair<string, NeuronTree>(regionName, thisRegionTree));
			this->thisCallback->setSWC(this->curWin, this->regionTreeMap.at(regionName));

			this->thisCallback->update_NeuronBoundingBox(this->thisCallback->find3DViewerByName(this->thisCallback->getImageName(this->curWin)));
		}

		//cout << regionListUI->tableWidget->item(i, 1)->text().toStdString() << endl;
		//cout << regionListUI->tableWidget->item(i, 1)->text().toStdString() << endl;
	}

	QTimer::singleShot(500, this, SLOT(scanCheckBoxes_list()));
}