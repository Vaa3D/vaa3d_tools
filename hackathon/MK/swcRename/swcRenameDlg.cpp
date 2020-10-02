#include <sstream>
#include <cstdio>

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
	uiPtr->lineEdit->setText(QFileDialog::getExistingDirectory(this, tr("Choose folder"), "", QFileDialog::DontUseNativeDialog));
}

void SWC_renameDlg::changeName()
{
	QString Vaa3DDir = QDir::currentPath();
	
	this->fileNameList.clear();
	this->rootPath = uiPtr->lineEdit->text();
	this->rootPath.replace("/", "\\");

	QDir swcFolder(this->rootPath);
	swcFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
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
					cout << command << endl;
					const char* commandC = command.c_str();
					if (system(commandC))
					{
						string commandDup = command + "_duplicated.swc";
						const char* commandDupC = commandDup.c_str();
						system(commandDupC);
					}
				}
			}
			cout << endl;
		}
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

	for (int i = 0; i < input.size(); ++i)
	{
		if (!string(1, input[i]).compare("x") || !string(1, input[i]).compare("X"))
		{
			input = input.substr(i, input.size() - i - 1);
			break;
		}
	}

	for (int i = 1; i < input.size(); ++i)
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

	for (int i = 1; i < input.size(); ++i)
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