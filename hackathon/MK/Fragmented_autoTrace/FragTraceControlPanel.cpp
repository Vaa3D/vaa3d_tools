#include <iostream>

#include <qstringlist.h>
#include <qsettings.h>
#include <qfileinfo.h>
#include <qspinbox.h>
#include <qtimer.h>

#include "FragTraceControlPanel.h"

using namespace std;

FragTraceControlPanel::FragTraceControlPanel(QWidget* parent, V3DPluginCallback2* callback, bool showMenu) : uiPtr(new Ui::FragmentedTraceUI), thisCallback(callback), QDialog(parent)
{
	if (showMenu)
	{
		uiPtr->setupUi(this);

		// ------- Adding widgets not provided in Qt Designer -------
		this->doubleSpinBox = new QDoubleSpinBox(uiPtr->frame_7);
		this->doubleSpinBox->setObjectName(QString::fromUtf8("doubleSpinBox"));
		this->doubleSpinBox->setGeometry(QRect(150, 10, 57, 22));
		this->doubleSpinBox->setValue(0);
		this->doubleSpinBox->setSingleStep(0.1);
		this->doubleSpinBox->setRange(-5, 5);
		//-----------------------------------------------------------


		QSettings callOldSettings("SEU-Allen", "Fragment tracing");

		// ------- Image Format and Work Mode -------
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


		// ------- Image Enhancement Group Box -------
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


		// ------- Mask Generation Group Box -------
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


		// ------- Object Filter Group Box -------
		if (callOldSettings.value("objFilter") == true)
		{
			if (callOldSettings.value("voxelCount") == true)
			{
				uiPtr->spinBox_14->setValue(callOldSettings.value("voxelCountThre").toInt());
			}
		}


		// ------- Object-based MST -------
		if (callOldSettings.value("MST") == true)
		{
			uiPtr->groupBox_8->setChecked(true);
			uiPtr->spinBox_5->setValue(callOldSettings.value("minNodeNum").toInt());
		}


		// ------- Segment Post-processing -------
		if (callOldSettings.value("PostElongDistChecked") == true)
		{
			uiPtr->lineEdit_4->setEnabled(true);
			uiPtr->lineEdit_4->setText(callOldSettings.value("PostElongDistThreshold").toString());
		}
		
		this->listViewBlankAreas = new QStandardItemModel(this);
		uiPtr->listView->setModel(listViewBlankAreas);
		uiPtr->lineEdit->setText(callOldSettings.value("savePath").toString());

		string versionString = to_string(MAINVERSION_NUM) + "." + to_string(SUBVERSION_NUM) + "." + to_string(PATCHVERSION_NUM) + " beta";
		QString windowTitleQ = "Fragment-based Auto-tracing v" + QString::fromStdString(versionString);
		this->setWindowTitle(windowTitleQ);
		this->show();

		this->traceManagerPtr = nullptr;
	}
	else this->traceButtonClicked();
}

