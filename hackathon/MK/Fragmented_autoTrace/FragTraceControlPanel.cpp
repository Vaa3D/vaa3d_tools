#include <iostream>
#include <algorithm>

#include <qstringlist.h>
#include <qsettings.h>
#include <qfileinfo.h>
#include <qspinbox.h>
#include <qtimer.h>

#include "FragTraceControlPanel.h"

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
	if (callOldSettings.value("PostElongDistChecked") == true)
	{
		uiPtr->lineEdit_4->setEnabled(true);
		uiPtr->lineEdit_4->setText(callOldSettings.value("PostElongDistThreshold").toString());
	}
		
	this->listViewBlankAreas = new QStandardItemModel(this);
	uiPtr->listView->setModel(listViewBlankAreas);
	uiPtr->lineEdit->setText(callOldSettings.value("savePath").toString());
	// --------------------------------------- //

	string versionString = to_string(MAINVERSION_NUM) + "." + to_string(SUBVERSION_NUM) + "." + to_string(PATCHVERSION_NUM) + " beta";
	QString windowTitleQ = "Neuron Assembler v" + QString::fromStdString(versionString);
	this->setWindowTitle(windowTitleQ);  

	this->show();
}

FragTraceControlPanel::~FragTraceControlPanel()
{
	delete this->doubleSpinBox;

	if (this->traceManagerPtr != nullptr) delete this->traceManagerPtr;

	delete uiPtr;
}



/* =========================== User Interface Configuration Buttons =========================== */
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

void FragTraceControlPanel::multiSomaTraceChecked(bool checked)
{
	QObject* signalSender = sender();
	QString checkName = signalSender->objectName();

	if (checked)
	{
		this->markerMonitorSwitch = true;
		this->markerMonitor();
	}
	else this->markerMonitorSwitch = false;
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


	// ------- Post Elongation ------- //
	if (uiPtr->groupBox_5->isChecked())
	{
		settings.setValue("PostElongDistChecked", true);
		settings.setValue("PostElongDistThreshold", uiPtr->lineEdit_4->text());
	}
	else
	{
		settings.setValue("PostElongDistChecked", false);
		settings.setValue("PostElongDistThreshold", "-1");
	}
	// ------------------------------- //


	settings.setValue("savaPath", uiPtr->lineEdit->text());
}
/* ====================== END of [User Interface Configuration Buttons] ======================= */



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
				this->pa_postElongation();                         // post elongation set up
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
				this->pa_postElongation();                          // post elongation set up
				// ---------------------------------------------------------- //
			}
		}

		this->fillUpParamsForm();
	}
	
	this->connect(this->traceManagerPtr, SIGNAL(emitTracedTree(NeuronTree)), this, SLOT(catchTracedTree(NeuronTree)));

	//emit switchOnSegPipe(); // ==> Qt's [emit] is equivalent to normal function call. Therefore, no new thread is created due to this keyword.
	//QTimer::singleShot(0, this->traceManagerPtr, SLOT(imgProcPipe_wholeBlock())); // ==> Qt's [singleShot] is still enforced on the thread of event loop.
	
	thisCallback->getPartialVolumeCoords(this->globalCoords, this->volumeAdjustedCoords, this->displayingDims);
	//cout << volumeAdjustedCoords[0] << " " << volumeAdjustedCoords[1] << " " << volumeAdjustedCoords[2] << " " << volumeAdjustedCoords[3] << " " << volumeAdjustedCoords[4] << " " << volumeAdjustedCoords[5] << endl;
	//cout << displayingDims[0] << " " << displayingDims[1] << " " << displayingDims[2] << endl;

	if (!this->traceManagerPtr->imgProcPipe_wholeBlock())
	{
		v3d_msg(QString("The process has been terminated."));
		return;
	}

	this->disconnect(this->traceManagerPtr, SIGNAL(emitTracedTree(NeuronTree)), this, SLOT(catchTracedTree(NeuronTree)));


	if (this->volumeAdjusted) 
	{
		//cout << this->volumeAdjustedCoords[0] << " " << this->volumeAdjustedCoords[2] << " " << this->volumeAdjustedCoords[4] << endl;
		this->tracedTree = NeuronStructUtil::swcShift(this->tracedTree, this->volumeAdjustedCoords[0] - 1, this->volumeAdjustedCoords[2] - 1, this->volumeAdjustedCoords[4] - 1);
	} 
	this->scaleTracedTree();

	NeuronTree existingTree = this->thisCallback->getSWCTeraFly();
	NeuronTree finalTree;
	if (existingTree.listNeuron.isEmpty())
	{
		NeuronStructExplorer myExplorer;
		profiledTree tracedProfiledTree(this->tracedTree);
		this->thisCallback->setSWCTeraFly(tracedProfiledTree.tree);

		finalTree = this->tracedTree;
	}
	else
	{
		vector<NeuronTree> trees;
		NeuronTree scaledBackExistingTree = this->treeScaleBack(existingTree);
		trees.push_back(scaledBackExistingTree);
		NeuronTree scaledBackTracedTree = this->treeScaleBack(this->tracedTree);
		NeuronTree newlyTracedPart = TreeGrower::swcSamePartExclusion(scaledBackTracedTree, scaledBackExistingTree, 4, 8);
		profiledTree newlyTracedPartProfiled(newlyTracedPart);
		NeuronTree cleaned_newlyTracedPart = NeuronStructUtil::singleDotRemove(newlyTracedPartProfiled, this->traceManagerPtr->minNodeNum);
		trees.push_back(cleaned_newlyTracedPart);

		profiledTree combinedProfiledTree(NeuronStructUtil::swcCombine(trees));
		profiledTree finalProfiledTree = this->traceManagerPtr->segConnectAmongTrees(combinedProfiledTree, 5);
		this->tracedTree = finalProfiledTree.tree;
		this->scaleTracedTree();
		this->thisCallback->setSWCTeraFly(this->tracedTree);
	}

	if (uiPtr->lineEdit->text() != "") writeSWC_file(uiPtr->lineEdit->text(), finalTree);
}
/* ============================================================================================ */



