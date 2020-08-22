#include <iostream>
#include <algorithm>
#include <cmath>

#include <boost\algorithm\string.hpp>

#include <qstringlist.h>
#include <qsettings.h>
#include <qfileinfo.h>
#include <qspinbox.h>
#include <qtimer.h>

#include "FragTraceControlPanel.h"
#include "FragTraceTester.h"

using namespace std;

FragTraceControlPanel::FragTraceControlPanel(QWidget* parent, V3DPluginCallback2* callback) : uiPtr(new Ui::FragmentedTraceUI), thisCallback(callback), QDialog(parent)
{
	// ------- CViewer instance acquisition ------- //
	this->CViewerPortal = thisCallback->castCViewer;
	if (this->CViewerPortal != nullptr)
	{
		this->CViewerPortal->changeFragTraceStatus(true);
		this->CViewerPortal->editingModeInit();
		this->CViewerPortal->sendCastNAUI2PMain(this);
	}
	// -------------------------------------------- //

	// ------- Initialization ------- //
	this->traceManagerPtr = nullptr;
	this->volumeAdjusted = false;
	this->volumeAdjustedCoords = new int[6];
	this->globalCoords = new int[6];
	this->displayingDims = new int[3];
	// ------------------------------ //

	// ------- Set up user interface ------- //
	uiPtr->setupUi(this);
	// ------------------------------------- //

	QSettings callOldSettings("Allen-Neuronanatomy", "Neuron Assembler");

	// ------- Image Format and Work Mode ------- //
	if (callOldSettings.value("terafly") == true)
	{
		uiPtr->checkBox->setChecked(true);
		uiPtr->checkBox_2->setChecked(false);
	}
	else if (callOldSettings.value("stack") == true)
	{
		uiPtr->checkBox->setChecked(false);
		uiPtr->checkBox_2->setChecked(true);
	}

	if (callOldSettings.value("wholeBlock") == true) // axon mode
	{
		uiPtr->radioButton->setChecked(true);
		uiPtr->radioButton_2->setChecked(false);
		uiPtr->radioButton_3->setChecked(false);
		uiPtr->groupBox_6->setEnabled(true);
		uiPtr->groupBox_7->setEnabled(true);
		uiPtr->radioButton_5->setEnabled(false);
	}
	else if (callOldSettings.value("dendrite") == true)
	{
		uiPtr->radioButton->setChecked(false);    // axon mode					  False
		uiPtr->radioButton_2->setChecked(true);   // dendrite mode				  True
		uiPtr->radioButton_3->setChecked(false);  
		uiPtr->groupBox_6->setEnabled(false);     // Histogram-based thresholding False
		uiPtr->groupBox_7->setEnabled(false);     // Extended axon tracing        False
		uiPtr->radioButton_5->setEnabled(true);   // Trace dendrite with marker   True
	}
	else if (callOldSettings.value("bouton") == true)
	{
		uiPtr->radioButton->setChecked(false);
		uiPtr->radioButton_2->setChecked(false);
		uiPtr->radioButton_3->setChecked(true);
	}
	// ------------------------------------------ //

	// ------- Image Enhancement Group Box ------- //
	if (callOldSettings.value("ada") == true)
	{
		uiPtr->groupBox_3->setChecked(true);
		uiPtr->spinBox->setValue(callOldSettings.value("ada_stepsize").toInt());
		uiPtr->spinBox_2->setValue(callOldSettings.value("ada_rate").toInt());
		uiPtr->spinBox_9->setValue(callOldSettings.value("cutoffIntensity").toInt());
	}
	if (callOldSettings.value("gamma") == true) uiPtr->checkBox_6->setChecked(true);
	else uiPtr->checkBox_6->setChecked(false);
	// ------------------------------------------ //

	// ------- Mask Generation Group Box ------- //
	if (callOldSettings.value("dendrite") == true) uiPtr->groupBox_6->setChecked(false);
	else
	{
		if (callOldSettings.value("histThre") == true)
		{
			uiPtr->groupBox_6->setChecked(true);
			uiPtr->doubleSpinBox_3->setValue(callOldSettings.value("histThre_std").toFloat());
		}
	}
	// ---------------------------------------- //

	// ------- Object Filter Group Box ------- //
	if (callOldSettings.value("objFilter") == true)
	{
		if (callOldSettings.value("voxelCount") == true)
		{
			uiPtr->spinBox_14->setValue(callOldSettings.value("voxelCountThre").toInt());
		}
	}
	// --------------------------------------- //

	// ------- Marker / Point Cloud Monitor Group Box ------- //
	this->somaListViewer = new QStandardItemModel(this);
	uiPtr->listView_2->setModel(this->somaListViewer);
	// ------------------------------------------------------ //

	// ------- Object-based MST ------- //
	if (callOldSettings.value("MST") == true)
	{
		uiPtr->groupBox_8->setChecked(true);
		uiPtr->spinBox_5->setValue(callOldSettings.value("minNodeNum").toInt());
	}
	uiPtr->spinBox_7->setValue(callOldSettings.value("axonCluster_range").toInt());
	// ------------------------------- //

	string versionString = to_string(MAINVERSION_NUM) + "." + to_string(SUBVERSION_NUM) + "." + to_string(PATCHVERSION_NUM);
	QString windowTitleQ = "Neuron Assembler v" + QString::fromStdString(versionString);
	this->setWindowTitle(windowTitleQ);  

	this->fragEditorPtr = new FragmentEditor(callback);
	this->fragEditorPtr->sequentialTypeToggled = true;
#ifdef __ACTIVATE_TESTER__
	FragTraceTester::instance(this);
#endif

	this->show();
}

FragTraceControlPanel::~FragTraceControlPanel()
{
	// QObject organizes construction and destruction by themsleves.
	// Therefore -- 
	//   1. Vaa3D's plugin (anythinig inherited from QObject classes) doesn't need to explicitly delete pointers before finishing runtime.
	//   2. Fragtracer plugin is guaranteed of only 1 instance existing (controled in v3dr_glwidget, [Alt + F]).
	//      No need to provide copy constructor and assignment operator.

	if (this->volumeAdjustedCoords != nullptr)
	{
		delete[] this->volumeAdjustedCoords;
		this->volumeAdjustedCoords = nullptr;
	}
	if (this->globalCoords != nullptr)
	{
		delete[] this->globalCoords;
		this->globalCoords = nullptr;
	}
	if (this->displayingDims != nullptr)
	{
		delete[] this->displayingDims;
		this->displayingDims = nullptr;
	}

	if (this->CViewerPortal != nullptr) this->CViewerPortal->segEditing_setCursor("restore");

	if (FragTraceTester::isInstantiated()) FragTraceTester::uninstance();
}

