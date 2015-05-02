#include "app2.h"
#include "v3d_interface.h"
#include "hierarchy_prune.h"
#include "marker_radius.h"
#include "math.h"


app2::app2()
{
}

template<class T> bool init_prune(vector<MyMarker*> &inswc, vector<MyMarker*> & outswc, T * inimg1d,
                                 long sz0, long sz1, long sz2, double bkg_thresh = 10.0, double length_thresh = 2.0,
                                  double SR_ratio = 1.0/9.5, bool is_leaf_prune = true, bool is_smooth = true)
{
    double T_max = (1ll << sizeof(T));

    V3DLONG sz01 = sz0 * sz1;
    V3DLONG tol_sz = sz01 * sz2;

    map<MyMarker*, int> child_num;
    getLeaf_markers(inswc, child_num);

    vector<HierarchySegment*> topo_segs;
    cout<<"Construct hierarchical segments"<<endl;
    swc2topo_segs(inswc, topo_segs, INTENSITY_DISTANCE_METHOD, inimg1d, sz0, sz1, sz2);
    vector<HierarchySegment*> filter_segs;
    int count_20,count15_20,count5_10,count0_5,count0,count;
    count_20=count15_20=count5_10=count0_5=count=count0=0;
    vector<HierarchySegment*> bone_num;

    for(int i = 0; i < topo_segs.size(); i++)
    {
        HierarchySegment *seg=topo_segs[i];

        //if(topo_segs[i]->length >= length_thresh) filter_segs.push_back(topo_segs[i]);
        int length= seg->length;
        count++;
        if (length<=0) continue;
        if (length>0&&length<=10)
        {
            count0_5++;
            filter_segs.push_back(topo_segs[i]);

        }
        else if (length>10&&length<=20)
        {
            count15_20++;
        }
        else if (length>20)
        {
            count_20++;
        }
    }
    qDebug()<<"count total:"<<count++<<"count0-5:"<<count0_5<<
              "count5-10:"<<count5_10<<"count10-20:"<<count15_20<<"count20:"<<
              count_20<<"filter seg size:"<<filter_segs.size();
    //vector<MyMarker> all_leaf_marker;
    for (V3DLONG i=0;i<filter_segs.size();i++)
    {
        MyMarker * leaf_marker = filter_segs[i]->leaf_marker;
        outswc.push_back(leaf_marker);
    }
    //write_csv(all_leaf_marker,"all_leaf_marker.marker");
    //topo_segs2swc(filter_segs,outswc);
    //qDebug()<<"all leaf makrer size:"<<all_leaf_marker.size();

//    MyMarker * root_marker = longest_seg->root_marker;
//    MyMarker * p = leaf_marker;
//    QString fn="test.swc";
//    FILE *fp=fopen(fn.toAscii(),"w");
//    fprintf(fp, "##n,type,x,y,z,radius,parent\n");
//    int count_num=1;
//    while(true)
//    {
//        double real_thres = 20; if (real_thres<bkg_thresh) real_thres = bkg_thresh; //by PHC 20121012
//          p->radius = markerRadiusXY(inimg1d, in_sz, *p, real_thres);
//        fprintf(fp, "%d 3 %.lf %.1f %.1f %.1f %d\n",count_num,p->x,p->y,p->z,p->radius,count_num-1);
//        count_num++;
////        for (V3DLONG dx=MAX(0,p->x+0.5-p->radius);dx<=MIN(sz0-1,p->x+0.5+p->radius);dx++){
////            for (V3DLONG dy=MAX(0,p->y+0.5-p->radius);dy<=MIN(sz1-1,p->y+0.5+p->radius);dy++){
////                for (V3DLONG dz=MAX(0,p->z+0.5-p->radius);dz<=MIN(sz2-1,p->z+0.5+p->radius);dz++){
////                    V3DLONG pos=dx+dy*sz0+dz*sz0*sz1;
////                    mask_img[pos]=0;
////                }
////            }
////        }
//        if(p == root_marker) break;
//        p = p->parent;
//    }
//   fclose(fp);

//    multimap<double, HierarchySegment*> seg_dist_map;
//    for(int i = 0; i < filter_segs.size(); i++)
//    {
//        double dst = filter_segs[i]->length;
//        seg_dist_map.insert(pair<double, HierarchySegment*> (dst, filter_segs[i]));
//    }
//    multimap<double, HierarchySegment*>::reverse_iterator it = seg_dist_map.rbegin();
//    for (int i=0;i<2;i++)
//    {
//        qDebug()<<"segment length:"<<it->first;
//        HierarchySegment *seg=it->second;
//        MyMarker * leaf_marker = seg->leaf_marker;
//        MyMarker * root_marker = seg->root_marker;
//        MyMarker * p = leaf_marker;
//        while(true)
//        {
//            outswc.push_back(p);
//            if(p == root_marker) break;
//            p = p->parent;
//        }
//        it++;
//    }

//    qDebug()<<"filter_seg size before calculating radius:"<<filter_segs.size();
//    cout<<"Calculating radius for every node"<<endl;
//    for(int i = 0; i < filter_segs.size(); i++)
//    {
//        HierarchySegment * seg = filter_segs[i];
//        MyMarker * leaf_marker = seg->leaf_marker;
//        MyMarker * root_marker = seg->root_marker;
//        MyMarker * p = leaf_marker;
//        int sum_radius=0;
//        while(true)
//        {
//            double real_thres = 20; if (real_thres<bkg_thresh) real_thres = bkg_thresh; //by PHC 20121012

//            p->radius = markerRadiusXY(inimg1d, in_sz, *p, real_thres);
//            sum_radius=sum_radius+p->radius;
//            if(p == root_marker) break;
//            p = p->parent;


//        }
//    }

}





