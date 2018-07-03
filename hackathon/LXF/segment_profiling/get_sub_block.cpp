#include "get_sub_block.h"



bool get_sub_block(V3DPluginCallback2 &callback,int model,NeuronTree &seg_m,PARA &PA,int n)
{
    cout<<"get_sub_block"<<endl;

    if(model)
    {
        double minx = 100000000;
        double miny = 100000000;
        double minz = 100000000;
        double maxx = -100000000;
        double maxy = -100000000;
        double maxz = -100000000;
        QString inimg_file = callback.getPathTeraFly();
        cout<<"inimg = "<<inimg_file.toStdString()<<endl;
        for(V3DLONG i = 0;i < seg_m.listNeuron.size();i++)
        {
            NeuronSWC curr = seg_m.listNeuron[i];
            if(curr.x<minx)
                minx = curr.x;
            if(curr.y<miny)
                miny = curr.y;
            if(curr.z<minz)
                minz = curr.z;
        }
        for(V3DLONG i = 0;i < seg_m.listNeuron.size();i++)
        {
            NeuronSWC curr2 = seg_m.listNeuron[i];
            if(curr2.x>maxx)
                maxx = curr2.x;
            if(curr2.y>maxy)
                maxy = curr2.y;
            if(curr2.z>maxz)
                maxz = curr2.z;
        }
        double lenx = maxx - minx;
        double leny = maxy - miny;
        double lenz = maxz - minz;
        int spacex = 0.4*lenx;
        int spacey = 0.4*leny;
        int spacez = 0.4*lenz;
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
        cout<<"--------------------------------------PA.Z = "<<PA.im_cropped_sz[2]<<endl;

        cout<<"x = "<<maxx-xb<<"  "<<minx-xb<<endl;
        cout<<"y = "<<maxy-yb<<"  "<<miny-yb<<endl;
        cout<<"z = "<<maxz-zb<<"  "<<minz-zb<<endl;

        cout<<"space "<<spacex<<"  "<<spacey<<"  "<<spacez<<"  "<<endl;

        cout<<"begin ==================="<<xb<<"  "<<yb<<"  "<<zb<<endl;
        cout<<"end   ==================="<<xe<<"  "<<ye<<"  "<<ze<<endl;
        //v3d_msg("test!");


        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}







        im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,
                                                  yb,ye+1,zb,ze+1);


        QString outimg_file;
        outimg_file = "segment_profiling_"+ QString::number(n)+".tif";
        PA.img_name = outimg_file;

        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,PA.im_cropped_sz,1);
        PA.data1d = im_cropped;
        return true;
        //if(im_cropped) {delete []im_cropped; im_cropped = 0;}

    }
}