FragTraceControlPanel::~FragTraceControlPanel()
{
	delete this->doubleSpinBox;

	if (this->traceManagerPtr != nullptr) delete this->traceManagerPtr;

	delete uiPtr;
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
	
	// ------- Image Format and Work Mode -------
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


	// ------- Image Enhancement Group Box -------
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


	// ------- Mask Generation Group Box -------
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


	// ------- Object Filter Group Box -------
	if (uiPtr->radioButton_4->isChecked())
	{
		settings.setValue("voxelCount", true);
		settings.setValue("voxelCountThre", uiPtr->spinBox_14->value());
	}
	if (uiPtr->groupBox_13->isChecked()) settings.setValue("objFilter", true);
	else settings.setValue("objFilter", false);


	// ------- Object-base MST -------
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


	// ------- Post Elongation -------
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
		
	if (this->isVisible())
	{
		if (uiPtr->radioButton->isChecked() && !uiPtr->radioButton_2->isChecked())
		{
			cout << " whole block tracing, acquiring image information.." << endl;
			QString rootQ = "";
			if (uiPtr->lineEdit->text() != "") // final result save place
			{
				QStringList saveFullNameParse = uiPtr->lineEdit->text().split("/");
				for (QStringList::iterator parseIt = saveFullNameParse.begin(); parseIt != saveFullNameParse.end() - 1; ++parseIt) rootQ = rootQ + *parseIt + "/";
			}

			if (uiPtr->checkBox->isChecked())
			{
				this->traceManagerPtr = new FragTraceManager(thisCallback->getImageTeraFly(), wholeBlock_axon);
				this->traceManagerPtr->finalSaveRootQ = rootQ;

				if (uiPtr->groupBox_7->isChecked())
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

				// ------- Image Enhancement -------
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


				// ------- Mask Generation -------
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


				// ------- Object Filter -------
				// -- size threshold --
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


				// ------- Object-base MST -------
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


				// ------- Post Elongation -------
				if (uiPtr->groupBox_5->isChecked())
					this->paramsFromUI.insert(pair<string, float>("labeledDistThreshold", atof(uiPtr->lineEdit_4->text().toStdString().c_str())));
				else
					this->paramsFromUI.insert(pair<string, float>("labeledDistThreshold", -1));
			}
		}
		else if (!uiPtr->radioButton->isChecked() && uiPtr->radioButton_2->isChecked())
		{
			cout << " dendritic tree tracing, acquiring image information.." << endl;
			QString rootQ = "";
			if (uiPtr->lineEdit->text() != "") // final result save place
			{
				QStringList saveFullNameParse = uiPtr->lineEdit->text().split("/");
				for (QStringList::iterator parseIt = saveFullNameParse.begin(); parseIt != saveFullNameParse.end() - 1; ++parseIt) rootQ = rootQ + *parseIt + "/";
			}

			if (uiPtr->checkBox->isChecked())
			{
				this->traceManagerPtr = new FragTraceManager(thisCallback->getImageTeraFly(), dendriticTree);
				this->traceManagerPtr->finalSaveRootQ = rootQ;

				if (uiPtr->groupBox_7->isChecked())
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

				// ------- Image Enhancement -------
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


				// ------- Object Filter -------
				// -- size threshold --
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


				// ------- Object-base MST -------
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


				// ------- Post Elongation -------
				if (uiPtr->groupBox_5->isChecked())
					this->paramsFromUI.insert(pair<string, float>("labeledDistThreshold", atof(uiPtr->lineEdit_4->text().toStdString().c_str())));
				else
					this->paramsFromUI.insert(pair<string, float>("labeledDistThreshold", -1));
			}
		}

		this->fillUpParamsForm();
	}
	else if (!this->isVisible())
	{
		QString rootQ = "";
		QString saveFullNameQ = currSettings.value("savePath").toString();
		if (saveFullNameQ != "")
		{
			QStringList saveFullNameParse = saveFullNameQ.split("/");
			for (QStringList::iterator parseIt = saveFullNameParse.begin(); parseIt != saveFullNameParse.end() - 1; ++parseIt) rootQ = rootQ + *parseIt + "/";
		}

		if (currSettings.value("terafly") == true)
		{
			this->traceManagerPtr = new FragTraceManager(thisCallback->getImageTeraFly(), wholeBlock_axon);
			this->traceManagerPtr->finalSaveRootQ = rootQ;

			// ------- Image Enhancement -------
			if (currSettings.value("ada") == true)
			{
				this->traceManagerPtr->ada = true;
				this->traceManagerPtr->adaImgName = currSettings.value("ada_imgName").toString().toStdString();
				this->traceManagerPtr->imgEnhanceSeq.push_back(this->traceManagerPtr->adaImgName);
				this->traceManagerPtr->simpleAdaStepsize = currSettings.value("ada_stepsize").toInt();
				this->traceManagerPtr->simpleAdaRate = currSettings.value("ada_rate").toInt();
				this->traceManagerPtr->cutoffIntensity = currSettings.value("cutoffIntensity").toInt();
				if (currSettings.value("ada_saveCheck") == true)
				{
					this->traceManagerPtr->saveAdaResults = true;
					this->traceManagerPtr->simpleAdaSaveDirQ = currSettings.value("ada_savePath").toString();
					this->traceManagerPtr->simpleAdaSaveDirQ.replace(QString(" "), QString("_"));
				}
				else this->traceManagerPtr->saveAdaResults = false;
			}
			else this->traceManagerPtr->ada = false;

			if (currSettings.value("gamma") == true) this->traceManagerPtr->gammaCorrection = true;
			else this->traceManagerPtr->gammaCorrection = false;


			// ------- Mask Generation -------
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
					this->traceManagerPtr->histThreSaveDirQ.replace(QString(" "), QString("_"));
				}
				else this->traceManagerPtr->saveHistThreResults = false;
			}
			else this->traceManagerPtr->histThre = false;


			// ------- Object Filter -------
			// -- size threshold --
			if (currSettings.value("objFilter") == true)
			{
				this->traceManagerPtr->objFilter = true;
				if (currSettings.value("voxelCount") == true)
				{
					this->traceManagerPtr->voxelSize = true;
					this->traceManagerPtr->voxelCount = currSettings.value("voxelCountThre").toInt();
				}
			}
			else
			{
				this->traceManagerPtr->objFilter = false;
				this->traceManagerPtr->voxelSize = false;
				this->traceManagerPtr->actualSize = false;
			}


			// ------- Object-based MST -------
			if (currSettings.value("MST") == true)
			{
				this->traceManagerPtr->MST = true;
				this->traceManagerPtr->MSTtreeName = currSettings.value("MSTtreeName").toString().toStdString();
				this->traceManagerPtr->minNodeNum = currSettings.value("minNodeNum").toInt();
			}
			else
			{
				this->traceManagerPtr->MST = false;
			}


			// ------- Post Elongation -------
			if (currSettings.value("PostElongDistChecked") == true)
				this->paramsFromUI.insert(pair<string, float>("labeledDistThreshold", currSettings.value("PostElongDistThreshold").toFloat()));
			else
				this->paramsFromUI.insert(pair<string, float>("labeledDistThreshold", -1));
		}
	}
	
	this->connect(this, SIGNAL(switchOnSegPipe()), this->traceManagerPtr, SLOT(imgProcPipe_wholeBlock()));
	this->connect(this->traceManagerPtr, SIGNAL(emitTracedTree(NeuronTree)), this, SLOT(catchTracedTree(NeuronTree)));

	emit switchOnSegPipe(); // Qt's [emit] is equivalent to normal function call. Therefore no new thread is created due to this keyword.
	//QTimer::singleShot(0, this->traceManagerPtr, SLOT(imgProcPipe_wholeBlock()));

	this->disconnect(this, SIGNAL(switchOnSegPipe()), this->traceManagerPtr, SLOT(imgProcPipe_wholeBlock()));
	this->disconnect(this->traceManagerPtr, SIGNAL(emitTracedTree(NeuronTree)), this, SLOT(catchTracedTree(NeuronTree)));


	this->scaleTracedTree();
	NeuronTree existingTree = this->thisCallback->getSWCTeraFly();
	NeuronTree finalTree;
	if (existingTree.listNeuron.isEmpty())
	{
		NeuronStructExplorer myExplorer;
		profiledTree tracedProfiledTree(this->tracedTree);
		this->thisCallback->setSWCTeraFly(tracedProfiledTree.tree);
		this->thisCallback->redrawEditInfo(12);

		finalTree = this->tracedTree;
	}
	else
	{
		vector<NeuronTree> trees;
		NeuronTree scaledBackExistingTree = this->treeScaleBack(existingTree);
		trees.push_back(scaledBackExistingTree);
		NeuronTree scaledBackTracedTree = this->treeScaleBack(this->tracedTree);
		NeuronTree newlyTracedPart = NeuronStructUtil::swcSamePartExclusion(scaledBackTracedTree, scaledBackExistingTree, 5);
		trees.push_back(newlyTracedPart);

		profiledTree combinedProfiledTree(NeuronStructUtil::swcCombine(trees));
		profiledTree finalProfiledTree = this->traceManagerPtr->segConnectAmongTrees(combinedProfiledTree, 5);
		this->tracedTree = finalProfiledTree.tree;
		this->scaleTracedTree();
		this->thisCallback->setSWCTeraFly(this->tracedTree);
	}

	if (uiPtr->lineEdit->text() != "") writeSWC_file(uiPtr->lineEdit->text(), finalTree);
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

void FragTraceControlPanel::fillUpParamsForm()
{
	this->thisCallback->getParamsFromFragTraceUI("labeledDistThreshold", atof(uiPtr->lineEdit_4->text().toStdString().c_str()));
	this->thisCallback->getParamsFromFragTraceUI("xyResRatio", float(this->thisCallback->getImageTeraFly()->getXDim() / this->thisCallback->getImageTeraFly()->getRezX()));
	this->thisCallback->getParamsFromFragTraceUI("zResRatio", float(this->thisCallback->getImageTeraFly()->getZDim() / this->thisCallback->getImageTeraFly()->getRezZ()));
}