#include <iostream>

#include <qsettings.h>
#include <qfileinfo.h>

#include "FragTraceControlPanel.h"

using namespace std;

FragTraceControlPanel::FragTraceControlPanel(QWidget* parent, V3DPluginCallback2* callback, bool showMenu) : uiPtr(new Ui::FragmentedTraceUI), thisCallback(callback), QDialog(parent)
{
	if (showMenu)
	{
		uiPtr->setupUi(this);

		QSettings callOldSettings("SEU-Allen", "Fragment tracing");

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
		}
		else if (callOldSettings.value("withSeed") == true)
		{
			uiPtr->radioButton->setChecked(false);
			uiPtr->radioButton_2->setChecked(true);
			uiPtr->radioButton_3->setChecked(false);
		}
		else if (callOldSettings.value("bouton") == true)
		{
			uiPtr->radioButton->setChecked(false);
			uiPtr->radioButton_2->setChecked(false);
			uiPtr->radioButton_3->setChecked(true);
		}

		if (callOldSettings.value("ada") == true)
		{
			uiPtr->groupBox_3->setChecked(true);
			uiPtr->spinBox->setValue(callOldSettings.value("ada_stepsize").toInt());
			uiPtr->spinBox_2->setValue(callOldSettings.value("ada_rate").toInt());

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

		if (callOldSettings.value("histThre") == true)
		{
			uiPtr->groupBox_6->setChecked(true);
			uiPtr->spinBox_3->setValue(callOldSettings.value("histThre_std").toInt());

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
		uiPtr->lineEdit_3->setText(callOldSettings.value("histThre_savePath").toString());

		if (callOldSettings.value("smallRemove") == true)
		{
			uiPtr->groupBox_7->setChecked(true);
			uiPtr->spinBox_2->setEnabled(true);
			uiPtr->spinBox_7->setValue(callOldSettings.value("sizeThre").toInt());
		}
		else
		{
			uiPtr->groupBox_7->setChecked(false);
			uiPtr->spinBox_7->setValue(callOldSettings.value("sizeThre").toInt());
			uiPtr->spinBox_7->setEnabled(false);
		}

		if (callOldSettings.value("MST") == true)
		{
			uiPtr->groupBox_8->setChecked(true);
			uiPtr->spinBox_5->setValue(callOldSettings.value("largestDist").toInt());
			uiPtr->spinBox_6->setValue(callOldSettings.value("minNodes").toInt());

			if (callOldSettings.value("tiledMST") == true)
			{
				uiPtr->spinBox_4->setValue(callOldSettings.value("tileLength").toInt());
				uiPtr->spinBox_8->setValue(callOldSettings.value("zSection").toInt());
			}
		}

		uiPtr->lineEdit->setText(callOldSettings.value("savePath").toString());

		this->show();
	}
	else this->traceButtonClicked();
}

FragTraceControlPanel::~FragTraceControlPanel()
{
	thisCallback = nullptr;
	delete uiPtr;

	if (this->traceManagerPtr != nullptr) delete this->traceManagerPtr;
}

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
	QString checkBoxName = signalSender->objectName();

	if (checked)
	{
		if (checkBoxName == "groupBox_7")
		{
			uiPtr->spinBox_7->setEnabled(true);
		}
	}
	else
	{
		if (checkBoxName == "groupBox_7")
		{
			uiPtr->spinBox_7->setEnabled(false);
		}
	}
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
		settings.setValue("withSeed", false);
		settings.setValue("bouton", false);
	}
	else if (uiPtr->radioButton_2->isChecked())
	{
		settings.setValue("wholeBlock", false);
		settings.setValue("withSeed", true);
		settings.setValue("bouton", false);
	}
	else if (uiPtr->radioButton_3->isChecked())
	{
		settings.setValue("wholeBlock", false);
		settings.setValue("withSeed", false);
		settings.setValue("bouton", true);
	}

	if (uiPtr->groupBox_3->isChecked())
	{
		settings.setValue("ada", true);
		settings.setValue("ada_stepsize", uiPtr->spinBox->value());
		settings.setValue("ada_rate", uiPtr->spinBox_2->value());
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
	}

	if (uiPtr->checkBox_6->isChecked()) settings.setValue("gamma", true);

	if (uiPtr->groupBox_6->isChecked())
	{
		settings.setValue("histThre", true);
		settings.setValue("histThre_std", uiPtr->spinBox_3->value());
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

	if (uiPtr->groupBox_7->isChecked())
	{
		settings.setValue("smallRemove", true);
		settings.setValue("sizeThre", uiPtr->spinBox_7->value());
	}
	else
	{
		settings.setValue("smallRemove", false);
		settings.setValue("sizeThre", "");
	}
	settings.setValue("smallRemoveTreeName", uiPtr->groupBox_7->title());

	if (uiPtr->groupBox_8->isChecked())
	{
		settings.setValue("MST", true);
		settings.setValue("largestDist", uiPtr->spinBox_5->value());
		settings.setValue("minNodes", uiPtr->spinBox_6->value());

		if (uiPtr->groupBox_9->isChecked())
		{
			settings.setValue("tiledMST", true);
			settings.setValue("tileLength", uiPtr->spinBox_4->value());
			settings.setValue("zSection", uiPtr->spinBox_8->value());
		}
		else
		{
			settings.setValue("tiledMST", false);
			settings.setValue("tileLength", "");
			settings.setValue("zSection", "");
		}
	}
	else
	{
		settings.setValue("MST", false);
		settings.setValue("largestDist", "");
		settings.setValue("minNodes", "");
	}
	settings.setValue("MSTtreeName", uiPtr->groupBox_8->title());

	settings.setValue("savaPath", uiPtr->lineEdit->text());
}

