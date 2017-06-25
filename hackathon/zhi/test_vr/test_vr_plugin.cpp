/* test_vr_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-6-24 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "test_vr_plugin.h"
Q_EXPORT_PLUGIN2(test_vr, test_vr);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    NeuronTree nt_search;
    NeuronTree nt_pattern;
};

void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList test_vr::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList test_vr::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void test_vr::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;
        ml_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2017-6-24"));
	}
}

bool test_vr::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("tracing_func"))
	{
//        bool bmenu = false;
//        input_PARA PARA;

//        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
//        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
//        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
//        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

//        if(infiles.empty())
//        {
//            fprintf (stderr, "Need input image. \n");
//            return false;
//        }
//        else
//            PARA.inimg_file = infiles[0];
//        int k=0;
//        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
//        QString inneuron_file = (paras.size() >= k+1) ? paras[k] : "";k++;
//        if(!inneuron_file.isEmpty())
//            PARA.nt = readSWC_file(inneuron_file);
//        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
//        QList<ImageMarker> file_inmarkers;
//        if(!inmarker_file.isEmpty())
//            file_inmarkers = readMarker_file(inmarker_file);

//        LocationSimple t;
//        for(int i = 0; i < file_inmarkers.size(); i++)
//        {
//            t.x = file_inmarkers[i].x+1;
//            t.y = file_inmarkers[i].y+1;
//            t.z = file_inmarkers[i].z+1;
//            PARA.listLandmarks.push_back(t);
//        }

//        ml_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of test_vr tracing **** \n");
		printf("vaa3d -x test_vr -f tracing_func -i <inimg_file> -p <channel> <swc_file> <marker_file> <other parameters>\n");
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
       PARA.nt_search = readSWC_file("original_vr_neuron.swc");
       PARA.nt_pattern = readSWC_file("areaofinterest.swc");
    }
    else
    {
//        int datatype = 0;
//        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
//        {
//            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
//            return;
//        }
//        if(PARA.channel < 1 || PARA.channel > in_sz[3])
//        {
//            fprintf (stderr, "Invalid channel number. \n");
//            return;
//        }
//        N = in_sz[0];
//        M = in_sz[1];
//        P = in_sz[2];
//        sc = in_sz[3];
//        c = PARA.channel;
    }

    //main neuron machine learning code
    QList <NeuronSWC> list_pattern = PARA.nt_pattern.listNeuron;
    QList <NeuronSWC> list_search = PARA.nt_search.listNeuron;

    if(list_pattern.size()!=0 || list_search.size()!=0)
    {
        for(V3DLONG i = 0; i < list_search.size(); i++)
        {
            PARA.nt_search.listNeuron[i].type += 1;
        }
    }

    //Output
    writeSWC_file("updated_vr_neuron.swc",PARA.nt_search);

    return;
}












