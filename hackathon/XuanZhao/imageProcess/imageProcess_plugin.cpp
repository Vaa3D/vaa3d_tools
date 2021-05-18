/* imageProcess_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-1-12 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "imageProcess_plugin.h"
#include "imageprocess.h"
using namespace std;
Q_EXPORT_PLUGIN2(imageProcess, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("enhanceImage")
        <<tr("get_2d_image")
        <<tr("getSWCL0Image")
        <<tr("convertData")
        <<tr("HE")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by YourName, 2020-1-12"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("enhanceImage"))
	{
        QString imgPath = infiles[0];
        QString imgNewPath = imgPath.split('.')[0] + "_result.v3draw";
        qDebug()<<imgNewPath;

        unsigned char* data1d = 0;
        V3DLONG sz[4] = {0,0,0,0};
        int datatype = 0;
        simple_loadimage_wrapper(callback,imgPath.toStdString().c_str(),data1d,sz,datatype);
        enhanceImage(data1d,sz);
        simple_saveimage_wrapper(callback,imgNewPath.toStdString().c_str(),data1d,sz,datatype);
	}
    else if (func_name == tr("get_2d_image"))
	{
        get_2d_image(input,output,callback);
	}
    else if (func_name == tr("get_2dimage2")){
        get_2d_image2(input,output,callback);
    }
    else if (func_name == tr("getSWCL0Image"))
    {
        QString swcPath = infiles[0];
        QString brainPath = inparas[0];
        QString outDir = outfiles[0];
        int times = atoi(inparas[1]);

        QString outPath = outDir + "\\result.v3draw";
        getSwcL0Image(swcPath,brainPath,outPath,times,callback);
    }
    else if (func_name == tr("convertData"))
    {
        QString path = infiles[0];
        QString outDir = outfiles[0];
        int times = atoi(inparas[0]);
        int t = 16/times;
        convertData(path,callback);
        downSampleData(path,t,callback);
        QString outPath = outDir + "\\result.v3draw";
        joinImage(path,outPath,times,callback);
    }
    else if (func_name == tr("convertTeraflyDataTov3draw")){
        QString path = infiles.size()>=1 ? infiles[0] : "";
        QString outPath = outfiles.size()>=1 ? outfiles[0] : "";
        int resolution = atoi(inparas[0]);
        joinImage2(path,outPath,resolution,callback);
    }
    else if (func_name == "HE") {
        QString imgPath = infiles[0];
        QString imgNewPath = imgPath + "_HE.v3draw";
        qDebug()<<imgNewPath;

        unsigned char* data1d = 0;
        V3DLONG sz[4] = {0,0,0,0};
        int datatype = 0;
        simple_loadimage_wrapper(callback,imgPath.toStdString().c_str(),data1d,sz,datatype);
        HE(data1d,sz);
        simple_saveimage_wrapper(callback,imgNewPath.toStdString().c_str(),data1d,sz,datatype);
        if(data1d){
            delete[] data1d;
            data1d = 0;
        }
    }
    else if (func_name == "bilateralfilter") {
        QString imgPath = infiles[0];
        QString imgNewPath = imgPath + "_bilateralfilter.v3draw";
        qDebug()<<imgNewPath;

        bool isNormal = inparas.size()>=1 ? atoi(inparas[0]) : 1;
        double spaceSigmaXY = inparas.size()>=2 ? atof(inparas[1]) : 2;
        double spaceSigmaZ = inparas.size()>=3 ? atof(inparas[2]) : 1;
        double colorSigma = inparas.size()>=4 ? atof(inparas[3]) : 35;

        unsigned char* data1d = 0;
        V3DLONG sz[4] = {0,0,0,0};
        int datatype = 0;
        simple_loadimage_wrapper(callback,imgPath.toStdString().c_str(),data1d,sz,datatype);

        removeAbnormalLine(data1d,sz);
        if(isNormal){
            convertDataTo0_255(data1d,sz);
        }

        unsigned char* dst = 0;
        V3DLONG kernelSZ[3] = {spaceSigmaXY*4+1,spaceSigmaXY*4+1,spaceSigmaZ*4+1};
        bilateralfilter(data1d,dst,sz,kernelSZ,spaceSigmaXY,spaceSigmaZ,colorSigma);
        simple_saveimage_wrapper(callback,imgNewPath.toStdString().c_str(),dst,sz,datatype);
        if(data1d){
            delete[] data1d;
            data1d = 0;
        }
        if(dst){
            delete[] dst;
            dst = 0;
        }
    }else if (func_name == tr("changeContrast")) {
        QString imgPath = infiles[0];
        QString imgNewPath = imgPath + "_enhance.v3draw";
        double percentDown = inparas.size()>=1 ? atof(inparas[0]) : 0.1;
        double percentUp = inparas.size()>=2 ? atof(inparas[1]) : 99.9;
        qDebug()<<imgNewPath;

        unsigned char* data1d = 0;
        V3DLONG sz[4] = {0,0,0,0};
        int datatype = 0;
        simple_loadimage_wrapper(callback,imgPath.toStdString().c_str(),data1d,sz,datatype);
        changeContrast2(data1d,sz,percentDown,percentUp);
        simple_saveimage_wrapper(callback,imgNewPath.toStdString().c_str(),data1d,sz,datatype);
        if(data1d){
            delete[] data1d;
            data1d = 0;
        }
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

