/* NeuroStalker_plugin.cpp
 * 
 * 2015-4-25 : by Siqi Liu, Donghao Zhang
 * WIP!
 */
 
#include <iostream>
#include <vector>
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "utils/vn_imgpreprocess.h"
#include "stackutil.h"

#include "NeuroStalker_plugin.h"
#include "lib/ImageOperation.h"
#include "test/unittest.h"
//#include "../../v3d_main/basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions

ImageOperation *IM;

Q_EXPORT_PLUGIN2(NeuroStalker, NeuroStalker);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    int preprocessing; // 2 : downsample the image within 256*256*256; 1: Crop the image; 0: keep the original image
    int unittest; // 2 : Run Unit-Test; 1: Run Tracing; 0: Run Nothing
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
unsigned char * downsample(V3DLONG* in_sz, V3DLONG c, unsigned char* data1d, V3DLONG * downsz);
unsigned char * cropfunc(const V3DLONG in_sz[4], unsigned char *data1d, V3DLONG sz_img_crop[4]);

QStringList NeuroStalker::menulist() const
{
    return QStringList() 
        <<tr("tracing_menu")
        <<tr("about");
}

QStringList NeuroStalker::funclist() const
{
    return QStringList()
        <<tr("tracing_func")
        <<tr("help");
}

void NeuroStalker::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("tracing_menu"))
    {
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);

    }
    else
    {
        v3d_msg(tr("A learning-based tracing algorithm. "
            "Developed by Siqi Liu, Donghao Zhang, 2015-4-25"));
    }
}


bool NeuroStalker::dofunc(const QString & func_name, 
                          const V3DPluginArgList & input,
                          V3DPluginArgList & output, 
                          V3DPluginCallback2 & callback,  
                          QWidget * parent)
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
        PARA.preprocessing = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        PARA.unittest = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        reconstruction_func(callback,parent,PARA,bmenu);
    }
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN

        printf("**** Usage of NeuroStalker tracing **** \n");
        printf("vaa3d -x NeuroStalker -f tracing_func -i <inimg_file> -p <channel> <preprocessing> <run unit-tests>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("preprocessing    The preprocessing flag - 1: Crop Only; 2: Downsample; 3: Downsample and crop; \n");
        printf("run unit-tests   - 1: Run Tracing Only; 2: Run unit-tests only; 3: Run Both Unit Tests and Tracing; \n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

    }
    else return false;

    return true;
}

