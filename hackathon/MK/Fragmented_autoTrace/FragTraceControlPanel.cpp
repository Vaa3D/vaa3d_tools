#include <iostream>
#include <algorithm>
#include <cmath>

#include <qstringlist.h>
#include <qsettings.h>
#include <qfileinfo.h>
#include <qspinbox.h>
#include <qtimer.h>

#include "TeraflyCommunicator.h"
#include "FragTraceControlPanel.h"
#include "FragTracer_Define.h"

using namespace std;

FragTraceControlPanel::FragTraceControlPanel(QWidget* parent, V3DPluginCallback2* callback) : uiPtr(new Ui::FragmentedTraceUI), thisCallback(callback), QDialog(parent)
{
	// ------- Initialization ------- //
	this->traceManagerPtr = nullptr;
	this->volumeAdjusted = false;
	this->volumeAdjustedCoords = new int[6];
	this->globalCoords = new int[6];
	this->displayingDims = new int[3];

	this->markerMonitorSwitch = false;
	// ------------------------------ //

	// ------- Set up user interface ------- //
	uiPtr->setupUi(this);
	// ------------------------------------- //

	// ------- Adding widgets not provided in Qt Designer ------- //
	this->doubleSpinBox = new QDoubleSpinBox(uiPtr->frame_7);
	this->doubleSpinBox->setObjectName(QString::fromUtf8("doubleSpinBox"));
	this->doubleSpinBox->setGeometry(QRect(150, 10, 57, 22));
	this->doubleSpinBox->setValue(0);
	this->doubleSpinBox->setSingleStep(0.1);
	this->doubleSpinBox->setRange(-5, 5);
	//----------------------------------------------------------- //


	QSettings callOldSettings("SEU-Allen", "Fragment tracing");

	// ------- Image Format and Work Mode ------- //
	if (callOldSettings.value("terafly") == true)
	{
		uiPtr->checkBox->setChecked(true);
		uiPtr->checkBox_2->setChecked(false);
		uiPtr->checkBox_3->setChecked(false);
	}
	else if (callOldSettings.value("stack") == true)
	{
		uiPtr->checkBox->setChecked(false);
		uiPtr->checkBox_2->setChecked(true);
		uiPtr->checkBox_3->setChecked(false);
	}
	else if (callOldSettings.value("series") == true)
	{
		uiPtr->checkBox->setChecked(false);
		uiPtr->checkBox_2->setChecked(false);
		uiPtr->checkBox_3->setChecked(true);
	}

	if (callOldSettings.value("wholeBlock") == true)
	{
		uiPtr->radioButton->setChecked(true);
		uiPtr->radioButton_2->setChecked(false);
		uiPtr->radioButton_3->setChecked(false);
		uiPtr->groupBox_6->setEnabled(true);
	}
	else if (callOldSettings.value("dendrite") == true)
	{
		uiPtr->radioButton->setChecked(false);
		uiPtr->radioButton_2->setChecked(true);
		uiPtr->radioButton_3->setChecked(false);
		uiPtr->groupBox_6->setEnabled(false);
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

		if (callOldSettings.value("ada_saveCheck") == true)
		{
			uiPtr->checkBox_4->setChecked(true);
			uiPtr->lineEdit_2->setEnabled(true);
			uiPtr->pushButton_5->setEnabled(true);
		}
		else
		{
			uiPtr->checkBox_4->setChecked(false);
			uiPtr->lineEdit_2->setEnabled(false);
			uiPtr->pushButton_5->setEnabled(false);
		}
	}
	uiPtr->lineEdit_2->setText(callOldSettings.value("ada_savePath").toString());
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
			this->doubleSpinBox->setValue(callOldSettings.value("histThre_std").toFloat());

			if (callOldSettings.value("histThre_saveCheck") == true)
			{
				uiPtr->checkBox_5->setChecked(true);
				uiPtr->lineEdit_3->setEnabled(true);
				uiPtr->pushButton_6->setEnabled(true);
			}
			else
			{
				uiPtr->checkBox_5->setChecked(false);
				uiPtr->lineEdit_3->setEnabled(false);
				uiPtr->pushButton_6->setEnabled(false);
			}
		}
	}
	uiPtr->lineEdit_3->setText(callOldSettings.value("histThre_savePath").toString());
	// ---------------------------------------- //


	// ------- Object Filter Group Box ------- //
	if (callOldSettings.value("objFilter") == true)
	{
		if (callOldSettings.value("voxelCount") == true)
		{
			uiPtr->spinBox_14->setValue(callOldSettings.value("voxelCountThre").toInt());
		}
	}

	this->somaListViewer = new QStandardItemModel(this);
	uiPtr->listView_2->setModel(this->somaListViewer);
	// --------------------------------------- //


	// ------- Object-based MST ------- //
	if (callOldSettings.value("MST") == true)
	{
		uiPtr->groupBox_8->setChecked(true);
		uiPtr->spinBox_5->setValue(callOldSettings.value("minNodeNum").toInt());
	}
	// ------------------------------- //


	// ------- Segment Post-processing ------- //		
	this->listViewBlankAreas = new QStandardItemModel(this);
	uiPtr->listView->setModel(listViewBlankAreas);
	uiPtr->lineEdit->setText(callOldSettings.value("savePath").toString());
	// --------------------------------------- //

	string versionString = to_string(MAINVERSION_NUM) + "." + to_string(SUBVERSION_NUM) + "." + to_string(PATCHVERSION_NUM) + " beta";
	QString windowTitleQ = "Neuron Assembler v" + QString::fromStdString(versionString);
	this->setWindowTitle(windowTitleQ);  

	this->fragEditorPtr = new FragmentEditor(parent, callback);

	this->show();
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
			uiPtr->checkBox_2->setChecked(false);
			uiPtr->checkBox_3->setChecked(false);
		}
		else if (checkBoxName == "checkBox_2")
		{
			uiPtr->checkBox->setChecked(false);
			uiPtr->checkBox_3->setChecked(false);
		}
		else if (checkBoxName == "checkBox_3")
		{
			uiPtr->checkBox->setChecked(false);
			uiPtr->checkBox_2->setChecked(false);
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
		}
		else if (checkName == "radioButton")
		{
			uiPtr->groupBox_6->setEnabled(true);
			uiPtr->groupBox_6->setChecked(true);
		}
	}
}

