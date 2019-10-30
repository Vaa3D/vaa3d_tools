/* adaboost_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-10-29 : by DZC
 */
 
#include "v3d_message.h"
#include <vector>
#include "adaboost_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(adaboost, adaboost);
 
QStringList adaboost::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList adaboost::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void adaboost::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by DZC, 2019-10-29"));
	}
}

bool adaboost::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
//        QString inimg=infiles[0];
//        Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(inimg) ));
//        int nChannel = p4dImage->getCDim();
//        V3DLONG mysz[4];
//        mysz[0] = p4dImage->getXDim();
//        mysz[1] = p4dImage->getYDim();
//        mysz[2] = p4dImage->getZDim();
//        mysz[3] = nChannel;
//        qDebug()<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
//        unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);

//        int start_x,start_y,start_z, end_x,end_y,end_z;
//        start_x= atoi(inparas[0]);
//        end_x=  atoi(inparas[1])
//        start_y= atoi(inparas[2]);
//        end_y= atoi(inparas[3]);
//        start_z= atoi(inparas[4]);
//        end_z= atoi(inparas[5]);
//        int ch=1;

        //call crop image function
        QString croppluginname= "cropped3DImageSeries";
        QString cropfuncname="crop3d_raw";
        //input arg list
        V3DPluginArgItem arg;
        V3DPluginArgList input_crop;
        V3DPluginArgList output_crop;
        arg.type = "random"; std::vector <char *>arg_input_crop;
        arg_input_crop.push_back(infiles[0]);
        arg.p = (void *) & arg_input_crop;input_crop<<arg;
        //para arg list
        arg.type="random";std::vector <char *> arg_para_crop;
        arg_para_crop.push_back(inparas[0]);
        arg_para_crop.push_back(inparas[1]);
        arg_para_crop.push_back(inparas[2]);
        arg_para_crop.push_back(inparas[3]);
        arg_para_crop.push_back(inparas[4]);
        arg_para_crop.push_back(inparas[5]);
        arg_para_crop.push_back("1");
        arg.p=(void *) & arg_para_crop;input_crop<<arg;
        //output arg list
        arg.type= "random";
        std::vector <char *> arg_output_crop;
        arg_output_crop.push_back(outfiles[0]); arg.p = (void *) & arg_output_crop; output_crop<< arg;

        callback.callPluginFunc(croppluginname,cropfuncname,input_crop,output_crop);



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