void FragTraceControlPanel::traceButtonClicked()
{
	QSettings currSettings("SEU-Allen", "Fragment tracing");
	if (currSettings.value("savePath").isNull())
	{
		cerr << " ==> Result save path not specified. Do nothing and return." << endl;
		return;
	}
	
	cout << "Fragment tracing procedure initiated." << endl;
	if (currSettings.value("wholeBlock") == true && currSettings.value("withSeed") == false)
	{
		cout << " whole block tracing, acquiring image information.." << endl;
		
		if (this->isVisible())
		{
			QStringList saveFullNameParse = uiPtr->lineEdit->text().split("/");
			QString rootQ;
			for (QStringList::iterator parseIt = saveFullNameParse.begin(); parseIt != saveFullNameParse.end() - 1; ++parseIt) rootQ = rootQ + *parseIt + "/";

			if (uiPtr->checkBox->isChecked())
			{
				this->traceManagerPtr = new FragTraceManager(thisCallback->getImageTeraFly());
				this->traceManagerPtr->finalSaveRootQ = rootQ;

				if (uiPtr->groupBox_3->isChecked())
				{
					this->traceManagerPtr->ada = true;
					this->traceManagerPtr->adaImgName = uiPtr->groupBox_3->title().toStdString();
					this->traceManagerPtr->imgEnhanceSeq.push_back(this->traceManagerPtr->adaImgName);
					this->traceManagerPtr->simpleAdaStepsize = uiPtr->spinBox->value();
					this->traceManagerPtr->simpleAdaRate = uiPtr->spinBox_2->value();
					if (uiPtr->checkBox_4->isChecked())
					{
						this->traceManagerPtr->saveAdaResults = true;
						this->traceManagerPtr->simpleAdaSaveDirQ = uiPtr->lineEdit_2->text();
					}
					else this->traceManagerPtr->saveAdaResults = false;
				}
				else this->traceManagerPtr->ada = false;

				if (uiPtr->checkBox_6->isChecked()) this->traceManagerPtr->gammaCorrection = true;
				else this->traceManagerPtr->gammaCorrection = false;

				if (uiPtr->groupBox_6->isChecked())
				{
					this->traceManagerPtr->histThre = true;
					this->traceManagerPtr->histThreImgName = uiPtr->groupBox_6->title().toStdString();
					this->traceManagerPtr->imgThreSeq.push_back(this->traceManagerPtr->histThreImgName);
					this->traceManagerPtr->stdFold = uiPtr->spinBox_3->value();
					if (uiPtr->checkBox_5->isChecked())
					{
						this->traceManagerPtr->saveHistThreResults = true;
						this->traceManagerPtr->histThreSaveDirQ = uiPtr->lineEdit_3->text();
					}
					else this->traceManagerPtr->saveHistThreResults = false;
				}
				else this->traceManagerPtr->histThre = false;

				if (uiPtr->groupBox_7->isChecked())
				{
					this->traceManagerPtr->smallBlobRemove = true;
					this->traceManagerPtr->smallBlobRemovalName = uiPtr->groupBox_7->title().toStdString();
					this->traceManagerPtr->smallBlobThreshold = uiPtr->spinBox_7->value();
				}
				else this->traceManagerPtr->smallBlobRemove = false;

				if (uiPtr->groupBox_8->isChecked())
				{
					this->traceManagerPtr->MST = true;
					this->traceManagerPtr->MSTtreeName = uiPtr->groupBox_8->title().toStdString();
					this->traceManagerPtr->segLengthLimit = uiPtr->spinBox_5->value();
					this->traceManagerPtr->minNodeNum = uiPtr->spinBox_6->value();

					if (uiPtr->groupBox_9->isChecked())
					{
						this->traceManagerPtr->tiledMST = true;
						this->traceManagerPtr->tileLength = uiPtr->spinBox_4->value();
						this->traceManagerPtr->zSectionNum = uiPtr->spinBox_8->value();
					}
					else this->traceManagerPtr->tiledMST = false;
				}
				else
				{
					this->traceManagerPtr->MST = false;
					this->traceManagerPtr->tiledMST = false;
				}
			}
		}
		else if (!this->isVisible())
		{
			QString saveFullNameQ = currSettings.value("savePath").toString();
			QStringList saveFullNameParse = saveFullNameQ.split("/");
			QString rootQ;
			for (QStringList::iterator parseIt = saveFullNameParse.begin(); parseIt != saveFullNameParse.end() - 1; ++parseIt) rootQ = rootQ + *parseIt + "/";

			if (currSettings.value("terafly") == true)
			{
				this->traceManagerPtr = new FragTraceManager(thisCallback->getImageTeraFly());
				this->traceManagerPtr->finalSaveRootQ = rootQ;

				if (currSettings.value("ada") == true)
				{
					this->traceManagerPtr->ada = true;
					this->traceManagerPtr->adaImgName = currSettings.value("ada_imgName").toString().toStdString();
					this->traceManagerPtr->imgEnhanceSeq.push_back(this->traceManagerPtr->adaImgName);
					this->traceManagerPtr->simpleAdaStepsize = currSettings.value("ada_stepsize").toInt();
					this->traceManagerPtr->simpleAdaRate = currSettings.value("ada_rate").toInt();
					if (currSettings.value("ada_saveCheck") == true)
					{
						this->traceManagerPtr->saveAdaResults = true;
						this->traceManagerPtr->simpleAdaSaveDirQ = currSettings.value("ada_savePath").toString();
					}
					else this->traceManagerPtr->saveAdaResults = false;
				}
				else this->traceManagerPtr->ada = false;

				if (currSettings.value("gamma") == true) this->traceManagerPtr->gammaCorrection = true;
				else this->traceManagerPtr->gammaCorrection = false;

				if (currSettings.value("histThre") == true)
				{
					this->traceManagerPtr->histThre = true;
					this->traceManagerPtr->histThreImgName = currSettings.value("histThre_imgName").toString().toStdString();
					this->traceManagerPtr->imgThreSeq.push_back(this->traceManagerPtr->histThreImgName);
					this->traceManagerPtr->stdFold = currSettings.value("histThre_std").toInt();
					if (currSettings.value("histThre_saveCheck") == true)
					{
						this->traceManagerPtr->saveHistThreResults = true;
						this->traceManagerPtr->histThreSaveDirQ = currSettings.value("histThre_savePath").toString();
					}
					else this->traceManagerPtr->saveHistThreResults = false;
				}
				else this->traceManagerPtr->histThre = false;

				if (currSettings.value("smallRemove") == true)
				{
					this->traceManagerPtr->smallBlobRemove = true;
					this->traceManagerPtr->smallBlobRemovalName = currSettings.value("smallRemoveTreeName").toString().toStdString();
					this->traceManagerPtr->smallBlobThreshold = currSettings.value("sizeThre").toInt();
				}
				else this->traceManagerPtr->smallBlobRemove = false;

				if (currSettings.value("MST") == true)
				{
					this->traceManagerPtr->MST = true;
					this->traceManagerPtr->MSTtreeName = currSettings.value("MSTtreeName").toString().toStdString();
					this->traceManagerPtr->segLengthLimit = currSettings.value("largestDist").toInt();
					this->traceManagerPtr->minNodeNum = currSettings.value("minNodes").toInt();

					if (currSettings.value("tiledMST") == true)
					{
						this->traceManagerPtr->tiledMST = true;
						this->traceManagerPtr->tileLength = currSettings.value("tileLength").toInt();
						this->traceManagerPtr->zSectionNum = currSettings.value("zSection").toInt();
					}
					else this->traceManagerPtr->tiledMST = false;
				}
				else
				{
					this->traceManagerPtr->MST = false;
					this->traceManagerPtr->tiledMST = false;
				}
			}
		}
		
		this->connect(this, SIGNAL(switchOnSegPipe()), this->traceManagerPtr, SLOT(imgProcPipe_wholeBlock()));
		this->connect(this->traceManagerPtr, SIGNAL(emitTracedTree(NeuronTree)), this, SLOT(catchTracedTree(NeuronTree)));

		emit switchOnSegPipe();

		this->disconnect(this, SIGNAL(switchOnSegPipe()), this->traceManagerPtr, SLOT(imgProcPipe_wholeBlock()));
		this->disconnect(this->traceManagerPtr, SIGNAL(emitTracedTree(NeuronTree)), this, SLOT(catchTracedTree(NeuronTree)));

		this->scaleTracedTree();
		this->thisCallback->setSWCTeraFly(this->tracedTree);
	}
	else if (currSettings.value("wholeBlock") == false && currSettings.value("withSeed") == true)
	{
		cout << " trace with given seed point, acquiring image information.." << endl;

		if (this->isVisible())
		{

		}
		else if (!this->isVisible())
		{

		}
	}
}

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

	NeuronTree scaledTree = NeuronStructUtil::swcScale(this->tracedTree, imgDims[0] / imgRes[0], imgDims[1] / imgRes[1], imgDims[2] / imgRes[2]);
	NeuronTree scaledShiftedTree = NeuronStructUtil::swcShift(scaledTree, imgOri[0], imgOri[1], imgOri[2]);
	this->tracedTree = scaledShiftedTree;
}