/* =========================== User Interface Buttons =========================== */
void FragTraceControlPanel::imgFmtChecked(bool checked)
{
	QObject* signalSender = sender();
	QString checkBoxName = signalSender->objectName();

	if (checked)
	{
		if (checkBoxName == "checkBox")
		{
			this->updateCViewerPortal();
			uiPtr->checkBox_2->setChecked(false);
			uiPtr->pushButton_2->setEnabled(true);
			this->refreshSomaCoords();
			uiPtr->groupBox_15->setChecked(false);
		}
		else if (checkBoxName == "checkBox_2")
		{
			this->CViewerPortal = nullptr;
			uiPtr->checkBox->setChecked(false);
			uiPtr->pushButton_2->setEnabled(false);
			this->refreshSomaCoords();
			uiPtr->groupBox_15->setChecked(false);
		}
	}
}

void FragTraceControlPanel::nestedChecks(bool checked)
{
	QObject* signalSender = sender();
	QString checkName = signalSender->objectName();

	if (checked)
	{
		if (checkName == "radioButton_2")
		{
			uiPtr->groupBox_6->setEnabled(false);
			uiPtr->groupBox_6->setChecked(false);
			uiPtr->groupBox_7->setChecked(false);
			uiPtr->groupBox_7->setEnabled(false);
			uiPtr->radioButton_5->setEnabled(true);
		}
		else if (checkName == "radioButton")
		{
			uiPtr->groupBox_6->setEnabled(true);
			uiPtr->groupBox_6->setChecked(true);
			uiPtr->groupBox_7->setEnabled(true);
			uiPtr->radioButton_5->setChecked(false);
			uiPtr->radioButton_5->setEnabled(false);
		}
	}
}

void FragTraceControlPanel::markerMonitorOption(bool checked)
{
	QObject* signalSender = sender();
	QString objName = signalSender->objectName();

	if (checked)
	{
		if (objName == "radioButton_5")
		{
			this->refreshSomaCoords();
		}
	}	
}

void FragTraceControlPanel::multiSomaTraceChecked(bool checked) // groupBox_15; [Marker / Point Cloud Monitor]
{
	if (checked)
	{
		if (uiPtr->checkBox->isChecked()) this->refreshSomaCoords();
		else if (uiPtr->checkBox_2->isChecked())
		{
			v3dhandle currImgWindow = this->thisCallback->currentImageWindow();
			QString imageName = this->thisCallback->getImageName(this->thisCallback->currentImageWindow());
			V3dR_MainWindow* currMainWindow = this->thisCallback->find3DViewerByName(imageName);	
			this->thisCallback->set3DViewerMarkerDetectorStatus(true, currMainWindow);
		}
	}
	else
	{
		if (uiPtr->checkBox_2->isChecked())
		{
			v3dhandle currImgWindow = this->thisCallback->currentImageWindow();
			QString imageName = this->thisCallback->getImageName(this->thisCallback->currentImageWindow());
			V3dR_MainWindow* currMainWindow = this->thisCallback->find3DViewerByName(imageName);
			this->thisCallback->set3DViewerMarkerDetectorStatus(false, currMainWindow);
		}
	}
}

void FragTraceControlPanel::refreshSomaCoords()
{	
	this->somaMap.clear();
	this->localSomaMap.clear();
	this->somaDisplayNameMap.clear();
	this->selectedMarkerList.clear();
	this->selectedLocalMarkerList.clear();
	this->localAxonMarkerMap.clear();

	if (uiPtr->checkBox->isChecked())
	{
		if (this->CViewerPortal != nullptr) this->CViewerPortal->refreshSelectedMarkers();
		else return;
	}
	else if (uiPtr->checkBox_2->isChecked())
	{
		v3dhandle currImgWindow = this->thisCallback->currentImageWindow();
		QString imageName = this->thisCallback->getImageName(this->thisCallback->currentImageWindow());
		V3dR_MainWindow* currMainWindow = this->thisCallback->find3DViewerByName(imageName);
		this->thisCallback->refreshSelectedMarkers(currMainWindow);
	}

	this->updateMarkerMonitor();
}

void FragTraceControlPanel::saveSettingsClicked()
{
	QSettings settings("Allen-Neuronanatomy", "Neuron Assembler");
	
	// ------- Image Format and Work Mode ------- //
	if (uiPtr->checkBox->isChecked())
	{
		settings.setValue("terafly", true);
		settings.setValue("stack", false);
	}
	else if (uiPtr->checkBox_2->isChecked())
	{
		settings.setValue("terafly", false);
		settings.setValue("stack", true);
	}

	if (uiPtr->radioButton->isChecked())
	{
		settings.setValue("wholeBlock", true);
		settings.setValue("dendrite", false);
		settings.setValue("bouton", false);
	}
	else if (uiPtr->radioButton_2->isChecked())
	{
		settings.setValue("wholeBlock", false);
		settings.setValue("dendrite", true);
		settings.setValue("bouton", false);
	}
	else if (uiPtr->radioButton_3->isChecked())
	{
		settings.setValue("wholeBlock", false);
		settings.setValue("dendrite", false);
		settings.setValue("bouton", true);
	}
	// ----------------------------------------- //

	// ------- Image Enhancement Group Box ------- //
	if (uiPtr->groupBox_3->isChecked())
	{
		settings.setValue("ada", true);
		settings.setValue("ada_stepsize", uiPtr->spinBox->value());
		settings.setValue("ada_rate", uiPtr->spinBox_2->value());
		settings.setValue("cutoffIntensity", uiPtr->spinBox_9->value());
	}
	else
	{
		settings.setValue("ada", false);
		settings.setValue("ada_stepsize", 0);
		settings.setValue("ada_rate", 0);
		settings.setValue("cutoffIntensity", 0);
	}

	if (uiPtr->checkBox_6->isChecked()) settings.setValue("gamma", true);
	else settings.setValue("gamma", false);
	// ----------------------------------------- //

	// ------- Mask Generation Group Box ------- //
	if (uiPtr->groupBox_6->isChecked())
	{
		settings.setValue("histThre", true);
		settings.setValue("histThre_std", uiPtr->doubleSpinBox_3->value());
	}
	// ----------------------------------------- //

	// ------- Object Filter Group Box ------- //
	if (uiPtr->radioButton_4->isChecked())
	{
		settings.setValue("voxelCount", true);
		settings.setValue("voxelCountThre", uiPtr->spinBox_14->value());
	}
	if (uiPtr->groupBox_13->isChecked()) settings.setValue("objFilter", true);
	else settings.setValue("objFilter", false);
	settings.setValue("axonCluster_range", uiPtr->spinBox_7->value());
	// --------------------------------------- //

	// ------- Object-base MST ------- //
	if (uiPtr->groupBox_8->isChecked())
	{
		settings.setValue("MST", true);
		settings.setValue("minNodeNum", uiPtr->spinBox_5->value());
	}
	else
	{
		settings.setValue("MST", false);
		settings.setValue("minNodeNum", "");
	}
	settings.setValue("MSTtreeName", uiPtr->groupBox_8->title());
	// ------------------------------- //
}

