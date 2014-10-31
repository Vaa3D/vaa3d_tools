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

using namespace std;
#define INF 1E9

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
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void autotrace_mst(V3DPluginCallback2 &callback, QWidget *parent);
template <class T> QList<NeuronSWC> seed_detection(T* data1d,
                                      V3DLONG *in_sz,
                                      unsigned int Ws,
                                      unsigned int c,
                                      double th);


void neurontracing_mst::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("tracing"))
	{
        autotrace_mst(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2014-10-30"));
	}
}

bool neurontracing_mst::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void autotrace_mst(V3DPluginCallback2 &callback, QWidget *parent)
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


    unsigned char* data1d = p4DImage->getRawData();
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    V3DLONG pagesz = N*M*P;

    V3DLONG in_sz[4];
    in_sz[0] = N;
    in_sz[1] = M;
    in_sz[2] = P;
    in_sz[3] = sc;

    bool ok1,ok2;
    int c, Ws;

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
   // writeSWC_file("mst.swc",nt_tmp);
   // return;
    LocationSimple p0;
    vector<LocationSimple> pp;
    NeuronTree nt;
    V3DLONG sz_tracing[4];
    sz_tracing[0] = in_sz[0];
    sz_tracing[1] = in_sz[1];
    sz_tracing[2] = in_sz[2];
    sz_tracing[3] = 1;

    unsigned char ****p4d = 0;
    if (!new4dpointer(p4d, sz_tracing[0], sz_tracing[1], sz_tracing[2], sz_tracing[3], data1d))
    {
        fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
        return;
    }

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
        NeuronSWC S = nt_seed.at(i);
        p0.x = nt_seed.at(S.pn-1).x - 1;
        p0.y = nt_seed.at(S.pn-1).y - 1;
        p0.z = nt_seed.at(S.pn-1).z - 1;
        pp.clear();
        LocationSimple tmpp;
        tmpp.x = S.x - 1;
        tmpp.y = S.y - 1;
        tmpp.z = S.z -1;
        pp.push_back(tmpp);
     //   v3d_msg(QString("%1,%2,%3,%4,%5,%6").arg(p0.x).arg(p0.y).arg(p0.z).arg(tmpp.x).arg(tmpp.y).arg(tmpp.z));
        nt = v3dneuron_GD_tracing(p4d, sz_tracing,
                                  p0, pp,
                                  trace_para, weight_xy_z);
        for(int j = nt.listNeuron.size()-1; j >=0;j--)
        {
            S_GD = nt.listNeuron.at(j);
            S_GD.n = id;
            if(S_GD.pn > 0) S_GD.pn = id -1;
            listNeuron.append(S_GD);
            hashNeuron.insert(S_GD.n, listNeuron.size()-1);
            id++;
         }
   //     QString outfilename = QString("test/mst_%2.swc").arg(i);
   //     writeSWC_file(outfilename,nt);
    }

    nt_final.n = -1;
    nt_final.on = true;
    nt_final.listNeuron = listNeuron;
    nt_final.hashNeuron = hashNeuron;
    QString swc_name = p4DImage->getFileName();
    swc_name.append("_MST_Tracing.swc");
    writeSWC_file(swc_name.toStdString().c_str(),nt_final);
    bool bmenu = 1;
    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

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
                            double PixelVaule = w +th;
                            th_local+= PixelVaule;
                            i++;
                            if (w > 30)
                            {
                                xm += w*i;
                                ym += w*j;
                                zm += w*k;
                                s += w;
                                n = n+1;
                            }
                        }
                    }
                }
                th_local = th_local/i;
                xm /= s; ym /=s; zm /=s;
                V3DLONG seed_index = (int)zm*M*N + (int)ym*N +(int)xm;
                if(s >0 && data1d[seed_index] > th+10 && th_local>th+10)
                {

                   // V3DLONG seed_index = (int)zm*M*N + (int)ym*N +(int)xm;
                   // pImage[seed_index] = 255;
                    ImageMarker MARKER;
                    MARKER.x = xm+1;
                    MARKER.y = ym+1;
                    MARKER.z = zm+1;
                    seeds.append(MARKER);
                }
            }
        }
    }

    V3DLONG marknum = seeds.size();

    double** markEdge = new double*[marknum];
    for(int i = 0; i < marknum; i++)
    {
        markEdge[i] = new double[marknum];

    }

    double x1,y1,z1;
    for (int i=0;i<marknum;i++)
    {
        x1 = seeds.at(i).x;
        y1 = seeds.at(i).y;
        z1 = seeds.at(i).z;
        for (int j=0;j<marknum;j++)
        {
            markEdge[i][j] = sqrt(double(x1-seeds.at(j).x)*double(x1-seeds.at(j).x) + double(y1-seeds.at(j).y)*double(y1-seeds.at(j).y) + double(z1-seeds.at(j).z)*double(z1-seeds.at(j).z));
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
   // writeSWC_file("mst.swc",marker_MST);
    QList<NeuronSWC> marker_MST_sorted;
    if (SortSWC(marker_MST.listNeuron, marker_MST_sorted ,1, 0))
    return marker_MST_sorted;
  //  outimg = pImage;

}
