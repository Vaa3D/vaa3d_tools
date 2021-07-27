#include <fstream>
#include <set>

#include "FileNameChangerIndexer.h"

using namespace std;

void FileNameChangerIndexer::nameChange_fromAssemble(const QStringList& fileNameList)
{
	for (auto& fileNameQ : fileNameList)
	{
		QString baseNameQ = fileNameQ.left(fileNameQ.length() - 4);

		QString oldFullNameQ = this->rootPath + "\\" + fileNameQ;
		NeuronTree inputTree = readSWC_file(oldFullNameQ);

		string brainID = this->getBrainID(fileNameQ.toStdString());
		string newFileName = brainID + connToken.at(0);

		vector<float> somaCoords;
		this->getCoordsFromSWC(inputTree.listNeuron, somaCoords);
		newFileName = newFileName + to_string(int(somaCoords.at(0))) + connToken.at(1) + "X" + to_string(int(somaCoords.at(1))) + connToken.at(2) + "Y" + to_string(int(somaCoords.at(2)));
		QString newFullFileNameQ = this->rootPath + "\\" + QString::fromStdString(newFileName) + ".swc";
		QFile::rename(oldFullNameQ, newFullFileNameQ);

		if (this->generalApoANO)
		{
			CellAPO somaMarker;
			RGBA8 color;
			color.r = 0;
			color.g = 0;
			color.b = 0;
			somaMarker.x = somaCoords.at(1);
			somaMarker.y = somaCoords.at(2);
			somaMarker.z = somaCoords.at(0);
			somaMarker.color = color;
			somaMarker.volsize = 500;
			QList<CellAPO> somaList;
			somaList.push_back(somaMarker);
			QString fullAPOnameQ = this->rootPath + "\\" + QString::fromStdString(newFileName) + ".apo";
			writeAPO_file(fullAPOnameQ, somaList);
			
			QString outputANOfullNameQ = this->rootPath + "\\" + QString::fromStdString(newFileName) + ".ano";
			QFile anoFile;
			anoFile.setFileName(outputANOfullNameQ);
			anoFile.open(QIODevice::ReadWrite);
			QTextStream anoOut(&anoFile);
			QString swcNameQ = QString::fromStdString(newFileName) + ".swc";
			QString apoNameQ = QString::fromStdString(newFileName) + ".apo";
			anoOut << "SWCFILE=" << swcNameQ << "\nAPOFILE=" << apoNameQ << "\n";
			anoFile.close();
		}
	}
}

