/* neurontracing_rotation_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-5-16 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "neurontracing_rotation_plugin.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"
#include "rotate_image.h"
#include "opt_rotate.h"

Q_EXPORT_PLUGIN2(neurontracing_rotation, neurontracing_rotation);

using namespace std;

struct input_PARA
{
    int is_gsdt;
    int is_break_accept;
    int  bkg_thresh;
    double length_thresh;
    int  cnn_type;
    int  channel;
    double SR_ratio;
    int  b_256cube;
    int b_RadiusFrom2D;
    int rotation_degree;

    Image4DSimple* image;
    LandmarkList listLandmarks;
    QString inimg_file,markerfilename;

};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList neurontracing_rotation::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList neurontracing_rotation::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void neurontracing_rotation::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;
        neuronrotation_app2 dialog(callback, parent);
        if (dialog.image == 0)
            return;
        if (dialog.exec()!=QDialog::Accepted)
            return;

        PARA.image = dialog.image;
        PARA.listLandmarks = dialog.listLandmarks;
        PARA.is_gsdt = dialog.is_gsdt;
        PARA.is_break_accept = dialog.is_break_accept;
        PARA.bkg_thresh = dialog.bkg_thresh;
        PARA.length_thresh = dialog.length_thresh;
        PARA.cnn_type = dialog.cnn_type;
        PARA.channel = dialog.channel;
        PARA.SR_ratio = dialog.SR_ratio;
        PARA.b_256cube = dialog.b_256cube;
        PARA.b_RadiusFrom2D = dialog.b_RadiusFrom2D;
        PARA.rotation_degree = dialog.rotation_degree;

        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2016-5-16"));
	}
}

bool neurontracing_rotation::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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


		printf("**** Usage of neurontracing_rotation tracing **** \n");
		printf("vaa3d -x neurontracing_rotation -f tracing_func -i <inimg_file> -p <channel> <other parameters>\n");
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
        data1d = PARA.image->getRawData();
        N = PARA.image->getXDim();
        M = PARA.image->getYDim();
        P = PARA.image->getZDim();
        sc = PARA.image->getCDim();

        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;

        PARA.inimg_file = PARA.image->getFileName();
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

    PARA_APP2 p2;
    QString versionStr = "v0.001";


    p2.is_gsdt = PARA.is_gsdt;
    p2.is_coverage_prune = true;
    p2.is_break_accept = PARA.is_break_accept;
    p2.bkg_thresh = PARA.bkg_thresh;
    p2.length_thresh = PARA.length_thresh;
    p2.cnn_type = 2;
    p2.channel = 0;
    p2.SR_ratio = 3.0/9.9;
    p2.b_256cube = PARA.b_256cube;
    p2.b_RadiusFrom2D = PARA.b_RadiusFrom2D;
    p2.b_resample = 1;
    p2.b_intensity = 0;
    p2.b_brightfiled = 0;
    p2.b_menu = 0; //if set to be "true", v3d_msg window will show up.

    Options_Rotate r_opt;
    r_opt.b_keepSameSize = true;
    r_opt.degree = -PARA.rotation_degree/180.0*3.141592635;
    r_opt.center_x = (in_sz[0]-1.0)/2;
    r_opt.center_y = (in_sz[1]-1.0)/2;
    r_opt.center_z = (in_sz[2]-1.0)/2;
    r_opt.fillcolor = 0;

    unsigned char * outvol1d=0;
    V3DLONG *outsz=0;
    bool b_res=false;
    b_res = rotate_inPlaneZ(PARA.image->getRawData(), in_sz, r_opt, outvol1d, outsz);

    Image4DSimple* total4DImage = new Image4DSimple;
    total4DImage->setData((unsigned char*)outvol1d, outsz[0], outsz[1], outsz[2], 1, V3D_UINT8);


    p2.p4dImage = total4DImage;
    p2.xc0 = p2.yc0 = p2.zc0 = 0;
    p2.xc1 = outsz[0]-1;
    p2.yc1 = outsz[1]-1;
    p2.zc1 = outsz[2]-1;

    //Output
  //  NeuronTree nt;
    QString swc_name = PARA.inimg_file + "_neurontracing_rotation.swc";
    p2.outswc_file = swc_name;

    for(int i = 0; i <PARA.listLandmarks.size();i++)
    {
        LocationSimple RootNewLocation;
        RootNewLocation.x = PARA.listLandmarks.at(i).x;
        RootNewLocation.y = PARA.listLandmarks.at(i).y;
        RootNewLocation.z = PARA.listLandmarks.at(i).z;
        p2.landmarks.push_back(RootNewLocation);
    }

    proc_app2(callback, p2, versionStr);
    if(outvol1d) {delete []outvol1d; outvol1d = 0;}

//	nt.name = "neurontracing_rotation";
 //   writeSWC_file(swc_name.toStdString().c_str(),nt);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
