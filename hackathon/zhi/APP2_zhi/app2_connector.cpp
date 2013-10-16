//last change: by Hanchuan Peng. 2012-12-30, 2013-06-05

#include "vn_app2.h"

#include "fastmarching_tree.h"
#include "fastmarching_dt.h"
#include "hierarchy_prune.h"
#include "marker_radius.h"
#include "basic_surf_objs.h"
#include "swc_convert.h"
#include <iostream>

#include "vn_imgpreprocess.h"

#include "volimg_proc.h"


struct root_node
{
    V3DLONG root_x;
    V3DLONG root_y;
    V3DLONG root_z;
    V3DLONG parent;
    struct root_node* next;


};

template <class T> void app2_block(V3DPluginCallback2 &callback,MyMarker root, T * somald,  V3DLONG *soma_sz,T * indata1d,V3DLONG *in_sz, PARA_APP2 &p,double dfactor_xy, double dfactor_z,V3DLONG *boundary,struct root_node *head,V3DLONG root_parent)
{
    cout<<"======================================="<<endl;
    cout<<"Construct the neuron tree"<<endl;
    vector<MyMarker *> outtree;
    fastmarching_tree(root, somald, outtree, soma_sz[0], soma_sz[1], soma_sz[2], p.cnn_type, p.bkg_thresh, p.is_break_accept);

    cout<<"======================================="<<endl;

    //save a copy of the ini tree
    cout<<"Save the initial unprunned tree"<<endl;
    V3DLONG xb =  boundary[0];
    V3DLONG xe =  boundary[1];
    V3DLONG yb =  boundary[2];
    V3DLONG ye =  boundary[3];

    vector<MyMarker*> & inswc = outtree;
    V3DLONG i;
    if (1)
    {
        V3DLONG tmpi;

        vector<MyMarker*> tmpswc;
        for (tmpi=0; tmpi<inswc.size(); tmpi++)
        {
            MyMarker * curp = new MyMarker(*(inswc[tmpi]));
            tmpswc.push_back(curp);

            if (dfactor_xy>1) inswc[tmpi]->x *= dfactor_xy;
            inswc[tmpi]->x += (p.xc0);
            if (dfactor_xy>1) inswc[tmpi]->x += dfactor_xy/2;

            if (dfactor_xy>1) inswc[tmpi]->y *= dfactor_xy;
            inswc[tmpi]->y += (p.yc0);
            if (dfactor_xy>1) inswc[tmpi]->y += dfactor_xy/2;

            if (dfactor_z>1) inswc[tmpi]->z *= dfactor_z;
            inswc[tmpi]->z += (p.zc0);
            if (dfactor_z>1)  inswc[tmpi]->z += dfactor_z/2;
        }

        QString rootposstr="", tmps;
        tmps.setNum(int(root.x+0.5+xb)).prepend("_x"); rootposstr += tmps;
        tmps.setNum(int(root.y+0.5+yb)).prepend("_y"); rootposstr += tmps;
        tmps.setNum(int(root.z+0.5)).prepend("_z"); rootposstr += tmps;
        QString outswc_file_ini = QString(p.p4dImage->getFileName()) + rootposstr + ".v3draw";

       // saveSWC_file(outswc_file_ini.toStdString(), inswc);

        for (tmpi=0; tmpi<inswc.size(); tmpi++)
        {
            inswc[tmpi]->x = tmpswc[tmpi]->x;
            inswc[tmpi]->y = tmpswc[tmpi]->y;
            inswc[tmpi]->z = tmpswc[tmpi]->z;
        }

        for(tmpi = 0; tmpi < tmpswc.size(); tmpi++)
            delete tmpswc[tmpi];
        tmpswc.clear();
       // std:: string fileName_Qstring2(outswc_file_ini.toStdString());char* fileName_string2 =  new char[fileName_Qstring2.length() + 1]; strcpy(fileName_string2, fileName_Qstring2.c_str());
       // simple_saveimage_wrapper(callback, fileName_string2,  (unsigned char *)somald, soma_sz, V3D_UINT8);

    }


   cout<<"Pruning neuron tree"<<endl;
   if (1)
   {
    vector<MyMarker*> outswc;
    if(p.is_coverage_prune)
    {
        v3d_msg("start to use happ.\n", 0);
        happ(inswc, outswc, somald, soma_sz[0], soma_sz[1], soma_sz[2], p.bkg_thresh, p.length_thresh, p.SR_ratio);
    }
    else
    {
        hierarchy_prune(inswc, outswc, somald, soma_sz[0], soma_sz[1], soma_sz[2], p.length_thresh);
        if(1) //get radius
        {
            double real_thres = 40; //PHC 20121011
            if (real_thres<p.bkg_thresh) real_thres = p.bkg_thresh;
            for(i = 0; i < outswc.size(); i++)
            {
                outswc[i]->radius = markerRadius(somald, soma_sz, *(outswc[i]), real_thres);
            }
        }
    }

        QString rootposstr="", tmps;
        tmps.setNum(int(root.x+0.5+xb)).prepend("_x"); rootposstr += tmps;
        tmps.setNum(int(root.y+0.5+yb)).prepend("_y"); rootposstr += tmps;
        tmps.setNum(int(root.z+0.5)).prepend("_z"); rootposstr += tmps;
        QString outswc_file = QString(p.p4dImage->getFileName()) + rootposstr + "_app2_temp.swc";

       // QString blockstr="", tmps2;
     //   tmps2.setNum(int(xb)).prepend("_x"); blockstr += tmps2;
     //   tmps2.setNum(int(yb)).prepend("_y"); blockstr += tmps2;
        QString blockswc_file = QString(p.p4dImage->getFileName()) + "_app2_v4.swc";


        if(outswc.size() > 1)
        {
            struct root_node *walker_inside;
            struct root_node *newNode;
            walker_inside = head;
            while(walker_inside->next != NULL)
            {
                 walker_inside = walker_inside->next;
            }


            for(V3DLONG d = 0; d < inswc.size(); d++)
            {

                  V3DLONG dz = inswc[d]->z;
                  V3DLONG dy = inswc[d]->y + yb;
                  V3DLONG dx = inswc[d]->x + xb;
                 if(dx < xe -4 && dy < ye-4 && dx >= xb+3 && dy >= yb+3)
                 {
                      indata1d[dz*in_sz[0]*in_sz[1] + dy*in_sz[0] + dx] = 0;
                }

            }

            for(i = 0; i < outswc.size(); i++) //add scaling 121127, PHC //add cutbox offset 121202, PHC
            {
                if (dfactor_xy>1) outswc[i]->x *= dfactor_xy;
                outswc[i]->x += (p.xc0);
                if (dfactor_xy>1) outswc[i]->x += dfactor_xy/2; //note that the offset corretion might not be accurate. PHC 121127

                if (dfactor_xy>1) outswc[i]->y *= dfactor_xy;
                outswc[i]->y += (p.yc0);
                if (dfactor_xy>1) outswc[i]->y += dfactor_xy/2;

                if (dfactor_z>1) outswc[i]->z *= dfactor_z;
                outswc[i]->z += (p.zc0);
                if (dfactor_z>1)  outswc[i]->z += dfactor_z/2;

                outswc[i]->radius *= dfactor_xy; //use xy for now

            }

            //re-estimate the radius using the original image
            double real_thres = 40; //PHC 20121011
            if (real_thres<p.bkg_thresh) real_thres = p.bkg_thresh;
            int method_radius_est = ( p.b_RadiusFrom2D ) ? 1 : 2;
            for(i = 0; i < outswc.size(); i++)
            {
               //printf(" node %ld of %ld.\n", i, outswc.size());
              outswc[i]->radius = markerRadius(somald, soma_sz, *(outswc[i]), real_thres, method_radius_est);
              outswc[i]->x = outswc[i]->x + xb;
              outswc[i]->y = outswc[i]->y + yb;
            }
            //prepare the output comments for neuron info in the swc file

            saveSWC_file(outswc_file.toStdString(), outswc);

            //call sort_swc function
             V3DPluginArgItem arg;
             V3DPluginArgList input_resample;
             V3DPluginArgList input_sort;
             V3DPluginArgList output;

             arg.type = "random";std::vector<char*> arg_input_resample;
             std:: string fileName_Qstring(outswc_file.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());
             arg_input_resample.push_back(fileName_string);
             arg.p = (void *) & arg_input_resample; input_resample<< arg;
             arg.type = "random";std::vector<char*> arg_resample_para; arg_resample_para.push_back("10");arg.p = (void *) & arg_resample_para; input_resample << arg;
             arg.type = "random";std::vector<char*> arg_output;arg_output.push_back(fileName_string); arg.p = (void *) & arg_output; output<< arg;

             QString full_plugin_name_resample = "resample_swc";
             QString func_name_resample = "resample_swc";
             callback.callPluginFunc(full_plugin_name_resample,func_name_resample,input_resample,output);

             arg.type = "random";std::vector<char*> arg_input_sort;
             arg_input_sort.push_back(fileName_string);
             arg.p = (void *) & arg_input_sort; input_sort<< arg;
             arg.type = "random";std::vector<char*> arg_sort_para; arg_sort_para.push_back("0");arg.p = (void *) & arg_sort_para; input_sort << arg;
             QString full_plugin_name_sort = "sort_neuron_swc";
             QString func_name_sort = "sort_swc";
             callback.callPluginFunc(full_plugin_name_sort,func_name_sort, input_sort,output);

             vector<MyMarker*> temp_out_swc = readSWC_file(outswc_file.toStdString());
             ifstream ifs(blockswc_file.toStdString().c_str());

             for(V3DLONG d = 0; d < temp_out_swc.size(); d++)
             {
                  V3DLONG dx = temp_out_swc[d]->x;
                  V3DLONG dy = temp_out_swc[d]->y;
                  V3DLONG dz = temp_out_swc[d]->z;
                 if((xe < in_sz[0]-1 && dx >= xe-4)|| (ye < in_sz[1]-1 && dy >= ye-4)|| (xb >0 && dx <= xb+3) || (yb > 0 && dy <= yb+3))
                 {
                         newNode =  new root_node[1];
                         newNode->root_x = dx;
                         newNode->root_y = dy;
                         newNode->root_z = dz;
                         if(ifs)
                         {
                             vector<MyMarker*> block_out_swc = readSWC_file(blockswc_file.toStdString());
                             newNode->parent = block_out_swc.size() + d;
                         }
                         else
                            newNode->parent  = d;
                         newNode->next = NULL;
                         walker_inside->next = newNode;
                         walker_inside = walker_inside->next;
                 }
             }



             if(ifs)
             {
               vector<MyMarker*> block_out_swc = readSWC_file(blockswc_file.toStdString());
               temp_out_swc[0]->parent = block_out_swc[root_parent];
               for(int j = 0; j < temp_out_swc.size(); j++)
               {
                   block_out_swc.push_back(temp_out_swc[j]);
               }
                saveSWC_file(blockswc_file.toStdString(), block_out_swc);
             }
             else
             {

                 saveSWC_file(blockswc_file.toStdString(), temp_out_swc);
             }

             remove(outswc_file.toStdString().c_str());
             for(V3DLONG i = 0; i < outtree.size(); i++) delete outtree[i];
             outtree.clear();
        }
   }
        return;

}

