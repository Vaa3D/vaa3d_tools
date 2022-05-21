#include "n_class.h"
#include <algorithm>
#include <map>
#include <fstream>
#include "v3dneuron_gd_tracing.h"
#include "v3d_message.h"
//#include "swc_to_maskimage.h"
//#include "sort_swc.h"
//#include "../../zhi/AllenNeuron_postprocessing/sort_swc_IVSCC.h"
#include "filter_dialog.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
#include "fastmarching_dt.h"
#include "bdb_minus.h"

#define PI 3.14159266

bool sortSWC(QList<NeuronSWC> &neurons, QList<NeuronSWC> &result, NeuronSWC s){
    float d = 10000;
    int root_id = -1;
    for(int i=0; i<neurons.size();++i){
            NeuronSWC n =neurons.at(i);
            if (n.parent == -1 && zx_dist(s, n) < d) {
                root_id = n.n;
                d = zx_dist(s, n);
            }
    }
//    for (NeuronSWC n : neurons) {
//        if (n.parent == -1 && zx_dist(s, n) < d) {
//            root_id = n.n;
//            d = zx_dist(s, n);
//        }
//    }
    return root_id == -1 ? SortSWC(neurons,result,VOID,1) : SortSWC(neurons, result, root_id, 1);
}

bool sortSWC(NeuronTree& nt, NeuronSWC s){
    QList<NeuronSWC> sortListNeuron = QList<NeuronSWC>();
    if(!nt.listNeuron.isEmpty()){
        nt.hashNeuron.clear();
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.hashNeuron.insert(nt.listNeuron[i].n,i);
        }
        sortSWC(nt.listNeuron,sortListNeuron, s);
        nt.listNeuron.clear();
        nt.listNeuron = sortListNeuron;
        nt.hashNeuron.clear();
        for(int i=0; i<nt.listNeuron.size(); i++){
            nt.hashNeuron.insert(nt.listNeuron[i].n,i);
        }
    }
    return true;
}

bool Comp(const int &a,const int &b)
{
    return a>b;
}

bool Branch::get_r_points_of_branch(vector<NeuronSWC> &r_points, NeuronTree &nt)
{
    NeuronSWC tmp=end_point;
    r_points.push_back(end_point);
    while(tmp.n!=head_point.n)
    {
        tmp=nt.listNeuron[nt.hashNeuron.value(tmp.parent)];
        r_points.push_back(tmp);
    }
    //r_points.push_back(head_point);
    return true;
}

bool Branch::get_points_of_branch(vector<NeuronSWC> &points, NeuronTree &nt)
{
    vector<NeuronSWC> r_points;
    this->get_r_points_of_branch(r_points,nt);
    while(!r_points.empty())
    {
        NeuronSWC tmp=r_points.back();
        r_points.pop_back();
        points.push_back(tmp);
    }

    return true;
}

bool Branch::refine_branch(vector<NeuronSWC> &points, QString braindir, V3DPluginCallback2 &callback, NeuronTree &nt)
{
    /*
    QDir all_braindir(braindir);
    all_braindir.cdUp();
    QFileInfoList list_braindir = all_braindir.entryInfoList(QStringList(),QDir::Dirs,QDir::Name);
    int dir_count = list_braindir.size();
    QString max_resolution_braindir = list_braindir[dir_count-1].absoluteFilePath();
    QString second_resolution_braindir = list_braindir[dir_count-2].absoluteFilePath();
    QString third_resolution_braindir = list_braindir[dir_count-3].absoluteFilePath();

    this->get_points_of_branch(points,nt);

    for(V3DLONG i=0; i<points.size(); ++i)
    {
        points[i].x /= 4;
        points[i].y /= 4;
        points[i].z /= 4;
    }

    unsigned char* mpdata = 0, spdata = 0, tpdata = 0;
    size_t x0 = IN, x1 = 0, y0 = IN, y1 = 0, z0 = IN, z1 = 0;
    for(V3DLONG i=0; i<points.size(); ++i)
    {
        x0 = (x0>points[i].x) ? points[i].x : x0;
        x1 = (x1<points[i].x) ? points[i].x : x1;
        y0 = (y0>points[i].y) ? points[i].y : y0;
        y1 = (y1<points[i].y) ? points[i].y : y1;
        z0 = (z0>points[i].z) ? points[i].z : z0;
        z1 = (z1<points[i].z) ? points[i].z : z1;
    }
    x0 -= 10, x1 += 10, y0 -= 10, y1 += 10, z0 -= 10, z1 += 10;

    qDebug()<<"start get subvolume...";

    mpdata = callback.getSubVolumeTeraFly(third_resolution_braindir.toStdString().c_str(),x0,x1,y0,y1,z0,z1);

    qDebug()<<"got subvolume";

    for(V3DLONG i=0; i<points.size(); ++i)
    {
        points[i].x -= x0;
        points[i].y -= y0;
        points[i].z -= z0;
    }

    V3DLONG sz0 = x1 - x0, sz1 = y1 - y0, sz2 = z1 - z0;

    V3DLONG sz[4] = {sz0,sz1,sz2,1};



    qDebug()<<"start to get origin image and branch swc";

    NeuronTree torig_nt;
    for(int i=0; i<points.size(); ++i)
    {
        torig_nt.listNeuron.push_back(points[i]);
    }

    QString torigtiffile = "C://Users//BrainCenter//Desktop//test_refine//image_swc//" + QString::number(x0) + "_" + QString::number(y0) + "_" + QString::number(z0) + "third_resolution.tif";
    QString torigeswcfile = "C://Users//BrainCenter//Desktop//test_refine//image_swc//" + QString::number(x0) + "_" + QString::number(y0) + "_" + QString::number(z0) + "third_resolution.eswc";

    simple_saveimage_wrapper(callback,torigtiffile.toStdString().c_str(),tpdata,sz,1);
    writeESWC_file(torigeswcfile,torig_nt);

    unsigned char* data1d_mask = 0;
    V3DLONG sz_num = sz0*sz1*sz2;
    data1d_mask = new unsigned char[sz_num];
    memset(data1d_mask,0,sz_num*sizeof(unsigned char));
    double margin = 5;
    ComputemaskImage(torig_nt,data1d_mask,sz0,sz1,sz2,margin);

    for(V3DLONG i=0; i<sz_num; ++i)
    {
        tpdata[i] = (data1d_mask[i]==0) ? 0 : tpdata[i];
    }



    qDebug()<<"start trans 3d...";

    unsigned char*** indata3d;
    indata3d = new unsigned char**[sz2];
    for(int i=0; i<sz2; ++i)
    {
        indata3d[i] = new unsigned char*[sz1];
        for(int j=0; j<sz1; ++j)
        {
            indata3d[i][j] = new unsigned char[sz0];
        }
    }
    for(int z=0; z<sz2; ++z)
    {
        for(int y=0; y<sz1; ++y)
        {
            for(int x=0; x<sz0; ++x)
            {
                indata3d[z][y][x] = tpdata[z*sz1*sz0+y*sz0+x];
            }
        }
    }

    float* phi = 0;
    int cnn_type = 3;
    double valueave = getImageAveValue(indata3d,sz0,sz1,sz2);
    double valuestd = getImageStdValue(indata3d,sz0,sz1,sz2);
    int bkg_thres = valueave + 0.5*valuestd;

    qDebug()<<"start fastmarching-----------------------------------------";
    fastmarching_dt(tpdata,phi,(int)sz0,(int)sz1,(int)sz2,cnn_type,bkg_thres);

    qDebug()<<"end fastmarching-------------------------------------------";

    float max_value = 0, min_value =IN;

    for(V3DLONG i=0; i<sz0*sz1*sz2; ++i)
    {
        max_value = (phi[i]>max_value) ? phi[i] : max_value;
        min_value = (phi[i]<min_value) ? phi[i] : min_value;
    }
    max_value -= min_value;

    for(int z=0; z<sz2; ++z)
    {
        for(int y=0; y<sz1; ++y)
        {
            for(int x=0; x<sz0; ++x)
            {
                indata3d[z][y][x] = (unsigned char)(((phi[z*sz1*sz0+y*sz0+x]-min_value)/max_value)*255);
            }
        }
    }

    qDebug()<<"end trans 3d";

    BDB_Minus_Prior_Parameter bdb_para;
    bdb_para.f_length = 0.1;
    bdb_para.f_prior = 0.1;
    bdb_para.f_smooth = 0.2;
    bool b_fix_end = false;

    vector<NeuronSWC> prior;

    multimap<unsigned char,int> intensitymap;
    for(int i=0; i<points.size(); ++i)
    {
        unsigned char intensity = indata3d[(int)points[i].z][(int)points[i].y][(int)points[i].x];
        intensitymap.insert(pair<unsigned char,int>(intensity,i));
    }
    int count = 0;
    multimap<unsigned char,int>::reverse_iterator it =intensitymap.rbegin();
    while(count<3&&it!=intensitymap.rend())
    {
        int index = it->second;
        prior.push_back(points[index]);
        it++;
        count++;
    }

    prior.clear();

    point_bdb_minus_3d_localwinmass_prior(indata3d,sz0,sz1,sz2,points,bdb_para,b_fix_end,prior);

    qDebug()<<"end point_bdb_minus_3d_localwinmass_prior";

    qDebug()<<"start to get last image and eswc";

    QString tresulttiffile = "C://Users//BrainCenter//Desktop//test_refine//image_swc//" + QString::number(x0) + "_" + QString::number(y0) + "_" + QString::number(z0) + "third_resolution_result.tif";
    QString tresultswcfile = "C://Users//BrainCenter//Desktop//test_refine//image_swc//" + QString::number(x0) + "_" + QString::number(y0) + "_" + QString::number(z0) + "third_resolution_result.eswc";

    NeuronTree result_nt;
    for(int i=0; i<points.size(); ++i)
    {
        points[i].type = 5;
        result_nt.listNeuron.push_back(points[i]);
    }

    simple_saveimage_wrapper(callback,tresulttiffile.toStdString().c_str(),tpdata,sz,1);
    writeESWC_file(tresultswcfile,result_nt);
*/

    QDir all_braindir(braindir);
//    all_braindir.cdUp();
    all_braindir.setSorting(QDir::Size);
    QFileInfoList list_braindir = all_braindir.entryInfoList(QStringList(),QDir::Dirs|QDir::NoDotAndDotDot,QDir::Type);
    int dir_count = list_braindir.size();

    qDebug()<<"dir count: "<<dir_count;

    map<int,int> sizemap;

    for(int i=0; i<dir_count; ++i)
    {
        qDebug()<<i<<": "<<list_braindir[i].absoluteFilePath();
        QString t = list_braindir[i].baseName();
        QStringList ts =t.split('x');
        int resolution = ts[1].toInt();
        qDebug()<<resolution;
        sizemap[resolution] = i;
    }

    QVector<QFileInfo> vtmp;
    map<int,int>::iterator it = sizemap.begin();
//    qDebug()<<"-------------------------";
    while(it!=sizemap.end())
    {
        int index = it->second;
        vtmp.push_back(list_braindir[index]);
        it++;
    }
//    qDebug()<<"end while";
    list_braindir.clear();
    for(int i=0; i<dir_count; ++i)
    {
        list_braindir.push_back(vtmp[i]);
    }
//    qDebug()<<"end assign";

//    qDebug()<<"size； "<<list_braindir.size();

    for(int i=0; i<dir_count; ++i)
    {
        qDebug()<<i<<": "<<list_braindir[i].absoluteFilePath();
    }

    QString current_braindir;

    BDB_Minus_Prior_Parameter bdb_para;
    bdb_para.f_length = 0;
    bdb_para.f_prior = 0;
    bdb_para.f_smooth = 0;
    bool b_fix_end = false;

    qDebug()<<"bdb_para---------------------------------------------------------------------------";
    qDebug()<<bdb_para.f_length<<" "<<bdb_para.f_prior<<" "<<bdb_para.f_smooth<<" "<<bdb_para.Kfactor<<" "<<bdb_para.nloops;

    BDB_Minus_ConfigParameter mypara;
    mypara.TH = 0.1;

    vector<NeuronSWC> prior;

    this->get_points_of_branch(points,nt);

    for(V3DLONG i=0; i<points.size(); ++i)
    {
        points[i].x /= 8;
        points[i].y /= 8;
        points[i].z /= 8;
    }
    V3DLONG *in_zz=0;
    callback.getDimTeraFly(current_braindir.toStdString().c_str(),in_zz);
    for(int i=0; i<3; ++i)
    {
        for(V3DLONG j=0; j<points.size(); ++j)
        {
            points[j].x *= 2;
            points[j].y *= 2;
            points[j].z *= 2;
        }
        size_t x0 = IN, x1 = 0, y0 = IN, y1 = 0, z0 = IN, z1 = 0;
        for(V3DLONG j=0; j<points.size(); ++j)
        {
            x0 = (x0>points[j].x) ? points[j].x : x0;
            x1 = (x1<points[j].x) ? points[j].x : x1;
            y0 = (y0>points[j].y) ? points[j].y : y0;
            y1 = (y1<points[j].y) ? points[j].y : y1;
            z0 = (z0>points[j].z) ? points[j].z : z0;
            z1 = (z1<points[j].z) ? points[j].z : z1;
        }
        x0 -= 10, x1 += 10, y0 -= 10, y1 += 10, z0 -= 10, z1 += 10;
        if(x0<=0){x0=1;}
        if(x1>=in_zz[0]){x1=in_zz[0]-1;}
        if(y0<=0){y0=1;}
        if(y1>=in_zz[1]){y1=in_zz[1]-1;}
        if(z0<=0){z0=1;}
        if(z1>=in_zz[2]){z1=in_zz[2]-1;}
        V3DLONG sz0 = x1 - x0, sz1 = y1 - y0, sz2 = z1 - z0;

        V3DLONG sz[4] = {sz0,sz1,sz2,1};

        unsigned char* pdata = 0;

        qDebug()<<"start to get volume "<<i<<"--------------------------";

        current_braindir = list_braindir[i+4].absoluteFilePath();

        qDebug()<<"braindir "<<current_braindir.toStdString().c_str();

        pdata = callback.getSubVolumeTeraFly(current_braindir.toStdString().c_str(),x0,x1,y0,y1,z0,z1);

        for(V3DLONG j=0; j<points.size(); ++j)
        {
            points[j].x -= x0;
            points[j].y -= y0;
            points[j].z -= z0;
        }

//        prior.clear();
//        for(int j=0; j<points.size(); ++j)
//        {
//            prior.push_back(points[j]);
//        }

        qDebug()<<"start to get origin image and branch sw "<<i<<"---------------------------";

        NeuronTree orig_nt;
        for(int j=0; j<points.size(); ++j)
        {
            points[j].type = 3;
            orig_nt.listNeuron.push_back(points[j]);
        }

        QString origtiffile = "C://Users//BrainCenter//Desktop//test_refine//image_swc//" + QString::number(x0) + "_" + QString::number(y0) + "_" + QString::number(z0) + "_" + QString::number(i) + "_resolution.tif";
        QString origeswcfile = "C://Users//BrainCenter//Desktop//test_refine//image_swc//" + QString::number(x0) + "_" + QString::number(y0) + "_" + QString::number(z0) + "_" + QString::number(i) + "_resolution.eswc";
        simple_saveimage_wrapper(callback,origtiffile.toStdString().c_str(),pdata,sz,1);
        writeESWC_file(origeswcfile,orig_nt);

        qDebug()<<"start to get mask "<<i<<"---------------------------------";

        unsigned char* data1d_mask = 0;
        V3DLONG sz_num = sz0*sz1*sz2;
        data1d_mask = new unsigned char[sz_num];
        memset(data1d_mask,0,sz_num*sizeof(unsigned char));
        double margin = 5;
        ComputemaskImage(orig_nt,data1d_mask,sz0,sz1,sz2,margin);

        for(V3DLONG j=0; j<sz_num; ++j)
        {
            pdata[j] = (data1d_mask[j]==0) ? 0 : pdata[j];
        }

        qDebug()<<"start to trans 3d "<<i<<"-------------------------------------";

        unsigned char*** indata3d;
        indata3d = new unsigned char**[sz2];
        for(int ii=0; ii<sz2; ++ii)
        {
            indata3d[ii] = new unsigned char*[sz1];
            for(int jj=0; jj<sz1; ++jj)
            {
                indata3d[ii][jj] = new unsigned char[sz0];
            }
        }
        for(int z=0; z<sz2; ++z)
        {
            for(int y=0; y<sz1; ++y)
            {
                for(int x=0; x<sz0; ++x)
                {
                    indata3d[z][y][x] = pdata[z*sz1*sz0+y*sz0+x];
                }
            }
        }

//        vector<Coord3D> points_out;
//        points_out.clear();

//        for(int i=0; i<points.size(); ++i)
//        {
//            Coord3D c;
//            c.x = points[i].x;
//            c.y = points[i].y;
//            c.z = points[i].z;
//            points_out.push_back(c);
//        }



        qDebug()<<"start to point_bdb_minus_3d_localwinmass_prior "<<i<<"----------------------------------------------";

        vector<vector<NeuronSWC> > pointslist;

        point_bdb_minus_3d_localwinmass_prior(indata3d,sz0,sz1,sz2,points,bdb_para,b_fix_end,prior);

//        point_bdb_minus_3d_localwinmass(indata3d,sz0,sz1,sz2,points_out,mypara);

//        qDebug()<<"mypara: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

//        qDebug()<<mypara.f_image<<" "<<mypara.f_length<<" "<<mypara.f_smooth<<" "<<mypara.Kfactor<<" "<<mypara.TH;

//        qDebug()<<"start to get result img and swc "<<i<<"--------------------------------------------";


//        for(int i=0; i<points.size(); ++i)
//        {
//            points[i].x = points_out[i].x;
//            points[i].y = points_out[i].y;
//            points[i].z = points_out[i].z;
//        }
        NeuronTree result_nt;
        QString resulttiffile;
        QString resultswcfile;

//        for(int j =0 ; j<pointslist.size(); ++j)
//        {

//            result_nt.listNeuron.clear();
//            for(int k=0; k<pointslist[j].size(); ++k)
//            {
//                pointslist[j][k].type = 4+j;
//                result_nt.listNeuron.push_back(pointslist[j][k]);
//            }

//            resulttiffile = "C://Users//BrainCenter//Desktop//test_refine//image_swc//" + QString::number(x0) + "_" + QString::number(y0) + "_" + QString::number(z0) + "_" + QString::number(i) + "_" + QString::number(j) + "_loop" + "_resolution_result.tif";
//            resultswcfile = "C://Users//BrainCenter//Desktop//test_refine//image_swc//" + QString::number(x0) + "_" + QString::number(y0) + "_" + QString::number(z0) + "_" + QString::number(i) + "_" + QString::number(j) + "_loop" + "_resolution_result.eswc";

//            simple_saveimage_wrapper(callback,resulttiffile.toStdString().c_str(),pdata,sz,1);
//            writeESWC_file(resultswcfile,result_nt);
//        }
        for(int i=0; i<points.size(); ++i)
        {
            points[i].type = 6;
            result_nt.listNeuron.push_back(points[i]);
        }

        resulttiffile = "C://Users//BrainCenter//Desktop//test_refine//image_swc//" + QString::number(x0) + "_" + QString::number(y0) + "_" + QString::number(z0) + "_" + QString::number(i) + "_resolution_result.tif";
        resultswcfile = "C://Users//BrainCenter//Desktop//test_refine//image_swc//" + QString::number(x0) + "_" + QString::number(y0) + "_" + QString::number(z0) + "_" + QString::number(i) + "_resolution_result.eswc";

        simple_saveimage_wrapper(callback,resulttiffile.toStdString().c_str(),pdata,sz,1);
        writeESWC_file(resultswcfile,result_nt);

        for(V3DLONG i=0; i<points.size(); ++i)
        {
            points[i].x += x0;
            points[i].y += y0;
            points[i].z += z0;
        }

        qDebug()<<"free memory "<<i<<"----------------------------------------";

        if(pdata) delete[] pdata;
        if(data1d_mask) delete[] data1d_mask;

        for(int i=0; i<sz2; ++i)
        {
            for(int j=0; j<sz1; ++j)
            {
                delete[] indata3d[i][j];
            }
            delete[] indata3d[i];
        }
        delete[] indata3d;
    }

    return true;
}




bool Branch::get_meanstd(QString braindir, V3DPluginCallback2 &callback, NeuronTree &nt, double &branchmean, double &branchstd, int mode, int resolution)
{
    QDir all_braindir(braindir);
    QFileInfoList list_braindir = all_braindir.entryInfoList(QStringList(),QDir::Dirs|QDir::NoDotAndDotDot);
    int dir_count = list_braindir.size();

    map<int,int> sizemap;

    for(int i=0; i<dir_count; ++i)
    {
//        qDebug()<<i<<": "<<list_braindir[i].absoluteFilePath();
        QString t = list_braindir[i].baseName();
        QStringList ts =t.split('x');
        int resolution = ts[1].toInt();
        sizemap[resolution] = i;
    }

    QVector<QFileInfo> vtmp;
    map<int,int>::iterator it =sizemap.begin();

    while(it!=sizemap.end())
    {
        int index = it->second;
        vtmp.push_back(list_braindir[index]);
        it++;
    }

    list_braindir.clear();
    for(int i=0; i<dir_count; ++i)
    {
        list_braindir.push_back(vtmp[i]);
//        qDebug()<<list_braindir[i].absoluteFilePath();
    }

    QString current_braindir = list_braindir[dir_count-resolution].absoluteFilePath();

    qDebug()<<current_braindir;

    vector<NeuronSWC> points;
    if(mode==0)
    {
        this->get_points_of_branch(points,nt);
    }
    else if(mode==1)
    {
        points.assign(nt.listNeuron.begin(),nt.listNeuron.end());
    }


    int times = pow(2,(resolution-1));

    for(int i=0; i<points.size(); ++i)
    {
        points[i].x /= times;
        points[i].y /= times;
        points[i].z /= times;
    }
    V3DLONG *in_zz=0;
    callback.getDimTeraFly(current_braindir.toStdString().c_str(),in_zz);
    size_t x0 = IN, x1 = 0, y0 = IN, y1 = 0, z0 = IN, z1 = 0;
    for(V3DLONG j=0; j<points.size(); ++j)
    {
        x0 = (x0>points[j].x) ? points[j].x : x0;
        x1 = (x1<points[j].x) ? points[j].x : x1;
        y0 = (y0>points[j].y) ? points[j].y : y0;
        y1 = (y1<points[j].y) ? points[j].y : y1;
        z0 = (z0>points[j].z) ? points[j].z : z0;
        z1 = (z1<points[j].z) ? points[j].z : z1;
    }
    x0 -= 10, x1 += 10, y0 -= 10, y1 += 10, z0 -= 10, z1 += 10;
    if(x0<=0){x0=1;}
    if(x1>=in_zz[0]){x1=in_zz[0]-1;}
    if(y0<=0){y0=1;}
    if(y1>=in_zz[1]){y1=in_zz[1]-1;}
    if(z0<=0){z0=1;}
    if(z1>=in_zz[2]){z1=in_zz[2]-1;}
    V3DLONG sz0 = x1 - x0, sz1 = y1 - y0, sz2 = z1 - z0;

    unsigned char* pdata = 0;

    pdata = callback.getSubVolumeTeraFly(current_braindir.toStdString().c_str(),x0,x1,y0,y1,z0,z1);

    for(V3DLONG j=0; j<points.size(); ++j)
    {
        points[j].x -= x0;
        points[j].y -= y0;
        points[j].z -= z0;
    }

    NeuronTree orig_nt;
    for(int i=0; i<points.size(); ++i)
    {
        orig_nt.listNeuron.push_back(points[i]);
    }


    unsigned char* data1d_mask = 0;
    V3DLONG sz_num = sz0*sz1*sz2;
    data1d_mask = new unsigned char[sz_num];
    memset(data1d_mask,0,sz_num*sizeof(unsigned char));
    double margin = 1;
    ComputemaskImage(orig_nt,data1d_mask,sz0,sz1,sz2,margin);

    int count = 0;

    for(V3DLONG i=0; i<sz_num; ++i)
    {
        if(data1d_mask[i]!=0)
        {
            branchmean += pdata[i];
            count++;
        }
    }
    branchmean /= count;

    for(V3DLONG i=0; i<sz_num; ++i)
    {
        if(data1d_mask[i]!=0)
        {
            branchstd += (pdata[i]-branchmean)*(pdata[i]-branchmean);
        }
    }

    branchstd = sqrt(branchstd)/count;

    return true;


}
bool Branch::get_meanstd_img(string inimg_file, V3DPluginCallback2 &callback, NeuronTree &nt, double &branchmean, double &branchstd, int mode, int resolution)
{
//    QDir all_braindir(braindir);
//    QFileInfoList list_braindir = all_braindir.entryInfoList(QStringList(),QDir::Dirs|QDir::NoDotAndDotDot);
//    int dir_count = list_braindir.size();

//    map<int,int> sizemap;

//    for(int i=0; i<dir_count; ++i)
//    {
//        qDebug()<<i<<": "<<list_braindir[i].absoluteFilePath();
//        QString t = list_braindir[i].baseName();
//        QStringList ts =t.split('x');
//        int resolution = ts[1].toInt();
//        sizemap[resolution] = i;
//    }

//    QVector<QFileInfo> vtmp;
//    map<int,int>::iterator it =sizemap.begin();

//    while(it!=sizemap.end())
//    {
//        int index = it->second;
//        vtmp.push_back(list_braindir[index]);
//        it++;
//    }

//    list_braindir.clear();
//    for(int i=0; i<dir_count; ++i)
//    {
//        list_braindir.push_back(vtmp[i]);
//        qDebug()<<list_braindir[i].absoluteFilePath();
//    }

//    QString current_braindir = list_braindir[dir_count-resolution].absoluteFilePath();

//    qDebug()<<current_braindir;

    vector<NeuronSWC> points;
    if(mode==0)
    {
        this->get_points_of_branch(points,nt);
    }
    else if(mode==1)
    {
        points.assign(nt.listNeuron.begin(),nt.listNeuron.end());
    }


    //int times = pow(2,(resolution-1));  //???
    int times =1;
    for(int i=0; i<points.size(); ++i)
    {
        points[i].x /= times;
        points[i].y /= times;
        points[i].z /= times;
    }
    V3DLONG in_zz[4];
//    callback.getDimTeraFly(inimg_file,in_zz);
//    qDebug()<<in_zz[0]<<in_zz[1]<<in_zz[2]<<endl;
    unsigned char* pdata = 0;
    //pdata = callback.getSubVolumeTeraFly(inimg_file,0,in_zz[0]-1,0,in_zz[1]-1,0,in_zz[2]-1);
//    unsigned char * inimg1d_raw = 0;
//  V3DLONG in_sz[4];
    int datatype;
    simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), pdata, in_zz, datatype);
