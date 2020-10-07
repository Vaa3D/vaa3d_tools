#include <sstream>
#include <fstream>
#include <cstdio>

#include <qtextstream.h>
#include <qstring.h>
#include <qdir.h>

#include "swcRenameDlg.h"

using namespace std;

SWC_renameDlg::SWC_renameDlg(QWidget* parent, V3DPluginCallback2* callback) : uiPtr(new Ui_dialog), thisCallback(callback), QDialog(parent)
{
	uiPtr->setupUi(this);
	
	this->connToken.push_back("_");
	this->connToken.push_back("-");
	this->connToken.push_back("-");

	this->show();
}

void SWC_renameDlg::browseFolderClicked()
{
	QObject* signalSender = sender();
	QString objName = signalSender->objectName();

	if (objName == "pushButton")
		uiPtr->lineEdit->setText(QFileDialog::getExistingDirectory(this, tr("Choose folder"), "", QFileDialog::DontUseNativeDialog));
	else if (objName == "pushButton_3")
		uiPtr->lineEdit_2->setText(QFileDialog::getExistingDirectory(this, tr("Choose folder"), "", QFileDialog::DontUseNativeDialog));
}

void SWC_renameDlg::changeName()
{
	QObject* signalSender = sender();
	QString objName = signalSender->objectName();

	if (objName == "buttonBox")
	{	
		this->rootPath = uiPtr->lineEdit->text();
		this->rootPath.replace("/", "\\");

		QDir swcFolder(this->rootPath);
		swcFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
		this->fileNameList.clear();
		this->fileNameList = swcFolder.entryList();

		this->oldNewMap.clear();
		for (auto& fileNameQ : this->fileNameList)
		{
			QStringList splitted = fileNameQ.split(".");
			if (*(splitted.end() - 1) == "swc" || *(splitted.end() - 1) == "eswc")
			{
				string newFileName;
				string fileName = fileNameQ.toStdString();
				this->oldNewMap.insert({ fileName, "" });

				string brainID = this->getBrainID(fileName);
				newFileName = brainID + connToken.at(0);

				string sliceNum = this->getSliceNum(fileName);
				newFileName = newFileName + sliceNum + connToken.at(1);

				string xCoord = this->getXcoord(fileName);
				newFileName = newFileName + xCoord + connToken.at(2);

				string yCoord = this->getYcoord(fileName);
				newFileName += yCoord;

				newFileName += ".swc";
				this->oldNewMap[fileNameQ.toStdString()] = newFileName;

				string oldName = fileNameQ.toStdString();
				cout << oldName << endl << newFileName << endl;

				QString oldNameQ = this->rootPath + "\\" + fileNameQ;
				QString newNameQ = this->rootPath + "\\" + QString::fromStdString(newFileName);
				//const char* oldNameC = (this->rootPath + "\\" + fileNameQ).toStdString().c_str();
				//const char* newNameC = ((this->rootPath + "\\").toStdString() + newFileName).c_str();
				if (!QFile::rename(oldNameQ, newNameQ))
				{
					if (oldNameQ != newNameQ)
					{
						cout << " -> QFile::rename falied, using system command instead:" << endl;

						string command = "rename " + oldNameQ.toStdString() + " " + newFileName;
						const char* commandC = command.c_str();
						cout << commandC << endl;
						int sysTry = system(commandC);
						if (sysTry)
						{
							QString newNameRootQ = newNameQ;
							int dupCount = 0;
							while (1)
							{
								++dupCount;
								newNameQ = newNameRootQ + "_duplicated" + QString::number(dupCount) + ".swc";
								if (QFile::rename(oldNameQ, newNameQ)) break;
							}
							//string commandDup = command + "_duplicated.swc";
							//const char* commandDupC = commandDup.c_str();
							//cout << system(commandDupC) << endl;
						}
					}
				}
				cout << endl;
			}
		}
	}
	else if (objName == "buttonBox_2")
	{
		this->rootPath = uiPtr->lineEdit_2->text();
		this->rootPath.replace("/", "\\");

		QDir seuFileFolder(this->rootPath);
		seuFileFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
		this->fileNameList.clear();
		this->fileNameList = seuFileFolder.entryList();

		QString newFolderName = this->rootPath + "\\renamed_indexed";
		QDir newDir(newFolderName);
		if (!newDir.exists()) newDir.mkpath(".");

		string indexFileName = this->rootPath.toStdString() + "\\formalName_SEU_map.csv";
		ofstream indexOut(indexFileName);

		this->seuCellMap.clear();
		for (auto& fileNameQ : this->fileNameList)
		{
			//qDebug() << fileNameQ;
			QStringList splitNameDot = fileNameQ.split(".");
			if (*(splitNameDot.end() - 1) == "swc" || *(splitNameDot.end() - 1) == "ano" || *(splitNameDot.end() - 1) == "apo")
			{
				QStringList splitNameUnderscore = splitNameDot.at(0).split("_");
				if (this->seuCellMap.find(splitNameUnderscore.at(0).toStdString()) == this->seuCellMap.end())
				{
					set<string> seuNums = { splitNameUnderscore.at(1).toStdString() };
					this->seuCellMap.insert({ splitNameUnderscore.at(0).toStdString(), seuNums });
				}
				else this->seuCellMap[splitNameUnderscore.at(0).toStdString()].insert(splitNameUnderscore.at(1).toStdString());
			}
		}

		for (auto& brainID : this->seuCellMap)
		{
			for (auto& seuCellID : brainID.second)
			{
				string inputFileBaseName = this->rootPath.toStdString() + "\\" + brainID.first + "_" + seuCellID;
				string inputSEUname = brainID.first + "_" + seuCellID;
				cout << inputSEUname << " -> ";
				string newFileBaseName, xCoord, yCoord, zCoord;
				QStringList apoLineSplit;
					
				string inputApoFileName = inputFileBaseName + ".apo";
				ifstream inputApoFile(inputApoFileName);
				if (inputApoFile.is_open())
				{
					string apoHeader, line;
					while (getline(inputApoFile, line))
					{
						if (line[0] == '#')
						{
							apoHeader = line;
							continue;
						}
						
						QString lineQ = QString::fromStdString(line);
						apoLineSplit = lineQ.split(",");
						zCoord = to_string(int(apoLineSplit.at(4).toFloat()));
						xCoord = to_string(int(apoLineSplit.at(5).toFloat()));
						yCoord = to_string(int(apoLineSplit.at(6).toFloat()));
						newFileBaseName = newFolderName.toStdString() + "\\" + brainID.first + "_" + zCoord + "-X" + xCoord + "-Y" + yCoord;
						string formalName = brainID.first + "_" + zCoord + "-X" + xCoord + "-Y" + yCoord;
						cout << formalName << endl;
						indexOut << inputSEUname << "," << formalName << endl;
						break;
					}
					QString newApoNameQ = QString::fromStdString(newFileBaseName) + ".apo";
					QFile apoFile;
					apoFile.setFileName(newApoNameQ);
					apoFile.open(QIODevice::ReadWrite);
					QTextStream apoOut(&apoFile);
					apoOut << QString::fromStdString(apoHeader) << "\n";
					apoOut << apoLineSplit.at(0) << "," << QString::fromStdString(seuCellID) << ",";
					for (QStringList::iterator it = apoLineSplit.begin() + 2; it != apoLineSplit.end() - 1; ++it) apoOut << *it << ",";
					apoOut << *(apoLineSplit.end() - 1) << "\n";
					apoFile.close();
					
				}
				inputApoFile.close();

				string inputAnoFileName = inputFileBaseName + ".ano";
				ifstream inputAno(inputAnoFileName);
				if (inputAno.is_open())
				{
					QString newAnoNameQ = QString::fromStdString(newFileBaseName) + ".ano";
					QFile anoFile;
					anoFile.setFileName(newAnoNameQ);
					anoFile.open(QIODevice::ReadWrite);
					QTextStream anoOut(&anoFile);
					QString swcNameQ = QString::fromStdString(brainID.first) + "_" + QString::fromStdString(zCoord) + "-X" + QString::fromStdString(xCoord) + "-Y" + QString::fromStdString(yCoord) + ".swc";
					QString apoNameQ = QString::fromStdString(brainID.first) + "_" + QString::fromStdString(zCoord) + "-X" + QString::fromStdString(xCoord) + "-Y" + QString::fromStdString(yCoord) + ".apo";
					anoOut << "SWCFILE=" << swcNameQ << "\nAPOFILE=" << apoNameQ << "\n";
					anoFile.close();
				}
				inputAno.close();

				QString seuSWCnameQ = QString::fromStdString(inputFileBaseName) + ".swc";
				QString formalSWCnameQ = QString::fromStdString(newFileBaseName) + ".swc";
				if (!QFile::copy(seuSWCnameQ, formalSWCnameQ)) cout << seuSWCnameQ.toStdString() << " not found.";
			}
		}
		indexOut.close();
	}
}

