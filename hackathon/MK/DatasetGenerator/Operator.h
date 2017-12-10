#ifndef OPERATOR_H
#define OPERATOR_H

#include <queue>

#include <QtGui>

#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "VirtualVolume.h"
#include "NeuronStructNavigator.h"

using namespace std;
using namespace iim;

enum listOpType { newList, merge, divide, subset, crossVal };
enum patchOpType { stackTo3D, stackTo2D, teraTo3D, teraTo2D, patch3DTo3D, patch3DTo2D, patch2DTo2D };
enum dim2D { xy, xz, yz };
enum opSequence { Crop, MIP, mIP };

struct taskFromUI
{
	bool createPatch;
	bool createList;

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
	void taskQueuDispatcher(); // The task queue sent from UI is handled here. This method addresses each individual task accordingly.

	void createListFromList(listOpType listOp);
	void createListFromPatch(double ratio);

	// ------------------- Patches generation ----------------------- 
	void create2DPatches(patchOpType patchOp);
	void create3DPatches(patchOpType patchOp);

	typedef void (Operator::* opPtr)(unsigned char InputImagePtr[], unsigned char VOIPtr[], unsigned char ROIPtr[],
		int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ);
	void funcSequencer(vector<opSequence> seq);

	NeuronTree cropSWCfile3D(NeuronTree nt, int xb, int xe, int yb, int ye, int zb, int ze, int type);
	void cropStack(unsigned char InputImagePtr[], unsigned char VOIPtr[], unsigned char dummiePtr[],
		int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ);
	void maxIPStack(unsigned char InputImagePtr[], unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[],
		int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ);
	void minIPStack(unsigned char InputImagePtr[], unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[],
		int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ);

	VirtualVolume* teraStack;
	// --------------------------------------------------------------

	QString inputSWCdir;
	QString inputImagedir;
	QString inputBkgDir;
	QVector<QString> imageFolders;
	QString outputImagedir;
	void getImageFolders();
	void pick_save();

private:
	V3DPluginCallback2* OperatorCallback;
	taskFromUI operatingTask;
	queue<taskFromUI> taskQueu;
	vector<opPtr> opFuncSeq;
};

#endif
