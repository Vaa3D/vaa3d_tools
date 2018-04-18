#include "get_sub_terafly.h"

#include "find_wrong_area.h"
#define MHDIS(a,b) ( (fabs((a).x-(b).x)) + (fabs((a).y-(b).y)) + (fabs((a).z-(b).z)) )
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
bool get_sub_terafly(V3DPluginCallback2 &callback,QWidget *parent)
{

    QString inimg_file = "/media/lxf/zhang/mouseID_321237-17302/RES(54600x34412x9847)";
    QString pattern = "/media/lxf/8213-B4FE/3.19/Data/finished_11/finished_11/005.swc";
    QString pattern2 = "/media/lxf/8213-B4FE/3.19/Data/auto_tracing/005_x_8584.04_y_8184.8_y_1458.26.swc";
    NeuronTree nt,nt_p,bt,bt_p;
    if (pattern.toUpper().endsWith(".SWC") ||pattern.toUpper().endsWith(".ESWC"))
        nt_p = readSWC_file(pattern);
    double prune_size =100;
    prune_branch(nt_p,nt,prune_size);


    if (pattern2.toUpper().endsWith(".SWC") ||pattern2.toUpper().endsWith(".ESWC"))
        bt_p = readSWC_file(pattern2);
    prune_branch(bt_p,bt,prune_size);

    int st = nt.listNeuron.size()/5;

    NeuronSWC D;
    D.x = 20359;
    D.y = 24409;
    D.z = 1331;

    QList<NeuronSWC> exp1,exp2;
    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {


        if(NTDIS(D,nt.listNeuron[i])<550)
        {
            exp1.push_back(nt.listNeuron[i]);
        }
    }

    for(V3DLONG i=0;i<bt.listNeuron.size();i++)
    {
        if(NTDIS(D,bt.listNeuron[i])<550)
        {
            exp2.push_back(bt.listNeuron[i]);
        }
    }
    QString exp11 = "exp1.swc";
    QString exp22 = "exp2.swc";
    export_list2file(exp1,exp11,exp11);
    export_list2file(exp2,exp22,exp22);





    bool model=1;   //1 for whole img,0 for sub img
    if(model)
    {


        V3DLONG im_cropped_sz[4];
        im_cropped_sz[0] = 400;
        im_cropped_sz[1] = 400;
        im_cropped_sz[2] = 400;
        im_cropped_sz[3] = 1;
        double sum_x=0;
        double sum_y=0;
        double sum_z=0;
        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
        {

            sum_x = sum_x+nt.listNeuron[i].x;
            sum_y = sum_y+nt.listNeuron[i].y;
            sum_z = sum_z+nt.listNeuron[i].z;
        }
        double mean_x,mean_y,mean_z;
//        mean_x = sum_x/nt.listNeuron.size();
//        mean_y = sum_y/nt.listNeuron.size();
//        mean_z = sum_z/nt.listNeuron.size();

        mean_x = 17161;
        mean_y = 16429;
        mean_z = 2911;
        NeuronSWC S;
        S.x = mean_x;
        S.y = mean_y;
        S.z = mean_z;
        cout<<mean_x<<"  "<<mean_y<<"  "<<mean_z<<endl;

        double max_c=0;
        double dis;
        for(V3DLONG i=0;i<nt.listNeuron.size();i++)
        {
            dis = NTDIS(nt.listNeuron[i],S);
            if(dis>max_c)
            {
                max_c = dis;
            }

        }


        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        pagesz = 400*400*400;


        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
        max_c = 200;
        V3DLONG xb = mean_x-max_c;
        V3DLONG xe = mean_x+max_c;
        V3DLONG yb = mean_y-max_c;
        V3DLONG ye = mean_y+max_c;
        V3DLONG zb = mean_z-max_c;
        V3DLONG ze = mean_z+max_c;



        cout<<"hahahahhahahaha"<<endl;
        im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,
                                                  yb,ye+1,zb,ze+1);


        QString outimg_file;
        outimg_file = "test.tif";


        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }
    else
    {
        unsigned char * im_cropped = 0;
        int step = st;
        double lens = 200;
        double ratio = 0.333;
        double L = 2*lens+1;
        double para = ratio*L;
        V3DLONG im_cropped_sz[4];
        im_cropped_sz[0] = L;
        im_cropped_sz[1] = L;
        im_cropped_sz[2] = L;
        im_cropped_sz[3] = 1;

        for(V3DLONG i=0;i<nt.listNeuron.size();i=i+step)
        {


            V3DLONG pagesz;
            pagesz = L*L*L;
            cout<<"pagesz = "<<pagesz<<endl;

            try {im_cropped = new unsigned char [pagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}

//            V3DLONG mean_x = nt.listNeuron[i].x;
//            V3DLONG mean_y = nt.listNeuron[i].y;
//            V3DLONG mean_z = nt.listNeuron[i].z;
            V3DLONG mean_x = 17161;
            V3DLONG mean_y = 16429;
            V3DLONG mean_z = 2911;


            V3DLONG xb = mean_x-lens;
            V3DLONG xe = mean_x+lens;
            V3DLONG yb = mean_y-lens;
            V3DLONG ye = mean_y+lens;
            V3DLONG zb = mean_z-lens;
            V3DLONG ze = mean_z+lens;

//            if(xb<0) xb = 0;
//            if(xe>=511-1) xe = 511-1;
//            if(yb<0) yb = 0;
//            if(ye>=511-1) ye = 511-1;
//            if(zb<0) zb = 0;
//            if(ze>=511-1) ze = 511-1;
            cout<<"get=========="<<endl;
            im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,
                                                      yb,ye+1,zb,ze+1);
            cout<<"done=========="<<endl;

            QString outimg_file;
            outimg_file = "test"+QString::number(i)+".v3dpbd";
            simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);

        }
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }
}


