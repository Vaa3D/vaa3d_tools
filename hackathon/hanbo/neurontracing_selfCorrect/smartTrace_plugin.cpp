/* smartTrace_plugin.cpp
 * This plugin will automatically trace and correct neuron reconstructions.
 * 2015-4-21 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "smartTrace_plugin.h"
#include "src/nt_selfcorrect_func.h"
Q_EXPORT_PLUGIN2(smartTrace, smartTrace_plugin);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList smartTrace_plugin::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList smartTrace_plugin::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
        <<tr("selfCorrectin")
        <<tr("smartTrace")
		<<tr("help");
}

void smartTrace_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("This plugin will automatically trace and correct neuron reconstructions.. "
			"Developed by Hanbo Chen, 2015-4-21"));
	}
}

bool smartTrace_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("tracing_func"))
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

        reconstruction_func(callback,parent,PARA,bmenu);
    }
    else if(func_name == tr("selfCorrection"))
    {
        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();

        if(infiles.size()<2)
        {
            fprintf (stderr, "Need input image and swc file. \n");
            return false;
        }
        if(outfiles.empty())
        {
            fprintf (stderr, "Need output file. \n");
            return false;
        }
        if(paras.empty())
        {
            fprintf (stderr, "Need confidential score file. \n");
            return false;
        }

        nt_selfcorrect_func tracefunc;
        tracefunc.correct_tracing(infiles.at(0), infiles.at(1), paras.at(0), outfiles.at(0), &callback);
    }
    else if (func_name == tr("smartTrace"))
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

        nt_selfcorrect_func tracefunc;
        tracefunc.smart_tracing(PARA.inimg_file,PARA.inimg_file+"_smartTracing",&callback);
    }
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN

        printf("\n\n**** Usage of smartTrace tracing **** \n");
        printf("vaa3d -x smartTrace -f tracing_func -i <inimg_file> -p <channel> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

        printf("vaa3d -x smartTrace -f selfCorrection -i <inimg_file> <inswc_file> -o <output> -p <score.txt> \n");
        printf("inimg_file       The input image\n");
        printf("inswc_file       The input tracing\n");
        printf("score.txt        Confidential score calculated by plugin: calculate_reliability_score.\n");

        printf("\n\nvaa3d -x smartTrace -f smartTrace -i <inimg_file> -p <channel> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
    V3DLONG in_sz[4];
    int datatype = 0;
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }


        data1d = p4DImage->getRawData();
        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        sc = p4DImage->getCDim();
        datatype = (int) p4DImage->getDatatype();

        bool ok1;

        if(sc==1)
        {
            c=1;
            ok1=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, sc, 1, &ok1);
        }

        if(!ok1)
            return;

        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;


        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
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

    if(datatype!=1){
        v3d_msg(QString("Now only support image type UINT8."),bmenu);
        return;
    }
    //main neuron reconstruction code

    QString swc_name=PARA.inimg_file+"_smartTracing";

    nt_selfcorrect_func tracefunc;
    tracefunc.smart_tracing(PARA.inimg_file,swc_name,&callback,c-1);


    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