void FragTraceControlPanel::eraseButtonClicked()
{
	if (uiPtr->pushButton_12->isChecked())
	{
		uiPtr->pushButton_13->setChecked(false);
		this->CViewerPortal->setEraserSize(0);
		this->CViewerPortal->segEditing_setCursor("erase");
		this->fragEditorPtr->segMap.clear();
		this->fragEditorPtr->node2segMap.clear();
	}
	else this->CViewerPortal->segEditing_setCursor("restore");
}

void FragTraceControlPanel::connectButtonClicked()
{
	if (uiPtr->pushButton_13->isChecked())
	{
		uiPtr->pushButton_12->setChecked(false);
		this->CViewerPortal->setConnectorSize(0);
		this->CViewerPortal->segEditing_setCursor("connect");
		this->fragEditorPtr->segMap.clear();
		this->fragEditorPtr->node2segMap.clear();
	}
	else this->CViewerPortal->segEditing_setCursor("restore");
}

void FragTraceControlPanel::getType16Locs()
{
	this->type16Locs.clear();
	this->updatedHiddenSegLocs.clear();
	for (vector<V_NeuronSWC>::iterator segIt = this->CViewerPortal->getDisplayingSegs()->begin(); segIt != this->CViewerPortal->getDisplayingSegs()->end(); ++segIt)
	{
		if (segIt->row.begin()->data[1] == 16)
		{
			this->type16Locs.push_back(segIt - this->CViewerPortal->getDisplayingSegs()->begin());
			if (segIt->to_be_deleted) this->updatedHiddenSegLocs.insert(segIt - this->CViewerPortal->getDisplayingSegs()->begin());
		}
	}
}

void FragTraceControlPanel::updateDisplayingVsegProfile()
{
	// Refresh this->dist2VsegLocMap with current displaying status.

	this->totalVsegNum = this->CViewerPortal->getDisplayingSegs()->size();
	this->getType16Locs();

	for (auto& dist : this->dist2VsegLocMap) dist.second.clear();
	//for (auto& dist : this->dist2segUnitMap) dist.second.clear();
	
	this->distInterval = 25;
	while (this->type16Locs.size() > 0)
	{
		vector<ptrdiff_t> delLocs;
		for (vector<ptrdiff_t>::iterator locIt = this->type16Locs.begin(); locIt != this->type16Locs.end(); ++locIt)
		{
			for (vector<V_NeuronSWC>::iterator non16it = this->CViewerPortal->getDisplayingSegs()->begin(); non16it != this->CViewerPortal->getDisplayingSegs()->end(); ++non16it)
			{
				if (non16it->row.begin()->data[1] == 16) continue;
				for (vector<V_NeuronSWC_unit>::iterator unitIt = non16it->row.begin(); unitIt != non16it->row.end(); ++unitIt)
				{
					float headDist, tailDist;
					headDist = sqrt((((this->CViewerPortal->getDisplayingSegs()->begin() + *locIt)->row.end() - 1)->data[2] - unitIt->data[2]) * (((this->CViewerPortal->getDisplayingSegs()->begin() + *locIt)->row.end() - 1)->data[2] - unitIt->data[2]) +
									(((this->CViewerPortal->getDisplayingSegs()->begin() + *locIt)->row.end() - 1)->data[3] - unitIt->data[3]) * (((this->CViewerPortal->getDisplayingSegs()->begin() + *locIt)->row.end() - 1)->data[3] - unitIt->data[3]) +
									(((this->CViewerPortal->getDisplayingSegs()->begin() + *locIt)->row.end() - 1)->data[4] - unitIt->data[4]) * (((this->CViewerPortal->getDisplayingSegs()->begin() + *locIt)->row.end() - 1)->data[4] - unitIt->data[4]));
					tailDist = sqrt(((this->CViewerPortal->getDisplayingSegs()->begin() + *locIt)->row.begin()->data[2] - unitIt->data[2]) * ((this->CViewerPortal->getDisplayingSegs()->begin() + *locIt)->row.begin()->data[2] - unitIt->data[2]) +
									((this->CViewerPortal->getDisplayingSegs()->begin() + *locIt)->row.begin()->data[3] - unitIt->data[3]) * ((this->CViewerPortal->getDisplayingSegs()->begin() + *locIt)->row.begin()->data[3] - unitIt->data[3]) +
									((this->CViewerPortal->getDisplayingSegs()->begin() + *locIt)->row.begin()->data[4] - unitIt->data[4]) * ((this->CViewerPortal->getDisplayingSegs()->begin() + *locIt)->row.begin()->data[4] - unitIt->data[4]));
					if (headDist <= this->distInterval || tailDist <= this->distInterval)
					{
						if (this->dist2VsegLocMap.find(this->distInterval) == this->dist2VsegLocMap.end())
						{
							boost::container::flat_set<ptrdiff_t> VsegSet = { *locIt };
							this->dist2VsegLocMap.insert(pair<int, boost::container::flat_set<ptrdiff_t>>(this->distInterval, VsegSet));

							//segUnit seg(this->CViewerPortal->getDisplayingSegs()->at(*locIt));
							//boost::container::flat_set<segUnit> segUnitSet = { seg };
							//this->dist2segUnitMap.insert(pair<int, boost::container::flat_set<segUnit>>(this->distInterval, segUnitSet));
						}
						else
						{
							this->dist2VsegLocMap[this->distInterval].insert(*locIt);

							//segUnit seg(this->CViewerPortal->getDisplayingSegs()->at(*locIt));
							//this->dist2segUnitMap[this->distInterval].insert(seg);
						}

						delLocs.push_back(int(locIt - type16Locs.begin()));
						goto SEGMENT_RECORDED;
					}
				}
			}

		SEGMENT_RECORDED:
			continue;
		}

		sort(delLocs.rbegin(), delLocs.rend());
		for (auto& delLoc : delLocs) type16Locs.erase(type16Locs.begin() + delLoc);
		delLocs.clear();
		this->distInterval += 20;
	}

	if (this->currentDist == -1) this->type16showingPtr = this->dist2VsegLocMap.end();
	else this->type16showingPtr = this->dist2VsegLocMap.find(this->currentDist);

	for (auto& dist : this->dist2VsegLocMap)
	{
		cout << dist.first << ": ";
		for (auto& segLoc : dist.second) cout << segLoc << " ";
		cout << endl << endl;
	}
}