/* =========================== TRACING VOLUME PREPARATION =========================== */
void FragTraceControlPanel::teraflyTracePrep(workMode mode)
{
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

		// ------- For debug purpose ------- //
		/*unsigned char* croppedBlock1Dptr2 = new unsigned char[croppedImg4DSimplePtr->getXDim() * croppedImg4DSimplePtr->getYDim() * croppedImg4DSimplePtr->getZDim()];
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
		ImgManager::saveimage_wrapper(saveNameC2, croppedBlock1Dptr2, saveDims, 1);*/
		// --------------------------------- /

		if (mode == axon) this->traceManagerPtr = new FragTraceManager(croppedImg4DSimplePtr, axon);
		else if (mode == dendriticTree) this->traceManagerPtr = new FragTraceManager(croppedImg4DSimplePtr, dendriticTree);

		delete[] currBlock1Dptr;
		delete[] croppedBlock1Dptr;		
	}
	else
	{
		if (mode == axon) this->traceManagerPtr = new FragTraceManager(currBlockImg4DSimplePtr, axon);
		else if (mode == dendriticTree) this->traceManagerPtr = new FragTraceManager(currBlockImg4DSimplePtr, dendriticTree);
	}
}
/* ================================================================================== */



/*************************** Parameter Collecting Functions ***************************/
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

void FragTraceControlPanel::pa_postElongation()
{
	if (uiPtr->groupBox_5->isChecked())
		this->paramsFromUI.insert(pair<string, float>("labeledDistThreshold", atof(uiPtr->lineEdit_4->text().toStdString().c_str())));
	else
		this->paramsFromUI.insert(pair<string, float>("labeledDistThreshold", -1));
}

