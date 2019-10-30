/* adaboost_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-10-29 : by DZC
 */
 
#include "v3d_message.h"
#include <vector>
#include "adaboost_plugin.h"
#include "basic_surf_objs.h"
#include "neuron_sim_scores.h"
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
        /*
        QString inimg=infiles[0];
        Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(inimg) ));
        int nChannel = p4dImage->getCDim();
        V3DLONG mysz[4];
        mysz[0] = p4dImage->getXDim();
        mysz[1] = p4dImage->getYDim();
        mysz[2] = p4dImage->getZDim();
        mysz[3] = nChannel;
        qDebug()<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
        unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);
        */

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
        arg_para_crop.push_back(inparas[0]); arg_para_crop.push_back(inparas[1]);arg_para_crop.push_back(inparas[2]);
        arg_para_crop.push_back(inparas[3]);arg_para_crop.push_back(inparas[4]);arg_para_crop.push_back(inparas[5]);
        arg_para_crop.push_back("1");
        arg.p=(void *) & arg_para_crop;input_crop<<arg;
        //output arg list
        arg.type= "random";
        std::vector <char *> arg_output_crop;
        arg_output_crop.push_back(outfiles[0]); arg.p = (void *) & arg_output_crop; output_crop<< arg;
        //call crop image function
        callback.callPluginFunc(croppluginname,cropfuncname,input_crop,output_crop);

        //prepare to call app2
        QString app2pluginname= "vn2";
        QString app2funcname="app2";
        //input -i
        V3DPluginArgList input_app2;
        V3DPluginArgList output_app2;
        arg.type = "random"; std::vector <char *>arg_input_app2; arg_input_app2.push_back(outfiles[0]); arg.p = (void *) & arg_input_app2;  input_app2<<arg;
         //parameters set for app2
        arg.type="random"; std::vector <char *> arg_para_app2;
        arg_para_app2.push_back("NULL"); arg_para_app2.push_back("0");arg_para_app2.push_back("AUTO");
        arg_para_app2.push_back("1"); arg_para_app2.push_back("1"); arg_para_app2.push_back("0"); arg_para_app2.push_back("0");
        arg_para_app2.push_back("5");arg_para_app2.push_back("0");arg_para_app2.push_back("0");arg_para_app2.push_back("0");arg.p= (void *)& arg_para_app2;
        input_app2<<arg;        //p NULL 0 AUTO 1 1  0 0 5 0 0 0
        //output -o
        arg.type= "random";
        std::vector <char *> arg_output_app2; arg_output_app2.push_back(outfiles[1]);arg.p=(void *) & arg_output_app2;output_app2<<arg;
         callback.callPluginFunc(app2pluginname,app2funcname,input_app2,output_app2);

         //calculate neuron distance
         int start_x,start_y,start_z, end_x,end_y,end_z;
         start_x= atoi(inparas[0]);
         end_x=  atoi(inparas[1]);
         start_y= atoi(inparas[2]);
         end_y= atoi(inparas[3]);
         start_z= atoi(inparas[4]);
         end_z= atoi(inparas[5]);
        NeuronTree nt= readSWC_file(outfiles[1]);

         for(int i=0;i<nt.listNeuron.size(); ++i)
         {
            nt.listNeuron[i].x+=start_x;
            nt.listNeuron[i].y+=start_y;
            nt.listNeuron[i].z+=start_z;
          }

         const NeuronTree tmpt(nt);
         QString swcfile=outfiles[2];
         writeSWC_file(swcfile,tmpt);

         //neuron distance
         double  d_thres = 2;
         bool bmenu = 0;
         NeuronTree nt1 = readSWC_file(outfiles[2]);
         double dist;
         if(nt1.listNeuron.size()<5)
         {
                dist=-1;
                qDebug()<<"no swc there to compare";
         }else
         {
            NeuronTree nt2 = readSWC_file(infiles[1]);
            NeuronDistSimple tmp_score = neuron_score_rounding_nearest_neighbor(&nt1, &nt2,bmenu,d_thres);

            double dist=tmp_score.dist_12_allnodes;
            qDebug()<<dist;
            }






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