void SWC_renameDlg::undoClicked()
{
	QString lineText = uiPtr->lineEdit->text();
	lineText.replace("/", "\\");

	if (this->oldNewMap.empty())
	{
		v3d_msg("No file names have been changed in this folder yet.");
		return;
	}
	else if (lineText != this->rootPath)
	{
		v3d_msg("You have changed to a different folder.");
		return;
	}

	for (auto& newNamePair : this->oldNewMap)
	{
		string oldName = this->rootPath.toStdString() + "/" + newNamePair.first;
		const char* oldNameC = oldName.c_str();
		string newName = this->rootPath.toStdString() + "/" + newNamePair.second;
		const char* newNameC = newName.c_str();

		rename(newNameC, oldNameC);
		cout << newNamePair.second << " -> " << newNamePair.first << endl;
		this->oldNewMap[newNamePair.first] = "";
	}
}

string SWC_renameDlg::getBrainID(string& inputFileName)
{
	string outputBrainID;
	for (int i = 0; i < inputFileName.size(); ++i)
	{
		try
		{
			//cout << inputFileName[i] << " ";
			stoi(string(1, inputFileName[i]));
		}
		catch (std::invalid_argument)
		{
			//cout << "Reach the end of brain ID digit -- " << inputFileName[i] << endl;
			inputFileName = inputFileName.substr(i + 1, inputFileName.size() - i - 1);
			return outputBrainID;
		}
		outputBrainID += inputFileName[i];
	}
}

