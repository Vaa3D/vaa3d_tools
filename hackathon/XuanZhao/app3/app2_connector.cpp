//last change: by Hanchuan Peng. 2012-12-30, 2013-06-05

#include "vn_app2.h"
#include "fastmarching_tree.h"
#include "fastmarching_dt.h"
#include "hierarchy_prune.h"
#include "marker_radius.h"
#include "basic_surf_objs.h"
#include "swc_convert.h"
#include "vn_imgpreprocess.h"
#include "volimg_proc.h"

#include "neuron_sim_scores.h"

bool saveSWC_file_app2(string swc_file, vector<MyMarker*> & outmarkers, list<string> & infostring)
{
    if(swc_file.find_last_of(".dot") == swc_file.size() - 1) return saveDot_file(swc_file, outmarkers);

    cout<<"marker num = "<<outmarkers.size()<<", save swc file to "<<swc_file<<endl;
    map<MyMarker*, int> ind;
    ofstream ofs(swc_file.c_str());

    if(ofs.fail())
    {
        cout<<"open swc file error"<<endl;
        return false;
    }
    ofs<<"#name "<<"APP2_Tracing"<<endl;
    ofs<<"#comment "<<endl;

    list<string>::iterator it;
    for (it=infostring.begin();it!=infostring.end(); it++)
        ofs<< *it <<endl;

    ofs<<"##n,type,x,y,z,radius,parent"<<endl;
    for(int i = 0; i < outmarkers.size(); i++) ind[outmarkers[i]] = i+1;

    for(int i = 0; i < outmarkers.size(); i++)
    {
        MyMarker * marker = outmarkers[i];
        int parent_id;
        if(marker->parent == 0) parent_id = -1;
        else parent_id = ind[marker->parent];
        ofs<<i+1<<" "<<marker->type<<" "<<marker->x<<" "<<marker->y<<" "<<marker->z<<" "<<marker->radius<<" "<<parent_id<<endl;
    }
    ofs.close();
    return true;
}