void FragTraceControlPanel::showHideButtonClicked(bool clicked)
{
	// NOTE: If Esc is hit or a change of basic editing function is made, [My4DImage::tracedNeuron] is refreshed.
	//       Therefore, show/hide data members need to be reinitialized.

	if (FragTraceTester::isInstantiated())
	{
		FragTraceTester::getInstance()->printOutHiddenType16Info();
	}

	if (this->CViewerPortal->getDisplayingSegs()->size() != this->totalVsegNum) this->updateDisplayingVsegProfile();

	if (clicked) // ------------------------ SHOW
	{
		if (!this->updatedHiddenSegLocs.empty())
		{
			for (auto& hiddenSegLoc : this->updatedHiddenSegLocs)
				(*this->CViewerPortal->getDisplayingSegs())[hiddenSegLoc].to_be_deleted = false;
			this->CViewerPortal->updateDisplayingSegs();
		}
	}
	else // -------------------------------- HIDE
	{
		for (vector<V_NeuronSWC>::iterator displayIt = (*this->CViewerPortal->getDisplayingSegs()).begin();
			displayIt != (*this->CViewerPortal->getDisplayingSegs()).end(); ++displayIt)
		{
			if (displayIt->row.begin()->data[1] == 16 && displayIt->to_be_deleted)
			{
				this->permanentDelSegLocs.insert(displayIt - (*this->CViewerPortal->getDisplayingSegs()).begin());
				if (this->updatedHiddenSegLocs.find(displayIt - (*this->CViewerPortal->getDisplayingSegs()).begin()) != this->updatedHiddenSegLocs.end())
					this->updatedHiddenSegLocs.erase(this->updatedHiddenSegLocs.find(displayIt - (*this->CViewerPortal->getDisplayingSegs()).begin()));
			}
			else if (displayIt->row.begin()->data[1] == 16 && !displayIt->to_be_deleted)
			{
				displayIt->to_be_deleted = true;
				this->updatedHiddenSegLocs.insert(displayIt - (*this->CViewerPortal->getDisplayingSegs()).begin());
			}
			else if (displayIt->row.begin()->data[1] != 16 && this->updatedHiddenSegLocs.find(displayIt - (*this->CViewerPortal->getDisplayingSegs()).begin()) != this->updatedHiddenSegLocs.end())
				this->updatedHiddenSegLocs.erase(this->updatedHiddenSegLocs.find(displayIt - (*this->CViewerPortal->getDisplayingSegs()).begin()));
		}

		this->CViewerPortal->updateDisplayingSegs();
	}
}

void FragTraceControlPanel::moveType16showPtr(bool increment)
{
	if (this->CViewerPortal->getDisplayingSegs()->size() != this->totalVsegNum) this->updateDisplayingVsegProfile();

	if (increment)
	{
		while (this->type16showingPtr->second.empty() && this->type16showingPtr != this->dist2VsegLocMap.end())
			++(this->type16showingPtr);

		if (this->type16showingPtr != this->dist2VsegLocMap.end())
		{
			cout << "Segment location to be marked SHOWN (distance = " << this->type16showingPtr->first << ") :";
			for (auto& loc:this->type16showingPtr->second)
			{
				cout << loc << " ";
				if (this->incrementHiddenLocs.find(loc) != this->incrementHiddenLocs.end())
					this->incrementHiddenLocs.erase(this->incrementHiddenLocs.find(loc));
				(this->CViewerPortal->getDisplayingSegs()->begin() + loc)->to_be_deleted = false;
			}		
			cout << endl << endl; 

			++(this->type16showingPtr);
			if (this->type16showingPtr == this->dist2VsegLocMap.end()) this->currentDist = -1;
			else this->currentDist = this->type16showingPtr->first;
		}
	}
	else
	{
		if (this->type16showingPtr != this->dist2VsegLocMap.begin())
		{
			do --(this->type16showingPtr);
			while (this->type16showingPtr->second.empty());
			this->currentDist = this->type16showingPtr->first;

			cout << "Segment location to be marked HIDDEN (distance = " << this->type16showingPtr->first << ") :";
			vector<ptrdiff_t> delLocs;
			for (auto& loc : this->type16showingPtr->second)
			{
				if ((this->CViewerPortal->getDisplayingSegs()->begin() + loc)->to_be_deleted || (this->CViewerPortal->getDisplayingSegs()->begin() + loc)->row.begin()->data[1] != 16)
				{
					delLocs.push_back(this->type16showingPtr->second.find(loc) - this->type16showingPtr->second.begin());
					continue;
				}

				cout << loc << " ";
				this->incrementHiddenLocs.insert(loc);
				(this->CViewerPortal->getDisplayingSegs()->begin() + loc)->to_be_deleted = true;
			}
			cout << endl << endl;

			sort(delLocs.rbegin(), delLocs.rend());
			for (auto& delLoc : delLocs) this->type16showingPtr->second.erase(this->type16showingPtr->second.begin() + delLoc);
		}
	}

	this->CViewerPortal->updateDisplayingSegs();
}

void FragTraceControlPanel::sequentialTypeChangingToggled(bool toggle)
{
	if (toggle) this->fragEditorPtr->sequentialTypeToggled = true;
	else if (!toggle) this->fragEditorPtr->sequentialTypeToggled = false;
}
/* ====================== END of [User Interface Buttons] ======================= */


