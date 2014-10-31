/* neurontracing_mst_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-10-30 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neurontracing_mst_plugin.h"

#include "basic_surf_objs.h"

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
template <class T> void seed_detection(T* data1d,
                                      V3DLONG *in_sz,
                                      unsigned int Ws,
                                      unsigned int c,
                                      double th,
                                      T* &outimg);


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
                                      1, 1, N, 1, &ok2);
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

    void* outimg = 0;
    ImagePixelType pixeltype = p4DImage->getDatatype();

    switch (pixeltype)
    {
    case V3D_UINT8: seed_detection(data1d, in_sz, Ws, c, th, (unsigned char*&)outimg); break;
    default: v3d_msg("Invalid data type. Do nothing."); return;
    }

    // display
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)outimg,N, M, P, 1, V3D_UINT8);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, "Seed locations");
    callback.updateImageWindow(newwin);
    return;

}

template <class T> void seed_detection(T* data1d,
                                      V3DLONG *in_sz,
                                      unsigned int Ws,
                                      unsigned int c,
                                      double th,
                                      T* &outimg)
{
    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG sc = in_sz[3];
    V3DLONG pagesz = N*M*P;

    //filtering
    V3DLONG offsetc = (c-1)*pagesz;

    //declare temporary pointer
    T *pImage = new T [pagesz];
    if (!pImage)
    {
        printf("Fail to allocate memory.\n");
        return;
    }
    else
    {
        for(V3DLONG i=0; i<pagesz; i++)
            pImage[i] = 0;
    }

    //Median Filtering
    double w;

    QList <ImageMarker> seeds;
    v3d_msg(QString("%1").arg(th));
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
                double xm=0,ym=0,zm=0, s=0, n=0;
                for(V3DLONG k=zb; k<=ze; k++)
                {
                    V3DLONG offsetkl = k*M*N;
                    for(V3DLONG j=yb; j<=ye; j++)
                    {
                        V3DLONG offsetjl = j*N;
                        for(V3DLONG i=xb; i<=xe; i++)
                        {
                            w = double(data1d[offsetc+offsetkl + offsetjl + i]) - th;
                            if (w > 50)
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
                if(s >0)
                {
                    xm /= s; ym /=s; zm /=s;
                    V3DLONG seed_index = (int)zm*M*N + (int)ym*N +(int)xm;
                    pImage[seed_index] = 255;
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
            markEdge[i][j] = sqrt(double(x1-seeds.at(i).x)*double(x1-seeds.at(i).x) + double(y1-seeds.at(i).y)*double(y1-seeds.at(i).y) + double(z1-seeds.at(i).z)*double(z1-seeds.at(i).z));
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
    writeSWC_file("mst.swc",marker_MST);
    outimg = pImage;

}
