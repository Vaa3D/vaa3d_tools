#include "branches_detection.h"
#include "Branches_Detection_plugin.h"
#include "rayshooting_func.h"
#include "v3d_interface.h"
#include "../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h"
//#include "readRawfile_func.h"
#include <iostream>
#include <vector>
using namespace std;
#define PI 3.1415926
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
typedef int BIT32_UNIT;
#define b_VERBOSE_PRINT 0
bool branches_detection(V3DPluginCallback2 &callback, QWidget *parent,BRANCH_LS_PARA &P)
{
 //   v3d_msg("branches_detection");
    NeuronTree nt_terafly = callback.getSWCTeraFly();
    LandmarkList marker_v,marker_rebase;
    for(V3DLONG i=0;i<nt_terafly.listNeuron.size();i=i+20)
    {
        NeuronSWC curr = nt_terafly.listNeuron[i];
        do_each_block_v2(callback,P,curr,nt_terafly);
        LandmarkList marker,marker2;
        do_detection(callback,P,marker);

        cout<<"marker.size = "<<marker.size()<<endl;
        v3d_msg("check marker size");
        cout<<"P"<<P.xb<<"  "<<P.yb<<"  "<<P.zb<<endl;
        for(V3DLONG i=0;i<marker.size();i++)
        {
            marker[i].x = marker[i].x+P.xb;
            marker[i].y = marker[i].y+P.yb;
            marker[i].z = marker[i].z+P.zb;
            //
            //marker[i].color

            cout<<"marker = "<<marker[i].x<<"  "<<marker[i].y<<"  "<<marker[i].y<<"  "<<endl;
        }
        QList<ImageMarker> markerlist;
        QString filename = "save.marker";
        for(V3DLONG i=0;i<marker.size();i++)
        {
            ImageMarker m;
            m.x = marker[i].x;
            m.y = marker[i].y;
            m.z = marker[i].z;
            m.color.a = 0;
            m.color.b = 0;
            //m.color.c = 0;
            m.color.g = 0;
            m.color.r = 0;
            markerlist.push_back(m);
        }
        writeMarker_file(filename,markerlist);

        if(marker_v.size()==0)
        {
            for(V3DLONG k=0;k<marker.size();k++)
            {
                marker_v.push_back(marker2[k]);
            }
        }
        else
        {
            for(V3DLONG j=0;j<marker_v.size();j++)
            {
                cout<<"hahhahaa"<<marker_v.size()<<endl;
                for(V3DLONG k=0;k<marker2.size();k++)
                {
                    double dis = NTDIS(marker2[k],marker_v[j]);
                    cout<<"dis = "<<dis<<endl;
                    if(dis>4)
                    {
                        marker_rebase.push_back(marker2[k]);
                    }
                }
            }
            for(V3DLONG j=0;j<marker_rebase.size();j++)
            {
                marker_v.push_back(marker_rebase[j]);
            }
        }
    }


    if(marker_v.size()>0)
    {
        callback.setLandmarkTeraFly(marker_v);
        return true;
    }
    else
    {
        v3d_msg("this area don't have any branch");return false;
    }

}
bool do_curr_point(V3DPluginCallback2 &callback,NeuronSWC &S,BRANCH_LS_PARA &P,NeuronTree &nt,unsigned char* &data)
{
    double l_x = 256;
    double l_y = 256;
    double l_z = 128;
    V3DLONG xb = S.x-l_x;
    V3DLONG xe = S.x+l_x-1;
    V3DLONG yb = S.y-l_y;
    V3DLONG ye = S.y+l_y-1;
    V3DLONG zb = S.z-l_z;
    V3DLONG ze = S.z+l_z-1;

                cout<<"begin ==================="<<xb<<"  "<<yb<<"  "<<"  "<<zb<<endl;
                cout<<"end   ==================="<<xe<<"  "<<ye<<"  "<<"  "<<ze<<endl;

    unsigned char * im_cropped = 0;
    V3DLONG pagesz;
    V3DLONG im_cropped_sz[4];
    pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
    im_cropped_sz[0] = xe-xb+1;
    im_cropped_sz[1] = ye-yb+1;
    im_cropped_sz[2] = ze-zb+1;
    im_cropped_sz[3] = 1;

    cout<<xb<<"  "<<yb<<"  "<<zb<<endl;
    cout<<xe<<"  "<<ye<<"  "<<ze<<endl;
    cout<<im_cropped_sz[0]<<"  "<<im_cropped_sz[1]<<"  "<<im_cropped_sz[2]<<"  "<<im_cropped_sz[3]<<endl;

    try {im_cropped = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}



    NeuronTree outswc;
    for(V3DLONG j=0;j<nt.listNeuron.size();j++)
    {
        NeuronSWC p;
        if(nt.listNeuron[j].x<xe&&nt.listNeuron[j].x>xb&&nt.listNeuron[j].y<ye&&nt.listNeuron[j].y>yb&&nt.listNeuron[j].z<ze&&nt.listNeuron[j].z>zb)
        {
            p.n = nt.listNeuron[j].n;
            p.x = nt.listNeuron[j].x-xb;
            p.y = nt.listNeuron[j].y-yb;
            p.z = nt.listNeuron[j].z-zb;
            p.type = nt.listNeuron[j].type;
            p.r = nt.listNeuron[j].r;
            p.pn = nt.listNeuron[j].pn;
            outswc.listNeuron.push_back(p);
        }
    }


    im_cropped = callback.getSubVolumeTeraFly(P.inimg_file.toStdString(),xb,xe+1,
                                          yb,ye+1,zb,ze+1);


    QString outimg_file,outswc_file;
    outimg_file = "test.tif";
    outswc_file = "test.swc";
    writeSWC_file(outswc_file,outswc);


    simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
    data = im_cropped;
    P.data1d = im_cropped;
    P.in_sz[0] = 2*l_x;
    P.in_sz[1] = 2*l_y;
    P.in_sz[2] = 2*l_z;
    if(im_cropped) {delete []im_cropped; im_cropped = 0;}
}
bool do_each_block(V3DPluginCallback2 &callback,NeuronTree &curr,BRANCH_LS_PARA &P)
{
    v3d_msg("do_each_block");
    V3DLONG im_cropped_sz[4];


    double max_x = -11111;
    double min_x = 100000000000;
    double max_y = -11111;
    double min_y = 100000000000;
    double max_z = -11111;
    double min_z = 100000000000;

    for(V3DLONG i=0;i<curr.listNeuron.size();i++)
    {
        if(max_x<curr.listNeuron[i].x)
        {
            max_x = curr.listNeuron[i].x;
        }
        if(max_y<curr.listNeuron[i].y)
        {
            max_y = curr.listNeuron[i].y;
        }
        if(max_z<curr.listNeuron[i].z)
        {
            max_z = curr.listNeuron[i].z;
        }

    }
    for(V3DLONG i=0;i<curr.listNeuron.size();i++)
    {
        if(min_x>curr.listNeuron[i].x)
        {
            min_x = curr.listNeuron[i].x;
        }
        if(min_y>curr.listNeuron[i].y)
        {
            min_y = curr.listNeuron[i].y;
        }
        if(min_z>curr.listNeuron[i].z)
        {
            min_z = curr.listNeuron[i].z;
        }
    }
    cout<<max_x<<"  "<<max_y<<"  "<<max_z<<"  "<<endl;
    cout<<min_x<<"  "<<min_y<<"  "<<min_z<<"  "<<endl;
    V3DLONG xb = min_x;
    V3DLONG xe = max_x-1;
    V3DLONG yb = min_y;
    V3DLONG ye = max_y-1;
    V3DLONG zb = min_z;
    V3DLONG ze = max_z-1;









        unsigned char * im_cropped = 0;
        V3DLONG pagesz;

        pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
        im_cropped_sz[0] = xe-xb+1;
        im_cropped_sz[1] = ye-yb+1;
        im_cropped_sz[2] = ze-zb+1;
        im_cropped_sz[3] = 1;



        NeuronTree nt;
        const Image4DSimple *  curr_win = callback.getImageTeraFly();
        double para_ratio_x = curr_win->getRezX()/curr_win->getXDim();
        double para_ratio_y = curr_win->getRezY()/curr_win->getYDim();
        double para_ratio_z = curr_win->getRezZ()/curr_win->getZDim();
        for(V3DLONG i=0;i<curr.listNeuron.size();i++)
        {
            NeuronSWC s;
            s.n = curr.listNeuron[i].n;
            s.x = (curr.listNeuron[i].x-min_x)/para_ratio_x;
            s.y = (curr.listNeuron[i].y-min_y)/para_ratio_y;
            s.z = (curr.listNeuron[i].z-min_z)/para_ratio_z;
            s.pn = curr.listNeuron[i].pn;
            nt.listNeuron.push_back(s);
        }
        for(V3DLONG i = 0;i<nt.listNeuron.size();i++)
        {
            nt.hashNeuron.insert(nt.listNeuron.at(i).n, nt.listNeuron.size()-1);
        }
        P.nt = nt;
        P.inimg_file_2nd = "test.raw";
        P.swcfilename = "test.swc";
        writeSWC_file(P.swcfilename,nt);



        cout<<xb<<"  "<<yb<<"  "<<zb<<endl;
        cout<<xe<<"  "<<ye<<"  "<<ze<<endl;
        cout<<im_cropped_sz[0]<<"  "<<im_cropped_sz[1]<<"  "<<im_cropped_sz[2]<<"  "<<im_cropped_sz[3]<<endl;
        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}


        cout<<"P.inimg_file.toStdString() = "<<P.inimg_file.toStdString()<<endl;
        string name = P.inimg_file.toStdString();

        im_cropped = callback.getSubVolumeTeraFly(name,xb,xe+1,
                                              yb,ye+1,zb,ze+1);


        P.data1d = im_cropped;
        P.in_sz[0] = im_cropped_sz[0];
        P.in_sz[1] = im_cropped_sz[1];
        P.in_sz[2] = im_cropped_sz[2];

        v3d_msg("check!");

       // simple_saveimage_wrapper(callback, P.inimg_file_2nd.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
        return true;


}
bool do_detection(V3DPluginCallback2 &callback,BRANCH_LS_PARA &PA,LandmarkList &curlist)
{
   // v3d_msg("do_detection");
    cout<<"do_detection"<<endl;
    int ray_numbers_2d=64;//
    int ray_length_2d=25;//30
    unsigned char* data1d = 0;

    int dsize;
    int T1=113;
    int thres_2d=25;
    int N,M,P,C;



    data1d = PA.data1d;
    N = PA.in_sz[0];
    M = PA.in_sz[1];
    P = PA.in_sz[2];
    C = 1;

    cout<<N<<"  "<<M<<"  "<<P<<"  "<<C<<endl;
 //   v3d_msg("nmpc");

    QDialog * dialog = new QDialog();


    if(P > 1)
            dialog->setWindowTitle("3D neuron image tip point detection Based on Ray-shooting algorithm");
    else
            dialog->setWindowTitle("2D neuron image tip point detection Based on Ray-shooting algorithm");

QGridLayout * layout = new QGridLayout();

QSpinBox * ray_numbers_2d_spinbox = new QSpinBox();
ray_numbers_2d_spinbox->setRange(1,1000);
ray_numbers_2d_spinbox->setValue(ray_numbers_2d);

QSpinBox * thres_2d_spinbox = new QSpinBox();
thres_2d_spinbox->setRange(-1, 255);
thres_2d_spinbox->setValue(thres_2d);

QSpinBox * ray_length_2d_spinbox = new QSpinBox();
ray_length_2d_spinbox->setRange(1,N);
ray_length_2d_spinbox->setValue(ray_length_2d);

QSpinBox * T1_spinbox = new QSpinBox();
T1_spinbox->setRange(1,255);
T1_spinbox->setValue(T1);


layout->addWidget(new QLabel("ray numbers"),0,0);
layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

layout->addWidget(new QLabel("intensity threshold"),1,0);
layout->addWidget(thres_2d_spinbox, 1,1,1,5);

layout->addWidget(new QLabel("maximum scale"),2,0);
layout->addWidget(ray_length_2d_spinbox, 2,1,1,5);

layout->addWidget(new QLabel("T1"),3,0);
layout->addWidget(T1_spinbox, 3,1,1,5);


QHBoxLayout * hbox2 = new QHBoxLayout();
QPushButton * ok = new QPushButton(" ok ");
ok->setDefault(true);
QPushButton * cancel = new QPushButton("cancel");
hbox2->addWidget(cancel);
hbox2->addWidget(ok);

layout->addLayout(hbox2,6,0,1,6);
dialog->setLayout(layout);
QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

//run the dialog
//v3d_msg("check1");
//if(dialog->exec() != QDialog::Accepted)
//    {
//            if (dialog)
//            {
//                    delete dialog;
//                    dialog=0;
//                    cout<<"delete dialog"<<endl;
//            }
//            return false;
//    }

//get the dialog return values
ray_numbers_2d = ray_numbers_2d_spinbox->value();
thres_2d = thres_2d_spinbox->value();
ray_length_2d = ray_length_2d_spinbox->value();
T1 = T1_spinbox->value();
int BlockSize=ray_length_2d+2;


    dsize=(N+2*BlockSize)*(M+2*BlockSize)*(P+2*BlockSize);
    // segmentation
    int pagesz1 = N*M*P;
    unsigned char *image_binary=0;
    try{image_binary=new unsigned char [pagesz1];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return false;}
    for(int i = 0; i < pagesz1; i++)
    {
        if(data1d[i] > thres_2d)
            image_binary[i] = 255;
        else
            image_binary[i] = 0;
    }
    //padding 0: succes
    unsigned char* data1dp = 0;
    data1dp = new unsigned char [dsize];
    //memset(data1dp, 0, sizeof(unsigned char)*dsize);
    for(int i=0;i<dsize;i++) {data1dp[i]=0;}
    for(int iy=0;iy<M;iy++)
    {
        int offsetj = iy*N;
        int offsetjp = (iy+BlockSize)*(N+2*BlockSize);
        for(int ix=0;ix<N;ix++)
        {
            for(int iz=0;iz<P;iz++)
            {
                int offsetk = iz*M*N;
                int offsetkp = (iz+BlockSize)*(M+2*BlockSize)*(N+2*BlockSize);
                data1dp[offsetkp + offsetjp + ix+BlockSize]=image_binary[offsetk + offsetj + ix];
            }
        }
    }
    //test show image: success
    //Image4DSimple * new4DImage = new Image4DSimple();
    //new4DImage->setData((unsigned char *)data1dp,N+2*BlockSize, M+2*BlockSize, P+2*BlockSize, 1, V3D_UINT8);
    //v3dhandle newwin = callback.newImageWindow();
    //callback.setImage(newwin, new4DImage);
    //callback.setImageName(newwin, "Local_adaptive_enhancement_result");
    //callback.updateImageWindow(newwin);
    //delete []data1dp;
    //data1dp =0;
    //return 1;
    //

    //creat 2d rays

 //   v3d_msg("check2");
    vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y(ray_numbers_2d,vector<float>(ray_length_2d));
    float ang = 2*PI/ray_numbers_2d;
    float x_dis, y_dis;
    for(int ir = 0; ir < ray_numbers_2d; ir++)
    {
        x_dis = cos(ang*(ir+1));
        y_dis = sin(ang*(ir+1));
        for(int jr = 0; jr<ray_length_2d; jr++)
        {
            ray_x[ir][jr] = x_dis*(jr+1);
            ray_y[ir][jr] = y_dis*(jr+1);
        }
    }
    //

    // get swc
    if(data1dp)
    {
        double x,y,z;


//        if(PA.swcfilename.isEmpty())
//            return 1;
        NeuronTree nt;
 //       if (PA.swcfilename.toUpper().endsWith(".SWC") || PA.swcfilename.toUpper().endsWith(".ESWC"))
        {
          //  nt = readSWC_file(PA.swcfilename);
            nt = PA.nt;
            unsigned char * Block = 0;
            int end_x,start_x,end_y,start_y,end_z,start_z;
            //v3d_msg(QString("%1").arg(nt.listNeuron.size()));
            //detection loop

            LocationSimple s;
            int M1=2*BlockSize+1,N1=2*BlockSize+1,P1=2*BlockSize+1;
            int CenterPoint_ind=(M1*N1*P1+1)/2-1;
            int CenterPoint=BlockSize+1;
            vector<int> stackb;
            int project_value;
            unsigned char bwlabel;
            int ind,indk,ik,ind2;
            int sumb;
            int bflag1;
            int bflag2;
            int bflag3;
            int sz[3];
            int showflag=1;
            int pagesz=(2*BlockSize+1)*(2*BlockSize+1)*(2*BlockSize+1);
            int ib;
            try {Block = new unsigned char [pagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for loading the region.",0); return 1;}
            int pagesz_mip=M1*N1;
            unsigned char *image_mipx=0;
            try {image_mipx = new unsigned char [pagesz_mip];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return 1;}
            unsigned char *image_mipy=0;
            try {image_mipy = new unsigned char [pagesz_mip];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return 1;}
            unsigned char *image_mipz=0;
            try {image_mipz = new unsigned char [pagesz_mip];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return 1;}
            //cout<<"before branch detection"<<endl;
            for(int i = 0; i < nt.listNeuron.size(); i++)//******************************************************************************************
            {
//                cout<<i<<endl;
                x=floor(nt.listNeuron[i].x+0.5)+BlockSize;
                end_x=x+BlockSize;start_x=x-BlockSize;

                y=floor(nt.listNeuron[i].y+0.5)+BlockSize;
                end_y=y+BlockSize;start_y=y-BlockSize;

                z=floor(nt.listNeuron[i].z+0.5)+BlockSize;
                end_z=z+BlockSize;start_z=z-BlockSize;

                //int offsetc=x;
                //int offseta=y*(N+2*BlockSize);
                //int offsetb=z*(N+2*BlockSize)*(M+2*BlockSize);

                //pix_inten=data1dp[offsetc+offseta+offsetb];

                //cout<<pix_inten<<endl;
                //v3d_msg("check");
                // get block success********
                ib = 0;
                for(int iz = start_z; iz < end_z+1; iz++)
                {
                    int offsetk = iz*(N+2*BlockSize)*(M+2*BlockSize);
                    for(int iy = start_y; iy < end_y+1; iy++)
                    {
                        int offsetj = iy*(N+2*BlockSize);
                        for(int ix = start_x; ix < end_x+1; ix++)
                        {
//                            if(data1dp[offsetk + offsetj + ix]>(pix_inten-55))
//                            {
                                Block[ib] = data1dp[offsetk + offsetj + ix];
//                            }
//                            else {Block[ib]=0;}
                            ib++;
                        }
                    }
                }
                //cout<<"block ok"<<endl;
                // lian tong yu

                        while(!stackb.empty())
                        {
                            ind=stackb.back();
                            //cout<<ind<<endl;
                            //cout<<stackb.size()<<endl;
                            //cout<<bwlabel<<endl;
                            stackb.erase(stackb.end()-1);
                            Block[ind]=bwlabel;
                            for(int k=-1;k<2;k++)
                            {
                                indk=ind+k*M1;
                                for(int j=-1;j<2;j++)
                                {
                                    if((indk+j*M1*N1-1)>=0&&(indk+j*M1*N1-1)<pagesz&&Block[indk+j*M1*N1-1]==255)
                                    {stackb.push_back(indk+j*M1*N1-1);Block[indk+j*M1*N1-1]=200;}
                                    //v3d_msg("check4");

                                    if((indk+j*M1*N1)>=0&&(indk+j*M1*N1)<pagesz&&Block[indk+j*M1*N1]==255)
                                    {stackb.push_back(indk+j*M1*N1);Block[indk+j*M1*N1]=200;}
                                    //v3d_msg("check5");

                                    if((indk+j*M1*N1+1)>=0&&(indk+j*M1*N1+1)<pagesz&&Block[indk+j*M1*N1+1]==255)
                                    {stackb.push_back(indk+j*M1*N1+1);Block[indk+j*M1*N1+1]=200;}
                                    //v3d_msg("check6");

                                }
                            }
                        }
                //
                //cout<<"label ok"<<endl;
                int c1=0;int c2=0;int c3=0;int c4=0;
                for(int qq=0;qq<pagesz;qq++)
                {
                    if(Block[qq]==1)
                        c1=c1+1;
                    if(Block[qq]==2)
                        c2=c2+1;
                    if(Block[qq]==3)
                        c3=c3+1;
                    if(Block[qq]==4)
                        c4=c4+1;
                }


                //Multidirection mip
                Image4DSimple * new4DImagez;
                Image4DSimple * new4DImagex;
                Image4DSimple * new4DImagey;
                v3dhandle newwinz;
                v3dhandle newwinx;
                v3dhandle newwiny;
                for(int ip=0;ip<3;ip++)
                {
                    switch (ip)
                    {
                    case 0:
                        for(int iy = 0; iy < M1; iy++)
                        {
                            int offsetj = iy*N1;
                            for(int ix = 0; ix < N1; ix++)
                            {
                                int max_mip = 0;
                                for(int iz = 0; iz < P1; iz++)
                                {
                                    int offsetk = iz*M1*N1;
                                    if(Block[offsetk + offsetj + ix] >= max_mip)
                                    {
                                        image_mipz[iy*N1 + ix] = Block[offsetk + offsetj + ix];
                                        max_mip = Block[offsetk + offsetj + ix];
                                    }
                                }
                            }
                        }

                        break;
                    case 1:
                        for(int iz = 0; iz < P1; iz++)
                        {
                            int offsetk = iz*M1*N1;
                            for(int ix = 0; ix < N1; ix++)
                            {
                                int max_mip = 0;
                                for(int iy = 0; iy < M1; iy++)
                                {
                                    int offsetj = iy*N1;
                                    if(Block[offsetk + offsetj + ix] >= max_mip)
                                    {
                                        image_mipy[iz*N1 + ix] = Block[offsetk + offsetj + ix];
                                        max_mip = Block[offsetk + offsetj + ix];
                                    }
                                }
                            }
                        }
                        break;
                    case 2:
                        for(int iz = 0; iz < P1; iz++)
                        {
                            int offsetk = iz*M1*N1;
                            for(int iy = 0; iy < M1; iy++)
                            {
                                int offsetj = iy*N1;
                                int max_mip = 0;
                                for(int ix = 0; ix < N1; ix++)
                                {
                                    if(Block[offsetk + offsetj + ix] >= max_mip)
                                    {
                                        image_mipx[iz*M1 + iy] = Block[offsetk + offsetj + ix];
                                        max_mip = Block[offsetk + offsetj + ix];
                                    }
                                }
                            }
                        }

                        break;
                    default:
                        return 1;
                    }
                }


                // mip finished
                // cout<<"branch detection"<<endl;
                // branch detection
                sumb=0;
                bflag1=0;
                bflag2=0;
                bflag3=0;
                sz[0]=N1;
                sz[1]=M1;
                sz[2]=P1;
                for(int i3=0;i3<3;i3++)
                {
                    switch (i3)
                    {
                    case 0:
                        project_value=get_2D_ValueUINT8(CenterPoint,CenterPoint,image_mipz,sz[0],sz[1]);
                        //cout<<"Centerpoint"<<CenterPoint<<","<<sz[0]<<endl;
                        if(project_value > thres_2d)
                        {
                            bflag1=rayinten_2D_multiscale(CenterPoint,CenterPoint,ray_numbers_2d ,ray_length_2d,T1,ray_x, ray_y, image_mipz,sz[0],sz[1]);
                            //cout<<"case1"<<endl;
                        }
                        break;
                    case 1:
                        //cout<<"case2"<<endl;
                        project_value=get_2D_ValueUINT8(CenterPoint,CenterPoint,image_mipy,sz[0],sz[1]);
                        if(project_value > thres_2d)
                        {
                            bflag2=rayinten_2D_multiscale(CenterPoint,CenterPoint,ray_numbers_2d ,ray_length_2d,T1,ray_x, ray_y, image_mipy,sz[0],sz[1]);
                        }

                        break;
                    case 2:
                        //cout<<"case3"<<endl;
                        project_value=get_2D_ValueUINT8(CenterPoint,CenterPoint,image_mipx,sz[0],sz[1]);
                        if(project_value > thres_2d)
                        {
                            bflag3=rayinten_2D_multiscale(CenterPoint,CenterPoint,ray_numbers_2d ,ray_length_2d,T1,ray_x, ray_y, image_mipx,sz[0],sz[1]);
                        }
                        break;
                    default:
                        return 1;
                    }
                }
                sumb=bflag1+bflag2+bflag3;
//                 cout<<bflag1<<endl;
//                 cout<<bflag2<<endl;
//                 cout<<bflag3<<endl;
//                 v3d_msg("check");
                if (sumb>0)
                {
                    s.x = x-BlockSize;
                    s.y = y-BlockSize;
                    s.z = z-BlockSize;
                    s.radius = 1;
                    s.color = random_rgba8(255);
                    //cout<<s.x<<","<<s.y<<","<<s.z<<","<<endl;
                    curlist << s;
                }
                //cout<<i<<"//"<<nt.listNeuron.size()<<endl;
                //cout<<"branch finished"<<endl;
            }
            if(image_mipx)
            {
                delete []image_mipx;
                image_mipx =0;
            }
            if(image_mipy)
            {
                delete []image_mipy;
                image_mipy =0;
            }
            if(image_mipz)
            {
                delete []image_mipz;
                image_mipz =0;
            }
            if(Block)
            {
                delete []Block;
                Block =0;
            }
            if(image_binary)
            {
                delete []image_binary;
                image_binary =0;
            }

            //callback.setLandmark(curwin, curlist);


        }
    }
    if(data1d)
    {
        delete []data1d;
        data1d =0;
    }
    return true;
}
void raymodel(V3DPluginCallback2 &callback, QWidget *parent)
{
    LandmarkList curlist;
    LocationSimple s;
    // 1 - Obtain the current 4D image pointer
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No image is open.");
        return;
    }
    Image4DSimple *p4DImage = callback.getImage(curwin);

    V3DLONG sz[3];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();

    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                                                "",
                                                QObject::tr("Supported file (*.swc *.eswc)"
                                                            ";;Neuron structure	(*.swc)"
                                                            ";;Extended neuron structure (*.eswc)"
                                                            ));
    if(fileOpenName.isEmpty())
        return;
    NeuronTree nt;
    if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
    {
    nt = readSWC_file(fileOpenName);
    float x_point=0,y_point=0,z_point=0;
    int ray_numble=64;
    int length_numble=8;

    QDialog * dialog = new QDialog();


    QGridLayout * layout = new QGridLayout();

    QSpinBox * index = new QSpinBox();
    index->setRange(1,10000);
    index->setValue(0);


    QSpinBox * number_ray_spinbox = new QSpinBox();
    number_ray_spinbox->setRange(1,1000);
    number_ray_spinbox->setValue(ray_numble);

    QSpinBox * length_ray_spinbox = new QSpinBox();
    length_ray_spinbox->setRange(1,100);
    length_ray_spinbox->setValue(length_numble);

//    if(p4DImage->getZDim() > 1)
//    {
//        QSpinBox * slice_number_spinbox = new QSpinBox();
//        slice_number_spinbox->setRange(0, p4DImage->getZDim()/2);
//        slice_number_spinbox->setValue(slice_number);

//        layout->addWidget(new QLabel("slice number"),5,0);
//        layout->addWidget(slice_number_spinbox, 5,1,1,5);

//        slice_number = slice_number_spinbox->value();
//    }


    layout->addWidget(new QLabel("index"),0,0);
    layout->addWidget(index, 0,1,1,5);

    layout->addWidget(new QLabel("ray_numble of model"),1,0);
    layout->addWidget(number_ray_spinbox, 1,1,1,5);

    layout->addWidget(new QLabel("length_numbers of model"),2,0);
    layout->addWidget(length_ray_spinbox, 2,1,1,5);

    QHBoxLayout * hbox2 = new QHBoxLayout();
    QPushButton * ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton * cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,6,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));


    //run the dialog

    if(dialog->exec() != QDialog::Accepted)
    {
        if (dialog)
        {
            delete dialog;
            dialog=0;
            cout<<"delete dialog"<<endl;
        }
        return;
    }

    //get the dialog return values
    int ind=index->value();
    x_point = floor(nt.listNeuron[ind].x+0.5);
    y_point = floor(nt.listNeuron[ind].y+0.5);
    z_point = floor(nt.listNeuron[ind].z+0.5);
    ray_numble = number_ray_spinbox->value();
    length_numble = length_ray_spinbox->value();

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }


    float ang = 2*PI/ray_numble;

    for(int i=0;i<ray_numble;i++)
    {
        float x_dis=cos(ang*(i+1));
        float y_dis=sin(ang*(i+1));
        for(int j=0;j<length_numble;j++)
        {
            s.x = x_point+(j+1)*x_dis;
            s.y = y_point+(j+1)*y_dis;
            s.z = z_point;
            s.radius = 1;
            s.color = random_rgba8(255);
            //cout<<s.x<<","<<s.y<<","<<s.z<<","<<endl;
            curlist << s;
        }
        //v3d_msg(QString("y is %1").arg(y_lac[i]));
    }
    callback.setLandmark(curwin, curlist);
    }
}








