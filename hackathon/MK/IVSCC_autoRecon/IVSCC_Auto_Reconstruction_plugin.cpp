/* IVSCC_Auto_Reconstruction_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-8-13 : by MK
 */
 
#include <iostream>
#include <vector>

#include "v3d_message.h"
#include "IVSCC_Auto_Reconstruction_plugin.h"
#include "SegPipe_Controller.h"

using namespace std;

Q_EXPORT_PLUGIN2(IVSCC_Auto_Reconstruction, IVSCC_autoRecon);
 
QStringList IVSCC_autoRecon::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList IVSCC_autoRecon::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void IVSCC_autoRecon::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by MK, 2018-8-13"));
	}
}

bool IVSCC_autoRecon::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	QString inputPathQ = infiles[0];

	QString outputPathQ;
	QString output2;
	if (!outfiles.empty()) outputPathQ = outfiles[0];

	SegPipe_Controller* segPipePtr = new SegPipe_Controller(inputPathQ, outputPathQ);

	if (func_name == tr("downSample2D_Max"))
	{
		if (inparas[0])
		{
			QString downFactorQ = inparas[0];
			int downFactor = downFactorQ.toInt();
			segPipePtr->sliceDownSample2D(downFactor, "max");
		}
		else
		{
			cerr << "Invalid parameter. Do nothing and return." << endl;
			return false;
		}
	}
	else if (func_name == tr("getMIP")) segPipePtr->makeMIPimgs();
	else if (func_name == tr("makeDescentSkeletons"))
	{
		if (outfiles[1])
			segPipePtr->outputRootPath2 = outfiles[1];
		segPipePtr->makeDescentSkeletons();
	}
	else if (func_name == tr("gammaCorrect2D")) segPipePtr->sliceGammaCorrect();
	else if (func_name == tr("threshold2D"))
	{
		if (inparas[0])
		{
			QString percentileQ = inparas[0];
			float percentile = percentileQ.toFloat();
			segPipePtr->sliceThre(percentile);
		}
		else
		{
			cerr << "Invalid parameter. Do nothing and return." << endl;
			return false;
		}
	}
	else if (func_name == tr("threshold3D"))
	{
		if (inparas[0])
		{
			QString percentileQ = inparas[0];
			float percentile = percentileQ.toFloat();
			segPipePtr->threshold3D(percentile);
		}
		else
		{
			cerr << "Invalid parameter. Do nothing and return." << endl;
			return false;
		}
	}
	else if (func_name == tr("hist")) segPipePtr->histQuickList();
	else if (func_name == tr("bkgThreshold")) segPipePtr->sliceBkgThre();
	else if (func_name == tr("cropImgWithSWC"))
	{	
		if (inparas[0]) segPipePtr->refSWCRootPath = inparas[0];
		if (outfiles[1])
		{
			segPipePtr->outputSWCRootPath.clear();
			segPipePtr->outputSWCRootPath = output2;
		}
		segPipePtr->swc_imgCrop();
	}
	else if (func_name == tr("findSoma")) segPipePtr->findSomaMass();
	else if (func_name == tr("findSomaCandidates"))
	{
		QString distThreQ = inparas[0];
		float distThre = distThreQ.toFloat();
		segPipePtr->getSomaCandidates(distThre);
	}
	else if (func_name == tr("getDenSkeletonStart"))
	{
		if (infiles[1])
		{
			segPipePtr->inputSWCRootPath.clear();
			segPipePtr->inputSWCRootPath = infiles[1];
		}
		if (outfiles[0]) segPipePtr->outputSWCRootPath = outfiles[0];
		segPipePtr->getDendriteSkeletonStart();
	}
	else if (func_name == tr("getSomaBlended"))
	{
		if (infiles[1])	segPipePtr->inputCaseRootPath2 = infiles[1];
		segPipePtr->getSomaBlendedImgs();
	}
	else if (func_name == tr("threStep"))
	{
		if (infiles[1]) segPipePtr->inputCaseRootPath2 = infiles[1];
		if (outfiles[0]) segPipePtr->outputRootPath = outfiles[0];
		if (outfiles[1]) segPipePtr->outputRootPath2 = outfiles[1];

		segPipePtr->skeletonThreFiltered();
	}
	else if (func_name == tr("swcMapBack"))
	{
		if (inparas[0]) segPipePtr->refSWCRootPath = inparas[0];
		if (infiles[1])
		{
			segPipePtr->inputSWCRootPath.clear();
			segPipePtr->inputSWCRootPath = infiles[1];
		}
		if (outfiles[0]) segPipePtr->outputSWCRootPath = outfiles[0];
		segPipePtr->swcMapBack();
	}
	else if (func_name == tr("signalBlob")) segPipePtr->findSignalBlobs2D();
	else if (func_name == tr("centroid")) segPipePtr->getChebyshevCenters(output2);
	else if (func_name == tr("2Dcentroid")) segPipePtr->swc2DsignalBlobsCenter();
	else if (func_name == tr("3Dcentroid")) segPipePtr->swcSignalBlob3Dcenter();
	else if (func_name == tr("MST")) segPipePtr->getMST();
	else if (func_name == tr("MSTslice")) segPipePtr->getMST_2Dslices();
	else if (func_name == tr("tiledMST")) segPipePtr->getTiledMST();
	else if (func_name == tr("MSTcut")) segPipePtr->cutMST();
	else if (func_name == tr("MSTtrim")) segPipePtr->MSTtrim();
	else if (func_name == tr("MSTbranchBreak")) segPipePtr->breakMSTbranch();
	else if (func_name == tr("correctSWC"))
	{
		if (inparas[0]) segPipePtr->refSWCRootPath = inparas[0];
		else
		{
			cerr << "No reference swc path sepecied. Do nothing and return." << endl;
			return false;
		}
		segPipePtr->correctSWC();
	}
	else if (func_name == tr("swcRegister"))
	{
		if (inparas[0]) segPipePtr->refSWCRootPath = inparas[0];
		else
		{
			cerr << "No reference swc path sepecied. Do nothing and return." << endl;
			return false;
		}
		segPipePtr->swcRegister();
	}
	else if (func_name == tr("swcScale"))
	{
		QString xScaleQ = inparas[0];
		QString yScaleQ = inparas[1];
		QString zScaleQ = inparas[2];
		float xScale = xScaleQ.toFloat();
		float yScale = yScaleQ.toFloat();
		float zScale = zScaleQ.toFloat();

		segPipePtr->swcScale(xScale, yScale, zScale);
	}
	else if (func_name == tr("swcOverlap"))
	{
		if (inparas[0]) segPipePtr->refSWCRootPath = inparas[0];
		else
		{
			cerr << "No reference swc path sepecied. Do nothing and return." << endl;
			return false;
		}
		segPipePtr->nodeIdentify();
	}
	else if (func_name == tr("swcSeparate"))
	{
		QString outputRoot2 = outfiles[1];
		segPipePtr->swcSeparate(outputRoot2);
	}
	else if (func_name == tr("cleanUp2DcentroidZ")) segPipePtr->cleanUpzFor2Dcentroids();
	else if (func_name == tr("segElongation")) segPipePtr->segElongation();
	else if (func_name == tr("segTerminalize")) segPipePtr->segTerminalize();
	else if (func_name == tr("removeDots")) segPipePtr->dotRemove();
	else if (func_name == tr("longConnCut")) segPipePtr->longConnCut();
	else if (func_name == tr("treeUnion"))
	{
		if (inparas[0]) segPipePtr->refSWCRootPath = inparas[0];
		segPipePtr->treeUnion();
	}
	else if (func_name == tr("treeWithinDist")) segPipePtr->treeWithinDist();
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

