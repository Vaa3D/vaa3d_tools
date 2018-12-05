/* test_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-5-24 : by YourName
 */

#include "v3d_message.h"
#include <vector>
#include "Branch_c_plugin.h"
#include <iostream>
#include <cmath>
#include "rayshooting_func.h"
using namespace std;
#define PI 3.1415926
Q_EXPORT_PLUGIN2(example, Branch_c);

QStringList Branch_c::menulist() const
{
    return QStringList()
            <<tr("Branch_c")
              <<tr("draw")
           <<tr("about");
}

QStringList Branch_c::funclist() const
{
    return QStringList()
            <<tr("Branch_c")
           <<tr("help");
}

void Branch_c::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Branch_c"))
    {
        Branch_detection(callback,parent);
    }
    else if (menu_name == tr("draw"))
    {
       raymodel(callback,parent);
    }
    else if (menu_name == tr("about"))
    {
        v3d_msg(tr("This is a demo plugin to perform binary thresholding on the current image.\n"
                   "Developed by Weixun Chen, 2018-05-24"));
    }
    else
    {
        v3d_msg(tr("This is a demo plugin to perform binary thresholding on the current image.\n"
                   "Developed by Weixun Chen, 2018-05-24"));
    }
}
bool Branch_c::dofunc(const QString & menu_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (menu_name == tr("Branch_c"))
    {
        Branch_detection(callback,parent);
    }
    else if (menu_name == tr("about"))
    {
        v3d_msg(tr("This is a demo plugin to perform binary thresholding on the current image.\n"
                   "Developed by Weixun Chen, 2018-05-24"));
    }
    else
    {
        v3d_msg(tr("This is a demo plugin to perform binary thresholding on the current image.\n"
                   "Developed by Weixun Chen, 2018-05-24"));
    }
    return 1;
}
int Branch_detection(V3DPluginCallback2 &callback, QWidget *parent)
{
    //get image
    int ray_numbers_2d=64;//
    int ray_length_2d=12;//
    unsigned char* data1d = 0;

    int dsize;
    int T1=113;
    int thres_2d=65;
    int N,M,P,C;
    v3dhandle curwin = callback.currentImageWindow();
    Image4DSimple* p4DImage = callback.getImage(curwin);
    data1d = p4DImage->getRawData();
    // get image size
    N = p4DImage->getXDim();
    M = p4DImage->getYDim();
    P = p4DImage->getZDim();
    C = p4DImage->getCDim();

    QDialog * dialog = new QDialog();


    if(p4DImage->getZDim() > 1)
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
ray_length_2d_spinbox->setRange(1,p4DImage->getXDim());
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

if(dialog->exec() != QDialog::Accepted)
    {
            if (dialog)
            {
                    delete dialog;
                    dialog=0;
                    cout<<"delete dialog"<<endl;
            }
            return -1;
    }

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
    catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
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
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                                                    "",
                                                    QObject::tr("Supported file (*.swc *.eswc)"
                                                                ";;Neuron structure	(*.swc)"
                                                                ";;Extended neuron structure (*.eswc)"
                                                                ));
        if(fileOpenName.isEmpty())
            return 1;
        NeuronTree nt;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
            nt = readSWC_file(fileOpenName);
            unsigned char * Block = 0;
            int end_x,start_x,end_y,start_y,end_z,start_z;
            //v3d_msg(QString("%1").arg(nt.listNeuron.size()));
            //detection loop
            LandmarkList curlist;
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
                if(0)
                {
                    bwlabel=0;
                    for (int i1=0;i1<pagesz;i1++)
                    {
                        if(Block[i1]==255)
                        {
                            bwlabel++;
                            //cout<<i<<endl;
                            //cout<<bwlabel<<endl;
                            Block[i1]=bwlabel;
                            for(int k=-1;k<2;k++)
                            {
                                ik=i1+k*M1;
                                for(int j=-1;j<2;j++)
                                {
                                    if((ik+j*M1*N1-1)>=0&&(ik+j*M1*N1-1)<pagesz&&Block[ik+j*M1*N1-1]==255)
                                    {stackb.push_back(ik+j*M1*N1-1);Block[ik+j*M1*N1-1]=200;}
//                                    v3d_msg("check1");

                                    if((ik+j*M1*N1)>=0&&(ik+j*M1*N1)<pagesz&&Block[ik+j*M1*N1]==255)
                                    {stackb.push_back(ik+j*M1*N1);Block[ik+j*M1*N1]=200;}
                                    //v3d_msg("check2");

                                    if((ik+j*M1*N1+1)>=0&&(ik+j*M1*N1+1)<pagesz&&Block[ik+j*M1*N1+1]==255)
                                    {stackb.push_back(ik+j*M1*N1+1);Block[ik+j*M1*N1+1]=200;}
                                    //v3d_msg("check3");
                                }
                            }
                            //cout<<stackb.size()<<endl;
                        }
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
                if(0)
                {
                    ind2=Block[CenterPoint_ind];
                    cout<<ind2<<endl;
                    v3d_msg("check6");
                    if(ind2>0)
                    {
                        for(int i2=0;i2<pagesz;i2++)
                        {
                            if(Block[i2]==ind2)
                                Block[i2]=255;
                            else
                                Block[i2]=0;
                        }
                    }
                    else
                    {
                        for(int k=-1;k<2;k++)
                        {
                            if(ind2==0)
                            {indk=CenterPoint_ind+k*M1;
                                for(int j=-1;j<2;j++)
                                {
                                    if(Block[indk+j*M1*N1-1]>0)
                                    {ind2=Block[indk+j*M1*N1-1];break;}
                                    //v3d_msg("check4");

                                    if(Block[indk+j*M1*N1]>0)
                                    {ind2=Block[indk+j*M1*N1];break;}
                                    //v3d_msg("check5");

                                    if(Block[indk+j*M1*N1+1]>0)
                                    {ind2=Block[indk+j*M1*N1+1];break;}
                                    //v3d_msg("check6");
                                }
                            }
                        }
                        if(ind2>0)
                        {
                            for(int i2=0;i2<pagesz;i2++)
                            {
                                if(Block[i2]==ind2)
                                    Block[i2]=255;
                                else
                                    Block[i2]=0;
                            }
                        }
                    }
                }
                //test show block
                if(showflag)
                {
                    Image4DSimple * new4DImage = new Image4DSimple();
                    new4DImage->setData((unsigned char *)Block,1+2*BlockSize, 1+2*BlockSize, 1+2*BlockSize, 1, V3D_UINT8);
                    v3dhandle newwin = callback.newImageWindow();
                    callback.setImage(newwin, new4DImage);
                    callback.setImageName(newwin, "Block");
                    callback.updateImageWindow(newwin);
                    callback.open3DWindow(newwin);
                    cout<<c1<<endl;
                    cout<<c2<<endl;
                    cout<<c3<<endl;
                    cout<<c4<<endl;
                    v3d_msg("check");
                    callback.close3DWindow(newwin);
                }
                //

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
                        if(showflag)
                        {new4DImagez = new Image4DSimple();
                            new4DImagez->setData((unsigned char *)image_mipz,1+2*BlockSize, 1+2*BlockSize, 1, 1, V3D_UINT8);
                            newwinz = callback.newImageWindow();
                            callback.setImage(newwinz, new4DImagez);
                            callback.setImageName(newwinz, "Block");
                            callback.updateImageWindow(newwinz);
                            callback.open3DWindow(newwinz);
                            v3d_msg("check");
                            callback.close3DWindow(newwinz);}
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
                        if(showflag)
                        {new4DImagey = new Image4DSimple();
                            new4DImagey->setData((unsigned char *)image_mipy,1+2*BlockSize, 1+2*BlockSize, 1, 1, V3D_UINT8);
                            newwiny = callback.newImageWindow();
                            callback.setImage(newwiny, new4DImagey);
                            callback.setImageName(newwiny, "Block");
                            callback.updateImageWindow(newwiny);
                            callback.open3DWindow(newwiny);
                            v3d_msg("check");
                            callback.close3DWindow(newwiny);}
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
                        if(showflag)
                        {new4DImagex = new Image4DSimple();
                            new4DImagex->setData((unsigned char *)image_mipx,1+2*BlockSize, 1+2*BlockSize, 1, 1, V3D_UINT8);
                            newwinx = callback.newImageWindow();
                            callback.setImage(newwinx, new4DImagex);
                            callback.setImageName(newwinx, "Block");
                            callback.updateImageWindow(newwinx);
                            callback.open3DWindow(newwinx);
                            v3d_msg("check");
                            callback.close3DWindow(newwinx);}
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
            //detection finished
            //cout<<"branch finished"<<endl;
            // Clustering
            if(0)
            {for(int i=0;i<curlist.size();i++)
                    for(int j=i+1;j<curlist.size();j++)
                    {

                        {
                            if(square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y)+square(curlist[j].z-curlist[i].z)<150)
                            {
                                curlist.removeAt(j);
                                j = j - 1;
                            }
                        }
                    }
            }
            //
            //show results
            callback.setLandmark(curwin, curlist);
        }
    }
    return 1;
}
void printHelp()
{
    cout<<"\nThis is a demo plugin to detect tip point in an image. by Keran Lin 2017-04"<<endl;
    cout<<"\nUsage: v3d -x <example_plugin_name> -f tip_detection -i <input_image_file> -o <output_image_file> -p <subject_color_channel> <threshold>"<<endl;
    cout<<"\t -i <input_image_file>                      input 3D image (tif, raw or lsm)"<<endl;
    cout<<"\t -o <output_image_file>                     output image of the thresholded subject channel"<<endl;
    cout<<"\t                                            the  paras must come in this order"<<endl;
    cout<<"\nDemo: v3d -x libexample_debug.dylib -f image_thresholding -i input.tif -o output.tif -p 0 100\n"<<endl;
    return;
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
