#include <sstream>
#include <cstdio>

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
	this->fileNameList.clear();

	this->rootPath = QFileDialog::getExistingDirectory(this, tr("Choose folder"), "", QFileDialog::DontUseNativeDialog);
	QDir swcFolder(this->rootPath);
	swcFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
	this->fileNameList = swcFolder.entryList();
	uiPtr->lineEdit->setText(this->rootPath);
}

void SWC_renameDlg::changeName()
{
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

			//if (*(splitted.end() - 1) == "swc") newFileName += ".swc";
			//else if (*(splitted.end() - 1) == "eswc") newFileName += ".eswc";
			newFileName += ".swc";
			this->oldNewMap[fileNameQ.toStdString()] = newFileName;
			//cout << "new file Name: " << newFileName << endl << endl;

			const char* oldNameC = (this->rootPath + "\\" + fileNameQ).toStdString().c_str();
			const char* newNameC = ((this->rootPath + "\\").toStdString() + newFileName).c_str();
			if (!rename(oldNameC, newNameC))
				cout << fileNameQ.toStdString() << " -> " << newFileName << endl;
			else
			{
				perror("Error");
				cout << fileNameQ.toStdString() << " file name change failed." << endl;
			}
		}
	}
}

void SWC_renameDlg::undoClicked()
{
	if (this->oldNewMap.empty())
	{
		v3d_msg("No file names have been changed in this folder yet.");
		return;
	}
	else if (uiPtr->lineEdit->text() != this->rootPath)
	{
		v3d_msg("You have changed to a different folder.");
		return;
	}

	for (auto& newNamePair : this->oldNewMap)
	{
		string oldName = this->rootPath.toStdString() + "\\" + newNamePair.first;
		const char* oldNameC = oldName.c_str();
		string newName = this->rootPath.toStdString() + "\\" + newNamePair.second;
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
			stoi(string(1, input[i]));
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

	for (int i = 1; i < input.size(); ++i)
	{
		try
		{
			stoi(string(i, input[i]));
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
			stoi(string(i, input[i]));
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