bool proc_app2(V3DPluginCallback2 &callback,PARA_APP2 &p, const QString & versionStr)
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
            if(t.x<p.xc0 || t.x>p.xc1 || t.y<p.yc0 || t.y>p.yc1 || t.z<p.zc0 || t.z>p.zc1)
            {
                if(i==0)
                {
                    v3d_msg("The first marker is invalid.",b_menu);
                    return false;
                }
                else
                    continue;

            }
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
    tmpstr =  qPrintable( qtstr.setNum(p.is_break_accept).prepend("#is_gap = ") ); infostring.push_back(tmpstr);
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

    vector<MyMarker *> outtree;

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
        v3d_msg("start to use happ.\n", 0);
        init_prune(inswc, outswc, indata1d, in_sz[0], in_sz[1], in_sz[2], p.bkg_thresh, p.length_thresh, p.SR_ratio);
    }

    V3DLONG tmpi;
    vector<MyMarker*> tmpswc;
    for (tmpi=0; tmpi<outswc.size(); tmpi++)
    {
        MyMarker * curp = new MyMarker(*(outswc[tmpi]));
        tmpswc.push_back(curp);

        if (dfactor_xy>1) outswc[tmpi]->x *= dfactor_xy;
        outswc[tmpi]->x += (p.xc0);
        if (dfactor_xy>1) outswc[tmpi]->x += dfactor_xy/2;

        if (dfactor_xy>1) outswc[tmpi]->y *= dfactor_xy;
        outswc[tmpi]->y += (p.yc0);
        if (dfactor_xy>1) outswc[tmpi]->y += dfactor_xy/2;

        if (dfactor_z>1) outswc[tmpi]->z *= dfactor_z;
        outswc[tmpi]->z += (p.zc0);
        if (dfactor_z>1)  outswc[tmpi]->z += dfactor_z/2;
    }

//    indata1d = p.p4dImage->getRawDataAtChannel(0);
//    in_sz[0] = p.p4dImage->getXDim();
//    in_sz[1] = p.p4dImage->getYDim();
//    in_sz[2] = p.p4dImage->getZDim();
//    V3DLONG sz_image[3]={in_sz[0],in_sz[1],in_sz[2]};

    //qDebug()<<"sz image size:"<<sz_image[0]<<":"<<sz_image[1]<<":"<<sz_image[2];
    qDebug()<<"outswc size:"<<outswc.size();


//    }
//    fclose(fp);
    QString filename="all_leaf.marker";
    FILE *fp=fopen(filename.toAscii(),"wt");
    fprintf(fp, "##x,y,z,radius,shape,name,comment\n");

     for (int i=0;i<outswc.size(); i++)
     {
         fprintf(fp, "%.lf,%.lf,%.lf,%.1f,%d," "," "\n",outswc[i]->x,outswc[i]->y,outswc[i]->z,1,1);

     }
     fclose(fp);
    //saveSWC_file(QString(p.p4dImage->getFileName()).append("_out.swc").toStdString(), outswc, infostring);

//    unsigned char* image_input=new unsigned char[in_sz[0]*in_sz[1]*in_sz[2]*sizeof(unsigned char)];
//    memcpy(image_input,indata1d,in_sz[0]*in_sz[1]*in_sz[2]*sizeof(unsigned char));
//    Image4DSimple image4d;
//    image4d.setData(image_input,in_sz[0],in_sz[1],in_sz[2],1,V3D_UINT8);
//    v3dhandle newwin=callback.newImageWindow();
//    callback.setImage(newwin,&image4d);
//    callback.setImageName(newwin,"test_win");

    qDebug()<<"the end_____________________";

}



