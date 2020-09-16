#ifndef SWCRENAMEDLG_H
#define SWCRENAMEDLG_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "v3d_interface.h"

#include "ui_renameSWC.h"

class SWC_renameDlg : public QDialog
{
	Q_OBJECT

public:
	SWC_renameDlg(QWidget* parent, V3DPluginCallback2* callback);

	QString rootPath;
	QStringList fileNameList;
	map<string, string> oldNewMap;

public slots:
	void browseFolderClicked();
	void okClicked() { this->changeName(); }
	void undoClicked();

private:
	Ui_dialog* uiPtr;
	V3DPluginCallback2* thisCallback;

	void changeName();

	vector<string> connToken;
	string getBrainID(string& inputFileName);
	string getSliceNum(string& input);
	string getXcoord(string& input);
	string getYcoord(string& input);
};

#endif