//    size_t x0 = IN, x1 = 0, y0 = IN, y1 = 0, z0 = IN, z1 = 0;
//    for(V3DLONG j=0; j<points.size(); ++j)
//    {
//        x0 = (x0>points[j].x) ? points[j].x : x0;
//        x1 = (x1<points[j].x) ? points[j].x : x1;
//        y0 = (y0>points[j].y) ? points[j].y : y0;
//        y1 = (y1<points[j].y) ? points[j].y : y1;
//        z0 = (z0>points[j].z) ? points[j].z : z0;
//        z1 = (z1<points[j].z) ? points[j].z : z1;
//    }
//    x0 -= 10, x1 += 10, y0 -= 10, y1 += 10, z0 -= 10, z1 += 10;
//    if(x0<=0){x0=1;}
//    if(x1>=in_zz[0]){x1=in_zz[0]-1;}
//    if(y0<=0){y0=1;}
//    if(y1>=in_zz[1]){y1=in_zz[1]-1;}
//    if(z0<=0){z0=1;}
//    if(z1>=in_zz[2]){z1=in_zz[2]-1;}
//    V3DLONG sz0 = x1 - x0, sz1 = y1 - y0, sz2 = z1 - z0;

//    unsigned char* pdata = 0;

//    pdata = callback.getSubVolumeTeraFly(inimg_file,x0,x1,y0,y1,z0,z1);

//    for(V3DLONG j=0; j<points.size(); ++j)
//    {
//        points[j].x -= x0;
//        points[j].y -= y0;
//        points[j].z -= z0;
//    }

    NeuronTree orig_nt;
    for(int i=0; i<points.size(); ++i)
    {
        orig_nt.listNeuron.push_back(points[i]);
    }


    unsigned char* data1d_mask = 0;
    V3DLONG sz_num = (in_zz[0])*(in_zz[1])*(in_zz[2]);
    data1d_mask = new unsigned char[sz_num];
    memset(data1d_mask,0,sz_num*sizeof(unsigned char));
    double margin = 1;
    ComputemaskImage(orig_nt,data1d_mask,in_zz[0],in_zz[1],in_zz[2],margin);

    int count = 0;

    for(V3DLONG i=0; i<sz_num; ++i)
    {
//        if(data1d_mask[i]!=0)
//        {
            branchmean += pdata[i];
            count++;
//        }
    }
    branchmean /= count;

    for(V3DLONG i=0; i<sz_num; ++i)
    {
//        if(data1d_mask[i]!=0)
//        {
            branchstd += (pdata[i]-branchmean)*(pdata[i]-branchmean);
//        }
    }

    branchstd = sqrt(branchstd)/count;

    return true;


}
bool Branch::splitbranch(NeuronTree &nt, vector<Branch> &segs, double thres)
{
    vector<NeuronSWC> points;
    this->get_points_of_branch(points,nt);
    qDebug()<<"points_of_branch_size:"<<points.size()<<endl;
    int size = points.size();
    double l = this->length;

    int seg_count = floor(l/thres);

    if(seg_count==1||seg_count==0)
    {
        Branch a;
        a.head_point = this->head_point;
        a.end_point = points[size/2];
        a.parent = this->parent;
        segs.push_back(a);
        Branch b;
        b.head_point = points[size/2];
        b.end_point = this->end_point;
        b.parent = &segs[0];
        segs.push_back(b);
    }
    else
    {
        for(int i=0; i<seg_count; ++i)
        {
            Branch tmp;
            tmp.head_point = points[(size/seg_count)*i];
            if(i==seg_count-1)
            {
                tmp.end_point = this->end_point;
            }
            else
            {
                tmp.end_point = points[(size/seg_count)*(i+1)];
            }

            if(i==0)
            {
                tmp.parent = this->parent;
            }
            else
            {
                tmp.parent = &segs[i-1];
            }
            segs.push_back(tmp);
        }
    }

    return true;

}
bool Branch::refine_branch_by_gd(LocationSimple points, LocationSimple p_childa, LocationSimple p_childb,NeuronTree &nt_gd, QString braindir, V3DPluginCallback2 &callback)
{
    QDir all_braindir(braindir);
    QFileInfoList list_braindir = all_braindir.entryInfoList(QStringList(),QDir::Dirs|QDir::NoDotAndDotDot);
    int dir_count = list_braindir.size();
    double x_max=max(max(points.x,p_childa.x),p_childb.x);
    double y_max=max(max(points.y,p_childa.y),p_childb.y);
    double z_max=max(max(points.z,p_childa.z),p_childb.z);
    double x_min=min(min(points.x,p_childa.x),p_childb.x);
    double y_min=min(min(points.y,p_childa.y),p_childb.y);
    double z_min=min(min(points.z,p_childa.z),p_childb.z);
    map<int,int> sizemap;

    for(int i=0; i<dir_count; ++i)
    {
//        qDebug()<<i<<": "<<list_braindir[i].absoluteFilePath();
        QString t = list_braindir[i].baseName();
        QStringList ts =t.split('x');
        int resolution = ts[1].toInt();
        sizemap[resolution] = i;
    }

    QVector<QFileInfo> vtmp;
    map<int,int>::iterator it =sizemap.begin();

    while(it!=sizemap.end())
    {
        int index = it->second;
        vtmp.push_back(list_braindir[index]);
        it++;
    }

    list_braindir.clear();
    for(int i=0; i<dir_count; ++i)
    {
        list_braindir.push_back(vtmp[i]);
//        qDebug()<<list_braindir[i].absoluteFilePath();
    }

    QString current_braindir = list_braindir[dir_count-1].absoluteFilePath();

    qDebug()<<current_braindir;

//    qDebug()<<"points size: "<<points.size();
    V3DLONG *in_zz=0;
    callback.getDimTeraFly(current_braindir.toStdString().c_str(),in_zz);
//    NeuronTree t;
//    for(int i=0; i<points.size(); ++i)
//    {
//        NeuronSWC tmpswc;
//        tmpswc.x = points[i].x;
//        tmpswc.y = points[i].y;
//        tmpswc.z = points[i].z;
//        tmpswc.n = i+1;
//        tmpswc.parent = (i==0) ? -1 : i;
//        t.listNeuron.push_back(tmpswc);
//    }
//    int mode = 1;
//    double branchmean,branchstd;
//    this->get_meanstd(braindir,callback,t,branchmean,branchstd,mode);

//    if(branchmean<30)
//    {
//        outpoints.assign(points.begin(),points.end());
//    }
//    else
//    {
        size_t x0 = IN, x1 = 0, y0 = IN, y1 = 0, z0 = IN, z1 = 0;
//        for(V3DLONG j=0; j<points.size(); ++j)
//        {
//            x0 = (x0>points[j].x) ? points[j].x : x0;
//            x1 = (x1<points[j].x) ? points[j].x : x1;
//            y0 = (y0>points[j].y) ? points[j].y : y0;
//            y1 = (y1<points[j].y) ? points[j].y : y1;
//            z0 = (z0>points[j].z) ? points[j].z : z0;
//            z1 = (z1<points[j].z) ? points[j].z : z1;
//        }
        x0 =x_min-10, x1 =x_max+10, y0 =y_min-10, y1 =y_max+10, z0 =z_min-10, z1 =z_max+10;
        if(x0<=0){x0=1;}
        if(x1>=in_zz[0]){x1=in_zz[0]-1;}
        if(y0<=0){y0=1;}
        if(y1>=in_zz[1]){y1=in_zz[1]-1;}
        if(z0<=0){z0=1;}
        if(z1>=in_zz[2]){z1=in_zz[2]-1;}
        qDebug()<<"subvolume: "<<x0<<" "<<x1<<" "<<y0<<" "<<y1<<" "<<z0<<" "<<z1;

        V3DLONG sz0 = x1 - x0, sz1 = y1 - y0, sz2 = z1 - z0;
        V3DLONG sz[4] = {sz0,sz1,sz2,1};

//        NeuronTree orig_nt;
//        for(V3DLONG j=0; j<points.size(); ++j)
//        {
//            points[j].x -= x0;
//            points[j].y -= y0;
//            points[j].z -= z0;
//            NeuronSWC p;
//            p.x = points[j].x;
//            p.y = points[j].y;
//            p.z = points[j].z;
//            p.n = j+1;
//            p.parent = (j==1) ? -1 : j;
//            orig_nt.listNeuron.push_back(p);
//        }

        unsigned char* pdata = 0;
        pdata = callback.getSubVolumeTeraFly(current_braindir.toStdString().c_str(),x0,x1,y0,y1,z0,z1);

        unsigned char* data1d_mask = 0;
        V3DLONG sz_num = sz0*sz1*sz2;
        data1d_mask = new unsigned char[sz_num];
        memset(data1d_mask,0,sz_num*sizeof(unsigned char));
//        double margin = 5;
//        ComputemaskImage(orig_nt,data1d_mask,sz0,sz1,sz2,margin);

//        for(V3DLONG i=0; i<sz_num; ++i)
//        {
//            if(data1d_mask[i]==0 && (double)pdata[i]>branchmean)
//            {
//                pdata[i] = 0;
//            }
//        }

        unsigned char**** p4d = 0;
        if(!new4dpointer(p4d,sz[0],sz[1],sz[2],sz[3],pdata))
        {
            fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
            if(p4d) {delete[] p4d; p4d = 0;}
            return false;
        }

        LocationSimple p0(points.x-x0,points.y-y0,points.z-z0);
       // qDebug()<<"poins size: "<<points.size()<<"???????????????????????????????????";
        vector<LocationSimple> pp;
        pp.clear();
        LocationSimple pend1(p_childa.x-x0,p_childa.y-y0,p_childa.z-z0);
        pp.push_back(pend1);
        LocationSimple pend2(p_childb.x-x0,p_childb.y-y0,p_childb.z-z0);
        pp.push_back(pend2);

        qDebug()<<"start point: "<<p0.x<<" "<<p0.y<<" "<<p0.z<<endl;
        qDebug()<<"end point1: "<<pp[0].x<<" "<<pp[0].y<<" "<<pp[0].z<<endl;
        qDebug()<<"end point2: "<<pp[1].x<<" "<<pp[1].y<<" "<<pp[1].z<<endl;
        double weight_xy_z = 1.0;
        CurveTracePara trace_para;
        trace_para.channo = 0;
        trace_para.sp_graph_background = 0;
        trace_para.b_postMergeClosebyBranches = false;
        trace_para.b_3dcurve_width_from_xyonly = false;
        trace_para.b_pruneArtifactBranches = false;
        trace_para.sp_num_end_nodes = 2;
        trace_para.b_deformcurve = false;
        trace_para.sp_graph_resolution_step = 1;
        trace_para.b_estRadii = false;

        qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

        nt_gd = v3dneuron_GD_tracing(p4d,sz,p0,pp,trace_para,weight_xy_z);
        qDebug()<<"--------------------------------------------------------------------------------------";

        for(int i=0; i<nt_gd.listNeuron.size(); ++i)
        {
    //        qDebug()<<__LINE__<<"i: "<<i<<" n: "<<nt_gd.listNeuron[i].n<<" p: "<<nt_gd.listNeuron[i].parent;
//            LocationSimple local((nt_gd.listNeuron[i].x+x0),(nt_gd.listNeuron[i].y+y0),(nt_gd.listNeuron[i].z+z0));
//            outpoints.push_back(local);
            nt_gd.listNeuron[i].x += x0;
            nt_gd.listNeuron[i].y += y0;
            nt_gd.listNeuron[i].z += z0;
        }
//        reverse(outpoints.begin(),outpoints.end());
//    }

    return true;
}
bool Branch::refine_by_gd(vector<LocationSimple> points, vector<LocationSimple> &outpoints, QString braindir, V3DPluginCallback2 &callback)
{
    QDir all_braindir(braindir);
    QFileInfoList list_braindir = all_braindir.entryInfoList(QStringList(),QDir::Dirs|QDir::NoDotAndDotDot);
    int dir_count = list_braindir.size();

    map<int,int> sizemap;

    for(int i=0; i<dir_count; ++i)
    {
//        qDebug()<<i<<": "<<list_braindir[i].absoluteFilePath();
        QString t = list_braindir[i].baseName();
        QStringList ts =t.split('x');
        int resolution = ts[1].toInt();
        sizemap[resolution] = i;
    }

    QVector<QFileInfo> vtmp;
    map<int,int>::iterator it =sizemap.begin();

    while(it!=sizemap.end())
    {
        int index = it->second;
        vtmp.push_back(list_braindir[index]);
        it++;
    }

    list_braindir.clear();
    for(int i=0; i<dir_count; ++i)
    {
        list_braindir.push_back(vtmp[i]);
//        qDebug()<<list_braindir[i].absoluteFilePath();
    }

    QString current_braindir = list_braindir[dir_count-1].absoluteFilePath();

    qDebug()<<current_braindir;

    qDebug()<<"points size: "<<points.size();
    V3DLONG *in_zz=0;
    callback.getDimTeraFly(current_braindir.toStdString().c_str(),in_zz);
    NeuronTree t;
    for(int i=0; i<points.size(); ++i)
    {
        NeuronSWC tmpswc;
        tmpswc.x = points[i].x;
        tmpswc.y = points[i].y;
        tmpswc.z = points[i].z;
        tmpswc.n = i+1;
        tmpswc.parent = (i==0) ? -1 : i;
        t.listNeuron.push_back(tmpswc);
    }
    int mode = 1;
    double branchmean,branchstd;
    this->get_meanstd(braindir,callback,t,branchmean,branchstd,mode);

    if(branchmean<30)
    {
        outpoints.assign(points.begin(),points.end());
    }
    else
    {
        size_t x0 = IN, x1 = 0, y0 = IN, y1 = 0, z0 = IN, z1 = 0;
        for(V3DLONG j=0; j<points.size(); ++j)
        {
            x0 = (x0>points[j].x) ? points[j].x : x0;
            x1 = (x1<points[j].x) ? points[j].x : x1;
            y0 = (y0>points[j].y) ? points[j].y : y0;
            y1 = (y1<points[j].y) ? points[j].y : y1;
            z0 = (z0>points[j].z) ? points[j].z : z0;
            z1 = (z1<points[j].z) ? points[j].z : z1;
        }
        x0 -= 10, x1 += 10, y0 -= 10, y1 += 10, z0 -= 10, z1 += 10;
        if(x0<=0){x0=1;}
        if(x1>=in_zz[0]){x1=in_zz[0]-1;}
        if(y0<=0){y0=1;}
        if(y1>=in_zz[1]){y1=in_zz[1]-1;}
        if(z0<=0){z0=1;}
        if(z1>=in_zz[2]){z1=in_zz[2]-1;}
        qDebug()<<"subvolume: "<<x0<<" "<<x1<<" "<<y0<<" "<<y1<<" "<<z0<<" "<<z1;

        V3DLONG sz0 = x1 - x0, sz1 = y1 - y0, sz2 = z1 - z0;
        V3DLONG sz[4] = {sz0,sz1,sz2,1};

        NeuronTree orig_nt;
        for(V3DLONG j=0; j<points.size(); ++j)
        {
            points[j].x -= x0;
            points[j].y -= y0;
            points[j].z -= z0;
            NeuronSWC p;
            p.x = points[j].x;
            p.y = points[j].y;
            p.z = points[j].z;
            p.n = j+1;
            p.parent = (j==1) ? -1 : j;
            orig_nt.listNeuron.push_back(p);
        }

        unsigned char* pdata = 0;
        pdata = callback.getSubVolumeTeraFly(current_braindir.toStdString().c_str(),x0,x1,y0,y1,z0,z1);

        unsigned char* data1d_mask = 0;
        V3DLONG sz_num = sz0*sz1*sz2;
        data1d_mask = new unsigned char[sz_num];
        memset(data1d_mask,0,sz_num*sizeof(unsigned char));
        double margin = 5;
        ComputemaskImage(orig_nt,data1d_mask,sz0,sz1,sz2,margin);

        for(V3DLONG i=0; i<sz_num; ++i)
        {
            if(data1d_mask[i]==0 && (double)pdata[i]>branchmean)
            {
                pdata[i] = 0;
            }
        }

        unsigned char**** p4d = 0;
        if(!new4dpointer(p4d,sz[0],sz[1],sz[2],sz[3],pdata))
        {
            fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
            if(p4d) {delete[] p4d; p4d = 0;}
            return false;
        }

        LocationSimple p0(points[0].x,points[0].y,points[0].z);
        qDebug()<<"poins size: "<<points.size()<<"???????????????????????????????????";
        vector<LocationSimple> pp;
        pp.clear();
        LocationSimple pend((points.end()-1)->x,(points.end()-1)->y,(points.end()-1)->z);
        pp.push_back(pend);

        qDebug()<<"start point: "<<p0.x<<" "<<p0.y<<" "<<p0.z<<" end point: "<<pend.x<<" "<<pend.y<<" "<<pend.z;

        double weight_xy_z = 1.0;
        CurveTracePara trace_para;
        trace_para.channo = 0;
        trace_para.sp_graph_background = 0;
        trace_para.b_postMergeClosebyBranches = false;
        trace_para.b_3dcurve_width_from_xyonly = false;
        trace_para.b_pruneArtifactBranches = false;
        trace_para.sp_num_end_nodes = 2;
        trace_para.b_deformcurve = false;
        trace_para.sp_graph_resolution_step = 1;
        trace_para.b_estRadii = false;

        qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

        NeuronTree nt_gd = v3dneuron_GD_tracing(p4d,sz,p0,pp,trace_para,weight_xy_z);
        qDebug()<<"--------------------------------------------------------------------------------------";

        for(int i=0; i<nt_gd.listNeuron.size(); ++i)
        {
    //        qDebug()<<__LINE__<<"i: "<<i<<" n: "<<nt_gd.listNeuron[i].n<<" p: "<<nt_gd.listNeuron[i].parent;
            LocationSimple local((nt_gd.listNeuron[i].x+x0),(nt_gd.listNeuron[i].y+y0),(nt_gd.listNeuron[i].z+z0));
            outpoints.push_back(local);
        }
        reverse(outpoints.begin(),outpoints.end());
    }



    return true;
}
bool Branch::refine_by_gd_img(vector<LocationSimple> points, vector<LocationSimple> &outpoints, string inimg_file, V3DPluginCallback2 &callback)
{
//    QDir all_braindir(braindir);
//    QFileInfoList list_braindir = all_braindir.entryInfoList(QStringList(),QDir::Dirs|QDir::NoDotAndDotDot);
//    int dir_count = list_braindir.size();

//    map<int,int> sizemap;

//    for(int i=0; i<dir_count; ++i)
//    {
//       qDebug()<<i<<": "<<list_braindir[i].absoluteFilePath();
//        QString t = list_braindir[i].baseName();
//        QStringList ts =t.split('x');
//        int resolution = ts[1].toInt();
//        sizemap[resolution] = i;
//    }

//    QVector<QFileInfo> vtmp;
//    map<int,int>::iterator it =sizemap.begin();

//    while(it!=sizemap.end())
//    {
//        int index = it->second;
//        vtmp.push_back(list_braindir[index]);
//        it++;
//    }

//    list_braindir.clear();
//    for(int i=0; i<dir_count; ++i)
//    {
//        list_braindir.push_back(vtmp[i]);
//        qDebug()<<list_braindir[i].absoluteFilePath();
//    }

//    QString current_braindir = list_braindir[dir_count-1].absoluteFilePath();

//    qDebug()<<current_braindir;

    qDebug()<<"points size: "<<points.size();
    V3DLONG in_zz[4];
    //callback.getDimTeraFly(inimg_file,in_zz);
    unsigned char* pdata = 0;
//    pdata = callback.getSubVolumeTeraFly(inimg_file,0,in_zz[0]-1,0,in_zz[1]-1,0,in_zz[2]-1);
    int datatype;
    simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), pdata, in_zz, datatype);
    NeuronTree t;
    for(int i=0; i<points.size(); ++i)
    {
        NeuronSWC tmpswc;
        tmpswc.x = points[i].x;
        tmpswc.y = points[i].y;
        tmpswc.z = points[i].z;
        tmpswc.n = i+1;
        tmpswc.parent = (i==0) ? -1 : i;
        t.listNeuron.push_back(tmpswc);
    }
    int mode = 1;
    double branchmean,branchstd;
    this->get_meanstd_img(inimg_file,callback,t,branchmean,branchstd,mode);
//     branchmean=255;
//    if(branchmean<30)
//    {
//        qDebug()<<"branchmean<30"<<endl;
//        outpoints.assign(points.begin(),points.end());
//    }
//    else
//    {
//        size_t x0 = IN, x1 = 0, y0 = IN, y1 = 0, z0 = IN, z1 = 0;
//        for(V3DLONG j=0; j<points.size(); ++j)
//        {
//            x0 = (x0>points[j].x) ? points[j].x : x0;
//            x1 = (x1<points[j].x) ? points[j].x : x1;
//            y0 = (y0>points[j].y) ? points[j].y : y0;
//            y1 = (y1<points[j].y) ? points[j].y : y1;
//            z0 = (z0>points[j].z) ? points[j].z : z0;
//            z1 = (z1<points[j].z) ? points[j].z : z1;
//        }
//        x0 -= 50, x1 += 50, y0 -= 50, y1 += 50, z0 -= 50, z1 += 50;
//        if(x0<=0){x0=1;}
//        if(x1>=in_zz[0]){x1=in_zz[0]-1;}
//        if(y0<=0){y0=1;}
//        if(y1>=in_zz[1]){y1=in_zz[1]-1;}
//        if(z0<=0){z0=1;}
//        if(z1>=in_zz[2]){z1=in_zz[2]-1;}
//        qDebug()<<"subvolume: "<<x0<<" "<<x1<<" "<<y0<<" "<<y1<<" "<<z0<<" "<<z1;

       //V3DLONG sz0 = x1-x0, sz1 = y1-y0, sz2 = z1-z0;
        V3DLONG sz0 = in_zz[0], sz1 = in_zz[1], sz2 = in_zz[2];
        V3DLONG sz[4] = {sz0,sz1,sz2,1};

        NeuronTree orig_nt;
        for(V3DLONG j=0; j<points.size(); ++j)
        {
//            points[j].x -= x0;
//            points[j].y -= y0;
//            points[j].z -= z0;
            NeuronSWC p;
            p.x = points[j].x;
            p.y = points[j].y;
            p.z = points[j].z;
            p.n = j+1;
            p.parent = (j==1) ? -1 : j;
            orig_nt.listNeuron.push_back(p);
        }

//        unsigned char* pdata = 0;
//        pdata = callback.getSubVolumeTeraFly(inimg_file,x0,x1,y0,y1,z0,z1);

        unsigned char* data1d_mask = 0;
        V3DLONG sz_num = sz0*sz1*sz2;
        data1d_mask = new unsigned char[sz_num];
        memset(data1d_mask,0,sz_num*sizeof(unsigned char));
        double margin = 5;
        ComputemaskImage(orig_nt,data1d_mask,sz0,sz1,sz2,margin);

//        for(V3DLONG i=0; i<sz_num; ++i)
//        {
//            if(data1d_mask[i]==0 && (double)pdata[i]>branchmean)
//            {
//                pdata[i] = 0;
//            }
//        }

        unsigned char**** p4d = 0;
        if(!new4dpointer(p4d,sz0,sz1,sz2,1,pdata))
        {
            fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
            if(p4d) {delete[] p4d; p4d = 0;}
            return false;
        }

        LocationSimple p0(points[0].x,points[0].y,points[0].z);
        qDebug()<<"poins size: "<<points.size()<<"???????????????????????????????????";
        vector<LocationSimple> pp;
        pp.clear();
        LocationSimple pend((points.end()-1)->x,(points.end()-1)->y,(points.end()-1)->z);
        pp.push_back(pend);

        qDebug()<<"start point: "<<p0.x<<" "<<p0.y<<" "<<p0.z<<" end point: "<<pend.x<<" "<<pend.y<<" "<<pend.z;

        double weight_xy_z = 1.0;
        CurveTracePara trace_para;
        trace_para.channo = 0;
        trace_para.sp_graph_background = 0;
        trace_para.b_postMergeClosebyBranches = false;
        trace_para.b_3dcurve_width_from_xyonly = false;
        trace_para.b_pruneArtifactBranches = false;
        trace_para.sp_num_end_nodes = 2;
        trace_para.b_deformcurve = false;
        trace_para.sp_graph_resolution_step = 1;
        trace_para.b_estRadii = false;

        qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

        NeuronTree nt_gd = v3dneuron_GD_tracing(p4d,sz,p0,pp,trace_para,weight_xy_z);
        qDebug()<<"--------------------------------------------------------------------------------------";

        for(int i=0; i<nt_gd.listNeuron.size(); ++i)
        {
    //        qDebug()<<__LINE__<<"i: "<<i<<" n: "<<nt_gd.listNeuron[i].n<<" p: "<<nt_gd.listNeuron[i].parent;
            LocationSimple local((nt_gd.listNeuron[i].x),(nt_gd.listNeuron[i].y),(nt_gd.listNeuron[i].z));
            outpoints.push_back(local);
        }
        reverse(outpoints.begin(),outpoints.end());
