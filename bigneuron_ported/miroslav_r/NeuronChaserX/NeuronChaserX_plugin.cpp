/*
Copyright (C) Erasmus MC. Permission to use this software and corresponding documentation for educational, research, and not-for-profit purposes, without a fee and without a signed licensing agreement, is granted, subject to the following terms and conditions.

IT IS NOT ALLOWED TO REDISTRIBUTE, SELL, OR LEASE THIS SOFTWARE, OR DERIVATIVE WORKS THEREOF, WITHOUT PERMISSION IN WRITING FROM THE COPYRIGHT HOLDER. THE COPYRIGHT HOLDER IS FREE TO MAKE VERSIONS OF THE SOFTWARE AVAILABLE FOR A FEE OR COMMERCIALLY ONLY.

IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OF ANY KIND WHATSOEVER, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF ADVISED OF THE POSSIBILITY THEREOF.

THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE EXPRESS OR IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND CORRESPONDING DOCUMENTATION IS PROVIDED "AS IS". THE COPYRIGHT HOLDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include "nf_dialog.h"
#include "toolbox.h"
#include "node.h"
#include "btracer.h"
#include "connected.h"
#include <ctime>
#include <cfloat>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <queue>
#include <climits>
#include <iomanip>

#include "NeuronChaserX_plugin.h"
Q_EXPORT_PLUGIN2(NeuronChaserX, NeuronChaserX);

using namespace std;

// input parameter default values (offered in user menu)
static int      scal    = 12;
static float    bratio  = 0.5;  // background ratio
static int      perc    = 99;   // this one varies between 2d and 3d since it refers to the volume
static float    znccTh  = 0.60; // corr. th
static int      Ndir    = 15;   // directions
static float    angSig  = 60;   // angular sigma
static int      Ni      = 30;   // trace length will go up to Ni*scal
static int      Ns      = 5;    // nr. states
static float    zDist   = 1.0;  // scaling along z

static int nrInputParams = 9; // to bound number of arguments in coding interface

// not necessary to tune
static V3DLONG channel = 1; // default channel
static int  saveMidres = 0; // for debugging
static unsigned char lowmargin = 2; // margin towards low end of the intensity range
static int GRAYLEVEL = 256; // plugin works with 8 bit images
static int Kskip = 3; // subsampling factor (useful for large stacks to reduce unnecessary computation)
static int maxBlobCount = 5; // max nr. blobs
static int minBlobSize  = 1; // in voxels

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    int     scal;       // scale
    float   bratio;     // background ratio - stopping the trace
    int     perc;       // fg. percentile
    float   znccTh;     // correlation threshold
    int     Ndir;       // number of directions
    float   angSig;     // angular deviation
    int     Ni;         // number of iterations
    int     Ns;         // number of states
    float   zDist;      // the distance between layers in pixels
    int     saveMidres; // save midresults
};

// some methods & classes used in reconstruction_func
template<typename T>
class BfsQueue  {
public:
    std::queue<T> kk;

    BfsQueue() {}

    void enqueue(T item) {this->kk.push(item);}

    T dequeue() {T output = kk.front(); kk.pop(); return output;}

    int size(){return kk.size();}

    bool hasItems(){return !kk.empty();}
};

int get_undiscovered(bool * disc, int nrnodes) {
    for (int i = 0; i < nrnodes; i++) {  // go indices sorted by the correlation value
        if (!disc[i]) {
            return i;
        }
    }
    return -1;
}

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList NeuronChaserX::menulist() const
{
	return QStringList() 
		<<tr("ncx_menu")
		<<tr("about");
}

QStringList NeuronChaserX::funclist() const
{
	return QStringList()
		<<tr("ncx_func")
		<<tr("help");
}

const QString title = QObject::tr("NeuronChaserX Plugin");

void NeuronChaserX::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("ncx_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;

        // pick the default params
        PARA.channel = channel;
        PARA.scal = scal;
        PARA.bratio = bratio;
        PARA.perc = perc;
        PARA.znccTh = znccTh;
        PARA.Ndir   = Ndir;
        PARA.angSig = angSig;
        PARA.Ni = Ni;
        PARA.Ns = Ns;
        PARA.zDist = zDist;
        PARA.saveMidres = saveMidres;

        // input through the menu, assign PARA fields

        // legend
        vector<string> items;
        items.push_back("Scale (5, 20] pix.");
        items.push_back("Background ratio (0, 1].");
        items.push_back("Percentile [50, 100].");
        items.push_back("Correlation threshold [0.5, 1.0).");
        items.push_back("nr. directions [5, 20].");
        items.push_back("Angular sigma [20,90] degs.");
        items.push_back("nr. iterations [2, 50].");
        items.push_back("nr. states [1, 20].");
        items.push_back("z layer dist [1, 4] pix.");

        // initialization
        vector<string> inits;
        inits.push_back(QString::number(PARA.scal).toStdString().c_str());
        inits.push_back(QString::number(PARA.bratio).toStdString().c_str());
        inits.push_back(QString::number(PARA.perc).toStdString().c_str());
        inits.push_back(QString::number(PARA.znccTh).toStdString().c_str());
        inits.push_back(QString::number(PARA.Ndir).toStdString().c_str());
        inits.push_back(QString::number(PARA.angSig).toStdString().c_str());
        inits.push_back(QString::number(PARA.Ni).toStdString().c_str());
        inits.push_back(QString::number(PARA.Ns).toStdString().c_str());
        inits.push_back(QString::number(PARA.zDist).toStdString().c_str());

        CommonDialog dialog(items, inits);
        dialog.setWindowTitle(title);
        if(dialog.exec() != QDialog::Accepted) return;

        dialog.get_num("Scale (5, 20] pix.", PARA.scal);
        dialog.get_num("Background ratio (0, 1].", PARA.bratio);
        dialog.get_num("Percentile [50, 100].", PARA.perc);
        dialog.get_num("Correlation threshold [0.5, 1.0).", PARA.znccTh);
        dialog.get_num("nr. directions [5, 20].", PARA.Ndir);
        dialog.get_num("Angular sigma [20,90] degs.", PARA.angSig);
        dialog.get_num("nr. iterations [2, 50].", PARA.Ni);
        dialog.get_num("nr. states [1, 20].", PARA.Ns);
        dialog.get_num("z layer dist [1, 4] pix.", PARA.zDist);

        // check input
        if(PARA.scal <= 5  || PARA.scal>20){v3d_msg(QObject::tr("Scale is out of range")); return;}
        if(PARA.bratio <= 0 || PARA.bratio > 1.0){v3d_msg(QObject::tr("Background ratio is out of range")); return;}
        if(PARA.perc < 50  || PARA.perc > 100){v3d_msg(QObject::tr("Percentile is out of range")); return;}
        if(PARA.znccTh < 0.5||PARA.znccTh >= 1.0){v3d_msg(QObject::tr("Correlation is out of range")); return;}
        if(PARA.Ndir<5|| PARA.Ndir>20){v3d_msg(QObject::tr("# directions is out of range")); return;}
        if(PARA.angSig < 20 || PARA.angSig>90){v3d_msg(QObject::tr("Angular sigma is out of range")); return;}
        if(PARA.Ni < 2 || PARA.Ni>50){v3d_msg(QObject::tr("# iterations is out of range")); return;}
        if(PARA.Ns <=  0 || PARA.Ns>20){v3d_msg(QObject::tr("# states is out of range")); return;}
        if(PARA.zDist < 1 || PARA.zDist>4){v3d_msg(QObject::tr("zdist is out of range")); return;}

        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("Plugin for neuron reconstruction.. "
			"Developed by Miroslav Radojevic, 2015-7-23"));
	}
}

void print_help(){
    printf("**** Usage of NeuronChaserX **** \n");
    printf("vaa3d -x NeuronChaserX -f ncx_func -i <inimg_file> -p <scal perc znccTh Ndir angSig Ni Ns zDist>\n");
    printf("inimg_file          The input image\n");
    printf("scal                Scale (5, 20] pix.\n");
    printf("bratio              Background ratio (0, 1].\n");
    printf("perc                Percentile [50, 100].\n");
    printf("znccTh              Correlation threshold [0.5, 1.0).\n");
    printf("Ndir                nr. directions [5, 20].\n");
    printf("angSig              Angular sigma [20,90] degs.\n");
    printf("Ni                  nr. iterations [2, 50].\n");
    printf("Ns                  nr. states [1, 20].\n");
    printf("zDist               z layer dist [1, 4] pix.\n");
    printf("outswc_file         Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");
}

bool NeuronChaserX::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("ncx_func"))
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

        // constrain number of input parameters
        if (paras.size()!=nrInputParams) {
            fprintf (stderr, "\n\nNeeds %d input parameters.\n\n", nrInputParams);
            print_help();
            return false;
        }

        int k=0;

        PARA.scal       = (paras.size() >= k+1)   ? atoi(paras[k])              : scal;      k++;
        PARA.bratio     = (paras.size() >= k+1)   ? QString(paras[k]).toFloat() : bratio;    k++;
        PARA.perc       = (paras.size() >= k+1)   ? atoi(paras[k])              : perc;      k++;
        PARA.znccTh     = (paras.size() >= k+1)   ? QString(paras[k]).toFloat() : znccTh;    k++;
        PARA.Ndir       = (paras.size() >= k+1)   ? atoi(paras[k])              : Ndir;      k++;
        PARA.angSig     = (paras.size() >= k+1)   ? QString(paras[k]).toFloat() : angSig;    k++;
        PARA.Ni         = (paras.size() >= k+1)   ? atoi(paras[k])              : Ni;        k++;
        PARA.Ns         = (paras.size() >= k+1)   ? atoi(paras[k])              : Ns;        k++;
        PARA.zDist      = (paras.size() >= k+1)   ? QString(paras[k]).toFloat() : zDist;     k++;
        PARA.channel    = channel; // these two are hardcoded, being not algorithm parameters
        PARA.saveMidres = saveMidres;

        // check user input
        if(PARA.scal <= 5  || PARA.scal>20){v3d_msg(QObject::tr("Scale is out of range")); return 0;}
        if(PARA.bratio <= 0 || PARA.bratio > 1.0){v3d_msg(QObject::tr("Background ratio is out of range"));return 0;}
        if(PARA.perc < 50  || PARA.perc > 100){v3d_msg(QObject::tr("Percentile is out of range")); return 0;}
        if(PARA.znccTh < 0.5 || PARA.znccTh >= 1.0){v3d_msg(QObject::tr("Correlation is out of range"));return 0;}
        if(PARA.Ndir<5|| PARA.Ndir>20){v3d_msg(QObject::tr("# directions is out of range")); return 0;}
        if(PARA.angSig<20|| PARA.angSig>90){v3d_msg(QObject::tr("Angular sigma is out of range")); return 0;}
        if(PARA.Ni < 2 || PARA.Ni>50){v3d_msg(QObject::tr("# iterations is out of range")); return 0;}
        if(PARA.Ns <=  0 || PARA.Ns>20){v3d_msg(QObject::tr("# states is out of range")); return 0;}
        if(PARA.zDist < 1 || PARA.zDist>4){v3d_msg(QObject::tr("zdist is out of range")); return 0;}

        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {
        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN
        print_help();
	}
	else return false;

	return true;
}

static bool compareNode(Node a, Node b) {return a.corr>b.corr;}

int count_vals(vector<bool> v, bool val) { // how many times val appears in v vector
    int cnt = 0;
    for (int var = 0; var < v.size(); ++var) {
        if (v[var] == val) {
            cnt++;
        }
    }
    return cnt;
}

class CompareIndicesByNodeVectorCorrValues {
    std::vector<Node>* _nodl;
public:
    CompareIndicesByNodeVectorCorrValues(std::vector<Node>* values) : _nodl(values) {}
public:
    bool operator() (const int& a, const int& b) const { return (*_nodl)[a].corr > (*_nodl)[b].corr; }
};

template<typename T>
T maximum(T x, T y, T z) {
    T max = x;
    if (y > max) max = y;
    if (z > max) max = z;
    return max;
}

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
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

        bool ok1;

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

        if(!ok1)
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

    //main neuron reconstruction code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE

    cout<<"----------  NeuronChaserX  ----------"   <<endl;
    cout<<"channel = "  <<PARA.channel              <<endl;
    cout<<"scal = "     <<PARA.scal                 <<endl;
    cout<<"bratio = "   <<PARA.bratio               <<endl;
    cout<<"perc = "     <<PARA.perc                 <<endl;
    cout<<"znccTh = "   <<PARA.znccTh               <<endl;
    cout<<"Ndir = "     <<PARA.Ndir                 <<endl;
    cout<<"angSig = "   <<PARA.angSig               <<endl;
    cout<<"Ni = "       <<PARA.Ni                   <<endl;
    cout<<"Ns = "       <<PARA.Ns                   <<endl;
    cout<<"zDist = "    <<PARA.zDist                <<endl;
    cout<<"saveMidres = "<<PARA.saveMidres          <<endl;
    cout<<"-------------------------------------"   <<endl;
    long size = N * M * P; // N : width, M : height, P : nr. layers

    // find min/max
    unsigned char globalmin = data1d[0];
    unsigned char globalmax = data1d[0];
    for (long i = 1; i < size; ++i) {
        if (data1d[i]<globalmin)
            globalmin = data1d[i];
        if (data1d[i]>globalmax)
            globalmax = data1d[i];
    }

    unsigned char imgTh = globalmin + lowmargin;

    ///////////////////////////////////////
    // offsets in different planes
    ///////////////////////////////////////
    int limxy = ceil( PARA.scal/2.0);// PARA.scal corresponds to the diameter
    int limz  = ceil((PARA.scal/2.0)/zDist);
    //// offsets XY
    vector<int> dx_XY, dy_XY;
    for (int dx = -limxy; dx <= limxy; ++dx) {
            for (int dy = -limxy; dy <= limxy; ++dy) {
                if (pow(dx,2)/pow(limxy,2)+pow(dy,2)/pow(limxy,2)<=1) {
                    dx_XY.push_back(dx);
                    dy_XY.push_back(dy);
                }
            }
    }

    //// offsets XZ
    vector<int> dx_XZ, dz_XZ;
    for (int dx = -limxy; dx <= limxy; ++dx) {
            for (int dz = -limz; dz <= limz; ++dz) {
                if (pow(dx,2)/pow(limxy,2)+pow(dz,2)/pow(limz,2)<=1) {
                    dx_XZ.push_back(dx);
                    dz_XZ.push_back(dz);
                }
            }
    }
    //// offsets YZ
    vector<int> dy_YZ, dz_YZ;
    for (int dy = -limxy; dy <= limxy; ++dy) {
            for (int dz = -limz; dz <= limz; ++dz) {
                if (pow(dy,2)/pow(limxy,2)+pow(dz,2)/pow(limz,2)<=1) {
                    dy_YZ.push_back(dy);
                    dz_YZ.push_back(dz);
                }
            }
    }

    // erosion, variance
    unsigned char * erd1d = new unsigned char[size]; // loc. min.
    unsigned char * scr1 = new unsigned char[size]; // score
    vector<unsigned char> scr;
    scr.reserve(size/(Kskip*Kskip));

    cout << "processing... " << flush;
    for (long i = 0; i < size; ++i) {

        // default values
        erd1d[i] = imgTh;
        scr1[i]  = 0;

        int x  = i % N;
        int z  = i / (N*M);
        int y  = i/N-z*M;

        if (x%Kskip==z%Kskip && y%Kskip==z%Kskip) { // every Kskip calculate

            if (data1d[i]<imgTh) { // if below lowest boundary defined with min. value and margin
                erd1d[i] = imgTh;
                scr.push_back(0);
                scr1[i] = 0;
            }
            else {
                unsigned char xymin=UCHAR_MAX, xymax=0;
                unsigned char scrxyz;

                for (int j = 0; j < dx_XY.size(); ++j) {

                    int xi = x + dx_XY[j];
                    int yi = y + dy_XY[j];

                    if (xi>=0 && xi<N && yi>=0 && yi<M) {

                        int ii = z*N*M+yi*N+xi;

                        if (data1d[ii]<xymin)
                            xymin = data1d[ii];

                        if (data1d[ii]>xymax)
                            xymax = data1d[ii];
                    }

                }

                scrxyz = xymax - xymin;

                if (P>1) {

                    // xz plane
                    unsigned char xzmin=UCHAR_MAX, xzmax=0;

                    for (int j = 0; j < dx_XZ.size(); ++j) {

                        int xi = x + dx_XZ[j];
                        int zi = z + dz_XZ[j];

                        if (xi>=0 && xi<N && zi>=0 && zi<P) {

                            int ii = zi*N*M+y*N+xi;

                            if (data1d[ii]<xzmin)
                                xzmin = data1d[ii];

                            if (data1d[ii]>xzmax)
                                xzmax = data1d[ii];
                        }

                    }

                    unsigned char sxz = xzmax - xzmin;
                    scrxyz = (sxz>scrxyz)?sxz:scrxyz;

                    // yz plane
                    unsigned char yzmin=UCHAR_MAX, yzmax=0;

                    for (int j = 0; j < dy_YZ.size(); ++j) {

                        int yi = y + dy_YZ[j];
                        int zi = z + dz_YZ[j];

                        if (yi>=0 && yi<M && zi>=0 && zi<P) {

                            int ii = zi*N*M+yi*N+x;

                            if (data1d[ii]<yzmin)
                                yzmin = data1d[ii];

                            if (data1d[ii]>yzmax)
                                yzmax = data1d[ii];

                        }

                    }

                    unsigned char syz = yzmax - yzmin;
                    scrxyz = (syz>scrxyz)?syz:scrxyz;

                } // P>1

                erd1d[i] = (xymin<imgTh)?imgTh:xymin;
                scr.push_back(scrxyz);
                scr1[i] = scrxyz;

            } // >= imgTh
        }
    }

    cout << "filling up... " << flush;
    for (long i = 0; i < size; ++i) {

        int x = i % N;
        int z = i / (N*M);
        int y = i/N-z*M;

            if (!(x%Kskip==z%Kskip && y%Kskip==z%Kskip)) {
                int x1 = (x/Kskip)*Kskip + z%Kskip;
                int y1 = (y/Kskip)*Kskip + z%Kskip;

                // those that are out of the boundaries are not filled up but kept the default
                if (x1>=0 && x1<N && y1>=0 && y1<M) {
                    int ii = z*N*M+y1*N+x1;
                    erd1d[i] = erd1d[ii];
                    scr1[i] = scr1[ii];
                }
            }
    }
    cout << "done." << endl;

    // threshold
    unsigned char imth = intermodes_th(erd1d, size);
    for (int i = 0; i < size; ++i)
        erd1d[i] = (erd1d[i]>=imth)?255:0;

    // dilate thresholded
    cout << "blobseg... " << flush;
    unsigned char * somaseg = new unsigned char[size]; // expands from erd1d 1s
    for (long i = 0; i < size; ++i)
        somaseg[i] = 0;

    for (long i = 0; i < size; ++i) {

        int x = i % N;
        int z = i / (N*M);
        int y = i/N-z*M;

        if (erd1d[i]==255) {
            // dilatation would happen in XY plane
            for (int j = 0; j < dx_XY.size(); ++j) {

                int xi = x + dx_XY[j];
                int yi = y + dy_XY[j];

                if (xi>=0 && xi<N && yi>=0 && yi<M) { // && zi>=0 && zi<P

                    int ii = z*N*M+yi*N+xi;

                    if (somaseg[ii]==0)
                        somaseg[ii] = 255;

                }
            }
        }
    }

    cout << "done." << endl;

    if (PARA.saveMidres) {
        QString of = PARA.inimg_file + "_scr1.tif";
        simple_saveimage_wrapper(callback, of.toStdString().c_str(), scr1, in_sz, V3D_UINT8);
        of = PARA.inimg_file + "_erd1dTh.tif";
        simple_saveimage_wrapper(callback, of.toStdString().c_str(), erd1d, in_sz, V3D_UINT8);
        of = PARA.inimg_file + "_somaseg.tif";
        simple_saveimage_wrapper(callback, of.toStdString().c_str(), somaseg, in_sz, V3D_UINT8);
    }

    delete erd1d; erd1d = 0;

    // tagmap - bookmark stack voxels:
    // 0    : background
    // -1   : foreground
    // -2   : mask this foreground location from calculating
    // 1    : node<1>   (soma)
    // 2    : node<2>   (soma)
    // ...
    // A    : node<A>   (gpnt)
    // A+1  : node<A+1> (gpnt)
    // ...
    // B    : node<B>   (trc)
    // B+1  : node<B+1> (trc)
    // ...
    // tag will correspond to the node index in the list
    // node belongs to soma, gpnt or trc

    int * tagmap = new int[size];  // size too large for static alloc

    for (long i = 0; i < size; ++i) {
        tagmap[i] = 0;
    }

    vector<float> xc, yc, zc, rc;
    conn3d(somaseg, N, M, P, tagmap, maxBlobCount, true, 0, minBlobSize, xc, yc, zc, rc);
    // connected c. -> soma labels and x,y,z,r

    // as a result tagmap will have
    // 0 (background)
    // 1, 2, 3, ... Nblobs
    // ...
    // -1 will be added to tag foreground in tagmap

//    cout << xc.size() << " blob(s)" << endl;
//    for (int i = 0; i < xc.size(); ++i) {
//        cout << xc[i] << " " << yc[i] << " " << zc[i] << " " << rc[i] << endl;
//    }

    delete somaseg; somaseg = 0;

    // percentile on score
    unsigned char th = quantile(scr, PARA.perc, 100);
//    cout << "\t" << PARA.perc << "th = " << (int)th << endl;

    // assign those that are in the foreground with -1 (necessary to disctinct from background)
    for (long i = 0; i < size; ++i) {
        if (scr1[i]>th && tagmap[i]==0)
            tagmap[i] = -1;
    }

    delete scr1; scr1 = 0;

    if (PARA.saveMidres) {
        float * tagmapcopy = new float[size];
        for (long i = 0; i < size; ++i)
            tagmapcopy[i] = tagmap[i];

        QString of = PARA.inimg_file + "_tagmapInit.raw";
        char * of1 = new char[of.length()+1];
        strcpy(of1, of.toLatin1().constData());
        Image4DSimple outimg1;
        outimg1.setData((unsigned char *)tagmapcopy, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);
        callback.saveImage(&outimg1, of1);
        tagmapcopy = 0;
        delete of1; of1 = 0;
    }

    // initialize tracer
    BTracer btrcr(PARA.Ni, PARA.Ns, PARA.scal, P==1, PARA.zDist);

    if (PARA.saveMidres) {
        btrcr.save_templates(callback, PARA.inimg_file); // templates used for zncc()
    }

    // count foreground locs for array allocation
    long cnt_fg = 0;
    for (long i = 0; i < size; ++i)
        if (tagmap[i]==-1)
            cnt_fg++;

    cout << setprecision(4) << cnt_fg/1000.0 << "k fg. locs, " << setprecision(2) << ((float)cnt_fg/size) << "%, at scale=" << PARA.scal << endl;

    int ** i2xyz = new int*[cnt_fg];
    for(long i = 0; i < cnt_fg; ++i)
        i2xyz[i] = new int[3];

    long * xyz2i = new long[size];
    for(long i = 0; i < size; ++i) xyz2i[i] = -1;

    cnt_fg = 0;
    for (long i = 0; i < size; ++i) {
        if (tagmap[i]==-1) {

            int xf  = i % N;
            int zf  = i / (N*M);
            int yf  = i/N-zf*M;

            i2xyz[cnt_fg][0] = xf;
            i2xyz[cnt_fg][1] = yf;
            i2xyz[cnt_fg][2] = zf;

            xyz2i[zf*N*M+yf*N+xf] = cnt_fg;

            cnt_fg++;
        }
    }

    cout << "\ngpnt calculation..." << endl;
    clock_t time1 = clock();
    // book-keep all zncc calculations (zncc, gcsstd, vx, vy, vz)
    // in case they are accessed several times from the same location while searching
    // helps avoiding double calculation
    float ** i2calcs = new float*[cnt_fg];

    for (long i = 0; i < cnt_fg; ++i) {
        i2calcs[i] = new float[5]; // zncc, gcsstd, vx, vy, vz
        for (int j = 0; j < 5; ++j) {
            i2calcs[i][j] = -FLT_MAX; // default
        }
    }

    vector<Node> nodelist = vector<Node>(); // list of recon nodes
    vector<Node> gpntlist = vector<Node>(); // gpnt nodes, won't be immediately added to recon

    // nodelist will have gcsstd as r (fixed later when exporting)
    Node dummynode(0,0,0,0,Node::UNDEFINED);
    nodelist.push_back(dummynode); // nodes start from index 1 (index 0 would mix with background tag)

    // add nodes that correspond to blobs (they have been tagged till now)
//    cout << xc.size() << "blobs:" << endl;
    for (int i = 0; i < xc.size(); ++i) {
        // Nodes have gcsstd as radius (scaled up with later on)
        // but blob extraction gets the real radius so reduce it so that the scaling up later does not mess things up
//        cout << nodelist.size() << " : " << setw(5) << xc[i] << setw(5) << yc[i] << setw(5) << zc[i] << setw(5) << rc[i] << endl;
        Node blobnode(xc[i], yc[i], zc[i], rc[i]/btrcr.gcsstd2rad, Node::SOMA); // undirected
        nodelist.push_back(blobnode);
    }

    // set of directions used for gpnt extraction
    float vx_corr[PARA.Ndir];
    float vy_corr[PARA.Ndir];
    float vz_corr[PARA.Ndir];

    double h_k, theta_k, phi_k, phi_k_1 = 0;

    for (int k = 0; k < PARA.Ndir; k++) { // generate Ndir directions

        // 3d
        if (P==1) {
            /////////////
            // 2d
            /////////////

//                float ang1 = ((3.14/2.0)-ang) + di * ((2*ang) / (nr_dirs-1));
            float ang1 = 0.0 + k * (3.14/(float)PARA.Ndir);
            vx_corr[k] =  cos(ang1);
            vy_corr[k] =  sin(ang1);
            vz_corr[k] =  0;

        }
        else {
            /////////////
            // 3d
            /////////////

            h_k = 1 - 1 * ((double)k/(PARA.Ndir-1)); // 1 : 0 defines angular range
            theta_k = acos(h_k);

            if(k==0 || k==(PARA.Ndir-1)) {
                phi_k   = 0;
                phi_k_1 = 0;
            }
            else {
                phi_k = phi_k_1 + 3.6 / ( sqrt(PARA.Ndir) * sqrt(1-h_k*h_k));
                phi_k_1 = phi_k;
            }

            vx_corr[k] = (float) (sin(theta_k) * cos(phi_k));
            vy_corr[k] = (float) (sin(theta_k) * sin(phi_k));
            vz_corr[k] = (float)  cos(theta_k);

        }

// cout << vx_corr[k] << "; " << vy_corr[k] << "; " << vz_corr[k] << "; " << endl; // << sqrt(vx_corr[k]*vx_corr[k]+vy_corr[k]*vy_corr[k]+vz_corr[k]*vz_corr[k])

    }

    float curr_zncc;
    int curr_gcsstd_idx;

    int rx = PARA.scal; // search for local max, radius of the search
    int ry = PARA.scal;
    int rz = round(PARA.scal/zDist);

    int rxout, ryout, rzout;  // outer sphere radius
    int rxin,  ryin,  rzin;   // inner

    // skip some foreground locaitons in case there is too many
    // 100.000 is a feasible number to work out, more than 200.000 starts with the reduction
    int every = cnt_fg/100000;
    every = (every<1)?1:every; // cannot be lower than 1
    cout << "every = " << every << endl;

    for (long i = 0; i < cnt_fg; ++i) { // go through fg. locs (-1)

        if (i%every!=0)
            continue;

        if (i%(cnt_fg/10)==0)
            cout << (i/(cnt_fg/100)) << "%\t" << std::flush;

        int x  = i2xyz[i][0]; // corresponding locations
        int y  = i2xyz[i][1];
        int z  = i2xyz[i][2];

        // it overlaps or is in the neighbourhood of the gpnt region (-2), to avoid overlap
        // zncc was calculated before, when searching for a local zncc max in the neighbourhood of the candidate point
        if (tagmap[z*N*M+y*N+x]==-1) { // can be a candidate
            if (i2calcs[i][0] == -FLT_MAX) { // not calculated yet, has default value, calculate it
                for (int dix = 0; dix < PARA.Ndir; ++dix) {

                    curr_zncc = btrcr.znccX(x, y, z, vx_corr[dix], vy_corr[dix], vz_corr[dix],
                                    data1d, N, M, P,
                                    curr_gcsstd_idx);

                    if (curr_zncc>i2calcs[i][0]) {
                        i2calcs[i][0] = curr_zncc;
                        i2calcs[i][1] = curr_gcsstd_idx;
                        i2calcs[i][2] = vx_corr[dix];
                        i2calcs[i][3] = vy_corr[dix];
                        i2calcs[i][4] = vz_corr[dix];
                    }
                }
            }

            if (i2calcs[i][0]>PARA.znccTh) { // check local neighbourhood for locally optimal

                long    lmax_i    = i; // fg. index of local max.
                float   lmax_zncc = i2calcs[i][0];
                int     offidx    = round(i2calcs[i][1]);

                for (int k = 0; k < btrcr.offx[offidx].size(); ++k) { // use offsets for search

                    int xn = x + btrcr.offx[offidx][k];//dx_XYZ[k];
                    int yn = y + btrcr.offy[offidx][k];//dy_XYZ[k];
                    int zn = z + btrcr.offz[offidx][k];//dz_XYZ[k];

                    if (!(xn==x && yn==y && zn==z)) { // skip central one
                        if (xn>=0 && xn<N && yn>=0 && yn<M && zn>=0 && zn<P) {
                            if (tagmap[zn*N*M+yn*N+xn]==-1) {
                                // not in background    ( 0)
                                // not in other region  (>0)
                                // would not overlap    (-2)
                                long in = xyz2i[zn*N*M+yn*N+xn];

                                if (i2calcs[in][0] == -FLT_MAX) {
                                    for (int dix = 0; dix < PARA.Ndir; ++dix) {

                                        curr_zncc = btrcr.znccX(
                                                    xn, yn, zn,
                                                    vx_corr[dix], vy_corr[dix], vz_corr[dix],
                                                    data1d, N, M, P, curr_gcsstd_idx);

                                        // return index of the gcsstd that would imply the offsets

                                        if (curr_zncc>i2calcs[in][0]) {
                                            i2calcs[in][0] = curr_zncc;
                                            i2calcs[in][1] = curr_gcsstd_idx;
                                            i2calcs[in][2] = vx_corr[dix];
                                            i2calcs[in][3] = vy_corr[dix];
                                            i2calcs[in][4] = vz_corr[dix];
                                        }
                                    }
                                }

                                if (i2calcs[in][0]>lmax_zncc) { // compare it with the current
                                    lmax_i    = in; // upate local max foreground index
                                    lmax_zncc = i2calcs[in][0];
                                }

                                tagmap[zn*N*M+yn*N+xn] = -2;
                            }
                        }
                    }
                }

                int gpntx = i2xyz[lmax_i][0];
                int gpnty = i2xyz[lmax_i][1];
                int gpntz = i2xyz[lmax_i][2];
                int gpntr_idx = round(i2calcs[lmax_i][1]);
                float gpntvx = i2calcs[lmax_i][2];
                float gpntvy = i2calcs[lmax_i][3];
                float gpntvz = i2calcs[lmax_i][4];
                float gpntcorr = i2calcs[lmax_i][0];

                Node gpntnode(gpntx, gpnty, gpntz,
                                  gpntvx, gpntvy, gpntvz,
                                  gpntcorr, btrcr.gcsstd[gpntr_idx], Node::UNDEFINED);
                gpntlist.push_back(gpntnode);

                // offsets for differnt scales are stored in BTracer upon initialization
                // fill the tags associated with the scale
                // tag offsets are ready in the btracer class, can be indexed

                for (int k = 0; k < btrcr.offx[gpntr_idx].size(); ++k) {

                    int xnn = gpntx + btrcr.offx[gpntr_idx][k];
                    int ynn = gpnty + btrcr.offy[gpntr_idx][k];
                    int znn = gpntz + btrcr.offz[gpntr_idx][k];

                    if (xnn>=0 && xnn<N && ynn>=0 && ynn<M && znn>=0 && znn<P) {

                        int ct = tagmap[znn*N*M+ynn*N+xnn];

                        if (ct==-1 || ct==0) {
                            tagmap[znn*N*M+ynn*N+xnn] = -2; // avoid processing nbrs
                        }

                    }

                }

            } // if (i2calcs[i][0]>PARA.znccTh)
        } // if (tagmap[z*N*M+y*N+x]==-1)
    } // for (long i = 0; i < cnt_fg; ++i)

    clock_t time2 = clock();
    cout << "okey. " << setprecision(4) << (double(time2-time1)/CLOCKS_PER_SEC) << "sec "<< setw(4) << gpntlist.size() << " gpnts" << endl;

    // expand foreground to accomodate traces
    cout << "\nexpand tagmap..." << flush;
    int * tagmap_temp = new int[size]; // tagmap copy
    for (long i = 0; i < size; ++i)
        tagmap_temp[i] = tagmap[i];

    // go through all of tagmap copy's non-zero elements and dilate with -1s (0 becomes -1)
    for (long i = 0; i < size; ++i) {
        int x  = i % N;
        int z  = i / (N*M);
        int y  = i/N-z*M;
        if (tagmap_temp[z*N*M+y*N+x]==0)
            continue; // skip those that are zero

        for (int k = 0; k < btrcr.offx[0].size(); ++k) {

            int xi = x + btrcr.offx[0][k];
            int yi = y + btrcr.offy[0][k];
            int zi = z + btrcr.offz[0][k];

            if (xi>=0 && xi<N && yi>=0 && yi<M && zi>=0 && zi<P) {

                int ii = zi*N*M + yi*N + xi;
                if (tagmap[ii]==0)
                    tagmap[ii] = -1;

            }
        }
    }

    delete tagmap_temp; tagmap_temp = 0;
    cout << "ok. " << endl;

    if (PARA.saveMidres) {

        float * tagmapcopy = new float[size];
        for (long i = 0; i < size; ++i)
            tagmapcopy[i] = tagmap[i];

        QString of = PARA.inimg_file + "_tagmapExpd.raw";
        char * of1 = new char[of.length()+1];
        strcpy(of1, of.toLatin1().constData());
        Image4DSimple outimg1;
        outimg1.setData((unsigned char *)tagmapcopy, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);
        callback.saveImage(&outimg1, of1);
        tagmapcopy = 0;
        delete of1; of1 = 0;

        NeuronTree gpntswc;
        for (int ni = 0; ni < gpntlist.size(); ++ni) {
                NeuronSWC nn;
                nn.n = nn.nodeinseg_id = ni+1;
                nn.type = Node::UNDEFINED;
                nn.x = gpntlist[ni].x;
                nn.y = gpntlist[ni].y;
                nn.z = gpntlist[ni].z;
                nn.r = btrcr.gcsstd2rad * gpntlist[ni].r;
                nn.parent = -1;
                gpntswc.listNeuron.append(nn);
        }

        QString gpntswc_name = PARA.inimg_file + "_gpntX.swc";
        gpntswc.name = "GuidepointLocs";
        writeSWC_file(gpntswc_name.toStdString().c_str(), gpntswc);

    }

    for (int i = 0; i < cnt_fg; ++i) {delete i2xyz[i]; delete i2calcs[i];}
    delete i2xyz; i2xyz = 0;
    delete i2calcs; i2calcs = 0;
    delete xyz2i; xyz2i = 0;

    // trace map, initialized with dilated tagmap (-2, -1, 0, 1, 2, nr. blobs)
    int * trcmap = new int[size];  // each added node will belong to a trace and a node
    for (long i = 0; i < size; ++i) {
        trcmap[i] = tagmap[i]; // tracemap is initialized with expanded tagmap with foreground, background, blobs but no traces
    }

    // arrange gpntlist by correlation score (start chasing from the one with highest correlation)
    // std::sort(list.begin(), list.end(), compareElement);
    // sort indices and trace from the top of the sorted original indices
    vector<int> indices(gpntlist.size()); // nr nodes before the tracing starts
    for (int i = 0; i < indices.size(); ++i) indices.at(i) = i; // std::iota does the same
    sort(indices.begin(), indices.end(), CompareIndicesByNodeVectorCorrValues(&gpntlist));

//    cout << indices[0] << " -> " << gpntlist[indices[0]].corr << endl;
//    cout << indices.back() << " -> " << gpntlist[indices.back()].corr << endl;

    // tracing bounded with tagmaps, initialized from the highest corr guidepoint
    // final output is the list of linked nodes
    cout << "chasingX... " << endl;

    time1 = clock();

    for (int ni = 0; ni < indices.size(); ++ni) {

        if (ni%(indices.size()/10)==0) cout << "\n--- " << ni/(indices.size()/10)*10 << "% ---" << endl;

        int     tt_i    = indices.at(ni); // get the nodelist index
        float   tt_x    = gpntlist.at(tt_i).x;
        float   tt_y    = gpntlist.at(tt_i).y;
        float   tt_z    = gpntlist.at(tt_i).z;
        float   tt_vx   = gpntlist.at(tt_i).vx;
        float   tt_vy   = gpntlist.at(tt_i).vy;
        float   tt_vz   = gpntlist.at(tt_i).vz;
        float   tt_r    = gpntlist.at(tt_i).r;

        bool tovlp = btrcr.traceoverlap(tt_x, tt_y, tt_z, tt_r, N, M, P, trcmap, tagmap, nodelist);

        if (tovlp) { // skip if it overlaps with earlier trace
//            cout << "o" << flush;
            continue;
        }
//        else {
//            cout << "\n" << ni << "/" << indices.size() << " [" << tt_i << "] " << endl;
//        }

        btrcr.traceX(
                    true,
                    tt_x, tt_y, tt_z,
                    tt_vx, tt_vy, tt_vz,
                    tt_r,
                    data1d,
                    N, M, P,
                    PARA.angSig,
                    tagmap,
                    trcmap,
                    nodelist,
                    PARA.bratio);

        btrcr.traceX(
                    false,
                    tt_x, tt_y, tt_z,
                    -tt_vx, -tt_vy, -tt_vz,
                    tt_r,
                    data1d,
                    N, M, P,
                    PARA.angSig,
                    tagmap,
                    trcmap,
                    nodelist,
                    PARA.bratio);

    }

    time2 = clock();
    cout << "\nDONE.\t" << nodelist.size() << " rec. nodes " << setprecision(5) << (double(time2-time1)/CLOCKS_PER_SEC) << " sec." << endl;

    // remove duplicate neighbourhood node links from the neighbour list
    for (int ni = 1; ni < nodelist.size(); ++ni) {
        sort(nodelist.at(ni).nbr.begin(), nodelist.at(ni).nbr.end());
        nodelist.at(ni).nbr.erase(unique(nodelist.at(ni).nbr.begin(), nodelist.at(ni).nbr.end()), nodelist.at(ni).nbr.end());
    }

    if (PARA.saveMidres) {

        NeuronTree recnodesswc;

        for (int ii = 1; ii < nodelist.size(); ++ii) {
            NeuronSWC n0;
            n0.nodeinseg_id = ii;
            n0.n = ii;
            n0.type = 6;
            n0.x = nodelist.at(ii).x;
            n0.y = nodelist.at(ii).y;
            n0.z = nodelist.at(ii).z;
            n0.r = btrcr.gcsstd2rad * nodelist.at(ii).r;
            n0.parent = -1;
            recnodesswc.listNeuron.append(n0);
        }

        QString recnodesswc_name = PARA.inimg_file + "_recnodesX.swc";
        recnodesswc.name = "ReconstructionNodes";
        writeSWC_file(recnodesswc_name.toStdString().c_str(), recnodesswc);

    }

    if (PARA.saveMidres) {

        float * tagmapcopy = new float[size];
        for (long i = 0; i < size; ++i)
            tagmapcopy[i] = tagmap[i];

        QString of = PARA.inimg_file + "_tagmapFinal.raw";
        char * of1 = new char[of.length()+1];
        strcpy(of1, of.toLatin1().constData());
        Image4DSimple outimg1;
        outimg1.setData((unsigned char *)tagmapcopy, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);
        callback.saveImage(&outimg1, of1);
        tagmapcopy = 0;
        delete of1; of1 = 0;

        //-------------------------------------------

        tagmapcopy = new float[size];
        for (long i = 0; i < size; ++i)
            tagmapcopy[i] = trcmap[i];

        of = PARA.inimg_file + "_trcmapFinal.raw";
        of1 = new char[of.length()+1];
        strcpy(of1, of.toLatin1().constData());
        outimg1.setData((unsigned char *)tagmapcopy, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);
        callback.saveImage(&outimg1, of1);
        tagmapcopy = 0;
        delete of1; of1 = 0;

    }

    delete tagmap; tagmap = 0;
    delete trcmap; trcmap = 0;

    cout << "extract tree..." << endl;

    /** traverse through the connected nodes to get the tree out
     *  breadth-first search (BFS) to traverse the tree from extracted node list
     *  http://en.wikipedia.org/wiki/Breadth-first_search
     *
        1  procedure BFS(G,v) is
        2      let Q be a queue
        3      Q.enqueue(v)
        4      label v as discovered
        5      while Q is not empty
                 6         v ← Q.dequeue()
                 7         for all edges from v to w in G.adjacentEdges(v) do
                 8             if w is not labeled as discovered
                 9                 Q.enqueue(w)
                 10                label w as discovered
     *
     */

    // will essentially convert linked nodelist into NeuronTree with the list of neuron nodes
    // each node (NeuronSWC) having 1 parent
    // BFS needs Queue data structure that is implemented in class BfsQueue
    // queue accumulates the linkages between nodes
    // linkage is described as vector<int> with 2 node indexes [curr_node_idx, next_node_idx]
    // knowing just the node_idx is not enough, need to know the index of the parent node

    // nodelist data structure:
    // nodelist[0] dummy node
    // nodelist[1] Node instance (tag 1) with connections to other neighbours stored in nbr
    // nodelist[2] Node instance (tag 2)
    // nodelist[3] Node instance (tag 3)
    // nodelist[4] ....
    // ...
    // nodelist[N].nbr will be a list of nodelist indexes of adjacent nodes

    bool        D[nodelist.size()]; // for book-keeping the discovered nodes