void FileNameChangerIndexer::nameChange(const QStringList& fileNameList, FileNameChangerIndexer::nameChangingMode mode, map<string, string>* oldMapPtr)
{
	if (mode == FileNameChangerIndexer::WMU_name)
	{
		QString newSavingPathQ = this->rootPath + "\\newFilesWithFormalizedNames";
		QDir newDir(newSavingPathQ);
		if (!newDir.exists()) newDir.mkpath(".");

		QString unProcessedSavingPathQ = this->rootPath + "\\noSomaOrMoreThan1Soma";
		QDir unProcessedDir(unProcessedSavingPathQ);		
		
		string indexFileName = this->rootPath.toStdString() + "\\formalName_WMU_map.csv";
		ofstream indexOut(indexFileName);
		indexOut << "Original Name" << "," << "Formatted Name" << "," << "Accurate Name" << endl;

		for (auto& fileNameQ : fileNameList)
		{
			if (fileNameQ.endsWith("swc") || fileNameQ.endsWith("eswc"))
			{ 
				string newFileName;
				string fileName = fileNameQ.toStdString();
				oldMapPtr->insert({ fileName, "" });

				string brainID = this->getBrainID(fileName);
				newFileName = brainID + connToken.at(0);

				string sliceNum = this->getSliceNum(fileName);
				newFileName = newFileName + sliceNum + connToken.at(1);

				string xCoord = this->getXcoord(fileName);
				newFileName = newFileName + xCoord + connToken.at(2);

				string yCoord = this->getYcoord(fileName);
				newFileName += yCoord;

				string tailInfo = this->getFileNameTail(fileName);
				if (!tailInfo.empty()) newFileName = newFileName + "_" + tailInfo;

				if (!this->fileNameAddition.isEmpty()) newFileName = newFileName + "_" + this->fileNameAddition.toStdString();

				(*oldMapPtr)[fileNameQ.toStdString()] = newFileName;

				string oldName = fileNameQ.toStdString();
				cout << oldName << endl << newFileName << endl << endl;

				QString oldFullNameQ = this->rootPath + "\\" + fileNameQ;
				QString newFullBaseNameQ = newSavingPathQ + "\\" + QString::fromStdString(newFileName);
				//QString newFullBaseRenameQ = this->rootPath + "\\" + QString::fromStdString(newFileName);
				QString newFullNameQ = newFullBaseNameQ + ".swc";
				//QString newFullRenameQ = newFullBaseRenameQ + ".swc";
				
				NeuronTree inputTree = readSWC_file(oldFullNameQ);
				vector<float> somaCoords;
				if (!this->getCoordsFromSWC(inputTree.listNeuron, somaCoords))
				{
					/*if (!unProcessedDir.exists()) unProcessedDir.mkpath(".");
					QString newUnprocessdFullName = unProcessedSavingPathQ + "\\" + fileNameQ;
					QFile::copy(oldFullNameQ, newUnprocessdFullName);
					continue;*/
				}
				
				int dupCount = 0;
				while (1)
				{
					if (QFile::exists(newFullNameQ))
					{
						++dupCount;
						newFullNameQ = newFullBaseNameQ + "_duplicated" + QString::number(dupCount) + ".swc";
					}
					else
					{
						QFile::copy(oldFullNameQ, newFullNameQ);
						if (indexOut.is_open())
						{
							string accurateName = brainID + "_" + to_string(int(somaCoords.at(0))) + "-X" + to_string(int(somaCoords.at(1))) + "-Y" + to_string(int(somaCoords.at(2)));
							if (fileNameQ.endsWith("swc"))
							{
								string oldNameNoExt = fileNameQ.left(fileNameQ.length() - 4).toStdString();
								indexOut << oldNameNoExt << "," << newFileName << "," << accurateName << endl;
							}
							else if (fileNameQ.endsWith("eswc"))
							{
								string oldNameNoExt = fileNameQ.left(fileNameQ.length() - 5).toStdString();
								indexOut << oldNameNoExt << "," << newFileName << "," << accurateName << endl;
							}
						}
						break;
					}
				}
				
				if (this->WMUapoAno)
				{
					CellAPO somaMarker;
					RGBA8 color;
					color.r = 0;
					color.g = 0;
					color.b = 0;
					somaMarker.x = somaCoords.at(1);
					somaMarker.y = somaCoords.at(2);
					somaMarker.z = somaCoords.at(0);
					somaMarker.color = color;
					somaMarker.volsize = 500;
					QList<CellAPO> somaList;
					somaList.push_back(somaMarker);
					QString fullAPOnameQ = newFullBaseNameQ + ".apo";
					//QString fullAPOrenameQ = newFullBaseRenameQ + ".apo";
					writeAPO_file(fullAPOnameQ, somaList);
					//writeAPO_file(fullAPOrenameQ, somaList);

					QString outputANOfullNameQ = newFullBaseNameQ + ".ano";
					//QString outputANOfullRenameQ = newFullBaseRenameQ + ".ano";
					QFile anoFile;
					anoFile.setFileName(outputANOfullNameQ);
					//anoFile.setFileName(outputANOfullRenameQ);
					anoFile.open(QIODevice::ReadWrite);
					QTextStream anoOut(&anoFile);
					QString swcNameQ = QString::fromStdString(newFileName) + ".swc";
					QString apoNameQ = QString::fromStdString(newFileName) + ".apo";
					anoOut << "SWCFILE=" << swcNameQ << "\nAPOFILE=" << apoNameQ << "\n";
					anoFile.close();
				}
			}
		}
		if (indexOut.is_open()) indexOut.close();
	}
	else if (mode == FileNameChangerIndexer::SEU_index)
	{
		this->seuCellMap.clear();
		for (auto& fileNameQ : fileNameList)
		{
			if (fileNameQ.endsWith("swc") || fileNameQ.endsWith("eswc"))
			{
				QStringList splitNameUnderscore = fileNameQ.split("_");
				string seuNum;
				for (QStringList::iterator it = splitNameUnderscore.begin() + 1; it != splitNameUnderscore.end() - 1; ++it) seuNum = seuNum + (*it).toStdString() + "_";
				seuNum += (*(splitNameUnderscore.end() - 1)).toStdString();
				if (this->seuCellMap.find(splitNameUnderscore.at(0).toStdString()) == this->seuCellMap.end())
				{
					set<string> seuNums = { seuNum };
					this->seuCellMap.insert({ splitNameUnderscore.at(0).toStdString(), seuNums });
				}
				else this->seuCellMap[splitNameUnderscore.at(0).toStdString()].insert(splitNameUnderscore.at(1).toStdString());
			}
		}

		if (this->mappingTableFullName.isEmpty()) this->SEUnameChange(this->seuCellMap, false);
		else this->SEUnameChange(this->seuCellMap, true);
	}
}