/* ============================== TRACING INITIALIZING FUNCTION =============================== */
void FragTraceControlPanel::traceButtonClicked()
{
	QSettings currSettings("Allen-Neuronanatomy", "Neuron Assembler");
	uiPtr->pushButton_4->setChecked(false);
	
	cout << "Fragment tracing procedure initiated." << endl;
	if (uiPtr->radioButton->isChecked() && !uiPtr->radioButton_2->isChecked()) // AXON TRACING
	{
		cout << " --> axon tracing, acquiring image information.." << endl;

		if (uiPtr->checkBox->isChecked()) // terafly format
		{				
			this->teraflyTracePrep(axon);	   	  // terafly image block preparation
			this->traceManagerPtr->continuousAxon = false;

			// ------------------- collect parameters ------------------- //
			this->pa_imgEnhancement();                         // image enhancement				
			this->pa_maskGeneration();                         // mask generation
			this->pa_objFilter();                              // object filter
			this->pa_objBasedMST();                            // object-based MST node connecting
			this->pa_axonContinuous();
			// ---------------------------------------------------------- //
		}
	}
	else if (!uiPtr->radioButton->isChecked() && uiPtr->radioButton_2->isChecked()) // DENDRITE TRACING
	{
		cout << " --> dendritic tree tracing, acquiring image information.." << endl;

		if (uiPtr->checkBox->isChecked()) // terafly format
		{
			this->teraflyTracePrep(dendriticTree); // terafly image block preparation
			this->traceManagerPtr->continuousAxon = false;

			// ------------------- collect parameters ------------------- //
			this->pa_imgEnhancement();                          // image enhancement
			this->pa_objFilter();                               // object filter
			this->pa_objBasedMST();                             // object-based MST node connecting 
			// ---------------------------------------------------------- //
		}
	}
	this->fillUpParamsForm();
	
	this->connect(this->traceManagerPtr, SIGNAL(emitTracedTree(NeuronTree)), this, SLOT(catchTracedTree(NeuronTree)));
	this->connect(this->traceManagerPtr, SIGNAL(getExistingFinalTree(NeuronTree&)), this, SLOT(sendExistingNeuronTree(NeuronTree&)));
	//QTimer::singleShot(0, this->traceManagerPtr, SLOT(imgProcPipe_wholeBlock())); // ==> Qt's [singleShot] is still enforced on the thread of event loop.
	
	if (this->somaListViewer->rowCount() == 0)
	{
		this->traceManagerPtr->selectedSomaMap.clear();
		this->traceManagerPtr->selectedLocalSomaMap.clear();
		this->traceManagerPtr->localAxonMarkerMap.clear();
	}

	/***************** TRACING PROCESS STARTS HERE *****************/
	if (!this->traceManagerPtr->imgProcPipe_wholeBlock())
	{
		// Newly traced tree will be stored in this->tracedTree.
		v3d_msg(QString("The process has been terminated."));
		return;
	}
	this->thisCallback->setSWCTeraFly(this->tracedTree);
	
	// -- Acquire all type-16 Vsegs that are to be hidden, and record their locations on [My4DImage::tracedNeuron.seg] -- //
	this->tracedType16Vsegs = this->traceManagerPtr->getCurrentVolumeV_NeuronSWCs();
	if (!this->tracedType16Vsegs.empty())
	{
		for (vector<V_NeuronSWC>::iterator segIt = this->tracedType16Vsegs.begin(); segIt != this->tracedType16Vsegs.end(); ++segIt)
		{
			this->CViewerPortal->getDisplayingSegs()->push_back(*segIt);
			this->updatedHiddenSegLocs.insert(this->CViewerPortal->getDisplayingSegs()->size() - 1);
		}
		this->updateDisplayingVsegProfile();
		this->type16showingPtr = this->dist2VsegLocMap.end();
		this->totalVsegNum = this->CViewerPortal->getDisplayingSegs()->size();
	}
	// ------------------------------------------------------------------------------------------------------------------ //

	// ------- SegEnd Cluster Debug ------- //
	if (FragTraceTester::isInstantiated())
	{
		for (auto& clusterMap : FragTraceTester::getInstance()->clusterSegEndNodeMaps)
		{
			for (const auto& cluster : clusterMap)
				FragTraceTester::getInstance()->pushMarkers(cluster.second.first, cluster.second.second);
		}
		FragTraceTester::getInstance()->clusterSegEndNodeMaps.clear();
	}
	// ------------------------------------ //
	/***************************************************************/

	this->traceManagerPtr->partialVolumeLowerBoundaries = { 0, 0, 0 };

	this->disconnect(this->traceManagerPtr, SIGNAL(emitTracedTree(NeuronTree)), this, SLOT(catchTracedTree(NeuronTree)));
	this->disconnect(this->traceManagerPtr, SIGNAL(getExistingFinalTree(NeuronTree&)), this, SLOT(sendExistingNeuronTree(NeuronTree&)));
}
/* ============================================================================================ */


/* =========================== TRACING VOLUME PREPARATION =========================== */
void FragTraceControlPanel::teraflyTracePrep(workMode mode)
{
	// This method prepares the image data for FragTraceManager to trace.
	// The FRAGTRACEMANAGER INSTANCE IS CREATED HERE.

	this->volumeAdjusted = this->CViewerPortal->getPartialVolumeCoords(this->globalCoords, this->volumeAdjustedCoords, this->displayingDims);
	if (FragTraceTester::isInstantiated()) FragTraceTester::getInstance()->printOutVolInfo();

	const Image4DSimple* currBlockImg4DSimplePtr = thisCallback->getImageTeraFly();
	Image4DSimple* croppedImg4DSimplePtr = new Image4DSimple;

	if (this->volumeAdjusted)
	{
		unsigned char* currBlock1Dptr = new unsigned char[currBlockImg4DSimplePtr->getXDim() * currBlockImg4DSimplePtr->getYDim() * currBlockImg4DSimplePtr->getZDim()];
		int totalbyte = currBlockImg4DSimplePtr->getTotalBytes();
		memcpy(currBlock1Dptr, currBlockImg4DSimplePtr->getRawData(), totalbyte);

		int originalDims[3] = { this->displayingDims[0], this->displayingDims[1], this->displayingDims[2] };
		int croppedDims[3];
		croppedDims[0] = this->volumeAdjustedCoords[1] - this->volumeAdjustedCoords[0] + 1;
		croppedDims[1] = this->volumeAdjustedCoords[3] - this->volumeAdjustedCoords[2] + 1;
		croppedDims[2] = this->volumeAdjustedCoords[5] - this->volumeAdjustedCoords[4] + 1;
		unsigned char* croppedBlock1Dptr = new unsigned char[croppedDims[0] * croppedDims[1] * croppedDims[2]];
		ImgProcessor::cropImg(currBlock1Dptr, croppedBlock1Dptr, this->volumeAdjustedCoords[0], this->volumeAdjustedCoords[1],
																 this->volumeAdjustedCoords[2], this->volumeAdjustedCoords[3],
																 this->volumeAdjustedCoords[4], this->volumeAdjustedCoords[5], originalDims);
		croppedImg4DSimplePtr->setData(croppedBlock1Dptr, croppedDims[0], croppedDims[1], croppedDims[2], 1, V3D_UINT8);	

		// ------- Debug ------- //
		/*if (FragTraceTester::isInstantiated()) 
		{
			string saveName = "C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\testCase4\\test1.tif";
			FragTraceTester::getInstance()->imgVolumeCheck(croppedImg4DSimplePtr, saveName);
		}*/
		// --------------------- //

		if (this->traceManagerPtr == nullptr)
		{
			this->traceManagerPtr = new FragTraceManager(croppedImg4DSimplePtr, mode);
			if (FragTraceTester::isInstantiated())
				FragTraceTester::getInstance()->sharedTraceManagerPtr = make_shared<FragTraceManager*>(this->traceManagerPtr);
			this->traceManagerPtr->partialVolumeLowerBoundaries[0] = this->volumeAdjustedCoords[0] - 1;
			this->traceManagerPtr->partialVolumeLowerBoundaries[1] = this->volumeAdjustedCoords[2] - 1;
			this->traceManagerPtr->partialVolumeLowerBoundaries[2] = this->volumeAdjustedCoords[4] - 1;
		}
		else
		{
			this->traceManagerPtr->reinit(croppedImg4DSimplePtr, mode);
			this->traceManagerPtr->partialVolumeLowerBoundaries[0] = this->volumeAdjustedCoords[0] - 1;
			this->traceManagerPtr->partialVolumeLowerBoundaries[1] = this->volumeAdjustedCoords[2] - 1;
			this->traceManagerPtr->partialVolumeLowerBoundaries[2] = this->volumeAdjustedCoords[4] - 1;
		}
		delete[] currBlock1Dptr;
		delete[] croppedBlock1Dptr;		

		this->traceManagerPtr->partialVolumeTracing = true;
	}
	else
	{
		if (this->traceManagerPtr == nullptr)
		{
			this->traceManagerPtr = new FragTraceManager(currBlockImg4DSimplePtr, mode);
			if (FragTraceTester::isInstantiated())
				FragTraceTester::getInstance()->sharedTraceManagerPtr = make_shared<FragTraceManager*>(this->traceManagerPtr);
		}
		else this->traceManagerPtr->reinit(currBlockImg4DSimplePtr, mode);
	}

	this->sendImgParams();
}

