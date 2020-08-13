/* JudgeBranch_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-6-11 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "JudgeBranch_plugin.h"

#include "filter.h"


using namespace std;
Q_EXPORT_PLUGIN2(JudgeBranch, JudgeBranchPlugin);
 
QStringList JudgeBranchPlugin::menulist() const
{
	return QStringList() 
        <<tr("JudgeBranch")
        <<tr("gaussian")
		<<tr("about");
}

QStringList JudgeBranchPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void JudgeBranchPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("JudgeBranch"))
	{
        JudgeBranchDialog dlg = JudgeBranchDialog(parent,callback);
        dlg.exec();
	}
    else if (menu_name == tr("gaussian"))
	{
//        v3dhandle cur = callback.currentImageWindow();
//        Image4DSimple* image = callback.getImage(cur);
//        unsigned char* pdata = image->getRawData();
//        double sigma = 5;
//        V3DLONG sz[4] = {image->getXDim(),image->getYDim(),image->getZDim(),image->getCDim()};
//        V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
//        unsigned char* outdata = new unsigned char[tolSZ];
//        laplacianOfGaussian(pdata,outdata,sz,sigma);
//        Image4DSimple* imageGaussian = new Image4DSimple();
//        imageGaussian->setData(outdata,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
//        v3dhandle gaussianHandle = callback.newImageWindow("gaussian image");
//        callback.setImage(gaussianHandle,imageGaussian);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2020-6-11"));
	}
}

bool JudgeBranchPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
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