int loadRawRegion(char * filename, unsigned char * & img, V3DLONG * & sz, V3DLONG * & region_sz,int & datatype,
                  V3DLONG startx, V3DLONG starty, V3DLONG startz,
                  V3DLONG endx, V3DLONG endy, V3DLONG endz)
{
    FILE * fid = fopen(filename, "rb");

    if (!fid)
    {
        printf("Fail to open file for reading.\n");
        return 0;
    }

#if defined (Q_OS_WIN32)
    _fseeki64(fid, 0, SEEK_END);
    V3DLONG fileSize = _ftelli64(fid);
#endif

#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
    fseek(fid, 0, SEEK_END);
    V3DLONG fileSize = ftell(fid);
#endif

    std::cout << "file size: " << fileSize << endl;
    rewind(fid);
/*
#endif
*/
    /* Read header */

    char formatkey[] = "raw_image_stack_by_hpeng";
    V3DLONG lenkey = strlen(formatkey);


#ifndef _MSC_VER //added by PHC, 2010-05-21
    if (fileSize<lenkey+2+4*4+1) // datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte.
    {
        printf("The size of your input file is too small and is not correct, -- it is too small to contain the legal header.\n");
        printf("The fseek-ftell produces a file size = %ld.", fileSize);
        return 1;
    }
#endif

    char * keyread = new char [lenkey+1];
    if (!keyread)
    {
        printf("Fail to allocate memory.\n");

        return 0;
    }

    V3DLONG nread = fread(keyread, 1, lenkey, fid);
    if (nread!=lenkey) {
        printf("File unrecognized or corrupted file.\n");

        if (keyread) {delete []keyread; keyread=0;}
        return 0;
    }

    keyread[lenkey] = '\0';

    V3DLONG i;
    if (strcmp(formatkey, keyread)) /* is non-zero then the two strings are different */
    {
        printf("Unrecognized file format.\n");
        if (keyread) {delete []keyread; keyread=0;}
        return 0;
    }

    char endianCodeData;
    int dummy = fread(&endianCodeData, 1, 1, fid);
    printf("The data endian code is [%c]\n", endianCodeData);
    if (endianCodeData!='B' && endianCodeData!='L')
    {
        if (keyread) {delete []keyread; keyread=0;}
        return 0;    }

    char endianCodeMachine;
    endianCodeMachine = checkMachineEndian();
    printf("The machine endian code is [%c]\n", endianCodeMachine);
    if (endianCodeMachine!='B' && endianCodeMachine!='L')
    {
        printf("This program only supports big- or little- endian but not other format. Check your data endian.\n");
        if (keyread) {delete []keyread; keyread=0;}
        return 0;
       }

    int b_swap = (endianCodeMachine==endianCodeData)?0:1;

    short int dcode = 0;
    dummy = fread(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
    if (b_swap)
        swap2bytes((void *)&dcode);

    switch (dcode)
    {
        case 1:
            datatype = 1; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
            break;

        case 2:
            datatype = 2;
            break;

        case 4:
            datatype = 4;
            break;

        default:
            if (keyread) {delete []keyread; keyread=0;}
            return  0;
    }

    V3DLONG unitSize = datatype; // temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future.

    BIT32_UNIT mysz[4];
    //long int mysz[4];
    mysz[0]=mysz[1]=mysz[2]=mysz[3]=0;
    int tmpn=(int)fread(mysz, 4, 4, fid); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read.
    if (tmpn!=4) {
        if (keyread) {delete []keyread; keyread=0;}
        return 0;
    }

    if (b_swap)
    {
        for (i=0;i<4;i++)
        {
            //swap2bytes((void *)(mysz+i));
            if (b_VERBOSE_PRINT)
                printf("mysz raw read unit[%ld]: [%d] ", i, mysz[i]);
            swap4bytes((void *)(mysz+i));
            if (b_VERBOSE_PRINT)
                printf("swap unit: [%d][%0x] \n", mysz[i], mysz[i]);
        }
    }
    if (sz) {delete []sz; sz=0;}
    sz = new V3DLONG [4]; // reallocate the memory if the input parameter is non-null. Note that this requests the input is also an NULL point, the same to img.
    if (!sz)
    {
        if (keyread) {delete []keyread; keyread=0;}
        return 0;
    }

    V3DLONG totalUnit = 1;

    for (i=0;i<4;i++)
    {
        sz[i] = (V3DLONG)mysz[i];
        totalUnit *= sz[i];
    }

    V3DLONG tmpw = endx - startx;
    V3DLONG tmph = endy - starty;
    V3DLONG tmpz = endz - startz;

    V3DLONG head = 4*4+2+1+lenkey; // header_len ?
    V3DLONG pgsz1=sz[2]*sz[1]*sz[0], pgsz2=sz[1]*sz[0], pgsz3=sz[0];
    V3DLONG cn = tmpw*tmph*tmpz;
    V3DLONG kn = tmpw*tmph;
    V3DLONG total = tmpw*tmph*tmpz*sz[3];

    if (region_sz) {delete []region_sz; region_sz=0;}
    region_sz = new V3DLONG [4]; // reallocate the memory if the input parameter is non-null. Note that this requests the input is also an NULL point, the same to img.
    if (!region_sz)
    {
        if (keyread) {delete []keyread; keyread=0;}
        return 0;
    }

    region_sz[0] = tmpw;
    region_sz[1] = tmph;
    region_sz[2] = tmpz;
    region_sz[3] = sz[3];

    if (img) {delete []img; img=0;}
    V3DLONG totalBytes = V3DLONG(unitSize)*V3DLONG(total);
    try
    {
        img = new unsigned char [totalBytes];
    }
    catch (...)
    {
        if (keyread) {delete []keyread; keyread=0;}
        if (sz) {delete []sz; sz=0;}
        return 0;
    }

    //V3DLONG count=0; // unused
#if defined (Q_OS_WIN32)
    V3DLONG c,j,k;
    for (c = 0; c < sz[3]; c++)
    {
        for (k = startz; k < endz; k++)
        {
            for (j = starty; j< endy; j++)
            {
                rewind(fid);
                _fseeki64(fid, (V3DLONG)(head + (c*pgsz1 + k*pgsz2 + j*pgsz3 + startx)*unitSize), SEEK_SET);
                int dummy = _ftelli64(fid);
                dummy = fread(img+(c*cn+(k-startz)*kn + (j-starty)*tmpw)*unitSize,unitSize,tmpw,fid);
            }
        }
    }
#endif

#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
    V3DLONG c,j,k;
    for (c = 0; c < sz[3]; c++)
    {
        for (k = startz; k < endz; k++)
        {
            for (j = starty; j< endy; j++)
            {
                rewind(fid);
                fseek(fid, (V3DLONG)(head + (c*pgsz1 + k*pgsz2 + j*pgsz3 + startx)*unitSize), SEEK_SET);
                int dummy = ftell(fid);
                dummy = fread(img+(c*cn+(k-startz)*kn + (j-starty)*tmpw)*unitSize,unitSize,tmpw,fid);
            }
        }
    }
#endif
    // swap the data bytes if necessary

    if (b_swap==1)
    {
        if (unitSize==2)
        {
            for (i=0;i<total; i++)
            {
                swap2bytes((void *)(img+i*unitSize));
            }
        }
        else if (unitSize==4)
        {
            for (i=0;i<total; i++)
            {
                swap4bytes((void *)(img+i*unitSize));
            }
        }
    }

    // clean and return

    if (keyread) {delete [] keyread; keyread = 0;}
    fclose(fid); //bug fix on 060412
    return 1;


}
char checkMachineEndian()
{
    char e='N'; //for unknown endianness

    V3DLONG int a=0x44332211;
    unsigned char * p = (unsigned char *)&a;
    if ((*p==0x11) && (*(p+1)==0x22) && (*(p+2)==0x33) && (*(p+3)==0x44))
        e = 'L';
    else if ((*p==0x44) && (*(p+1)==0x33) && (*(p+2)==0x22) && (*(p+3)==0x11))
        e = 'B';
    else if ((*p==0x22) && (*(p+1)==0x11) && (*(p+2)==0x44) && (*(p+3)==0x33))
        e = 'M';
    else
        e = 'N';

    //printf("[%c] \n", e);
    return e;
}


void swap2bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+1);
    *(tp+1) = a;
}