void FragTraceControlPanel::multiSomaTraceChecked(bool checked) // groupBox_15; [Marker List / Multiple Dendritic Tracing]
{
	QObject* signalSender = sender();
	QString checkName = signalSender->objectName();

	if (checked)
	{
		this->refreshSomaCoords();
		this->markerMonitor();
	}
	else this->markerMonitorSwitch = false;
}

void FragTraceControlPanel::refreshSomaCoords()
{
	this->markerMonitorSwitch = false;
	thisCallback->refreshSelectedMarkers();
	for (int rowi = 0; rowi < this->somaListViewer->rowCount(); ++rowi) this->somaListViewer->removeRow(rowi);
	this->somaMap.clear();
	this->localSomaMap.clear();
	this->somaDisplayNameMap.clear();
	this->markerMonitorSwitch = true;
}

void FragTraceControlPanel::saveSegStepsResultChecked(bool checked)
{
	QObject* signalSender = sender();
	QString checkBoxName = signalSender->objectName();

	if (checked)
	{
		if (checkBoxName == "checkBox_4")
		{
			uiPtr->lineEdit_2->setEnabled(true);
			uiPtr->pushButton_5->setEnabled(true);
		}
		else if (checkBoxName == "checkBox_5")
		{
			uiPtr->lineEdit_3->setEnabled(true);
			uiPtr->pushButton_6->setEnabled(true);
		}
	}
}

void FragTraceControlPanel::blankAreaClicked()
{
	QObject* signalSender = sender();
	QString pushButtonName = signalSender->objectName();
	
	if (pushButtonName == "pushButton_7")
	{
		if (uiPtr->lineEdit_5->text() == "" || uiPtr->lineEdit_6->text() == "" || uiPtr->lineEdit_7->text() == "" || uiPtr->lineEdit_8->text() == "")
		{
			v3d_msg("One or more parameters weren't specified. Please double check.");
			return;
		}

		QString blankAreaName;
		blankAreaName = "(" + uiPtr->lineEdit_5->text() + ", " + uiPtr->lineEdit_6->text() + ", " + uiPtr->lineEdit_7->text() + ") radius = " + uiPtr->lineEdit_8->text();
		QStandardItem* newItem = new QStandardItem(blankAreaName);
		listViewBlankAreas->appendRow(newItem);
	}
	else if (pushButtonName == "pushButton_8")
	{
		QModelIndexList selectedArea = uiPtr->listView->selectionModel()->selectedRows();
		if (selectedArea.empty()) return;

		int rowNum = selectedArea.begin()->row();
		listViewBlankAreas->removeRow(rowNum);
	}
}