string SWC_renameDlg::getSliceNum(string& input)
{
	if (input[0] == 'Z') input = input.substr(1, input.size() - 1);

	string outputSliceNum;
	for (int i = 0; i < input.size(); ++i)
	{
		try
		{
			int digit = stoi(string(1, input[i]));
			if (i == 0 && digit == 0) continue;
		}
		catch (std::invalid_argument)
		{
			//cout << "Reach the end of slice number digit -- " << input[i] << endl;
			input = input.substr(i + 1, input.size() - i - 1);
			return outputSliceNum;
		}
		outputSliceNum += input[i];
	}
}

string SWC_renameDlg::getXcoord(string& input)
{
	string outputXcoord;
	outputXcoord += "X";

	//int startDigit = 0;
	for (int i = 0; i < input.size(); ++i)
	{
		if (input[i] == 'x' || input[i] == 'X')
		{
			input = input.substr(i + 1, input.size() - i - 1);
			//startDigit = i + 1;
			break;
		}
	}

	for (int i = 0; i < input.size(); ++i)
	{
		try
		{
			stoi(string(1, input[i]));
		}
		catch (std::invalid_argument)
		{
			//cout << "Reach the end of X coord digit -- " << input[i] << endl;
			input = input.substr(i + 1, input.size() - i - 1);
			return outputXcoord;
		}
		outputXcoord += input[i];
	}
}

string SWC_renameDlg::getYcoord(string& input)
{
	string outputYcoord;
	outputYcoord += "Y";

	//int startDigit = 0;
	for (int i = 0; i < input.size(); ++i)
	{
		
		if (input[i] == 'y' || input[i] == 'Y')
		{
			input = input.substr(i + 1, input.size() - i - 1);
			//startDigit = i + 1;
			break;
		}
	}

	for (int i = 0; i < input.size(); ++i)
	{
		try
		{
			stoi(string(1, input[i]));
		}
		catch (std::invalid_argument)
		{
			//cout << "Reach the end of Y coord digit -- " << input[i] << endl;
			input = input.substr(i + 1, input.size() - i - 1);
			return outputYcoord;
		}
		outputYcoord += input[i];
	}
}