#ifndef FILENAMECHANGERINDEXER_H
#define FILENAMECHANGERINDEXER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <qstring.h>
#include <qstringlist.h>
#include <qdir.h>

#include "basic_surf_objs.h"

using namespace std;

class FileNameChangerIndexer
{
public:
	enum nameChangingMode { WMU_name, WMU_index, SEU_index };

	bool WMUapoAno, generalApoANO;
	vector<string> connToken;
	QString rootPath;
	QString fileNameAddition;
	QString mappingTableFullName;
	map<string, set<string>> seuCellMap;
	void nameChange(const QStringList& fileNameList, nameChangingMode mode, map<string, string>* oldMapPtr = nullptr);
	void nameChange_fromAssemble(const QStringList& fileNameList);

private:
	string getBrainID(string& inputFileName);
	string getSliceNum(string& input);
	string getXcoord(string& input);
	string getYcoord(string& input);
	string getFileNameTail(const string& input);
	bool getCoordsFromSWC(const QList<NeuronSWC>& inputNodes, vector<float>& somaCoords);
	void SEUnameChange(const map<string, set<string>>& seuNameMap, bool table);
};

#endif