void swap4bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+3);
    *(tp+3) = a;
    a = *(tp+1);
    *(tp+1) = *(tp+2);
    *(tp+2) = a;
}
bool do_each_block_v2(V3DPluginCallback2 &callback,BRANCH_LS_PARA &PA,NeuronSWC curr_point,NeuronTree &nt)
{
    double l_x = 256;
    double l_y = 256;
    double l_z = 128;
    V3DLONG xb = curr_point.x-l_x;
    V3DLONG xe = curr_point.x+l_x-1;
    V3DLONG yb = curr_point.y-l_y;
    V3DLONG ye = curr_point.y+l_y-1;
    V3DLONG zb = curr_point.z-l_z;
    V3DLONG ze = curr_point.z+l_z-1;

                cout<<"begin ==================="<<xb<<"  "<<yb<<"  "<<"  "<<zb<<endl;
                cout<<"end   ==================="<<xe<<"  "<<ye<<"  "<<"  "<<ze<<endl;

    unsigned char * im_cropped = 0;
    V3DLONG pagesz;
    V3DLONG im_cropped_sz[4];
    pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
    im_cropped_sz[0] = xe-xb+1;
    im_cropped_sz[1] = ye-yb+1;
    im_cropped_sz[2] = ze-zb+1;
    im_cropped_sz[3] = 1;

    cout<<xb<<"  "<<yb<<"  "<<zb<<endl;
    cout<<xe<<"  "<<ye<<"  "<<ze<<endl;
    cout<<im_cropped_sz[0]<<"  "<<im_cropped_sz[1]<<"  "<<im_cropped_sz[2]<<"  "<<im_cropped_sz[3]<<endl;

    try {im_cropped = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}



    NeuronTree outswc;
    for(V3DLONG j=0;j<nt.listNeuron.size();j++)
    {
        NeuronSWC p;
        if(nt.listNeuron[j].x<xe&&nt.listNeuron[j].x>xb&&nt.listNeuron[j].y<ye&&nt.listNeuron[j].y>yb&&nt.listNeuron[j].z<ze&&nt.listNeuron[j].z>zb)
        {
            p.n = nt.listNeuron[j].n;
            p.x = nt.listNeuron[j].x-xb;
            p.y = nt.listNeuron[j].y-yb;
            p.z = nt.listNeuron[j].z-zb;
            p.type = nt.listNeuron[j].type;
            p.r = nt.listNeuron[j].r;
            p.pn = nt.listNeuron[j].pn;
            outswc.listNeuron.push_back(p);
        }
    }


    im_cropped = callback.getSubVolumeTeraFly(PA.inimg_file.toStdString(),xb,xe+1,
                                          yb,ye+1,zb,ze+1);


    QString outimg_file,outswc_file;
    outimg_file = "test.tif";
    outswc_file = "test.swc";
    writeSWC_file(outswc_file,outswc);


  //  simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
    PA.data1d = im_cropped;
    PA.nt = outswc;
    PA.in_sz[0] = 2*l_x;
    PA.in_sz[1] = 2*l_y;
    PA.in_sz[2] = 2*l_z;
    PA.xb = xb;
    PA.yb = yb;
    PA.zb = zb;



}