bool proc_app2(V3DPluginCallback2 &callback, PARA_APP2 &p, const QString & versionStr)
{
  //  bool b_menu = true;
    bool b_dofunc = false;
    if (!p.p4dImage || !p.p4dImage->valid())
    {
        if (p.inimg_file.isEmpty()) return false;
        
        b_dofunc = true;
        
        //in this case try to read the image files
        QString infile = p.inimg_file;
        p.p4dImage = callback.loadImage((char *)(qPrintable(infile) ));
        if (!p.p4dImage || !p.p4dImage->valid()) return false;
        else
        {
            p.xc0 = p.yc0 = p.zc0 = 0;
            p.xc1 = p.p4dImage->getXDim()-1;
            p.yc1 = p.p4dImage->getYDim()-1;
            p.zc1 = p.p4dImage->getZDim()-1;
        }
        
        vector<MyMarker> file_inmarkers; 
        if(!p.inmarker_file.isEmpty()) file_inmarkers = readMarker_file(string(qPrintable(p.inmarker_file)));

        LocationSimple t;
        for(int i = 0; i < file_inmarkers.size(); i++)
        {
            t.x = file_inmarkers[i].x;
            t.y = file_inmarkers[i].y;
            t.z = file_inmarkers[i].z;
            if(t.x<p.xc0+1 || t.x>p.xc1+1 || t.y<p.yc0+1 || t.y>p.yc1+1 || t.z<p.zc0+1 || t.z>p.zc1+1)
            {
                if(i==0)
                {
                    v3d_msg("The first marker is invalid.",p.b_menu);
                    return false;
                }
                else continue;
            }
            p.landmarks.push_back(t);
        }
    }

    if(p.landmarks.size() < 2 && p.b_intensity ==1)
    {
       v3d_msg("You have to select at least two markers if using high intensity background option.",p.b_menu);
       return false;
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
    tmpstr =  qPrintable( qtstr.setNum(p.is_break_accept).prepend("#is_gap = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.cnn_type).prepend("#cnn_type = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_256cube).prepend("#b_256cube = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_RadiusFrom2D).prepend("#b_radiusFrom2D = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_resample).prepend("#b_resample = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_intensity).prepend("#b_intensity = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_brightfiled).prepend("#b_brightfiled = ") ); infostring.push_back(tmpstr);
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
        
        if (p.b_brightfiled)
        {
            if(!invertedsubvolumecopy(p4dImageNew,
                                      p.p4dImage,
                                      p.xc0, p.xc1-p.xc0+1,
                                      p.yc0, p.yc1-p.yc0+1,
                                      p.zc0, p.zc1-p.zc0+1,
                                      p.channel, 1))
            return false;
        }
        else
        {
            if(!subvolumecopy(p4dImageNew,
                              p.p4dImage,
                              p.xc0, p.xc1-p.xc0+1,
                              p.yc0, p.yc1-p.yc0+1,
                              p.zc0, p.zc1-p.zc0+1,
                              p.channel, 1))
            return false;
        }
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

    int marker_thresh = INF;
    if(p.b_intensity)
    {
        if(p.b_brightfiled) p.bkg_thresh = 255 - p.bkg_thresh;

        for(int d = 1; d < p.landmarks.size(); d++)
        {
            int marker_x = p.landmarks[d].x - p.xc0;
            int marker_y = p.landmarks[d].y - p.yc0;
            int marker_z = p.landmarks[d].z - p.zc0;

            if(indata1d[marker_z*in_sz[0]*in_sz[1] + marker_y*in_sz[0] + marker_x] < marker_thresh)
            {
                marker_thresh = indata1d[marker_z*in_sz[0]*in_sz[1] + marker_y*in_sz[0] + marker_x];
            }
        }

        p.bkg_thresh = (marker_thresh - 10 > p.bkg_thresh) ? marker_thresh - 10 : p.bkg_thresh;
    }

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
//            p.bkg_thresh = imgAve; //+0.5*imgStd ; //(imgAve < imgStd)? imgAve : (imgAve+imgStd)*.5;
            double td= (imgStd<10)? 10: imgStd;
            p.bkg_thresh = imgAve +0.5*td ; //(imgAve < imgStd)? imgAve : (imgAve+imgStd)*.5; //20170523, PHC
        }
        else
            p.bkg_thresh = 0;
        
        tmpstr =  qPrintable( qtstr.setNum(p.bkg_thresh).prepend("#autoset #bkg_thresh = ") ); infostring.push_back(tmpstr);
    }
    else if (p.b_brightfiled)
    {
        p.bkg_thresh = 255 - p.bkg_thresh;
    }



    float * phi = 0;
    vector<MyMarker> inmarkers;
    for(i = 0; i < p.landmarks.size(); i++)
    {
        double x = p.landmarks[i].x - p.xc0 -1;
        double y = p.landmarks[i].y - p.yc0 -1;
        double z = p.landmarks[i].z - p.zc0 -1;
        
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
    
    vector<MyMarker *> outtree;
    
    //add a timer by PHC 121005
    QElapsedTimer timer2;
    timer2.start();
    
    if(inmarkers.empty())
    {
        cout<<"Start detecting cellbody"<<endl;
		cout << "IMAGE DATATYPE: " << datatype << endl;
        switch(datatype)
        {
            case V3D_UINT8:
                fastmarching_dt_XY(indata1d, phi, in_sz[0], in_sz[1], in_sz[2],p.cnn_type, p.bkg_thresh);
                break;
            case V3D_UINT16:  //this is no longer needed, as the data type has been converted above
                fastmarching_dt_XY((short int*)indata1d, phi, in_sz[0], in_sz[1], in_sz[2],p.cnn_type, p.bkg_thresh);
                break;
        default:
            v3d_msg("Unsupported data type");
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
    
    cout<<"======================================="<<endl;
    cout<<"Construct the neuron tree"<<endl;
    if(inmarkers.empty())
    {
        cerr<<"need at least one markers"<<endl;
    }
    else if(inmarkers.size() == 1)
    {
        cout<<"only one input marker"<<endl;
        if(p.is_gsdt)
        {
            if(phi == 0)
            {
                cout<<"processing fastmarching distance transformation ..."<<endl;
                switch(datatype)
                {
                    case V3D_UINT8:
                        fastmarching_dt(indata1d, phi, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh);
                        break;
                    case V3D_UINT16:  //this is no longer needed, as the data type has been converted above
                        fastmarching_dt((short int *)indata1d, phi, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh);
                        break;
                    default:
                        v3d_msg("Unsupported data type");
                        break;
                }
            }
            
            cout<<endl<<"constructing fastmarching tree ..."<<endl;
            fastmarching_tree(inmarkers[0], phi, outtree, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh, p.is_break_accept);
        }
        else
        {
            switch(datatype)
            {
                case V3D_UINT8:
                    v3d_msg("8bit", 0);
                    fastmarching_tree(inmarkers[0], indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh, p.is_break_accept);
                    break;
                case V3D_UINT16: //this is no longer needed, as the data type has been converted above
                    v3d_msg("16bit", 0);
                    fastmarching_tree(inmarkers[0], (short int*)indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh, p.is_break_accept);
                    break;
                default:
                    v3d_msg("Unsupported data type");
                    break;
            }
        }
    }
    else
    {
        vector<MyMarker> target; target.insert(target.end(), inmarkers.begin()+1, inmarkers.end());
        if(p.is_gsdt)
        {
            if(phi == 0)
            {
                cout<<"processing fastmarching distance transformation ..."<<endl;
                switch(datatype)
                {
                    case V3D_UINT8:
                        fastmarching_dt(indata1d, phi, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh);
                        break;
                    case V3D_UINT16:
                        fastmarching_dt((short int *)indata1d, phi, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh);
                        break;
                }
            }
            cout<<endl<<"constructing fastmarching tree ..."<<endl;
            fastmarching_tree(inmarkers[0], target, phi, outtree, in_sz[0], in_sz[1], in_sz[2], p.cnn_type);
        }
        else
        {
            switch(datatype)
            {
                case V3D_UINT8:
                    fastmarching_tree(inmarkers[0], target, indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], p.cnn_type);
                    break;
                case V3D_UINT16:
                    fastmarching_tree(inmarkers[0], target, (short int*) indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], p.cnn_type);
                    break;
            }
        }
    }
    cout<<"======================================="<<endl;

    //save a copy of the ini tree
    cout<<"Save the initial unprunned tree"<<endl;
    vector<MyMarker*> & inswc = outtree;

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

        saveSWC_file(QString(p.p4dImage->getFileName()).append("_ini.swc").toStdString(), inswc, infostring);

        for (tmpi=0; tmpi<inswc.size(); tmpi++)
        {
            inswc[tmpi]->x = tmpswc[tmpi]->x;
            inswc[tmpi]->y = tmpswc[tmpi]->y;
            inswc[tmpi]->z = tmpswc[tmpi]->z;
        }

        for(tmpi = 0; tmpi < tmpswc.size(); tmpi++)
            delete tmpswc[tmpi];
        tmpswc.clear();
    }


    cout<<"Pruning neuron tree"<<endl;

    vector<MyMarker*> outswc;
    if(p.is_coverage_prune)
    {
        v3d_msg("start to use APP2 program.\n", 0);
        happ(inswc, outswc, indata1d, in_sz[0], in_sz[1], in_sz[2], p.bkg_thresh, p.length_thresh, p.SR_ratio);
    }
    else
    {
        hierarchy_prune2(inswc, outswc, indata1d, in_sz[0], in_sz[1], in_sz[2], p.bkg_thresh, p.length_thresh, true);

        if(1) //get radius
        {
            double real_thres = 40; //PHC 20121011
            if (real_thres<p.bkg_thresh) real_thres = p.bkg_thresh;
            for(i = 0; i < outswc.size(); i++)
            {
                outswc[i]->radius = markerRadius(indata1d, in_sz, *(outswc[i]), real_thres);
            }
        }

        V3DLONG x0,y0,z0;
        V3DLONG wx,wy,wz;
        double pc1,pc2,pc3;
        double* vec1 = new double[3];
        double* vec2 = new double[3];
        double* vec3 = new double[3];

        int pointSize = outswc.size();
        vector<vector<V3DLONG> > children = vector<vector<V3DLONG> >(pointSize,vector<V3DLONG>());
        QStack<int> stack = QStack<int>();
        QHash<MyMarker*,int> hashNeuron = QHash<MyMarker*,int>();
        for(int i=0; i<pointSize; i++){
            hashNeuron.insert(outswc[i],i);
        }

        int* isRemain = new int[pointSize];

        V3DLONG ori;

        for(V3DLONG i=0 ; i<pointSize; i++){
            isRemain[i] = 0;
            if(outswc[i]->parent == 0){
                ori = i;
                stack.push(ori);
                continue;
            }
            V3DLONG prtIndex = hashNeuron.value(outswc[i]->parent);
            children[prtIndex].push_back(i);

            x0 = outswc[i]->x;
            y0 = outswc[i]->y;
            z0 = outswc[i]->z;
            wx = wy = outswc[i]->radius + 4;
            wz = outswc[i]->radius + 2;
            computeCubePcaEigVec(indata1d,in_sz,x0,y0,z0,wx,wy,wz,pc1,pc2,pc3,vec1,vec2,vec3);
    //        computeCylinderPcaEigVec(data1d,sz,p1.x,p1.y,p1.z,p2.x,p2.y,p2.z,wz,pc1,pc2,pc3,vec1,vec2,vec3);
            outswc[i]->radius = pc1/pc2;

        }

        double somaR;

        for(V3DLONG i=0; i<pointSize; i++){
            if(outswc[i]->parent == 0){
                somaR = outswc[i]->radius;
                continue;
            }
            V3DLONG prtIndex = hashNeuron.value(outswc[i]->parent);
            double r1 = outswc[i]->radius;
            double r2 = outswc[prtIndex]->radius;

            if(r2/r1>2.0 || r1<2.0){
                outswc[i]->type = 5;
            }else {
                outswc[i]->type = 7;
            }
        }

        vector<V3DLONG> pointIndexs = vector<V3DLONG>();
        V3DLONG t,tmp;

        while (!stack.empty()) {
            t = stack.pop();
            pointIndexs.push_back(t);
            vector<V3DLONG> child = children[t];
            for(int i=0; i<child.size(); i++){
                tmp = child[i];

                while(children[tmp].size() == 1){
                    pointIndexs.push_back(tmp);
                    V3DLONG ch = children[tmp][0];
                    tmp = ch;
                }

                int chsz = children[tmp].size();
                if(chsz>1){
                    stack.push(tmp);
                }else {
                    pointIndexs.push_back(tmp);
                }

            }
        }

        //pruning bifurcation
        for(V3DLONG i=0; i<pointSize; i++){
            t = pointIndexs[i];
            if(dist(*outswc[t],*outswc[ori]) < somaR*3 || isRemain[t] != 0){
                if(isRemain[t] == 0){
                    isRemain[t] = 1;
                }
            }else if(children[t].size()>1){

                XYZ p1,p2;

                V3DLONG prtIndex = hashNeuron[outswc[t]->parent];

                int count = 0;
                MyMarker* tar = outswc[prtIndex];
                XYZ ptp = XYZ(tar->x,tar->y,tar->z);
                XYZ ptpp = XYZ(0,0,0);
                while (count<15) {
                    if(tar->parent){
                        tar = tar->parent;
                        if(count<5){
                            ptpp.x += tar->x;
                            ptpp.y += tar->y;
                            ptpp.z += tar->z;
                        }
                        count++;
                    }else {
                        break;
                    }
                }
                ptpp.x /= 5.0;
                ptpp.y /= 5.0;
                ptpp.z /= 5.0;
                p1 = ptp - ptpp;

                vector<V3DLONG> child = children[t];
                vector<vector<V3DLONG> > cbs;

                for(int j=0; j<child.size(); j++){
                    vector<V3DLONG> cb = vector<V3DLONG>();
                    tmp = child[j];
                    cb.push_back(tmp);
                    while(children[tmp].size() == 1){
                        tmp = children[tmp][0];
                        cb.push_back(tmp);
                    }
                    cbs.push_back(cb);
                }

                qDebug()<<"cbs size: "<<cbs.size();

                if(cbs.size()>2){
                    int maxIndex1, maxIndex2;
                    double maxD = -1;
                    vector<int> index1 = vector<int>();
                    vector<int> index2 = vector<int>();
                    vector<double> indexD = vector<double>();
                    for(int j=0; j<cbs.size(); j++){
                        if(j != 0){
                            tmp = cbs[j-1][0];
                            ptp = XYZ(0,0,0);
                            ptpp = XYZ(0,0,0);
                            count = 0;
                            while (children[tmp].size() != 0) {
                                tmp = children[tmp][0];
                                if(count<5){
                                    ptp.x += outswc[tmp]->x;
                                    ptp.y += outswc[tmp]->y;
                                    ptp.z += outswc[tmp]->z;
                                }else{
                                    ptpp.x += outswc[tmp]->x;
                                    ptpp.y += outswc[tmp]->y;
                                    ptpp.z += outswc[tmp]->z;
                                }
                                count++;
                                if(count>10){
                                    break;
                                }
                            }
                            if((count-5)>0){
                                ptp.x /= 5.0;
                                ptp.y /= 5.0;
                                ptp.z /= 5.0;

                                ptpp.x /= (double)(count-5);
                                ptpp.y /= (double)(count-5);
                                ptpp.z /= (double)(count-5);

                            }

                            p1 = ptp - ptpp;
                        }
                        for(int k=j; k<cbs.size(); k++){
                            tmp = cbs[k][0];
                            ptp = XYZ(0,0,0);
                            count = 0;
                            while(children[tmp].size() != 0){
                                tmp = children[tmp][0];
                                if(count>=5){
                                    ptp.x += outswc[tmp]->x;
                                    ptp.y += outswc[tmp]->y;
                                    ptp.z += outswc[tmp]->z;
                                }
                                count++;
                                if(count>10){
                                    break;
                                }
                            }
                            if(count-5>0){
                                ptp.x /= (double)(count-5);
                                ptp.y /= (double)(count-5);
                                ptp.z /= (double)(count-5);
                            }

                            p2 = ptp - ptpp;
                            double tmpD = dot(normalize(p1),normalize(p2));

                            qDebug()<<"tow line id: "<<(j-1)<<" "<<k<<" tmpD: "<<tmpD;
                            index1.push_back(j-1);
                            index2.push_back(k);
                            indexD.push_back(tmpD);
                            if(tmpD>maxD){
                                maxD = tmpD;
                                maxIndex1 = j-1;
                                maxIndex2 = k;
                            }
                        }
                    }
                    qDebug()<<"maxD: "<<maxD<<" maxIndex: "<<maxIndex1<<" "<<maxIndex2;
                    isRemain[t] = 1;
                    if(maxIndex1 == -1){
                        for(int j=0; j<indexD.size(); j++){
                            if(index1[j] != maxIndex1 && index1[j] != maxIndex2
                                    && index2[j] != maxIndex1 && index2[j] != maxIndex2){
                                if(indexD[j]>0.9){
                                    for(int k=0; k<cbs.size(); k++){
                                        if(k == maxIndex2){
                                            for(int kk=0; kk<cbs[k].size(); kk++){
                                                isRemain[cbs[k][kk]] = 1;
                                            }
                                        }else {
                                            for(int kk=0; kk<cbs[k].size(); kk++){
                                                isRemain[cbs[k][kk]] = 2;
                                            }
                                            QStack<int> tmpStack = QStack<int>();
                                            if(children[cbs[k].back()].size()>1){
                                                tmpStack.push_back(cbs[k].back());
                                            }
                                            while (!tmpStack.isEmpty()) {
                                                tmp = tmpStack.pop();
                                                isRemain[tmp] = 2;
                                                for(int kk=0; kk<children[tmp].size(); kk++){
                                                    V3DLONG tmp1 = children[tmp][kk];
                                                    while(children[tmp1].size() == 1){
                                                        isRemain[tmp1] = 2;
                                                        tmp1 = children[tmp1][0];
                                                    }
                                                    if(children[tmp1].size() == 0){
                                                        isRemain[tmp1] = 2;
                                                    }else {
                                                        tmpStack.push_back(tmp1);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }else{
                                    for(int k=0; k<cbs.size(); k++){
                                        if(k == maxIndex2){
                                            for(int kk=0; kk<cbs[k].size(); kk++){
                                                isRemain[cbs[k][kk]] = 1;
                                            }
                                        }else {
                                            for(int kk=0; kk<cbs[k].size(); kk++){
                                                isRemain[cbs[k][kk]] = 0;
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                        }

                    }else{
                        for(int j=0; j<cbs.size(); j++){
                            if(j != maxIndex1 && j != maxIndex2){
                                for(int k=0; k<cbs[j].size(); k++){
                                    isRemain[cbs[j][k]] = 1;
                                }
                            }else{
                                for(int k=0; k<cbs[j].size(); k++){
                                    isRemain[cbs[j][k]] = 2;
                                }
                                QStack<int> tmpStack = QStack<int>();
                                if(children[cbs[j].back()].size()>1){
                                    tmpStack.push_back(cbs[j].back());
                                }
                                while (!tmpStack.isEmpty()) {
                                    tmp = tmpStack.pop();
                                    isRemain[tmp] = 2;
                                    for(int k=0; k<children[tmp].size(); k++){
                                        V3DLONG tmp1 = children[tmp][k];
                                        while(children[tmp1].size() == 1){
                                            isRemain[tmp1] = 2;
                                            tmp1 = children[tmp1][0];
                                        }
                                        if(children[tmp1].size() == 0){
                                            isRemain[tmp1] = 2;
                                        }else {
                                            tmpStack.push_back(tmp1);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    continue;
                }else{
                    int minSize = INT_MAX;
                    int minIndex = 0;

                    for(int j=0; j<cbs.size(); j++){
                        if(cbs[j].size()<minSize){
                            minSize = cbs[j].size();
                            minIndex = j;
                        }
                    }
                    if(minSize<10){
                        V3DLONG next_cur = cbs[minIndex].back();
                        qDebug()<<"children nex_cur size: "<<children[next_cur].size();
                        if(children[next_cur].size()>1){
                            child = children[next_cur];
                            for(int j=0; j<child.size(); j++){
                                vector<V3DLONG> cb = vector<V3DLONG>();
                                tmp = child[j];
                                cb.push_back(tmp);
                                while(children[tmp].size() == 1){
                                    tmp = children[tmp][0];
                                    cb.push_back(tmp);
                                }
                                cbs.push_back(cb);
                            }
                            int maxIndex1, maxIndex2;
                            double maxD = -1;
                            vector<int> index1 = vector<int>();
                            vector<int> index2 = vector<int>();
                            vector<double> indexD = vector<double>();

                            for(int j=0; j<cbs.size(); j++){
                                if(j-1 == minIndex)
                                    continue;
                                if(j != 0){
                                    tmp = cbs[j-1][0];
                                    ptp = XYZ(0,0,0);
                                    ptpp = XYZ(0,0,0);
                                    count = 0;
                                    while (children[tmp].size() != 0) {
                                        tmp = children[tmp][0];
                                        if(count<5){
                                            ptp.x += outswc[tmp]->x;
                                            ptp.y += outswc[tmp]->y;
                                            ptp.z += outswc[tmp]->z;
                                        }else{
                                            ptpp.x += outswc[tmp]->x;
                                            ptpp.y += outswc[tmp]->y;
                                            ptpp.z += outswc[tmp]->z;
                                        }
                                        count++;
                                        if(count>10){
                                            break;
                                        }
                                    }
                                    if(count-5>0){
                                        ptp.x /= 5.0;
                                        ptp.y /= 5.0;
                                        ptp.z /= 5.0;

                                        ptpp.x /= (double)(count-5);
                                        ptpp.y /= (double)(count-5);
                                        ptpp.z /= (double)(count-5);

                                    }

                                    p1 = ptp - ptpp;
                                }
                                for(int k=j; k<cbs.size(); k++){
                                    if(k == minIndex)
                                        continue;
                                    tmp = cbs[k][0];
                                    ptp = XYZ(0,0,0);
                                    count = 0;
                                    while(children[tmp].size() != 0){
                                        tmp = children[tmp][0];
                                        if(count>=5){
                                            ptp.x += outswc[tmp]->x;
                                            ptp.y += outswc[tmp]->y;
                                            ptp.z += outswc[tmp]->z;
                                        }
                                        count++;
                                        if(count>10){
                                            break;
                                        }
                                    }
                                    if(count-5>0){
                                        ptp.x /= (double)(count-5);
                                        ptp.y /= (double)(count-5);
                                        ptp.z /= (double)(count-5);
                                    }

                                    p2 = ptp - ptpp;
                                    double tmpD = dot(normalize(p1),normalize(p2));

                                    qDebug()<<"tow line id: "<<(j-1)<<" "<<k<<" tmpD: "<<tmpD;
                                    index1.push_back(j-1);
                                    index2.push_back(k);
                                    indexD.push_back(tmpD);
                                    if(tmpD>maxD){
                                        maxD = tmpD;
                                        maxIndex1 = j-1;
                                        maxIndex2 = k;
                                    }
                                }
                            }
                            qDebug()<<"maxD: "<<maxD<<" maxIndex: "<<maxIndex1<<" "<<maxIndex2;
                            isRemain[t] = 1;
                            if(maxIndex1 == -1){
                                for(int j=0; j<indexD.size(); j++){
                                    if(index1[j] != maxIndex1 && index1[j] != maxIndex2
                                            && index2[j] != maxIndex1 && index2[j] != maxIndex2){
                                        if(indexD[j]>0.9){
                                            for(int k=0; k<cbs.size(); k++){
                                                if(k == maxIndex2 || k == minIndex){
                                                    for(int kk=0; kk<cbs[k].size(); kk++){
                                                        isRemain[cbs[k][kk]] = 1;
                                                    }
                                                }else {
                                                    for(int kk=0; kk<cbs[k].size(); kk++){
                                                        isRemain[cbs[k][kk]] = 2;
                                                    }
                                                    QStack<int> tmpStack = QStack<int>();
                                                    if(children[cbs[k].back()].size()>1){
                                                        tmpStack.push_back(cbs[k].back());
                                                    }
                                                    while (!tmpStack.isEmpty()) {
                                                        tmp = tmpStack.pop();
                                                        isRemain[tmp] = 2;
                                                        for(int kk=0; kk<children[tmp].size(); kk++){
                                                            V3DLONG tmp1 = children[tmp][kk];
                                                            while(children[tmp1].size() == 1){
                                                                isRemain[tmp1] = 2;
                                                                tmp1 = children[tmp1][0];
                                                            }
                                                            if(children[tmp1].size() == 0){
                                                                isRemain[tmp1] = 2;
                                                            }else {
                                                                tmpStack.push_back(tmp1);
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }else{
                                            for(int k=0; k<cbs.size(); k++){
                                                if(k == maxIndex2 || k == minIndex){
                                                    for(int kk=0; kk<cbs[k].size(); kk++){
                                                        isRemain[cbs[k][kk]] = 1;
                                                    }
                                                }else {
                                                    for(int kk=0; kk<cbs[k].size(); kk++){
                                                        isRemain[cbs[k][kk]] = 0;
                                                    }
                                                }
                                            }
                                        }
                                        break;
                                    }
                                }
                            }else{
                                for(int j=0; j<cbs.size(); j++){
                                    if(j != maxIndex1 && j != maxIndex2){
                                        for(int k=0; k<cbs[j].size(); k++){
                                            isRemain[cbs[j][k]] = 1;
                                        }
                                    }else{
                                        for(int k=0; k<cbs[j].size(); k++){
                                            isRemain[cbs[j][k]] = 2;
                                        }
                                        QStack<int> tmpStack = QStack<int>();
                                        if(children[cbs[j].back()].size()>1){
                                            tmpStack.push_back(cbs[j].back());
                                        }
                                        while (!tmpStack.isEmpty()) {
                                            tmp = tmpStack.pop();
                                            qDebug()<<"tmp: "<<tmp;
                                            isRemain[tmp] = 2;
                                            for(int k=0; k<children[tmp].size(); k++){
                                                V3DLONG tmp1 = children[tmp][k];
                                                while(children[tmp1].size() == 1){
                                                    isRemain[tmp1] = 2;
                                                    tmp1 = children[tmp1][0];
                                                }
                                                if(children[tmp1].size() == 0){
                                                    isRemain[tmp1] = 2;
                                                }else {
                                                    tmpStack.push_back(tmp1);
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            continue;
                        }

                    }
                }

            }
        }


        //pruning unbifurcation
        for(V3DLONG i=0; i<pointIndexs.size(); i++){
             t = pointIndexs[i];

             if(isRemain[t] != 0 || outswc[t]->type != 5 || children[t].size()>1){
                 if(isRemain[t] == 0){
                     isRemain[t] = 1;
                 }
             }else if (outswc[t]->type == 5) {
                 qDebug()<<"in line";
                 vector<MyMarker> target;
                 MyMarker* s;
                 V3DLONG prtIndex = hashNeuron[outswc[t]->parent];
                 vector<vector<V3DLONG> > cbs;

                 XYZ p1,p2;

                 int count = 0;
                 MyMarker* tar = outswc[prtIndex];
                 XYZ ptp = XYZ(tar->x,tar->y,tar->z);
                 XYZ ptpp = XYZ(0,0,0);
                 while (count<15) {
                     if(tar->parent){
                         tar = tar->parent;
                         if(count<5){
                             ptpp.x += ptpp.x;
                             ptpp.y += ptpp.y;
                             ptpp.z += ptpp.z;
                         }
                         count++;
                     }else {
                         break;
                     }
                 }
                 ptpp.x /= 5.0;
                 ptpp.y /= 5.0;
                 ptpp.z /= 5.0;
                 p1 = ptp - ptpp;
                 target.push_back(*tar);
                 qDebug()<<"tar r: "<<tar->radius;

                 ptp = XYZ(0,0,0);
                 count = 0;
                 tmp = t;
                 while(children[tmp].size() == 1){
                     ptp.x += outswc[tmp]->x;
                     ptp.y += outswc[tmp]->y;
                     ptp.z += outswc[tmp]->z;
                     count++;
                     if(count>5){
                         break;
                     }
                 }
                 ptp.x /= (double)count;
                 ptp.y /= (double)count;
                 ptp.z /= (double)count;

                 p2 = ptp - XYZ(outswc[t]->x,outswc[t]->y,outswc[t]->z);

                 if(dot(normalize(p1),normalize(p2)) > sqrt(3.0)/2.0){
                     isRemain[t] = 1;
                     continue;
                 }

                 vector<V3DLONG> cb0 = vector<V3DLONG>();
                 tmp = t;
                 cb0.push_back(tmp);
                 while (children[tmp].size() == 1) {
                     tmp = children[tmp][0];
                     cb0.push_back(tmp);
                 }
                 qDebug()<<"cb0 size: "<<cb0.size();
                 if(cb0.size() == 1){
                     isRemain[t] = 1;
                     continue;
                 }

                 if(cb0.size()<10){

                     for(int j=0; j<cb0.size(); j++){
                         isRemain[cb0[j]] = 1;
                     }

                     V3DLONG next_cur = cb0.back();
                     vector<V3DLONG> child = children[next_cur];
                     qDebug()<<"child size:  "<<child.size();
                     if(child.size() == 0){
                         continue;
                     }
                     for(int j=0; j<child.size(); j++){
                         tmp = child[j];
                         if(isRemain[tmp] != 0){
                             continue;
                         }
                         vector<V3DLONG> cb = vector<V3DLONG>();
                         cb.push_back(tmp);
                         while(children[tmp].size() == 1){
                             tmp = children[tmp][0];
                             cb.push_back(tmp);
                         }
                         s = outswc[cb[cb.size()/2]];
                         p2 = XYZ(s->x - s->parent->x,
                                  s->y - s->parent->y,
                                  s->z - s->parent->z);

                         V3DLONG tmpi;
                         V3DLONG tmpOri;
                         vector<MyMarker*> tmpTree;
                         vector<MyMarker*> tmpOutTree;
                         qDebug()<<"-------------start--------------";
                         fastmarching_tree(*s,target,indata1d,tmpTree,in_sz[0],in_sz[1],in_sz[2],p.cnn_type);
                         qDebug()<<"-------------start_pruning-------------";
                         hierarchy_prune2(tmpTree,tmpOutTree,indata1d,in_sz[0], in_sz[1], in_sz[2], p.bkg_thresh, p.length_thresh,false);
                         qDebug()<<"--------------pruning_end-----------";

                         vector<MyMarker*> tmpOutTree2;
                         vector<vector<V3DLONG> > tmpChildren = vector<vector<V3DLONG> >(tmpOutTree.size(),vector<V3DLONG>());

                         qDebug()<<"tmpOutTree size: "<<tmpOutTree.size();
                         if(tmpOutTree.size() == 0){
                             for(tmpi=0; tmpi<cb.size(); tmpi++){
                                 isRemain[cb[tmpi]] = 1;
                             }
                             continue;
                         }

                         QHash<MyMarker*,int> tmpHashNeuron = QHash<MyMarker*,int>();
                         for(tmpi=0; tmpi<tmpOutTree.size(); tmpi++){
                             tmpHashNeuron.insert(tmpOutTree[tmpi],tmpi);
                         }

                         for(tmpi=0; tmpi<tmpOutTree.size(); tmpi++){
                             if(tmpOutTree[tmpi]->parent == 0){
                                 tmpOri = tmpi;
                                 continue;
                             }else {
                                 V3DLONG tmpPrtIdx = tmpHashNeuron.value(tmpOutTree[tmpi]->parent);
                                 tmpChildren[tmpPrtIdx].push_back(tmpi);
                             }
                         }
                         vector<V3DLONG> tmpChild = tmpChildren[tmpOri];

                         qDebug()<<"tmpOri"<<tmpOri;

                         qDebug()<<"tmpChild size: "<<tmpChild.size();
                         if(tmpChild.size() == 0){
                             for(tmpi=0; tmpi<cb.size(); tmpi++){
                                 isRemain[cb[tmpi]] = 1;
                             }
                             continue;
                         }
                         double maxD = -1;
                         double maxIndex = 0;
                         for(tmpi=0; tmpi<tmpChild.size(); tmpi++){
                             V3DLONG tmpt = tmpChild[tmpi];
                             XYZ tmpP = XYZ(tmpOutTree[tmpOri]->x - tmpOutTree[tmpt]->x,
                                            tmpOutTree[tmpOri]->y - tmpOutTree[tmpt]->y,
                                            tmpOutTree[tmpOri]->z - tmpOutTree[tmpt]->z);
                             double tmpD = dot(normalize(tmpP),normalize(p2));
                             if(tmpD>maxD){
                                 maxD = tmpD;
                                 maxIndex = tmpi;
                             }
                         }
                         tmpOutTree2.push_back(tmpOutTree[tmpOri]);
                         V3DLONG tmpT = tmpChild[maxIndex];
                         while (tmpChildren[tmpT].size() == 1) {
                             tmpOutTree2.push_back(tmpOutTree[tmpT]);
                             tmpT = tmpChildren[tmpT][0];
                         }
                         int tmpBranchCount = 1;
                         if(tmpChildren[tmpT].size() == 0){
                             tmpOutTree2.push_back(tmpOutTree[tmpT]);
                         }else {
                             QStack<V3DLONG> tmpStack;
                             tmpStack.push_back(tmpT);
                             while(!tmpStack.isEmpty()){
                                 tmpT = tmpStack.pop();
                                 tmpOutTree2.push_back(tmpOutTree[tmpT]);
                                 for(tmpi=0; tmpi<tmpChildren[tmpT].size(); tmpi++){
                                     tmpBranchCount++;
                                     V3DLONG tmpt = tmpChildren[tmpT][tmpi];
                                     while (tmpChildren[tmpt].size() == 1) {
                                         tmpOutTree2.push_back(tmpOutTree[tmpt]);
                                         tmpt = tmpChildren[tmpt][0];
                                     }
                                     if(tmpChildren[tmpt].size() == 0){
                                         tmpOutTree2.push_back(tmpOutTree[tmpt]);
                                     }else {
                                         tmpStack.push(tmpt);
                                     }
                                 }
                             }
                         }
                         bool targetExist = false;
                         for(tmpi=0; tmpi<tmpOutTree2.size(); tmpi++){
                             if(tmpOutTree2[tmpi]->x == target[0].x
                                     && tmpOutTree2[tmpi]->y == target[0].y
                                     && tmpOutTree2[tmpi]->z == target[0].z){
                                 targetExist = true;
                                 break;
                             }
                         }
                         bool otherExist = false;
                         for(tmpi=0; tmpi<tmpOutTree2.size(); tmpi++){
                             double minD = INT_MAX;
                             MyMarker* tmpCur = tmpOutTree2[tmpi];
                             for(int tmpj=0; tmpj<outswc.size(); tmpj++){
                                 MyMarker* tmpPar = outswc[tmpj]->parent;
                                 if(tmpPar == 0)
                                     continue;
                                 MyMarker* tmpChi = outswc[tmpj];
                                 double tmpD = dist_pt_to_line_seg(XYZ(tmpCur->x,tmpCur->y,tmpCur->z),
                                                                   XYZ(tmpChi->x,tmpChi->y,tmpChi->z),
                                                                   XYZ(tmpPar->x,tmpPar->y,tmpPar->z));
                                 if(tmpD<minD)
                                     minD = tmpD;
                             }
                             if(minD>5){
                                 otherExist = true;
                                 break;
                             }
                         }
                         if(targetExist && !otherExist){
                             for(tmpi=0; tmpi<cb.size(); tmpi++){
                                 isRemain[cb[tmpi]] = 1;
                             }
                         }else {
                             for(tmpi=0; tmpi<cb.size(); tmpi++){
                                 isRemain[cb[tmpi]] = 2;
                             }

                             QStack<int> tmpStack = QStack<int>();
                             if(children[cb.back()].size()>1){
                                 tmpStack.push_back(cb.back());
                             }
                             while (!tmpStack.isEmpty()) {
                                 tmp = tmpStack.pop();
                                 qDebug()<<"tmp: "<<tmp;
                                 isRemain[tmp] = 2;
                                 for(int k=0; k<children[tmp].size(); k++){
                                     V3DLONG tmp1 = children[tmp][k];
                                     while(children[tmp1].size() == 1){
                                         isRemain[tmp1] = 2;
                                         tmp1 = children[tmp1][0];
                                     }
                                     if(children[tmp1].size() == 0){
                                         isRemain[tmp1] = 2;
                                     }else {
                                         tmpStack.push_back(tmp1);
                                     }
                                 }
                             }

                         }
                         for (tmpi=0; tmpi<tmpOutTree2.size(); tmpi++)
                         {
                             if (dfactor_xy>1) tmpOutTree2[tmpi]->x *= dfactor_xy;
                             tmpOutTree2[tmpi]->x += (p.xc0);
                             if (dfactor_xy>1) tmpOutTree2[tmpi]->x += dfactor_xy/2;

                             if (dfactor_xy>1) tmpOutTree2[tmpi]->y *= dfactor_xy;
                             tmpOutTree2[tmpi]->y += (p.yc0);
                             if (dfactor_xy>1) tmpOutTree2[tmpi]->y += dfactor_xy/2;

                             if (dfactor_z>1) tmpOutTree2[tmpi]->z *= dfactor_z;
                             tmpOutTree2[tmpi]->z += (p.zc0);
                             if (dfactor_z>1)  tmpOutTree2[tmpi]->z += dfactor_z/2;

                             tmpOutTree2[tmpi]->type = 6;
                         }

                         saveSWC_file(QString(p.p4dImage->getFileName()).append(QString::number(t)).append("_ini.swc").toStdString(), tmpOutTree2, infostring);

                         qDebug()<<"tmpTree size: "<<tmpTree.size();
                         for(tmpi = 0; tmpi<tmpTree.size(); tmpi++){
                             delete tmpTree[tmpi];
                         }
                         tmpTree.clear();
                         qDebug()<<"---------delete tmpTree----------";

                     }
                 }else {
                     s = outswc[cb0[cb0.size()/2]];
                     p2 = XYZ(s->x - s->parent->x,
                              s->y - s->parent->y,
                              s->z - s->parent->z);

                     V3DLONG tmpi;
                     V3DLONG tmpOri;
                     vector<MyMarker*> tmpTree;
                     vector<MyMarker*> tmpOutTree;
                     qDebug()<<"-------------start--------------";
                     fastmarching_tree(*s,target,indata1d,tmpTree,in_sz[0],in_sz[1],in_sz[2],p.cnn_type);
                     qDebug()<<"-------------start_pruning-------------";
                     hierarchy_prune2(tmpTree,tmpOutTree,indata1d,in_sz[0], in_sz[1], in_sz[2], p.bkg_thresh, p.length_thresh,false);
                     qDebug()<<"--------------pruning_end-----------";

                     vector<MyMarker*> tmpOutTree2;
                     vector<vector<V3DLONG> > tmpChildren = vector<vector<V3DLONG> >(tmpOutTree.size(),vector<V3DLONG>());

                     qDebug()<<"tmpOutTree size: "<<tmpOutTree.size();
                     if(tmpOutTree.size() == 0){
                         for(tmpi=0; tmpi<cb0.size(); tmpi++){
                             isRemain[cb0[tmpi]] = 1;
                         }
                         continue;
                     }

                     QHash<MyMarker*,int> tmpHashNeuron = QHash<MyMarker*,int>();
                     for(tmpi=0; tmpi<tmpOutTree.size(); tmpi++){
                         tmpHashNeuron.insert(tmpOutTree[tmpi],tmpi);
                     }

                     for(tmpi=0; tmpi<tmpOutTree.size(); tmpi++){
                         if(tmpOutTree[tmpi]->parent == 0){
                             tmpOri = tmpi;
                             continue;
                         }else {
                             V3DLONG tmpPrtIdx = tmpHashNeuron.value(tmpOutTree[tmpi]->parent);
                             tmpChildren[tmpPrtIdx].push_back(tmpi);
                         }
                     }
                     vector<V3DLONG> tmpChild = tmpChildren[tmpOri];

                     qDebug()<<"tmpOri"<<tmpOri;

                     qDebug()<<"tmpChild size: "<<tmpChild.size();
                     if(tmpChild.size() == 0){
                         for(tmpi=0; tmpi<cb0.size(); tmpi++){
                             isRemain[cb0[tmpi]] = 1;
                         }
                         continue;
                     }
                     double maxD = -1;
                     double maxIndex = 0;
                     for(tmpi=0; tmpi<tmpChild.size(); tmpi++){
                         V3DLONG tmpt = tmpChild[tmpi];
                         XYZ tmpP = XYZ(tmpOutTree[tmpOri]->x - tmpOutTree[tmpt]->x,
                                        tmpOutTree[tmpOri]->y - tmpOutTree[tmpt]->y,
                                        tmpOutTree[tmpOri]->z - tmpOutTree[tmpt]->z);
                         double tmpD = dot(normalize(tmpP),normalize(p2));
                         if(tmpD>maxD){
                             maxD = tmpD;
                             maxIndex = tmpi;
                         }
                     }
                     tmpOutTree2.push_back(tmpOutTree[tmpOri]);
                     V3DLONG tmpT = tmpChild[maxIndex];
                     while (tmpChildren[tmpT].size() == 1) {
                         tmpOutTree2.push_back(tmpOutTree[tmpT]);
                         tmpT = tmpChildren[tmpT][0];
                     }
                     int tmpBranchCount = 1;
                     if(tmpChildren[tmpT].size() == 0){
                         tmpOutTree2.push_back(tmpOutTree[tmpT]);
                     }else {
                         QStack<V3DLONG> tmpStack;
                         tmpStack.push_back(tmpT);
                         while(!tmpStack.isEmpty()){
                             tmpT = tmpStack.pop();
                             tmpOutTree2.push_back(tmpOutTree[tmpT]);
                             for(tmpi=0; tmpi<tmpChildren[tmpT].size(); tmpi++){
                                 tmpBranchCount++;
                                 V3DLONG tmpt = tmpChildren[tmpT][tmpi];
                                 while (tmpChildren[tmpt].size() == 1) {
                                     tmpOutTree2.push_back(tmpOutTree[tmpt]);
                                     tmpt = tmpChildren[tmpt][0];
                                 }
                                 if(tmpChildren[tmpt].size() == 0){
                                     tmpOutTree2.push_back(tmpOutTree[tmpt]);
                                 }else {
                                     tmpStack.push(tmpt);
                                 }
                             }
                         }
                     }
                     bool targetExist = false;
                     for(tmpi=0; tmpi<tmpOutTree2.size(); tmpi++){
                         if(tmpOutTree2[tmpi]->x == target[0].x
                                 && tmpOutTree2[tmpi]->y == target[0].y
                                 && tmpOutTree2[tmpi]->z == target[0].z){
                             targetExist = true;
                             break;
                         }
                     }
                     bool otherExist = false;
                     for(tmpi=0; tmpi<tmpOutTree2.size(); tmpi++){
                         double minD = INT_MAX;
                         MyMarker* tmpCur = tmpOutTree2[tmpi];
                         for(int tmpj=0; tmpj<outswc.size(); tmpj++){
                             MyMarker* tmpPar = outswc[tmpj]->parent;
                             if(tmpPar == 0)
                                 continue;
                             MyMarker* tmpChi = outswc[tmpj];
                             double tmpD = dist_pt_to_line_seg(XYZ(tmpCur->x,tmpCur->y,tmpCur->z),
                                                               XYZ(tmpChi->x,tmpChi->y,tmpChi->z),
                                                               XYZ(tmpPar->x,tmpPar->y,tmpPar->z));
                             if(tmpD<minD)
                                 minD = tmpD;
                         }
                         if(minD>5){
                             otherExist = true;
                             break;
                         }
                     }
                     if(targetExist && !otherExist){
                         for(tmpi=0; tmpi<cb0.size(); tmpi++){
                             isRemain[cb0[tmpi]] = 1;
                         }
                     }else {
                         for(tmpi=0; tmpi<cb0.size(); tmpi++){
                             isRemain[cb0[tmpi]] = 2;
                         }

                         QStack<int> tmpStack = QStack<int>();
                         if(children[cb0.back()].size()>1){
                             tmpStack.push_back(cb0.back());
                         }
                         while (!tmpStack.isEmpty()) {
                             tmp = tmpStack.pop();
                             qDebug()<<"tmp: "<<tmp;
                             isRemain[tmp] = 2;
                             for(int k=0; k<children[tmp].size(); k++){
                                 V3DLONG tmp1 = children[tmp][k];
                                 while(children[tmp1].size() == 1){
                                     isRemain[tmp1] = 2;
                                     tmp1 = children[tmp1][0];
                                 }
                                 if(children[tmp1].size() == 0){
                                     isRemain[tmp1] = 2;
                                 }else {
                                     tmpStack.push_back(tmp1);
                                 }
                             }
                         }

                     }

                     for (tmpi=0; tmpi<tmpOutTree2.size(); tmpi++)
                     {
                         if (dfactor_xy>1) tmpOutTree2[tmpi]->x *= dfactor_xy;
                         tmpOutTree2[tmpi]->x += (p.xc0);
                         if (dfactor_xy>1) tmpOutTree2[tmpi]->x += dfactor_xy/2;

                         if (dfactor_xy>1) tmpOutTree2[tmpi]->y *= dfactor_xy;
                         tmpOutTree2[tmpi]->y += (p.yc0);
                         if (dfactor_xy>1) tmpOutTree2[tmpi]->y += dfactor_xy/2;

                         if (dfactor_z>1) tmpOutTree2[tmpi]->z *= dfactor_z;
                         tmpOutTree2[tmpi]->z += (p.zc0);
                         if (dfactor_z>1)  tmpOutTree2[tmpi]->z += dfactor_z/2;

                         tmpOutTree2[tmpi]->type = 6;
                     }

                     saveSWC_file(QString(p.p4dImage->getFileName()).append(QString::number(t)).append("_ini.swc").toStdString(), tmpOutTree2, infostring);

                     qDebug()<<"tmpTree size: "<<tmpTree.size();
                     for(tmpi = 0; tmpi<tmpTree.size(); tmpi++){
                         delete tmpTree[tmpi];
                     }
                     tmpTree.clear();
                     qDebug()<<"---------delete tmpTree----------";
                 }


             }
        }


        for(int i=0; i<pointSize; i++){
            if(isRemain[i] == 2){
                outswc[i]->type = 2;
            }else{
                outswc[i]->type = 7;
            }
        }

        if(vec1){
            delete[] vec1;
            vec1 = 0;
        }
        if(vec2){
            delete[] vec2;
            vec2 = 0;
        }
        if(vec3){
            delete[] vec3;
            vec3 = 0;
        }
        if(isRemain){
            delete[] isRemain;
            isRemain = 0;
        }

    }
    
    qint64 etime2 = timer2.elapsed();
    qDebug() << " **** neuron tracing procedure takes [" << etime2 << " milliseconds]";
    
    if (p4dImageNew) {delete p4dImageNew; p4dImageNew=0;} //free buffer

    if(p.b_256cube)
    {
        inmarkers[0].x *= dfactor_xy;
        inmarkers[0].y *= dfactor_xy;
        inmarkers[0].z *= dfactor_z;

    }
    
    if(1)
    {
        QString rootposstr="", tmps;
        tmps.setNum(int(inmarkers[0].x+0.5)).prepend("_x"); rootposstr += tmps;
        tmps.setNum(int(inmarkers[0].y+0.5)).prepend("_y"); rootposstr += tmps;
        tmps.setNum(int(inmarkers[0].z+0.5)).prepend("_z"); rootposstr += tmps;
        //QString outswc_file = callback.getImageName(curwin) + rootposstr + "_app2.swc";
        QString outswc_file;
        if(!p.outswc_file.isEmpty())
            outswc_file = p.outswc_file;
        else
            outswc_file = QString(p.p4dImage->getFileName()) + rootposstr + "_app2.swc";
              
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
        double real_thres = 40; //PHC 20121011 //This should be rescaled later for datatypes that are not UINT8

        if (real_thres<p.bkg_thresh) real_thres = p.bkg_thresh;
        V3DLONG szOriginalData[4] = {p.p4dImage->getXDim(), p.p4dImage->getYDim(), p.p4dImage->getZDim(), 1};
        unsigned char * pOriginalData = (unsigned char *)(p.p4dImage->getRawDataAtChannel(p.channel));
        if(p.b_brightfiled)
        {
            for(V3DLONG i = 0; i < p.p4dImage->getTotalUnitNumberPerChannel(); i++)
                pOriginalData[i] = 255 - pOriginalData[i];

        }

        int method_radius_est = ( p.b_RadiusFrom2D ) ? 1 : 2;
        
        switch (p.p4dImage->getDatatype())
        {
            case V3D_UINT8:
            {
                for(i = 0; i < outswc.size(); i++)
                {
                    //printf(" node %ld of %ld.\n", i, outswc.size());
                    outswc[i]->radius = markerRadius(pOriginalData, szOriginalData, *(outswc[i]), real_thres, method_radius_est);
                }
            }
                break;
            case V3D_UINT16:
            {
                unsigned short int *pOriginalData_uint16 = (unsigned short int *)pOriginalData;
                for(i = 0; i < outswc.size(); i++)
                {
                    //printf(" node %ld of %ld.\n", i, outswc.size());
                    outswc[i]->radius = markerRadius(pOriginalData_uint16, szOriginalData, *(outswc[i]), real_thres * 16, method_radius_est); //*16 as it is often 12 bit data
                }
            }
                break;
            case V3D_FLOAT32:
            {
                float *pOriginalData_float = (float *)pOriginalData;
                for(i = 0; i < outswc.size(); i++)
                {
                    //printf(" node %ld of %ld.\n", i, outswc.size());
                    outswc[i]->radius = markerRadius(pOriginalData_float, szOriginalData, *(outswc[i]), real_thres, method_radius_est);
                }
            }
                break;
            default:
                break;
        }
        
        if(p.b_brightfiled)
        {
            for(V3DLONG i = 0; i < p.p4dImage->getTotalUnitNumberPerChannel(); i++)
                pOriginalData[i] = 255 - pOriginalData[i];

        }
        //prepare the output comments for neuron info in the swc file
       
        tmpstr =  qPrintable( qtstr.setNum(etime1).prepend("#neuron preprocessing time (milliseconds) = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(etime2).prepend("#neuron tracing time (milliseconds) = ") ); infostring.push_back(tmpstr);
        saveSWC_file(outswc_file.toStdString(), outswc, infostring);

        if(outswc.size()>1)
        {

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
            if(p.b_resample)
                callback.callPluginFunc(full_plugin_name_resample,func_name_resample,input_resample,output);
            arg.type = "random";std::vector<char*> arg_input_sort;
            arg_input_sort.push_back(fileName_string);
            arg.p = (void *) & arg_input_sort; input_sort<< arg;
            arg.type = "random";std::vector<char*> arg_sort_para; arg_sort_para.push_back("0");arg.p = (void *) & arg_sort_para; input_sort << arg;
            QString full_plugin_name_sort = "sort_neuron_swc";
            QString func_name_sort = "sort_swc";
            callback.callPluginFunc(full_plugin_name_sort,func_name_sort, input_sort,output);
            
            vector<MyMarker*> temp_out_swc = readSWC_file(outswc_file.toStdString());
            saveSWC_file_app2(outswc_file.toStdString(), temp_out_swc, infostring);
        }
        //v3d_msg(QString("The tracing uses %1 ms (%2 ms for preprocessing and %3 for tracing). Now you can drag and drop the generated swc fle [%4] into Vaa3D."
        //                ).arg(etime1+etime2).arg(etime1).arg(etime2).arg(outswc_file), p.b_menu);
        
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
    else
    {
//        NeuronTree nt = swc_convert(outswc);
//        callback.setSWC(curwin, nt);
//        callback.open3DWindow(curwin);
//        callback.getView3DControl(curwin)->updateWithTriView();
    }
    //release memory
    if(phi){delete [] phi; phi = 0;}
    for(V3DLONG i = 0; i < outtree.size(); i++) delete outtree[i];
    outtree.clear();
    
    if (b_dofunc)
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
  //  outswc_file = outfiles.empty() ? inimg_file + "_app2.swc" : outfiles[0];
    if(!outfiles.empty()) outswc_file = outfiles[0];
    //try to use as much as the default value in the PARA_APP2 constructor as possible
    channel = (paras.size() >= k+1) ? atoi(paras[k]) : channel;  k++;//0;
    bkg_thresh = (paras.size() >= k+1) ? atoi(paras[k]) : bkg_thresh; if(bkg_thresh == atoi("AUTO")) bkg_thresh = -1;k++;// 30;
    birfucation_thres = (paras.size() >= k+1) ? atoi(paras[k]) : birfucation_thres; k++; //30
    is_coverage_prune = (paras.size() >= k+1) ? atoi(paras[k]) :is_coverage_prune; k++;// added by xz
    b_256cube = (paras.size() >= k+1) ? atoi(paras[k]) : b_256cube; k++;// true
    b_RadiusFrom2D = (paras.size() >= k+1) ? atoi(paras[k]) : b_RadiusFrom2D; k++;// true
    is_gsdt = (paras.size() >= k+1) ? atoi(paras[k]) : is_gsdt; k++;// true
    is_break_accept = (paras.size() >= k+1) ? atoi(paras[k]) : is_break_accept; k++;// true
    length_thresh = (paras.size() >= k+1) ? atof(paras[k]) : length_thresh; k++;// 1.0;
    b_resample = (paras.size() >= k+1) ? atoi(paras[k]) : b_resample; k++;// 1.0;
    b_brightfiled = (paras.size() >= k+1) ? atoi(paras[k]) : b_brightfiled; k++;// 0.0;
    b_intensity = (paras.size() >= k+1) ? atoi(paras[k]) : b_intensity; k++;// 0.0;

    b_menu = false;

    //cnn_type = 2; // default connection type 2
    //SR_ratio = 3.0/9.0;
    //is_coverage_prune = true;
    //is_break_accept = false;
    
    return true;
}

bool proc_app3(V3DPluginCallback2 &callback, PARA_APP2 &p, const QString & versionStr)
{
  //  bool b_menu = true;
    bool b_dofunc = false;
    if (!p.p4dImage || !p.p4dImage->valid())
    {
        if (p.inimg_file.isEmpty()) return false;

        b_dofunc = true;

        //in this case try to read the image files
        QString infile = p.inimg_file;
        p.p4dImage = callback.loadImage((char *)(qPrintable(infile) ));
        if (!p.p4dImage || !p.p4dImage->valid()) return false;
        else
        {
            p.xc0 = p.yc0 = p.zc0 = 0;
            p.xc1 = p.p4dImage->getXDim()-1;
            p.yc1 = p.p4dImage->getYDim()-1;
            p.zc1 = p.p4dImage->getZDim()-1;
        }

        vector<MyMarker> file_inmarkers;
        if(!p.inmarker_file.isEmpty()) file_inmarkers = readMarker_file(string(qPrintable(p.inmarker_file)));

        LocationSimple t;
        for(int i = 0; i < file_inmarkers.size(); i++)
        {
            t.x = file_inmarkers[i].x;
            t.y = file_inmarkers[i].y;
            t.z = file_inmarkers[i].z;
            if(t.x<p.xc0+1 || t.x>p.xc1+1 || t.y<p.yc0+1 || t.y>p.yc1+1 || t.z<p.zc0+1 || t.z>p.zc1+1)
            {
                if(i==0)
                {
                    v3d_msg("The first marker is invalid.",p.b_menu);
                    return false;
                }
                else continue;
            }
            p.landmarks.push_back(t);
        }
    }

    if(p.landmarks.size() < 2 && p.b_intensity ==1)
    {
       v3d_msg("You have to select at least two markers if using high intensity background option.",p.b_menu);
       return false;
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
    tmpstr =  qPrintable( qtstr.setNum(p.is_break_accept).prepend("#is_gap = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.cnn_type).prepend("#cnn_type = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_256cube).prepend("#b_256cube = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_RadiusFrom2D).prepend("#b_radiusFrom2D = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_resample).prepend("#b_resample = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_intensity).prepend("#b_intensity = ") ); infostring.push_back(tmpstr);
    tmpstr =  qPrintable( qtstr.setNum(p.b_brightfiled).prepend("#b_brightfiled = ") ); infostring.push_back(tmpstr);
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

        if (p.b_brightfiled)
        {
            if(!invertedsubvolumecopy(p4dImageNew,
                                      p.p4dImage,
                                      p.xc0, p.xc1-p.xc0+1,
                                      p.yc0, p.yc1-p.yc0+1,
                                      p.zc0, p.zc1-p.zc0+1,
                                      p.channel, 1))
            return false;
        }
        else
        {
            if(!subvolumecopy(p4dImageNew,
                              p.p4dImage,
                              p.xc0, p.xc1-p.xc0+1,
                              p.yc0, p.yc1-p.yc0+1,
                              p.zc0, p.zc1-p.zc0+1,
                              p.channel, 1))
            return false;
        }
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

    int marker_thresh = INF;
    if(p.b_intensity)
    {
        if(p.b_brightfiled) p.bkg_thresh = 255 - p.bkg_thresh;

        for(int d = 1; d < p.landmarks.size(); d++)
        {
            int marker_x = p.landmarks[d].x - p.xc0;
            int marker_y = p.landmarks[d].y - p.yc0;
            int marker_z = p.landmarks[d].z - p.zc0;

            if(indata1d[marker_z*in_sz[0]*in_sz[1] + marker_y*in_sz[0] + marker_x] < marker_thresh)
            {
                marker_thresh = indata1d[marker_z*in_sz[0]*in_sz[1] + marker_y*in_sz[0] + marker_x];
            }
        }

        p.bkg_thresh = (marker_thresh - 10 > p.bkg_thresh) ? marker_thresh - 10 : p.bkg_thresh;
    }

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
//            p.bkg_thresh = imgAve; //+0.5*imgStd ; //(imgAve < imgStd)? imgAve : (imgAve+imgStd)*.5;
            double td= (imgStd<10)? 10: imgStd;
            p.bkg_thresh = imgAve +0.5*td ; //(imgAve < imgStd)? imgAve : (imgAve+imgStd)*.5; //20170523, PHC
        }
        else
            p.bkg_thresh = 0;

        tmpstr =  qPrintable( qtstr.setNum(p.bkg_thresh).prepend("#autoset #bkg_thresh = ") ); infostring.push_back(tmpstr);
    }
    else if (p.b_brightfiled)
    {
        p.bkg_thresh = 255 - p.bkg_thresh;
    }



    float * phi = 0;
    vector<MyMarker> inmarkers;
    for(i = 0; i < p.landmarks.size(); i++)
    {
        double x = p.landmarks[i].x - p.xc0 -1;
        double y = p.landmarks[i].y - p.yc0 -1;
        double z = p.landmarks[i].z - p.zc0 -1;

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

    vector<MyMarker *> outtree;

    //add a timer by PHC 121005
    QElapsedTimer timer2;
    timer2.start();

    if(inmarkers.empty())
    {
        cout<<"Start detecting cellbody"<<endl;
        cout << "IMAGE DATATYPE: " << datatype << endl;
        switch(datatype)
        {
            case V3D_UINT8:
                fastmarching_dt_XY(indata1d, phi, in_sz[0], in_sz[1], in_sz[2],p.cnn_type, p.bkg_thresh);
                break;
            case V3D_UINT16:  //this is no longer needed, as the data type has been converted above
                fastmarching_dt_XY((short int*)indata1d, phi, in_sz[0], in_sz[1], in_sz[2],p.cnn_type, p.bkg_thresh);
                break;
        default:
            v3d_msg("Unsupported data type");
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

    cout<<"======================================="<<endl;
    cout<<"Construct the neuron tree"<<endl;
    if(inmarkers.empty())
    {
        cerr<<"need at least one markers"<<endl;
    }
    else if(inmarkers.size() == 1)
    {
        cout<<"only one input marker"<<endl;
        if(p.is_gsdt)
        {
            if(phi == 0)
            {
                cout<<"processing fastmarching distance transformation ..."<<endl;
                switch(datatype)
                {
                    case V3D_UINT8:
                        fastmarching_dt(indata1d, phi, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh);
                        break;
                    case V3D_UINT16:  //this is no longer needed, as the data type has been converted above
                        fastmarching_dt((short int *)indata1d, phi, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh);
                        break;
                    default:
                        v3d_msg("Unsupported data type");
                        break;
                }
            }

            cout<<endl<<"constructing fastmarching tree ..."<<endl;
            fastmarching_tree_discardCross(inmarkers[0], phi, outtree, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh, p.birfucation_thres, p.is_break_accept);
        }
        else
        {
            switch(datatype)
            {
                case V3D_UINT8:
                    v3d_msg("8bit", 0);
                    fastmarching_tree_discardCross(inmarkers[0], indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh, p.birfucation_thres, p.is_break_accept);
                    break;
                case V3D_UINT16: //this is no longer needed, as the data type has been converted above
                    v3d_msg("16bit", 0);
                    fastmarching_tree_discardCross(inmarkers[0], (short int*)indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh, p.birfucation_thres, p.is_break_accept);
                    break;
                default:
                    v3d_msg("Unsupported data type");
                    break;
            }
        }
    }
    else
    {
        vector<MyMarker> target; target.insert(target.end(), inmarkers.begin()+1, inmarkers.end());
        if(p.is_gsdt)
        {
            if(phi == 0)
            {
                cout<<"processing fastmarching distance transformation ..."<<endl;
                switch(datatype)
                {
                    case V3D_UINT8:
                        fastmarching_dt(indata1d, phi, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh);
                        break;
                    case V3D_UINT16:
                        fastmarching_dt((short int *)indata1d, phi, in_sz[0], in_sz[1], in_sz[2], p.cnn_type, p.bkg_thresh);
                        break;
                }
            }
            cout<<endl<<"constructing fastmarching tree ..."<<endl;
            fastmarching_tree(inmarkers[0], target, phi, outtree, in_sz[0], in_sz[1], in_sz[2], p.cnn_type);
        }
        else
        {
            switch(datatype)
            {
                case V3D_UINT8:
                    fastmarching_tree(inmarkers[0], target, indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], p.cnn_type);
                    break;
                case V3D_UINT16:
                    fastmarching_tree(inmarkers[0], target, (short int*) indata1d, outtree, in_sz[0], in_sz[1], in_sz[2], p.cnn_type);
                    break;
            }
        }
    }
    cout<<"======================================="<<endl;

    //save a copy of the ini tree
    cout<<"Save the initial unprunned tree"<<endl;
    vector<MyMarker*> & inswc = outtree;

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

        saveSWC_file(QString(p.p4dImage->getFileName()).append("_ini.swc").toStdString(), inswc, infostring);

        for (tmpi=0; tmpi<inswc.size(); tmpi++)
        {
            inswc[tmpi]->x = tmpswc[tmpi]->x;
            inswc[tmpi]->y = tmpswc[tmpi]->y;
            inswc[tmpi]->z = tmpswc[tmpi]->z;
        }

        for(tmpi = 0; tmpi < tmpswc.size(); tmpi++)
            delete tmpswc[tmpi];
        tmpswc.clear();
    }


    cout<<"Pruning neuron tree"<<endl;

    vector<MyMarker*> outswc;
    if(p.is_coverage_prune)
    {
        v3d_msg("start to use APP2 program.\n", 0);
        happ(inswc, outswc, indata1d, in_sz[0], in_sz[1], in_sz[2], p.bkg_thresh, p.length_thresh, p.SR_ratio);
    }
    else
    {
        hierarchy_prune(inswc, outswc, indata1d, in_sz[0], in_sz[1], in_sz[2], p.length_thresh);
        if(1) //get radius
        {
            double real_thres = 40; //PHC 20121011
            if (real_thres<p.bkg_thresh) real_thres = p.bkg_thresh;
            for(i = 0; i < outswc.size(); i++)
            {
                outswc[i]->radius = markerRadius(indata1d, in_sz, *(outswc[i]), real_thres);
            }
        }
    }

    qint64 etime2 = timer2.elapsed();
    qDebug() << " **** neuron tracing procedure takes [" << etime2 << " milliseconds]";

    if (p4dImageNew) {delete p4dImageNew; p4dImageNew=0;} //free buffer

    if(p.b_256cube)
    {
        inmarkers[0].x *= dfactor_xy;
        inmarkers[0].y *= dfactor_xy;
        inmarkers[0].z *= dfactor_z;

    }

    if(1)
    {
        QString rootposstr="", tmps;
        tmps.setNum(int(inmarkers[0].x+0.5)).prepend("_x"); rootposstr += tmps;
        tmps.setNum(int(inmarkers[0].y+0.5)).prepend("_y"); rootposstr += tmps;
        tmps.setNum(int(inmarkers[0].z+0.5)).prepend("_z"); rootposstr += tmps;
        //QString outswc_file = callback.getImageName(curwin) + rootposstr + "_app2.swc";
        QString outswc_file;
        if(!p.outswc_file.isEmpty())
            outswc_file = p.outswc_file;
        else
            outswc_file = QString(p.p4dImage->getFileName()) + rootposstr + "_app2.swc";

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
        double real_thres = 40; //PHC 20121011 //This should be rescaled later for datatypes that are not UINT8

        if (real_thres<p.bkg_thresh) real_thres = p.bkg_thresh;
        V3DLONG szOriginalData[4] = {p.p4dImage->getXDim(), p.p4dImage->getYDim(), p.p4dImage->getZDim(), 1};
        unsigned char * pOriginalData = (unsigned char *)(p.p4dImage->getRawDataAtChannel(p.channel));
        if(p.b_brightfiled)
        {
            for(V3DLONG i = 0; i < p.p4dImage->getTotalUnitNumberPerChannel(); i++)
                pOriginalData[i] = 255 - pOriginalData[i];

        }

        int method_radius_est = ( p.b_RadiusFrom2D ) ? 1 : 2;

        switch (p.p4dImage->getDatatype())
        {
            case V3D_UINT8:
            {
                for(i = 0; i < outswc.size(); i++)
                {
                    //printf(" node %ld of %ld.\n", i, outswc.size());
                    outswc[i]->radius = markerRadius(pOriginalData, szOriginalData, *(outswc[i]), real_thres, method_radius_est);
                }
            }
                break;
            case V3D_UINT16:
            {
                unsigned short int *pOriginalData_uint16 = (unsigned short int *)pOriginalData;
                for(i = 0; i < outswc.size(); i++)
                {
                    //printf(" node %ld of %ld.\n", i, outswc.size());
                    outswc[i]->radius = markerRadius(pOriginalData_uint16, szOriginalData, *(outswc[i]), real_thres * 16, method_radius_est); //*16 as it is often 12 bit data
                }
            }
                break;
            case V3D_FLOAT32:
            {
                float *pOriginalData_float = (float *)pOriginalData;
                for(i = 0; i < outswc.size(); i++)
                {
                    //printf(" node %ld of %ld.\n", i, outswc.size());
                    outswc[i]->radius = markerRadius(pOriginalData_float, szOriginalData, *(outswc[i]), real_thres, method_radius_est);
                }
            }
                break;
            default:
                break;
        }

        if(p.b_brightfiled)
        {
            for(V3DLONG i = 0; i < p.p4dImage->getTotalUnitNumberPerChannel(); i++)
                pOriginalData[i] = 255 - pOriginalData[i];

        }
        //prepare the output comments for neuron info in the swc file

        tmpstr =  qPrintable( qtstr.setNum(etime1).prepend("#neuron preprocessing time (milliseconds) = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(etime2).prepend("#neuron tracing time (milliseconds) = ") ); infostring.push_back(tmpstr);
        saveSWC_file(outswc_file.toStdString(), outswc, infostring);

        if(outswc.size()>1)
        {

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
            if(p.b_resample)
                callback.callPluginFunc(full_plugin_name_resample,func_name_resample,input_resample,output);
            arg.type = "random";std::vector<char*> arg_input_sort;
            arg_input_sort.push_back(fileName_string);
            arg.p = (void *) & arg_input_sort; input_sort<< arg;
            arg.type = "random";std::vector<char*> arg_sort_para; arg_sort_para.push_back("0");arg.p = (void *) & arg_sort_para; input_sort << arg;
            QString full_plugin_name_sort = "sort_neuron_swc";
            QString func_name_sort = "sort_swc";
            callback.callPluginFunc(full_plugin_name_sort,func_name_sort, input_sort,output);

            vector<MyMarker*> temp_out_swc = readSWC_file(outswc_file.toStdString());
            saveSWC_file_app2(outswc_file.toStdString(), temp_out_swc, infostring);
        }
        //v3d_msg(QString("The tracing uses %1 ms (%2 ms for preprocessing and %3 for tracing). Now you can drag and drop the generated swc fle [%4] into Vaa3D."
        //                ).arg(etime1+etime2).arg(etime1).arg(etime2).arg(outswc_file), p.b_menu);

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
    else
    {
//        NeuronTree nt = swc_convert(outswc);
//        callback.setSWC(curwin, nt);
//        callback.open3DWindow(curwin);
//        callback.getView3DControl(curwin)->updateWithTriView();
    }
    //release memory
    if(phi){delete [] phi; phi = 0;}
    for(V3DLONG i = 0; i < outtree.size(); i++) delete outtree[i];
    outtree.clear();

    if (b_dofunc)
    {
        if (p.p4dImage) {delete p.p4dImage; p.p4dImage=NULL;}
    }

    return true;
}




