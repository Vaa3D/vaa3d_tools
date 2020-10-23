#include "FileNameChangerIndexer.h"

using namespace std;

void FileNameChangerIndexer::nameChange(const QStringList& fileNameList, FileNameChangerIndexer::nameChangingMode mode, map<string, string>* oldMapPtr)
{
	if (mode == FileNameChangerIndexer::WMU_name)
	{
		for (auto& fileNameQ : fileNameList)
		{
			QStringList splitted = fileNameQ.split(".");
			if (*(splitted.end() - 1) == "swc" || *(splitted.end() - 1) == "eswc")
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

				newFileName += ".swc";
				(*oldMapPtr)[fileNameQ.toStdString()] = newFileName;

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
}

vector<float> FileNameChangerIndexer::getCoordsFromSWC(const QList<NeuronSWC>& inputNodes)
{
	vector<float> outputVec;
	for (auto& node : inputNodes)
	{
		if (node.type == 1)
		{
			outputVec.push_back(node.z);
			outputVec.push_back(node.x);
			outputVec.push_back(node.y);
			return outputVec;
		}
	}

	return outputVec;
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

string FileNameChangerIndexer::getYcoord(string& input)
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