//    }



    return true;
}
bool Branch::refine_by_2gd(vector<LocationSimple> &outbranch, QString braindir, V3DPluginCallback2 &callback, NeuronTree &nt, double thres,vector<int> &neuron_type)
{
    vector<Branch> segs;
    this->splitbranch(nt,segs,thres);
    int seg_count = segs.size();
    qDebug()<<__LINE__<<"seg size: "<<seg_count;
//    vector<LocationSimple> tmp;
    vector<int> indexs_of_gd2;
    vector<int> neuron_type1;
//    vector<NeuronSWC> points1;
//    segs[0].get_points_of_branch(points1,nt);
    indexs_of_gd2.push_back(0);
//    neuron_type1.push_back(points1[0].type);
    neuron_type1.push_back(2);
    vector<LocationSimple> gd1_points;

    for(int i=0; i<seg_count; ++i)
    {
        vector<NeuronSWC> points;
        segs[i].get_points_of_branch(points,nt);
        vector<LocationSimple> inpoints;
        int color=2;
        for(int j=0; j<points.size(); ++j)
        {
            LocationSimple local(points[j].x,points[j].y,points[j].z);
            inpoints.push_back(local);
            if(points[j].type==3){color=3;}
        }
        qDebug()<<__LINE__<<"i: "<<i<<" "<<inpoints.size();
        vector<LocationSimple> outpoints;
        this->refine_by_gd(inpoints,outpoints,braindir,callback);       
        qDebug()<<__LINE__<<"i: outpoints: "<<i<<" "<<outpoints.size();

        int index = outpoints.size()/2 + gd1_points.size();
        indexs_of_gd2.push_back(index);
        neuron_type1.push_back(color);
        if(i!=(seg_count-1))
        {
//            if(outpoints.size()==0)
//            {
//                gd1_points.insert(gd1_points.end(),inpoints.begin(),inpoints.end()-1);
//            }
//            else
//            {
//                gd1_points.insert(gd1_points.end(),outpoints.begin(),outpoints.end()-1);
//            }

              if(outpoints.size()>0)
              {
                  gd1_points.insert(gd1_points.end(),outpoints.begin(),outpoints.end()-1);
              }else{
                  gd1_points.insert(gd1_points.end(),inpoints.begin(),inpoints.end()-1);
              }
        }
        else
        {
//            if(outpoints.size()==0)
//            {
//                gd1_points.insert(gd1_points.end(),inpoints.begin(),inpoints.end());
//            }
//            else
//            {
//                gd1_points.insert(gd1_points.end(),outpoints.begin(),outpoints.end());
//            }
            if(outpoints.size()>0)
            {
                gd1_points.insert(gd1_points.end(),outpoints.begin(),outpoints.end());
            }else{
                gd1_points.insert(gd1_points.end(),inpoints.begin(),inpoints.end());
            }
        }
    }

    qDebug()<<__LINE__<<"index_of_gd size: "<<indexs_of_gd2.size();

    for(int i=0; i<indexs_of_gd2.size(); ++i)
    {
        vector<LocationSimple> inpoints;
        vector<LocationSimple> outpoints;
        int color;
        if(i!=(indexs_of_gd2.size()-1))
        {
            inpoints.assign(gd1_points.begin()+indexs_of_gd2[i],gd1_points.begin()+indexs_of_gd2[i+1]);
            if(neuron_type1[i]!=neuron_type1[i+1])
            {
                color=3;
            }
//            else
//            {
//                color=neuron_type1[i];
//            }
        }
        else
        {
            inpoints.assign(gd1_points.begin()+indexs_of_gd2[i],gd1_points.end());
            color=neuron_type1[i];
        }
        qDebug()<<__LINE__<<"i: "<<i<<" "<<inpoints.size();
        if(inpoints.size()==0){
            vector<NeuronSWC> points;
            vector<LocationSimple> rawpoints;
            this->get_points_of_branch(points,nt);
            for(int k=0; k<points.size(); ++k)
            {
                LocationSimple local(points[k].x,points[k].y,points[k].z);
                rawpoints.push_back(local);
                neuron_type.push_back(points[k].type);
            }
            for(int k=0;k<rawpoints.size(); ++k){
                outbranch.push_back(rawpoints[k]);
            }
            return true;
        }
        this->refine_by_gd(inpoints,outpoints,braindir,callback);
        qDebug()<<__LINE__<<"i: outpoints: "<<i<<" "<<outpoints.size();
        if(i!=(indexs_of_gd2.size()-1))
        {
//            if(outpoints.size()==0)
//            {
//                outbranch.insert(outbranch.end(),inpoints.begin(),inpoints.end()-1);
//            }
//            else
//            {
//                outbranch.insert(outbranch.end(),outpoints.begin(),outpoints.end()-1);
//            }
            if(outpoints.size()>0)
            {
                outbranch.insert(outbranch.end(),outpoints.begin(),outpoints.end()-1);
                if(color!=3){color=2;}
                vector<int> tt(outpoints.size(), color);
                neuron_type.insert(neuron_type.end(),tt.begin(),tt.end()-1);
            }
        }
        else
        {
//            if(outpoints.size()==0)
//            {
//                outbranch.insert(outbranch.end(),inpoints.begin(),inpoints.end());
//            }
//            else
//            {
//                outbranch.insert(outbranch.end(),outpoints.begin(),outpoints.end());
//            }
            if(outpoints.size()>0)
            {
                outbranch.insert(outbranch.end(),outpoints.begin(),outpoints.end());
                if(color!=3){color=2;}
                vector<int> tt(outpoints.size(), color);
                neuron_type.insert(neuron_type.end(),tt.begin(),tt.end());
            }
        }
    }
//    outbranch.push_back(tmp[0]);
//    if(outbranch.size()>=2)
//    {
//        for(int k=1;k<outbranch.size()-1;++k)
//        {
//              float x0=outbranch[k-1].x;
//              float y0=outbranch[k-1].y;
//              float z0=outbranch[k-1].z;
//              float x1=outbranch[k].x;
//              float y1=outbranch[k].y;
//              float z1=outbranch[k].z;
//              float x2=outbranch[k+1].x;
//              float y2=outbranch[k+1].y;
//              float z2=outbranch[k+1].z;
//              float x3=(x0+x2)/2;
//              float y3=(y0+y2)/2;
//              float z3=(z0+z2)/2;
//              if((abs(x3-x1)>=1)&&(abs(y3-y1)>=1)&&(abs(z3-z1)>=1))
//              {
//                  tmp.push_back(outbranch[k]);
//              }
//        }
//    }
    return true;

}

bool Branch::refine_by_2gd_img(vector<LocationSimple> &outbranch, string inimg_file, V3DPluginCallback2 &callback, NeuronTree &nt, double thres,vector<int> &neuron_type)
{
    vector<Branch> segs;
    this->splitbranch(nt,segs,thres);
    int seg_count = segs.size();
    qDebug()<<__LINE__<<"seg size: "<<seg_count;
//    vector<LocationSimple> tmp;
    vector<int> indexs_of_gd2;
    vector<int> neuron_type1;
//    vector<NeuronSWC> points1;
//    segs[0].get_points_of_branch(points1,nt);dd
    indexs_of_gd2.push_back(0);
//    neuron_type1.push_back(points1[0].type);
    neuron_type1.push_back(2);
    vector<LocationSimple> gd1_points;

    for(int i=0; i<seg_count; ++i)
    {
        vector<NeuronSWC> points;
        segs[i].get_points_of_branch(points,nt);
        vector<LocationSimple> inpoints;
        int color=2;
        for(int j=0; j<points.size(); ++j)
        {
            LocationSimple local(points[j].x,points[j].y,points[j].z);
            inpoints.push_back(local);
            if(points[j].type==3){color=3;}
        }
        qDebug()<<__LINE__<<"i: "<<i<<" "<<inpoints.size();
        vector<LocationSimple> outpoints;
        this->refine_by_gd_img(inpoints,outpoints,inimg_file,callback);
        qDebug()<<__LINE__<<"i: outpoints: "<<i<<" "<<outpoints.size();

        int index = outpoints.size()/2 + gd1_points.size();
        indexs_of_gd2.push_back(index);
        neuron_type1.push_back(color);
        if(i!=(seg_count-1))
        {
//            if(outpoints.size()==0)
//            {
//                gd1_points.insert(gd1_points.end(),inpoints.begin(),inpoints.end()-1);
//            }
//            else
//            {
//                gd1_points.insert(gd1_points.end(),outpoints.begin(),outpoints.end()-1);
//            }

              if(outpoints.size()>0)
              {
                  gd1_points.insert(gd1_points.end(),outpoints.begin(),outpoints.end()-1);
              }else{
                  gd1_points.insert(gd1_points.end(),inpoints.begin(),inpoints.end()-1);
              }
        }
        else
        {
//            if(outpoints.size()==0)
//            {
//                gd1_points.insert(gd1_points.end(),inpoints.begin(),inpoints.end());
//            }
//            else
//            {
//                gd1_points.insert(gd1_points.end(),outpoints.begin(),outpoints.end());
//            }
            if(outpoints.size()>0)
            {
                gd1_points.insert(gd1_points.end(),outpoints.begin(),outpoints.end());
            }else{
                gd1_points.insert(gd1_points.end(),inpoints.begin(),inpoints.end());
            }
        }
    }

    qDebug()<<__LINE__<<"index_of_gd size: "<<indexs_of_gd2.size();

    for(int i=0; i<indexs_of_gd2.size(); ++i)
    {
        vector<LocationSimple> inpoints;
        vector<LocationSimple> outpoints;
        int color;
        if(i!=(indexs_of_gd2.size()-1))
        {
            inpoints.assign(gd1_points.begin()+indexs_of_gd2[i],gd1_points.begin()+indexs_of_gd2[i+1]);
            if(neuron_type1[i]!=neuron_type1[i+1])
            {
                color=3;
            }
//            else
//            {
//                color=neuron_type1[i];
//            }
        }
        else
        {
            inpoints.assign(gd1_points.begin()+indexs_of_gd2[i],gd1_points.end());
            color=neuron_type1[i];
        }
        qDebug()<<__LINE__<<"i: "<<i<<" "<<inpoints.size();
        if(inpoints.size()==0){
            vector<NeuronSWC> points;
            vector<LocationSimple> rawpoints;
            this->get_points_of_branch(points,nt);
            for(int k=0; k<points.size(); ++k)
            {
                LocationSimple local(points[k].x,points[k].y,points[k].z);
                rawpoints.push_back(local);
                neuron_type.push_back(points[k].type);
            }
            for(int k=0;k<rawpoints.size(); ++k){
                outbranch.push_back(rawpoints[k]);
            }
            return true;
        }
        this->refine_by_gd_img(inpoints,outpoints,inimg_file,callback);
        qDebug()<<__LINE__<<"i: outpoints: "<<i<<" "<<outpoints.size();
        if(i!=(indexs_of_gd2.size()-1))
        {
//            if(outpoints.size()==0)
//            {
//                outbranch.insert(outbranch.end(),inpoints.begin(),inpoints.end()-1);
//            }
//            else
//            {
//                outbranch.insert(outbranch.end(),outpoints.begin(),outpoints.end()-1);
//            }
            if(outpoints.size()>0)
            {
                outbranch.insert(outbranch.end(),outpoints.begin(),outpoints.end()-1);
                if(color!=3){color=2;}
                vector<int> tt(outpoints.size(), color);
                neuron_type.insert(neuron_type.end(),tt.begin(),tt.end()-1);
            }
        }
        else
        {
//            if(outpoints.size()==0)
//            {
//                outbranch.insert(outbranch.end(),inpoints.begin(),inpoints.end());
//            }
//            else
//            {
//                outbranch.insert(outbranch.end(),outpoints.begin(),outpoints.end());
//            }
            if(outpoints.size()>0)
            {
                outbranch.insert(outbranch.end(),outpoints.begin(),outpoints.end());
                if(color!=3){color=2;}
                vector<int> tt(outpoints.size(), color);
                neuron_type.insert(neuron_type.end(),tt.begin(),tt.end());
            }
        }
    }
//    outbranch.push_back(tmp[0]);
//    if(outbranch.size()>=2)
//    {
//        for(int k=1;k<outbranch.size()-1;++k)
//        {
//              float x0=outbranch[k-1].x;
//              float y0=outbranch[k-1].y;
//              float z0=outbranch[k-1].z;
//              float x1=outbranch[k].x;
//              float y1=outbranch[k].y;
//              float z1=outbranch[k].z;
//              float x2=outbranch[k+1].x;
//              float y2=outbranch[k+1].y;
//              float z2=outbranch[k+1].z;
//              float x3=(x0+x2)/2;
//              float y3=(y0+y2)/2;
//              float z3=(z0+z2)/2;
//              if((abs(x3-x1)>=1)&&(abs(y3-y1)>=1)&&(abs(z3-z1)>=1))
//              {
//                  tmp.push_back(outbranch[k]);
//              }
//        }
//    }
    return true;

}




bool SwcTree::initialize(NeuronTree t)
{
    nt.deepCopy(t);

    NeuronSWC ori;

    V3DLONG num_p=nt.listNeuron.size();
    vector<vector<V3DLONG> > children=vector<vector<V3DLONG> >(num_p,vector<V3DLONG>());
    for(V3DLONG i=0;i<num_p;++i)
    {
        V3DLONG par=nt.listNeuron[i].parent;
        if(par<0)
        {
            ori=nt.listNeuron[i];
            qDebug()<<"root id: "<<nt.listNeuron[i].n;
            continue;
        }
        children[nt.hashNeuron.value(par)].push_back(i);
    }

    vector<NeuronSWC> queue;
    queue.push_back(ori);

    //initial head_point,end_point,distance,length
    cout<<"initial head_point,end_point,distance,length"<<endl;
    while(!queue.empty())
    {
        NeuronSWC tmp=queue.front();
        queue.erase(queue.begin());
        for(int i=0;i<children[nt.hashNeuron.value(tmp.n)].size();++i)
        {
            Branch branch;
            //branch.head_point=new NeuronSWC;
            branch.head_point=tmp;
            NeuronSWC child=nt.listNeuron[children[nt.hashNeuron.value(branch.head_point.n)][i]];
            branch.length+=distance_two_point(tmp,child);
            Angle near_point_angle = Angle(child.x-tmp.x,child.y-tmp.y,child.z-tmp.z);
            near_point_angle.norm_angle();
            double sum_angle = 0;
            while(children[nt.hashNeuron.value(child.n)].size()==1/*&&branch.length<100*/)
            {
                NeuronSWC par=child;
                child=nt.listNeuron[children[nt.hashNeuron.value(par.n)][0]];
                branch.length+=distance_two_point(par,child);
                Angle next_point_angle = Angle(child.x-par.x,child.y-par.y,child.z-par.z);
                next_point_angle.norm_angle();
                sum_angle += acos(near_point_angle*next_point_angle);
//                if(sum_angle/branch.length>0.098)
//                {
//                    break;
//                }
//                else
//                {
//                    near_point_angle = next_point_angle;
//                }
            }
            if(children[nt.hashNeuron.value(child.n)].size()>=1)
            {
                queue.push_back(child);
            }
            //branch.end_point=new NeuronSWC;
            branch.end_point=child;
            branch.distance=branch.get_distance();
            branch.sum_angle = sum_angle;
            branchs.push_back(branch);
        }
    }

    //initial head_angle,end_angle
    cout<<"initial head_angle,end_angle"<<endl;
    for(int i=0;i<branchs.size();++i)
    {
        vector<NeuronSWC> points;
        branchs[i].get_points_of_branch(points,nt);
        double length=0;
        NeuronSWC par0=points[0];
        for(int j=1;j<points.size();++j)
        {
            NeuronSWC child0=points[j];
            length+=distance_two_point(par0,child0);
            if(length>5)
            {
                //branchs[i].head_angle.x_angle=x_angle_two_point(points[0],child0);
                //branchs[i].head_angle.z_angle=z_angle_two_point(points[0],child0);

                branchs[i].head_angle.x=child0.x-points[0].x;
                branchs[i].head_angle.y=child0.y-points[0].y;
                branchs[i].head_angle.z=child0.z-points[0].z;

                //double d=sqrt(branchs[i].head_angle.x*branchs[i].head_angle.x+branchs[i].head_angle.y*branchs[i].head_angle.y+branchs[i].head_angle.z*branchs[i].head_angle.z);
                double d=norm_v(branchs[i].head_angle);

                branchs[i].head_angle.x/=d;
                branchs[i].head_angle.y/=d;
                branchs[i].head_angle.z/=d;

                break;
            }
            par0=child0;
        }
        if(length<=5)
        {
            branchs[i].head_angle.x=par0.x-points[0].x;
            branchs[i].head_angle.y=par0.y-points[0].y;
            branchs[i].head_angle.z=par0.z-points[0].z;

            //double d=sqrt(branchs[i].head_angle.x*branchs[i].head_angle.x+branchs[i].head_angle.y*branchs[i].head_angle.y+branchs[i].head_angle.z*branchs[i].head_angle.z);
            double d=norm_v(branchs[i].head_angle);

            branchs[i].head_angle.x/=d;
            branchs[i].head_angle.y/=d;
            branchs[i].head_angle.z/=d;
        }

        length=0;
        points.clear();
        branchs[i].get_r_points_of_branch(points,nt);
        NeuronSWC child1=points[0];
        for(int j=1;j<points.size();++j)
        {
            NeuronSWC par1=points[j];
            length+=distance_two_point(par1,child1);
            if(length>5)
            {
                branchs[i].end_angle.x=par1.x-points[0].x;
                branchs[i].end_angle.y=par1.y-points[0].y;
                branchs[i].end_angle.z=par1.z-points[0].z;

                //double d=sqrt(branchs[i].end_angle.x*branchs[i].end_angle.x+branchs[i].end_angle.y*branchs[i].end_angle.y+branchs[i].end_angle.z*branchs[i].end_angle.z);
                double d=norm_v(branchs[i].end_angle);

                branchs[i].end_angle.x/=d;
                branchs[i].end_angle.y/=d;
                branchs[i].end_angle.z/=d;

                break;
            }
            child1=par1;
        }
        if(length<=5)
        {
            branchs[i].end_angle.x=child1.x-points[0].x;
            branchs[i].end_angle.y=child1.y-points[0].y;
            branchs[i].end_angle.z=child1.z-points[0].z;

            //double d=sqrt(branchs[i].end_angle.x*branchs[i].end_angle.x+branchs[i].end_angle.y*branchs[i].end_angle.y+branchs[i].end_angle.z*branchs[i].end_angle.z);
            double d=norm_v(branchs[i].end_angle);

            branchs[i].end_angle.x/=d;
            branchs[i].end_angle.y/=d;
            branchs[i].end_angle.z/=d;
        }
    }

    //initial parent
    cout<<"initial parent"<<endl;
    for(int i=0;i<branchs.size();++i)
    {
        if(branchs[i].head_point.parent<0)
        {
            branchs[i].parent=0;
        }
        else
        {
            for(int j=0;j<branchs.size();++j)
            {
                if(branchs[i].head_point==branchs[j].end_point)
                {
                    //branchs[i].parent=new Branch;
                    branchs[i].parent=&branchs[j];
                    branchs[j].child_num=branchs[j].child_num+1;
                    if(branchs[j].child_num==2)
                    {
                       branchs[j].child_a=&branchs[i];
                    }
                }
            }
        }
    }

    //initial level
    for(int i=0;i<branchs.size();++i)
    {
        Branch* tmp;
        tmp=&branchs[i];
        int level=0;
        while(tmp->parent!=0)
        {
            level++;
            tmp=tmp->parent;
        }
        branchs[i].level=level;
    }
/*
    int size=branchs.size();

    map<Branch,int> mapbranch;
    for(int i=0;i<size;++i)
    {
        mapbranch[branchs[i]]=i;
    }


    vector<vector<int> > children_b=vector<vector<int> >(size,vector<int>());

    for(int i=0;i<size;++i)
    {
        if(branchs[i].parent==0) continue;
        children_b[mapbranch[*(branchs[i].parent)]].push_back(i);
    }

    vector<int> level0_index;
    this->get_level_index(level0_index,0);
    vector<int> queue_t;
    for(int i=0;i<level0_index.size();++i)
    {
        queue_t.push_back(level0_index[i]);

//        Trunk t;
//        int par_index=level0_index[i];
//        t.branch_index.push_back(par_index);
//        double length=branchs[par_index].length;
//        t.length.push_back(length);
//        t.head_angle=branchs[par_index].head_angle;
//        while(children[par_index].size()!=0)
//        {
//            int child0_index=children[par_index].at(0);
//            int child1_index=children[par_index].at(1);
//            double pc_dot0=branchs[par_index].end_angle*branchs[child0_index].head_angle;
//            double pc_dot1=branchs[par_index].end_angle*branchs[child1_index].head_angle;
//            if(pc_dot0>pc_dot1)
//            {
//                t.branch_index.push_back(child0_index);
//                length+=branchs[child0_index].length;
//                t.length.push_back(length);
//                queue.push_back(child1_index);
//                par_index=child0_index;
//            }
//            else
//            {
//                t.branch_index.push_back(child1_index);
//                length+=branchs[child1_index].length;
//                t.length.push_back(length);
//                queue.push_back(child0_index);
//                par_index=child1_index;
//            }
//        }
//        t.end_angle=branchs[par_index].end_angle;
//        trunks.push_back(t);
    }

    while(!queue_t.empty())
    {
        Trunk t;
        int par_index=queue_t.front();
        queue_t.erase(queue_t.begin());
        t.branch_index.push_back(par_index);
        double length=branchs[par_index].length;
        t.length.push_back(length);
        t.head_angle=branchs[par_index].head_angle;
        while(children_b[par_index].size()!=0)
        {
            int child0_index=children_b[par_index].at(0);
            int child1_index=children_b[par_index].at(1);
            double pc_dot0=branchs[par_index].end_angle*branchs[child0_index].head_angle;
            double pc_dot1=branchs[par_index].end_angle*branchs[child1_index].head_angle;
            if(pc_dot0<pc_dot1)
            {
                t.branch_index.push_back(child0_index);
                length+=branchs[child0_index].length;
                t.length.push_back(length);
                queue_t.push_back(child1_index);
                par_index=child0_index;
            }
            else
            {
                t.branch_index.push_back(child1_index);
                length+=branchs[child1_index].length;
                t.length.push_back(length);
                queue_t.push_back(child0_index);
                par_index=child1_index;
            }
        }
        t.end_angle=branchs[par_index].end_angle;
        trunks.push_back(t);
    }

    for(int i=0;i<trunks.size();++i)
    {
        trunks[i].n=i;
        if(branchs[trunks[i].branch_index[0]].parent==0)
        {
            trunks[i].parent=-1;
            continue;
        }
        int par_index=mapbranch[*branchs[trunks[i].branch_index[0]].parent];
        for(int j=0;j<trunks.size();++j)
        {
            bool stop=false;
            for(int k=0;k<trunks[j].branch_index.size();++k)
            {
                if(par_index==trunks[j].branch_index[k])
                {
                    stop=true;
                    trunks[i].parent=j;
                    break;
                }
            }
            if(stop)
                break;
        }
    }
*/
    // give branch type
    for (int i=0; i<branchs.size(); ++i) {
        branchs[i].get_points_of_branch(branchs[i].allpoints, nt);
        if(branchs[i].allpoints.size()>=2)
        {
            branchs[i].type=branchs[i].allpoints[branchs[i].allpoints.size()-2].type;
        }else
        {
            branchs[i].type=branchs[i].allpoints[0].type;
        }

    }
    for(int i=0;i<branchs.size();++i)
    {
        if(branchs[i].type==2 && branchs[i].allpoints.size()>0)
        {
            int flag=0;
            for(int j=0;j<branchs.size();++j)
            {
                if(branchs[j].type==3 || branchs[j].type==4)
                {
                    for(int i1=0; i1<branchs[i].allpoints.size();++i1)
                    {
                        for(int j1=0; j1<branchs[j].allpoints.size();++j1)
                        {
                            double dis=zx_dist(branchs[i].allpoints[i1],branchs[j].allpoints[j1]);
                            if(dis<=30)
                            {
                                flag=1;
                                break;
                            }
                        }
                    }
                }/*else
                {
                    for(int i1=0; i1<branchs[i].allpoints.size();++i1)
                    {
                        for(int j1=0; j1<branchs[j].allpoints.size();++j1)
                        {
                            double dis=zx_dist(branchs[i].allpoints[i1],branchs[j].allpoints[j1]);
                            if(dis<=5)
                            {
                                flag=1;
                            }
                        }
                    }
                }*/
            }
         branchs[i].flag_near_dendrite=flag;
        }
    }
    return true;
}

bool SwcTree::branchs_to_nt(vector<int> more)
{
    NeuronTree nt_tmp;
    NeuronSWC tmp;
    for(V3DLONG i=0;i<nt.listNeuron.size();++i)
    {
        if(nt.listNeuron[i].parent<0)
        {
            tmp=nt.listNeuron[i];
            break;
        }
    }
    nt_tmp.listNeuron.push_back(tmp);
    cout<<"branch_size: "<<branchs.size()<<endl;
    for(int i=0;i<branchs.size();++i)
    {
        vector<NeuronSWC> points;
        points.clear();
        branchs[i].get_points_of_branch(points,nt);
        bool flag=false;
        for(int j=0;j<more.size();++j)
        {
            if(i==more[j])
                flag=true;
        }

        if(flag==true)
        {
            for(int j=1;j<points.size();++j)
            {
                points[j].type=5;
                nt_tmp.listNeuron.push_back(points[j]);
            }
        }else
        {
            for(int j=1;j<points.size();++j)
            {
                points[j].type=2;
                nt_tmp.listNeuron.push_back(points[j]);
            }
        }

    }
    nt.deepCopy(nt_tmp);
    nt_tmp.listNeuron.clear();
    return true;
}

