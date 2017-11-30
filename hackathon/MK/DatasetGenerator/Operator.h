#ifndef OPERATOR_H
#define OPERATOR_H

#include <QtGui>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include <queue>

using namespace std;

enum listOpType { newList, merge, divide, subset, crossVal };
enum patchOpType { stackTo3D, stackTo2D, teraTo3D, teraTo2D, patch3DTo3D, patch3DTo2D, patch2DTo2D };
enum dim2D { xy, xz, yz };
enum opSequence { Crop, MIP };

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
	vector<opSequence> opSeq;
	dim2D dimSelection;
	int sideX, sideY, sideZ;

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

	void pick_save();

	void createListFromList(listOpType listOp);

	NeuronTree cropSWCfile3D(NeuronTree nt, int xb, int xe, int yb, int ye, int zb, int ze, int type);
	void cropStack(unsigned char InputImagePtr[], unsigned char OutputImagePtr[],
		int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ);
	void mipStack(unsigned char InputImagePtr[], unsigned char OutputImagePtr[],
		int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ);

	void create2DPatches(patchOpType patchOp);

	void taskQueuDispatcher();

private:
	V3DPluginCallback2* OperatorCallback;
	queue<taskFromUI> taskQueu;
	taskFromUI operatingTask;

	typedef void (Operator::* opPtr)(unsigned char InputImagePtr[], unsigned char OutputImagePtr[],
		int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ);
};



#endif
