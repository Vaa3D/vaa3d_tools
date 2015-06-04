/* nctuTW_tips_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-6-4 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "nctuTW_tips_plugin.h"
#include "findingtips.h"

#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h""
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "stackutil.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.h"

Q_EXPORT_PLUGIN2(nctuTW_tips, nctuTW_tips);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList nctuTW_tips::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList nctuTW_tips::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void nctuTW_tips::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2015-6-4"));
	}
}

bool nctuTW_tips::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of nctuTW_tips tracing **** \n");
		printf("vaa3d -x nctuTW_tips -f tracing_func -i <inimg_file> -p <channel> <other parameters>\n");
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

    //main neuron reconstruction code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE
    string sImgPath = PARA.inimg_file.toStdString();
    g_sAppDir = normalizePath(parentPath(sImgPath));

    QString swc_name = PARA.inimg_file + "_nctuTW_GD.swc";
    g_sOutSwcFilePath = swc_name.toStdString();

    size_t nDataSize = N * M * P * sc ;

    width = N;
    height = M;
    zSize = P;

    imgBuf_raw = new unsigned char [nDataSize];
    memcpy(imgBuf_raw, data1d, nDataSize);

    findingtipsmain();

    QList <ImageMarker> file_inmarkers;
    QString markerfile = "EndPoints3D.marker";
    file_inmarkers = readMarker_file(markerfile);
    //GD_tracing
    LocationSimple p0;
    vector<LocationSimple> pp;
    NeuronTree nt;

    double weight_xy_z=1.0;
    bool b_mergeCloseBranches = false;
    bool b_usedshortestpathonly = false;
    bool b_postTrim = true;
    bool b_pruneArtifactBranches = true;
    int ds_step = 2;

    CurveTracePara trace_para;
    trace_para.channo = 0;
    trace_para.sp_graph_resolution_step = ds_step;
    trace_para.b_deformcurve = b_usedshortestpathonly;
    trace_para.b_postMergeClosebyBranches = b_mergeCloseBranches;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = b_postTrim;
    trace_para.b_pruneArtifactBranches = b_pruneArtifactBranches;

    p0.x = file_inmarkers.at(0).x;
    p0.y = file_inmarkers.at(0).y;
    p0.z = file_inmarkers.at(0).z;

    LocationSimple tmpp;
    for(V3DLONG i = 1; i <file_inmarkers.size(); i++)
    {
        tmpp.x = file_inmarkers.at(i).x;
        tmpp.y = file_inmarkers.at(i).y;
        tmpp.z = file_inmarkers.at(i).z;
        pp.push_back(tmpp);
    }

    V3DLONG sz_tracing[4];
    sz_tracing[0] = N;
    sz_tracing[1] = M;
    sz_tracing[2] = P;
    sz_tracing[3] = 1;

    unsigned char ****p4d = 0;
    if (!new4dpointer(p4d, sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3], data1d))
    {
        fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
        return;
    }

    nt = v3dneuron_GD_tracing(p4d, sz_tracing,
                              p0, pp,
                              trace_para, weight_xy_z);


    if(p4d) {delete []p4d; p4d = 0;}

    writeSWC_file(swc_name,nt);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