bool SwcTree::cut_cross()
{
    V3DLONG num_p=nt.listNeuron.size();
    vector<vector<V3DLONG> > children=vector<vector<V3DLONG> >(num_p,vector<V3DLONG>());
    for(V3DLONG i=0;i<num_p;++i)
    {
        V3DLONG par=nt.listNeuron[i].parent;
        if(par<0)
        {
            continue;
        }
        children[nt.hashNeuron.value(par)].push_back(i);
    }

    cout<<"initial delete_branch......"<<endl;
    int b_num=branchs.size();
    vector<int> delete_branch;

    for(int i=0;i<b_num;++i)
    {
        if(branchs[i].parent!=0
                &&children[nt.hashNeuron.value(branchs[i].end_point.n)].size()>=2
                &&branchs[i].length<20)
        {
            double x_angle_sum=0,z_angle_sum=0;
            int count=0;
            for(int j=0;j<b_num;++j)
            {
                //double x_angle=0,z_angle=0;
                NeuronSWC p1,p2,p;
                if(branchs[i].end_point==branchs[j].head_point)
                {
                    vector<NeuronSWC> points;
                    branchs[j].get_points_of_branch(points,nt);
                    double length=0;
                    NeuronSWC par0=points[0];
                    for(int k=1;k<points.size();++k)
                    {
                        NeuronSWC child0=points[k];
                        length+=distance_two_point(par0,child0);
                        if(length>30)
                        {
                            /*
                            x_angle=x_angle_two_point(points[0],child0);
                            z_angle=z_angle_two_point(points[0],child0);
                            break;
                            */
                            p=child0;
                            break;
                        }
                        par0=child0;
                    }
                    if(length<=30)
                    {
                        //x_angle=x_angle_two_point(points[0],par0);
                        //z_angle=z_angle_two_point(points[0],par0);
                        p=par0;
                    }
                    count++;
                }
                if(count==1)
                {
                    //x_angle_sum+=x_angle;
                    //z_angle_sum+=z_angle;
                    p1=p;
                }
                if(count==2)
                {
                    //x_angle_sum-=x_angle;
                    //z_angle_sum-=z_angle;
                    //x_angle_sum=abs(x_angle_sum);
                    //z_angle_sum=abs(z_angle_sum);
                    p2=p;
                    x_angle_sum=angle_three_point(branchs[i].end_point,p1,p2);
                    break;
                }
            }
            if(x_angle_sum>2.5/*&&z_angle_sum>3*/)
            {
                delete_branch.push_back(i);
            }
        }
    }

    cout<<"cross......."<<endl;
    NeuronTree nttmp;
    for(int i=0;i<delete_branch.size();++i)
    {
        cout<<delete_branch[i]<<endl;
        vector<NeuronSWC> a;
        branchs[delete_branch[i]].get_points_of_branch(a,nt);
        for(int j=0;j<a.size();++j)
        {
            a[j].type=2;
            nttmp.listNeuron.push_back(a[j]);
        }
    }
    writeESWC_file("D://2.eswc",nttmp);
    cout<<"cross end........."<<endl;

    cout<<"initial delete_queue......."<<endl;

    vector<int> delete_queue;
    for(int i=0;i<delete_branch.size();++i)
    {
        if(children[nt.hashNeuron.value(branchs[delete_branch[i]].end_point.n)].size()>=2)
        {
            delete_queue.push_back(delete_branch[i]);
        }
    }

    while(!delete_queue.empty())
    {
        int delete_index=delete_queue.front();
        delete_queue.erase(delete_queue.begin());
        for(int i=0;i<branchs.size();++i)
        {
            if(branchs[delete_index].end_point==branchs[i].head_point)
            {
                delete_branch.push_back(i);
                if(children[nt.hashNeuron.value(branchs[i].end_point.n)].size()>=2)
                {
                    delete_queue.push_back(i);
                }
            }
        }
    }

    cout<<"delete children...."<<endl;


    sort(delete_branch.begin(),delete_branch.end());
    vector<int>::iterator iter_r=unique(delete_branch.begin(),delete_branch.end());
    delete_branch.erase(iter_r,delete_branch.end());
    sort(delete_branch.begin(),delete_branch.end(),Comp);

    cout<<"deleting......"<<endl;

    for(int i=0;i<delete_branch.size();++i)
    {
        cout<<delete_branch[i]<<endl;
        vector<Branch>::iterator iter=branchs.begin()+delete_branch[i];
        branchs.erase(iter);
    }

    return true;

}


bool SwcTree::find_big_turn()
{
    NeuronTree nttmp;

    for(int i=0;i<branchs.size();++i)
    {
        vector<NeuronSWC> r_points;
        r_points.clear();
        branchs[i].get_r_points_of_branch(r_points,nt);

        int num_p=r_points.size();
        cout<<"num_p:"<<num_p<<endl;
        //NeuronSWC child=r_points[0];
        vector<double> angles=vector<double>(num_p-2,0);
        double max_angle=0;

        for(int j=1;j<num_p-1;++j)
        {
            //NeuronSWC par=r_points[j];
            //NeuronSWC p_par=r_points[j+1];
            angles[j-1]=PI-angle_three_point(r_points[j],r_points[j-1],r_points[j+1]);

            max_angle=(max_angle>angles[j-1])?max_angle:angles[j-1];

            //cout<<"index: "<<j<<" "<<angles[j-1]<<endl;

            if(angles[j-1]>1)
            {
                for(int k=0;k<j+1;++k)
                {
                    nttmp.listNeuron.push_back(r_points[k]);
                }
            }


            //child=par;
        }


        cout<<i<<":"<<"max_angle: "<<max_angle<<endl;
        cout<<endl;

    }

    for(int i=0;i<nttmp.listNeuron.size();++i)
    {
        nttmp.listNeuron[i].type=2;
    }

    writeESWC_file("D://3.eswc",nttmp);



    return true;
}

bool SwcTree::get_level_index(vector<int> &level_index,int level)
{
    for(int i=0;i<branchs.size();++i)
    {
        if(branchs[i].level==level)
        {
            level_index.push_back(i);
        }
    }
    return true;
}

bool SwcTree::get_points_of_branchs(vector<Branch> &b,vector<NeuronSWC> &points,NeuronTree &ntb)
{
    for(int i=0;i<(b.size()-1);++i)
    {
        b[i].get_points_of_branch(points,ntb);
        points.pop_back();
    }
    b[(b.size()-1)].get_points_of_branch(points,ntb);
    return true;
}

int SwcTree::get_max_level()
{
    int max_level;
    for(int i=0;i<branchs.size();++i)
    {
        max_level=(max_level>branchs[i].level)?max_level:branchs[i].level;
    }
    return max_level;
}
/*
NeuronTree SwcTree::refine_swc(QString braindir, double thresh, V3DPluginCallback2 &callback)
{
    NeuronTree tree_out;
    cout<<"0000000000"<<endl;

    for(int i=0; i<this->branchs.size();++i)
    {
        cout <<"i="<<i<<endl;
        Branch branch=this->branchs[i];
        vector<NeuronSWC> nodes;
        branch.get_points_of_branch(nodes,this->nt);
        //vector<NeuronSWC> control_points;
        //control_points.push_back(nodes[0]);
        //2nd check points
        vector<int> break_points_2nd;
        break_points_2nd.push_back(0);

        double length=0;
        NeuronTree branch_nt;
        branch_nt.listNeuron.clear();
        branch_nt.hashNeuron.clear();
        NeuronTree seg_nt;

        for(int j=0; j<nodes.size()-1;++j)
        {
            cout <<"node size="<<nodes.size()<<endl;
            cout <<"j="<<j<<endl;
            length+= distance_two_point(nodes[j],nodes[j+1]);
            cout<<"length="<<length<<endl;
            if(length<thresh)
            {
                seg_nt.listNeuron.push_back(nodes[j]);
                if(j==nodes.size()-2)
                {
                    // run gd on last break point towards leaf node
                    seg_nt.listNeuron.push_back(nodes[j+1]);
                    cout<<"11111111111"<<endl;
                    this->gd_on_nt(seg_nt,branch_nt,braindir,callback);
                    length=0;
                    seg_nt.listNeuron.clear();
                    seg_nt.hashNeuron.clear();
                    break_points_2nd.push_back(branch_nt.listNeuron.size()-1);
                    //break_points_2nd.push_back(nodes.size()-1);
                }

            } else
            {
                int pre_sz=branch_nt.listNeuron.size();
                cout<<"222222222222222222222222222"<<endl;
                this->gd_on_nt(seg_nt,branch_nt,braindir,callback);
                int cur_sz=branch_nt.listNeuron.size();
                int break_index_2nd=(cur_sz-pre_sz)/2+pre_sz;
                break_points_2nd.push_back(break_index_2nd);
                length=0;
                seg_nt.listNeuron.clear();
                seg_nt.hashNeuron.clear();
            }
        }

        NeuronTree branch_nt_1=SortSWC_pipeline(branch_nt.listNeuron,VOID,20);//sort and connect
        cout<<"branch_nt_1.size="<< branch_nt_1.listNeuron.size()<<endl;
        //NeuronTree branch_nt_1;
        //branch_nt_1.deepCopy(branch_nt);
        NeuronTree branch_nt_2;
        branch_nt_2.listNeuron.clear();
        branch_nt_2.hashNeuron.clear();
        NeuronTree seg_nt_2;
        cout<<"break_points_2nd sz="<<break_points_2nd.size()<<endl;
        for( int j=0;j<break_points_2nd.size()-1;++j)
        {
            for(int jj=break_points_2nd[j]; jj<break_points_2nd[j+1];++jj)
            {
                seg_nt_2.listNeuron.push_back(branch_nt_1.listNeuron.at(jj));
            }
            this->gd_on_nt(seg_nt_2,branch_nt_2,braindir,callback);
            seg_nt_2.listNeuron.clear();
            seg_nt_2.listNeuron.clear();
        }

        V3DLONG index;
        V3DLONG tree_out_length=tree_out.listNeuron.size();
        if(tree_out_length>0)
            index = tree_out.listNeuron.at(tree_out_length-1).n;
        else
            index = 0;
        for (int d=0;d<branch_nt_2.listNeuron.size();d++)
        {
            NeuronSWC curr = branch_nt_2.listNeuron.at(d);
            NeuronSWC S;
            S.n 	= curr.n + index; //use index to coordinate
            S.type 	= curr.type;
            S.x 	= curr.x ;
            S.y 	= curr.y ;
            S.z 	= curr.z ;
            S.r 	= curr.r;
            S.level = 20;
            S.pn 	= (curr.pn == -1)?  curr.pn : curr.pn + index;
            tree_out.listNeuron.append(S);
            tree_out.hashNeuron.insert(S.n, tree_out.listNeuron.size()-1);
        }
    }
    NeuronTree tree_out_sort=SortSWC_pipeline(tree_out.listNeuron,VOID,20);
    //NeuronTree tree_out_sort;
    //tree_out_sort.deepCopy(tree_out);
    return tree_out_sort;
}


bool SwcTree::gd_on_nt(NeuronTree &seg_nt, NeuronTree &tree_out,QString braindir,V3DPluginCallback2 &callback)
{
    int seg_size=seg_nt.listNeuron.size();

    cout<<"seg size="<<seg_size<<endl;
    size_t x0,x1,y0,y1,z0,z1;
    x0=1000000;x1=0;
    y0=1000000;y1=0;
    z0=1000000;z1=0;
    for(int j=0; j<seg_size;++j)
    {
        if (seg_nt.listNeuron.at(j).x < x0) x0=seg_nt.listNeuron.at(j).x;
        if (seg_nt.listNeuron.at(j).x > x1) x1=seg_nt.listNeuron.at(j).x;
        if (seg_nt.listNeuron.at(j).y < y0) y0=seg_nt.listNeuron.at(j).y;
        if (seg_nt.listNeuron.at(j).y > y1) y1=seg_nt.listNeuron.at(j).y;
        if (seg_nt.listNeuron.at(j).z < z0) z0=seg_nt.listNeuron.at(j).z;
        if (seg_nt.listNeuron.at(j).z > z1) z1=seg_nt.listNeuron.at(j).z;
    }

    x0-=50;x1+=50;
    y0-=50;y1+=50;
    z0-=50;z1+=50;
//    for(int k=0; k<seg_size;++k)
//    {
//        seg_nt.listNeuron.at(k).x-= x0;
//        seg_nt.listNeuron.at(k).y-= y0;
//        seg_nt.listNeuron.at(k).z-= z0;
//    }
    unsigned char* data1d=0;
    data1d=callback.getSubVolumeTeraFly(braindir.toStdString(),x0,x1+1,y0,y1+1,z0,z1+1);

    V3DLONG sz[4];
    sz[0]=x1-x0+1;
    sz[1]=y1-y0+1;
    sz[2]=z1-z0+1;
    sz[3]=1;
    //simple_saveimage_wrapper(callback,"/home/penglab/Ding/new_refine_test/1.v3draw",data1d,sz,1);
    //compute mask
    unsigned char * data1d_mask=0;
    data1d_mask=new unsigned char [sz[0]*sz[1]*sz[2]];
    memset(data1d_mask,0,sz[0]*sz[1]*sz[2]*sizeof(unsigned char));
    double margin;
    margin =10;//could be 5
    //convert vector<NeuronSWC> to n
    cout<<"**********computermaskimage in***********" <<endl;
    //offset seg_nt to nt;
    NeuronTree block_seg_nt;
    for(int j=0; j<seg_size;++j)
    {
        NeuronSWC S;
        S.n= seg_nt.listNeuron.at(j).n;
        S.type=seg_nt.listNeuron.at(j).type;
        S.x=seg_nt.listNeuron.at(j).x-x0;
        S.y=seg_nt.listNeuron.at(j).y-y0;
        S.z=seg_nt.listNeuron.at(j).z-z0;
        S.r=1;
        S.pn=seg_nt.listNeuron.at(j).pn;
        block_seg_nt.listNeuron.push_back(S);
    }
    //writeSWC_file("/home/penglab/Ding/new_refine_test/1.swc",block_seg_nt);
    ComputemaskImage(block_seg_nt,data1d_mask,sz[0],sz[1],sz[2],margin);
    //simple_saveimage_wrapper(callback,"/home/penglab/Ding/new_refine_test/1_mask.v3draw",data1d_mask,sz,1);

    for(V3DLONG k=0; k<sz[0]*sz[1]*sz[2]; ++k)
    {
        data1d[k]=(data1d_mask[k] == 0)?0:data1d[k];
    }
    //simple_saveimage_wrapper(callback,"/home/penglab/Ding/new_refine_test/1_masked_img.v3draw",data1d,sz,1);
    v3d_msg("check",0);
    cout<<"*********computermaskimage out**********" <<endl;

    unsigned char ****p4d = 0;
    if (!new4dpointer(p4d, sz[0], sz[1], sz[2], sz[3], data1d))
    {
        fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
        if(p4d) {delete []p4d; p4d = 0;}
        return false;
    }

    LocationSimple p0;
    p0.x = seg_nt.listNeuron.at(0).x - x0;
    p0.y = seg_nt.listNeuron.at(0).y - y0;
    p0.z = seg_nt.listNeuron.at(0).z - z0;

    vector<LocationSimple> pp;
    LocationSimple pEnd;
    pEnd.x = seg_nt.listNeuron.at(seg_size-1).x - x0;
    pEnd.y = seg_nt.listNeuron.at(seg_size-1).y - y0;
    pEnd.z = seg_nt.listNeuron.at(seg_size-1).z - z0;
    pp.push_back(pEnd);

    cout<<"x y z "<<x0<<"  "<<x1<<"  "<<y0<<"  "<<y1<<"  "<<z0<<"  "<<z1<<"  "<<endl;
    cout<<"start point= "<< p0.x<<"  "<<p0.y<<"  "<<p0.z<<endl;
    cout<<"end point =" << pEnd.x<<"  "<<pEnd.y<<"  "<<pEnd.z;



    //gd parameters
    double weight_xy_z=1.0;
    CurveTracePara trace_para;
    trace_para.channo = 0;
    trace_para.sp_graph_background = 0;
    trace_para.b_postMergeClosebyBranches = false;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_pruneArtifactBranches = false;
    trace_para.sp_num_end_nodes = 2;
    trace_para.b_deformcurve = false;
    trace_para.sp_graph_resolution_step = 1;
    trace_para.b_estRadii = false;

    cout<<"**********gd in*************" <<endl;

    NeuronTree nt_gd = v3dneuron_GD_tracing(p4d, sz,
                              p0, pp,trace_para, weight_xy_z);
    cout<<"**********gd out*************" <<endl;
    cout<<"nt_gd size="<<nt_gd.listNeuron.size()<<endl;
    //append gd to global swc
    V3DLONG index;
    V3DLONG tree_out_length=tree_out.listNeuron.size();

    if(tree_out_length>0)
        index = tree_out.listNeuron.at(tree_out_length-1).n;
    else
        index = 0;
   for (int d=0;d<nt_gd.listNeuron.size();d++)
    {
        NeuronSWC curr = nt_gd.listNeuron.at(d);
        NeuronSWC S;
        S.n 	= curr.n + index; //todo :use two index to coordinate
        S.type 	= curr.type;
        S.x 	= curr.x + x0;
        S.y 	= curr.y + y0;
        S.z 	= curr.z + z0;
        S.r 	= curr.r;
        S.level = 20;
        S.pn 	= (curr.pn == -1)?  curr.pn : curr.pn + index;
        tree_out.listNeuron.append(S);
        tree_out.hashNeuron.insert(S.n, tree_out.listNeuron.size()-1);
    }

   cout<<"branch_nt_length="<<tree_out.listNeuron.size()<<endl;
   pp.clear();
   if(data1d) {delete [] data1d;data1d=0;}
   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}


   return true;
}



bool Swc_Compare::get_sub_image(QString dir, vector<int> &a_false, vector<int> &b_false,SwcTree &a,SwcTree &b, QString braindir, V3DPluginCallback2 &callback)
{
    int a_size=a_false.size();
    int b_size=b_false.size();
    if(a_size!=b_size)
        return false;


    for(int i=0;i<a_size;++i)
    {
        NeuronTree nttmp_a,nttmp_b;
        vector<NeuronSWC> tmp_a,tmp_b;
//        a.branchs[a_false[i]].get_points_of_branch(tmp_a,a.nt);
//        b.branchs[b_false[i]].get_points_of_branch(tmp_b,b.nt);

        int a_branch_size=a.trunks[a_false[i]].branch_index.size();

        for(int j=0;j<a_branch_size-1;++j)
        {
            int a_branch_index=a.trunks[a_false[i]].branch_index[j];
            a.branchs[a_branch_index].get_points_of_branch(tmp_a,a.nt);
            tmp_a.pop_back();
        }
        int a_branch_index=a.trunks[a_false[i]].branch_index[a_branch_size-1];
        a.branchs[a_branch_index].get_points_of_branch(tmp_a,a.nt);

        int b_branch_size=b.trunks[b_false[i]].branch_index.size();

        for(int j=0;j<b_branch_size-1;++j)
        {
            int b_branch_index=b.trunks[b_false[i]].branch_index[j];
            b.branchs[b_branch_index].get_points_of_branch(tmp_b,b.nt);
            tmp_b.pop_back();
        }
        int b_branch_index=b.trunks[b_false[i]].branch_index[b_branch_size-1];
        b.branchs[b_branch_index].get_points_of_branch(tmp_b,b.nt);

        size_t x0=IN,x1=0,y0=IN,y1=0,z0=IN,z1=0;
        for(int j=0;j<tmp_a.size();++j)
        {
            tmp_a[j].type=3;

            size_t tmpx=(size_t)tmp_a[j].x;
            size_t tmpy=(size_t)tmp_a[j].y;
            size_t tmpz=(size_t)tmp_a[j].z;
            x0=(tmpx>x0)?x0:tmpx;
            x1=(tmpx>x1)?tmpx:x1;
            y0=(tmpy>y0)?y0:tmpy;
            y1=(tmpy>y1)?tmpy:y1;
            z0=(tmpz>z0)?z0:tmpz;
            z1=(tmpz>z1)?tmpz:z1;
        }
        for(int j=0;j<tmp_b.size();++j)
        {
            tmp_b[j].type=2;
            size_t tmpx=(size_t)tmp_b[j].x;
            size_t tmpy=(size_t)tmp_b[j].y;
            size_t tmpz=(size_t)tmp_b[j].z;
            x0=(tmpx>x0)?x0:tmpx;
            x1=(tmpx>x1)?tmpx:x1;
            y0=(tmpy>y0)?y0:tmpy;
            y1=(tmpy>y1)?tmpy:y1;
            z0=(tmpz>z0)?z0:tmpz;
            z1=(tmpz>z1)?tmpz:z1;
        }

        x0-=30;
        x1+=30;
        y0-=30;
        y1+=30;
        z0-=30;
        z1+=30;

        for(int j=0;j<tmp_a.size();++j)
        {
            tmp_a[j].x-=x0;
            tmp_a[j].y-=y0;
            tmp_a[j].z-=z0;
            nttmp_a.listNeuron.push_back(tmp_a[j]);
        }

        for(int j=0;j<tmp_b.size();++j)
        {
            tmp_b[j].x-=x0;
            tmp_b[j].y-=y0;
            tmp_b[j].z-=z0;
            nttmp_b.listNeuron.push_back(tmp_b[j]);
        }

        cout<<"x0: "<<x0<<" x1: "<<x1<<" y0: "<<y0<<" y1: "<<y1<<" z0: "<<z0<<" z1: "<<z1<<endl;

        unsigned char* pdata=0;
        pdata=callback.getSubVolumeTeraFly(braindir.toStdString(),x0,x1,y0,y1,z0,z1);

        V3DLONG sz[4]={(x1-x0),(y1-y0),(z1-z0),1};
        int datatype=1;
        QString tif_filename=dir+"/"+QString::number(i,10)+".tif";
        QString a_eswc_filename=dir+"/"+QString::number(i,10)+"manual.eswc";
        QString b_eswc_filename=dir+"/"+QString::number(i,10)+"auto.eswc";

        simple_saveimage_wrapper(callback,tif_filename.toStdString().c_str(),pdata,sz,datatype);
        writeESWC_file(a_eswc_filename,nttmp_a);
        writeESWC_file(b_eswc_filename,nttmp_b);
    }


}
*/

