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
#include "utils/matmath.h"
#include "PressureSampler.h"

//#include "../../v3d_main/basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions
using namespace std;
typedef vector<float> vectype;

ImageOperation *IM;
//PressureSampler *p;

Q_EXPORT_PLUGIN2(NeuroStalker, NeuroStalker);

using namespace std;
const int ForegroundThreshold = 30; // The threshold used in APP2

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    int preprocessing; // 2 : downsample the image within 256*256*256; 1: Crop the image; 0: keep the original image
    int unittest; // 2 : Run Unit-Test; 1: Run Tracing; 0: Run Nothing
};


void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
unsigned char * downsample(V3DLONG* in_sz, V3DLONG c, unsigned char* data1d, V3DLONG * downsz);
unsigned char * crop(const V3DLONG in_sz[4], unsigned char *data1d, V3DLONG sz_img_crop[4], vectype * boxlowsize);
LabelImagePointer DeriveForegroundLabelImage(const ImagePointer I, const int threshold);
void TraceReal(ImagePointer OriginalImage, GradientImagePointer GVF, LabelImagePointer wallimg,
 PointList3D seeds, vectype * xpfinal, vectype * ypfinal, vectype * zpfinal, vectype * pn, vectype * rfinal, vectype * sn);

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
        PARA.channel = 1;
        PARA.preprocessing = 1;
        PARA.unittest = 1;
        cout<<"PARA.channel: "<<PARA.channel<<"PARA.preprocessing: "<<PARA.preprocessing<<"PARA.unittest"
            <<PARA.unittest<<endl;
        reconstruction_func(callback,parent,PARA,bmenu);
     //   v3d_msg(tr("This is working or after?"));
    }
    else
    {
        v3d_msg(tr("A tracing algorithm. "
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
    Image4DSimple* p4DImage;

    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        p4DImage = callback.getImage(curwin);

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

    if (PARA.unittest & 2)
    {
        cout<<"+++++ Running Unit-Tests +++++"<<endl;
        //TestRadius(data1d, in_sz);
    }

    // ------- Main neuron reconstruction code

    // Crop The image
    vectype boxlowsize;
    if (PARA.preprocessing & 1)
    {
        cout<<"=============== Cropping the image ==============="<<endl;
        V3DLONG sz_img_crop[4];
        unsigned char *p_img8u_crop = crop(in_sz, data1d, sz_img_crop, &boxlowsize);
        cout<<"boxlowsize: "<<boxlowsize[0]<<"boxlowsize: "<<boxlowsize[1]<<"boxlowsize: "<<boxlowsize[2]<<endl;    
        //cout<<"Saving cropped image to downsample.v3draw"<<endl;
        //saveImage("test/cropoutside.v3draw", p_img8u_crop, sz_img_crop, V3D_UINT8);
        if(!bmenu)
        {
            if (data1d) delete [] data1d;
        }
        data1d = p_img8u_crop;

        for (int i=0; i<4; i++){
            in_sz[i] = sz_img_crop[i];
        }
        cout<<"boxlowsize: "<<boxlowsize[0]<<"boxlowsize: "<<boxlowsize[1]<<"boxlowsize: "<<boxlowsize[2]<<endl;    

        cout<<"=============== Image Cropped ==============="<<endl;
    }

    // Downsample the image
    if (PARA.preprocessing & 2)
    {
        cout<<"=============== Downsampling the image..."<<endl;
        V3DLONG downsz[4];
        cout<<"Data size before downsample: "<<in_sz[0]<<","<<in_sz[1]<<","<<in_sz[2]<<endl;

        unsigned char* downdata1d = downsample(in_sz, c, data1d, downsz);
        data1d = downdata1d;
        in_sz[0] = downsz[0];
        in_sz[1] = downsz[1];
        in_sz[2] = downsz[2];
        in_sz[3] = downsz[3];
        cout<<"Data size after downsample: "<<in_sz[0]<<","<<in_sz[1]<<","<<in_sz[2]<<endl;
        //cout<<"Saving downsampled image to test/downsample.v3draw"<<endl;
        //saveImage("test/downsample.v3draw", downdata1d, downsz, V3D_UINT8);
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

    // Adaptive thresholding here, may replace with graph cut
    //IM->ImComputeInitBackgroundModel(IM->v_threshold);
    //IM->ImComputeInitForegroundModel();

    // Get the Binary Image
    LabelImagePointer binaryimg = DeriveForegroundLabelImage(IM->I, ForegroundThreshold);

    // Save the binary img to visualise the segmentation
    unsigned short int * binaryimgbuffer =  binaryimg->GetBufferPointer();
    unsigned char * binaryimg2uchar = new unsigned char [in_sz[0]*in_sz[1]*in_sz[2]];
    for (int i = 0; i < in_sz[0]*in_sz[1]*in_sz[2]; i++)
    {
        binaryimg2uchar[i] = (unsigned char) ((double)(binaryimgbuffer[i]) * 255.0);
    }
    //saveImage("test/binaryimage.v3draw", binaryimg2uchar, in_sz, V3D_UINT8);

    vectype xpfinal, ypfinal, zpfinal, rfinal, pn, sn;

    // ------- Run Unit-Tests
    if (PARA.unittest & 2){
        cout<<"+++++ Running Unit-Tests +++++"<<endl;
        TestMatMath();
        TestPressureSampler(IM->I, IM->IGVF, binaryimg, IM->SeedPt, &xpfinal, &ypfinal, &zpfinal, &pn,  &rfinal, &sn);
        cout<<"All Tests Finished!!!!!!! G'Day!!"<<endl;
    }

    if (PARA.unittest & 1) 
    {
        //PressureSampler p(100, 100, IM->I, IM->IGVF, 10);
        TraceReal(IM->I, IM->IGVF, binaryimg, IM->SeedPt, &xpfinal, &ypfinal, &zpfinal, &pn,  &rfinal, &sn);
    }
    cout<<"pn size: "<<pn.size()<<" xpoint size: "<<xpfinal.size()<<" ypoint size: "
            <<ypfinal.size()<<" zpoint size: "<<zpfinal.size()<<" rpoint size: "<<rfinal.size()<<endl; 
    //Output
    NeuronTree nt;
    QList <NeuronSWC> listNeuron;
    QHash <int, int> hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    NeuronSWC S;
    for (int i = 0; i < ypfinal.size(); i++)
    {
        S.n = sn[i];
        S.type = 7;
        S.x = xpfinal[i] + boxlowsize[0];
        S.y = ypfinal[i] + boxlowsize[1];
        S.z = zpfinal[i] + boxlowsize[2];
        S.r = rfinal[i];
        S.pn = pn[i];
        listNeuron.append(S);
        hashNeuron.insert(S.n, listNeuron.size() - 1);       
    }
    nt.n = -1;
    nt.on = true;
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;

    QString swc_name = PARA.inimg_file + "_NeuroStalker.swc";
    nt.name = "NeuroStalker";
    writeSWC_file(swc_name.toStdString().c_str(), nt);

    if(!bmenu)
    {
        if(data1d) {delete [] data1d; data1d = 0;}
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


unsigned char * crop(const V3DLONG in_sz[4], unsigned char *data1d, V3DLONG sz_img_crop[4], vectype * boxlowsize)
{    
    printf("1. Find the bounding box and crop image. \n");
    V3DLONG V3DLONG l_boundbox_min[3], l_boundbox_max[3];//xyz
    V3DLONG l_npixels_crop;
    
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
    for(V3DLONG X=0;X<in_sz[0];X++)
        for(V3DLONG Y=0;Y<in_sz[1];Y++)
            for(V3DLONG Z=0;Z<in_sz[2];Z++)
            {
                if(p_img8u_3d[Z][Y][X]>0.1)
                {
                    if(l_boundbox_min[0] > X) l_boundbox_min[0] = X;    if(l_boundbox_max[0] < X) l_boundbox_max[0] = X;
                    if(l_boundbox_min[1] > Y) l_boundbox_min[1] = Y;    if(l_boundbox_max[1] < Y) l_boundbox_max[1] = Y;
                    if(l_boundbox_min[2] > Z) l_boundbox_min[2] = Z;    if(l_boundbox_max[2] < Z) l_boundbox_max[2] = Z;
                }
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

    for(V3DLONG Z = 0;Z < sz_img_crop[2];Z++)
        for(V3DLONG Y = 0;Y < sz_img_crop[1];Y++)
            for(V3DLONG X = 0;X < sz_img_crop[0];X++)
            {
                *p_tmp = p_img8u_3d[Z+l_boundbox_min[2]][Y+l_boundbox_min[1]][X+l_boundbox_min[0]];
                p_tmp++;
            }

    if(p_img8u_3d) {delete3dpointer(p_img8u_3d, in_sz[0], in_sz[1], in_sz[2]);}

    //saveImage("test/cropinside.v3draw", p_img8u_crop, sz_img_crop, V3D_UINT8);
    (*boxlowsize).push_back(float(l_boundbox_min[0]));
    (*boxlowsize).push_back(float(l_boundbox_min[1]));
    (*boxlowsize).push_back(float(l_boundbox_min[2]));
    return p_img8u_crop;
 }   


LabelImagePointer DeriveForegroundLabelImage(const ImagePointer I, const int threshold)
{   itk::ImageLinearConstIteratorWithIndex<ImageType> originitr(I, I->GetRequestedRegion());
    originitr.SetDirection(2);
    originitr.GoToBegin();
    ImageType::SizeType originsize = I->GetLargestPossibleRegion().GetSize();

    LabelImagePointer pBinaryImage = LabelImageType::New();
    LabelImageType::SizeType size;
    size[0] = originsize[0];
    size[1] = originsize[1];
    size[2] = originsize[2];
    LabelImageType::IndexType idx;
    idx.Fill(0);
    LabelImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(idx);
    pBinaryImage->SetRegions(region);
    pBinaryImage->Allocate();
    pBinaryImage->FillBuffer(0);
    itk::ImageLinearIteratorWithIndex<LabelImageType> 
                        binaryitr(pBinaryImage, pBinaryImage->GetRequestedRegion());
    binaryitr.SetDirection(2);
    binaryitr.GoToBegin();

    while( !originitr.IsAtEnd() && !binaryitr.IsAtEnd())
    {
        while(!originitr.IsAtEndOfLine()){
            if (originitr.Get() > threshold){
                binaryitr.Set(1);
            }
            ++originitr;
            ++binaryitr;
        }
        originitr.NextLine();
        binaryitr.NextLine();
    }
    
    return pBinaryImage;
}

void TraceReal(ImagePointer OriginalImage, GradientImagePointer GVF, LabelImagePointer wallimg,
 PointList3D seeds, vectype * xpfinal, vectype * ypfinal, vectype * zpfinal, vectype * pn, vectype * rfinal, vectype * sn)
{
    #define INF 1E9
    int nseed = seeds.GetLength();
    vectype seedx, seedy, seedz;
    LabelImageType::IndexType binaryidx;
    int M = wallimg->GetLargestPossibleRegion().GetSize()[0];
    int N = wallimg->GetLargestPossibleRegion().GetSize()[1];
    int Z = wallimg->GetLargestPossibleRegion().GetSize()[2];

    for (int i=0; i<nseed; i++)
    {
        if (seeds.Pt[i].x < 0 || seeds.Pt[i].x > M || seeds.Pt[i].x != seeds.Pt[i].x || 
            seeds.Pt[i].y < 0 || seeds.Pt[i].y > N || seeds.Pt[i].y != seeds.Pt[i].y ||
            seeds.Pt[i].z < 0 || seeds.Pt[i].z > Z || seeds.Pt[i].z != seeds.Pt[i].z) 
            continue;
        binaryidx[0] = (int)seeds.Pt[i].x;
        binaryidx[1] = (int)seeds.Pt[i].y;
        binaryidx[2] = (int)seeds.Pt[i].z;
        unsigned short p = wallimg->GetPixel(binaryidx);
        if ( p != 0) 
        {
            seedx.push_back(seeds.Pt[i].x); 
            seedy.push_back(seeds.Pt[i].y); 
            seedz.push_back(seeds.Pt[i].z); 
        }
    }
    if (seedx.size() > 1000) 
        {
            seedadjust(&seedx, &seedy, &seedz);
        }
    int step = 1;
    int ndir = 100;
    char mpfiletitle[80];        
    vectype xpoint, ypoint, zpoint, rpoint;
    LabelImageType::IndexType wallfilteridx;
    unsigned short filter;
    if (seedx.size() < 2000)
    {
        //cout<<"seedx size: "<<seedx.size()<<endl;
        for (int j = 0; j < seedx.size(); j++)
            {
                PressureSampler p(ndir, 100, OriginalImage, GVF, 10);
                p.UpdatePosition(seedx[j], seedy[j], seedz[j]);
                //cout<<"Visualising Seed: "<<j<<" -- "<<seedx[j]<<","<<seedy[j]<<","<<seedz[j]<<endl;
                for (int i = 1; i < 20; i++)
                    {
                        //cout<<"RandSample stage: "<<endl;
                        p.RandSample();
                        //cout<<"NextMove stage: "<<endl;
                        p.NextMove(1.1);
                        //cout<<"push_back stage: "<<endl;
                        wallfilteridx[0] = int (constrain((p.x), 1, M - 1));
                        wallfilteridx[1] = int (constrain((p.y), 1, N - 1));
                        wallfilteridx[2] = int (constrain((p.z), 1, Z - 1));
                        filter = wallimg->GetPixel(wallfilteridx);
                        if (filter != 0)
                        {
                            xpoint.push_back(p.x);
                            ypoint.push_back(p.y);
                            zpoint.push_back(p.z);
                            p.GetRadius();
                            rpoint.push_back(p.radius);
                            //cout<<"filter work or not: "<<endl;
                        }
                    }
            }
    }

    int edgesize = xpoint.size();
    float** edgemap = new float*[edgesize];
    for(int i = 0; i < edgesize; i++)
    {
        edgemap[i] = new float[edgesize];

    }
    for (int edgei = 0; edgei < edgesize; edgei++)
        {
            for(int edgej = 0; edgej < edgesize; edgej++)
                {
                    edgemap[edgei][edgej] = (xpoint[edgei] - xpoint[edgej]) *(xpoint[edgei] - xpoint[edgej]) +
                    (ypoint[edgei] - ypoint[edgej]) *(ypoint[edgei] - ypoint[edgej]) +
                    (zpoint[edgei] - zpoint[edgej]) *(zpoint[edgei] - zpoint[edgej]);
                } 

        }
    int* pi = new int[edgesize];
    for(int i = 0; i< edgesize;i++)
    {
        pi[i] = 0;
    }
    pi[0] = 1;
    int indexi, indexj;
    //vectype pn, xpfinal, ypfinal, zpfinal, rfinal;
    (*pn).push_back(-1);
    (*xpfinal).push_back(xpoint[0]);
    (*ypfinal).push_back(ypoint[0]);
    (*zpfinal).push_back(zpoint[0]);
    (*rfinal).push_back(rpoint[0]);
    (*sn).push_back(1);
    for(int loop = 0; loop<edgesize;loop++)
        {
            double min = INF;
            for(int i = 0; i<edgesize; i++)
              {
                if (pi[i] == 1)
                {
                    for(int j = 0;j<edgesize; j++)
                    {
                        if(pi[j] == 0 && min > edgemap[i][j])
                        {
                            min = edgemap[i][j];
                            indexi = i;
                            indexj = j;
                        }
                    }
                }

              }
              if(indexi>=0)
              {
                (*pn).push_back(indexi+1);
                (*xpfinal).push_back(xpoint[indexj]);
                (*ypfinal).push_back(ypoint[indexj]);
                (*zpfinal).push_back(zpoint[indexj]);
                (*rfinal).push_back(rpoint[indexj]);
                (*sn).push_back(indexj+1);
              }else
              {
                  break;
              }
            pi[indexj] = 1;
            indexi = -1;
            indexj = -1;
        }
}
