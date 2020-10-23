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

	vector<string> connToken;
	QString rootPath;
	void nameChange(const QStringList& fileNameList, nameChangingMode mode, map<string, string>* oldMapPtr = nullptr);

private:
	string getBrainID(string& inputFileName);
	string getSliceNum(string& input);
	string getXcoord(string& input);
	string getYcoord(string& input);
	vector<float> getCoordsFromSWC(const QList<NeuronSWC>& inputNodes);
};