NeuronTree SwcTree::refine_swc_by_bdb(QString braindir, V3DPluginCallback2 &callback)
{
    NeuronTree refinetree;

    int size = branchs.size();

    map<Branch,int> mapbranch;
    for(int i=0; i<size; ++i)
    {
        mapbranch[branchs[i]] = i;
    }

    vector<vector<int> >children_b = vector<vector<int> >(size,vector<int>());
    vector<int> queue;

    for(int i =0; i<size; ++i)
    {
        if(branchs[i].parent==0)
        {
            queue.push_back(i);
            continue;
        }
        children_b[mapbranch[*branchs[i].parent]].push_back(i);
    }

    vector<vector<NeuronSWC> > listbpoints;
    map<int,int> map_ntoindex;

    while(!queue.empty())
    {
        int branchindex = queue.front();
        queue.erase(queue.begin());
        vector<NeuronSWC> points;
        if(branchs[branchindex].parent==0)
        {
            branchs[branchindex].length_to_soma = 0;
        }
        else
        {
            branchs[branchindex].length_to_soma = branchs[branchindex].parent->length_to_soma + branchs[branchindex].parent->length;
        }

        if(branchs[branchindex].length_to_soma>30&&branchs[branchindex].length>8)
        {
            branchs[branchindex].refine_branch(points,braindir,callback,nt);
        }
        else
        {
            branchs[branchindex].get_points_of_branch(points,nt);
        }

        if(map_ntoindex.find(points[0].n)==map_ntoindex.end())
        {
            vector<NeuronSWC> tmp;
            tmp.push_back(points[0]);
            map_ntoindex[points[0].n] = listbpoints.size();
            listbpoints.push_back(tmp);
        }
        else
        {
            int index = map_ntoindex[points[0].n];
            listbpoints[index].push_back(points[0]);
        }

        if(map_ntoindex.find(points[points.size()-1].n)==map_ntoindex.end())
        {
            vector<NeuronSWC> tmp;
            tmp.push_back(points[points.size()-1]);
            map_ntoindex[points[points.size()-1].n] = listbpoints.size();
            listbpoints.push_back(tmp);
        }
        else
        {
            int index = map_ntoindex[points[points.size()-1].n];
            listbpoints[index].push_back(points[points.size()-1]);
        }

        for(int i=1; i<points.size()-1; ++i)
        {
            points[i].type = 6;
            refinetree.listNeuron.push_back(points[i]);
        }

        for(int i=0; i<children_b[branchindex].size(); ++i)
        {
            queue.push_back(children_b[branchindex][i]);
        }

    }

    vector<NeuronSWC> bpoints;
    for(int i=0; i<listbpoints.size(); ++i)
    {
        int size = listbpoints[i].size();
        double x = 0, y = 0, z = 0;
        for(int j=0; j<size; ++j)
        {
            x += listbpoints[i].at(j).x;
            y += listbpoints[i].at(j).y;
            z += listbpoints[i].at(j).z;
        }
        x /= size, y /= size, z /= size;
        listbpoints[i].at(0).x = x;
        listbpoints[i].at(0).y = y;
        listbpoints[i].at(0).z = z;
        bpoints.push_back(listbpoints[i].at(0));
    }

    for(int i=0; i<bpoints.size(); ++i)
    {
        bpoints[i].type = 6;
        refinetree.listNeuron.push_back(bpoints[i]);
    }

    return refinetree;

}
NeuronTree SwcTree::refine_bifurcation_by_gd(QString braindir, V3DPluginCallback2 &callback,QString eswcfile){

    NeuronTree refinetree;
    int wins=20;

    int size = branchs.size();
    qDebug()<<"branch size : "<<size<<"----------------------------------------------------------------";

    map<Branch,int> mapbranch;
    for(int i=0; i<size; ++i)
    {
        mapbranch[branchs[i]] = i;
    }

    vector<vector<int> >children_b = vector<vector<int> >(size,vector<int>());
    vector<int> queue;

    for(int i =0; i<size; ++i)  //0-》1
    {
        if(branchs[i].parent==0)
        {
            queue.push_back(i);
            continue;
        }
        children_b[mapbranch[*branchs[i].parent]].push_back(i);
    }

    vector<vector<NeuronSWC> > pointslist;
    map<int,int> branchindex_to_pointslistindex;

    while(!queue.empty()){
        int branchindex = queue.front();
        queue.erase(queue.begin());
        Branch& cur_b = branchs[branchindex];
        int type=branchs[branchindex].type;
        vector<NeuronSWC> points;
        if(branchs[branchindex].parent==0)
        {
            branchs[branchindex].length_to_soma = 0;
        }
        else
        {
            branchs[branchindex].length_to_soma = branchs[branchindex].parent->length_to_soma + branchs[branchindex].parent->length;
        }
        if(children_b[branchindex].size()==2 && branchs[branchindex].length_to_soma>150 && branchs[branchindex].level>3 && branchs[branchindex].flag_near_dendrite==0){
           qDebug()<<"----------------branch point refinement processing-----------------"<<endl;

           vector<NeuronSWC> pp0_list, pp1_list, pp2_list;
           NeuronSWC ns0, ns1, ns2;
           ns0.type = ns1.type = ns2.type = -1;

           //Branch& cur_b = branchs[branchindex];
           Branch& ch_b1 = branchs[children_b[branchindex][0]];
           Branch& ch_b2 = branchs[children_b[branchindex][1]];
           int count = 0;
           for (int i = cur_b.allpoints.size() - 1 ; i >= 0; --i ){
               if (count >= wins || i == 0){
                   if (ns0.type == -1){
                       ns0 = cur_b.allpoints[i];
                   } else {
//                       NeuronSWC p;
//                       p.x = cur_b.allpoints[i].x;
                       pp0_list.push_back(cur_b.allpoints[i]);
                   }
               }
               count++;
           }
           reverse(pp0_list.begin(), pp0_list.end());
           count = 0;
           for (int i = 0; i < ch_b1.allpoints.size(); ++i ){
               if (count >= wins || i == ch_b1.allpoints.size()-1){
                   if (ns1.type == -1){
                       ns1 = ch_b1.allpoints[i];
                   } else {
                       pp1_list.push_back(ch_b1.allpoints[i]);
                   }
               }
               count++;
           }
           count = 0;
           for (int i = 0; i < ch_b2.allpoints.size(); ++i ){
               if (count >= wins || i == ch_b2.allpoints.size()-1){
                   if (ns2.type == -1){
                       ns2 = ch_b2.allpoints[i];
                   } else {
                       pp2_list.push_back(ch_b2.allpoints[i]);
                   }
               }
               count++;
           }
           LocationSimple pp0(ns0.x,ns0.y,ns0.z);
           LocationSimple pp1(ns1.x,ns1.y,ns1.z);
           LocationSimple pp2(ns2.x,ns2.y,ns2.z);
           qDebug()<<"start point: x: "<<ns0.x<<" y: "<<ns0.y<<" z: "<<ns0.z<<endl;
           qDebug()<<"start point: x: "<<ns1.x<<" y: "<<ns1.y<<" z: "<<ns1.z<<endl;
           qDebug()<<"start point: x: "<<ns2.x<<" y: "<<ns2.y<<" z: "<<ns2.z<<endl;
           NeuronTree nt_gd;
           branchs[branchindex].refine_branch_by_gd(pp0, pp1, pp2,nt_gd,braindir,callback);
           cout<<"refine_branch_by_gd_finished"<<endl;
           sortSWC(nt_gd, ns0);
           V3DLONG num_p=nt_gd.listNeuron.size();
           cout<<"nt_gd_size: "<<num_p<<endl;
          // writeESWC_file(eswcfile,nt_gd);
           if(num_p>=1){
               vector<vector<V3DLONG> > children=vector<vector<V3DLONG> >(num_p,vector<V3DLONG>());
               for(V3DLONG i=0;i<num_p;++i)
               {
                   V3DLONG par=nt_gd.listNeuron[i].parent;
                   if(par<0)
                   {
                       continue;
                   }
                   children[nt_gd.hashNeuron.value(par)].push_back(i);
               }
               cout<<"1"<<endl;
               cout<<"nt_gd_size: "<<num_p<<endl;
               int index = -1;
               int flag=0;
               for(int i=0; i<children.size();++i){
                   if(children[i].size()==2){
                       index=i;
                   }else if(children[i].size()>2){   //three branches
                       flag=1;
                   }
               }
               int pp0_index=-1;
               int pp1_index=-1;
               int pp2_index=-1;
               //find the index
               for (V3DLONG i=0;i<num_p;++i) {
                   NeuronSWC s = nt_gd.listNeuron[i];
                   if (children[i].size() == 0 && zx_dist(s, pp1) < 2) {
                       pp1_index = i;
                   } else if (children[i].size() == 0 && zx_dist(s, pp2) < 2) {
                       pp2_index = i;
                   } else if (s.parent == -1) {
                       pp0_index = i;
                   }
               }
               // 跳过三分叉
               if(pp0_index != -1 && pp1_index != -1 && pp2_index != -1 && flag == 0 && index != -1){
                   cout<<"2"<<endl;
                   cout<<"branch_index_nt_gd: "<<index<<endl;
                    cout<<"pp1_index: "<<pp1_index<<endl;
                     cout<<"pp2_index: "<<pp2_index<<endl;
                    cout<<"pp0_index: "<<pp0_index<<endl;
                   vector<NeuronSWC> pp0_list_tmp, pp1_list_tmp, pp2_list_tmp;
                   int tmp_index = index;
                   while (tmp_index != pp0_index) {
                       pp0_list_tmp.push_back(nt_gd.listNeuron[tmp_index]);
                       int tmp_par = nt_gd.listNeuron[tmp_index].parent;
                       tmp_index = nt_gd.hashNeuron.value(tmp_par);;
                   }
                   pp0_list_tmp.push_back(nt_gd.listNeuron[pp0_index]);
                   cout<<"child_of_nt_gd_done"<<index<<endl;
                   reverse(pp0_list_tmp.begin(), pp0_list_tmp.end());
                   pp0_list.insert(pp0_list.end(), pp0_list_tmp.begin(), pp0_list_tmp.end());
                   cout<<"3"<<endl;

                   tmp_index = pp1_index;
                   while (tmp_index != index) {
                       pp1_list_tmp.push_back(nt_gd.listNeuron[tmp_index]);
                       int tmp_par = nt_gd.listNeuron[tmp_index].parent;
                       tmp_index = nt_gd.hashNeuron.value(tmp_par);
                   }
                   pp1_list_tmp.push_back(nt_gd.listNeuron[index]);
                   reverse(pp1_list_tmp.begin(), pp1_list_tmp.end());
                   pp1_list.insert(pp1_list.begin(), pp1_list_tmp.begin(), pp1_list_tmp.end());
                   cout<<"4"<<endl;

                   tmp_index = pp2_index;
                   while (tmp_index != index) {
                       pp2_list_tmp.push_back(nt_gd.listNeuron[tmp_index]);
                       int tmp_par = nt_gd.listNeuron[tmp_index].parent;
                       tmp_index = nt_gd.hashNeuron.value(tmp_par);
                   }
                   pp2_list_tmp.push_back(nt_gd.listNeuron[index]);
                   reverse(pp2_list_tmp.begin(), pp2_list_tmp.end());
                   pp2_list.insert(pp2_list.begin(), pp2_list_tmp.begin(), pp2_list_tmp.end());

                   cout<<"5"<<endl;
                   cur_b.allpoints.clear();
                   cur_b.allpoints.assign(pp0_list.begin(),pp0_list.end());
                   ch_b1.allpoints.clear();
                   ch_b1.allpoints.assign(pp1_list.begin(),pp1_list.end());
                   qDebug()<<"chb2 size: "<<ch_b2.allpoints.size();
                   ch_b2.allpoints.clear();
                   qDebug()<<"chb2 size: "<<ch_b2.allpoints.size();
                   ch_b2.allpoints.assign(pp2_list.begin(),pp2_list.end());
                   qDebug()<<"chb2 size: "<<ch_b2.allpoints.size();
                   qDebug()<<"pp2 list size: "<<pp2_list.size();
    //               cur_b.allpoints = pp0_list;
    //               ch_b1.allpoints = pp1_list;
    //               ch_b2.allpoints = pp2_list;
                   for(int i=0; i<cur_b.allpoints.size(); ++i)
                   {
                       NeuronSWC p;
                       p.n = i+1;
                       p.parent = (p.n==1) ? -1 : i;
                       p.x = cur_b.allpoints[i].x;
                       p.y = cur_b.allpoints[i].y;
                       p.z = cur_b.allpoints[i].z;
                       p.type = type;
                       cur_b.allpoints[i] = p;
                       qDebug()<<"cur_b n p: "<<cur_b.allpoints[i].n<<" "<<cur_b.allpoints[i].parent;
//                       points.push_back(p);
                   }
                   for(int i=0; i<ch_b1.allpoints.size(); ++i)
                   {
                       NeuronSWC p;
                       p.n = i+1;
                       p.parent = (p.n==1) ? -1 : i;
                       p.x = ch_b1.allpoints[i].x;
                       p.y = ch_b1.allpoints[i].y;
                       p.z = ch_b1.allpoints[i].z;
                       p.type = ch_b1.type;
                       ch_b1.allpoints[i] = p;
    //                   ch_b1.parent=&cur_b;
                   }
                   for(int i=0; i<ch_b2.allpoints.size(); ++i)
                   {
                       NeuronSWC p;
                       p.n = i+1;
                       p.parent = (p.n==1) ? -1 : i;
                       p.x = ch_b2.allpoints[i].x;
                       p.y = ch_b2.allpoints[i].y;
                       p.z = ch_b2.allpoints[i].z;
                       p.type = ch_b2.type;
                       ch_b2.allpoints[i] = p;
    //                   ch_b2.parent=&cur_b;
                   }
               }
               cout<<"6"<<endl;
           }
        }

        for(int i=0; i<cur_b.allpoints.size(); ++i)
        {
            NeuronSWC p;
            p.n = i;
            p.parent = (p.n==0) ? -1 : i - 1;
            p.x = cur_b.allpoints[i].x;
            p.y = cur_b.allpoints[i].y;
            p.z = cur_b.allpoints[i].z;
            p.type = type;
//            cur_b.allpoints[i] = p;
            points.push_back(p);
        }
//        pointslist.push_back(cur_b.allpoints);
        qDebug()<<"points size: "<<points.size();
        pointslist.push_back(points);
        branchindex_to_pointslistindex[branchindex] = pointslist.size()-1;
        for(int i=0; i<children_b[branchindex].size(); ++i)
        {
            queue.push_back(children_b[branchindex][i]);
        }
    }
    for(int i=0; i<size; ++i)
    {
        if(branchs[i].parent==0)
        {
            queue.push_back(i);
            continue;
        }
    }
    cout<<"queue.size: "<<queue.size()<<endl;
    NeuronSWC o;
    o.n = 1;
    o.parent = -1;
    o.type = 1;
    o.x = pointslist[branchindex_to_pointslistindex[queue[0]]].at(0).x;
    o.y = pointslist[branchindex_to_pointslistindex[queue[0]]].at(0).y;
    o.z = pointslist[branchindex_to_pointslistindex[queue[0]]].at(0).z;
    refinetree.listNeuron.push_back(o);
    int lastindex;

    while(!queue.empty())
    {
        lastindex = refinetree.listNeuron.size();
        int branchindex = queue.front();
        queue.erase(queue.begin());
        int pointslistindex = branchindex_to_pointslistindex[branchindex];

        V3DLONG lastparent;
        if(branchs[branchindex].parent==0)
        {
            lastparent = 1;
//            cout<<branchindex<<endl;
        }
        else
        {
            int lastpointslistindex = branchindex_to_pointslistindex[mapbranch[*(branchs[branchindex].parent)]];
            lastparent = pointslist[lastpointslistindex].back().n; //(pointslist[lastpointslistindex].end()-1)->n;
            if(lastparent==1){ cout<<"lastparent: "<<lastparent<<endl;}

        }
        qDebug()<<"1"<<endl;

        for(int i=1; i<pointslist[pointslistindex].size(); ++i)
        {

//            NeuronSWC p;
            if(i==1)
            {
                pointslist[pointslistindex].at(i).parent = lastparent;
//                p.parent = lastparent;
            }
            else
            {
                pointslist[pointslistindex].at(i).parent += lastindex;
//                p.parent = pointslist[pointslistindex].at(i).parent + lastindex;
            }

            pointslist[pointslistindex].at(i).n += lastindex;

            refinetree.listNeuron.push_back(pointslist[pointslistindex].at(i));

//            NeuronSWC o;
//            p.n = pointslist[pointslistindex].at(i).n + lastindex;
            if (pointslist[pointslistindex].at(i).parent == 1) {
                qDebug()<<"lastindex: "<<lastindex<<" lastparent: "<<lastparent<<" i: "<<i
                       <<" branchindex: "<<branchindex<<" points index: "<<pointslistindex;
            }
//            o.parent = pointslist[pointslistindex].at(i).parent;
//            p.type = pointslist[pointslistindex].at(i).type;
//            p.x = pointslist[pointslistindex].at(i).x;
//            p.y = pointslist[pointslistindex].at(i).y;
//            p.z = pointslist[pointslistindex].at(i).z;
//            refinetree.listNeuron.push_back(p);
        }
        qDebug()<<"2"<<endl;
        for(int i=0; i<children_b[branchindex].size(); ++i)
        {
            queue.push_back(children_b[branchindex][i]);
        }
        qDebug()<<"pointslistindex: "<<pointslistindex<<endl;
        qDebug()<<" branchindex: "<<branchindex<<endl;
//        writeESWC_file(eswcfile,refinetree);
  }

   return refinetree;
}
NeuronTree SwcTree::refine_swc_by_gd(QString braindir, V3DPluginCallback2 &callback)
{
    const double thres = 50;

    NeuronTree refinetree;

    int size = branchs.size();
    qDebug()<<"branch size : "<<size<<"----------------------------------------------------------------";

    map<Branch,int> mapbranch;
    for(int i=0; i<size; ++i)
    {
        mapbranch[branchs[i]] = i;
    }

    vector<vector<int> >children_b = vector<vector<int> >(size,vector<int>());
    vector<int> queue;

    for(int i =0; i<size; ++i)
    {
        if(branchs[i].parent==0)
        {
            queue.push_back(i);
            continue;
        }
        children_b[mapbranch[*branchs[i].parent]].push_back(i);
    }

    vector<vector<NeuronSWC> > pointslist;
    map<int,int> branchindex_to_pointslistindex;

    while(!queue.empty())
    {
        int branchindex = queue.front();
        queue.erase(queue.begin());
        vector<NeuronSWC> points;
        if(branchs[branchindex].parent==0)
        {
            branchs[branchindex].length_to_soma = 0;
        }
        else
        {
            branchs[branchindex].length_to_soma = branchs[branchindex].parent->length_to_soma + branchs[branchindex].parent->length;
        }

        double branchmean,branchstd;
        branchs[branchindex].get_meanstd(braindir,callback,nt,branchmean,branchstd);

        qDebug()<<"branchmean: "<<branchmean;
        //加入branch order的判断  Yiwei Li 2022/1/18
        if((branchs[branchindex].length_to_soma<100||branchs[branchindex].length<80)&&branchs[branchindex].length<300 || (branchs[branchindex].level<=3 && (branchs[branchindex].type==3||branchs[branchindex].type==4)) || branchs[branchindex].flag_near_dendrite==1)
        {
            branchs[branchindex].get_points_of_branch(points,nt);
            for(int i=0; i<points.size(); ++i)
            {
                NeuronSWC p;
                p.n = i+1;
                p.parent = (p.n==1) ? -1 : i;
                p.x = points[i].x;
                p.y = points[i].y;
                p.z = points[i].z;
                p.type = points[i].type;
                points[i] = p;
            }
        }
        else
        {
            if(branchs[branchindex].child_num==2){
               qDebug()<<"----------------branch point refinement processing-----------------"<<endl;
               NeuronSWC p0;
               p0=branchs[branchindex].end_point;
               branchs[branchindex].end_point=meanshift(callback, p0,braindir);
            }
            vector<int> neuron_type;
            vector<LocationSimple> outbranch;
            branchs[branchindex].refine_by_2gd(outbranch,braindir,callback,nt,thres,neuron_type);
            smooth_curve(outbranch,6);
            qDebug()<<__LINE__<<"outbranch size: "<<outbranch.size();
            for(int i=0; i<outbranch.size(); ++i)
            {
                    NeuronSWC p;
                    p.n = i+1;
                    p.parent = (p.n==1) ? -1 : i;
                    p.x = outbranch[i].x;
                    p.y = outbranch[i].y;
                    p.z = outbranch[i].z;
//                    vector<NeuronSWC> ps;
//                    branchs[branchindex].get_points_of_branch(ps,nt);
//                    float x0=ps[0].x;
//                    float y0=ps[0].y;
//                    float z0=ps[0].z;
//                    float dis = sqrt((x0-p.x)*(x0-p.x)+(y0-p.y)*(y0-p.y)+(z0-p.z)*(z0-p.z));
//                    float dis1;
//                    p.type = 2;
//                    for(int j=1; j<ps.size();++j){
//                        dis1 = sqrt((ps[j].x-p.x)*(ps[j].x-p.x)+(ps[j].y-p.y)*(ps[j].y-p.y)+(ps[j].z-p.z)*(ps[j].z-p.z));
//                        if(dis1<=dis){
//                              dis=dis1;
//                              p.type = ps[j].type;
//                        }
//                    }
                    p.type = branchs[branchindex].type;
                    points.push_back(p);
            }
        }
        qDebug()<<__LINE__<<"_______________________________________";

        pointslist.push_back(points);
        branchindex_to_pointslistindex[branchindex] = pointslist.size()-1;

        qDebug()<<__LINE__<<"children size: "<<children_b[branchindex].size();

        for(int i=0; i<children_b[branchindex].size(); ++i)
        {
            queue.push_back(children_b[branchindex][i]);
        }
   }

    for(int i=0; i<size; ++i)
    {
        if(branchs[i].parent==0)
        {
            queue.push_back(i);
            continue;
        }
    }
    NeuronSWC o;
    o.n = 1;
    o.parent = -1;
    o.type = 1;
    o.x = pointslist[branchindex_to_pointslistindex[queue[0]]].at(0).x;
    o.y = pointslist[branchindex_to_pointslistindex[queue[0]]].at(0).y;
    o.z = pointslist[branchindex_to_pointslistindex[queue[0]]].at(0).z;
    refinetree.listNeuron.push_back(o);

    int lastindex;

    while(!queue.empty())
    {
        lastindex = refinetree.listNeuron.size();
        int branchindex = queue.front();
        queue.erase(queue.begin());
        int pointslistindex = branchindex_to_pointslistindex[branchindex];

        V3DLONG lastparent;
        if(branchs[branchindex].parent==0)
        {
            lastparent = 1;
        }
        else
        {
            int lastpointslistindex = branchindex_to_pointslistindex[mapbranch[*(branchs[branchindex].parent)]];
            lastparent = (pointslist[lastpointslistindex].end()-1)->n;
        }

        for(int i=1; i<pointslist[pointslistindex].size(); ++i)
        {

//            NeuronSWC p;
            if(i==1)
            {
                pointslist[pointslistindex].at(i).parent = lastparent;
            }
            else
            {
                pointslist[pointslistindex].at(i).parent += lastindex;
            }

            pointslist[pointslistindex].at(i).n += lastindex;

            refinetree.listNeuron.push_back(pointslist[pointslistindex].at(i));
        }

        for(int i=0; i<children_b[branchindex].size(); ++i)
        {
            queue.push_back(children_b[branchindex][i]);
        }
    }

    return refinetree;

}
NeuronTree SwcTree::refine_swc_branch_by_gd_img(string inimg_file, V3DPluginCallback2 &callback)
{
    int size=branchs.size();
    qDebug()<<"branch_refine_processing,branch size: "<<size<<endl;
    NeuronTree refinetree;
    for(int i =0; i<size;++i)
    {
       if(branchs[i].child_num==2)
       {
          qDebug()<<"----------------branch point refinement processing-----------------"<<endl;
//          Branch A=
       }
    }
    return refinetree;
}
NeuronTree SwcTree::refine_swc_by_gd_img(string inimg_file, V3DPluginCallback2 &callback)
{
    const double thres = 50;

    NeuronTree refinetree;

    int size = branchs.size();
    qDebug()<<"branch size : "<<size<<"----------------------------------------------------------------";

    map<Branch,int> mapbranch;
    for(int i=0; i<size; ++i)
    {
        mapbranch[branchs[i]] = i;
    }

    vector<vector<int> >children_b = vector<vector<int> >(size,vector<int>());
    vector<int> queue;

    for(int i =0; i<size; ++i)
    {
        if(branchs[i].parent==0)
        {
            queue.push_back(i);
            continue;
        }
        children_b[mapbranch[*branchs[i].parent]].push_back(i);
    }

    vector<vector<NeuronSWC> > pointslist;
    map<int,int> branchindex_to_pointslistindex;

    while(!queue.empty())
    {
        int branchindex = queue.front();
        queue.erase(queue.begin());
        vector<NeuronSWC> points;
        if(branchs[branchindex].parent==0)
        {
            branchs[branchindex].length_to_soma = 0;
        }
        else
        {
            branchs[branchindex].length_to_soma = branchs[branchindex].parent->length_to_soma + branchs[branchindex].parent->length;
        }

//        double branchmean,branchstd;
//        branchs[branchindex].get_meanstd_img(inimg_file,callback,nt,branchmean,branchstd);

//        qDebug()<<"branchmean: "<<branchmean;
        //add the judgement of branch order  Yiwei Li 2022/1/18
            vector<int> neuron_type;
            vector<LocationSimple> outbranch;
            if(branchs[branchindex].child_num==2){
               qDebug()<<"----------------branch point refinement processing-----------------"<<endl;
               NeuronSWC p0;
               p0=branchs[branchindex].end_point;
            }
            branchs[branchindex].refine_by_2gd_img(outbranch,inimg_file,callback,nt,thres,neuron_type);

            qDebug()<<__LINE__<<"outbranch size: "<<outbranch.size();
            for(int i=0; i<outbranch.size(); ++i)
            {
                    NeuronSWC p;
                    p.n = i+1;
                    p.parent = (p.n==1) ? -1 : i;
                    p.x = outbranch[i].x;
                    p.y = outbranch[i].y;
                    p.z = outbranch[i].z;
                    vector<NeuronSWC> ps;
                    branchs[branchindex].get_points_of_branch(ps,nt);
                    float x0=ps[0].x;
                    float y0=ps[0].y;
                    float z0=ps[0].z;
                    float dis = sqrt((x0-p.x)*(x0-p.x)+(y0-p.y)*(y0-p.y)+(z0-p.z)*(z0-p.z));
                    float dis1;
                    p.type = 2;
                    for(int j=1; j<ps.size();++j){
                        dis1 = sqrt((ps[j].x-p.x)*(ps[j].x-p.x)+(ps[j].y-p.y)*(ps[j].y-p.y)+(ps[j].z-p.z)*(ps[j].z-p.z));
                        if(dis1<=dis){
                              dis=dis1;
                              p.type = ps[j].type;
                        }
                    }
//                    p.type = neuron_type[i];
                    points.push_back(p);
            }
        qDebug()<<__LINE__<<"_______________________________________";

        pointslist.push_back(points);
        branchindex_to_pointslistindex[branchindex] = pointslist.size()-1;

        qDebug()<<__LINE__<<"children size: "<<children_b[branchindex].size();

        for(int i=0; i<children_b[branchindex].size(); ++i)
        {
            queue.push_back(children_b[branchindex][i]);
        }
    }

    for(int i=0; i<size; ++i)
    {
        if(branchs[i].parent==0)
        {
            queue.push_back(i);
            continue;
        }
    }
    NeuronSWC o;
    o.n = 1;
    o.parent = -1;
    o.type = 1;
    o.x = pointslist[branchindex_to_pointslistindex[queue[0]]].at(0).x;
    o.y = pointslist[branchindex_to_pointslistindex[queue[0]]].at(0).y;
    o.z = pointslist[branchindex_to_pointslistindex[queue[0]]].at(0).z;
    refinetree.listNeuron.push_back(o);

    int lastindex;

    while(!queue.empty())
    {
        lastindex = refinetree.listNeuron.size();
        int branchindex = queue.front();
        queue.erase(queue.begin());
        int pointslistindex = branchindex_to_pointslistindex[branchindex];

        V3DLONG lastparent;
        if(branchs[branchindex].parent==0)
        {
            lastparent = 1;
        }
        else
        {
            int lastpointslistindex = branchindex_to_pointslistindex[mapbranch[*(branchs[branchindex].parent)]];
            lastparent = (pointslist[lastpointslistindex].end()-1)->n;
        }

        for(int i=1; i<pointslist[pointslistindex].size(); ++i)
        {

//            NeuronSWC p;
            if(i==1)
            {
                pointslist[pointslistindex].at(i).parent = lastparent;
            }
            else
            {
                pointslist[pointslistindex].at(i).parent += lastindex;
            }

            pointslist[pointslistindex].at(i).n += lastindex;

            refinetree.listNeuron.push_back(pointslist[pointslistindex].at(i));
        }

        for(int i=0; i<children_b[branchindex].size(); ++i)
        {
            queue.push_back(children_b[branchindex][i]);
        }
    }

    return refinetree;

}
bool SwcTree::test(QString braindir, V3DPluginCallback2 &callback)
{
    int size = branchs.size();
    vector<double> means = vector<double>(size,0);
    vector<double> stds = vector<double>(size,0);

    qDebug()<<"in test-----------------------------------";

    for(int i=0; i<size; ++i)
    {
        branchs[i].get_meanstd(braindir,callback,nt,means[i],stds[i]);
    }

    NeuronTree t;
    vector<NeuronSWC> points;

    for(int i=0; i<size; ++i)
    {
        points.clear();

        int type;
        if(means[i]<30)
        {
            type = 2;
        }
        else if(means[i]<50)
        {
            type = 3;
        }
        else if(means[i]<70)
        {
            type = 4;
        }
        else
        {
            type = 5;
        }

        branchs[i].get_points_of_branch(points,nt);
        for(int j=0; j<points.size(); ++j)
        {
            points[j].type = type;
            t.listNeuron.push_back(points[j]);
        }
    }
    QString eswcfile = "C://Users//BrainCenter//Desktop//test_refine//test.eswc";

    writeESWC_file(eswcfile,t);

    return true;
}










