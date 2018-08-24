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

	if (func_name == tr("segmentationPipe"))
	{
		QString inputPathQ = infiles[0];
		QString inputSWCPathQ;
		if (infiles[1]) inputSWCPathQ = infiles[1];
		QString outputPathQ;

		if (!outfiles.empty()) outputPathQ = outfiles[0];
		SegPipe_Controller* segPipePtr = new SegPipe_Controller(inputPathQ, outputPathQ);
		//segPipePtr->sliceDownSample2D(2, "max");
		//segPipePtr->adaSliceGammaCorrect();
		//segPipePtr->sliceReversedGammaCorrect();
		//segPipePtr->sliceThre(0.999);
		//segPipePtr->histQuickList();
		//segPipePtr->sliceBkgThre();
		//segPipePtr->findConnComponent();
		//segPipePtr->findSomaMass();
		segPipePtr->swc_imgCrop(inputSWCPathQ);
		SegPipe_Controller* segPipePtr2 = new SegPipe_Controller(outputPathQ, "H:");
		segPipePtr2->findConnComponent();
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