void FragTraceControlPanel::markerMonitor()
{
	if (this->markerMonitorSwitch)
	{
		int* global = new int[6];
		thisCallback->getCurrentGlobalVolumeCoords(global);

		list<vector<int>> newSomaList;
		list<vector<int>> tempLeftList;
		list<vector<int>> oldSomaList;
		this->somaNum = thisCallback->getSelectedMarkerNum();
		if (this->somaNum > 0)
		{
			this->somaCoords = new int[this->somaNum * 3];
			thisCallback->getSelectedMarkerCoords(this->somaCoords);
			for (int i = 0; i < this->somaNum; ++i)
			{
				vector<int> currSoma(3);
				for (int j = 0; j < 3; ++j) currSoma[j] = this->somaCoords[i * 3 + j];
				newSomaList.push_back(currSoma);
			}
			delete[] this->somaCoords;
			oldSomaList = this->somaList;
			this->somaList = newSomaList;
			tempLeftList = newSomaList;

			for (list<vector<int>>::iterator it = newSomaList.begin(); it != newSomaList.end(); ++it)
			{
				if (std::find(oldSomaList.begin(), oldSomaList.end(), *it) != oldSomaList.end())
				{
					oldSomaList.remove(*it);
					tempLeftList.remove(*it);
				}
			}
		}
		else
		{
			oldSomaList = this->somaList;
			this->somaList.clear();
		}

		for (list<vector<int>>::iterator it = oldSomaList.begin(); it != oldSomaList.end(); ++it)
		{
			QString itemName2beRemovedGlobal;
			QString itemName2beRemovedLocal;
			QString itemName2beRemoved;

			int realLocalX = int((float(it->at(0)) / 256) * float(global[1] - global[0]));
			int realLocalY = int((float(it->at(1)) / 256) * float(global[3] - global[2]));
			int realLocalZ = int((float(it->at(2)) / 256) * float(global[5] - global[4]));
			if (this->volumeAdjusted)
			{				
				itemName2beRemovedGlobal = "Marker coordinate: (x" + QString::number(realLocalX + global[0]) + ", y" +
																	 QString::number(realLocalY + global[2]) + ", z" +
																	 QString::number(realLocalZ + global[4]) + ")";
				/*itemName2beRemovedLocal = "Local coordinate: (" + QString::number(it->at(0) - volumeLocalCoords[0]) + ", " +
																  QString::number(it->at(1) - volumeLocalCoords[2]) + ", " +
																  QString::number(it->at(2) - volumeLocalCoords[4]) + ")";*/
				itemName2beRemoved = itemName2beRemovedGlobal + "  " + itemName2beRemovedLocal;
			}
			else
			{
				cout << global[0] << " " << global[2] << " " << global[4] << endl;
				itemName2beRemovedGlobal = "Marker coordinate: (x" + QString::number(realLocalX + global[0]) + ", y" +
																	 QString::number(realLocalY + global[2]) + ", z" +
																	 QString::number(realLocalZ + global[4]) + ")";
				//itemName2beRemovedLocal = "Local coordinate: (" + QString::number(it->at(0)) + ", " + QString::number(it->at(1)) + ", " + QString::number(it->at(2)) + ")";
				itemName2beRemoved = itemName2beRemovedGlobal + "  " + itemName2beRemovedLocal;
			}
			
			QList<QStandardItem*> matchedList = this->somaListViewer->findItems(itemName2beRemoved);
			int rowNum = (*matchedList.begin())->row();
			this->somaListViewer->removeRow(rowNum);
		}

		for (list<vector<int>>::iterator it = tempLeftList.begin(); it != tempLeftList.end(); ++it)
		{
			QString itemName2beAddedGlobal;
			QString itemName2beAddedLocal;
			QString itemName2beAdded;

			int realLocalX = int((float(it->at(0)) / 256) * float(global[1] - global[0]));
			int realLocalY = int((float(it->at(1)) / 256) * float(global[3] - global[2]));
			int realLocalZ = int((float(it->at(2)) / 256) * float(global[5] - global[4]));
			if (this->volumeAdjusted)
			{
				itemName2beAddedGlobal = "Marker coordinate: (x" + QString::number(realLocalX + global[0]) + ", y" +
																   QString::number(realLocalY + global[2]) + ", z" +
																   QString::number(realLocalZ + global[4]) + ")";
				/*itemName2beAddedLocal = "Local coordinate: (" + QString::number(it->at(0) - volumeLocalCoords[0]) + ", " +
																QString::number(it->at(1) - volumeLocalCoords[2]) + ", " +
																QString::number(it->at(2) - volumeLocalCoords[4]) + ")";*/
				itemName2beAdded = itemName2beAddedGlobal + "  " + itemName2beAddedLocal;
			}
			else
			{
				cout << global[0] << " " << global[2] << " " << global[4] << endl;
				itemName2beAddedGlobal = "Marker coordinate: (x" + QString::number(realLocalX + global[0]) + ", y" +
																   QString::number(realLocalY + global[2]) + ", z" +
																   QString::number(realLocalZ + global[4]) + ")";
				//itemName2beAddedLocal = "Local coordinate: (" + QString::number(it->at(0)) + ", " + QString::number(it->at(1)) + ", " + QString::number(it->at(2)) + ")";
				itemName2beAdded = itemName2beAddedGlobal + "  " + itemName2beAddedLocal;
			}

			QStandardItem* newItemPtr = new QStandardItem(itemName2beAdded);
			this->somaListViewer->appendRow(newItemPtr);
		}
		delete[] global;

		QTimer::singleShot(50, this, SLOT(markerMonitor()));
	}
}
/********************** END of [Parameter Collecting Functions] ***********************/



/***************** Result and Scaling Functions *****************/
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
	imgOri[0] = this->thisCallback->getImageTeraFly()->getOriginX();
	imgOri[1] = this->thisCallback->getImageTeraFly()->getOriginY();
	imgOri[2] = this->thisCallback->getImageTeraFly()->getOriginZ();

	NeuronTree scaledTree = NeuronStructUtil::swcScale(this->tracedTree, imgRes[0] / imgDims[0], imgRes[1] / imgDims[1], imgRes[2] / imgDims[2]);
	NeuronTree scaledShiftedTree = NeuronStructUtil::swcShift(scaledTree, imgOri[0], imgOri[1], imgOri[2]);

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
	imgOri[0] = this->thisCallback->getImageTeraFly()->getOriginX();
	imgOri[1] = this->thisCallback->getImageTeraFly()->getOriginY();
	imgOri[2] = this->thisCallback->getImageTeraFly()->getOriginZ();

	NeuronTree shiftBackTree = NeuronStructUtil::swcShift(inputTree, -imgOri[0], -imgOri[1], -imgOri[2]);
	NeuronTree shiftScaleBackTree = NeuronStructUtil::swcScale(shiftBackTree, imgDims[0] / imgRes[0], imgDims[1] / imgRes[1], imgDims[2] / imgRes[2]); 

	return shiftScaleBackTree;
}
/************ END of [Result and Scaling Functions] *************/




void FragTraceControlPanel::fillUpParamsForm()
{
	this->thisCallback->getParamsFromFragTraceUI("labeledDistThreshold", atof(uiPtr->lineEdit_4->text().toStdString().c_str()));
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