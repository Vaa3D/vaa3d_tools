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

#include "../../xiaoxiaol/consensus_skeleton_2/consensus_skeleton.h"


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
NeuronTree swc_rotation(NeuronTree nt, V3DLONG *insz, double degree);
 
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
    r_opt.b_keepSameSize = false;
    r_opt.center_x = (in_sz[0]-1.0)/2;
    r_opt.center_y = (in_sz[1]-1.0)/2;
    r_opt.center_z = (in_sz[2]-1.0)/2;
    r_opt.fillcolor = 0;

    vector<NeuronTree> nt_list;

    for(int i = 0; i<360; i=i+PARA.rotation_degree)
    {
        double curret_degree = (double)i;
        if(curret_degree > 180)
            curret_degree = 180 - curret_degree;
        p2.outswc_file = PARA.inimg_file + QString("_neurontracing_rotation_%1.swc").arg(curret_degree);


        if(i == 0)
        {
            p2.p4dImage = PARA.image;
            p2.xc0 = p2.yc0 = p2.zc0 = 0;
            p2.xc1 = in_sz[0]-1;
            p2.yc1 = in_sz[1]-1;
            p2.zc1 = in_sz[2]-1;

            proc_app2(callback, p2, versionStr);
            NeuronTree nt = readSWC_file(p2.outswc_file);
            nt_list.push_back(nt);

        }else
        {
            r_opt.degree = -curret_degree/180.0*3.141592635;
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

        //    for(int i = 0; i <PARA.listLandmarks.size();i++)
        //    {
        //        LocationSimple RootNewLocation;
        //        RootNewLocation.x = PARA.listLandmarks.at(i).x;
        //        RootNewLocation.y = PARA.listLandmarks.at(i).y;
        //        RootNewLocation.z = PARA.listLandmarks.at(i).z;
        //        p2.landmarks.push_back(RootNewLocation);
        //    }

            proc_app2(callback, p2, versionStr);

            NeuronTree nt = readSWC_file(p2.outswc_file);
            NeuronTree nt_rotated = swc_rotation(nt,in_sz,-curret_degree);

            nt_list.push_back(nt_rotated);

//            QString swc_rotated_name = PARA.inimg_file + QString("_neurontracing_rotation_%1_back.swc").arg(curret_degree);
//            writeSWC_file(swc_rotated_name.toStdString().c_str(),nt_rotated);

            if(outvol1d) {delete []outvol1d; outvol1d = 0;}
            if(outsz) {delete []outsz; outsz = 0;}
        }
    }
    v3d_msg(QString("nt_list size is %1").arg(nt_list.size()),0);

    QList<NeuronSWC> merge_result;
    QString outfileName = PARA.inimg_file + "_consesus.swc";
    if (!consensus_skeleton_match_center(nt_list, merge_result, 3,6, 0, callback))
    {
        v3d_msg("error in consensus_skeleton",bmenu);
        return;
    }
    export_listNeuron_2swc(merge_result,qPrintable(outfileName));

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(outfileName.toStdString().c_str()),bmenu);

    return;
}

NeuronTree swc_rotation(NeuronTree nt, V3DLONG *in_sz, double degree)
{
    double alpha = -degree/180.0*3.141592635;
    double xc = (in_sz[0]-1.0)/2, yc = (in_sz[1]-1.0)/2;
    V3DLONG nx = in_sz[0];
    V3DLONG ny = in_sz[1];

    struct PixelPos{double x, y;};


    PixelPos e00, e01, e10, e11; //for the four corners
    e00.x = 0 - xc;
    e00.y = 0 - yc;

    e01.x = 0 - xc;
    e01.y = (ny-1) - yc;

    e10.x = (nx-1) - xc;
    e10.y = 0 - yc;

    e11.x = (nx-1) - xc;
    e11.y = (ny-1) - yc;

    double c00, c01, c10, c11;
    c00 = cos(alpha);
    c01 = sin(alpha);
    c10 = -sin(alpha);
    c11 = cos(alpha);

    PixelPos e00t, e01t, e10t, e11t; //the coordinates of the transformed corners
    e00t.x = c00*e00.x + c01*e00.y;
    e00t.y = c10*e00.x + c11*e00.y;

    e01t.x = c00*e01.x + c01*e01.y;
    e01t.y = c10*e01.x + c11*e01.y;

    e10t.x = c00*e10.x + c01*e10.y;
    e10t.y = c10*e10.x + c11*e10.y;

    e11t.x = c00*e11.x + c01*e11.y;
    e11t.y = c10*e11.x + c11*e11.y;

    double px_min = local_min(local_min(local_min(e00t.x, e01t.x), e10t.x), e11t.x);
    double py_min = local_min(local_min(local_min(e00t.y, e01t.y), e10t.y), e11t.y);

    double c00b, c01b, c10b, c11b;
    c00b = cos(-alpha); //c00b=(c00b<my_eps)?0:c00b; c00b=(c00b>1-my_eps)?1:c00b;
    c01b = sin(-alpha); //c01b=(c01b<my_eps)?0:c01b; c01b=(c01b>1-my_eps)?1:c01b;
    c10b = -sin(-alpha);//c10b=(c10b<my_eps)?0:c10b; c10b=(c10b>1-my_eps)?1:c10b;
    c11b = cos(-alpha); //c11b=(c11b<my_eps)?0:c11b; c11b=(c11b>1-my_eps)?1:c11b;


    QList<NeuronSWC> list = nt.listNeuron;

    //NeutronTree structure
    NeuronTree nt_rotated;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    //set node

    NeuronSWC S;
    for (int i=0;i<list.size();i++)
    {
        NeuronSWC curr = list.at(i);
        S.n 	= curr.n;
        S.type 	= curr.type;
        S.x 	= c00*(curr.x+px_min) + c01*(curr.y+py_min) + xc;
        S.y 	= c10*(curr.x+px_min) + c11*(curr.y+py_min) + yc;
        S.z 	= curr.z;
        S.r 	= curr.r;
        S.pn 	= curr.pn;
        listNeuron.append(S);
        hashNeuron.insert(S.n, listNeuron.size()-1);

    }
    nt_rotated.n = -1;
    nt_rotated.on = true;
    nt_rotated.listNeuron = listNeuron;
    nt_rotated.hashNeuron = hashNeuron;

    return nt_rotated;
}