void FragTraceControlPanel::browseSavePathClicked()
{
	QObject* signalSender = sender();
	QString browseButtonName = signalSender->objectName();

	if (browseButtonName == "pushButton_4")
	{
		this->saveSWCFullName = QFileDialog::getSaveFileName(0, QObject::tr("Save auto-traced neuron"), "",
			QObject::tr("Supported file extension (*.swc *.eswc)" ";;Neuron structure (*.swc)" ";;Extended neuron structure (*.eswc)"));

		QSettings currSettings("SEU-Allen", "Fragment tracing");
		currSettings.setValue("savePath", this->saveSWCFullName);

		uiPtr->lineEdit->setText(this->saveSWCFullName);
	}
	else if (browseButtonName == "pushButton_5")
	{
		this->adaSaveRoot = QFileDialog::getExistingDirectory(0, QObject::tr("Save intermediate result (adaptive thresholding)"));

		QSettings currSettings("SEU-Allen", "Fragment tracing");
		currSettings.setValue("ada_savePath", this->adaSaveRoot);

		uiPtr->lineEdit_2->setText(this->adaSaveRoot);
	}
	else if (browseButtonName == "pushButton_6")
	{
		this->histMaskRoot = QFileDialog::getExistingDirectory(0, QObject::tr("Save intermediate result (histogram-based mask generation)"));

		QSettings currSettings("SEU-Allen", "Fragment tracing");
		currSettings.setValue("histThre_savePath", this->histMaskRoot);

		uiPtr->lineEdit_3->setText(this->histMaskRoot);
	}
}