void FragTraceControlPanel::sendImgParams()
{
	this->traceManagerPtr->volumeAdjustedBounds = this->volumeAdjustedCoords;
	if (!this->volumeAdjusted)
	{
		this->traceManagerPtr->volumeAdjustedBounds[0] = 1;
		this->traceManagerPtr->volumeAdjustedBounds[1] = 256;
		this->traceManagerPtr->volumeAdjustedBounds[2] = 1;
		this->traceManagerPtr->volumeAdjustedBounds[3] = 256;
		this->traceManagerPtr->volumeAdjustedBounds[4] = 1;
		this->traceManagerPtr->volumeAdjustedBounds[5] = 256;
	}

	float imgDims[3];
	imgDims[0] = this->thisCallback->getImageTeraFly()->getXDim();
	imgDims[1] = this->thisCallback->getImageTeraFly()->getYDim();
	imgDims[2] = this->thisCallback->getImageTeraFly()->getZDim();
	if (FragTraceTester::isInstantiated()) FragTraceTester::getInstance()->imgDims = imgDims;

	float imgRes[3];
	imgRes[0] = this->thisCallback->getImageTeraFly()->getRezX();
	imgRes[1] = this->thisCallback->getImageTeraFly()->getRezY();
	imgRes[2] = this->thisCallback->getImageTeraFly()->getRezZ();
	if (FragTraceTester::isInstantiated()) FragTraceTester::getInstance()->imgRes = imgRes;

	float factor = pow(2, abs(this->CViewerPortal->getTeraflyTotalResLevel() - 1 - this->CViewerPortal->getTeraflyResLevel()));
	cout << " -- Image volume scaling information: " << endl;
	cout << endl << "  -- scaling factor = " << factor << endl;
	cout << "  -- current resolutionl level = " << this->CViewerPortal->getTeraflyResLevel() + 1 << endl;
	cout << "  -- total res levels: " << this->CViewerPortal->getTeraflyTotalResLevel() << endl << endl;

	float imgOri[3];
	string currWinTitle = this->CViewerPortal->getCviewerWinTitle();
	vector<string> splitWhole;
	boost::split(splitWhole, currWinTitle, boost::is_any_of("["));
	vector<string> xSplit;
	boost::split(xSplit, splitWhole[1], boost::is_any_of(","));
	imgOri[0] = stof(xSplit[0]) * factor - 1;
	vector<string> ySplit;
	boost::split(ySplit, splitWhole[2], boost::is_any_of(","));
	imgOri[1] = stof(ySplit[0]) * factor - 1;
	vector<string> zSplit;
	boost::split(zSplit, splitWhole[3], boost::is_any_of(","));
	imgOri[2] = stof(zSplit[0]) * factor - 1;
	if (FragTraceTester::isInstantiated()) FragTraceTester::getInstance()->imgOrigin = imgOri;

	if (FragTraceTester::isInstantiated()) FragTraceTester::getInstance()->printOutImgInfo();

	this->traceManagerPtr->scalingFactor = factor;
	this->traceManagerPtr->imgOrigin[0] = imgOri[0];
	this->traceManagerPtr->imgOrigin[1] = imgOri[1];
	this->traceManagerPtr->imgOrigin[2] = imgOri[2];
}
/* ================================================================================== */


/* ========================= Parameter Collecting Functions ========================= */
void FragTraceControlPanel::pa_imgEnhancement()
{
	if (uiPtr->groupBox_3->isChecked())
	{
		this->traceManagerPtr->ada = true;
		this->traceManagerPtr->adaImgName = uiPtr->groupBox_3->title().toStdString();
		this->traceManagerPtr->imgEnhanceSeq.push_back(this->traceManagerPtr->adaImgName);
		this->traceManagerPtr->simpleAdaStepsize = uiPtr->spinBox->value();
		this->traceManagerPtr->simpleAdaRate = uiPtr->spinBox_2->value();
		this->traceManagerPtr->cutoffIntensity = uiPtr->spinBox_9->value();
	}
	else this->traceManagerPtr->ada = false;

	if (uiPtr->checkBox_6->isChecked()) this->traceManagerPtr->gammaCorrection = true;
	else this->traceManagerPtr->gammaCorrection = false;
}

void FragTraceControlPanel::pa_maskGeneration()
{
	if (uiPtr->groupBox_6->isChecked())
	{
		this->traceManagerPtr->histThre = true;
		this->traceManagerPtr->histThreImgName = uiPtr->groupBox_6->title().toStdString();
		this->traceManagerPtr->imgThreSeq.push_back(this->traceManagerPtr->histThreImgName);
		this->traceManagerPtr->stdFold = uiPtr->doubleSpinBox_3->value();
	}
	else this->traceManagerPtr->histThre = false;
}