bool proc_app2(V3DPluginCallback2 &callback, PARA_APP2 &p, const QString & versionStr)
{
    bool b_menu = true;
    
    if (!p.p4dImage || !p.p4dImage->valid())
    {
        if (p.inimg_file.isEmpty())
            return false;
        
        b_menu = false;
        
        //in this case try to read the image files
        QString infile = p.inimg_file;
        p.p4dImage = callback.loadImage((char *)(qPrintable(infile) ));
        if (!p.p4dImage || !p.p4dImage->valid())
            return false;
        else
        {
            p.xc0 = p.yc0 = p.zc0 = 0;
            p.xc1 = p.p4dImage->getXDim()-1;
            p.yc1 = p.p4dImage->getYDim()-1;
            p.zc1 = p.p4dImage->getZDim()-1;
        }
        
        vector<MyMarker> file_inmarkers; 
        if(!p.inmarker_file.isEmpty()) 
            file_inmarkers = readMarker_file(string(qPrintable(p.inmarker_file)));
        
        LocationSimple t;
        for(int i = 0; i < file_inmarkers.size(); i++)
        {
            t.x = file_inmarkers[i].x;
            t.y = file_inmarkers[i].y;
            t.z = file_inmarkers[i].z;
            p.landmarks.push_back(t);
        }
    }
    
    int i;
    list<string>::iterator it;
    
    //these info should also be output to the swc file. need to add later. Noted by PHC, 121124
    list<string> infostring;
    string tmpstr; QString qtstr;
    tmpstr =  qPrintable( qtstr.prepend("##Vaa3D-Neuron-APP2 ").append(versionStr) ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.channel).prepend("#channel = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.bkg_thresh).prepend("#bkg_thresh = ") ); infostring.push_back(tmpstr);

    tmpstr =  qPrintable( qtstr.setNum(p.length_thresh).prepend("#length_thresh = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.SR_ratio).prepend("#SR_ratio = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.is_gsdt).prepend("#is_gsdt = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.cnn_type).prepend("#cnn_type = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_256cube).prepend("#b_256cube = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_RadiusFrom2D).prepend("#b_radiusFrom2D = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.xc0).prepend("#xc0 = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.xc1).prepend("#xc1 = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.yc0).prepend("#yc0 = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.yc1).prepend("#yc1 = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.zc0).prepend("#zc0 = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.zc1).prepend("#zc1 = ") ); infostring.push_back(tmpstr);
    
    v3d_msg("start to preprocessing.\n", 0);
    
    QElapsedTimer timer1;
    timer1.start();
    
    //copy data to a temp buffer, so that won't influence the original input data
    Image4DSimple * p4dImageNew = 0;
    
    if (p.xc1>=p.xc0 && p.yc1>=p.yc0 && p.zc1>=p.zc0 &&
        p.xc0>=0 && p.xc1<p.p4dImage->getXDim() &&
        p.yc0>=0 && p.yc1<p.p4dImage->getYDim() &&
        p.zc0>=0 && p.zc1<p.p4dImage->getZDim())
    {
        p4dImageNew = new Image4DSimple;
        if(!p4dImageNew->createImage(p.xc1-p.xc0+1, p.yc1-p.yc0+1, p.zc1-p.zc0+1, 1, p.p4dImage->getDatatype()))
            return false;
        
        if (!subvolumecopy(p4dImageNew,
                           p.p4dImage,
                           p.xc0, p.xc1-p.xc0+1,
                           p.yc0, p.yc1-p.yc0+1,
                           p.zc0, p.zc1-p.zc0+1,
                           p.channel, 1))
            return false;
    }
    else
    {
        v3d_msg("Somehow invalid volume box info is detected. Ignore it. But check your Vaa3D program.");
        return false;
    }
    
    //always just use the buffer data
    unsigned char * indata1d = p4dImageNew->getRawDataAtChannel(0);
    V3DLONG in_sz[4] = {p4dImageNew->getXDim(), p4dImageNew->getYDim(), p4dImageNew->getZDim(), 1};
    int datatype = p.p4dImage->getDatatype();
    
    double dfactor_xy = 1, dfactor_z = 1;
    if(datatype != V3D_UINT8 || in_sz[0]>256 || in_sz[1]>256 || in_sz[2]>256)// && datatype != V3D_UINT16)
    {
        if (datatype!=V3D_UINT8)
        {
            if (!scale_img_and_convert28bit(p4dImageNew, 0, 255))
                return false;
            
            indata1d = p4dImageNew->getRawDataAtChannel(0);
            in_sz[0] = p4dImageNew->getXDim();
            in_sz[1] = p4dImageNew->getYDim();
            in_sz[2] = p4dImageNew->getZDim();
            in_sz[3] = p4dImageNew->getCDim();
            
            datatype = V3D_UINT8;
        }
        
        printf("x = %ld  ", in_sz[0]);
        printf("y = %ld  ", in_sz[1]);
        printf("z = %ld  ", in_sz[2]);
        printf("c = %ld\n", in_sz[3]);
        
        if (p.b_256cube)
        {
            if (in_sz[0]<=256 && in_sz[2]<=256 && in_sz[2]<=256)
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
                    return false; //need to clean memory before return. a bug here
                
                p4dImageNew->setData(outimg, out_sz[0], out_sz[1], out_sz[2], out_sz[3], V3D_UINT8);
                
                indata1d = p4dImageNew->getRawDataAtChannel(0);
                in_sz[0] = p4dImageNew->getXDim();
                in_sz[1] = p4dImageNew->getYDim();
                in_sz[2] = p4dImageNew->getZDim();
                in_sz[3] = p4dImageNew->getCDim();
            }
            
        }
    }
    
    //QString outtmpfile = QString(p.p4dImage->getFileName()) + "_extract_tmp000.raw";
    //p4dImageNew->saveImage(qPrintable(outtmpfile));  v3d_msg(QString("save immediate input image to ") + outtmpfile, 0);
    
    if (p.bkg_thresh < 0)
    {
        if (p.channel >=0 && p.channel <= p.p4dImage->getCDim()-1)
        {
            double imgAve, imgStd;
            mean_and_std(p4dImageNew->getRawDataAtChannel(0), p4dImageNew->getTotalUnitNumberPerChannel(), imgAve, imgStd);
            p.bkg_thresh = imgAve; //+0.5*imgStd ; //(imgAve < imgStd)? imgAve : (imgAve+imgStd)*.5;
        }
        else
            p.bkg_thresh = 0;
        
        tmpstr =  qPrintable( qtstr.setNum(p.bkg_thresh).prepend("autoset #bkg_thresh = ") ); infostring.push_back(tmpstr);
    }

    
    float * phi = 0;
    vector<MyMarker> inmarkers;
    for(i = 0; i < p.landmarks.size(); i++)
    {
        double x = p.landmarks[i].x - p.xc0;
        double y = p.landmarks[i].y - p.yc0;
        double z = p.landmarks[i].z - p.zc0;
        
        //add scaling by PHC 121127
        x /= dfactor_xy;
        y /= dfactor_xy;
        z /= dfactor_z;
        
        inmarkers.push_back(MyMarker(x,y,z));
    }
    qint64 etime1 = timer1.elapsed();
    qDebug() << " **** neuron preprocessing takes [" << etime1 << " milliseconds]";
    
    
    for (it=infostring.begin();it!=infostring.end();it++)
        cout << *it <<endl;
    
    v3d_msg("start neuron tracing for the preprocessed image.\n", 0);
    

    //add a timer by PHC 121005
    QElapsedTimer timer2;
    timer2.start();
    
    if(inmarkers.empty())
    {
        cout<<"Start detecting cellbody"<<endl;
        switch(datatype)
        {
            case V3D_UINT8:
                fastmarching_dt_XY(indata1d, phi, in_sz[0], in_sz[1], in_sz[2],p.cnn_type, p.bkg_thresh);
                break;
            case V3D_UINT16:  //this is no longer needed, as the data type has been converted above
                fastmarching_dt_XY((short int*)indata1d, phi, in_sz[0], in_sz[1], in_sz[2],p.cnn_type, p.bkg_thresh);
                break;
        }
        
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
    }


    //crop the area with soma
    inmarkers[0].x = 1033; inmarkers[0].y = 1238; inmarkers[0].z = 25;

    struct root_node *head = new root_node[1];
    struct root_node *walker;
  //  struct root_node *newNode;
    struct root_node *temp;
    MyMarker root;

    head->root_x = inmarkers[0].x;
    head->root_y = inmarkers[0].y;
    head->root_z = inmarkers[0].z;
    head->parent = 0;
    head->next = NULL;
    walker = head;


    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];

    while(walker != NULL)
    {
            V3DLONG ixb = walker->root_x - 512; if(ixb<0) ixb = 0;
            V3DLONG ixe = walker->root_x  + 512; if(ixe>=N-1) ixe = N-1;
            V3DLONG iyb = walker->root_y  - 512; if(iyb<0) iyb = 0;
            V3DLONG iye = walker->root_y  + 512; if(iye>=M-1) iye = M-1;

            V3DLONG blocksz = (ixe-ixb)*(iye-iyb)*P;
            unsigned char *blockld = 0;
            blockld = new unsigned char [blocksz];

            V3DLONG i = 0;
            for(V3DLONG iz = 0; iz < P; iz++)
            {
                V3DLONG offsetk = iz*M*N;
                for(V3DLONG iy = iyb; iy < iye; iy++)
                {
                    V3DLONG offsetj = iy*N;
                    for(V3DLONG ix = ixb; ix < ixe; ix++)
                    {
                        blockld[i] = indata1d[offsetk + offsetj + ix];
                        i++;
                    }
                }
            }

            V3DLONG block_sz[4];
            block_sz[0] = ixe-ixb;
            block_sz[1] = iye-iyb;
            block_sz[2] = P;
            block_sz[3] = 1;

            V3DLONG boundary[4];
            boundary[0] = ixb;
            boundary[1] = ixe;
            boundary[2] = iyb;
            boundary[3] = iye;

            root.x = walker->root_x  - ixb;
            root.y = walker->root_y  - iyb;
            root.z = walker->root_z ;

            printf("root is (%d, %d, %d), parent is %d\n\n\n",walker->root_x,walker->root_y,walker->root_z,walker->parent);
            app2_block(callback,root, blockld, block_sz,indata1d, in_sz,p,dfactor_xy, dfactor_z,boundary,head,walker->parent);
            walker = walker->next;

            if (blockld) {delete blockld; blockld=NULL;}
    }

    temp = head;
    while(temp!=NULL)
    {
       head = head->next;
       free(temp);
       temp = head;
    }

    if (!b_menu)
    {
       if (p.p4dImage) {delete p.p4dImage; p.p4dImage=NULL;}
    }


    return true;
}


bool PARA_APP2::fetch_para_commandline(const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
    vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
    vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
    vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
    vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
    vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
    vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
    
    if(infiles.empty())
    {
        cerr<<"Need input image"<<endl;
        return false;
    }
    
    inimg_file = infiles[0];
    int k=0;
    inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
    //outswc_file = outfiles.empty() ? inimg_file + "_app2.swc" : outfiles[0];
    
    //try to use as much as the default value in the PARA_APP2 constructor as possible
    channel = (paras.size() >= k+1) ? atoi(paras[k]) : channel;  k++;//0;
    bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : bkg_thresh; k++;// 30;
    b_256cube = (paras.size() >= k+1) ? atoi(paras[k]) : b_256cube; k++;// true
    b_RadiusFrom2D = (paras.size() >= k+1) ? atoi(paras[k]) : b_RadiusFrom2D; k++;// true
    is_gsdt = (paras.size() >= k+1) ? atoi(paras[k]) : is_gsdt; k++;// true
    length_thresh = (paras.size() >= k+1) ? atof(paras[k]) : length_thresh; k++;// 1.0;
    //cnn_type = 2; // default connection type 2
    //SR_ratio = 3.0/9.0;
    //is_coverage_prune = true;
    //is_break_accept = false;
    
    return true;
}