double Swc_Compare::get_distance_two_branch(Branch &a, Branch &b, NeuronTree &nta, NeuronTree &ntb)
{
    vector<NeuronSWC> branch_a,branch_b;
    a.get_points_of_branch(branch_a,nta);
    b.get_points_of_branch(branch_b,ntb);

    double d_btob=0;
    for(int i=0;i<branch_b.size();++i)
    {
        double d_tp=IN;
        for(int j=0;j<branch_a.size()-1;++j)
        {
            double tmp=distance_two_point(branch_b[i],branch_a[j])+distance_two_point(branch_b[i],branch_b[j+1]);
            if(d_tp>tmp)
            {
                d_tp=tmp;
            }
            else
            {
                d_btob+=p_to_line<NeuronSWC,Angle>(branch_b[i],branch_a[j],branch_a[j+1]);
                break;
            }
        }
    }
    return (d_btob/branch_b.size());
}

double Swc_Compare::get_distance_two_branchs(vector<Branch> &a, vector<Branch> &b, NeuronTree &nta, NeuronTree &ntb)
{
    cout<<"in distance..."<<endl;
    vector<NeuronSWC> branchs_a,branchs_b;
    cout<<"a size:"<<a.size()<<endl;
    cout<<"b size:"<<b.size()<<endl;
    for(int i=0;i<(a.size()-1);++i)
    {
        a[i].get_points_of_branch(branchs_a,nta);
        branchs_a.pop_back();
    }

    a[(a.size()-1)].get_points_of_branch(branchs_a,nta);

    cout<<"a finish..."<<endl;

    for(int i=0;i<(b.size()-1);++i)
    {
        b[i].get_points_of_branch(branchs_b,ntb);
        branchs_b.pop_back();
    }
    b[(b.size()-1)].get_points_of_branch(branchs_b,ntb);

    cout<<"b finish..."<<endl;

    double d_bs_to_bs=0;
    for(int i=0;i<branchs_b.size();++i)
    {
        double min_d=IN;
        int min_index;
        for(int j=0;j<branchs_a.size()-1;++j)
        {
            double tmp_d=distance_two_point(branchs_b[i],branchs_a[j])+distance_two_point(branchs_b[i],branchs_a[j+1]);
            if(tmp_d<min_d)
            {
                min_index=j;
                min_d=tmp_d;
            }
        }
        d_bs_to_bs+=p_to_line<NeuronSWC,Angle>(branchs_b[i],branchs_a[min_index],branchs_a[min_index+1]);
    }

    cout<<"d finish..."<<endl;

    return (d_bs_to_bs/branchs_b.size());

}

int Swc_Compare::get_corresponding_branch(Branch &a,vector<int> &b_index,SwcTree &b,vector<bool> &flag_b)
{
    double thres=0.5;
    double max_s=0;
    int index;
    for(int i=0;i<b_index.size();++i)
    {
        if(flag_b[b_index[i]]==true) continue;
        double tmp=a.head_angle*b.branchs[b_index[i]].head_angle;
        if(max_s<tmp)
        {
            max_s=tmp;
            index=b_index[i];
        }
    }
    cout<<"max_s: "<<max_s<<endl;
    if(max_s>thres)
    {
        flag_b[index]=true;
        return index;
    }
    else
        return -1;
}

bool Swc_Compare::compare_two_swc(SwcTree &a, SwcTree &b, vector<int> &a_false, vector<int> &b_false, vector<int> &a_more, vector<int> &b_more, NeuronTree &nta, NeuronTree &ntb, QString dir_a , QString dir_b , QString braindir, V3DPluginCallback2 &callback)
{


    cout<<"in compare..."<<endl;

    int a_branch_size=a.branchs.size();
    int b_branch_size=b.branchs.size();

    map<Branch,int> map_a,map_b;

    for(int i=0;i<a_branch_size;++i)
    {
        map_a[a.branchs[i]]=i;
    }
    for(int i=0;i<b_branch_size;++i)
    {
        map_b[b.branchs[i]]=i;
    }

    cout<<"end map..."<<endl;

    vector<vector<int> > children_a=vector<vector<int> >(a_branch_size,vector<int>());
    vector<vector<int> > children_b=vector<vector<int> >(b_branch_size,vector<int>());

    for(int i=0;i<a_branch_size;++i)
    {
        if(a.branchs[i].parent==0) continue;
        children_a[map_a[*(a.branchs[i].parent)]].push_back(i);
    }
    for(int i=0;i<b_branch_size;++i)
    {
        if(b.branchs[i].parent==0) continue;
        children_b[map_b[*(b.branchs[i].parent)]].push_back(i);
    }

    cout<<"end children..."<<endl;

    vector<int> a_level0_index,b_level0_index;//,a_level1_index,b_level1_index;
    a.get_level_index(a_level0_index,0);
    b.get_level_index(b_level0_index,0);
//    a.get_level_index(a_level1_index,1);
//    b.get_level_index(b_level1_index,1);

//    vector<bool> flag_b=vector<bool>(b_branch_size,false);
//    vector<bool> flag_a=vector<bool>(a_branch_size,false);

    map<int,int> map_b_a,map_a_b;

    vector<int> queue;

    for(int i=0;i<b_level0_index.size();++i)
    {

        int b_branch_index=b_level0_index[i];
        //b_false.push_back(b_branch_index);
        //cout<<"b_branch_index: "<<b_branch_index<<endl;
        //cout<<"start corresponding..."<<i<<endl;
        //double d=this->get_distance_for_manual(b.branchs[b_branch_index],nta,ntb);

        double d=this->get_distance_branchs_to_branch(a.branchs,b.branchs[b_branch_index],nta,ntb,map_b,map_b_a);

        if(d<10||b.branchs[b_branch_index].length<15)
        {
            for(int j=0;j<children_b[b_branch_index].size();++j)
            {
                int child_index=children_b[b_branch_index].at(j);

                //cout<<"children index: "<<child_index<<endl;
                queue.push_back(child_index);
                //b_false.push_back(child_index); Revised by DZC 24 JUL
            }
        }
        else
        {
            b_more.push_back(b_branch_index);
            //b_false.push_back(b_branch_index);
        }
    }
    cout<< "b_more number at level0 ="<<b_more.size()<<endl;

    int count=1;
    while(!queue.empty())
    {

        //cout<<"count: "<<count<<endl;
        count++;
        int b_branch_index=queue.front();
        queue.erase(queue.begin());
        //cout<<"start corresponding..."<<i<<endl;
        //double d=this->get_distance_for_manual(b.branchs[b_branch_index],nta,ntb);

        double d=this->get_distance_branchs_to_branch(a.branchs,b.branchs[b_branch_index],nta,ntb,map_b,map_b_a);

        if(d<10||b.branchs[b_branch_index].length<15)
        {
            for(int j=0;j<children_b[b_branch_index].size();++j)
            {
                int child_index=children_b[b_branch_index].at(j);

                //cout<<"children index: "<<child_index<<endl;
                queue.push_back(child_index);
                //b_false.push_back(child_index); Revised by DZC 24 JUL
                b_false.push_back(b_branch_index);
            }
        }
        else
        {
            b_more.push_back(b_branch_index);
            //b_false.push_back(b_branch_index);
        }
    }
    cout <<"b_more number at all level ="<<b_more.size()<<endl;
    queue.clear();

    for(int i=0;i<a_level0_index.size();++i)
    {
        int a_branch_index=a_level0_index[i];
        //a_false.push_back(a_branch_index);  // no level 0
        cout<<"start corresponding..."<<i<<endl;
        //double d=this->get_distance_for_manual(a.branchs[a_branch_index],ntb,nta);

        double d=this->get_distance_branchs_to_branch(b.branchs,a.branchs[a_branch_index],ntb,nta,map_a,map_a_b);

        if(d<5||a.branchs[a_branch_index].length<10)
        {
            for(int j=0;j<children_a[a_branch_index].size();++j)
            {
                int child_index=children_a[a_branch_index].at(j);
                queue.push_back(child_index);
                a_false.push_back(child_index);
            }
        }
        else
        {
            a_more.push_back(a_branch_index);
            //a_false.push_back(a_branch_index);
        }
    }

    cout<< "a_more number at level0 ="<<a_more.size()<<endl;

    while(!queue.empty())
    {
        int a_branch_index=queue.front();
        queue.erase(queue.begin());
        //double d=this->get_distance_for_manual(a.branchs[a_branch_index],ntb,nta);

        double d=this->get_distance_branchs_to_branch(b.branchs,a.branchs[a_branch_index],ntb,nta,map_a,map_a_b);

        if(d<5||a.branchs[a_branch_index].length<10)
        {
            for(int j=0;j<children_a[a_branch_index].size();++j)
            {
                int child_index=children_a[a_branch_index].at(j);
                queue.push_back(child_index);
                a_false.push_back(child_index);
            }
        }
        else
        {
            a_more.push_back(a_branch_index);

        }
    }

    cout<< "a_more number at all level ="<<a_more.size()<<endl;


//    int mode=1;
//    this->get_sub_image(dir_a,a_more,a,b,braindir,callback,mode,map_a_b);
//    mode=2;
//    this->get_sub_image(dir_b,b_more,b,a,braindir,callback,mode,map_b_a);




//    cout<<"in compare..."<<endl;
//    double th=3;
//    int a_size=a.branchs.size();
//    int b_size=b.branchs.size();

//    map<Branch,int> mapa,mapb;
//    for(int i=0;i<a_size;++i)
//    {
//        mapa[a.branchs[i]]=i;
//    }
//    for(int i=0;i<b_size;++i)
//    {
//        mapb[b.branchs[i]]=i;
//    }

//    cout<<"end map..."<<endl;

//    vector<vector<int> > children_a=vector<vector<int> >(a_size,vector<int>());
//    vector<vector<int> > children_b=vector<vector<int> >(b_size,vector<int>());

//    for(int i=0;i<a_size;++i)
//    {
//        if(a.branchs[i].parent==0) continue;
//        children_a[mapa[*(a.branchs[i].parent)]].push_back(i);
//    }
//    for(int i=0;i<b_size;++i)
//    {
//        if(b.branchs[i].parent==0) continue;
//        children_b[mapb[*(b.branchs[i].parent)]].push_back(i);
//    }

//    cout<<"end children..."<<endl;

//    vector<int> a_level0_index,b_level0_index;
//    a.get_level0_index(a_level0_index);
//    b.get_level0_index(b_level0_index);

//    cout<<"a_level0_size:"<<a_level0_index.size()<<endl;
//    cout<<"b_level0_size:"<<b_level0_index.size()<<endl;

//    cout<<"end level0..."<<endl;

//    vector<bool> flag_a=vector<bool>(a_size,false);
//    vector<bool> flag_b=vector<bool>(b_size,false);

//    map<int,int> map_a_b;
//    vector<int> queue;
//    for(int i=0;i<a_level0_index.size();++i)
//    {
//        cout<<"for "<<i<<": "<<endl;
//        int b_index=this->get_corresponding_branch(a.branchs[a_level0_index[i]],b_level0_index,b,flag_b);
//        cout<<"end get branch..."<<endl;
//        if(b_index==-1)
//        {
//            a_more.push_back(a_level0_index[i]);
//        }
//        else
//        {
//            flag_a[a_level0_index[i]]=true;
//            map_a_b[a_level0_index[i]]=b_index;
////            double d=this->get_distance_two_branch(a.branchs[a_level0_index[i]],b.branchs[b_index],nta,ntb);
////            if(d>th)
////            {
////                a_false.push_back(a_level0_index[i]);
////                b_false.push_back(b_index);
////            }
////            else
////            {
////                queue.push_back(a_level0_index[i]);
////            }

//            double l_a=a.branchs[a_level0_index[i]].length;
//            double l_b=b.branchs[b_index].length;
//            double l_a_sum=l_a;
//            double l_b_sum=l_b;

//            while(l_b_sum<l_a_sum)
//            {
//                for(int j=0;j<children_b[b_index].size();++j)
//                {
//                    int b_child_index=children_b[b_index].at(j);
//                    double tmp=b.branchs[b_index].end_angle*b.branchs[b_child_index].head_angle;

//                    if(tmp>0.9)
//                    {
//                        map_a_b[a_level0_index[i]]=b_child_index;
//                    }

//                }
//            }
//        }
//        cout<<"end find..."<<endl;
//    }

//    cout<<"end initialize..."<<endl;

//    for(int i=0;i<b_level0_index.size();++i)
//    {
//        if(flag_b[b_level0_index[i]]==true)
//            continue;
//        else
//        {
//            b_more.push_back(b_level0_index[i]);
//        }

//    }

//    while(!queue.empty())
//    {
//        int a_index=queue.front();
//        queue.erase(queue.begin());

//        int b_index=map_a_b[a_index];
//        vector<int> b_children_indexs;
//        for(int i=0;i<children_b[b_index].size();++i)
//        {
//            b_children_indexs.push_back(children_b[b_index].at(i));
//        }

//        for(int i=0;i<children_a[a_index].size();++i)
//        {
//            int a_child_index=children_a[a_index].at(i);
//            int b_child_index=this->get_corresponding_branch(a.branchs[a_child_index],b_children_indexs,b,flag_b);
//            if(b_child_index==-1)
//            {
//                a_more.push_back(a_child_index);
//            }
//            else
//            {
//                flag_a[a_child_index]=true;
//                map_a_b[a_child_index]=b_child_index;
//                double d=this->get_distance_two_branch(a.branchs[a_child_index],b.branchs[b_child_index],nta,ntb);
//                if(d>th)
//                {
//                    a_false.push_back(a_child_index);
//                    b_false.push_back(b_child_index);
//                }
//                else
//                {
//                    queue.push_back(a_child_index);
//                }
//            }

//        }



//        for(int i=0;i<b_children_indexs.size();++i)
//        {
//            if(flag_b[b_children_indexs[i]]==true)
//                continue;
//            else
//                b_more.push_back(b_children_indexs[i]);
//        }

//    }

//    cout<<"end compare..."<<endl;

    return true;

}

bool Swc_Compare::compare_two_swc(SwcTree &a, SwcTree &b, vector<int> &a_trunk, vector<int> &b_trunk, vector<int> &a_more, vector<int> &b_more, NeuronTree &nta, NeuronTree &ntb, int mode)
{
    cout<<"in compare..."<<endl;
    double th=3;
    int a_size=a.branchs.size();
    int b_size=b.branchs.size();

    map<Branch,int> mapa,mapb;
    for(int i=0;i<a_size;++i)
    {
        mapa[a.branchs[i]]=i;
    }
    for(int i=0;i<b_size;++i)
    {
        mapb[b.branchs[i]]=i;
    }

    cout<<"end map..."<<endl;

    vector<vector<int> > children_a=vector<vector<int> >(a_size,vector<int>());
    vector<vector<int> > children_b=vector<vector<int> >(b_size,vector<int>());

    for(int i=0;i<a_size;++i)
    {
        if(a.branchs[i].parent==0) continue;
        children_a[mapa[*(a.branchs[i].parent)]].push_back(i);
    }
    for(int i=0;i<b_size;++i)
    {
        if(b.branchs[i].parent==0) continue;
        children_b[mapb[*(b.branchs[i].parent)]].push_back(i);
    }

    cout<<"end children..."<<endl;

    map<int,int> map_b_t_a,map_b_t_b;

    for(int i=0;i<a_size;++i)
    {
        for(int j=0;j<a.trunks.size();++j)
        {
            bool stop=false;
            for(int k=0;k<a.trunks[j].branch_index.size();++k)
            {
                if(i==a.trunks[j].branch_index[k])
                {
                    stop=true;
                    map_b_t_a[i]=j;
                    break;
                }
            }
            if(stop)
                break;
        }
    }

    for(int i=0;i<b_size;++i)
    {
        for(int j=0;j<b.trunks.size();++j)
        {
            bool stop=false;
            for(int k=0;k<b.trunks[j].branch_index.size();++k)
            {
                if(i==b.trunks[j].branch_index[k])
                {
                    stop=true;
                    map_b_t_b[i]=j;
                    break;
                }
            }
            if(stop)
                break;
        }
    }

    int a_trunk_size=a.trunks.size();
    int b_trunk_size=b.trunks.size();

    vector<vector<int> > children_trunk_a=vector<vector<int> >(a_trunk_size,vector<int>());
    vector<vector<int> > children_trunk_b=vector<vector<int> >(b_trunk_size,vector<int>());

    for(int i=0;i<a_trunk_size;++i)
    {
        if(a.trunks[i].parent<0) continue;
        children_trunk_a[a.trunks[i].parent].push_back(i);
    }

    for(int i=0;i<b_trunk_size;++i)
    {
        if(b.trunks[i].parent<0) continue;
        children_trunk_b[b.trunks[i].parent].push_back(i);
    }

    vector<int> a_level0_index,b_level0_index;
    a.get_level_index(a_level0_index,0);
    b.get_level_index(b_level0_index,0);

    cout<<"a_level0_size:"<<a_level0_index.size()<<endl;
    cout<<"b_level0_size:"<<b_level0_index.size()<<endl;

    cout<<"end level0..."<<endl;

    map<int,int> map_tt_a_b;

    vector<bool> flag_a=vector<bool>(a_size,false);
    vector<bool> flag_b=vector<bool>(b_size,false);

    vector<int> trunk_queue;

    for(int i=0;i<a_level0_index.size();++i)
    {
        int a_index=a_level0_index[i];
        int b_index=this->get_corresponding_branch(a.branchs[a_index],b_level0_index,b,flag_b);

        if(b_index==-1)
        {
            a_more.push_back(a_index);
            continue;
        }
        int a_t_index=map_b_t_a[a_index];
        int b_t_index=map_b_t_b[b_index];
        map_tt_a_b[a_t_index]=b_t_index;
        trunk_queue.push_back(a_t_index);
    }

    cout<<"end initial trunk_queue..."<<endl;

    for(int i=0;i<b_level0_index.size();++i)
    {
        if(flag_b[b_level0_index[i]]==true)
            continue;
        else
        {
            b_more.push_back(b_level0_index[i]);
        }
    }

    cout<<"end b_more..."<<endl;
    int count=1;


    while(!trunk_queue.empty())
    {
        int a_t_index=trunk_queue.front();
        int b_t_index=map_tt_a_b[a_t_index];
        a_trunk.push_back(a_t_index);
        b_trunk.push_back(b_t_index);
        trunk_queue.erase(trunk_queue.begin());
    }

//    while(!trunk_queue.empty())
//    {
//        cout<<"for: "<<count<<"..."<<endl;

//        int a_t_index=trunk_queue.front();
//        trunk_queue.erase(trunk_queue.begin());
//        int b_t_index=map_tt_a_b[a_t_index];

//        int a_t_branch_size=children_trunk_a[a_t_index].size();
//        int b_t_branch_size=children_trunk_b[b_t_index].size();
//        int a_t_branch_index=0;
//        int b_t_branch_index=0;

//        int incount=0;

//        while((a_t_branch_index<=(a_t_branch_size))&&(b_t_branch_index<=(b_t_branch_size)))
//        {
//            cout<<"incount: "<<incount<<endl;
//            incount++;

//            double d=0;

//            cout<<"length..."<<endl;

//            double length_a=a.trunks[a_t_index].length[a_t_branch_index];
//            double length_b=b.trunks[b_t_index].length[b_t_branch_index];

//            cout<<"000"<<endl;

//            vector<Branch> a_branchs,b_branchs;
//            for(int i=0;i<=a_t_branch_index;++i)
//            {
//                a_branchs.push_back(a.branchs[a.trunks[a_t_index].branch_index[i]]);
//            }
//            cout<<"111"<<endl;
//            b_branchs.push_back(b.branchs[b.trunks[b_t_index].branch_index[b_t_branch_index]]);

//            cout<<"222"<<endl;

//            if(length_a>length_b)
//            {
//                d=this->get_distance_two_branchs(a_branchs,b_branchs,nta,ntb);
//                cout<<"d: "<<d<<endl;
//            }

//            cout<<"branch..."<<endl;

//            if(d>5)
//            {
//                a_trunk.push_back(a_t_index);
//                b_trunk.push_back(b_t_index);
//                break;
//            }
//            cout<<"end d..."<<endl;

//            if((a_t_branch_index==a_t_branch_size&&b_t_branch_index<=b_t_branch_size)||(b_t_branch_index==b_t_branch_size&&a_t_branch_index<=a_t_branch_size))
//            {
//                a_trunk.push_back(a_t_index);
//                b_trunk.push_back(b_t_index);
//                break;
//            }

//            cout<<"end bian..."<<endl;

//            if((a_t_branch_index==a_t_branch_size-1)||(b_t_branch_index==b_t_branch_size-1))
//            {
//                if(a_t_branch_index==a_t_branch_size-1)
//                {
//                    a_t_branch_index++;
//                    continue;
//                }
//                if(b_t_branch_index==b_t_branch_size-1)
//                {
//                    b_t_branch_index++;
//                    continue;
//                }
//            }

//            int a_trunk_child_index=children_trunk_a[a_t_index].at(a_t_branch_index);
//            int b_trunk_child_index=children_trunk_b[b_t_index].at(b_t_branch_index);

//            cout<<"1111"<<endl;




//            if(length_b<length_a*0.8)
//            {
//                //int b_trunk_child_index=children_trunk_b[b_t_index].at(b_t_branch_index);
//                int b_child_index=b.trunks[b_trunk_child_index].branch_index[0];
//                b_more.push_back(b_child_index);
//                b_t_branch_index++;
//                continue;
//            }
//            if(length_a<length_b*0.8)
//            {
//                //int a_trunk_child_index=children_trunk_b[a_t_index].at(a_t_branch_index);
//                int a_child_index=a.trunks[a_trunk_child_index].branch_index[0];
//                a_more.push_back(a_child_index);
//                a_t_branch_index++;
//                continue;
//            }
//            if(length_b>length_a*0.8&&length_a>length_b*0.8)
//            {
//                map_tt_a_b[a_trunk_child_index]=b_trunk_child_index;
//                trunk_queue.push_back(a_trunk_child_index);
//                a_t_branch_index++;
//                b_t_branch_index++;
//            }

//            cout<<"end..."<<endl;
//        }

//        count++;


//    }








    return true;
}

bool Swc_Compare::_get_sub_image(QString filename, vector<NeuronSWC> manual_points, vector<NeuronSWC> auto_points, vector<NeuronSWC> false_points , QString braindir, V3DPluginCallback2 &callback)
{
    cout<<"in sub image"<<endl;

    NeuronTree manual_tree,auto_tree,false_tree;
    size_t x0=IN,x1=0,y0=IN,y1=0,z0=IN,z1=0;
    for(int i=0;i<manual_points.size();++i)
    {
        manual_points[i].type=2;

        size_t tmpx=(size_t)manual_points[i].x;
        size_t tmpy=(size_t)manual_points[i].y;
        size_t tmpz=(size_t)manual_points[i].z;

        x0=(tmpx>x0)?x0:tmpx;
        x1=(tmpx>x1)?tmpx:x1;
        y0=(tmpy>y0)?y0:tmpy;
        y1=(tmpy>y1)?tmpy:y1;
        z0=(tmpz>z0)?z0:tmpz;
        z1=(tmpz>z1)?tmpz:z1;

    }

    for(int i=0;i<auto_points.size();++i)
    {
        auto_points[i].type=3;

        size_t tmpx=(size_t)auto_points[i].x;
        size_t tmpy=(size_t)auto_points[i].y;
        size_t tmpz=(size_t)auto_points[i].z;

        x0=(tmpx>x0)?x0:tmpx;
        x1=(tmpx>x1)?tmpx:x1;
        y0=(tmpy>y0)?y0:tmpy;
        y1=(tmpy>y1)?tmpy:y1;
        z0=(tmpz>z0)?z0:tmpz;
        z1=(tmpz>z1)?tmpz:z1;

    }

    for(int i=0;i<false_points.size();++i)
    {
        false_points[i].type=4;

        size_t tmpx=(size_t)false_points[i].x;
        size_t tmpy=(size_t)false_points[i].y;
        size_t tmpz=(size_t)false_points[i].z;

        x0=(tmpx>x0)?x0:tmpx;
        x1=(tmpx>x1)?tmpx:x1;
        y0=(tmpy>y0)?y0:tmpy;
        y1=(tmpy>y1)?tmpy:y1;
        z0=(tmpz>z0)?z0:tmpz;
        z1=(tmpz>z1)?tmpz:z1;
    }

    x0-=30;
    x1+=30;
    y0-=30;
    y1+=30;
    z0-=30;
    z1+=30;

    cout<<"xyz..."<<endl;

    for(int i=0;i<manual_points.size();++i)
    {
        manual_points[i].x-=x0;
        manual_points[i].y-=y0;
        manual_points[i].z-=z0;
        manual_tree.listNeuron.push_back(manual_points[i]);
    }

    for(int i=0;i<auto_points.size();++i)
    {
        auto_points[i].x-=x0;
        auto_points[i].y-=y0;
        auto_points[i].z-=z0;
        auto_tree.listNeuron.push_back(auto_points[i]);
    }

    for(int i=0;i<false_points.size();++i)
    {
        false_points[i].x-=x0;
        false_points[i].y-=y0;
        false_points[i].z-=z0;
        false_tree.listNeuron.push_back(false_points[i]);
    }

    cout<<"finish tree..."<<endl;

    unsigned char* pdata=0;
    pdata=callback.getSubVolumeTeraFly(braindir.toStdString(),x0,x1,y0,y1,z0,z1);

    V3DLONG sz[4]={(x1-x0),(y1-y0),(z1-z0),1};
    int datatype=1;
    QString tif_filename=filename+".tif";
    QString manual_eswc_filename=filename+"manual.eswc";
    QString auto_eswc_filename=filename+"auto.eswc";
    QString false_eswc_filename=filename+"false.eswc";

    simple_saveimage_wrapper(callback,tif_filename.toStdString().c_str(),pdata,sz,datatype);
    writeESWC_file(manual_eswc_filename,manual_tree);
    writeESWC_file(auto_eswc_filename,auto_tree);
    writeESWC_file(false_eswc_filename,false_tree);

    cout<<"finish...."<<endl;

    return true;
}