void reconstruction_func(V3DPluginCallback2 &callback, 
                         QWidget *parent, 
                         input_PARA &PARA, 
                         bool bmenu)
{
    cout<<"Welcome to NeuroStalker!!"<<endl;
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
        if(p4DImage) {delete p4DImage;};
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

    // ------- Run Unit-Tests
    if (PARA.unittest & 2){
        cout<<"+++++ Running Unit-Tests +++++"<<endl;
        TestMatMath();
    }

    if (!(PARA.unittest & 1)) return;

    // ------- Main neuron reconstruction code
    // Crop The image
    if (PARA.preprocessing & 1)
    {
        cout<<"=============== Cropping the image ==============="<<endl;
        V3DLONG sz_img_crop[4];
        unsigned char *p_img8u_crop = cropfunc(in_sz, data1d, sz_img_crop);    
        cout<<"Saving cropped image to downsample.v3draw"<<endl;
        saveImage("test/testdata/cropoutside.v3draw", p_img8u_crop, sz_img_crop, V3D_UINT8);
        if (data1d) delete [] data1d;
        data1d = p_img8u_crop;

        for (int i=0; i<4; i++){
            in_sz[i] = sz_img_crop[i];
        }
        cout<<"=============== Image Cropped ==============="<<endl;
    }

    // Downsample the image
    if (PARA.preprocessing & 2)
    {
        cout<<"=============== Downsampling the image..."<<endl;
        V3DLONG downsz[4];
        cout<<"Data size before downsample: "<<in_sz[0]<<","<<in_sz[1]<<","<<in_sz[2]<<endl;

        unsigned char* downdata1d = downsample(in_sz, c, data1d, downsz);
        cout<<"Data size after downsample: "<<in_sz[0]<<","<<in_sz[1]<<","<<in_sz[2]<<endl;
        cout<<"Saving downsampled image to test/testdata/downsample.v3draw"<<endl;
        saveImage("downsample.v3draw", downdata1d, downsz, V3D_UINT8);
        cout<<"=============== Image Downsampled..."<<endl;
    }

    // Using the Image Operation found in vaa3d_tools/hackathon/zhi/snake_tracing/TracingCore/ in for some simple Image Processing
    IM = new ImageOperation;

    // Imcreate takes in_sz with int*
    int in_sz_int[4];

    for(int i = 0; i < 4; i++)
    {
        in_sz_int[i] = (int)in_sz[i];
    }

    
    // Preprocessing
    IM->Imcreate(data1d, in_sz_int);
    std::cout<<"=== Compute Gradient Vector Flow..."<<std::endl;
    IM->computeGVF(1000, 5, 1);
    std::cout<<"=== Compute Vesselness (CPU)..."<<std::endl;
    IM->ComputeGVFVesselness();
    std::cout<<"=== Detect Seed Points..."<<std::endl;
    IM->SeedDetection(IM->v_threshold, 0, 0);
    std::cout<<"=== Adjust Seed Points..."<<std::endl;
    IM->SeedAdjustment(10);
    std::cout<<"=== Preprocessing Finished..."<<std::endl;

    // Adaptive Tracing here, may replace with graph cut
    IM->ImComputeInitBackgroundModel(IM->v_threshold);
    IM->ImComputeInitForegroundModel();
    
    //Output
    NeuronTree nt;
    QString swc_name = PARA.inimg_file + "_NeuroStalker.swc";
    nt.name = "NeuroStalker";
    writeSWC_file(swc_name.toStdString().c_str(), nt);

    if(!bmenu)
    {
        if(data1d) {delete [] data1d; data1d = 0;}
        //if(p_img8u_crop) {delete [] p_img8u_crop;  p_img8u_crop = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);
    return;
}


unsigned char * downsample(V3DLONG *in_sz,
                           V3DLONG c, 
                           unsigned char* data1d, 
                           V3DLONG * downsz)
{
    V3DLONG N, M, P;
    N = in_sz[0];
    M = in_sz[1];
    P = in_sz[2];

    // --- Downsample the image
    V3DLONG pagesz = N*M*P;
    unsigned char *data1d_1ch;
    try {data1d_1ch = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for data1d_1ch."); return NULL;}

    for(V3DLONG i = 0; i < pagesz; i++)
        data1d_1ch[i] = data1d[i+(c-1)*pagesz];

    Image4DSimple * p4dImageNew = 0;
    p4dImageNew = new Image4DSimple;

    if(!p4dImageNew->createImage(N,M,P,1, V3D_UINT8))
        return NULL;

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
        {
            cout<<"=== Downsample Failed!!"<<endl;
            return NULL;
        }

        p4dImageNew->setData(outimg, out_sz[0], out_sz[1], out_sz[2], out_sz[3], V3D_UINT8);

        indata1d = p4dImageNew->getRawDataAtChannel(0);
        downsz[0] = p4dImageNew->getXDim();
        downsz[1] = p4dImageNew->getYDim();
        downsz[2] = p4dImageNew->getZDim();
        downsz[3] = p4dImageNew->getCDim();
    }

    return indata1d;
}


unsigned char * cropfunc(const V3DLONG in_sz[4], unsigned char *data1d, V3DLONG sz_img_crop[4])
{    
    printf("1. Find the bounding box and crop image. \n");
    V3DLONG long l_boundbox_min[3], l_boundbox_max[3];//xyz
    long l_npixels_crop;
    
    //find bounding box
    unsigned char ***p_img8u_3d = 0;
    if(!new3dpointer(p_img8u_3d ,in_sz[0], in_sz[1], in_sz[2], data1d))
    {
        printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
        if(p_img8u_3d) {delete3dpointer(p_img8u_3d, in_sz[0], in_sz[1], in_sz[2]);}
    }
    printf("boundingbox x dimension: %d,y dimension: %d,z dimension: %d.\n", in_sz[0], in_sz[1], in_sz[2]);
    l_boundbox_min[0] = in_sz[0];  l_boundbox_min[1] = in_sz[1];  l_boundbox_min[2] = in_sz[2];
    l_boundbox_max[0] = 0;                l_boundbox_max[1] = 0;                l_boundbox_max[2] = 0;
    for(long X=0;X<in_sz[0];X++)
        for(long Y=0;Y<in_sz[1];Y++)
            for(long Z=0;Z<in_sz[2];Z++)
                if(p_img8u_3d[Z][Y][X]>0.1)
                {
                    if(l_boundbox_min[0] > X) l_boundbox_min[0] = X;    if(l_boundbox_max[0] < X) l_boundbox_max[0] = X;
                    if(l_boundbox_min[1] > Y) l_boundbox_min[1] = Y;    if(l_boundbox_max[1] < Y) l_boundbox_max[1] = Y;
                    if(l_boundbox_min[2] > Z) l_boundbox_min[2] = Z;    if(l_boundbox_max[2] < Z) l_boundbox_max[2] = Z;
                }
    printf(">>boundingbox: x[%ld~%ld],y[%ld~%ld],z[%ld~%ld]\n",l_boundbox_min[0], l_boundbox_max[0],
                                                               l_boundbox_min[1], l_boundbox_max[1],
                                                               l_boundbox_min[2], l_boundbox_max[2]);

    //crop image
    sz_img_crop[0] = l_boundbox_max[0] - l_boundbox_min[0] + 1;
    sz_img_crop[1] = l_boundbox_max[1] - l_boundbox_min[1] + 1;
    sz_img_crop[2] = l_boundbox_max[2] - l_boundbox_min[2] + 1;
    sz_img_crop[3] = 1;
    l_npixels_crop = sz_img_crop[0] * sz_img_crop[1] * sz_img_crop[2];

    unsigned char *p_img8u_crop = new(std::nothrow) unsigned char[l_npixels_crop]();
    if(!p_img8u_crop)
    {
        printf("ERROR: Fail to allocate memory for p_img32f_crop!\n");
        if(p_img8u_3d)              {delete3dpointer(p_img8u_3d, in_sz[0], in_sz[1], in_sz[2]);}
    }
    unsigned char *p_tmp = p_img8u_crop;
    for(long Z = 0;Z < sz_img_crop[2];Z++)
        for(long Y = 0;Y < sz_img_crop[1];Y++)
            for(long X = 0;X < sz_img_crop[0];X++)
            {
                *p_tmp = p_img8u_3d[Z+l_boundbox_min[2]][Y+l_boundbox_min[1]][X+l_boundbox_min[0]];
                p_tmp++;
            }
    if(p_img8u_3d) {delete3dpointer(p_img8u_3d, in_sz[0], in_sz[1], in_sz[2]);}
    saveImage("test/testdata/cropinside.v3draw", p_img8u_crop, sz_img_crop, V3D_UINT8);

    return p_img8u_crop;
 }   
