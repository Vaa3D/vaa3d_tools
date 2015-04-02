//last change: by Hanchuan Peng. 2013-01-02
//last change: by Hanchuan Peng, add auto-thres similar to previous APP1 code

#include "vn_app1.h"

#include "vn_imgpreprocess.h"

#include "basic_surf_objs.h"

#include "marker_radius.h"

#include <list>
#include <string>
#include <iostream>

#include "v3dneuron_gd_tracing.h"

#include "fastmarching_dt.h"

#include "volimg_proc.h"


using namespace std;

bool proc_app1(V3DPluginCallback2 &callback, PARA_APP1 &p, const QString & versionStr)
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
        
        QList <ImageMarker> file_inmarkers;
        if(!p.inmarker_file.isEmpty()) 
            file_inmarkers = readMarker_file(p.inmarker_file);
        
        LocationSimple t;
        for(V3DLONG i = 0; i < file_inmarkers.size(); i++)
        {
            t.x = file_inmarkers[i].x;
            t.y = file_inmarkers[i].y;
            t.z = file_inmarkers[i].z;
            p.landmarks.push_back(t);
        }
    }
    
    //these info should also be output to the swc file. need to add later. Noted by PHC, 121124
    QStringList infostring;
    QString tmpstr; QString qtstr;
    tmpstr =  qPrintable( qtstr.prepend("##Vaa3D-Neuron-APP1 ").append(versionStr) ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.channel).prepend("#channel = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.bkg_thresh).prepend("#bkg_thresh = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.visible_thresh).prepend("#visible_thresh = ") ); infostring.push_back(tmpstr);
    double imgAve, imgStd;
    if (p.bkg_thresh < 0)
    {
        if (p.channel >=0 && p.channel <= p.p4dImage->getCDim()-1)
        {
            mean_and_std(p.p4dImage->getRawDataAtChannel(p.channel), p.p4dImage->getTotalUnitNumberPerChannel(), imgAve, imgStd);
            p.bkg_thresh = imgAve; //+0.5*imgStd ; //(imgAve < imgStd)? imgAve : (imgAve+imgStd)*.5;
        }
        else
            p.bkg_thresh = 0;

        tmpstr =  qPrintable( qtstr.setNum(p.bkg_thresh).prepend("autoset #bkg_thresh = ") ); infostring.push_back(tmpstr);
    }
    
    
    tmpstr =  qPrintable( qtstr.setNum(p.downsample_factor).prepend("#downsample_factor = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_256cube).prepend("#b_256cube = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.xc0).prepend("#xc0 = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.xc1).prepend("#xc1 = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.yc0).prepend("#yc0 = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.yc1).prepend("#yc1 = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.zc0).prepend("#zc0 = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.zc1).prepend("#zc1 = ") ); infostring.push_back(tmpstr);
//    list<string>::iterator it;
//    for (it=infostring.begin();it!=infostring.end();it++)
//        cout << *it <<endl;
    for (int tmpj=0;tmpj<infostring.size(); tmpj++)
        cout << qPrintable( infostring.at(tmpj) ) << endl;
    
    v3d_msg("start to preprocessing.\n", 0);
        
    qint64 etime1, etime2;
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
        
        printf("x = %ld, y = %ld, z = %ld, c = %ld\n", in_sz[0], in_sz[1], in_sz[2], in_sz[3]);
        
        if (!p.b_256cube)
        {
            //v3d_msg("p.b_256cube=FALSE!");
            
            dfactor_z = dfactor_xy = p.downsample_factor;
        }
        else 
        {
            //v3d_msg("p.b_256cube=TRUE!");
            
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
        }
        
        printf("dfactor_xy=%5.3f\n", dfactor_xy);
        printf("dfactor_z=%5.3f\n", dfactor_z);

        
        if (0 && (dfactor_z>1 || dfactor_xy>1)) //130220. instead of direct downsampling, try to use the original APP1 method
        {
            V3DLONG out_sz[4];
            unsigned char * outimg=0;
            if (!downsampling_img_xyz( indata1d, in_sz, dfactor_xy, dfactor_z, outimg, out_sz))
            {
                v3d_msg("Down-sampling fails... and quit.");
                return false; //need to clean memory before return. a bug here
            }
            
            p4dImageNew->setData(outimg, out_sz[0], out_sz[1], out_sz[2], out_sz[3], V3D_UINT8);
            
            indata1d = p4dImageNew->getRawDataAtChannel(0);
            in_sz[0] = p4dImageNew->getXDim();
            in_sz[1] = p4dImageNew->getYDim();
            in_sz[2] = p4dImageNew->getZDim();
            in_sz[3] = p4dImageNew->getCDim();
        }
    }
    
    float * phi = 0;
    vector<ImageMarker> inmarkers;
    {
        for(V3DLONG i = 0; i < p.landmarks.size(); i++)
        {
            double x = p.landmarks[i].x - p.xc0;
            double y = p.landmarks[i].y - p.yc0;
            double z = p.landmarks[i].z - p.zc0;

            if (0) //130220. instead of direct downsampling, try to use the original APP1 method
            {
                //add scaling by PHC 121127
                x /= dfactor_xy;
                y /= dfactor_xy;
                z /= dfactor_z;
            }
            
            inmarkers.push_back(ImageMarker(x,y,z));
        }
    }
    etime1 = timer1.elapsed();
    qDebug() << " **** neuron preprocessing takes [" << etime1 << " milliseconds]";
    
    v3d_msg("start neuron tracing for the preprocessed image.\n", 0);
    
    //now invoke APP1

    CurveTracePara trace_para;
    trace_para.channo = p.channel;
    trace_para.sp_graph_resolution_step = dfactor_xy; //no need to downsample further, 2013-02-10. //if change to 1 then make it more accurate, by PHC, 2013-02-09 //ds_step;
    trace_para.b_deformcurve = false; //true; //b_usedshortestpathonly;
    trace_para.b_postMergeClosebyBranches = false; //true; //b_mergeCloseBranches;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = true; //b_postTrim;
    trace_para.b_pruneArtifactBranches = true; //b_pruneArtifactBranches;
    trace_para.sp_graph_background = 1;
    if (p.bkg_thresh==0)
        trace_para.sp_graph_background = 0;
    if (trace_para.sp_graph_background)
    {
        trace_para.imgTH = imgAve;
    }
    else
    {
        trace_para.imgTH = 0;
    }
    trace_para.visible_thresh = p.visible_thresh;    

    LocationSimple p0;
    vector<LocationSimple> pp;
    NeuronTree nt;
 
    unsigned char * img_subject = 0; // must be initialized as NULL
	V3DLONG * sz_subject = 0; // must be initialized as NULL
	int datatype_subject = 0;
	V3DLONG channel_bytes = 0;
	V3DLONG total_units=0;
	V3DLONG total_bytes=0;
	unsigned char *img_new = 0;
    
    unsigned char ****p4d = 0;
    float weight_xy_z = dfactor_xy/dfactor_z; //1. //130220. instead of direct downsampling, try to use the original APP1 method

    V3DLONG *channelsToUse = 0;
    int nChannelsToUse=0;
    if (!channelsToUse) //if not specified the channels, then set the default to be the first channel
	{
		nChannelsToUse=1;
		channelsToUse = new V3DLONG[nChannelsToUse];
		channelsToUse[0] = 0;
	}
	
	for (int i=0;i<nChannelsToUse;i++)
		printf("You really specified channel [%ld] to use.\n", channelsToUse[i]);

    img_subject = p4dImageNew->getRawDataAtChannel(0);
    
    if (nChannelsToUse==1 && channelsToUse[0]==0)
    {
        img_new = img_subject;
    }
    else //this part needs further check/rewrite later. for brainbow tracing
    {
        //fprintf (stderr, "Now only support 8bit data [%s]. Exit. \n", dfile_subject);
        
        total_units =  p4dImageNew->getTotalUnitNumberPerChannel();
        channel_bytes = total_units * p4dImageNew->getUnitBytes();
        total_bytes = nChannelsToUse * total_units;
        
        if (img_new) {delete []img_new; img_new=0;}
        try
        {
            img_new = new unsigned char [total_bytes];
            short int *tmp_si = 0;
            float * tmp_float = 0;
            if (p.p4dImage->getDatatype()==1)
            {
                for (V3DLONG j=0;j<nChannelsToUse;j++)
                {
                    unsigned char *datahead = img_subject + (V3DLONG)channelsToUse[j]*channel_bytes;
                    unsigned char *targethead = img_new + j*total_units;
                    memcpy(targethead, datahead, total_units);
                }
            }
            else if (p.p4dImage->getDatatype()==2)
            {
                for (V3DLONG j=0;j<nChannelsToUse;j++)
                {
                    short int *datahead = (short int *)(img_subject + (V3DLONG)channelsToUse[j]*channel_bytes);
                    unsigned char *targethead = img_new + j*total_units;
                    short int minv, maxv; V3DLONG pos_min, pos_max;
                    minMaxInVector(datahead, total_units, pos_min, minv, pos_max, maxv);
                    if (minv==maxv)
                    {
                        fprintf (stderr, "The data has the min value equals its max value. Thus the neuron tracing won't make sense. Exit. \n");
                        goto Label_exit;
                    }
                    else
                    {
                        double dd=double(255.0)/(maxv-minv);
                        
                        for (V3DLONG i=0;i<total_units;i++)
                            targethead[i] = (datahead[i]-minv)*dd;
                    }
                }
            }
            else // (datatype_subject==4)
            {
                for (V3DLONG j=0;j<nChannelsToUse;j++)
                {
                    float *datahead = (float *)(img_subject + (V3DLONG)channelsToUse[j]*channel_bytes);
                    unsigned char *targethead = img_new + j*total_units;
                    float minv, maxv; V3DLONG pos_min, pos_max;
                    minMaxInVector(datahead, total_units, pos_min, minv, pos_max, maxv);
                    if (minv==maxv)
                    {
                        fprintf (stderr, "The data has the min value equals its max value. Thus the neuron tracing won't make sense. Exit. \n");
                        goto Label_exit;
                    }
                    else
                    {
                        double dd=double(255.0)/(maxv-minv);
                        
                        for (V3DLONG i=0;i<total_units;i++)
                            targethead[i] = (datahead[i]-minv)*dd;
                    }
                }
            }
        }
        catch(...)
        {
            fprintf (stderr, "The data conversion encounters a problem. Exit. \n");
            goto Label_exit;
        }
    }

    //NOW START THE RECONSTRUCTION
    {
        QElapsedTimer timer2;
        timer2.start();

        V3DLONG sz_tracing[4]; sz_tracing[0] = in_sz[0]; sz_tracing[1] = in_sz[1]; sz_tracing[2] = in_sz[2]; sz_tracing[3] = nChannelsToUse;

        //soma detection if necessary
        if(inmarkers.empty())
        {
            cout<<"Start detecting cellbody"<<endl;

            //should always use UINT8 here as indata has been converted to uint8
            fastmarching_dt_XY(indata1d, phi, sz_tracing[0], sz_tracing[1], sz_tracing[2], 2, p.bkg_thresh);

            if (!phi)
            {
                v3d_msg("Fail to run the GSDT code. Exit.", 0);
                goto Label_exit;
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
            ImageMarker max_marker(max_loc % sz0, max_loc % sz01 / sz0, max_loc / sz01);
            inmarkers.push_back(max_marker);
            
            p0.x = max_marker.x;
            p0.y = max_marker.y;
            p0.z = max_marker.z;
        }
        else
        {
            p0.x = inmarkers[0].x;
            p0.y = inmarkers[0].y;
            p0.z = inmarkers[0].z;
            
            LocationSimple t;
            for (V3DLONG i=1; i<inmarkers.size();i++)
            {
                t.x = inmarkers[i].x;
                t.y = inmarkers[i].y;
                t.z = inmarkers[i].z;
                pp.push_back(t);
            }
        }
    
        //tracing

        printf("\n*** data sz for actual tracing [%ld, %ld, %ld, %ld]\n", sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3]);
        
        try {
            new4dpointer(p4d, sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3], img_new);
        } catch (...) {
            fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
            goto Label_exit;
        }
        
        nt = v3dneuron_GD_tracing(p4d, sz_tracing,
                                  p0, pp,
                                  trace_para, weight_xy_z);
        
        //free memory
        if (p4d) delete4dpointer(p4d, sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3]);
        if (p4dImageNew) {delete p4dImageNew; p4dImageNew=0;} //free buffer
        
        etime2 = timer2.elapsed();
        qDebug() << " **** neuron tracing procedure takes [" << etime2 << " milliseconds]";
    }
    
    if (1)
    {
        
        QString rootposstr="", tmps;
        tmps.setNum(int(inmarkers[0].x+0.5)).prepend("_x"); rootposstr += tmps;
        tmps.setNum(int(inmarkers[0].y+0.5)).prepend("_y"); rootposstr += tmps;
        tmps.setNum(int(inmarkers[0].z+0.5)).prepend("_z"); rootposstr += tmps;

        QString outswc_file;
        if(!p.outswc_file.isEmpty())
            outswc_file = p.outswc_file;
        else
            outswc_file = QString(p.p4dImage->getFileName()) + rootposstr + "_app1.swc";

        if (0) //130220. instead of direct downsampling, try to use the original APP1 method
        {
            QList <NeuronSWC> &nn = nt.listNeuron;
            V3DLONG i;
            for(i = 0; i < nn.size(); i++) //add scaling 121127, PHC //add cutbox offset 121202, PHC
            {
                if (dfactor_xy>1) nn[i].x *= dfactor_xy; nn[i].x += (p.xc0); if (dfactor_xy>1) nn[i].x += dfactor_xy/2; //note that the offset corretion might not be accurate. PHC 121127
                if (dfactor_xy>1) nn[i].y *= dfactor_xy; nn[i].y += (p.yc0); if (dfactor_xy>1) nn[i].y += dfactor_xy/2;
                if (dfactor_z>1) nn[i].z *= dfactor_z;  nn[i].z += (p.zc0); if (dfactor_z>1)  nn[i].z += dfactor_z/2;
                nn[i].radius *= dfactor_xy; //use xy for now
            }
        }
        
        //need to add radius re-est code later here.
        
        //if(!writeSWC_file(outswc_file, nt))
        //    fprintf(stderr, "Fail to produce the output file %s.\n", qPrintable(outswc_file));

        tmpstr =  qPrintable( qtstr.setNum(etime1).prepend("#neuron preprocessing time (milliseconds) = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(etime2).prepend("#neuron tracing time (milliseconds) = ") ); infostring.push_back(tmpstr);
        
        //writeSWC_file(outswc_file.toStdString(), nt, infostring);
        if(!writeSWC_file(outswc_file, nt, &infostring))
            fprintf(stderr, "Fail to produce the output file %s.\n", qPrintable(outswc_file));

        v3d_msg(QString("The tracing uses %1 ms (%2 ms for preprocessing and %3 for tracing). Now you can drag and drop the generated swc fle [%4] into Vaa3D."
                        ).arg(etime1+etime2).arg(etime1).arg(etime2).arg(outswc_file), b_menu);
    }
    else //save SWC
    {
        vector<NeuronSWC*> outswc;
        
        QString rootposstr="", tmps;
        tmps.setNum(int(inmarkers[0].x+0.5)).prepend("_x"); rootposstr += tmps;
        tmps.setNum(int(inmarkers[0].y+0.5)).prepend("_y"); rootposstr += tmps;
        tmps.setNum(int(inmarkers[0].z+0.5)).prepend("_z"); rootposstr += tmps;
        //QString outswc_file = callback.getImageName(curwin) + rootposstr + "_app2.swc";
        QString outswc_file = QString(p.p4dImage->getFileName()) + rootposstr + "_app2.swc";
              
        V3DLONG i;
        
        for(i = 0; i < outswc.size(); i++) //add scaling 121127, PHC //add cutbox offset 121202, PHC
        {
            if (dfactor_xy>1) outswc[i]->x *= dfactor_xy; outswc[i]->x += (p.xc0); if (dfactor_xy>1) outswc[i]->x += dfactor_xy/2; //note that the offset corretion might not be accurate. PHC 121127
            if (dfactor_xy>1) outswc[i]->y *= dfactor_xy; outswc[i]->y += (p.yc0); if (dfactor_xy>1) outswc[i]->y += dfactor_xy/2;
            if (dfactor_z>1) outswc[i]->z *= dfactor_z;  outswc[i]->z += (p.zc0); if (dfactor_z>1)  outswc[i]->z += dfactor_z/2;
            outswc[i]->radius *= dfactor_xy; //use xy for now
        }
        
        //re-estimate the radius using the original image
        double real_thres = 40; //PHC 20121011
        if (real_thres<p.bkg_thresh) real_thres = p.bkg_thresh;
        
        V3DLONG szOriginalData[4] = {p.p4dImage->getXDim(), p.p4dImage->getYDim(), p.p4dImage->getZDim(), 1};
        unsigned char * pOriginalData = (unsigned char *)(p.p4dImage->getRawDataAtChannel(p.channel));
        int method_radius_est=2;
        
        switch (p.p4dImage->getDatatype())
        {
            case V3D_UINT8:
            {
                for(i = 0; i < outswc.size(); i++)
                {
                    //printf(" node %ld of %ld.\n", i, outswc.size());
                    outswc[i]->radius = markerRadius(pOriginalData, szOriginalData, *(outswc[i]), real_thres);
                }
            }
                break;
            case V3D_UINT16:
            {
                unsigned short int *pOriginalData_uint16 = (unsigned short int *)pOriginalData;
                for(i = 0; i < outswc.size(); i++)
                {
                    //printf(" node %ld of %ld.\n", i, outswc.size());
                    outswc[i]->radius = markerRadius(pOriginalData_uint16, szOriginalData, *(outswc[i]), real_thres * 16); //*16 as it is often 12 bit data
                }
            }
                break;
            case V3D_FLOAT32:
            {
                float *pOriginalData_float = (float *)pOriginalData;
                for(i = 0; i < outswc.size(); i++)
                {
                    //printf(" node %ld of %ld.\n", i, outswc.size());
                    outswc[i]->radius = markerRadius(pOriginalData_float, szOriginalData, *(outswc[i]), real_thres);
                }
            }
                break;
            default:
                break;
        }
        
        //prepare the output comments for neuron info in the swc file
        
        tmpstr =  qPrintable( qtstr.setNum(etime1).prepend("#neuron preprocessing time (milliseconds) = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(etime2).prepend("#neuron tracing time (milliseconds) = ") ); infostring.push_back(tmpstr);
        
        //saveSWC_file(outswc_file.toStdString(), outswc, infostring);//disable for now, as the save function is different now, 2013-02-10
        
        v3d_msg(QString("The tracing uses %1 ms (%2 ms for preprocessing and %3 for tracing). Now you can drag and drop the generated swc fle [%4] into Vaa3D."
                        ).arg(etime1+etime2).arg(etime1).arg(etime2).arg(outswc_file), b_menu);
        
        if (0) //by PHC 120909
        {
//            try
//            {
//                NeuronTree nt = readSWC_file(outswc_file);
//                callback.setSWC(curwin, nt);
//                callback.open3DWindow(curwin);
//                callback.getView3DControl(curwin)->updateWithTriView();
//            }
//            catch(...)
//            {
//                return false;
//            }
        }
        
    }
    //release memory
    
Label_exit:
    
	//if (img_new && img_new!=img_subject) {delete [] img_new; img_new=0;}
	//if (img_subject) {delete [] img_subject; img_subject=0;}
	if (sz_subject) {delete [] sz_subject; sz_subject=0;}
	
	if (channelsToUse) {delete []channelsToUse; channelsToUse=0;}
    
    if(phi){delete [] phi; phi = 0;}
    
//    for(int i = 0; i < outtree.size(); i++) delete outtree[i];
//    outtree.clear();
    
    if (!b_menu)
    {
        if (p.p4dImage) {delete p.p4dImage; p.p4dImage=NULL;}
    }
    
    return true;
}


bool PARA_APP1::fetch_para_commandline(const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
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
    
    if(!outfiles.empty()) outswc_file = outfiles[0];

    //try to use as much as the default value in the PARA_APP2 constructor as possible
    channel = paras.size() >= k+1 ? atoi(paras[k]) : channel;  k++;//0;
    bkg_thresh = paras.size() >= k+1 ? atoi(paras[k]) : bkg_thresh; if(bkg_thresh == atoi("AUTO")) bkg_thresh = -1;k++;// 30;
    b_256cube = paras.size() >= k+1 ? atoi(paras[k]) : b_256cube; k++;// true
    
    return true;
}