bool Swc_Compare::get_sub_false_trunk_image(QString dir, vector<int> &manual_false, vector<int> auto_false, SwcTree &manual_t, SwcTree &auto_t, QString braindir, V3DPluginCallback2 &callback)
{
    int manual_size=manual_false.size();
    int auto_size=auto_false.size();
    if(manual_size!=auto_size)
        return false;

    cout<<"in get image..."<<endl;

    map<Branch,int> map_manual,map_auto;
    for(int i=0;i<manual_t.branchs.size();++i)
    {
        map_manual[manual_t.branchs[i]]=i;
    }
    for(int i=0;i<auto_t.branchs.size();++i)
    {
        map_auto[auto_t.branchs[i]]=i;
    }

    cout<<"end map............"<<endl;

    vector<vector<int> > children_manual=vector<vector<int> >(manual_t.branchs.size(),vector<int>());
    vector<vector<int> > children_auto=vector<vector<int> >(auto_t.branchs.size(),vector<int>());

    for(int i=0;i<manual_t.branchs.size();++i)
    {
        if(manual_t.branchs[i].parent==0) continue;
        children_manual[map_manual[*(manual_t.branchs[i].parent)]].push_back(i);
    }
    for(int i=0;i<auto_t.branchs.size();++i)
    {
        if(auto_t.branchs[i].parent==0) continue;
        children_auto[map_auto[*(auto_t.branchs[i].parent)]].push_back(i);
    }

    for(int i=0;i<auto_size;++i)
    {
        cout<<"in for: "<<i<<"...."<<endl;
        int manual_branch_index=manual_false[i];
        int auto_branch_index=auto_false[i];

        vector<Branch> manual_branchs,auto_branchs;

        manual_branchs.push_back(manual_t.branchs[manual_branch_index]);
        while(manual_t.branchs[manual_branch_index].parent!=0)
        {
            int par_branch_index=map_manual[*(manual_t.branchs[manual_branch_index].parent)];
            manual_branchs.push_back(manual_t.branchs[par_branch_index]);
            manual_branch_index=par_branch_index;
        }

        reverse(manual_branchs.begin(),manual_branchs.end());

        auto_branchs.push_back(auto_t.branchs[auto_branch_index]);
        while(auto_t.branchs[auto_branch_index].parent!=0)
        {
            int par_branch_index=map_auto[*(auto_t.branchs[auto_branch_index].parent)];
            auto_branchs.push_back(auto_t.branchs[par_branch_index]);
            auto_branch_index=par_branch_index;
        }

        reverse(auto_branchs.begin(),auto_branchs.end());

        vector<NeuronSWC> manual_points,auto_points,false_points;
        manual_t.get_points_of_branchs(manual_branchs,manual_points,manual_t.nt);
        auto_t.get_points_of_branchs(auto_branchs,auto_points,auto_t.nt);

        vector<NeuronSWC> manual_child_points,auto_child_points;

        for(int j=0;j<children_manual[manual_branch_index].size();++j)
        {
            vector<NeuronSWC> tmp;
            int child_index=children_manual[manual_branch_index].at(j);
            manual_t.branchs[child_index].get_points_of_branch(tmp,manual_t.nt);
            tmp.erase(tmp.begin());
            manual_child_points.insert(manual_child_points.end(),tmp.begin(),tmp.end());
        }
        for(int j=0;j<children_auto[auto_branch_index].size();++j)
        {
            vector<NeuronSWC> tmp;
            int child_index=children_auto[auto_branch_index].at(j);
            auto_t.branchs[child_index].get_points_of_branch(tmp,auto_t.nt);
            tmp.erase(tmp.begin());
            auto_child_points.insert(auto_child_points.end(),tmp.begin(),tmp.end());
        }

        manual_points.insert(manual_points.end(),manual_child_points.begin(),manual_child_points.end());
        auto_points.insert(auto_points.end(),auto_child_points.begin(),auto_child_points.end());

        QString filename=dir+"/_"+QString::number(i,10)+"_false_trunk_";
        this->_get_sub_image(filename,manual_points,auto_points,false_points,braindir,callback);

    }

    return true;

}

double Swc_Compare::get_distance_for_manual(Branch &auto_branch, NeuronTree &nta, NeuronTree &ntb)
{
    cout<<"in distance..."<<endl;
    vector<NeuronSWC> auto_points;
    auto_branch.get_points_of_branch(auto_points,ntb);
    double d_btotree=0;
    for(int i=0;i<auto_points.size();++i)
    {
        double min_d=IN;
        int min_index;
        for(int j=0;j<nta.listNeuron.size()-1;++j)
        {
            double tmp_d=p_to_line<NeuronSWC,Angle>(auto_points[i],nta.listNeuron[j],nta.listNeuron[j+1]);
            if(tmp_d<min_d)
            {
                min_index=j;
                min_d=tmp_d;
            }
        }
        d_btotree+=p_to_line<NeuronSWC,Angle>(auto_points[i],nta.listNeuron[min_index],nta.listNeuron[min_index+1]);
    }

    return (d_btotree/auto_points.size());
}

bool Swc_Compare::get_sub_image(QString dir, vector<int> &false_index,SwcTree &t,SwcTree &t0,QString braindir,V3DPluginCallback2 &callback,int mode,map<int,int> &map_branch)
{

    int auto_size=false_index.size();

    cout<<"in get image..."<<endl;

    map<Branch,int> map_auto,map_manual;
    for(int i=0;i<t.branchs.size();++i)
    {
        map_auto[t.branchs[i]]=i;
    }
    for(int i=0;i<t0.branchs.size();++i)
    {
        map_manual[t0.branchs[i]]=i;
    }

    cout<<"end map............"<<endl;

    vector<vector<int> > children_auto=vector<vector<int> >(t.branchs.size(),vector<int>());
    vector<vector<int> > children_manual=vector<vector<int> >(t0.branchs.size(),vector<int>());


    for(int i=0;i<t.branchs.size();++i)
    {
        if(t.branchs[i].parent==0) continue;
        children_auto[map_auto[*(t.branchs[i].parent)]].push_back(i);
    }
    for(int i=0;i<t0.branchs.size();++i)
    {
        if(t0.branchs[i].parent==0) continue;
        children_manual[map_manual[*(t0.branchs[i].parent)]].push_back(i);
    }


    for(int i=0;i<auto_size;++i)
    {
        cout<<"in for: "<<i<<"...."<<endl;

        int auto_branch_index=false_index[i];

        vector<NeuronSWC> auto_child_points;
        vector<NeuronSWC> auto_points,manual_points,false_points;


        //QString filename=dir+"/_"+QString::number(i,10)+"_false_auto_";


        if(t.branchs[auto_branch_index].parent!=0)
        {
            int up_index=map_auto[*(t.branchs[auto_branch_index].parent)];
            int manual_branch_index=map_branch[up_index];

            for(int j=0;j<children_manual[manual_branch_index].size();++j)
            {
                vector<NeuronSWC> tmp;
                int child_index=children_manual[manual_branch_index].at(j);
                t0.branchs[child_index].get_points_of_branch(tmp,t0.nt);
                tmp.erase(tmp.begin());
                auto_child_points.insert(auto_child_points.end(),tmp.begin(),tmp.end());
            }
            vector<Branch> manual_branch;
            double length=0;

            manual_branch.push_back(t0.branchs[manual_branch_index]);

            while(t0.branchs[manual_branch_index].parent!=0&&length<100)
            {
                length+=t.branchs[manual_branch_index].length;

                int par_branch_index=map_manual[*(t0.branchs[manual_branch_index].parent)];
                manual_branch.push_back(t0.branchs[par_branch_index]);
                manual_branch_index=par_branch_index;
            }

            if(manual_branch.size()>0)
            {
                reverse(manual_branch.begin(),manual_branch.end());
                t0.get_points_of_branchs(manual_branch,manual_points,t0.nt);
            }

            manual_points.insert(manual_points.end(),auto_child_points.begin(),auto_child_points.end());

        }


//        auto_points.clear();
//        manual_points.clear();
        auto_child_points.clear();
        t.branchs[auto_branch_index].get_points_of_branch(false_points,t.nt);


        for(int j=0;j<children_auto[auto_branch_index].size();++j)
        {
            vector<NeuronSWC> tmp;
            int child_index=children_auto[auto_branch_index].at(j);
            t.branchs[child_index].get_points_of_branch(tmp,t.nt);
            tmp.erase(tmp.begin());
            auto_child_points.insert(auto_child_points.end(),tmp.begin(),tmp.end());
        }

        vector<Branch> auto_branchs;

        //auto_branchs.push_back(t.branchs[auto_branch_index]);

        double length=0;

        while(t.branchs[auto_branch_index].parent!=0&&length<100)
        {
            length+=t.branchs[auto_branch_index].length;

            int par_branch_index=map_auto[*(t.branchs[auto_branch_index].parent)];
            auto_branchs.push_back(t.branchs[par_branch_index]);
            auto_branch_index=par_branch_index;
        }

        if(auto_branchs.size()>0)
        {
            reverse(auto_branchs.begin(),auto_branchs.end());


            t.get_points_of_branchs(auto_branchs,auto_points,t.nt);
        }





        auto_points.insert(auto_points.end(),auto_child_points.begin(),auto_child_points.end());
        //manual_points.clear();



        QString filename=dir+"/_"+QString::number(i,10)+"_";
        if(mode==1)
        {
            //filename=dir+"/_"+QString::number(i,10)+"_auto_more_";
            this->_get_sub_image(filename,auto_points,manual_points,false_points,braindir,callback);
        }
        if(mode==2)
        {
            this->_get_sub_image(filename,manual_points,auto_points,false_points,braindir,callback);
            //filename=dir+"/_"+QString::number(i,10)+"_manual_more_";
        }


    }

    return true;
}


double Swc_Compare::get_distance_branchs_to_branch(vector<Branch> &a, Branch &b, NeuronTree &nta, NeuronTree &ntb,map<Branch,int> map_b,map<int,int> &map_b_a)
{
    vector<NeuronSWC> b_points;
    b.get_points_of_branch(b_points,ntb);
    double d_b_to_branchs=0;

    vector<int> min_branch_index=vector<int>(a.size(),0);

    for(int i=0;i<b_points.size();++i)
    {
        vector<NeuronSWC> a_points_j;
        double min_d=IN;
        int min_j,min_k;
        for(int j=0;j<a.size();++j)
        {
            a_points_j.clear();
            a[j].get_points_of_branch(a_points_j,nta);
            for(int k=0;k<a_points_j.size()-1;++k)
            {
                double tmp_d=p_to_line<NeuronSWC,Angle>(b_points[i],a_points_j[k],a_points_j[k+1]);

                if(tmp_d>100&&tmp_d>a[j].length)
                //if(tmp_d>a[j].length)
                    break;

                if(tmp_d<min_d)
                {
                    min_j=j;
                    min_k=k;
                    min_d=tmp_d;
                }
            }
        }

        min_branch_index[min_j]++;

        a_points_j.clear();
        a[min_j].get_points_of_branch(a_points_j,nta);
        d_b_to_branchs+=p_to_line<NeuronSWC,Angle>(b_points[i],a_points_j[min_k],a_points_j[min_k+1]);
    }

    vector<int>::iterator it=max_element(min_branch_index.begin(),min_branch_index.end());
    int branch_index_a=distance(min_branch_index.begin(),it);

    d_b_to_branchs/=b_points.size();

    if(d_b_to_branchs<20)
    {
        int branch_index_b=map_b[b];
        map_b_a[branch_index_b]=branch_index_a;
    }


    return d_b_to_branchs;
}

bool Swc_Compare::get_false_point_image(QString dir, vector<int> & more, SwcTree & a_tree, SwcTree & b_tree, V3DPluginCallback2 &callback, QString braindir, bool manual)
{
    for (int i=0; i<more.size();++i)
    {   // crop the point
        vector<NeuronSWC> seg_points;

        map<Branch,int> amap;
        for(int i=0;i<a_tree.branchs.size();++i)
        {
            amap[a_tree.branchs[i]]=i;
        }

        int a_branch_index=more[i];
        while(a_tree.branchs[a_branch_index].parent!=0)
        {
            a_tree.branchs[a_branch_index].get_r_points_of_branch(seg_points,a_tree.nt);
            seg_points.pop_back();
            a_branch_index=amap[*(a_tree.branchs[a_branch_index].parent)];
        }
        a_tree.branchs[a_branch_index].get_r_points_of_branch(seg_points,a_tree.nt);

        double d=IN;
        int count=0;
        int segpoints_index=seg_points.size()-1;

        //cout<<"reach here 0"<<endl;
        //cout<<"seg_points number"<<seg_points.size()<<endl;

        for(int ii=0;ii<seg_points.size();++ii)
        {
            //cout<<"ii:"<<ii<<endl;

            d=this->get_distance_branchs_to_point(b_tree.branchs,seg_points[ii],b_tree.nt);
            if(d<5)
            {
                count++;
            }else
            {
                if(count>0)
                {
                    count--;
                }
            }
            if(count>=10)
            {
                segpoints_index=ii-9;
                break;
            }
        }


        V3DLONG block_size=128;
        size_t x0= seg_points[segpoints_index].x-block_size/2;
        size_t x1= seg_points[segpoints_index].x+block_size/2;
        size_t y0= seg_points[segpoints_index].y-block_size/2;
        size_t y1= seg_points[segpoints_index].y+block_size/2;
        size_t z0= seg_points[segpoints_index].z-block_size/2;
        size_t z1= seg_points[segpoints_index].z+block_size/2;
        cout<<"x y z"<<x0<<"  "<<x1<<"  "<<y0<<"  "<<y1<<"  "<<z0<<"  "<<z1<<"  "<<endl;

        QList<CellAPO> falsep_list;
        CellAPO falsep;
        falsep.x=block_size/2+1;
        falsep.y=block_size/2+1;
        falsep.z=block_size/2+1;
        falsep.color.r=255;
        falsep.color.g=0;
        falsep.color.b=0;
        falsep.volsize=0.5;
        falsep_list.push_back(falsep);
        QString markerfilename=dir+"/"+QString::number(seg_points[segpoints_index].x)+"_"+QString::number(seg_points[segpoints_index].y)
                +"_"+QString::number(seg_points[segpoints_index].z)+".apo";
        writeAPO_file(markerfilename,falsep_list);

        unsigned char* data1d=callback.getSubVolumeTeraFly(braindir.toStdString(),x0,x1,y0,y1,z0,z1);
        int datatype=1;
        V3DLONG sz[4]={block_size,block_size,block_size,datatype};
        QString filename=dir+"/"+QString::number(seg_points[segpoints_index].x)+"_"+QString::number(seg_points[segpoints_index].y)
                +"_"+QString::number(seg_points[segpoints_index].z)+".v3draw";
        simple_saveimage_wrapper(callback,filename.toStdString().c_str(),data1d,sz,datatype);
        //crop swc
        NeuronTree nt_out_manual;
        NeuronTree nt_out_auto;

        if (manual==true)
        {
            crop_swc(a_tree.nt,nt_out_manual,2,x0,x1,y0,y1,z0,z1);
            crop_swc(b_tree.nt,nt_out_auto,3,x0,x1,y0,y1,z0,z1);
        } else
        {
            crop_swc(a_tree.nt,nt_out_auto,3,x0,x1,y0,y1,z0,z1);
            crop_swc(b_tree.nt,nt_out_manual,2,x0,x1,y0,y1,z0,z1);
        }

        QString autoswcfilename=dir+"/"+QString::number(seg_points[segpoints_index].x)+"_"+QString::number(seg_points[segpoints_index].y)
                +"_"+QString::number(seg_points[segpoints_index].z)+"_auto.swc";
        QString manualswcfilename=dir+"/"+QString::number(seg_points[segpoints_index].x)+"_"+QString::number(seg_points[segpoints_index].y)
                +"_"+QString::number(seg_points[segpoints_index].z)+"_manual.swc";
        writeSWC_file(autoswcfilename,nt_out_auto);
        writeSWC_file(manualswcfilename,nt_out_manual);

        //crop the point with the largest distance

//        double d_fore,d_back=IN;
//        double dd=0;
//        int seg_index=seg_points.size()-1;

////        d_fore=this->get_distance_branchs_to_point(b_tree.branchs,seg_points[0],b_tree.nt);
//        for(int i=0;i<seg_points.size();++i)
//        {
//            d_back=this->get_distance_branchs_to_point(b_tree.branchs,seg_points[i],b_tree.nt);
////            dd=abs(d_back-d_fore);
//            if(d_back<2)
//            {
//                seg_index=i;
//            }
////            d_fore=d_back;
//        }

//        if(data1d) delete[] data1d;

//        x0= seg_points[seg_index].x-block_size/2;
//        x1= seg_points[seg_index].x+block_size/2;
//        y0= seg_points[seg_index].y-block_size/2;
//        y1= seg_points[seg_index].y+block_size/2;
//        z0= seg_points[seg_index].z-block_size/2;
//        z1= seg_points[seg_index].z+block_size/2;

//        data1d=callback.getSubVolumeTeraFly(braindir.toStdString(),x0,x1,y0,y1,z0,z1);

//        QString filename2=dir+"/"+QString::number(i)+"_last.v3draw";
//        simple_saveimage_wrapper(callback,filename2.toStdString().c_str(),data1d,sz,datatype);

//        nt_out_manual.listNeuron.clear();
//        nt_out_auto.listNeuron.clear();

//        if (manual==true)
//        {
//            crop_swc(a_tree.nt,nt_out_manual,2,x0,x1,y0,y1,z0,z1);
//            crop_swc(b_tree.nt,nt_out_auto,3,x0,x1,y0,y1,z0,z1);
//        } else
//        {
//            crop_swc(a_tree.nt,nt_out_auto,3,x0,x1,y0,y1,z0,z1);
//            crop_swc(b_tree.nt,nt_out_manual,2,x0,x1,y0,y1,z0,z1);
//        }

//        QString autoswcfilename2=dir+"/"+QString::number(i)+"_last_auto.swc";
//        QString manualswcfilename2=dir+"/"+QString::number(i)+"_last_manual.swc";
//        writeSWC_file(autoswcfilename2,nt_out_auto);
//        writeSWC_file(manualswcfilename2,nt_out_manual);





//        QList<NeuronSWC> list_b=b_tree.nt.listNeuron;
//        vector <double> dis_vec;
//        vector <V3DLONG> index_vec;
//        dis_vec.clear();
//        for(V3DLONG j=0; j<seg_points.size();++j)
//        {
//            for(V3DLONG k=0;k<list_b.size();++k)
//            {
//                V3DLONG index=0;
//                double mindis=100000.0;
//                double ds=distance_two_point(seg_points[j],list_b.at(k));
//                if(ds<mindis)
//                {
//                    mindis=ds;
//                    index=k;
//                }
//                dis_vec.push_back(mindis);
//                index_vec.push_back(index);

//            }
//        }

//        for(int j=0; j<dis_vec.size()-1;++j)
//        {
//            double mingrad=0;
//            double grad= (dis_vec[j+1]-dis_vec[j])/dis_vec[j];
//            if (grad>mingrad) ;

//        }


    }

    return true;

}

bool Swc_Compare::crop_swc(NeuronTree &nt_in, NeuronTree &nt_out, int type, size_t x0, size_t x1, size_t y0, size_t y1, size_t z0, size_t z1)
{
    QList<NeuronSWC> list=nt_in.listNeuron;

    for(int i=0; i<list.size();++i)
    {
        NeuronSWC s=list.at(i);
        if(s.x>=x0 && s.x <=x1 && s.y>=y0 && s.y<=y1 && s.z>= z0 && s.z<= z1 )
        {
            s.x -=x0;
            s.y -=y0;
            s.z -=z0;
            s.type=type;
            nt_out.listNeuron.push_back(s);
        }
    }
    return true;
}


double Swc_Compare::get_distance_branchs_to_point(vector<Branch> &a, NeuronSWC &b,NeuronTree &nta)
{
    double min_d=IN;
    vector<NeuronSWC> a_points_i;
    for(int i=0;i<a.size();++i)
    {
        a_points_i.clear();
        a[i].get_points_of_branch(a_points_i,nta);
        for(int j=0;j<a_points_i.size()-1;++j)
        {
            double tmp_d=p_to_line<NeuronSWC,Angle>(b,a_points_i[j],a_points_i[j+1]);
            if(tmp_d>100 && tmp_d>a[i].length) break;
            if(tmp_d<min_d)
                min_d=tmp_d;
        }
    }
    return min_d;
}

bool Swc_Compare::global_compare(SwcTree & a_tree, SwcTree & b_tree, QString braindir, QString outdir, V3DPluginCallback2 &callback)
{
    //nt0: auto
    //qDebug()<<"reach here1";
    int x0_nt0=100000,x1_nt0=0,y0_nt0=100000,y1_nt0=0,z0_nt0=100000,z1_nt0=0;
    NeuronTree nt0;
    nt0.deepCopy(a_tree.nt);
    for(V3DLONG i=0; i<nt0.listNeuron.size();++i)
    {
        NeuronSWC s=nt0.listNeuron.at(i);
        if(s.x<x0_nt0) x0_nt0=s.x;
        if(s.x>x1_nt0) x1_nt0=s.x;
        if(s.y<y0_nt0) y0_nt0=s.y;
        if(s.y>y1_nt0) y1_nt0=s.y;
        if(s.z<z0_nt0) z0_nt0=s.z;
        if(s.z>z1_nt0) z1_nt0=s.z;
    }

    int x0_nt1=100000,x1_nt1=0,y0_nt1=100000,y1_nt1=0,z0_nt1=100000,z1_nt1=0;

    NeuronTree nt1;
    nt1.deepCopy(b_tree.nt);
    for(V3DLONG i=0; i<nt1.listNeuron.size();++i)
    {
        NeuronSWC s=nt1.listNeuron.at(i);
        if(s.x<x0_nt1) x0_nt1=s.x;
        if(s.x>x1_nt1) x1_nt1=s.x;
        if(s.y<y0_nt1) y0_nt1=s.y;
        if(s.y>y1_nt1) y1_nt1=s.y;
        if(s.z<z0_nt1) z0_nt1=s.z;
        if(s.z>z1_nt1) z1_nt1=s.z;
    }

    int x0,x1,y0,y1,z0,z1;
    x0=MAX(x0_nt0,x0_nt1);
    x1=MIN(x1_nt0,x1_nt1);
    y0=MAX(y0_nt0,y0_nt1);
    y1=MIN(y1_nt0,y1_nt1);
    z0=MAX(z0_nt0,z0_nt1);
    z1=MIN(z1_nt0,z1_nt1);

    qDebug()<<x0<<x1<<y0<<y1<<z0<<z1;

    //crop the swc and count
    //nt0: auto
    //qDebug()<<"reach here2";
    int consensus_count=0;
    int false_count=0;
    for(V3DLONG i=0; i< nt0.listNeuron.size();++i)
    {
        NeuronSWC s=nt0.listNeuron.at(i);

        if(s.x>=x0 && s.x <=x1 && s.y>=y0 && s.y<=y1 && s.z>= z0 && s.z<= z1 )
        {
            consensus_count++;
            double mindis= this->get_distance_branchs_to_point(b_tree.branchs,s,nt1);

            if(mindis>10) false_count++;

        } else continue;
   }

    ofstream out;
    QString recordfile=outdir+"/record.txt";
    out.open(recordfile.toStdString().c_str(),ios::app);
    out<<"Automatic swc node number="<<nt0.listNeuron.size()<<endl;
    out<<"consensus area node number="<<consensus_count<<endl;
    out<<"false nodes in the consensus area number="<<false_count<<endl;
    out<<"percentage of false nodes in the consensus area="<< (double(false_count)/double(consensus_count))*100<<"%"<<endl;
    false_count+=(nt0.listNeuron.size()-consensus_count);
    out<<"false nodes in all area number="<<false_count<<endl;
    out<<"percentage of false nodes in all area="<< (double(false_count)/double(nt0.listNeuron.size()))*100<<"% \n"<<endl;
    out.close();

    false_count-= (nt0.listNeuron.size()-consensus_count);
    QString recordfile1=outdir+"/record_number.txt";
    out.open(recordfile1.toStdString().c_str(),ios::app);
    out<<nt0.listNeuron.size()<<" ";
    out<<consensus_count<<" ";
    out<<false_count<<" ";
    out<< (double(false_count)/double(consensus_count))*100<<"%"<<" ";
    false_count+=(nt0.listNeuron.size()-consensus_count);
    out<<false_count<<" ";
    out<<(double(false_count)/double(nt0.listNeuron.size()))*100<<"%"<<" "<<endl;
    out.close();

    return true;
}

