/* APP2_ported_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-2-13 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "APP2_ported_plugin.h"

#include "vn_imgpreprocess.h"
#include "fastmarching_dt.h"
#include "fastmarching_tree.h"
#include "hierarchy_prune.h"
#include "marker_radius.h"



Q_EXPORT_PLUGIN2(APP2_ported, APP2_ported);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList APP2_ported::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList APP2_ported::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void APP2_ported::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by Zhi Zhou, 2015-2-13"));
	}
}

bool APP2_ported::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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


		printf("**** Usage of APP2_ported tracing **** \n");
		printf("vaa3d -x APP2_ported -f tracing_func -i <inimg_file> -p <channel> <other parameters>\n");
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

        if(p4DImage->getDatatype()!=V3D_UINT8)
        {
            QMessageBox::information(0, "", "Please convert the image to be UINT8 and try again!");
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

        if(datatype !=1)
        {
            fprintf (stderr, "Please convert the image to be UINT8 and try again!\n");
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

    V3DLONG pagesz = N*M*P;
    unsigned char *data1d_1ch;
    try {data1d_1ch = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for data1d_1ch."); return;}

    for(V3DLONG i = 0; i < pagesz; i++)
        data1d_1ch[i] = data1d[i+(c-1)*pagesz];

    Image4DSimple * p4dImageNew = 0;
    p4dImageNew = new Image4DSimple;

    if(!p4dImageNew->createImage(N,M,P,1, V3D_UINT8))
        return;

    memcpy(p4dImageNew->getRawData(), data1d_1ch, pagesz);

    unsigned char * indata1d = p4dImageNew->getRawDataAtChannel(0);

    in_sz[3] = 1;
    double dfactor_xy = 1, dfactor_z = 1;

    if (in_sz[0]<=256 && in_sz[1]<=256 && in_sz[2]<=256)
    {
        dfactor_z = dfactor_xy = 1;
    }
    else if (in_sz[0] >= 2*in_sz[2] || in_sz[1] >= 2*in_sz[2])
    {
        if (in_sz[2]<=256)
        {
            double MM = in_sz[0];
            if (MM<in_sz[1]) MM=in_sz[1];
            dfactor_xy = MM / 256.0;
            dfactor_z = 1;
        }
        else
        {
            double MM = in_sz[0];
            if (MM<in_sz[1]) MM=in_sz[1];
            if (MM<in_sz[2]) MM=in_sz[2];
            dfactor_xy = dfactor_z = MM / 256.0;
        }
    }
    else
    {
        double MM = in_sz[0];
        if (MM<in_sz[1]) MM=in_sz[1];
        if (MM<in_sz[2]) MM=in_sz[2];
        dfactor_xy = dfactor_z = MM / 256.0;
    }

    printf("dfactor_xy=%5.3f\n", dfactor_xy);
    printf("dfactor_z=%5.3f\n", dfactor_z);

    if (dfactor_z>1 || dfactor_xy>1)
    {
        v3d_msg("enter ds code", 0);

        V3DLONG out_sz[4];
        unsigned char * outimg=0;
        if (!downsampling_img_xyz( indata1d, in_sz, dfactor_xy, dfactor_z, outimg, out_sz))
            return;

        p4dImageNew->setData(outimg, out_sz[0], out_sz[1], out_sz[2], out_sz[3], V3D_UINT8);

        indata1d = p4dImageNew->getRawDataAtChannel(0);
        in_sz[0] = p4dImageNew->getXDim();
        in_sz[1] = p4dImageNew->getYDim();
        in_sz[2] = p4dImageNew->getZDim();
        in_sz[3] = p4dImageNew->getCDim();
    }

    vector<MyMarker *> outtree;

    cout<<"Start detecting cellbody"<<endl;

    float * phi = 0;
    vector<MyMarker> inmarkers;

    fastmarching_dt_XY(indata1d, phi, in_sz[0], in_sz[1], in_sz[2],2, 10);

    V3DLONG sz0 = in_sz[0];
    V3DLONG sz1 = in_sz[1];
    V3DLONG sz2 = in_sz[2];
    V3DLONG sz01 = sz0 * sz1;
    V3DLONG tol_sz = sz01 * sz2;

    V3DLONG max_loc = 0;
    double max_val = phi[0];
    for(V3DLONG i = 0; i < tol_sz; i++)
    {
        if(phi[i] > max_val)
        {
            max_val = phi[i];
            max_loc = i;
        }
    }
    MyMarker max_marker(max_loc % sz0, max_loc % sz01 / sz0, max_loc / sz01);
    inmarkers.push_back(max_marker);

    cout<<"======================================="<<endl;
    cout<<"Construct the neuron tree"<<endl;

    v3d_msg("8bit", 0);

    fastmarching_tree(inmarkers[0], indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], 2, 10, false);
    cout<<"======================================="<<endl;

    //save a copy of the ini tree
    cout<<"Save the initial unprunned tree"<<endl;
    vector<MyMarker*> & inswc = outtree;

    if (1)
    {
        V3DLONG tmpi;

        vector<MyMarker*> tmpswc;
        for (tmpi=0; tmpi<inswc.size(); tmpi++)
        {
            MyMarker * curp = new MyMarker(*(inswc[tmpi]));
            tmpswc.push_back(curp);

            if (dfactor_xy>1) inswc[tmpi]->x *= dfactor_xy;
            inswc[tmpi]->x += (0);
            if (dfactor_xy>1) inswc[tmpi]->x += dfactor_xy/2;

            if (dfactor_xy>1) inswc[tmpi]->y *= dfactor_xy;
            inswc[tmpi]->y += (0);
            if (dfactor_xy>1) inswc[tmpi]->y += dfactor_xy/2;

            if (dfactor_z>1) inswc[tmpi]->z *= dfactor_z;
            inswc[tmpi]->z += (0);
            if (dfactor_z>1)  inswc[tmpi]->z += dfactor_z/2;
        }

        saveSWC_file(QString(PARA.inimg_file).append("_ini.swc").toStdString(), inswc);

        for (tmpi=0; tmpi<inswc.size(); tmpi++)
        {
            inswc[tmpi]->x = tmpswc[tmpi]->x;
            inswc[tmpi]->y = tmpswc[tmpi]->y;
            inswc[tmpi]->z = tmpswc[tmpi]->z;
        }

        for(tmpi = 0; tmpi < tmpswc.size(); tmpi++)
            delete tmpswc[tmpi];
        tmpswc.clear();
    }

    cout<<"Pruning neuron tree"<<endl;

    vector<MyMarker*> outswc;
    v3d_msg("start to use happ.\n", 0);
    happ(inswc, outswc, indata1d, in_sz[0], in_sz[1], in_sz[2],10, 5, 0.3333);

    if (p4dImageNew) {delete p4dImageNew; p4dImageNew=0;} //free buffe

    inmarkers[0].x *= dfactor_xy;
    inmarkers[0].y *= dfactor_xy;
    inmarkers[0].z *= dfactor_z;


    for(V3DLONG i = 0; i < outswc.size(); i++)
    {
        if (dfactor_xy>1) outswc[i]->x *= dfactor_xy;
        outswc[i]->x += 0;
        if (dfactor_xy>1) outswc[i]->x += dfactor_xy/2;

        if (dfactor_xy>1) outswc[i]->y *= dfactor_xy;
        outswc[i]->y += 0;
        if (dfactor_xy>1) outswc[i]->y += dfactor_xy/2;

        if (dfactor_z>1) outswc[i]->z *= dfactor_z;
        outswc[i]->z += 0;
        if (dfactor_z>1)  outswc[i]->z += dfactor_z/2;

        outswc[i]->radius *= dfactor_xy; //use xy for now
    }

    //re-estimate the radius using the original image
    double real_thres = 40;

    V3DLONG szOriginalData[4] = {N,M,P, 1};

    int method_radius_est = 2;
    for(V3DLONG i = 0; i < outswc.size(); i++)
    {
        //printf(" node %ld of %ld.\n", i, outswc.size());
        outswc[i]->radius = markerRadius(data1d_1ch, szOriginalData, *(outswc[i]), real_thres, method_radius_est);
    }
    //Output

   QString swc_name = PARA.inimg_file + "_APP2_ported.swc";
   // NeuronTree nt;
   // nt.name = "tracing method";
   // writeSWC_file(swc_name.toStdString().c_str(),nt);

    saveSWC_file(swc_name.toStdString(), outswc);


    if(phi){delete [] phi; phi = 0;}
    for(V3DLONG i = 0; i < outtree.size(); i++) delete outtree[i];
    outtree.clear();
    if(data1d_1ch){delete []data1d_1ch; data1d_1ch = 0;}


    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
