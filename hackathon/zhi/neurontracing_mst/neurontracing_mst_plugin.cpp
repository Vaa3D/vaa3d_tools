/* neurontracing_mst_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-10-30 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neurontracing_mst_plugin.h"

#include "basic_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h""
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "stackutil.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.h"



using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))

#define INF 1E9

struct MST_PARA
{
    QString inimg_file;
    V3DLONG channel;
    V3DLONG Ws;
};

Q_EXPORT_PLUGIN2(neurontracing_mst, neurontracing_mst);
 
QStringList neurontracing_mst::menulist() const
{
	return QStringList() 
        <<tr("tracing")
		<<tr("about");
}

QStringList neurontracing_mst::funclist() const
{
	return QStringList()
        <<tr("trace_mst")
		<<tr("help");
}

void autotrace_mst(V3DPluginCallback2 &callback, QWidget *parent, MST_PARA &PARA, bool bmenu);
template <class T> QList<NeuronSWC> seed_detection(T* data1d,
                                      V3DLONG *in_sz,
                                      unsigned int Ws,
                                      unsigned int c,
                                      double th);

template <class T> T pow2(T a)
{
    return a*a;

}
NeuronTree post_process(NeuronTree nt);

void neurontracing_mst::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("tracing"))
    {
        bool bmenu = true;
        MST_PARA PARA;
        autotrace_mst(callback,parent,PARA,bmenu);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2014-10-30"));
	}
}

bool neurontracing_mst::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{

    if (func_name == tr("trace_mst"))
	{
        bool bmenu = false;
        MST_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        PARA.Ws = (paras.size() >= k+1) ? atoi(paras[k]) : 10;  k++;
        autotrace_mst(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {
        printf("\n**** Usage of MST tracing ****\n");
        printf("vaa3d -x plugin_name -f trace_mst -i <inimg_file> -p <channel> <window size>\n");
        printf("inimg_file       Should be 8 bit image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("window size      Window size for seed detection. Default 10\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

void autotrace_mst(V3DPluginCallback2 &callback, QWidget *parent, MST_PARA &PARA, bool bmenu)
{

    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c,Ws;
    V3DLONG *in_sz = 0;
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }


        data1d = p4DImage->getRawData();
        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        sc = p4DImage->getCDim();

        bool ok1,ok2;

        if(sc==1)
        {
            c=1;
            ok1=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, sc, 1, &ok1);
        }

        if(ok1)
        {
            Ws = QInputDialog::getInteger(parent, "Window size ",
                                          "Enter window size:",
                                          10, 1, N, 1, &ok2);
        }

        if(!ok2)
            return;

        in_sz = new V3DLONG[4];
        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;

        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (loadImage(const_cast<char *>(PARA.inimg_file.toStdString().c_str()), data1d, in_sz, datatype)!=true)
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }

        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;
        Ws = PARA.Ws;
    }


    V3DLONG pagesz = N*M*P;
    double th = 0;
    V3DLONG offsetc = (c-1)*pagesz;

    for(V3DLONG iz = 0; iz < P; iz++)
    {
        double PixelSum = 0;
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy <  M; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < N; ix++)
            {

                double PixelVaule = data1d[offsetc+offsetk + offsetj + ix];
                PixelSum = PixelSum + PixelVaule;
            }
        }
        th += PixelSum/(M*N*P);
    }


    QList<NeuronSWC> nt_seed = seed_detection(data1d, in_sz, Ws, c, th);
    NeuronTree nt_tmp;
    nt_tmp.listNeuron = nt_seed;
    writeSWC_file("mst.swc",nt_tmp);
   // return;
    LocationSimple p0;
    vector<LocationSimple> pp;
    NeuronTree nt;

    V3DLONG channelNo_subject=0;
    V3DLONG *channelsToUse=0;
    V3DLONG nChannelsToUse=0;
    double weight_xy_z=1.0;
    bool b_useEntireImg = false;
    bool b_mergeCloseBranches = false;
    bool b_usedshortestpathonly = false;
    bool b_postTrim = true;
    bool b_pruneArtifactBranches = true;
    int ds_step = 2;
    int medianf_rr = 0;
    double th_global = 0;
    int traceOnDTImg_method = 0; //default do NOT use DT image for tracing

    bool b_produceProjectionPatterns = false;


    CurveTracePara trace_para;
    trace_para.channo = 0;
    trace_para.sp_graph_resolution_step = ds_step;
    trace_para.b_deformcurve = b_usedshortestpathonly;
    trace_para.b_postMergeClosebyBranches = b_mergeCloseBranches;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = b_postTrim;
    trace_para.b_pruneArtifactBranches = b_pruneArtifactBranches;

    NeuronTree nt_final;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    NeuronSWC S_GD;

    V3DLONG id = 1;
    for(V3DLONG i = 1; i <nt_seed.size(); i++)
    {
       // v3d_msg(QString("mst_%1.swc").arg(i));
        NeuronSWC S = nt_seed.at(i);
        p0.x = nt_seed.at(S.pn-1).x;
        p0.y = nt_seed.at(S.pn-1).y;
        p0.z = nt_seed.at(S.pn-1).z;
        pp.clear();
        LocationSimple tmpp;
        tmpp.x = S.x;
        tmpp.y = S.y;
        tmpp.z = S.z;
       // pp.push_back(tmpp);

        double cent_x =  0.5 * (p0.x + tmpp.x);
        double cent_y =  0.5 * (p0.y + tmpp.y);
        double cent_z =  0.5 * (p0.z + tmpp.z);

        V3DLONG xb = cent_x - 2*Ws; if(xb < 0) xb = 0;
        V3DLONG xe = cent_x + 2*Ws-1; if(xe>N-1) xe = N-1;
        V3DLONG yb = cent_y - 2*Ws; if(yb < 0) yb = 0;
        V3DLONG ye = cent_y + 2*Ws-1; if(ye>M-1) ye = M-1;
        V3DLONG zb = cent_z - 2*Ws; if(zb < 0) zb = 0;
        V3DLONG ze = cent_z + 2*Ws-1; if(ze>P-1) ze = P-1;

        unsigned char *localarea=0;
        V3DLONG blockpagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
        localarea = new unsigned char [blockpagesz];
        V3DLONG d = 0;
        for(V3DLONG iz = zb; iz < ze + 1; iz++)
        {
            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = yb; iy < ye+1; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = xb; ix < xe+1; ix++)
                {

                    localarea[d] = data1d[offsetc+offsetk + offsetj + ix];
                    d++;
                }
            }
        }

        V3DLONG sz_tracing[4];
        sz_tracing[0] = xe-xb+1;
        sz_tracing[1] = ye-yb+1;
        sz_tracing[2] = ze-zb+1;
        sz_tracing[3] = 1;

        unsigned char ****p4d = 0;
        if (!new4dpointer(p4d, sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3], localarea))
        {
            fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
            return;
        }


        p0.x = p0.x - xb;
        p0.y = p0.y - yb;
        p0.z = p0.z - zb;
        tmpp.x = S.x - xb;
        tmpp.y = S.y - yb;
        tmpp.z = S.z - zb;
        pp.push_back(tmpp);

        nt = v3dneuron_GD_tracing(p4d, sz_tracing,
                                  p0, pp,
                                  trace_para, weight_xy_z);

        if(p4d) {delete []p4d; p4d = 0;}
        if(localarea) {delete []localarea; localarea = 0;}

        if(nt.listNeuron.size()<1)
            continue;

        for(int j = nt.listNeuron.size()-1; j >=0;j--)
        {
            S_GD = nt.listNeuron.at(j);
            S_GD.x = S_GD.x + xb;
            S_GD.y = S_GD.y + yb;
            S_GD.z = S_GD.z + zb;


            S_GD.n = id;
            if(S_GD.pn > 0) S_GD.pn = id -1;
            listNeuron.append(S_GD);
            hashNeuron.insert(S_GD.n, listNeuron.size()-1);
            id++;
         }
   //
   //     writeSWC_file(outfilename,nt);
    }

    nt_final.n = -1;
    nt_final.on = true;
    nt_final.listNeuron = listNeuron;
    nt_final.hashNeuron = hashNeuron;

    QString swc_name = PARA.inimg_file + "_MST_Tracing.swc";

    writeSWC_file(swc_name,nt_final);

    if(in_sz) {delete []in_sz; in_sz = 0;}
    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;

    V3DPluginArgItem arg;
    V3DPluginArgList input_sort;
    V3DPluginArgList output;

    arg.type = "random";std::vector<char*> arg_input_resample;
    std:: string fileName_Qstring(swc_name.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());
    arg_input_resample.push_back(fileName_string);
    arg.type = "random";std::vector<char*> arg_input_sort;
    arg_input_sort.push_back(fileName_string);
    arg.p = (void *) & arg_input_sort; input_sort<< arg;
    arg.type = "random";std::vector<char*> arg_sort_para;arg.p = (void *) & arg_sort_para; input_sort << arg;
    arg.type = "random";std::vector<char*> arg_output;arg_output.push_back(fileName_string); arg.p = (void *) & arg_output; output<< arg;

    QString full_plugin_name_sort = "sort_neuron_swc";
    QString func_name_sort = "sort_swc";
    callback.callPluginFunc(full_plugin_name_sort,func_name_sort, input_sort,output);

    return;
    //writeSWC_file(swc_name.toStdString().c_str(),nt_sorted);

    NeuronTree nt_sorted = readSWC_file(swc_name);
    NeuronTree nt_sorted_prund = post_process(nt_sorted);
    NeuronTree nt_sorted_prund_2nd = post_process(nt_sorted_prund);

    p0.x = nt_sorted_prund_2nd.listNeuron.at(0).x;
    p0.y = nt_sorted_prund_2nd.listNeuron.at(0).y;
    p0.z = nt_sorted_prund_2nd.listNeuron.at(0).z;
    pp.clear();

    QVector<QVector<V3DLONG> > childs;

    V3DLONG neuronNum = nt_sorted_prund_2nd.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt_sorted_prund_2nd.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt_sorted_prund_2nd.hashNeuron.value(par)].push_back(i);
    }


    QList<NeuronSWC> list = nt_sorted_prund_2nd.listNeuron;
    for (int i=1;i<list.size();i++)
    {
        if (childs[i].size()==0)
        {
            LocationSimple tmpp;
            tmpp.x = list.at(i).x;
            tmpp.y = list.at(i).y;
            tmpp.z = list.at(i).z;
            pp.push_back(tmpp);
        }
    }

    V3DLONG sz_tracing[4];
    sz_tracing[0] = in_sz[0];
    sz_tracing[1] = in_sz[1];
    sz_tracing[2] = in_sz[2];
    sz_tracing[3] = 1;

    unsigned char ****p4d_entire = 0;
    if (!new4dpointer(p4d_entire, sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3], data1d))
    {
        fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
        return;
    }

   // trace_para.b_postMergeClosebyBranches = true;
    NeuronTree nt_2nd = v3dneuron_GD_tracing(p4d_entire, sz_tracing,
                              p0, pp,
                              trace_para, weight_xy_z);
    if(p4d_entire) {delete []p4d_entire; p4d_entire = 0;}

    NeuronTree nt_2nd_sorted;
    SortSWC(nt_2nd.listNeuron, nt_2nd_sorted.listNeuron ,1, 5);

    nt_2nd_sorted.name = "MST_Tracing";
    writeSWC_file(swc_name.toStdString().c_str(),nt_2nd_sorted);


    return;

}

template <class T> QList<NeuronSWC> seed_detection(T* data1d,
                                      V3DLONG *in_sz,
                                      unsigned int Ws,
                                      unsigned int c,
                                      double th)
{
    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG sc = in_sz[3];
    V3DLONG pagesz = N*M*P;

    V3DLONG offsetc = (c-1)*pagesz;

    double w;
    QList <ImageMarker> seeds;
    QList <ImageMarker> loc_points_list;

    printf("\nDetecting seed location ...\n");
    for(V3DLONG iz = 0; iz < P; iz = iz + Ws)
    {
      //  V3DLONG offsetk = iz*M*N;
        V3DLONG zb = iz;
        V3DLONG ze = iz+Ws-1; if(ze>=P-1) ze = P-1;
        for(V3DLONG iy = 0; iy < M; iy = iy + Ws)
        {
          //  V3DLONG offsetj = iy*N;
            V3DLONG yb = iy;
            V3DLONG ye = iy+Ws-1; if(ye>=M-1) ye = M-1;
            for(V3DLONG ix = 0; ix < N; ix = ix + Ws)
            {
                V3DLONG xb = ix;
                V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;
                //now get the center of mass
                double th_local = 0;
                double xm=0,ym=0,zm=0, s=0, n=0;
                V3DLONG i = 0;
                for(V3DLONG k=zb; k<=ze; k++)
                {
                    V3DLONG offsetkl = k*M*N;
                    for(V3DLONG j=yb; j<=ye; j++)
                    {
                        V3DLONG offsetjl = j*N;
                        for(V3DLONG i=xb; i<=xe; i++)
                        {
                            w = double(data1d[offsetc+offsetkl + offsetjl + i]) - th;
                            if (w >0)  //30
                            {
                                xm += w*i;
                                ym += w*j;
                                zm += w*k;
                                s += w;
                                n = n+1;
                                if(w > th + 30)
                                {
                                    ImageMarker local_point;
                                    local_point.x = i;
                                    local_point.y = j;
                                    local_point.z = k;
                                    loc_points_list.append(local_point);
                                }
                            }
                        }
                    }
                }
                xm /= s; ym /=s; zm /=s;
                V3DLONG seed_index = (int)zm*M*N + (int)ym*N +(int)xm;
                if(s >0 && data1d[seed_index] <= th + 30 && loc_points_list.size()>1) //find medoid point
                {
                    double dist_min = INF;
                    V3DLONG medoid_index = -1;
                    for(int io = 0; io < loc_points_list.size(); io++)
                    {
                        double dis = 0;
                        for(int jo = 0; jo < loc_points_list.size(); jo++)
                        {
                            dis += sqrt(pow2(loc_points_list.at(io).x - loc_points_list.at(jo).x) + pow2(pow2(loc_points_list.at(io).y - loc_points_list.at(jo).y) + pow2(pow2(loc_points_list.at(io).z - loc_points_list.at(jo).z))));
                        }
                        if(dis < dist_min)
                        {
                            dist_min = dis;
                            medoid_index = io;
                        }
                    }
                    seed_index = (int)loc_points_list.at(medoid_index).z*M*N + (int)loc_points_list.at(medoid_index).y*N +(int)loc_points_list.at(medoid_index).x;
                    xm = loc_points_list.at(medoid_index).x;
                    ym = loc_points_list.at(medoid_index).y;
                    zm = loc_points_list.at(medoid_index).z;
                   // printf("\n(%.4f,%.4f,%.4f)",xm,ym,zm);

                }
                loc_points_list.clear();

                if(s >0 && data1d[seed_index] > th + 30)
                {
                    ImageMarker MARKER;
                    MARKER.x = xm;
                    MARKER.y = ym;
                    MARKER.z = zm;
                    seeds.append(MARKER);
                }

            }
        }
    }


    printf("\nGenerating Minimum Spanning Tree (MST) for all seed locations ...\n");

    V3DLONG marknum = seeds.size();

    double** markEdge = new double*[marknum];
    for(int i = 0; i < marknum; i++)
    {
        markEdge[i] = new double[marknum];

    }

//    double x1,y1,z1;
//    for (int i=0;i<marknum;i++)
//    {
//        x1 = seeds.at(i).x;
//        y1 = seeds.at(i).y;
//        z1 = seeds.at(i).z;
//        for (int j=0;j<marknum;j++)
//        {
//            markEdge[i][j] = sqrt(double(x1-seeds.at(j).x)*double(x1-seeds.at(j).x) + double(y1-seeds.at(j).y)*double(y1-seeds.at(j).y) + double(z1-seeds.at(j).z)*double(z1-seeds.at(j).z));
//        }
//    }



    double weight_xy_z=1.0;
    bool b_mergeCloseBranches = false;
    bool b_usedshortestpathonly = false;
    bool b_postTrim = true;
    bool b_pruneArtifactBranches = true;
    int ds_step = 2;

    CurveTracePara trace_para;
    trace_para.channo = 0;
    trace_para.sp_graph_resolution_step = ds_step;
    trace_para.b_deformcurve = b_usedshortestpathonly;
    trace_para.b_postMergeClosebyBranches = b_mergeCloseBranches;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = b_postTrim;
    trace_para.b_pruneArtifactBranches = b_pruneArtifactBranches;

    #define X_I(i)				 	(0+(i)*ds_step)
    #define Y_I(i)				 	(0+(i)*ds_step)
    #define Z_I(i)				 	(0+(i)*ds_step)

    LocationSimple p0;
    vector<LocationSimple> pp;
    NeuronTree nt;

    double x1,y1,z1;
    for (int i=0;i<marknum;i++)
    {
        x1 = seeds.at(i).x;
        y1 = seeds.at(i).y;
        z1 = seeds.at(i).z;
        for (int j=i;j<marknum;j++)
        {
            double x2 = seeds.at(j).x;
            double y2 = seeds.at(j).y;
            double z2 = seeds.at(j).z;
            if(sqrt(double(x1-x2)*double(x1-x2) + double(y1-y2)*double(y1-y2) + double(z1-z2)*double(z1-z2)) > 2*Ws)
                 markEdge[i][j] = INF;
            else
            {

                p0.x = x1;
                p0.y = y1;
                p0.z = z1;
                pp.clear();

                LocationSimple tmpp;
                tmpp.x = x2;
                tmpp.y = y2;
                tmpp.z = z2;
             //   pp.push_back(tmpp);
                double cent_x =  0.5 * (p0.x + tmpp.x);
                double cent_y =  0.5 * (p0.y + tmpp.y);
                double cent_z =  0.5 * (p0.z + tmpp.z);

                V3DLONG xb = cent_x - 2*Ws; if(xb < 0) xb = 0;
                V3DLONG xe = cent_x + 2*Ws-1; if(xe>N-1) xe = N-1;
                V3DLONG yb = cent_y - 2*Ws; if(yb < 0) yb = 0;
                V3DLONG ye = cent_y + 2*Ws-1; if(ye>M-1) ye = M-1;
                V3DLONG zb = cent_z - 2*Ws; if(zb < 0) zb = 0;
                V3DLONG ze = cent_z + 2*Ws-1; if(ze>P-1) ze = P-1;

                unsigned char *localarea=0;
                V3DLONG blockpagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
                localarea = new unsigned char [blockpagesz];
                V3DLONG d = 0;
                for(V3DLONG iz = zb; iz < ze + 1; iz++)
                {
                    V3DLONG offsetk = iz*M*N;
                    for(V3DLONG iy = yb; iy < ye+1; iy++)
                    {
                        V3DLONG offsetj = iy*N;
                        for(V3DLONG ix = xb; ix < xe+1; ix++)
                        {

                            localarea[d] = data1d[offsetc+offsetk + offsetj + ix];
                            d++;
                        }
                    }
                }

                V3DLONG sz_tracing[4];
                sz_tracing[0] = xe-xb+1;
                sz_tracing[1] = ye-yb+1;
                sz_tracing[2] = ze-zb+1;
                sz_tracing[3] = 1;

                unsigned char ****p4d = 0;
                if (!new4dpointer(p4d, sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3], localarea))
                {
                    fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
                }


                p0.x = p0.x - xb;
                p0.y = p0.y - yb;
                p0.z = p0.z - zb;
                tmpp.x = x2 - xb;
                tmpp.y = y2 - yb;
                tmpp.z = z2 - zb;
                pp.push_back(tmpp);

                nt = v3dneuron_GD_tracing(p4d, sz_tracing,
                                          p0, pp,
                                          trace_para, weight_xy_z);



                if(nt.listNeuron.size() >0)
                {
                    unsigned char ***img3d = p4d[0];
                    double intensity_sum = 0;

                    for(V3DLONG nn = 0; nn < nt.listNeuron.size() ;nn++)
                    {

                        V3DLONG ii = nt.listNeuron.at(nn).x;
                        V3DLONG jj = nt.listNeuron.at(nn).y;;
                        V3DLONG kk = nt.listNeuron.at(nn).z;

                        double va = getBlockAveValue(img3d, sz_tracing[0], sz_tracing[1], sz_tracing[2], ii,jj,kk,
                                                     ds_step, ds_step, (ds_step/weight_xy_z));
                        double tmpv = 1-va/255;
                        intensity_sum =  intensity_sum + exp((tmpv*tmpv)*10);

                    }
                    markEdge[i][j] =  intensity_sum;

                }

                if(p4d) {delete []p4d; p4d = 0;}
                if(localarea) {delete []localarea; localarea = 0;}
            }

        }
    }

    for (int i=0;i<marknum;i++)
    {
        for (int j=i;j<marknum;j++)
        {
            markEdge[j][i] =  markEdge[i][j];
        }
    }
     //NeutronTree structure
    NeuronTree marker_MST;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    //set node

    NeuronSWC S;
    S.n 	= 1;
    S.type 	= 3;
    S.x 	= seeds.at(0).x;
    S.y 	= seeds.at(0).y;
    S.z 	= seeds.at(0).z;
    S.r 	= 1;
    S.pn 	= -1;
    listNeuron.append(S);
    hashNeuron.insert(S.n, listNeuron.size()-1);

    int* pi = new int[marknum];
    for(int i = 0; i< marknum;i++)
        pi[i] = 0;
    pi[0] = 1;
    int indexi,indexj;
    for(int loop = 0; loop<marknum;loop++)
    {
        double min = INF;
        for(int i = 0; i<marknum; i++)
        {
            if (pi[i] == 1)
            {
                for(int j = 0;j<marknum; j++)
                {
                    if(pi[j] == 0 && min > markEdge[i][j])
                    {
                        min = markEdge[i][j];
                        indexi = i;
                        indexj = j;
                    }
                }
            }

        }
        if(indexi>=0)
        {
            S.n 	= indexj+1;
            S.type 	= 7;
            S.x 	= seeds.at(indexj).x;
            S.y 	= seeds.at(indexj).y;
            S.z 	= seeds.at(indexj).z;
            S.r 	= 1;
            S.pn 	= indexi+1;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

        }else
        {
            break;
        }
        pi[indexj] = 1;
        indexi = -1;
        indexj = -1;
    }
    marker_MST.n = -1;
    marker_MST.on = true;
    marker_MST.listNeuron = listNeuron;
    marker_MST.hashNeuron = hashNeuron;

    if(markEdge) {delete []markEdge, markEdge = 0;}
  //  writeSWC_file("mst.swc",marker_MST);
    QList<NeuronSWC> marker_MST_sorted;
    if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))
    return marker_MST_sorted;
  //  outimg = pImage;

}

NeuronTree  post_process(NeuronTree nt)
{

    double length = 5.0;
    QVector<QVector<V3DLONG> > childs;

    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    V3DLONG *flag = new V3DLONG[neuronNum];

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        flag[i] = 1;

        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    QList<NeuronSWC> list = nt.listNeuron;
    for (int i=0;i<list.size();i++)
    {
        if (childs[i].size()==0)
        {
            int index_tip = 0;
            int parent_tip = getParent(i,nt);
            while(childs[parent_tip].size()<2)
            {

                parent_tip = getParent(parent_tip,nt);
                index_tip++;
            }
            if(index_tip < length)
            {
                flag[i] = -1;

                int parent_tip = getParent(i,nt);
                while(childs[parent_tip].size()<2)
                {
                    flag[parent_tip] = -1;
                    parent_tip = getParent(parent_tip,nt);
                }
            }

        }

    }

   //NeutronTree structure
   NeuronTree nt_prunned;
   QList <NeuronSWC> listNeuron;
   QHash <int, int>  hashNeuron;
   listNeuron.clear();
   hashNeuron.clear();

   //set node
   NeuronSWC S;
   for (int i=0;i<list.size();i++)
   {
       if(flag[i] == 1)
       {
            NeuronSWC curr = list.at(i);
            S.n 	= curr.n;
            S.type = curr.type;
            S.r 	= curr.r;
            S.x 	= curr.x;
            S.y 	= curr.y;
            S.z 	= curr.z;
            S.pn 	= curr.pn;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
       }

  }
   nt_prunned.n = -1;
   nt_prunned.on = true;
   nt_prunned.listNeuron = listNeuron;
   nt_prunned.hashNeuron = hashNeuron;

   if(flag) {delete[] flag; flag = 0;}
   return nt_prunned;

}
