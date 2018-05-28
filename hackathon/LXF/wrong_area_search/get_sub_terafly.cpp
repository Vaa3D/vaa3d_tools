#include "get_sub_terafly.h"

#include "find_wrong_area.h"
#define MHDIS(a,b) ( (fabs((a).x-(b).x)) + (fabs((a).y-(b).y)) + (fabs((a).z-(b).z)) )
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
bool get_sub_terafly(V3DPluginCallback2 &callback,QWidget *parent)
{



    bool model=0;   //1 for whole img,0 for sub img
    if(model)
    {
        QString inimg_file = "/home/penglab/mouseID_321237-17302/RES(54600x34412x9847)";
        LandmarkList terafly_landmarks = callback.getLandmarkTeraFly();
        QString pattern = "/home/penglab/Desktop/037_05082018.ano.swc";
        NeuronTree nt;
        if (pattern.toUpper().endsWith(".SWC") ||pattern.toUpper().endsWith(".ESWC"))
            nt = readSWC_file(pattern);
        for(V3DLONG i=0;i<terafly_landmarks.size();i++)
        {
            LocationSimple t;
            t.x = terafly_landmarks[i].x;
            t.y = terafly_landmarks[i].y;
            t.z = terafly_landmarks[i].z;





            V3DLONG im_cropped_sz[4];







            unsigned char * im_cropped = 0;
            V3DLONG pagesz;
            double l_x = 256;
            double l_y = 256;
            double l_z = 128;



            V3DLONG xb = t.x-l_x;
            V3DLONG xe = t.x+l_x-1;
            V3DLONG yb = t.y-l_y;
            V3DLONG ye = t.y+l_y-1;
            V3DLONG zb = t.z-l_z;
            V3DLONG ze = t.z+l_z-1;
            pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
            im_cropped_sz[0] = xe-xb+1;
            im_cropped_sz[1] = ye-yb+1;
            im_cropped_sz[2] = ze-zb+1;
            im_cropped_sz[3] = 1;


            try {im_cropped = new unsigned char [pagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}




//            cout<<"begin ==================="<<xb<<"  "<<yb<<"  "<<"  "<<zb<<endl;
//            cout<<"end   ==================="<<xe<<"  "<<ye<<"  "<<"  "<<ze<<endl;
//            v3d_msg("test!");

            QList<NeuronSWC> outswc;
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
                    outswc.push_back(p);
                }
            }


            im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,
                                                      yb,ye+1,zb,ze+1);


            QString outimg_file,outswc_file;
            outimg_file = "/home/penglab/Desktop/data_for_hunanuniversity/img/test"+QString::number(i)+".tif";
            outswc_file = "/home/penglab/Desktop/data_for_hunanuniversity/swc/test"+QString::number(i)+".swc";

            export_list2file(outswc,outswc_file,outswc_file);
            simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
            if(im_cropped) {delete []im_cropped; im_cropped = 0;}

        }
    }
    else
    {
        QString inimg_file = "/home/penglab/mouseID_321237-17302/RES(54600x34412x9847)";
        //QString inimg_file = "/home/penglab/mouseID_321237-17302/RES(54600x34412x9847)";
        QString pattern = "/home/penglab/Desktop/037_05082018.ano.swc";
        //QString inimg_file = "/media/lxf/zhang/mouseID_321237-17302/RES(54600x34412x9847)";
        //QString pattern = "/media/lxf/8213-B4FE/3.19/Data/finished_11/finished_11/005.swc";
        //QString pattern2 = "/media/lxf/8213-B4FE/3.19/Data/auto_tracing/005_x_8584.04_y_8184.8_y_1458.26.swc";


        NeuronTree nt,nt_p,bt,bt_p;
        if (pattern.toUpper().endsWith(".SWC") ||pattern.toUpper().endsWith(".ESWC"))
            nt = readSWC_file(pattern);
        //double prune_size =100;
        //prune_branch(nt_p,nt,prune_size);


        //nt_p = nt;
        int st = nt.listNeuron.size()/100;





        V3DLONG im_cropped_sz[4];

        int i=0;
        while(i<19576)
        {
            NeuronSWC S;
            S.x = nt.listNeuron[i].x;
            S.y = nt.listNeuron[i].y;
            S.z = nt.listNeuron[i].z;

            double l_x = 256;
            double l_y = 256;
            double l_z = 128;
            V3DLONG xb = S.x-l_x;
            V3DLONG xe = S.x+l_x-1;
            V3DLONG yb = S.y-l_y;
            V3DLONG ye = S.y+l_y-1;
            V3DLONG zb = S.z-l_z;
            V3DLONG ze = S.z+l_z-1;


            unsigned char * im_cropped = 0;
            V3DLONG pagesz;

            pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
            im_cropped_sz[0] = xe-xb+1;
            im_cropped_sz[1] = ye-yb+1;
            im_cropped_sz[2] = ze-zb+1;
            im_cropped_sz[3] = 1;

            try {im_cropped = new unsigned char [pagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}



//            cout<<"begin ==================="<<xb<<"  "<<yb<<"  "<<"  "<<zb<<endl;
//            cout<<"end   ==================="<<xe<<"  "<<ye<<"  "<<"  "<<ze<<endl;
            QList<NeuronSWC> outswc;
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
                    outswc.push_back(p);
                }
            }




            im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,
                                                  yb,ye+1,zb,ze+1);


            QString outimg_file,outswc_file;
            outimg_file = "/home/penglab/Desktop/data_for_hunanuniversity/img/test"+QString::number(i)+".tif";
            outswc_file = "/home/penglab/Desktop/data_for_hunanuniversity/swc/test"+QString::number(i)+".swc";
            export_list2file(outswc,outswc_file,outswc_file);


            simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
            if(im_cropped) {delete []im_cropped; im_cropped = 0;}

            QList<ImageMarker> listmarker,listmarker_o;

            ImageMarker u,u_o;


            u_o.x = S.x;
            u_o.y = S.y;
            u_o.z = S.z;
            u.x = S.x-xb;
            u.y = S.y-yb;
            u.z = S.z-zb;
            //v3d_msg("this is marker info");
            cout<<"this is marker info"<<endl;
            cout<<u_o.x<<"  "<<u_o.y<<"  "<<u_o.z<<endl;
            cout<<u.x<<"  "<<u.y<<"  "<<u.z<<endl;
            listmarker.push_back(u);
            listmarker_o.push_back(u_o);
            writeMarker_file(QString(QString::number(i)+".marker"),listmarker);
            writeMarker_file(QString(QString::number(i)+"_o.marker"),listmarker_o);


            i=i+100;
        }


   }
}


