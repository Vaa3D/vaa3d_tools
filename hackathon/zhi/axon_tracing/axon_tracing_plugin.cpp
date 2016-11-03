/* axon_tracing_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-9-29 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "axon_tracing_plugin.h"
#include "../jba/c++/histeq.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h""
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "stackutil.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.h"
#include <boost/lexical_cast.hpp>



Q_EXPORT_PLUGIN2(axon_tracing, axon_tracing);

using namespace std;
using namespace boost;

#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))

#define INF 1E29

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);

template <class T> QList<NeuronSWC> seed_detection(V3DPluginCallback2 &callback,T* data1d,
                                      V3DLONG *in_sz,
                                      unsigned int Ws,
                                      unsigned int c,
                                      double th);

template <class T> T pow2(T a)
{
    return a*a;

}

template <class T>
void BinaryProcess(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d)
{
    V3DLONG i, j,k,n,count;
    double t, temp;

    V3DLONG mCount = iImageHeight * iImageWidth;
    for (i=0; i<iImageLayer; i++)
    {
        for (j=0; j<iImageHeight; j++)
        {
            for (k=0; k<iImageWidth; k++)
            {
                V3DLONG curpos = i * mCount + j*iImageWidth + k;
                V3DLONG curpos1 = i* mCount + j*iImageWidth;
                V3DLONG curpos2 = j* iImageWidth + k;
                temp = 0;
                count = 0;
                for(n =1 ; n <= d  ;n++)
                {
                    if (k>h*n) {temp += apsInput[curpos1 + k-(h*n)]; count++;}
                    if (k+(h*n)< iImageWidth) { temp += apsInput[curpos1 + k+(h*n)]; count++;}
                    if (j>h*n) {temp += apsInput[i* mCount + (j-(h*n))*iImageWidth + k]; count++;}//
                    if (j+(h*n)<iImageHeight) {temp += apsInput[i* mCount + (j+(h*n))*iImageWidth + k]; count++;}//
                    if (i>(h*n)) {temp += apsInput[(i-(h*n))* mCount + curpos2]; count++;}//
                    if (i+(h*n)< iImageLayer) {temp += apsInput[(i+(h*n))* mCount + j* iImageWidth + k ]; count++;}
                }
                t =  apsInput[curpos]-temp/(count);
                aspOutput[curpos]= (t > 0)? t : 0;
            }
        }
    }
}
 
QStringList axon_tracing::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList axon_tracing::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void axon_tracing::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;
        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2016-9-29"));
	}
}

bool axon_tracing::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("tracing_func"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of axon_tracing tracing **** \n");
		printf("vaa3d -x axon_tracing -f tracing_func -i <inimg_file> -p <channel> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c,Ws;
    V3DLONG in_sz[4];
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
                                          30, 1, N, 1, &ok2);
        }

        if(!ok2)
            return;

        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;


        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
        }
        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;
    }


    printf("\nAdaptive thresholding ...\n");

    unsigned char* data1d_th = 0;
    try
    {
        data1d_th = new unsigned char [N*M*P];
    }
        catch (...)
    {
        v3d_msg("Fail to allocate memory in Distance Transform.",0);
        if (data1d_th) {delete []data1d_th; data1d_th=0;}
        return;
    }

    V3DLONG h = 5, d = 3;
    BinaryProcess(data1d, (unsigned char *)data1d_th, N, M, P, h, d  );

    printf("\nhistogram equalization ...\n");

    int lowerbound = 30, higherbound = 255;
    if (!hist_eq_range_uint8( (unsigned char *)data1d_th, N*M*P, lowerbound, higherbound))
    {
        v3d_msg("Error happens in proj_general_hist_equalization();\n");
        return;
    }

//        Image4DSimple p4DImage;
//        p4DImage.setData((unsigned char*)data1d_th, N, M, P, 1, V3D_UINT8);

//        v3dhandle newwin;
//        if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
//            newwin = callback.currentImageWindow();
//        else
//            newwin = callback.newImageWindow();

//        callback.setImage(newwin, &p4DImage);
//        callback.setImageName(newwin, QString("thresholded image"));
//        callback.updateImageWindow(newwin);


//        return;
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

                double PixelVaule = data1d_th[offsetc+offsetk + offsetj + ix];
                PixelSum = PixelSum + PixelVaule;
            }
        }
        th += PixelSum/(M*N*P);
    }

    QList<NeuronSWC> nt_seed = seed_detection(callback,data1d_th, in_sz, Ws, c, th);
    QString tmp_name = PARA.inimg_file + "_mst_only.swc";
    export_list2file(nt_seed,tmp_name,tmp_name);

    return;

    NeuronTree nt_tmp;
    nt_tmp.listNeuron = nt_seed;

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

                    localarea[d] = data1d_th[offsetc+offsetk + offsetj + ix];
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
    }

    nt_final.n = -1;
    nt_final.on = true;
    nt_final.listNeuron = listNeuron;
    nt_final.hashNeuron = hashNeuron;

    if (data1d_th) {delete []data1d_th; data1d_th=0;}

    //Output
    QString swc_name = PARA.inimg_file + "_axon_tracing.swc";
    nt.name = "axon_tracing";
    writeSWC_file(swc_name.toStdString().c_str(),nt_final);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}

template <class T> QList<NeuronSWC> seed_detection(V3DPluginCallback2 &callback,T* data1d,
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

    UndirectedGraph g(marknum);
    double x1,y1,z1;
    for (int i=0;i<marknum;i++)
    {
        x1 = seeds.at(i).x;
        y1 = seeds.at(i).y;
        z1 = seeds.at(i).z;
        for (int j=0;j<marknum;j++)
        {
            EdgeQuery edgeq= edge(i, j, *&g);
            if (!edgeq.second && i!=j)
            {
                int Vedge = sqrt(double(x1-seeds.at(j).x)*double(x1-seeds.at(j).x) + double(y1-seeds.at(j).y)*double(y1-seeds.at(j).y) + double(z1-seeds.at(j).z)*double(z1-seeds.at(j).z));
                add_edge(i, j, LastVoted(i, WeightNew(Vedge)), *&g);
            }
        }
    }

    vector < graph_traits < UndirectedGraph >::vertex_descriptor > p(num_vertices(*&g));
    prim_minimum_spanning_tree(*&g, &p[0]);

    NeuronTree marker_MST;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    for (std::size_t i = 0; i != p.size(); ++i)
    {
        NeuronSWC S;
        int pn;
        if(p[i] == i)
            pn = -1;
        else
            pn = p[i] + 1;

        S.n 	= i+1;
        S.type 	= 7;
        S.x 	= seeds.at(i).x;
        S.y 	= seeds.at(i).y;
        S.z 	= seeds.at(i).z;;
        S.r 	= 1;
        S.pn 	= pn;
        listNeuron.append(S);
        hashNeuron.insert(S.n, listNeuron.size()-1);
    }

    marker_MST.n = -1;
    marker_MST.on = true;
    marker_MST.listNeuron = listNeuron;
    marker_MST.hashNeuron = hashNeuron;


    QList<NeuronSWC> marker_MST_sorted;
    if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))
    return marker_MST_sorted;
  //  outimg = pImage;

}