bool do_detection_v2(V3DPluginCallback2 &callback,BRANCH_LS_PARA &PA,NeuronSWC curr_point,LandmarkList &curlist,NeuronTree &nt)
{

    double l_x = 256;
    double l_y = 256;
    double l_z = 128;
    V3DLONG xb = curr_point.x-l_x;
    V3DLONG xe = curr_point.x+l_x-1;
    V3DLONG yb = curr_point.y-l_y;
    V3DLONG ye = curr_point.y+l_y-1;
    V3DLONG zb = curr_point.z-l_z;
    V3DLONG ze = curr_point.z+l_z-1;

                cout<<"begin ==================="<<xb<<"  "<<yb<<"  "<<"  "<<zb<<endl;
                cout<<"end   ==================="<<xe<<"  "<<ye<<"  "<<"  "<<ze<<endl;

    unsigned char * im_cropped = 0;
    V3DLONG pagesz;
    V3DLONG im_cropped_sz[4];
    pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
    im_cropped_sz[0] = xe-xb+1;
    im_cropped_sz[1] = ye-yb+1;
    im_cropped_sz[2] = ze-zb+1;
    im_cropped_sz[3] = 1;

    cout<<xb<<"  "<<yb<<"  "<<zb<<endl;
    cout<<xe<<"  "<<ye<<"  "<<ze<<endl;
    cout<<im_cropped_sz[0]<<"  "<<im_cropped_sz[1]<<"  "<<im_cropped_sz[2]<<"  "<<im_cropped_sz[3]<<endl;

    try {im_cropped = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}



    NeuronTree outswc;
    for(V3DLONG j=0;j<nt.listNeuron.size();j++)
    {
        NeuronSWC p;
        if(nt.listNeuron[j].x<xe&&nt.listNeuron[j].x>xb&&nt.listNeuron[j].y<ye&&nt.listNeuron[j].y>yb&&nt.listNeuron[j].z<ze&&nt.listNeuron[j].z>zb)
        {
            p.n = nt.listNeuron[j].n;
            p.x = nt.listNeuron[j].x-xb;
            p.y = nt.listNeuron[j].y-yb;
            p.z = nt.listNeuron[j].z-zb;
            p.type = nt.listNeuron[j].type;
            p.r = nt.listNeuron[j].r;
            p.pn = nt.listNeuron[j].pn;
            outswc.listNeuron.push_back(p);
        }
    }


    im_cropped = callback.getSubVolumeTeraFly(PA.inimg_file.toStdString(),xb,xe+1,
                                          yb,ye+1,zb,ze+1);


    QString outimg_file,outswc_file;
    outimg_file = "test.tif";
    outswc_file = "test.swc";
    writeSWC_file(outswc_file,outswc);


    simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
    PA.data1d = im_cropped;
    PA.in_sz[0] = 2*l_x;
    PA.in_sz[1] = 2*l_y;
    PA.in_sz[2] = 2*l_z;






    //v3d_msg("do_detection");
    int ray_numbers_2d=64;//
    int ray_length_2d=12;//
    unsigned char* data1d = 0;

    int dsize;
    int T1=113;
    int thres_2d=65;
    int N,M,P,C;


    data1d = im_cropped;
    N = PA.in_sz[0];
    M = PA.in_sz[1];
    P = PA.in_sz[2];
    C = 1;

    cout<<N<<"  "<<M<<"  "<<P<<"  "<<C<<endl;


    QDialog * dialog = new QDialog();


    if(P > 1)
            dialog->setWindowTitle("3D neuron image tip point detection Based on Ray-shooting algorithm");
    else
            dialog->setWindowTitle("2D neuron image tip point detection Based on Ray-shooting algorithm");

QGridLayout * layout = new QGridLayout();

QSpinBox * ray_numbers_2d_spinbox = new QSpinBox();
ray_numbers_2d_spinbox->setRange(1,1000);
ray_numbers_2d_spinbox->setValue(ray_numbers_2d);

QSpinBox * thres_2d_spinbox = new QSpinBox();
thres_2d_spinbox->setRange(-1, 255);
thres_2d_spinbox->setValue(thres_2d);

QSpinBox * ray_length_2d_spinbox = new QSpinBox();
ray_length_2d_spinbox->setRange(1,N);
ray_length_2d_spinbox->setValue(ray_length_2d);

QSpinBox * T1_spinbox = new QSpinBox();
T1_spinbox->setRange(1,255);
T1_spinbox->setValue(T1);


layout->addWidget(new QLabel("ray numbers"),0,0);
layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

layout->addWidget(new QLabel("intensity threshold"),1,0);
layout->addWidget(thres_2d_spinbox, 1,1,1,5);

layout->addWidget(new QLabel("maximum scale"),2,0);
layout->addWidget(ray_length_2d_spinbox, 2,1,1,5);

layout->addWidget(new QLabel("T1"),3,0);
layout->addWidget(T1_spinbox, 3,1,1,5);


QHBoxLayout * hbox2 = new QHBoxLayout();
QPushButton * ok = new QPushButton(" ok ");
ok->setDefault(true);
QPushButton * cancel = new QPushButton("cancel");
hbox2->addWidget(cancel);
hbox2->addWidget(ok);

layout->addLayout(hbox2,6,0,1,6);
dialog->setLayout(layout);
QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

//run the dialog
v3d_msg("check1");
//if(dialog->exec() != QDialog::Accepted)
//    {
//            if (dialog)
//            {
//                    delete dialog;
//                    dialog=0;
//                    cout<<"delete dialog"<<endl;
//            }
//            return false;
//    }

//get the dialog return values
ray_numbers_2d = ray_numbers_2d_spinbox->value();
thres_2d = thres_2d_spinbox->value();
ray_length_2d = ray_length_2d_spinbox->value();
T1 = T1_spinbox->value();
int BlockSize=ray_length_2d+2;

//v3d_msg("1");
    dsize=(N+2*BlockSize)*(M+2*BlockSize)*(P+2*BlockSize);
    // segmentation
 //   cout<<"dsize = "<<dsize<<endl;
    int pagesz1 = N*M*P;
//    cout<<"pagesz1 = "<<pagesz1<<endl;
    unsigned char *image_binary=0;
    try{image_binary=new unsigned char [pagesz1];}
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return false;}
 //   v3d_msg("2");