void FragTraceControlPanel::pa_objFilter()
{
	if (uiPtr->groupBox_13->isChecked())
	{
		this->traceManagerPtr->objFilter = true;
		if (uiPtr->radioButton_4->isChecked())
		{
			this->traceManagerPtr->voxelSize = true;
			this->traceManagerPtr->voxelCount = uiPtr->spinBox_14->value();
		}
	}
	else
	{
		this->traceManagerPtr->objFilter = false;
		this->traceManagerPtr->voxelSize = false;
	}

	if (uiPtr->groupBox_15->isChecked())
	{
		if (uiPtr->radioButton_5->isEnabled() || uiPtr->radioButton_5->isChecked())
		{
			this->traceManagerPtr->selectedSomaMap.clear();
			this->traceManagerPtr->selectedLocalSomaMap.clear();
			for (map<int, ImageMarker>::iterator somaIt = this->somaMap.begin(); somaIt != this->somaMap.end(); ++somaIt)
			{
				ImageMarker localMarker;
				for (QList<ImageMarker>::iterator it = this->selectedLocalMarkerList.begin(); it != this->selectedLocalMarkerList.end(); ++it)
				{
					if (somaIt->first == it->n)
					{
						localMarker = *it;
						this->localSomaMap.insert({ somaIt->first, localMarker });
						break;
					}
				}
			}
		
			this->traceManagerPtr->selectedSomaMap = this->somaMap;
			this->traceManagerPtr->selectedLocalSomaMap = this->localSomaMap;
		}
	}
}

void FragTraceControlPanel::pa_axonContinuous()
{
	if (uiPtr->groupBox_7->isChecked())
	{
		this->traceManagerPtr->continuousAxon = true;
		this->traceManagerPtr->axonMarkerAllowance = uiPtr->spinBox_7->value();
	}
	else this->traceManagerPtr->continuousAxon = false;
}

void FragTraceControlPanel::pa_objBasedMST()
{
	if (uiPtr->groupBox_8->isChecked())
	{
		this->traceManagerPtr->MST = true;
		this->traceManagerPtr->MSTtreeName = uiPtr->groupBox_8->title().toStdString();
		this->traceManagerPtr->minNodeNum = uiPtr->spinBox_5->value();
	}
	else
	{
		this->traceManagerPtr->MST = false;
	}
}

void FragTraceControlPanel::updateMarkerMonitor()
{
	map<int, ImageMarker> newMarkerMap;
	map<int, ImageMarker> oldMarkerMap;	
	if (this->selectedMarkerList.size() > 0)
	{
		// When trace dendritic area option is on, markers need to be labeled before being passed to [FragTraceManager].
		if ((uiPtr->radioButton_5->isEnabled() && uiPtr->radioButton_5->isChecked()))
		{
			for (QList<ImageMarker>::iterator it = this->selectedMarkerList.begin(); it != this->selectedMarkerList.end(); ++it)
			{
				it->selected = true;
				it->on = false;
				newMarkerMap.insert({ it->n, *it });
			}
		}
		else // For monitor purpose through Object Manager only, use [ImageMarker.name] to label markers instead.
		{
			for (QList<ImageMarker>::iterator it = this->selectedMarkerList.begin(); it != this->selectedMarkerList.end(); ++it)
			{
				it->selected = true;
				it->on = false;
				newMarkerMap.insert({ it->name.toInt(), *it });
			}
		}
		oldMarkerMap = this->somaMap;

		for (map<int, ImageMarker>::iterator it1 = newMarkerMap.begin(); it1 != newMarkerMap.end(); ++it1)
			if (oldMarkerMap.find(it1->first) == oldMarkerMap.end()) this->somaMap.insert({ it1->first, it1->second });

		for (map<int, ImageMarker>::iterator it2 = oldMarkerMap.begin(); it2 != oldMarkerMap.end(); ++it2)
			if (newMarkerMap.find(it2->first) == newMarkerMap.end()) this->somaMap[it2->first].selected = false;
	}
	else
	{
		if (this->somaListViewer->rowCount() > 0)
			for (int rowi = this->somaListViewer->rowCount() - 1; rowi >= 0; --rowi) this->somaListViewer->removeRow(rowi);
		this->somaMap.clear();
	}

	// When trace dendritic area option is on, markers need to be labeled before being passed to [FragTraceManager].
	if ((uiPtr->radioButton_5->isEnabled() && uiPtr->radioButton_5->isChecked()))
	{
		for (map<int, ImageMarker>::iterator markerIt = this->somaMap.begin(); markerIt != this->somaMap.end(); ++markerIt)
		{
			if (markerIt->second.selected && !markerIt->second.on)
			{
				int markerGlobalX = int(markerIt->second.x);
				int markerGlobalY = int(markerIt->second.y);
				int markerGlobalZ = int(markerIt->second.z);
				string displayName;
				if (uiPtr->checkBox->isChecked())
					displayName = "marker " + to_string(markerIt->first + 1) + ": (Z" + to_string(markerGlobalZ) + ", X" + to_string(markerGlobalX) + ", Y" + to_string(markerGlobalY) + ")";
				else if (uiPtr->checkBox_2->isChecked())
				{
					if (this->surType == 1)
						displayName = "marker " + to_string(markerIt->first + 1) + ": (Z" + to_string(markerGlobalZ) + ", X" + to_string(markerGlobalX) + ", Y" + to_string(markerGlobalY) + ")";
					else if (this->surType == 5)
						displayName = "point cloud " + to_string(markerIt->first + 1) + ": (Z" + to_string(markerGlobalZ) + ", X" + to_string(markerGlobalX) + ", Y" + to_string(markerGlobalY) + ")";
				}
				this->somaDisplayNameMap.insert({ markerIt->first, displayName });
				QString displayNameQ = QString::fromStdString(this->somaDisplayNameMap.at(markerIt->first));
				QStandardItem* newItemPtr = new QStandardItem(displayNameQ);
				this->somaListViewer->appendRow(newItemPtr);
			}
			else if (!markerIt->second.selected)
			{
				QString displayNameQ = QString::fromStdString(this->somaDisplayNameMap.at(markerIt->first));
				this->somaDisplayNameMap.erase(this->somaDisplayNameMap.find(markerIt->first));
				this->somaMap.erase(this->somaMap.find(markerIt->first));
				QList<QStandardItem*> matchedList = this->somaListViewer->findItems(displayNameQ);
				if (!matchedList.isEmpty())
				{
					int rowNum = (*matchedList.begin())->row();
					this->somaListViewer->removeRow(rowNum);
				}
			}
		}
	}
	else // For monitor purpose through Object Manager only, use [ImageMarker.name] to label markers instead.
	{
		for (map<int, ImageMarker>::iterator markerIt = this->somaMap.begin(); markerIt != this->somaMap.end(); ++markerIt)
		{
			if (markerIt->second.selected && !markerIt->second.on)
			{
				int markerGlobalX = int(markerIt->second.x);
				int markerGlobalY = int(markerIt->second.y);
				int markerGlobalZ = int(markerIt->second.z);
				string displayName;
				if (uiPtr->checkBox->isChecked())
					displayName = "marker " + to_string(markerIt->first) + ": (Z" + to_string(markerGlobalZ) + ", X" + to_string(markerGlobalX) + ", Y" + to_string(markerGlobalY) + ")";			
				else if (uiPtr->checkBox_2->isChecked())
				{
					if (this->surType == 1)
						displayName = "marker " + to_string(markerIt->first) + ": (Z" + to_string(markerGlobalZ) + ", X" + to_string(markerGlobalX) + ", Y" + to_string(markerGlobalY) + ")";
					else if (this->surType == 5)
						displayName = "point cloud " + to_string(markerIt->first) + ": (Z" + to_string(markerGlobalZ) + ", X" + to_string(markerGlobalX) + ", Y" + to_string(markerGlobalY) + ")";
				}
				this->somaDisplayNameMap.insert({ markerIt->first, displayName });
				QString displayNameQ = QString::fromStdString(this->somaDisplayNameMap.at(markerIt->first));
				QStandardItem* newItemPtr = new QStandardItem(displayNameQ);
				this->somaListViewer->appendRow(newItemPtr);
			}
			else if (!markerIt->second.selected)
			{
				QString displayNameQ = QString::fromStdString(this->somaDisplayNameMap.at(markerIt->first));
				this->somaDisplayNameMap.erase(this->somaDisplayNameMap.find(markerIt->first));
				this->somaMap.erase(this->somaMap.find(markerIt->first));
				QList<QStandardItem*> matchedList = this->somaListViewer->findItems(displayNameQ);
				if (!matchedList.isEmpty())
				{
					int rowNum = (*matchedList.begin())->row();
					this->somaListViewer->removeRow(rowNum);
				}
			}
		}
	}

	for (map<int, ImageMarker>::iterator markerIt = this->somaMap.begin(); markerIt != this->somaMap.end(); ++markerIt)
		markerIt->second.on = true;
}
/* ====================== END of [Parameter Collecting Functions] ====================== */