//    int         L[nodelist.size()]; // len store the distance to backtrack towards nearest CP
    int         I2Swc[nodelist.size()];// map node index with corresponding swc index

    for (int i = 0; i < nodelist.size(); ++i) {
        D[i]        = (i==0)?true:false;
        I2Swc[i]    = -1; // undefined
//        L[i]        = -1; // undefined
    }

    NeuronTree nt; // selected output tree
    int maxtreenodes = -1;
    int seedidx;
    bool fisrttime = true; // first time seed from blobs

    while ((seedidx = get_undiscovered(D, nodelist.size()))!=-1) {

        BfsQueue< vector<int> > boob; // queue for the BFS
        vector<int> seed; // seed indexeses of nodes that will initialize the tree genesis
        if (fisrttime && xc.size()>=1) { // if it's first time and soma regions exist
            // take soma regions as seeds (seedidx won't be used)
            for (int si = 1; si <= xc.size(); ++si)
                    seed.push_back(si);
        }
        else
            seed.push_back(seedidx); // yet undiscovered with highest corr.

        fisrttime = false;

        NeuronTree ntcurr;
        ntcurr.name =
                "NeuronChaserX\n#author: Miroslav (miroslav.radojevic@gmail.com)\n#params:\n#channel="+QString("%1").arg(PARA.channel)+
                    "\n#scal="+QString("%1").arg(PARA.scal)+
                    "\n#perc="+QString("%1").arg(PARA.perc)+
                    "\n#znccTh="+QString("%1").arg(PARA.znccTh)+
                    "\n#Ndir="+QString("%1").arg(PARA.Ndir)+
                    "\n#angSig="+QString("%1").arg(PARA.angSig)+
                    "\n#Ni="+QString("%1").arg(PARA.Ni)+
                    "\n#Ns="+QString("%1").arg(PARA.Ns)+
                    "\n#zDist="+QString("%1").arg(PARA.zDist)+
                    "\n#---------------------\n#Kskip="+QString("%1").arg(Kskip)+
                    "\n#bgratio="+QString("%1").arg(PARA.bratio)+
                    "\n#maxBlobCount="+QString("%1").arg(maxBlobCount)+
                    "\n#minSomaSize="+QString("%1").arg(minBlobSize);


        for (int i = 0; i < seed.size(); i++) {
// enqueue(), add to FIFO structure, http://en.wikipedia.org/wiki/Queue_%28abstract_data_type%29

            vector<int> lnk(2);
            lnk[0] = -1;
            lnk[1] = seed[i];
            boob.enqueue(lnk);

            D[seed[i]] = true; // mark as discovered
//            L[seed[i]] = 0; // information on length toowards CP
                // I2Swc will be filled at the dequeue moment
        }


        while (boob.hasItems()) { // while queue is not empty

                // dequeue(), take from FIFO structure, http://en.wikipedia.org/wiki/Queue_%28abstract_data_type%29
                vector<int> tt = boob.dequeue();
                int prev = tt[0];
                int curr = tt[1];

                I2Swc[curr] = ntcurr.listNeuron.size()+1; // index that will be after adding

                // add to swc
                NeuronSWC n0;
                n0.n = n0.nodeinseg_id = I2Swc[curr];
                n0.type = Node::AXON;
                n0.x = nodelist[curr].x;
                n0.y = nodelist[curr].y;
                n0.z = nodelist[curr].z;
                n0.r = btrcr.gcsstd2rad * nodelist[curr].r;
                n0.parent = (prev==-1)? -1 : I2Swc[prev];
                ntcurr.listNeuron.append(n0);

                vector<int> nextlist;
                nextlist.empty();

                for (int k = 0; k < nodelist[curr].nbr.size(); ++k) {
                    int next = nodelist[curr].nbr[k];
                    if (!D[next]) {

                        vector<int> lnk(2);
                        lnk[0] = curr;
                        lnk[1] = next;
                        boob.enqueue(lnk);

                        D[next] = true;
//                        L[next] = L[curr] + 1;

                        nextlist.push_back(next);

                    }
                }

                if (nextlist.size()==0 && nodelist[curr].nbr.size()>1) {
                    // termination in tree search
    //                ntcurr.listNeuron.last().type = Node::END;
    //                for (int k = 0; k < L[curr]; ++k)
                        ntcurr.listNeuron.removeLast();
                }
//                else if (nextlist.size()>1) {
    //                for (int k = 0; k < nextlist.size(); ++k)
    //                    L[nextlist[k]] = 1; // reset if they are expanding from CP
//                }
        }

        cout << "DONE > " <<  ntcurr.listNeuron.size() << " nodes found." << endl;

        if (ntcurr.listNeuron.size()>maxtreenodes) {
            maxtreenodes = ntcurr.listNeuron.size();
            nt = ntcurr; // keep the one with highest number of nodes in nt
        }

            if (PARA.saveMidres) { // can be enabled to save the rest
                QString ntcurrswc_name = PARA.inimg_file+"_NeuronChaserX_"+QString("%1").arg(ntcurr.listNeuron.size(), 4, 10, QChar('0'))+".swc";
                    writeSWC_file(ntcurrswc_name.toStdString().c_str(), ntcurr);
            }

        }

        cout << "\ntree with " << nt.listNeuron.size() << " nodes selected.\n" << endl;

        QString swc_name = PARA.inimg_file + "_NeuronChaserX.swc";
        writeSWC_file(swc_name.toStdString().c_str(), nt);

//    //Output
//    NeuronTree nt;
//	QString swc_name = PARA.inimg_file + "_NeuronChaserX.swc";
//	nt.name = "NeuronChaserX";
//    writeSWC_file(swc_name.toStdString().c_str(),nt);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}