//    for(V3DLONG i=0;i<100000000;i++)
//    {
//        cout<<"l"<<endl;
//        cout<<int(data1d[i])<<endl;
//    }
    for(int i = 0; i < pagesz1; i++)
    {
        //cout<<"i"<<endl;
        if(int(data1d[i]) > thres_2d)
        {
            //cout<<"if"<<endl;
            image_binary[i] = 255;
        }
        else
        {
            //cout<<"else"<<endl;
            image_binary[i] = 0;
        }
    }
//    v3d_msg("2.5");

    //padding 0: succes
    unsigned char* data1dp = 0;
    data1dp = new unsigned char [dsize];
    //memset(data1dp, 0, sizeof(unsigned char)*dsize);
    for(int i=0;i<dsize;i++) {data1dp[i]=0;}

  //  v3d_msg("3");
    for(int iy=0;iy<M;iy++)
    {
        int offsetj = iy*N;
        int offsetjp = (iy+BlockSize)*(N+2*BlockSize);
        for(int ix=0;ix<N;ix++)
        {
            for(int iz=0;iz<P;iz++)
            {
                int offsetk = iz*M*N;
                int offsetkp = (iz+BlockSize)*(M+2*BlockSize)*(N+2*BlockSize);
                data1dp[offsetkp + offsetjp + ix+BlockSize]=image_binary[offsetk + offsetj + ix];
            }
        }
    }


 //  v3d_msg("check2");
    vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y(ray_numbers_2d,vector<float>(ray_length_2d));
    float ang = 2*PI/ray_numbers_2d;
    float x_dis, y_dis;
    for(int ir = 0; ir < ray_numbers_2d; ir++)
    {
        x_dis = cos(ang*(ir+1));
        y_dis = sin(ang*(ir+1));
        for(int jr = 0; jr<ray_length_2d; jr++)
        {
            ray_x[ir][jr] = x_dis*(jr+1);
            ray_y[ir][jr] = y_dis*(jr+1);
        }
    }
    //

    // get swc
    if(data1dp)
    {
        double x,y,z;
        {

            unsigned char * Block = 0;
            int end_x,start_x,end_y,start_y,end_z,start_z;
            //v3d_msg(QString("%1").arg(nt.listNeuron.size()));
            //detection loop

            LocationSimple s;
            int M1=2*BlockSize+1,N1=2*BlockSize+1,P1=2*BlockSize+1;
            int CenterPoint_ind=(M1*N1*P1+1)/2-1;
            int CenterPoint=BlockSize+1;
            vector<int> stackb;
            int project_value;
            unsigned char bwlabel;
            int ind,indk,ik,ind2;
            int sumb;
            int bflag1;
            int bflag2;
            int bflag3;
            int sz[3];
            int showflag=1;
            int pagesz=(2*BlockSize+1)*(2*BlockSize+1)*(2*BlockSize+1);
            int ib;
            try {Block = new unsigned char [pagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for loading the region.",0); return 1;}
            int pagesz_mip=M1*N1;
            unsigned char *image_mipx=0;
            try {image_mipx = new unsigned char [pagesz_mip];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return 1;}
            unsigned char *image_mipy=0;
            try {image_mipy = new unsigned char [pagesz_mip];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return 1;}
            unsigned char *image_mipz=0;
            try {image_mipz = new unsigned char [pagesz_mip];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return 1;}
            //cout<<"before branch detection"<<endl;
           // v3d_msg("yyyyyyyyyyyy");
            for(int i = 0; i < 1; i++)//******************************************************************************************
            {
                cout<<i<<endl;
                x=floor(curr_point.x+0.5)+BlockSize;
                end_x=x+BlockSize;
                start_x=x-BlockSize;

                y=floor(curr_point.y+0.5)+BlockSize;
                end_y=y+BlockSize;
                start_y=y-BlockSize;

                z=floor(curr_point.z+0.5)+BlockSize;
                end_z=z+BlockSize;
                start_z=z-BlockSize;

                cout<<x<<"  "<<y<<"  "<<z<<endl;
                cout<<start_x<<"  "<<start_y<<"  "<<start_z<<"  "<<endl;
                cout<<end_x<<"  "<<end_y<<"  "<<end_z<<"  "<<endl;


                double show_x = start_x - xb;
                double show_y = start_y - yb;
                double show_z = start_z - zb;

                cout<<"show = "<<show_x<<"  "<<show_y<<"  "<<show_z<<"  "<<endl;
                //int offsetc=x;
                //int offseta=y*(N+2*BlockSize);
                //int offsetb=z*(N+2*BlockSize)*(M+2*BlockSize);

                //pix_inten=data1dp[offsetc+offseta+offsetb];

                //cout<<pix_inten<<endl;
                v3d_msg("check");
                // get block success********
                ib = 0;
                for(int iz = start_z; iz < end_z+1; iz++)
                {
                    int offsetk = iz*(N+2*BlockSize)*(M+2*BlockSize);
                    for(int iy = start_y; iy < end_y+1; iy++)
                    {
                        int offsetj = iy*(N+2*BlockSize);
                        for(int ix = start_x; ix < end_x+1; ix++)
                        {
//                            if(data1dp[offsetk + offsetj + ix]>(pix_inten-55))
//                            {
                                Block[ib] = data1dp[offsetk + offsetj + ix];
//                            }
//                            else {Block[ib]=0;}
                            ib++;
                        }
                    }
                }
                cout<<"block ok"<<endl;
                // lian tong yu

                        while(!stackb.empty())
                        {
                            ind=stackb.back();
                            //cout<<ind<<endl;
                            //cout<<stackb.size()<<endl;
                            //cout<<bwlabel<<endl;
                            stackb.erase(stackb.end()-1);
                            Block[ind]=bwlabel;
                            for(int k=-1;k<2;k++)
                            {
                                indk=ind+k*M1;
                                for(int j=-1;j<2;j++)
                                {
                                    if((indk+j*M1*N1-1)>=0&&(indk+j*M1*N1-1)<pagesz&&Block[indk+j*M1*N1-1]==255)
                                    {stackb.push_back(indk+j*M1*N1-1);Block[indk+j*M1*N1-1]=200;}
                                    //v3d_msg("check4");

                                    if((indk+j*M1*N1)>=0&&(indk+j*M1*N1)<pagesz&&Block[indk+j*M1*N1]==255)
                                    {stackb.push_back(indk+j*M1*N1);Block[indk+j*M1*N1]=200;}
                                    //v3d_msg("check5");

                                    if((indk+j*M1*N1+1)>=0&&(indk+j*M1*N1+1)<pagesz&&Block[indk+j*M1*N1+1]==255)
                                    {stackb.push_back(indk+j*M1*N1+1);Block[indk+j*M1*N1+1]=200;}
                                    //v3d_msg("check6");

                                }
                            }
                        }
                //
                cout<<"label ok"<<endl;
                int c1=0;int c2=0;int c3=0;int c4=0;
                for(int qq=0;qq<pagesz;qq++)
                {
                    if(Block[qq]==1)
                        c1=c1+1;
                    if(Block[qq]==2)
                        c2=c2+1;
                    if(Block[qq]==3)
                        c3=c3+1;
                    if(Block[qq]==4)
                        c4=c4+1;
                }

                for(int ip=0;ip<3;ip++)
                {
                    switch (ip)
                    {
                    case 0:
                        for(int iy = 0; iy < M1; iy++)
                        {
                            int offsetj = iy*N1;
                            for(int ix = 0; ix < N1; ix++)
                            {
                                int max_mip = 0;
                                for(int iz = 0; iz < P1; iz++)
                                {
                                    int offsetk = iz*M1*N1;
                                    if(Block[offsetk + offsetj + ix] >= max_mip)
                                    {
                                        image_mipz[iy*N1 + ix] = Block[offsetk + offsetj + ix];
                                        max_mip = Block[offsetk + offsetj + ix];
                                    }
                                }
                            }
                        }

                        break;
                    case 1:
                        for(int iz = 0; iz < P1; iz++)
                        {
                            int offsetk = iz*M1*N1;
                            for(int ix = 0; ix < N1; ix++)
                            {
                                int max_mip = 0;
                                for(int iy = 0; iy < M1; iy++)
                                {
                                    int offsetj = iy*N1;
                                    if(Block[offsetk + offsetj + ix] >= max_mip)
                                    {
                                        image_mipy[iz*N1 + ix] = Block[offsetk + offsetj + ix];
                                        max_mip = Block[offsetk + offsetj + ix];
                                    }
                                }
                            }
                        }
                        break;
                    case 2:
                        for(int iz = 0; iz < P1; iz++)
                        {
                            int offsetk = iz*M1*N1;
                            for(int iy = 0; iy < M1; iy++)
                            {
                                int offsetj = iy*N1;
                                int max_mip = 0;
                                for(int ix = 0; ix < N1; ix++)
                                {
                                    if(Block[offsetk + offsetj + ix] >= max_mip)
                                    {
                                        image_mipx[iz*M1 + iy] = Block[offsetk + offsetj + ix];
                                        max_mip = Block[offsetk + offsetj + ix];
                                    }
                                }
                            }
                        }

                        break;
                    default:
                        return 1;
                    }
                }

                sumb=0;
                bflag1=0;
                bflag2=0;
                bflag3=0;
                sz[0]=N1;
                sz[1]=M1;
                sz[2]=P1;
                for(int i3=0;i3<3;i3++)
                {
                    switch (i3)
                    {
                    case 0:
                        project_value=get_2D_ValueUINT8(CenterPoint,CenterPoint,image_mipz,sz[0],sz[1]);
                        //cout<<"Centerpoint"<<CenterPoint<<","<<sz[0]<<endl;
                        if(project_value > thres_2d)
                        {
                            bflag1=rayinten_2D_multiscale(CenterPoint,CenterPoint,ray_numbers_2d ,ray_length_2d,T1,ray_x, ray_y, image_mipz,sz[0],sz[1]);
                            //cout<<"case1"<<endl;
                        }
                        break;
                    case 1:
                        //cout<<"case2"<<endl;
                        project_value=get_2D_ValueUINT8(CenterPoint,CenterPoint,image_mipy,sz[0],sz[1]);
                        if(project_value > thres_2d)
                        {
                            bflag2=rayinten_2D_multiscale(CenterPoint,CenterPoint,ray_numbers_2d ,ray_length_2d,T1,ray_x, ray_y, image_mipy,sz[0],sz[1]);
                        }

                        break;
                    case 2:
                        //cout<<"case3"<<endl;
                        project_value=get_2D_ValueUINT8(CenterPoint,CenterPoint,image_mipx,sz[0],sz[1]);
                        if(project_value > thres_2d)
                        {
                            bflag3=rayinten_2D_multiscale(CenterPoint,CenterPoint,ray_numbers_2d ,ray_length_2d,T1,ray_x, ray_y, image_mipx,sz[0],sz[1]);
                        }
                        break;
                    default:
                        return 1;
                    }
                }
                sumb=bflag1+bflag2+bflag3;
//                 cout<<bflag1<<endl;
//                 cout<<bflag2<<endl;
//                 cout<<bflag3<<endl;
//                 v3d_msg("check");
                if (sumb>0)
                {
                    s.x = x-BlockSize;
                    s.y = y-BlockSize;
                    s.z = z-BlockSize;
                    s.radius = 1;
                    s.color = random_rgba8(255);
                    //cout<<s.x<<","<<s.y<<","<<s.z<<","<<endl;
                    curlist << s;
                }
                //cout<<i<<"//"<<nt.listNeuron.size()<<endl;
                cout<<"branch finished"<<endl;
            }
            if(image_mipx)
            {
                delete []image_mipx;
                image_mipx =0;
            }
            if(image_mipy)
            {
                delete []image_mipy;
                image_mipy =0;
            }
            if(image_mipz)
            {
                delete []image_mipz;
                image_mipz =0;
            }
            if(Block)
            {
                delete []Block;
                Block =0;
            }
            if(image_binary)
            {
                delete []image_binary;
                image_binary =0;
            }

            //callback.setLandmark(curwin, curlist);


        }
    }
    if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    return true;
}
