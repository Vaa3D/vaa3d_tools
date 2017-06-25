/* region_match_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-6-21 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include <iostream>
#include "region_match_plugin.h"

#include "match_swc.h"
Q_EXPORT_PLUGIN2(region_match, region_match);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    NeuronTree nt;
    NeuronTree mk;
};

void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);


QStringList region_match::menulist() const
{
	return QStringList() 
		<<tr("matching_menu")
		<<tr("about");
}

QStringList region_match::funclist() const
{
	return QStringList()
		<<tr("matching_func")
		<<tr("help");
}

void region_match::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("matching_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;
        ml_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2017-6-21"));
	}
}

bool region_match::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("matching_func"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        QString inneuron_file = (paras.size() >= k+1) ? paras[k] : "";k++;
        if(!inneuron_file.isEmpty())
            PARA.nt = readSWC_file(inneuron_file);
        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        //QList<NeuronSWC> file_inmarkers;
        if(!inmarker_file.isEmpty())
            //file_inmarkers=readSWC_file(inmarker_file).listNeuron;
            PARA.mk=readSWC_file(inmarker_file);

        ml_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of region_match tracing **** \n");
		printf("vaa3d -x region_match -f matching_func -i <inimg_file> -p <channel> <swc_file> <marker_file> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("swc_file         SWC file path.\n");
        printf("marker_file      Marker file path.\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n");
        printf("outmarker_file   Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
    V3DLONG in_sz[4];
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }
        PARA.mk = callback.getSWC(curwin);
        PARA.nt = callback.getSWC(curwin);
        //PARA.inimg_file = p4DImage->getFileName();
        vector<NeuronTree> s_forest;
        NeuronTree s_mk;
        match_swc(PARA.nt,PARA.mk,s_mk,s_forest);

    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
        }
        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;
    }

    //main neuron machine learning code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON MACHINE LEARNING CODE
    cout<<"******************This is main function*********************"<<endl;

    vector<NeuronTree> s_forest;
    NeuronTree s_mk;
    match_swc(PARA.nt,PARA.mk,s_mk,s_forest);

    //Output
    NeuronTree nt_output;
    //QList<NeuronSWC> marker_output;
    //QList<ImageMarker> marker_output;

	QString swc_name = PARA.inimg_file + "_region_match.swc";
	nt_output.name = "region_match";
    //QString marker_name = PARA.inimg_file + "_region_match.marker";
    //writeSWC_file(swc_name.toStdString().c_str(),nt_output);
    //writeMarker_file(marker_name,marker_output);


    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    //if(nt_output.listNeuron.size()>0) v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);
    //if(marker_output.size()>0) v3d_msg(QString("Now you can drag and drop the generated marker fle [%1] into Vaa3D.").arg(marker_name.toStdString().c_str()),bmenu);


    return;
}













