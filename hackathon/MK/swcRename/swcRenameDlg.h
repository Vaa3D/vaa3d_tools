#ifndef SWCRENAMEDLG_H
#define SWCRENAMEDLG_H

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "v3d_interface.h"

#include "NeuronReconFileManager_Define.h"
#include "ui_renameSWC.h"
#include "FileNameChangerIndexer.h"
#include "ReconOperator.h"

class SWC_renameDlg : public QDialog
{
	Q_OBJECT

public:
	SWC_renameDlg(QWidget* parent, V3DPluginCallback2* callback);

	QString rootPath;
	QStringList fileNameList;
	map<string, string> oldNewMap;
	map<string, set<string>> seuCellMap;

public slots:
	void browseFolderClicked();
	void preProcessParam(bool toggle);
	void okClicked() { this->changeName(); }
	void okClicked2() { this->reconOp(); }
	void okClicked3() { this->fileConversion(); }
	void undoClicked();

private:
	Ui_dialog* uiPtr;
	V3DPluginCallback2* thisCallback;

	FileNameChangerIndexer fileManager;
	ReconOperator myOperator;

	void changeName();
	void reconOp();
	void fileConversion();

	vector<string> connToken;
	string getBrainID(string& inputFileName);
	string getSliceNum(string& input);
	string getXcoord(string& input);
	string getYcoord(string& input);
	vector<float> getCoordsFromSWC(const QList<NeuronSWC>& inputNodes);
};

#endif