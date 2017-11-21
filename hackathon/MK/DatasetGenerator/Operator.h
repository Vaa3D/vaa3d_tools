#ifndef OPERATOR_H
#define OPERATPR_H

#include <QtGui>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include <queue>

using namespace std;

enum listOpType { newList, merge, divide, subset, crossVal };
enum patchOpType { stackTo3D, stackTo2D, teraTo3D, teraTo2D, patch3DTo3D, patch3DTo2D, patch2DTo2D };

struct taskFromUI
{
	bool createPatch;
	bool createList;
	bool createPatchNList;

	listOpType listOp;
	double subsetRatio;
	int foldNum;
	int classNum;

	patchOpType patchOp;

	string source;
	string outputFileName;
	string outputDirName;
	string neuronStrucFileName;
};

class Operator : public QObject
{

	Q_OBJECT
	friend class DatasetGeneratorUI;

signals:
	void progressBarReporter(QString taskName, int percentage);

public:
	QString inputSWCdir;
	QString inputImagedir;
	QString inputBkgDir;
	QVector<QString> imageFolders;
	QString outputImagedir;
	void getImageFolders();

	double valProportion;
	double proportion1;
	double proportion2;
	void createListFromList(listOpType listOp);


	void shapeFilter_line_dot();

	void pick_save();

	void taskQueuDispatcher();

private:
	queue<taskFromUI> taskQueu;
	taskFromUI operatingTask;
};



#endif