bool Swc_Compare::get_accurate_false_point_image(QString dir, vector<int> &false_branches, SwcTree &a_tree, SwcTree &b_tree, V3DPluginCallback2 &callback, QString braindir, bool manual)
{
    for (int i=0; i<false_branches.size();++i)
    {
        int index=false_branches[i];
        vector<NeuronSWC> branch_nodes;
        a_tree.branchs[index].get_points_of_branch(branch_nodes,a_tree.nt);

        int count=0;
        int correct_count=0;
        for(int j=0;j<branch_nodes.size();++j)
        {
            double dis=this->get_distance_branchs_to_point(b_tree.branchs,branch_nodes[j],b_tree.nt);
            double thresh;
            if(manual) thresh=3; else thresh=3;
            if (dis>thresh) {
                count++;
                correct_count=0;
                continue;
            }else
            {
                correct_count++;
                //crop image&swc and save
                if (correct_count==1)
                {
                    NeuronSWC s=branch_nodes[j-count];
                    V3DLONG block_size=128;
                    size_t x0= s.x-block_size/2;
                    size_t x1= s.x+block_size/2;
                    size_t y0= s.y-block_size/2;
                    size_t y1= s.y+block_size/2;
                    size_t z0= s.z-block_size/2;
                    size_t z1= s.z+block_size/2;
                    cout<<"x y z"<<x0<<"  "<<x1<<"  "<<y0<<"  "<<y1<<"  "<<z0<<"  "<<z1<<"  "<<endl;

                    QList<CellAPO> falsep_list;
                    CellAPO falsep;
                    falsep.x=block_size/2+1;
                    falsep.y=block_size/2+1;
                    falsep.z=block_size/2+1;
                    falsep.color.r=255;
                    falsep.color.g=0;
                    falsep.color.b=0;
                    falsep.volsize=0.5;
                    falsep_list.push_back(falsep);
                    QString markerfilename=dir+"/"+QString::number(s.x)+"_"+QString::number(s.y)+"_"+QString::number(s.z)+".apo";
                    writeAPO_file(markerfilename,falsep_list);

                    unsigned char* data1d=callback.getSubVolumeTeraFly(braindir.toStdString(),x0,x1,y0,y1,z0,z1);
                    int datatype=1;
                    V3DLONG sz[4]={block_size,block_size,block_size,datatype};
                    QString filename=dir+"/"+QString::number(s.x)+"_"+QString::number(s.y)+"_"+QString::number(s.z)+".v3draw";
                    simple_saveimage_wrapper(callback,filename.toStdString().c_str(),data1d,sz,datatype);
                    //crop swc
                    NeuronTree nt_out_manual;
                    NeuronTree nt_out_auto;
                    if (manual==true)
                    {
                        crop_swc(a_tree.nt,nt_out_manual,2,x0,x1,y0,y1,z0,z1);
                        crop_swc(b_tree.nt,nt_out_auto,3,x0,x1,y0,y1,z0,z1);
                    } else
                    {
                        crop_swc(a_tree.nt,nt_out_auto,3,x0,x1,y0,y1,z0,z1);
                        crop_swc(b_tree.nt,nt_out_manual,2,x0,x1,y0,y1,z0,z1);
                    }
                    QString autoswcfilename=dir+"/"+QString::number(s.x)+"_"+QString::number(s.y)+"_"+QString::number(s.z)+"_auto.swc";
                    QString manualswcfilename=dir+"/"+QString::number(s.x)+"_"+QString::number(s.y)+"_"+QString::number(s.z)+"_manual.swc";
                    writeSWC_file(autoswcfilename,nt_out_auto);
                    writeSWC_file(manualswcfilename,nt_out_manual);


                }
                count=0;


            }


        }

    }
    return true;
}

float get_feature_extraction_rate(string inimg_file,NeuronTree& nt, V3DPluginCallback2 &callback){

//    QDir dir(inputfolder);
//    QStringList nameFilters;
//    nameFilters << "*.eswc"<<"*.swc"<<"*.ESWC"<<"*.SWC";
//    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
//    for(int j=0;j<files.size();j++){
//        QString swc_file = files.at(j);
//        QString swc_path=inputfolder+"/"+swc_file;
//        NeuronTree nt=readSWC_file(swc_path);
        float feature_extraction_rate;
        QList<NeuronSWC>& listNeuron =  nt.listNeuron;
        unsigned char* pdata = 0;
        int datatype;
        V3DLONG in_sz[4];
        simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), pdata, in_sz, datatype);
        V3DLONG total_size=in_sz[0]*in_sz[1]*in_sz[2];
        V3DLONG siz = nt.listNeuron.size();
        double bkg_thresh=200;
        int gt_area=0;
        V3DLONG intensity=0;
        for(V3DLONG i=0;i<total_size;i++)
        {
            intensity=pdata[i];
            if(intensity>=bkg_thresh){
                 gt_area+=1;
            }
        }
        qDebug()<<"ground truth area is :"<< gt_area<<endl;
        qDebug()<<"neuron size is :"<< siz<<endl;
        for(V3DLONG i=0;i<siz;i++)
        {
            //for all the node, if is axonal node and level=1,this is a virgin node that needs to be processed.
            NeuronSWC s = listNeuron.at(i);
            float r,x,y,z=0;
            r=s.r;
            x=s.x;
            y=s.y;
            z=s.z;
            float x3, x2, y2,y3,z2,z3;
            if(x+r+0.5>=in_sz[0]){x3=in_sz[0]-1;}else{x3=x+r+0.5;}
            if(x-r-0.5<0){x2=0;}else{x2=x-r-0.5;}
            if(y+r+0.5>=in_sz[1]){y3=in_sz[1]-1;}else{y3=y+r+0.5;}
            if(y-r-0.5<0){y2=0;}else{y2=y-r;}
            if(z+r+0.5>=in_sz[2]){z3=in_sz[2]-1;}else{z3=z+r+0.5;}
            if(z-r-0.5<0){z2=0;}else{z2=z-r-0.5;}
            for(int x1=x2; x1<=x3;++x1){
                for(int y1=y2; y1<=y3;++y1){
                    for(int z1=z2; z1<=z3;++z1){
                         double dis =sqrt((z1-z)*(z1-z)+(y1-y)*(y1-y)+(x1-x)*(x1-x));
                         if(dis<=r){
                             int pid=z1 * in_sz[0] * in_sz[1] + y1 * in_sz[0] + x1;
                             if(pdata[pid]>=bkg_thresh){pdata[pid]=120;}
                         }
                    }
                }
            }
        }
        float trace_area=0;
        for(V3DLONG i=0;i<total_size;i++)
        {
            intensity=pdata[i];
    //        if(intensity>0){qDebug()<<intensity<<endl;}
            if(intensity==120){
                 //qDebug()<<intensity<<endl;
                 trace_area+=1;
            }
        }
        qDebug()<<"traced_area : "<<trace_area<<endl;
        feature_extraction_rate=trace_area / gt_area;
        qDebug()<<"feature_extraction_rate is :"<< feature_extraction_rate<<endl;
    //}

  return feature_extraction_rate;
}
void  compute_image_snr(QString infolder_1,QString infolder_2,QString SNr_out, V3DPluginCallback2 &callback){
        QDir dir1(infolder_1);
        QStringList nameFilters;
        nameFilters << "*.tif"<<"*.v3draw";
        QStringList files1 = dir1.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
        QDir dir2(infolder_2);;
        QStringList files2 = dir2.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
        ofstream csvOutFile;
        csvOutFile.open(SNr_out.toStdString().c_str(),ios::out | ios::app);
        csvOutFile<<"img_name"<<","<<"SNR"<<endl;
//        csvOutFile.close();
        for(int i=0;i<i<files1.size();++i){
             QString img_file_1 = files1.at(i);
             QString img_path_1=infolder_1+"/"+img_file_1;
             QString img_file_2 = files2.at(i);
             QString img_path_2=infolder_2+"/"+img_file_2;
             unsigned char* pdata1 = 0;
             int datatype1;
             V3DLONG in_sz1[4];
             simple_loadimage_wrapper(callback,img_path_1.toStdString().c_str(), pdata1, in_sz1, datatype1);
             unsigned char* pdata2 = 0;
             int datatype2;
             V3DLONG in_sz2[4];
             simple_loadimage_wrapper(callback,img_path_2.toStdString().c_str(), pdata2, in_sz2, datatype2);
             V3DLONG total_size1=in_sz1[0]*in_sz1[1]*in_sz1[2];
             V3DLONG total_size2=(in_sz2[0]+1)*(in_sz2[1]+1)*(in_sz2[2]+1);
             if(total_size1!=total_size2){qDebug()<<"please check the image"<<endl;}
             qDebug()<<"image_size1:"<<total_size1<<endl;
             qDebug()<<"image_size2:"<<total_size2<<endl;
             double std1;
             double std2;
             double mean1;
             double mean2;
             double signal=0;
             double noise=0;
             double signal1=0;
             double noise1=0;
             V3DLONG sz0_1 = in_sz1[0] * in_sz1[1];
             V3DLONG sz1_1 = in_sz1[0];
             V3DLONG sz0_2 = in_sz2[0] * in_sz2[1];
             V3DLONG sz1_2 = in_sz2[0];
//             for(int j =0; j<total_size2; ++j){
//                 if(pdata1[j]<=80){pdata1[j]=0;}
//                 mean1+=pdata1[j];
//                 mean2+=pdata2[j]-pdata1[j];
//             }
             for (int x=0;x<in_sz2[0];x++){
                 for (int y=0;y<in_sz2[1];y++){
                     for (int z=0;z<in_sz2[2];z++){
                               int id1= (x+1)+(y+1)*sz1_1+(z+1)*sz0_1;
//                               int id1= x+y*sz1_1+z*sz0_1;;
                               int id2= x+y*sz1_2+z*sz0_2;
                               if(pdata1[id1]<=80){pdata1[id1]=0;}
                               mean1+=pdata1[id1];
                               mean2+=pdata2[id2]-pdata1[id1];
                               if(pdata1[id1]>=120){
                                   signal+=pdata1[id1]*pdata1[id1];
                                   noise+=(pdata2[id2]-pdata1[id1])*(pdata2[id2]-pdata1[id1]);
                               }
                               signal1+=pdata1[id1]*pdata1[id1];
                               noise1+=(pdata2[id2]-pdata1[id1])*(pdata2[id2]-pdata1[id1]);

                     }
                 }
             }
             qDebug()<<"mean cal finished"<<endl;
             mean1=mean1/total_size2;
             mean2=mean2/total_size2;
//             for(int j =0; j<total_size2; ++j){
//                 std1+=(pdata1[j]-mean1)*(pdata1[j]-mean1);
//                 std2+=(pdata2[j]-pdata1[j]-mean2)*(pdata2[j]-pdata1[j]-mean2);
//             }
             for (int x=0;x<in_sz2[0];x++){
                 for (int y=0;y<in_sz2[1];y++){
                     for (int z=0;z<in_sz2[2];z++){
                         int id1= (x+1)+(y+1)*sz1_1+(z+1)*sz0_1;
                         int id2= x+y*sz1_2+z*sz0_2;
                         std1+=(pdata1[id1]-mean1)*(pdata1[id1]-mean1);
                         std2+=(pdata2[id2]-pdata1[id1]-mean2)*(pdata2[id2]-pdata1[id1]-mean2);
                     }
                 }
             }
             qDebug()<<"image_mean1:"<<mean1<<endl;
             qDebug()<<"image_mean2:"<<mean2<<endl;
             std1=sqrt(std1)/total_size2;
             std2=sqrt(std2)/total_size2;
             double snr=0;
             double snr1=0;
//             if(std2>0){snr = std1/std2;snr=10*qLn(snr)/qLn(10);}
             if(std2>0){snr = signal/noise;snr=10*qLn(snr)/qLn(10);}
             snr1 = signal1/noise1;snr1=10*qLn(snr1)/qLn(10);
             //double snr = std1/std2;
//             csvOutFile.open(SNr_out.toStdString().c_str(),ios::out | ios::app);
             csvOutFile<<img_path_2.toStdString().c_str()<<","<<snr<<endl;
             csvOutFile<<img_path_2.toStdString().c_str()<<","<<snr1<<endl;
//             csvOutFile.close();
        }
 csvOutFile.close();
}
void  compare_2swc_change(NeuronTree &nt1,NeuronTree &nt2,V3DPluginCallback2 &callback,QString out_path){
      V3DLONG siz1 = nt1.listNeuron.size();
      QList<NeuronSWC>& listNeuron1 =  nt1.listNeuron;
      V3DLONG siz2 = nt2.listNeuron.size();
      QList<NeuronSWC>& listNeuron2 =  nt2.listNeuron;
      ofstream csvOutFile;
      csvOutFile.open(out_path.toStdString().c_str(),ios::out | ios::app);
      csvOutFile<<"distance_change"<<","<<"radius_change"<<","<<"intensity_change"<<","<<"sholl_distance"<<endl;
      NeuronSWC s0 = listNeuron1.at(0);
      float x0=s0.x;
      float y0=s0.y;
      float z0=s0.z;
      for(int i=1; i<siz1;i++){
          NeuronSWC s = listNeuron1.at(i);
          float r,x,y,z,in=0;
          r=s.r;
          x=s.x;
          y=s.y;
          z=s.z;
          in=s.level;
          float dmin=10;
          float r_out,dis_out,in_out,soma_out=0;
          soma_out=sqrt((x0-x)*(x0-x)+(y0-y)*(y0-y)+(z0-z)*(z0-z));
          for(int j=0; j<siz2;j++){
              NeuronSWC s2 = listNeuron2.at(j);
              float r2,x2,y2,z2,in2=0;
              r2=s2.r;
              x2=s2.x;
              y2=s2.y;
              z2=s2.z;
              in2=s2.level;
              float dis=sqrt((x2-x)*(x2-x)+(y2-y)*(y2-y)+(z2-z)*(z2-z));
              if(dis<dmin){
                  dmin=dis;
                  r_out=r2-r;
                  dis_out=dis;
                  in_out=in2-in;
              }
          }
          if(soma_out>0){csvOutFile<<dis_out<<","<<r_out<<","<<in_out<<","<<soma_out<<endl;}

      }
       csvOutFile.close();
}

NeuronSWC meanshift(V3DPluginCallback2 &callback, NeuronSWC p0,QString braindir)
{
    QDir all_braindir(braindir);
    QFileInfoList list_braindir = all_braindir.entryInfoList(QStringList(),QDir::Dirs|QDir::NoDotAndDotDot);
    int dir_count = list_braindir.size();

    map<int,int> sizemap;

    for(int i=0; i<dir_count; ++i)
    {
//        qDebug()<<i<<": "<<list_braindir[i].absoluteFilePath();
        QString t = list_braindir[i].baseName();
        QStringList ts =t.split('x');
        int resolution = ts[1].toInt();
        sizemap[resolution] = i;
    }

    QVector<QFileInfo> vtmp;
    map<int,int>::iterator it =sizemap.begin();

    while(it!=sizemap.end())
    {
        int index = it->second;
        vtmp.push_back(list_braindir[index]);
        it++;
    }

    list_braindir.clear();
    for(int i=0; i<dir_count; ++i)
    {
        list_braindir.push_back(vtmp[i]);
//        qDebug()<<list_braindir[i].absoluteFilePath();
    }

    QString current_braindir = list_braindir[dir_count-3].absoluteFilePath();

    qDebug()<<current_braindir;

           NeuronSWC p1;
           int r=15;
           V3DLONG *in_zz = 0;
           if(!callback.getDimTeraFly(current_braindir.toStdString().c_str(),in_zz)){cout<<"can't load terafly img"<<endl;}
           V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
           start_x=p0.x-r; if(start_x<0){start_x=0;} if(start_x>=in_zz[0]){start_x=in_zz[0]-1;cout<<"x axis over border"<<endl;}
           start_y=p0.y-r;if(start_y<0){start_y=0;}if(start_y>=in_zz[1]){start_y=in_zz[1]-1;cout<<"y axis over border"<<endl; }
           start_z=p0.z-r;if(start_z<0){start_z=0;}if(start_z>=in_zz[2]){start_z=in_zz[2]-1;cout<<"z axis over border"<<endl; }
           end_x=p0.x+r;if(end_x>in_zz[0]){end_x=in_zz[0]-1;}
           end_y=p0.y+r;if(end_y>in_zz[1]){end_y=in_zz[1]-1;}
           end_z=p0.z+r;if(end_z>in_zz[2]){end_z=in_zz[2]-1;}
           unsigned char * im_cropped = 0;
           V3DLONG *in_sz = new V3DLONG[4];
           if(end_x<=start_x){cout<<"over border"<<endl; return p0;}
           if(end_y<=start_y){cout<<"over border"<<endl;return p0;}
           if(end_z<=start_z){cout<<"over border"<<endl;return p0;}
           in_sz[0] = end_x-start_x+1;
           in_sz[1] = end_y-start_y+1;
           in_sz[2] = end_z-start_z+1;
           in_sz[3]=in_zz[3];
           V3DLONG pagesz;
           pagesz = in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];
           try {im_cropped = new unsigned char [pagesz];}
           catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl;}
           cout<<"2"<<endl;
           im_cropped = callback.getSubVolumeTeraFly(current_braindir.toStdString().c_str(),start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
           cout<<"1"<<endl;
           if(im_cropped==NULL){
               cout<<"3"<<endl;}
           V3DLONG y_offset=in_sz[0];
           V3DLONG z_offset=in_sz[0]*in_sz[1];
           V3DLONG windowradius=r-1;
           V3DLONG pos;
           V3DLONG pos0;
   //        float thresHold =100;
           float total_x,total_y,total_z,v_color,sum_v,thresHold,mean,std;
           total_x=0;
           total_y=0;
           total_z=0;
           v_color=0;
           sum_v=0;
           mean=0;
           std=0;
           pos0=p0.z*z_offset+p0.y*y_offset+p0.x;
//center_intensity[i]=im_cropped[pos0];
           for(int j=0; j<pagesz;j++){
                  mean=mean+im_cropped[j];
           }
           mean=mean/pagesz;
           for(int j=0; j<pagesz;j++){
                  std=std+(mean-im_cropped[j])*(mean-im_cropped[j]);
           }
           std=sqrt(std)/pagesz;
           thresHold=mean+std;
           if(thresHold<=100){thresHold=100;}
           for(V3DLONG dx=p0.x+0.5-start_x-windowradius; dx<=p0.x+0.5-start_x+windowradius; dx++){
               for(V3DLONG dy=p0.y+0.5-start_y-windowradius; dy<=p0.y+0.5-start_y+windowradius; dy++){
                   for(V3DLONG dz=p0.z+0.5-start_z-windowradius; dz<=p0.z+0.5-start_z+windowradius; dz++){
                       pos=dz*z_offset+dy*y_offset+dx;
   //                    double tmp=(dx-s.x)*(dx-s.x)+(dy-s.y)*(dy-s.y)
   //                         +(dz-s.z)*(dz-s.z);
   //                    double distance=sqrt(tmp);
   //                    if (distance>windowradius) continue;
                       v_color=im_cropped[pos];
                       //cout<<v_color<<endl;
                       if(v_color<thresHold)
                           v_color=0;
                       total_x=v_color*(float)dx+total_x;
                       total_y=v_color*(float)dy+total_y;
                       total_z=v_color*(float)dz+total_z;
                       sum_v=sum_v+v_color;
                    }
                }
            }
           if(sum_v>0){
               p1.x=total_x/sum_v+start_x;
               p1.y=total_y/sum_v+start_y;
               p1.z=total_z/sum_v+start_z;
           }else{
               p1=p0;
           }
           if(im_cropped) {delete []im_cropped; im_cropped=0;}
       return p1;
}


NeuronSWC meanshift_img(V3DPluginCallback2 &callback, NeuronSWC p0,QString braindir)
{
    QDir all_braindir(braindir);
    QFileInfoList list_braindir = all_braindir.entryInfoList(QStringList(),QDir::Dirs|QDir::NoDotAndDotDot);
    int dir_count = list_braindir.size();

    map<int,int> sizemap;

    for(int i=0; i<dir_count; ++i)
    {
//        qDebug()<<i<<": "<<list_braindir[i].absoluteFilePath();
        QString t = list_braindir[i].baseName();
        QStringList ts =t.split('x');
        int resolution = ts[1].toInt();
        sizemap[resolution] = i;
    }

    QVector<QFileInfo> vtmp;
    map<int,int>::iterator it =sizemap.begin();

    while(it!=sizemap.end())
    {
        int index = it->second;
        vtmp.push_back(list_braindir[index]);
        it++;
    }

    list_braindir.clear();
    for(int i=0; i<dir_count; ++i)
    {
        list_braindir.push_back(vtmp[i]);
//        qDebug()<<list_braindir[i].absoluteFilePath();
    }

    QString current_braindir = list_braindir[dir_count-3].absoluteFilePath();

    qDebug()<<current_braindir;

           NeuronSWC p1;
           int r=10;
           V3DLONG *in_zz = 0;
           if(!callback.getDimTeraFly(current_braindir.toStdString().c_str(),in_zz)){cout<<"can't load terafly img"<<endl;}
           V3DLONG start_x,start_y,start_z,end_x,end_y,end_z;
           start_x=p0.x-r; if(start_x<0){start_x=0;} if(start_x>=in_zz[0]){start_x=in_zz[0]-1;cout<<"x axis over border"<<endl;}
           start_y=p0.y-r;if(start_y<0){start_y=0;}if(start_y>=in_zz[1]){start_y=in_zz[1]-1;cout<<"y axis over border"<<endl; }
           start_z=p0.z-r;if(start_z<0){start_z=0;}if(start_z>=in_zz[2]){start_z=in_zz[2]-1;cout<<"z axis over border"<<endl; }
           end_x=p0.x+r;if(end_x>in_zz[0]){end_x=in_zz[0]-1;}
           end_y=p0.y+r;if(end_y>in_zz[1]){end_y=in_zz[1]-1;}
           end_z=p0.z+r;if(end_z>in_zz[2]){end_z=in_zz[2]-1;}
           unsigned char * im_cropped = 0;
           V3DLONG *in_sz = new V3DLONG[4];
           if(end_x<=start_x){cout<<"over border"<<endl; return p0;}
           if(end_y<=start_y){cout<<"over border"<<endl;return p0;}
           if(end_z<=start_z){cout<<"over border"<<endl;return p0;}
           in_sz[0] = end_x-start_x+1;
           in_sz[1] = end_y-start_y+1;
           in_sz[2] = end_z-start_z+1;
           in_sz[3]=in_zz[3];
           V3DLONG pagesz;
           pagesz = in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];
           try {im_cropped = new unsigned char [pagesz];}
           catch(...)  {cout<<"cannot allocate memory for image_mip."<<endl;}
           cout<<"2"<<endl;
           im_cropped = callback.getSubVolumeTeraFly(current_braindir.toStdString().c_str(),start_x,end_x+1,start_y,end_y+1,start_z,end_z+1);
           cout<<"1"<<endl;
           if(im_cropped==NULL){
               cout<<"3"<<endl;}
           V3DLONG y_offset=in_sz[0];
           V3DLONG z_offset=in_sz[0]*in_sz[1];
           V3DLONG windowradius=r-1;
           V3DLONG pos;
           V3DLONG pos0;
   //        float thresHold =100;
           float total_x,total_y,total_z,v_color,sum_v,thresHold,mean,std;
           total_x=0;
           total_y=0;
           total_z=0;
           v_color=0;
           sum_v=0;
           mean=0;
           std=0;
           pos0=p0.z*z_offset+p0.y*y_offset+p0.x;
//center_intensity[i]=im_cropped[pos0];
           for(int j=0; j<pagesz;j++){
                  mean=mean+im_cropped[j];
           }
           mean=mean/pagesz;
           for(int j=0; j<pagesz;j++){
                  std=std+(mean-im_cropped[j])*(mean-im_cropped[j]);
           }
           std=sqrt(std)/pagesz;
           thresHold=mean+std;
           if(thresHold<=100){thresHold=100;}
           for(V3DLONG dx=p0.x+0.5-start_x-windowradius; dx<=p0.x+0.5-start_x+windowradius; dx++){
               for(V3DLONG dy=p0.y+0.5-start_y-windowradius; dy<=p0.y+0.5-start_y+windowradius; dy++){
                   for(V3DLONG dz=p0.z+0.5-start_z-windowradius; dz<=p0.z+0.5-start_z+windowradius; dz++){
                       pos=dz*z_offset+dy*y_offset+dx;
   //                    double tmp=(dx-s.x)*(dx-s.x)+(dy-s.y)*(dy-s.y)
   //                         +(dz-s.z)*(dz-s.z);
   //                    double distance=sqrt(tmp);
   //                    if (distance>windowradius) continue;
                       v_color=im_cropped[pos];
                       //cout<<v_color<<endl;
                       if(v_color<thresHold)
                           v_color=0;
                       total_x=v_color*(float)dx+total_x;
                       total_y=v_color*(float)dy+total_y;
                       total_z=v_color*(float)dz+total_z;
                       sum_v=sum_v+v_color;
                    }
                }
            }
           if(sum_v>0){
               p1.x=total_x/sum_v+start_x;
               p1.y=total_y/sum_v+start_y;
               p1.z=total_z/sum_v+start_z;
           }else{
               p1=p0;
           }
           if(im_cropped) {delete []im_cropped; im_cropped=0;}
       return p1;
}

