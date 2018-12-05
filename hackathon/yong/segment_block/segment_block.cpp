#include "segment_block.h"
#include "basic_surf_objs.h"
#include "my_surf_objs.h"
#include "v3d_interface.h"
//V3DLONG o_x;
//V3DLONG o_y;
//V3DLONG o_z;

//bool segment_block(V3DPluginCallback2 &callback,int model,vector<Segment*> &seg_list,PARA &PA,int n)
bool segment_block(V3DPluginCallback2 &callback,vector<Segment*> &seg_list,PARA &PA)
{
    for(V3DLONG i=0;i<seg_list.size();i++)
    {
        double minx = 100000000;
        double miny = 100000000;
        double minz = 100000000;
        double maxx = -100000000;
        double maxy = -100000000;
        double maxz = -100000000;
        QString inimg_file = callback.getPathTeraFly();
        for(V3DLONG j=0;j<seg_list[i]->size();j++)
        {
            Point* pt=seg_list[i]->at(j);
            if(pt->x<minx)
               minx= pt->x;
            if(pt->y<miny)
                miny=pt->y;
            if(pt->z<minz)
                minz=pt->z;
        }
        for(V3DLONG j=0;j<seg_list[i]->size();j++)
        {
          //Point* pt= new Point;
            Point* pt=seg_list[i]->at(j);

            if(pt->x>maxx)
                maxx=pt->x;
            if(pt->y>maxy)
                maxy=pt->y;
            if(pt->z>maxz)
                maxz=pt->z;
        }

        double lenx = maxx - minx;
        double leny = maxy - miny;
        double lenz = maxz - minz;
        int spacex = 0.2*lenx;
        int spacey = 0.2*leny;
        int spacez = 0.2*lenz;
        if(spacex<1)spacex=15;
        if(spacey<1)spacey=15;
        if(spacez<1)spacez=15;


        unsigned char * im_cropped = 0;
        V3DLONG pagesz;

        V3DLONG xb = minx-spacex;
        V3DLONG xe = maxx+spacex;
        V3DLONG yb = miny-spacey;
        V3DLONG ye = maxy+spacey;
        V3DLONG zb = minz-spacez;
        V3DLONG ze = maxz+spacez;
        PA.original_o[0] = xb;
        PA.original_o[1] = yb;
        PA.original_o[2] = zb;

        pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
        PA.im_cropped_sz[0] = xe-xb+1; //problem here
        PA.im_cropped_sz[1] = ye-yb+1;
        PA.im_cropped_sz[2] = ze-zb+1;
        PA.im_cropped_sz[3] = 1;

        cout<<" PA.Z = "<<PA.im_cropped_sz[2]<<endl;

        cout<<"x = "<<maxx-xb<<"  "<<minx-xb<<endl;
        cout<<"y = "<<maxy-yb<<"  "<<miny-yb<<endl;
        cout<<"z = "<<maxz-zb<<"  "<<minz-zb<<endl;

        cout<<"space "<<spacex<<"  "<<spacey<<"  "<<spacez<<"  "<<endl;

        cout<<"begin = "<<xb<<"  "<<yb<<"  "<<zb<<endl;
        cout<<"end = "<<xe<<"  "<<ye<<"  "<<ze<<endl;
        //v3d_msg("test!");


        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}


        im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,
                                                  yb,ye+1,zb,ze+1);


        QString outimg_file;
        outimg_file = "segment_block"+ QString::number(i)+".tif";
        PA.img_name = outimg_file;

        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,PA.im_cropped_sz,1);
        PA.data1d = im_cropped;
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}

        QList<NeuronSWC> seg_result2;
        seg_result2.clear();
        for(V3DLONG j=0;j<seg_list[i]->size();j++)
        {
            NeuronSWC S;
            Point* pt= new Point;
            pt=seg_list[i]->at(j);
            S.n=j;
            S.parent=j-1;
            S.x=pt->x-PA.original_o[0];
            S.y=pt->y-PA.original_o[1];
            S.z=pt->z-PA.original_o[2];
            S.r=pt->r;
            S.type=pt->type;

            seg_result2.push_back(S);

        }
        QString filename;
        NeuronTree seg_nt2;
        seg_nt2.listNeuron=seg_result2;
        filename=QString("segment"+QString::number(i)+".swc");
        writeSWC_file(filename,seg_nt2);
   }

    return true;
}