bool FileNameChangerIndexer::getCoordsFromSWC(const QList<NeuronSWC>& inputNodes, vector<float>& somaCoords)
{
	somaCoords.clear();
	for (auto& node : inputNodes)
	{
		if (node.parent == -1 && node.type == 1)
		{
			somaCoords.push_back(node.z);
			somaCoords.push_back(node.x);
			somaCoords.push_back(node.y);
		}
	}

	if (somaCoords.size() != 3) return false;
	else return true;
}

string FileNameChangerIndexer::getBrainID(string& inputFileName)
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

string FileNameChangerIndexer::getSliceNum(string& input)
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

string FileNameChangerIndexer::getXcoord(string& input)
{
	string outputXcoord = "X";

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

string FileNameChangerIndexer::getYcoord(string& input)
{
	string outputYcoord = "Y";

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

string FileNameChangerIndexer::getFileNameTail(const string& input)
{
	string downSampleInfo;
	
	string::size_type xyLoc, zLoc, regLoc;
	xyLoc = input.find("xy");
	zLoc = input.find("z");
	regLoc = input.find("reg");

	if (xyLoc != string::npos && zLoc != string::npos)
	{
		if (zLoc == xyLoc + 4) downSampleInfo = input.substr(xyLoc, 6);
	}
	else if (regLoc != string::npos) downSampleInfo = input.substr(regLoc, 3);

	return downSampleInfo;
}

void FileNameChangerIndexer::SEUnameChange(const map<string, set<string>>& seuNameMap, bool table)
{
	if (!table)
	{
		QString newSavingPathQ = this->rootPath + "\\newFilesWithFormalizedNames";
		QDir newDir(newSavingPathQ);
		if (!newDir.exists()) newDir.mkpath(".");

		QString unProcessedSavingPathQ = this->rootPath + "\\noSomaOrMoreThan1Soma";
		QDir unProcessedDir(unProcessedSavingPathQ);

		string indexFileName = this->rootPath.toStdString() + "\\formalName_SEU_map.csv";
		ofstream indexOut(indexFileName);

		for (auto& brainID : this->seuCellMap)
		{
			for (auto& seuCellID : brainID.second)
			{
				QString swcFullNameQ = this->rootPath + "\\" + QString::fromStdString(brainID.first) + "_" + QString::fromStdString(seuCellID);
				NeuronTree inputTree = readSWC_file(swcFullNameQ);
				vector<float> somaCoords;
				if (!getCoordsFromSWC(inputTree.listNeuron, somaCoords))
				{
					if (!unProcessedDir.exists()) unProcessedDir.mkpath(".");
					QString newUnprocessdFullName = unProcessedSavingPathQ + "\\" + QString::fromStdString(brainID.first) + "_" + QString::fromStdString(seuCellID);
					QFile::copy(swcFullNameQ, newUnprocessdFullName);
					continue;
				}
				QString newFullBaseNameQ = newSavingPathQ + "\\" + QString::fromStdString(brainID.first) + "_" + QString::number(int(somaCoords.at(0))) + "-X" + QString::number(int(somaCoords.at(1))) + "-Y" + QString::number(int(somaCoords.at(2)));
				//QString newFullBaseNameQ = newSavingPathQ + "\\" + QString::fromStdString(brainID.first) + "_" + QString::number(int(round(somaCoords.at(0)))) + "-X" + QString::number(int(round(somaCoords.at(1)))) + "-Y" + QString::number(int(round(somaCoords.at(2))));
				string tailInfo = this->getFileNameTail(seuCellID);
				if (!tailInfo.empty()) newFullBaseNameQ = newFullBaseNameQ + "_" + QString::fromStdString(tailInfo);
				if (!this->fileNameAddition.isEmpty()) newFullBaseNameQ = newFullBaseNameQ + "_" + this->fileNameAddition;
				QString newSWCfullNameQ = newFullBaseNameQ + ".swc";

				if (!QFile::copy(swcFullNameQ, newSWCfullNameQ)) cout << newSWCfullNameQ.toStdString() << " not found." << endl;
				string inputSEUname;
				if (seuCellID.find("eswc") != string::npos) inputSEUname = brainID.first + "_" + seuCellID.substr(0, seuCellID.length() - 5);
				else inputSEUname = brainID.first + "_" + seuCellID.substr(0, seuCellID.length() - 4);
				string outputBaseName = brainID.first + "_" + to_string(int(somaCoords.at(0))) + "-X" + to_string(int(somaCoords.at(1))) + "-Y" + to_string(int(somaCoords.at(2)));
				//string outputBaseName = brainID.first + "_" + to_string(int(round(somaCoords.at(0)))) + "-X" + to_string(int(round(somaCoords.at(1)))) + "-Y" + to_string(int(round(somaCoords.at(2))));
				if (!tailInfo.empty()) outputBaseName = outputBaseName + "_" + tailInfo;
				if (!this->fileNameAddition.isEmpty()) outputBaseName = outputBaseName + "_" + this->fileNameAddition.toStdString();
				if (indexOut.is_open()) indexOut << inputSEUname << "," << outputBaseName << endl;
				cout << inputSEUname << " -> " << outputBaseName << endl;

				CellAPO somaMarker;
				RGBA8 color;
				color.r = 0;
				color.g = 0;
				color.b = 0;
				somaMarker.x = somaCoords.at(1);
				somaMarker.y = somaCoords.at(2);
				somaMarker.z = somaCoords.at(0);
				somaMarker.color = color;
				somaMarker.volsize = 500;
				QList<CellAPO> somaList;
				somaList.push_back(somaMarker);
				QString outputAPOfullNameQ = newFullBaseNameQ + ".apo";
				writeAPO_file(outputAPOfullNameQ, somaList);

				QString outputANOfullNameQ = newFullBaseNameQ + ".ano";
				QFile anoFile;
				anoFile.setFileName(outputANOfullNameQ);
				anoFile.open(QIODevice::ReadWrite);
				QTextStream anoOut(&anoFile);
				QString swcNameQ = QString::fromStdString(outputBaseName) + ".swc";
				QString apoNameQ = QString::fromStdString(outputBaseName) + ".apo";
				anoOut << "SWCFILE=" << swcNameQ << "\nAPOFILE=" << apoNameQ << "\n";
				anoFile.close();
			}
		}

		if (indexOut.is_open()) indexOut.close();
	}
	else
	{
		QString newSavingPathQ = this->rootPath + "\\newFilesWithFormalizedNames";
		QDir newDir(newSavingPathQ);
		if (!newDir.exists()) newDir.mkpath(".");
		
		QString renamedFilesSavingPathQ = this->rootPath + "\\files_renamed";
		QDir renamedDir(renamedFilesSavingPathQ);
		if (!renamedDir.exists()) renamedDir.mkpath(".");
		
		ifstream mappingTableIn(this->mappingTableFullName.toStdString());
		string line;
		if (mappingTableIn.is_open())
		{
			QDir seuFileFolder(this->rootPath);
			seuFileFolder.setFilter(QDir::Files | QDir::NoDotAndDotDot);
			QStringList fileNameList = seuFileFolder.entryList();

			while (getline(mappingTableIn, line))
			{
				QString lineQ = QString::fromStdString(line);
				QStringList lineSplitQ = lineQ.split(",");
				cout << lineSplitQ.at(0).toStdString() << " " << lineSplitQ.at(1).toStdString() << endl;

				QString SEUname = lineSplitQ.at(0).split(".").at(0);
				QString inputFileFullNameQ;
				for (auto& fileName : fileNameList)
				{
					if (fileName.contains(SEUname))
					{
						inputFileFullNameQ = this->rootPath + "\\" + fileName;
						break;
					}
				}
				//QString inputFileFullNameQ = this->rootPath + "\\" + lineSplitQ.at(0) + ".swc";
				QString outputFileFullNameQ = newSavingPathQ + "\\" + lineSplitQ.at(1);

				if (this->fileNameAddition.isEmpty()) outputFileFullNameQ += ".swc";
				else outputFileFullNameQ = outputFileFullNameQ + "_" + this->fileNameAddition + ".swc";
				if (QFile::exists(inputFileFullNameQ)) QFile::copy(inputFileFullNameQ, outputFileFullNameQ);

				QString renamedFullNameQ = renamedFilesSavingPathQ + "\\" + lineSplitQ.at(0) + ".swc";
				QFile::rename(inputFileFullNameQ, renamedFullNameQ);
			}
		}
	}
}