/* =================== Terafly/Editing Communicating Methods =================== */
void FragTraceControlPanel::getNAVersionNum()
{
	cout << endl << endl << "  --- Neuron Assembler: v" << MAINVERSION_NUM << "." << SUBVERSION_NUM << "." << PATCHVERSION_NUM << endl << endl;
}

void FragTraceControlPanel::updateCViewerPortal()
{
	this->CViewerPortal = this->thisCallback->castCViewer;
	this->fragEditorPtr->updateCViewerPortal();
}

void FragTraceControlPanel::sendSelectedMarkers2NA(const QList<ImageMarker>& selectedMarkerList, const QList<ImageMarker>& selectedLocalMarkerList)
{
	//for (QList<ImageMarker>::const_iterator it = selectedMarkerList.begin(); it != selectedMarkerList.end(); ++it)
	//	cout << it->n << ": " << it->x << " " << it->y << " " << it->z << endl;
	//cout << endl;

	if (this->selectedMarkerList != selectedMarkerList)
	{
		this->selectedMarkerList = selectedMarkerList;
		this->selectedLocalMarkerList = selectedLocalMarkerList;
		this->updateMarkerMonitor();
	}
}

void FragTraceControlPanel::eraserSegProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC>>& seg2includedNodeMap)
{
	//cout << "number of displaying segs: " << displayingSegs.seg.size() << endl;
	this->fragEditorPtr->erasingProcess(displayingSegs, seg2includedNodeMap);
}

void FragTraceControlPanel::connectSegProcess(V_NeuronSWC_list& displayingSegs, const map<int, vector<NeuronSWC>>& seg2includedNodeMap)
{
	//cout << "number of displaying segs: " << displayingSegs.seg.size() << endl;
	this->fragEditorPtr->connectingProcess(displayingSegs, seg2includedNodeMap);

	/*if (!FragTraceTester::isInstantiated())
	{
		FragTraceTester::instance(this);
		set<vector<float>> coords;
		for (auto& segID : seg2includedNodeMap)
		{
			for (auto& node : segID.second)
			{
				vector<float> coord;
				coord.push_back(node.x);
				coord.push_back(node.y);
				coord.push_back(node.z);
				coords.insert(coord);
			}
		}

		RGBA8 color;
		color.r = 255;
		color.g = 255;
		color.b = 255;

		FragTraceTester::getInstance()->pushMarkers(coords, color);

		FragTraceTester::uninstance();
	}*/
}

void FragTraceControlPanel::exitNAeditingMode()
{
	uiPtr->pushButton_12->setChecked(false);
	uiPtr->pushButton_13->setChecked(false);
	this->CViewerPortal->segEditing_setCursor("restore");
}

bool FragTraceControlPanel::changeAssociatedSegsClicked()
{
	if (uiPtr->pushButton_5->isChecked()) return true;
	else return false;
}

void FragTraceControlPanel::signalNA2retypeConnectedSegs(V_NeuronSWC_list& displayingSegs, const int retypedSeg, const int type)
{
	this->fragEditorPtr->sequencialTypeChanging(displayingSegs, retypedSeg, type);
}

void FragTraceControlPanel::signalNA2retypeConnectedSegs(V_NeuronSWC_list& displayingSegs, const set<int>& retypedSegs, const int type)
{
	//for (auto& segID : retypedSegs) cout << segID << " ";
	//cout << endl;

	if (retypedSegs.empty()) return;

	this->fragEditorPtr->sequencialTypeChanging(displayingSegs, retypedSegs, type);
}
/* =============== END of [Terafly/Editing Communicating Methods] =============== */



void FragTraceControlPanel::fillUpParamsForm()
{
	this->CViewerPortal->getParamsFromFragTraceUI("xyResRatio", float(this->thisCallback->getImageTeraFly()->getXDim() / this->thisCallback->getImageTeraFly()->getRezX()));
	this->CViewerPortal->getParamsFromFragTraceUI("zResRatio", float(this->thisCallback->getImageTeraFly()->getZDim() / this->thisCallback->getImageTeraFly()->getRezZ()));
}