void FragTraceControlPanel::saveSettingsClicked()
{
	QSettings settings("SEU-Allen", "Fragment tracing");
	
	// ------- Image Format and Work Mode ------- //
	if (uiPtr->checkBox->isChecked())
	{
		settings.setValue("terafly", true);
		settings.setValue("stack", false);
		settings.setValue("series", false);
	}
	else if (uiPtr->checkBox_2->isChecked())
	{
		settings.setValue("terafly", false);
		settings.setValue("stack", true);
		settings.setValue("series", false);
	}
	else if (uiPtr->checkBox_3->isChecked())
	{
		settings.setValue("terafly", false);
		settings.setValue("stack", false);
		settings.setValue("series", true);
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
		if (uiPtr->checkBox_4->isChecked())
		{
			settings.setValue("ada_saveCheck", true);
			settings.setValue("ada_savePath", uiPtr->lineEdit_2->text());
		}
		else
		{
			settings.setValue("ada_saveCheck", false);
			settings.setValue("ada_savePath", "");
		}
		settings.setValue("ada_imgName", uiPtr->groupBox_3->title());
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
		settings.setValue("histThre_std", this->doubleSpinBox->value());
		if (uiPtr->checkBox_5->isChecked())
		{
			settings.setValue("histThre_saveCheck", true);
			settings.setValue("histThre_savePath", uiPtr->lineEdit_3->text());
		}
		else
		{
			settings.setValue("histThre_saveCheck", false);
			settings.setValue("histThre_savePath", "");
		}
		settings.setValue("histThre_imgName", uiPtr->groupBox_6->title());
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


	settings.setValue("savaPath", uiPtr->lineEdit->text());
}

void FragTraceControlPanel::eraseButtonClicked()
{
	if (uiPtr->pushButton_12->isChecked())
	{
		this->fragEditorPtr->test("checked!");
		this->thisCallback->setEraseCursor(true);
		TeraflyCommunicator* myCommunicator;
		myCommunicator->getCViewerInstanceTest(myCommunicator);
		
	}
	else
	{
		this->fragEditorPtr->test("unChecked!");
		this->thisCallback->setEraseCursor(false);
	}
}
/* ====================== END of [User Interface Buttons] ======================= */


/* ============================== TRACING INITIALIZING FUNCTION =============================== */
void FragTraceControlPanel::traceButtonClicked()
{
	this->markerMonitorSwitch = false;

	QSettings currSettings("SEU-Allen", "Fragment tracing");
	if (currSettings.value("savePath").isNull())
	{
		cerr << " ==> Result save path not specified. Do nothing and return." << endl;
		return;
	}
	
	cout << "Fragment tracing procedure initiated." << endl;
		
	if (this->isVisible())
	{
		if (uiPtr->radioButton->isChecked() && !uiPtr->radioButton_2->isChecked()) // AXON TRACING
		{
			cout << " --> axon tracing, acquiring image information.." << endl;
			QString rootQ = "";
			if (uiPtr->lineEdit->text() != "") // final result save place
			{
				QStringList saveFullNameParse = uiPtr->lineEdit->text().split("/");
				for (QStringList::iterator parseIt = saveFullNameParse.begin(); parseIt != saveFullNameParse.end() - 1; ++parseIt) rootQ = rootQ + *parseIt + "/";
			}

			if (uiPtr->checkBox->isChecked()) // terafly format
			{				
				this->teraflyTracePrep(axon);	   	  // terafly image block preparation
				this->traceManagerPtr->finalSaveRootQ = rootQ;

				// ------------------- collect parameters ------------------- //
				this->pa_imgEnhancement();                         // image enhancement				
				this->pa_maskGeneration();                         // mask generation
				this->pa_objFilter();                              // object filter
				this->pa_objBasedMST();                            // object-based MST node connecting
				// ---------------------------------------------------------- //
			}
		}
		else if (!uiPtr->radioButton->isChecked() && uiPtr->radioButton_2->isChecked()) // DENDRITE TRACING
		{
			cout << " --> dendritic tree tracing, acquiring image information.." << endl;
			QString rootQ = "";
			if (uiPtr->lineEdit->text() != "") // final result save place
			{
				QStringList saveFullNameParse = uiPtr->lineEdit->text().split("/");
				for (QStringList::iterator parseIt = saveFullNameParse.begin(); parseIt != saveFullNameParse.end() - 1; ++parseIt) rootQ = rootQ + *parseIt + "/";
			}

			if (uiPtr->checkBox->isChecked()) // terafly format
			{
				this->teraflyTracePrep(dendriticTree); // terafly image block preparation
				this->traceManagerPtr->finalSaveRootQ = rootQ;

				// ------------------- collect parameters ------------------- //
				this->pa_imgEnhancement();                          // image enhancement
				this->pa_objFilter();                               // object filter
				this->pa_objBasedMST();                             // object-based MST node connecting 
				// ---------------------------------------------------------- //
			}
		}

		this->fillUpParamsForm();
	}
	
	this->connect(this->traceManagerPtr, SIGNAL(emitTracedTree(NeuronTree)), this, SLOT(catchTracedTree(NeuronTree)));

	//emit switchOnSegPipe(); // ==> Qt's [emit] is equivalent to normal function call. Therefore, no new thread is created due to this keyword.
	//QTimer::singleShot(0, this->traceManagerPtr, SLOT(imgProcPipe_wholeBlock())); // ==> Qt's [singleShot] is still enforced on the thread of event loop.
	
	if (this->somaListViewer->rowCount() == 0)
	{
		this->traceManagerPtr->selectedSomaMap.clear();
		this->traceManagerPtr->selectedLocalSomaMap.clear();
	}
	if (!this->traceManagerPtr->imgProcPipe_wholeBlock())
	{
		// Newly traced tree will be stored in this->tracedTree.
		v3d_msg(QString("The process has been terminated."));
		return;
	}

	this->disconnect(this->traceManagerPtr, SIGNAL(emitTracedTree(NeuronTree)), this, SLOT(catchTracedTree(NeuronTree)));


	if (this->volumeAdjusted) 
	{
		//cout << this->volumeAdjustedCoords[0] << " " << this->volumeAdjustedCoords[2] << " " << this->volumeAdjustedCoords[4] << endl;
		this->tracedTree = NeuronStructUtil::swcShift(this->tracedTree, this->volumeAdjustedCoords[0] - 1, this->volumeAdjustedCoords[2] - 1, this->volumeAdjustedCoords[4] - 1);
	} 

	NeuronTree existingTree = this->thisCallback->getSWCTeraFly();
	NeuronTree finalTree;
	if (existingTree.listNeuron.isEmpty())
	{
		this->scaleTracedTree();
		profiledTree tracedProfiledTree(this->tracedTree);
		this->thisCallback->setSWCTeraFly(tracedProfiledTree.tree);

		finalTree = this->tracedTree; // this is still the tree before iteratively connected by TreeGrower::itered_connectSegsWithinClusters
	}
	else
	{
		vector<NeuronTree> trees;
		cout << endl << "Scailing existing tree to local coords first: " << endl;
		NeuronTree scaledBackExistingTree = this->treeScaleBack(existingTree);
		trees.push_back(scaledBackExistingTree);
		NeuronTree newlyTracedPart = TreeGrower::swcSamePartExclusion(this->tracedTree, scaledBackExistingTree, 4, 8);
		profiledTree newlyTracedPartProfiled(newlyTracedPart);
		
		NeuronTree cleaned_newlyTracedPart;
		if (this->traceManagerPtr->minNodeNum > 0) cleaned_newlyTracedPart = NeuronStructUtil::singleDotRemove(newlyTracedPartProfiled, this->traceManagerPtr->minNodeNum);
		trees.push_back(cleaned_newlyTracedPart);

		profiledTree combinedProfiledTree(NeuronStructUtil::swcCombine(trees));
		//profiledTree finalProfiledTree = this->traceManagerPtr->segConnectAmongTrees(combinedProfiledTree, 5); // This line might be causing problem:
																												 //   1. Structural error that causes image block reading freezed when moving/zomming.
																												 //   2. Scailing error on final traced tree?? (compressed in z direction) - not confirmed. 
		this->tracedTree = combinedProfiledTree.tree;
		//this->tracedTree = combinedProfiledTree.tree;
		cout << endl << "Scaling combined tree back to real world coords: " << endl;
		this->scaleTracedTree();
		this->thisCallback->setSWCTeraFly(this->tracedTree);
	}

	if (uiPtr->lineEdit->text() != "") writeSWC_file(uiPtr->lineEdit->text(), finalTree);

	if (uiPtr->groupBox_15->isChecked())
	{
		this->markerMonitorSwitch = true;
		this->markerMonitor();
	}

	this->traceManagerPtr->partialVolumeLowerBoundaries = { 0, 0, 0 };
}
/* ============================================================================================ */



/* =========================== TRACING VOLUME PREPARATION =========================== */
void FragTraceControlPanel::teraflyTracePrep(workMode mode)
{
	// This method prepares the image data for FragTraceManager to trace.
	// The FRAGTRACEMANAGER INSTANCE IS CREATED HERE.

	this->volumeAdjusted = thisCallback->getPartialVolumeCoords(this->globalCoords, this->volumeAdjustedCoords, this->displayingDims);
#ifdef __IMAGE_VOLUME_PREPARATION_PRINTOUT__
	cout << " -- Displaying image local coords: x(" << volumeAdjustedCoords[0] << "-" << volumeAdjustedCoords[1] << ") y(" << volumeAdjustedCoords[2] << "-" << volumeAdjustedCoords[3] << ") z(" << volumeAdjustedCoords[4] << "-" << volumeAdjustedCoords[5] << ")" << endl;
	cout << " -- Whole image block dimension: " << displayingDims[0] << " " << displayingDims[1] << " " << displayingDims[2] << endl;
	cout << " -- Displaying image global coords: x(" << globalCoords[0] << "-" << globalCoords[1] << ") y(" << globalCoords[2] << "-" << globalCoords[3] << ") z(" << globalCoords[4] << "-" << globalCoords[5] << ")" << endl;
#endif

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

#ifdef __IMAGE_VOLUME_PREPARATION_DEBUG__
		// ------- For debug purpose ------- //
		unsigned char* croppedBlock1Dptr2 = new unsigned char[croppedImg4DSimplePtr->getXDim() * croppedImg4DSimplePtr->getYDim() * croppedImg4DSimplePtr->getZDim()];
		memcpy(croppedBlock1Dptr2, croppedImg4DSimplePtr->getRawData(), croppedImg4DSimplePtr->getTotalBytes());

		V3DLONG saveDims[4];
		saveDims[0] = croppedDims[0];
		saveDims[1] = croppedDims[1];
		saveDims[2] = croppedDims[2];
		saveDims[3] = 1;

		string saveName = "C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\testCase4\\test1.tif";
		const char* saveNameC = saveName.c_str();
		ImgManager::saveimage_wrapper(saveNameC, croppedBlock1Dptr, saveDims, 1);

		string saveName2 = "C:\\Users\\hsienchik\\Desktop\\Work\\FragTrace\\testCase4\\test2.tif";
		const char* saveNameC2 = saveName2.c_str();
		ImgManager::saveimage_wrapper(saveNameC2, croppedBlock1Dptr2, saveDims, 1);
		// --------------------------------- //
#endif

		if (this->traceManagerPtr == nullptr)
		{
			this->traceManagerPtr = new FragTraceManager(croppedImg4DSimplePtr, mode);
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
	}
	else
	{
		if (this->traceManagerPtr == nullptr)
			this->traceManagerPtr = new FragTraceManager(currBlockImg4DSimplePtr, mode);
		else this->traceManagerPtr->reinit(currBlockImg4DSimplePtr, mode);
	}
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
		if (uiPtr->checkBox_4->isChecked())
		{
			this->traceManagerPtr->saveAdaResults = true;
			this->traceManagerPtr->simpleAdaSaveDirQ = uiPtr->lineEdit_2->text();
			this->traceManagerPtr->simpleAdaSaveDirQ.replace(QString(" "), QString("_"));
		}
		else this->traceManagerPtr->saveAdaResults = false;
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
		this->traceManagerPtr->stdFold = this->doubleSpinBox->value();
		if (uiPtr->checkBox_5->isChecked())
		{
			this->traceManagerPtr->saveHistThreResults = true;
			this->traceManagerPtr->histThreSaveDirQ = uiPtr->lineEdit_3->text();
			this->traceManagerPtr->histThreSaveDirQ.replace(QString(" "), QString("_"));
		}
		else this->traceManagerPtr->saveHistThreResults = false;
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

void FragTraceControlPanel::markerMonitor()
{
	if (this->markerMonitorSwitch)
	{
		map<int, ImageMarker> newMarkerMap;
		map<int, ImageMarker> oldMarkerMap;
		thisCallback->getSelectedMarkerList(this->selectedMarkerList, this->selectedLocalMarkerList);
		if (this->selectedMarkerList.size() > 0)
		{
			for (QList<ImageMarker>::iterator it = this->selectedMarkerList.begin(); it != this->selectedMarkerList.end(); ++it)
			{
				it->selected = true;
				it->on = false;
				newMarkerMap.insert({ it->n, *it });
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
				for (int rowi = 0; rowi < this->somaListViewer->rowCount(); ++rowi) this->somaListViewer->removeRow(rowi);
			oldMarkerMap = this->somaMap;
			this->somaMap.clear();
		}

		for (map<int, ImageMarker>::iterator markerIt = this->somaMap.begin(); markerIt != this->somaMap.end(); ++markerIt)
		{
			if (markerIt->second.selected && !markerIt->second.on)
			{
				int markerGlobalX = int(markerIt->second.x);
				int markerGlobalY = int(markerIt->second.y);
				int markerGlobalZ = int(markerIt->second.z);
				string displayName = "marker " + to_string(markerIt->first + 1) + ": (Z" + to_string(markerGlobalZ) + ", X" + to_string(markerGlobalX) + ", Y" + to_string(markerGlobalY) + ")";
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

		for (map<int, ImageMarker>::iterator markerIt = this->somaMap.begin(); markerIt != this->somaMap.end(); ++markerIt)
			markerIt->second.on = true;

		QTimer::singleShot(50, this, SLOT(markerMonitor()));
	}
}
/* ====================== END of [Parameter Collecting Functions] ====================== */



/* =================== Result and Scaling Functions =================== */
void FragTraceControlPanel::scaleTracedTree()
{	
	float imgDims[3];
	imgDims[0] = this->thisCallback->getImageTeraFly()->getXDim();
	imgDims[1] = this->thisCallback->getImageTeraFly()->getYDim();
	imgDims[2] = this->thisCallback->getImageTeraFly()->getZDim();

	float imgRes[3];
	imgRes[0] = this->thisCallback->getImageTeraFly()->getRezX();
	imgRes[1] = this->thisCallback->getImageTeraFly()->getRezY();
	imgRes[2] = this->thisCallback->getImageTeraFly()->getRezZ();

	float imgOri[3];
	float factor = pow(2, abs(5 - this->thisCallback->getTeraflyResLevel()));
	cout << "  -- scaling factor = " << endl;
	/*if (this->thisCallback->getXlockStatus()) imgOri[0] = this->globalCoords[0];
	else imgOri[0] = this->thisCallback->getImageTeraFly()->getOriginX();
	if (this->thisCallback->getYlockStatus()) imgOri[1] = this->globalCoords[2];
	else imgOri[1] = this->thisCallback->getImageTeraFly()->getOriginY();
	if (this->thisCallback->getZlockStatus()) imgOri[2] = this->globalCoords[4];
	else imgOri[2] = this->thisCallback->getImageTeraFly()->getOriginZ();*/

	v3dhandle handle = this->thisCallback->currentImageWindow();
	QString imageTitle = this->thisCallback->getImageName(handle);
	//qDebug() << imageTitle;
	QStringList splitWhole = imageTitle.split("[");
	QStringList xSplit = splitWhole[1].split(",");
	QString xOriginQ = xSplit[0];
	QStringList ySplit = splitWhole[2].split(",");
	QString yOriginQ = ySplit[0];
	QStringList zSplit = splitWhole[3].split(",");
	QString zOriginQ = zSplit[0];

	imgOri[0] = xOriginQ.toFloat() * factor;
	imgOri[1] = yOriginQ.toFloat() * factor;
	imgOri[2] = zOriginQ.toFloat() * factor;

	//NeuronTree scaledTree = NeuronStructUtil::swcScale(this->tracedTree, imgRes[0] / imgDims[0], imgRes[1] / imgDims[1], imgRes[2] / imgDims[2]);
	NeuronTree scaledTree = NeuronStructUtil::swcScale(this->tracedTree, factor, factor, factor);
	NeuronTree scaledShiftedTree = NeuronStructUtil::swcShift(scaledTree, imgOri[0], imgOri[1], imgOri[2]);

#ifdef __IMAGE_VOLUME_PREPARATION_PRINTOUT__
	cout << "  -- Scaling back to real world dimension:" << endl;
	cout << "      image dims: " << imgDims[0] << " " << imgDims[1] << " " << imgDims[2] << endl;
	cout << "      image res: " << imgRes[0] << " " << imgRes[1] << " " << imgRes[2] << endl;
	cout << "      image origin: " << imgOri[0] << " " << imgOri[1] << " " << imgOri[2] << endl;
#endif

	//terafly::CViewer* currCiewerPtr = terafly::CViewer::getCurrent();
	//int XDim = currCiewerPtr->getXDim();
	//cout << XDim << endl;
	//system("pause");

	this->tracedTree = scaledShiftedTree;
}

NeuronTree FragTraceControlPanel::treeScaleBack(const NeuronTree& inputTree)
{
	float imgDims[3];
	imgDims[0] = this->thisCallback->getImageTeraFly()->getXDim();
	imgDims[1] = this->thisCallback->getImageTeraFly()->getYDim();
	imgDims[2] = this->thisCallback->getImageTeraFly()->getZDim();

	float imgRes[3];
	imgRes[0] = this->thisCallback->getImageTeraFly()->getRezX();
	imgRes[1] = this->thisCallback->getImageTeraFly()->getRezY();
	imgRes[2] = this->thisCallback->getImageTeraFly()->getRezZ();

	float imgOri[3];
	float factor = pow(2, abs(5 - this->thisCallback->getTeraflyResLevel()));
	cout << "  -- scaling factor = " << factor << " " << this->thisCallback->getTeraflyResLevel() << endl;
	/*if (this->thisCallback->getXlockStatus()) imgOri[0] = this->globalCoords[0];
	else imgOri[0] = this->thisCallback->getImageTeraFly()->getOriginX();
	if (this->thisCallback->getYlockStatus()) imgOri[1] = this->globalCoords[2];
	else imgOri[1] = this->thisCallback->getImageTeraFly()->getOriginY();
	if (this->thisCallback->getZlockStatus()) imgOri[2] = this->globalCoords[4];
	else imgOri[2] = this->thisCallback->getImageTeraFly()->getOriginZ();*/

	v3dhandle handle = this->thisCallback->currentImageWindow();
	QString imageTitle = this->thisCallback->getImageName(handle);
	qDebug() << imageTitle;
	QStringList splitWhole = imageTitle.split("[");
	QStringList xSplit = splitWhole[1].split(",");
	QString xOriginQ = xSplit[0];
	QStringList ySplit = splitWhole[2].split(",");
	QString yOriginQ = ySplit[0];
	QStringList zSplit = splitWhole[3].split(",");
	QString zOriginQ = zSplit[0];

	imgOri[0] = xOriginQ.toFloat() * factor;
	imgOri[1] = yOriginQ.toFloat() * factor;
	imgOri[2] = zOriginQ.toFloat() * factor;

	NeuronTree shiftBackTree = NeuronStructUtil::swcShift(inputTree, -imgOri[0], -imgOri[1], -imgOri[2]);
	//NeuronTree shiftScaleBackTree = NeuronStructUtil::swcScale(shiftBackTree, imgDims[0] / imgRes[0], imgDims[1] / imgRes[1], imgDims[2] / imgRes[2]); 
	NeuronTree shiftScaleBackTree = NeuronStructUtil::swcScale(shiftBackTree, 1 / factor, 1 / factor, 1 / factor);

#ifdef __IMAGE_VOLUME_PREPARATION_PRINTOUT__
	cout << "  -- Scaling to local volume dimension:" << endl;
	cout << "      image dims: " << imgDims[0] << " " << imgDims[1] << " " << imgDims[2] << endl;
	cout << "      image res: " << imgRes[0] << " " << imgRes[1] << " " << imgRes[2] << endl;
	cout << "      image origin: " << imgOri[0] << " " << imgOri[1] << " " << imgOri[2] << endl;
#endif

	return shiftScaleBackTree;
}
/* ================ END of [Result and Scaling Functions] ================ */




void FragTraceControlPanel::fillUpParamsForm()
{
	this->thisCallback->getParamsFromFragTraceUI("xyResRatio", float(this->thisCallback->getImageTeraFly()->getXDim() / this->thisCallback->getImageTeraFly()->getRezX()));
	this->thisCallback->getParamsFromFragTraceUI("zResRatio", float(this->thisCallback->getImageTeraFly()->getZDim() / this->thisCallback->getImageTeraFly()->getRezZ()));
}



void FragTraceControlPanel::blankArea()
{
	if (this->listViewBlankAreas->rowCount() != 0)
	{
		this->traceManagerPtr->blankArea = true;
		for (int areai = 0; areai < this->listViewBlankAreas->rowCount(); ++areai)
		{
			QStandardItem* thisArea = this->listViewBlankAreas->item(areai);
			QString thisAreaQString = thisArea->text();
			QStringList spaceSplits = thisAreaQString.split(" ");
			this->traceManagerPtr->blankRadius.push_back(spaceSplits.back().toInt());
			QStringList rightParanSplits = thisAreaQString.split(")");
			QStringList rightParanBlankSplits = rightParanSplits[0].split(" ");
			this->traceManagerPtr->blankZs.push_back(rightParanBlankSplits.back().toInt());
			rightParanBlankSplits[0].replace("(", "");
			rightParanBlankSplits[0].replace(",", "");
			rightParanBlankSplits[1].replace(",", "");
			this->traceManagerPtr->blankXs.push_back(rightParanBlankSplits[0].toInt() / 2);
			this->traceManagerPtr->blankYs.push_back(rightParanBlankSplits[1].toInt() / 2);
		}
	}
}