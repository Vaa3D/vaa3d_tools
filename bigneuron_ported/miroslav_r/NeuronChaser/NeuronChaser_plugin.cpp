/*
Copyright (C) Erasmus MC. Permission to use this software and corresponding documentation for educational, research, and not-for-profit purposes, without a fee and without a signed licensing agreement, is granted, subject to the following terms and conditions.

IT IS NOT ALLOWED TO REDISTRIBUTE, SELL, OR LEASE THIS SOFTWARE, OR DERIVATIVE WORKS THEREOF, WITHOUT PERMISSION IN WRITING FROM THE COPYRIGHT HOLDER. THE COPYRIGHT HOLDER IS FREE TO MAKE VERSIONS OF THE SOFTWARE AVAILABLE FOR A FEE OR COMMERCIALLY ONLY.

IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OF ANY KIND WHATSOEVER, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF ADVISED OF THE POSSIBILITY THEREOF.

THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE EXPRESS OR IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE AND CORRESPONDING DOCUMENTATION IS PROVIDED "AS IS". THE COPYRIGHT HOLDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 * 2015-5-31 : by Miroslav Radojevic
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include "nf_dialog.h"
#include "toolbox.h"
#include "model.h"
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

#include "NeuronChaser_plugin.h"
Q_EXPORT_PLUGIN2(NeuronChaser, NeuronChaser);

using namespace std;

// input parameter default values (offered in user menu)
static long     channel = 1;
static int      scal    = 12;
static int      perc    = 90;
static float    znccTh  = 0.60;
static int      Ndir    = 13;
static float    angSig  = 60;
//static float    gcsSig  = 3;
static int      Ni      = 30; // trace length will go up to Ni*scal
static int      Ns      = 5; // nr. states
static float    zDist   = 1.0;
static int      saveMidres = 0;

static int nrInputParams = 10; // to constrain number of arguments

// not necessary to tune
static unsigned char lowmargin = 2; // margin towards low end of the intensity range
//static int prune_len = 4;   // those that reach endpoint after 1 step are removed from the reconstrction to clean the tree
static int GRAYLEVEL = 256; // plugin works with 8 bit images
static int Kskip = 3; // subsampling factor (useful for large stacks to reduce unnecessary computation)
static int maxBlobCount = 5; // max nr. somas
static int minSomaSize  = 1; // in voxels

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;

    int     scal;       // scale
    int     perc;       // fg. percentile
    float   znccTh;     // correlation threshold
    int     Ndir;       // number of directions
    float   angSig;     // angular deviation
//    float   gcsSig;     // cross-section deviation
    int     Ni;         // number of iterations
    int     Ns;         // number of states
    float   zDist;      // the distance between layers in pixels
    int     saveMidres; // save midresults

};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList NeuronChaser::menulist() const
{
	return QStringList() 
		<<tr("nc_menu")
		<<tr("about");
}

QStringList NeuronChaser::funclist() const
{
	return QStringList()
		<<tr("nc_func")
		<<tr("help");
}

const QString title = QObject::tr("NeuronChaser Plugin");

void NeuronChaser::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("nc_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;

        // pick the default params
        PARA.channel = channel;
        PARA.scal = scal;
        PARA.perc = perc;
        PARA.znccTh = znccTh;
        PARA.Ndir   = Ndir;
        PARA.angSig = angSig;
//        PARA.gcsSig = gcsSig;
        PARA.Ni = Ni;
        PARA.Ns = Ns;
        PARA.zDist = zDist;
        PARA.saveMidres = saveMidres;

        /*
         *  input through the menu, assign PARA fields
         */

        // legend
        vector<string> items;
        items.push_back("Channel");
        items.push_back("Scale (5, 20] pix.");
        items.push_back("Percentile [50, 100].");
        items.push_back("Correlation threshold [0.5, 1.0).");
        items.push_back("nr. directions [5, 20].");
        items.push_back("Angular sigma [1,90] degs.");
//        items.push_back("Gaussian Cross Section sigma (1-10) [pix]");
        items.push_back("nr. iterations [2, 50].");
        items.push_back("nr. states [1, 20].");
        items.push_back("z layer dist [1, 4] pix.");
        items.push_back("Save midresults 0-no, 1 -yes.");

        // initialization
        vector<string> inits;
        inits.push_back(QString::number(PARA.channel).toStdString().c_str());
        inits.push_back(QString::number(PARA.scal).toStdString().c_str());
        inits.push_back(QString::number(PARA.perc).toStdString().c_str());
        inits.push_back(QString::number(PARA.znccTh).toStdString().c_str());
        inits.push_back(QString::number(PARA.Ndir).toStdString().c_str());
        inits.push_back(QString::number(PARA.angSig).toStdString().c_str());
//        inits.push_back(QString::number(PARA.gcsSig).toStdString().c_str());
        inits.push_back(QString::number(PARA.Ni).toStdString().c_str());
        inits.push_back(QString::number(PARA.Ns).toStdString().c_str());
        inits.push_back(QString::number(PARA.zDist).toStdString().c_str());
        inits.push_back(QString::number(PARA.saveMidres).toStdString().c_str());

//        CommonDialog dialog(items);
        CommonDialog dialog(items, inits);

        dialog.setWindowTitle(title);
        if(dialog.exec() != QDialog::Accepted) return;

        dialog.get_num("Channel", PARA.channel);
        dialog.get_num("Scale (5, 20] pix.", PARA.scal);
        dialog.get_num("Percentile [50, 100].", PARA.perc);
        dialog.get_num("Correlation threshold [0.5, 1.0).", PARA.znccTh);
        dialog.get_num("nr. directions [5, 20].", PARA.Ndir);
        dialog.get_num("Angular sigma [1,90] degs.", PARA.angSig);
//        dialog.get_num("Gaussian Cross Section sigma (1-10) [pix]", PARA.gcsSig);
        dialog.get_num("nr. iterations [2, 50].", PARA.Ni);
        dialog.get_num("nr. states [1, 20].", PARA.Ns);
        dialog.get_num("z layer dist [1, 4] pix.", PARA.zDist);
        dialog.get_num("Save midresults 0-no, 1 -yes.", PARA.saveMidres);

        // check input
        if(PARA.channel <= 0)                           {v3d_msg(QObject::tr("Channel is out of range")); return;}
        if(PARA.scal <= 5       || PARA.scal>20)        {v3d_msg(QObject::tr("Scale is out of range")); return;}
        if(PARA.perc < 50       || PARA.perc > 100)     {v3d_msg(QObject::tr("Percentile is out of range")); return;}
        if(PARA.znccTh < 0.5    || PARA.znccTh >= 1.0)  {v3d_msg(QObject::tr("Correlation is out of range")); return;}
        if(PARA.Ndir<5          || PARA.Ndir>20)        {v3d_msg(QObject::tr("# directions is out of range")); return;}
        if(PARA.angSig < 1      || PARA.angSig>90)      {v3d_msg(QObject::tr("Angular sigma is out of range")); return;}
//        if(PARA.gcsSig < 1      || PARA.gcsSig>10)      {v3d_msg(QObject::tr("Gaussian Cross Section sigma is out of range")); return;}
        if(PARA.Ni < 2          || PARA.Ni>50)          {v3d_msg(QObject::tr("# iterations is out of range")); return;}
        if(PARA.Ns <=  0        || PARA.Ns>20)          {v3d_msg(QObject::tr("# states is out of range")); return;}
        if(PARA.zDist < 1       || PARA.zDist>4    )    {v3d_msg(QObject::tr("zdist is out of range")); return;}
        if(PARA.saveMidres<0    || PARA.saveMidres>1)   {v3d_msg(QObject::tr("saveMidres has to be 0 or 1")); return;}

        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
        v3d_msg(tr("Neuron reconstruction plugin. "
            "Developed by Miroslav Radojevic, 2015-5-31"));
	}
}

void print_help(){
    printf("**** Usage of NeuronChaser tracing **** \n");
    printf("vaa3d -x NeuronChaser -f nc_func -i <inimg_file> -p <channel> <scal perc znccTh Ndir angSig Ni Ns zDist saveMidres>\n");
    printf("inimg_file          The input image\n");
    printf("channel             Data channel for tracing. Start from 1 (default 1).\n");
    printf("scal                Scale (5, 20] pix.\n");
    printf("perc                Percentile [50, 100].\n");
    printf("znccTh              Correlation threshold [0.5, 1.0).\n");
    printf("Ndir                nr. directions [5, 20].\n");
    printf("angSig              Angular sigma [1,90] degs.\n");
//        printf("gcsSig              Gaussian Cross Section sigma (1-10) [pix].\n");
    printf("Ni                  nr. iterations [2, 50].\n");
    printf("Ns                  nr. states [1, 20].\n");
    printf("zDist               z layer dist [1, 4] pix.\n");
    printf("saveMidres          Save midresults 0-no, 1 -yes.\n");
    printf("outswc_file         Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");
}

bool NeuronChaser::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("nc_func"))
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
        PARA.channel    = (paras.size() >= k+1)   ? atoi(paras[k])              : channel;   k++;
        PARA.scal       = (paras.size() >= k+1)   ? atoi(paras[k])              : scal;      k++;
        PARA.perc       = (paras.size() >= k+1)   ? atoi(paras[k])              : perc;      k++;
        PARA.znccTh     = (paras.size() >= k+1)   ? QString(paras[k]).toFloat() : znccTh;    k++;
        PARA.Ndir       = (paras.size() >= k+1)   ? atoi(paras[k])              : Ndir;      k++;
        PARA.angSig     = (paras.size() >= k+1)   ? QString(paras[k]).toFloat() : angSig;    k++;
//        PARA.gcsSig     = (paras.size() >= k+1)   ? QString(paras[k]).toFloat() : gcsSig;    k++;
        PARA.Ni         = (paras.size() >= k+1)   ? atoi(paras[k])              : Ni;        k++;
        PARA.Ns         = (paras.size() >= k+1)   ? atoi(paras[k])              : Ns;        k++;
        PARA.zDist      = (paras.size() >= k+1)   ? QString(paras[k]).toFloat() : zDist;     k++;
        PARA.saveMidres = (paras.size() >= k+1)   ? atoi(paras[k])              : saveMidres;k++;

        // check input
        if(PARA.channel <= 0)                            {v3d_msg(QObject::tr("Channel is out of range")); return false;}
        if(PARA.scal <= 5       || PARA.scal>20)         {v3d_msg(QObject::tr("Scale is out of range")); return false;}
        if(PARA.perc < 50       || PARA.perc > 100)      {v3d_msg(QObject::tr("Percentile is out of range")); return false;}
        if(PARA.znccTh < 0.5    || PARA.znccTh >= 1.0)   {v3d_msg(QObject::tr("Correlation is out of range")); return false;}
        if(PARA.Ndir<5          || PARA.Ndir>20)         {v3d_msg(QObject::tr("# directions is out of range")); return false;}
        if(PARA.angSig < 1      || PARA.angSig>90)       {v3d_msg(QObject::tr("Angular sigma is out of range")); return false;}
//        if(PARA.gcsSig < 1      || PARA.gcsSig>10)      {v3d_msg(QObject::tr("Gaussian Cross Section sigma is out of range")); return false;}
        if(PARA.Ni < 2         || PARA.Ni>50      )      {v3d_msg(QObject::tr("# iterations is out of range")); return false;}
        if(PARA.Ns <= 0         || PARA.Ns>20        )   {v3d_msg(QObject::tr("# states is out of range")); return false;}
        if(PARA.zDist < 1       || PARA.zDist>4    )    {v3d_msg(QObject::tr("Z layer dist is out of range")); return false;}
        if(PARA.saveMidres<0    || PARA.saveMidres>1)    {v3d_msg(QObject::tr("saveMidres has to be 0 or 1")); return false;}

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

// queue used for the breadth first search, basic usage:
//BfsQueue< vector<int> > boob;
//vector<int> lnk(2); lnk[0] = i1; lnk[1] = i2;
//boob.enqueue(lnk);
//vector<int> takeit = boob.dequeue();

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

int get_undiscovered(vector<int> indices, bool * disc) {

    for (int i = 0; i < indices.size(); i++) {  // go indices sorted by the correlation value

        if (indices[i]!=0) {

            if (!disc[indices[i]])
                return indices[i];

        }

    }

    return -1;

}

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

    v3d_msg("\nreconstruction_func...\n", 0);

//    srand (static_cast <unsigned> (time(0))); // needed for random

    cout<<"----------  NeuronChaser  ----------"   <<endl;
    cout<<"channel = "  <<PARA.channel              <<endl;
    cout<<"scal = "     <<PARA.scal                 <<endl;
    cout<<"perc = "     <<PARA.perc                 <<endl;
    cout<<"znccTh = "   <<PARA.znccTh               <<endl;
    cout<<"Ndir = "     <<PARA.Ndir                 <<endl;
    cout<<"angSig = "   <<PARA.angSig               <<endl;
//    cout<<"gcsSig = "   <<PARA.gcsSig               <<endl;
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

    ///////////////////////////////////////
    // erosion, variance
    ///////////////////////////////////////

    unsigned char * erd1d = new unsigned char[size]; // local min.
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

    if (PARA.saveMidres) {
        QString of = PARA.inimg_file + "_erd1d.tif";
        simple_saveimage_wrapper(callback, of.toStdString().c_str(), erd1d, in_sz, V3D_UINT8);
        of = PARA.inimg_file + "_scr1.tif";
        simple_saveimage_wrapper(callback, of.toStdString().c_str(), scr1, in_sz, V3D_UINT8);
    }

    // threshold
    unsigned char imth = intermodes_th(erd1d, size);
    for (int i = 0; i < size; ++i)
        erd1d[i] = (erd1d[i]>=imth)?255:0;

    if (PARA.saveMidres) {
        QString of = PARA.inimg_file + "_erd1dTh.tif";
        simple_saveimage_wrapper(callback, of.toStdString().c_str(), erd1d, in_sz, V3D_UINT8);
    }

    // dilate thresholded
    cout << "blobseg... " << flush;
    unsigned char * somaseg = new unsigned char[size];
    for (long i = 0; i < size; ++i)
        somaseg[i] = 0;

    for (long i = 0; i < size; ++i) {

        int x = i % N;
        int z = i / (N*M);
        int y = i/N-z*M;

        if (erd1d[i]==255) {

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

    delete erd1d; erd1d = 0;

    if (PARA.saveMidres) {
        QString of = PARA.inimg_file + "_somaseg.tif";
        simple_saveimage_wrapper(callback, of.toStdString().c_str(), somaseg, in_sz, V3D_UINT8);
    }

    ////////////////////////////////////////
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
    ////////////////////////////////////////

    int * tagmap = new int[size];   // size too large for static alloc
    for (long i = 0; i < size; ++i)
        tagmap[i] = 0;

    vector<float> xc, yc, zc, rc;
    conn3d(somaseg, N, M, P, tagmap, maxBlobCount, true, 0, minSomaSize, xc, yc, zc, rc); // connected c. -> soma labels and x,y,z,r

    cout << xc.size() << " blob(s)" << endl;
    for (int i = 0; i < xc.size(); ++i) {
        cout << xc[i] << " " << yc[i] << " " << zc[i] << " " << rc[i] << endl;
    }

    delete somaseg; somaseg = 0;

    ////////////////////////////////////////
    // percentile on score
    unsigned char th = quantile(scr, PARA.perc, 100);
    cout << "\t" << PARA.perc << "th = " << (int)th << endl;

    // assign those that are in the foreground with -1
    for (long i = 0; i < size; ++i) {
        if (scr1[i]>th && tagmap[i]==0)
            tagmap[i] = -1;
    }

    delete scr1; scr1 = 0;

//    unsigned char * conn = new unsigned char[size];
//    ConnectedComponents cc(30);
//    cc.connected(eroded1d, conn, N, M, std::equal_to<unsigned char>(), constant<bool,true>());
//    delete conn; conn = 0;
//    delete labimg; labimg = 0;

    if (PARA.saveMidres) {
        float * tagmapcopy = new float[size];
        for (long i = 0; i < size; ++i)
            tagmapcopy[i] = tagmap[i];

        QString of = PARA.inimg_file + "_tagmap_blob_and_fgbg.raw";//+QString("%1").arg(zz, 4, 10, QChar('0'))+".txt";
        char * of1 = new char[of.length()+1];
        strcpy(of1, of.toLatin1().constData());
        Image4DSimple outimg1;
        outimg1.setData((unsigned char *)tagmapcopy, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);
        callback.saveImage(&outimg1, of1);
        tagmapcopy = 0;
        delete of1; of1 = 0;
    }

    ////////////////////////////////////////
    // tracer
    BTracer btrcr(PARA.Ni, PARA.Ns, PARA.scal, P==1, PARA.zDist);

    if (PARA.saveMidres) {
        btrcr.save_templates(callback, PARA.inimg_file); // templates used for zncc()
    }
    ////////////////////////////////////////
    // count foreground locs for array allocation
    long cnt_fg = 0;
    for (long i = 0; i < size; ++i)
        if (tagmap[i]==-1)
            cnt_fg++;

    cout << setprecision(4) << cnt_fg/1000.0 << "k fg. locs, " << setprecision(2) << ((float)cnt_fg/size) << "%, at scale=" << PARA.scal << endl;

    ////////////////////////////////////////

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

    ////////////////////////////////////////
    cout << "\ngpnt calculation..." << endl;
    // book-keep all zncc calculations (zncc, gcsstd, vx, vy, vz)
    // in case they are accessed several times from the same location
    // this helps avoiding double calculation
    float ** i2calcs = new float*[cnt_fg];

    for (long i = 0; i < cnt_fg; ++i) {
        i2calcs[i] = new float[5]; // zncc, gcsstd, vx, vy, vz
        for (int j = 0; j < 5; ++j) {
            i2calcs[i][j] = -FLT_MAX; // compare with -FLT_MAX to know if they are caluculated at this location
        }
    }

    vector<Node> nodelist = vector<Node>(); // list of nodes (guidepoint nodes and trace nodes), reminder!! nodelist will have r as gcsstd
    Node dummynode(0,0,0,0,Node::UNDEFINED);//
    nodelist.push_back(dummynode); // nodes start from index 1 (index 0 would mix with background tag)

    // add nodes that correspond to somas

    for (int i = 0; i < xc.size(); ++i) {
        Node gpntnode(xc[i], yc[i], zc[i], rc[i]/btrcr.gcsstd2rad, Node::SOMA);
        nodelist.push_back(gpntnode);
    }

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
    float curr_gcsstd;

    int rx = PARA.scal; // search for local max, radius of the search
    int ry = PARA.scal;
    int rz = round(PARA.scal/zDist);

    int rxout, ryout, rzout;  // outer sphere radius
    int rxin,  ryin,  rzin;   // inner

    for (long i = 0; i < cnt_fg; ++i) { // go through all foreground locations (initially tagged as -1)

        if (i%(cnt_fg/10)==0) cout << (i/(cnt_fg/100)) << "%\t" << std::flush;

        int x  = i2xyz[i][0]; // get corresponding real locations in 3d
        int y  = i2xyz[i][1];
        int z  = i2xyz[i][2];

        // we go throught the foreground points - the tagmap value cannot be 0
        // we can skip location:
        // * if it is in the neighbourhood of the guidepoint node region (to avoid overlap)
        // * if it is in the guidepoint node region
        // * the point zncc score was calculated already when searching for a local zncc max in the neighbourhood of the candidate point

        if (tagmap[z*N*M+y*N+x]==-1) { // foreground location that can be a candidate for a node center

            if (i2calcs[i][0] == -FLT_MAX) { // calculate zncc at this location if it's not been done yet

                for (int dix = 0; dix < PARA.Ndir; ++dix) {
                    curr_zncc = btrcr.zncc(x, y, z, vx_corr[dix], vy_corr[dix], vz_corr[dix],
                                data1d, N, M, P,
                                curr_gcsstd,
                                false);
                    if (curr_zncc>i2calcs[i][0]) {
                        i2calcs[i][0] = curr_zncc;
                        i2calcs[i][1] = curr_gcsstd;
                        i2calcs[i][2] = vx_corr[dix];
                        i2calcs[i][3] = vy_corr[dix];
                        i2calcs[i][4] = vz_corr[dix];
                    }
                }
            }

            if (i2calcs[i][0]>PARA.znccTh) { // candidate for guidepoint, check local neighbourhood

                // guidepoint will be added - centered at this one or around a local max.
                long    lmax_i    = i;
                float   lmax_zncc = i2calcs[i][0];

                for (int xn = x-rx; xn <= x+rx; ++xn) { // serach zncc local max.
                    for (int yn = y-ry; yn <= y+ry; ++yn) {
                        for (int zn = z-rz; zn <= z+rz; ++zn) {

                            if (!(xn==x && yn==y && zn==z)) { // skip the midpoint

                                if (xn>=0 && xn<N && yn>=0 && yn<M && zn>=0 && zn<P) { // is in image

                                    if (tagmap[zn*N*M+yn*N+xn]==-1) { // not in background (0), not in other region (>0) and would not overlap (-2)

                                        if (pow(xn-x,2)/pow(rx,2)+pow(yn-y,2)/pow(ry,2)+pow(zn-z,2)/pow(rz,2)<=1) { // in the spherical neighbourhood (this is a bit expensive in loop)

                                            long in = xyz2i[zn*N*M+yn*N+xn];

                                            if (i2calcs[in][0] == -FLT_MAX) { // calculate value if not calculated

                                                for (int dix = 0; dix < PARA.Ndir; ++dix) {

                                                    curr_zncc = btrcr.zncc(xn, yn, zn, vx_corr[dix], vy_corr[dix], vz_corr[dix], data1d, N, M, P, curr_gcsstd, false);

                                                    if (curr_zncc>i2calcs[in][0]) {
                                                        i2calcs[in][0] = curr_zncc;
                                                        i2calcs[in][1] = curr_gcsstd;
                                                        i2calcs[in][2] = vx_corr[dix];
                                                        i2calcs[in][3] = vy_corr[dix];
                                                        i2calcs[in][4] = vz_corr[dix];
                                                    }
                                                }
                                            }

                                            if (i2calcs[in][0]>lmax_zncc) { // compare it with the current local max
                                                lmax_i    = in; // upate local max foreground index
                                                lmax_zncc = i2calcs[in][0];

                                            }
                                        }
                                    }
                                }
                            }
                        } // zn
                    } // yn
                } // xn

                // add node with direction (guidepoint node)
                int gpntx = i2xyz[lmax_i][0];
                int gpnty = i2xyz[lmax_i][1];
                int gpntz = i2xyz[lmax_i][2];

                float gpntr = i2calcs[lmax_i][1]; // gcsstd is added as radius

                float gpntvx = i2calcs[lmax_i][2];
                float gpntvy = i2calcs[lmax_i][3];
                float gpntvz = i2calcs[lmax_i][4];

                float gpntcorr = i2calcs[lmax_i][0];

                Node gpntnode((float)gpntx, (float)gpnty, (float)gpntz, gpntvx, gpntvy, gpntvz, gpntcorr, gpntr, Node::UNDEFINED);
                nodelist.push_back(gpntnode);
                int nodetag = nodelist.size()-1; // nodetag will correspond to the index of appended node

                // added the guidepoint!!
                // fill the tags associated with this guidepoint node
                rxin  =  ryin  =        ceil(btrcr.gcsstd2rad * i2calcs[lmax_i][1]);
                rzin  =                 ceil(btrcr.gcsstd2rad * i2calcs[lmax_i][1]/zDist);

                rxout =  ryout =        ceil(2*rxin); //rxin + PARA.scal;// + round(btrcr.gcsstd2rad * btrcr.gcsstd_max);
                rzout =                 ceil(2*rzin); //ryin + PARA.scal;// + round(btrcr.gcsstd2rad * btrcr.gcsstd_max / zDist);

                for (int xnn = gpntx-rxout; xnn <= gpntx+rxout; ++xnn) {
                    for (int ynn = gpnty-ryout; ynn <= gpnty+ryout; ++ynn) {
                        for (int znn = gpntz-rzout; znn <= gpntz+rzout; ++znn) {
                            if (xnn>=0 && xnn<N && ynn>=0 && ynn<M && znn>=0 && znn<P) { // is in image

                                float x2 = pow(xnn-gpntx,2);
                                float y2 = pow(ynn-gpnty,2);
                                float z2 = pow(znn-gpntz,2);

                                bool isouter = x2/pow(rxout,2)+y2/pow(ryout,2)+z2/pow(rzout,2)<=1;

                                if (isouter) {

                                    bool isinner = x2/pow(rxin,2)+y2/pow(ryin,2)+z2/pow(rzin,2)<=1;

                                    if (isinner) {
                                        int currenttag = tagmap[znn*N*M+ynn*N+xnn];
                                        if (currenttag==-1 || currenttag==-2 || currenttag==0) {
                                            // overwiriting those tags makes sense
                                            tagmap[znn*N*M+ynn*N+xnn] = nodetag;
                                        }
                                        else {
                                            // add the link between existing tag (currenttag) and the current tag (nodetag)
                                            nodelist[nodetag].nbr.push_back(currenttag);
                                            nodelist[currenttag].nbr.push_back(nodetag);
                                            //cout << "\nHappened@" << endl;
                                        }
                                    }
                                    else { // overwrite -1, 0
                                        int currenttag = tagmap[znn*N*M+ynn*N+xnn];
                                        if (currenttag==-1 || currenttag==0) {
                                            // overwriting makes sense
                                            tagmap[znn*N*M+ynn*N+xnn] = -2;
                                        }

                                    }
                                }
                            }
                        }
                    }
                }

            } // if (i2calcs[i][0]>PARA.znccTh)
        } // if (tagmap[z*N*M+y*N+x]==-1)
    } // for (long i = 0; i < cnt_fg; ++i)

    cout << "DONE." << endl;

    ////////////////////////////////////////
    // expand foreground to accomodate traces
    ////////////////////////////////////////
    cout << "\nexpand tagmap..." << flush;

    int * tagmap_temp = new int[size]; // tagmap copy
    for (int i = 0; i < size; ++i)
        tagmap_temp[i] = tagmap[i];

    // go through all of tagmap copy's non-zero elements and dilate with -1s, substituting zeros with -1s in original tagmap

    // offsets XYZ
//    limxy = ceil( PARA.scal/2.0);
//    limz  = 1;//ceil((PARA.scal/1.0)/zDist);
    vector<int> dx_XYZ, dy_XYZ, dz_XYZ;
    for (int dx = -limxy; dx <= limxy; ++dx) {
        for (int dy = -limxy; dy <= limxy; ++dy) {
            for (int dz = -limz; dz <= limz; ++dz) {
                if (pow(dx,2)/pow(limxy,2)+pow(dy,2)/pow(limxy,2)+pow(dz,2)/pow(limz,2)<=1) {
                    dx_XYZ.push_back(dx);
                    dy_XYZ.push_back(dy);
                    dz_XYZ.push_back(dz);
                }
            }
        }
    }

    for (long i = 0; i < size; ++i) {

        int x  = i % N;
        int z  = i / (N*M);
        int y  = i/N-z*M;

        if (tagmap_temp[z*N*M+y*N+x]==0)
            continue; // skip those that are zero

        for (int k = 0; k < dx_XYZ.size(); ++k) {

                int xi = x+dx_XYZ[k];
                int yi = y+dy_XYZ[k];
                int zi = z+dz_XYZ[k];

                if (xi>=0 && xi<N && yi>=0 && yi<M && zi>=0 && zi<P) { // inside image boundaries
                    int ii = zi*N*M + yi*N + xi;
                    if (tagmap[ii]==0)
                        tagmap[ii] = -1;
                }
        }

    }

    delete tagmap_temp; tagmap_temp = 0;

    cout << "done.\n--------\n"<< endl;

    // test: check how many locations were calculated and how large they were
    int cnt_skipped = 0;
    float znccmax = -99;
    float znccmin = 99;
    for (int ii = 0; ii < cnt_fg; ++ii) {
        if (i2calcs[ii][0] == -FLT_MAX) {
            cnt_skipped++;
        }
        else {
            if (i2calcs[ii][0]>znccmax) znccmax = i2calcs[ii][0];
            if (i2calcs[ii][0]<znccmin) znccmin = i2calcs[ii][0];
        }
    }

    cout << setprecision(4) << cnt_fg/1000.0 << "k " << flush;
    cout << setprecision(4) << (cnt_skipped/(float)cnt_fg)*100.0 << "% skipped" << endl;
    cout << setprecision(2) << znccmin << " zncc min, " << flush;
    cout << setprecision(2) << znccmax << " zncc max" << endl;

    // new tagmap after calculating
    long cnt_new_fg = 0;
    for (long ii = 0; ii < size; ++ii) {
        if (tagmap[ii]!=0) cnt_new_fg++;
    }
    cout << setprecision(4) << cnt_new_fg/1000.0 << "k after expanding." << endl;

    ////////////////////////////////////////
    for (int i = 0; i < cnt_fg; ++i) {delete i2xyz[i]; delete i2calcs[i];}
    delete i2xyz; i2xyz = 0;
    delete i2calcs; i2calcs = 0;
    delete xyz2i; xyz2i = 0;
    ////////////////////////////////////////

    if (PARA.saveMidres) {
        float * tagmapcopy = new float[size];
        for (long i = 0; i < size; ++i)
            tagmapcopy[i] = tagmap[i];

        QString of = PARA.inimg_file + "_tagmap_gpnts.raw";//+QString("%1").arg(zz, 4, 10, QChar('0'))+".txt";
        char * of1 = new char[of.length()+1];
        strcpy(of1, of.toLatin1().constData());
        Image4DSimple outimg1;
        outimg1.setData((unsigned char *)tagmapcopy, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);
        callback.saveImage(&outimg1, of1);
        tagmapcopy = 0;
        delete of1; of1 = 0;
    }

    if (PARA.saveMidres) { // save guidepoint nodes into swc

        NeuronTree gpntswc;

        for (int ni = 1; ni < nodelist.size(); ++ni) { // first element of the nodelist will be 0

            NeuronSWC nn;
            nn.n = nn.nodeinseg_id = ni;
            nn.type = (ni<=xc.size())? Node::SOMA : Node::APICAL_DENDRITE;
            nn.x = nodelist[ni].x;
            nn.y = nodelist[ni].y;
            nn.z = nodelist[ni].z;
            nn.r = btrcr.gcsstd2rad * nodelist[ni].r;
            nn.parent = -1;
            gpntswc.listNeuron.append(nn);

        }

        QString gpntswc_name = PARA.inimg_file + "_gpnt.swc";
        gpntswc.name = "GuidepointLocs";
        writeSWC_file(gpntswc_name.toStdString().c_str(), gpntswc);

    }

    ////////////////////////////////////////
    // arrange guidepoint nodes by correlation score (start chasing from the one with highest correlation)
    // std::sort(nodelist.begin(), nodelist.end(), compareNode); // highest correlation first
    // tags have been assigned to the index of the node in the list and changing the position in the list with sorting will deteriorate the tagmap
    // sort indices and trace from the top of the sorted original indices
    // sorted base od corr score
    vector<int> indices(nodelist.size()); // nr nodes before the tracing starts
    for (int i = 0; i < indices.size(); ++i) indices.at(i) = i; // std::iota does the same
    sort(indices.begin(), indices.end(), CompareIndicesByNodeVectorCorrValues(&nodelist));
    ////////////////////////////////////////

    if (PARA.saveMidres) { // ttrace

        int tt_i    = indices.at(0);
        float tt_x  = nodelist[tt_i].x;
        float tt_y  = nodelist[tt_i].y;
        float tt_z  = nodelist[tt_i].z;
        float tt_vx = nodelist[tt_i].vx;
        float tt_vy = nodelist[tt_i].vy;
        float tt_vz = nodelist[tt_i].vz;
        float tt_r  = nodelist[tt_i].r;

        NeuronTree ttraceswc;

        NeuronSWC n0;
        n0.n = n0.nodeinseg_id = tt_i;
        n0.type = 6;
        n0.x = tt_x;
        n0.y = tt_y;
        n0.z = tt_z;
        n0.r = btrcr.gcsstd2rad * tt_r;
        n0.parent = -1;
        ttraceswc.listNeuron.append(n0);

        // tagmap only for this trace - to disable stopping the trace due to tag interruption
        int * tt_tagmap = new int[size]; // again static would be ok but size of the array does not allow it
        for (long iii = 0; iii < size; ++iii) {tt_tagmap[iii] = tt_i;} // initialize test tagmap with tt_i

        // trace in one direction
        vector<int> out1 = btrcr.trace(tt_x,tt_y,tt_z,tt_vx,tt_vy,tt_vz,tt_r,data1d,N,M,P,PARA.angSig,tt_tagmap, tt_i, false); // PARA.gcsSig,

//        for (int var = 0; var < out1.size(); ++var) {
//            cout << out1[var] << " " << flush;
//        }
//        cout << endl;

        for (int ni = 0; ni < btrcr.node_cnt; ++ni) {
            int node_cnt = ttraceswc.listNeuron.back().n+1;
            NeuronSWC nn;
            nn.n = nn.nodeinseg_id = node_cnt;
            nn.type = 4;
            nn.x = btrcr.xc[ni];
            nn.y = btrcr.yc[ni];
            nn.z = btrcr.zc[ni];
            nn.r = btrcr.gcsstd2rad * btrcr.rc[ni];
            nn.parent = (ni==0)? tt_i : node_cnt-1;
            ttraceswc.listNeuron.append(nn);
        }

        // trace in the other direction
        vector<int> out2 = btrcr.trace(tt_x,tt_y,tt_z,-tt_vx,-tt_vy,-tt_vz,tt_r,data1d,N,M,P,PARA.angSig, tt_tagmap, tt_i, false); // PARA.gcsSig,

        for (int ni = 0; ni < btrcr.node_cnt; ++ni) {
            int node_cnt = ttraceswc.listNeuron.back().n+1;
            NeuronSWC nn;
            nn.n = nn.nodeinseg_id = node_cnt;
            nn.type = 4;
            nn.x = btrcr.xc[ni];
            nn.y = btrcr.yc[ni];
            nn.z = btrcr.zc[ni];
            nn.r = btrcr.gcsstd2rad * btrcr.rc[ni];
            nn.parent = (ni==0)? tt_i : node_cnt-1;
            ttraceswc.listNeuron.append(nn);
        }

        QString ttraceswc_name = PARA.inimg_file + "_ttrace.swc";
        ttraceswc.name = "TestTrace";
        writeSWC_file(ttraceswc_name.toStdString().c_str(), ttraceswc);

        delete tt_tagmap; tt_tagmap = 0;
    }

    ////////////////////////////////////////
    // tracing constrained with tagmaps, starts from the highest corr guidepoint and grows the map
    // final output is the list of linked nodes  (save individual nodes)
    cout << "chasing... " << endl;
    for (int ni = 0; ni < indices.size(); ++ni) {

        if (ni%(indices.size()/10)==0) cout << ni/(indices.size()/10)*10 << "%\t" << flush;

        for (int vsgn = -1; vsgn <= 1; vsgn+=2) { // -1, 1 direction

            int tt_i    = indices.at(ni); // get the nodelist index

            // skip dummy node and soma nodes as they are directionless
            if (tt_i < xc.size()+1) continue; // node(0) is dummy node in nodelist and xc.size() nodes after are all without direction

            float tt_x  = nodelist.at(tt_i).x;
            float tt_y  = nodelist.at(tt_i).y;
            float tt_z  = nodelist.at(tt_i).z;
            float tt_vx = nodelist.at(tt_i).vx;
            float tt_vy = nodelist.at(tt_i).vy;
            float tt_vz = nodelist.at(tt_i).vz;
            float tt_r  = nodelist.at(tt_i).r;

            if (tt_vx==-FLT_MAX || tt_vy==-FLT_MAX || tt_vz==-FLT_MAX) {
                // shouldn't happen, check nevertheless
                cout << "ERROR: trace cannot start directe with vx, vy, vz" << endl;
                return;
            }

            vector<int> outcome_tags = btrcr.trace(tt_x, tt_y, tt_z,
                        vsgn*tt_vx, vsgn*tt_vy, vsgn*tt_vz, tt_r,
                        data1d, N, M, P,
                        PARA.angSig,
                        tagmap, tt_i, false); // each trace will use updated tagmap // PARA.gcsSig,

            // there will be at least one member in the vector
            if (outcome_tags[0]>0 || outcome_tags[0]==-2) { // if the trace will be added

                for (int ti = 0; ti <= btrcr.node_cnt; ++ti) {

                    // add node to the nodelist
                    Node gpntnode(btrcr.xc[ti], btrcr.yc[ti], btrcr.zc[ti], btrcr.rc[ti], Node::AXON); // reminder!! tracer's r will be gcsstd
                    nodelist.push_back(gpntnode); // reminder!! nodelist will have r as gcsstd
                    int nodetag = nodelist.size()-1;

                    // add bidirectional linking
                    if (ti==0) {
                        // towards previous
                        nodelist[nodetag].nbr.push_back(tt_i);
                        nodelist[tt_i].nbr.push_back(nodetag);

                        // radius equalization
                        float ravg = 0.5*(nodelist[tt_i].r + nodelist[nodetag].r);
                        nodelist[tt_i].r    = ravg;
                        nodelist[nodetag].r = ravg;
                    }

                    if (ti==btrcr.node_cnt && outcome_tags[0]>0) {
                        // it is the last one vector with tags is reached, reached tag==-2 means tag was not reached so don't add tip linking
                        for (int k1 = 0; k1 < outcome_tags.size(); ++k1) {
                            nodelist[nodetag].nbr.push_back(outcome_tags[k1]);
                            nodelist[outcome_tags[k1]].nbr.push_back(nodetag);

                            // radius equalization
                            float ravg = 0.5*(nodelist[outcome_tags[k1]].r + nodelist[nodetag].r);
                            nodelist[outcome_tags[k1]].r = ravg;
                            nodelist[nodetag].r          = ravg;

                        }

                    }

                    if (ti>0) { // previous will exist
                        nodelist[nodetag].nbr.push_back(nodetag-1);
                        nodelist[nodetag-1].nbr.push_back(nodetag);
                    }

                    // trace tagging - fill the tagmap with current nodetag - the same way it was in guidepoint extraction and trace checking
                    // add the links in case there was an overlap in between node regions
                    float x_sph = nodelist.at(nodetag).x;
                    float y_sph = nodelist.at(nodetag).y;
                    float z_sph = nodelist.at(nodetag).z;
                    float rxy_sph = btrcr.gcsstd2rad * nodelist.at(nodetag).r;
                    float rz_sph  = (btrcr.gcsstd2rad * nodelist.at(nodetag).r)/zDist;

                    int x1 = floor(x_sph - rxy_sph);
                    int x2 = ceil( x_sph + rxy_sph);
                    int y1 = floor(y_sph - rxy_sph);
                    int y2 = ceil( y_sph + rxy_sph);
                    int z1 = floor(z_sph - rz_sph);
                    int z2 = ceil( z_sph + rz_sph);

                    for (int xfill = x1; xfill <= x2; ++xfill) {
                        for (int yfill = y1; yfill <= y2; ++yfill) {
                            for (int zfill = z1; zfill <= z2; ++zfill) {
                                if (xfill>=0 && xfill<N && yfill>=0 && yfill<M && zfill>=0 && zfill<P) { // is in image (general 2d/3d)

                                    // overwriting those tags makes sense, if it is 0+ don't overwrite already tagged ones
                                    float x2 = pow(xfill-x_sph,2);
                                    float y2 = pow(yfill-y_sph,2);
                                    float z2 = pow(zfill-z_sph,2);

                                    bool isinelipse = x2/pow(rxy_sph,2)+y2/pow(rxy_sph,2)+z2/pow(rz_sph,2)<=1;

                                    if (isinelipse) {
                                        int currenttag = tagmap[zfill*N*M+yfill*N+xfill];
                                        if (currenttag==-1 || currenttag==-2 || currenttag==0) {
                                            tagmap[zfill*N*M+yfill*N+xfill] = nodetag;
                                        }
                                        else {
                                            // link them (nodetag @ currenttag)
                                            nodelist[nodetag].nbr.push_back(currenttag);
                                            nodelist[currenttag].nbr.push_back(nodetag);
                                        }
                                    }

                                }
                            }
                        }
                    }

                }

            }

        }

    }

    cout << "DONE.\t" << nodelist.size() << " nodes" << endl;

    // remove double neighbourhoods from the neighbour list for each node
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

        QString recnodesswc_name = PARA.inimg_file + "_recnodes.swc";
        recnodesswc.name = "ReconstructionNodes";
        writeSWC_file(recnodesswc_name.toStdString().c_str(), recnodesswc);

    }

    if (PARA.saveMidres) {
        float * tagmapcopy = new float[size];
        for (long i = 0; i < size; ++i)
            tagmapcopy[i] = tagmap[i];

        QString of = PARA.inimg_file + "_tagmap_chases.raw";//+QString("%1").arg(zz, 4, 10, QChar('0'))+".txt";
        char * of1 = new char[of.length()+1];
        strcpy(of1, of.toLatin1().constData());
        Image4DSimple outimg1;
        outimg1.setData((unsigned char *)tagmapcopy, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);
        callback.saveImage(&outimg1, of1);
        tagmapcopy = 0;
        delete of1; of1 = 0;
    }

    delete tagmap; tagmap = 0;

    ////////////////////////////////////////
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
    int         L[nodelist.size()]; // len store the distance to backtrack towards nearest CP
    int         I2Swc[nodelist.size()];// map node index with corresponding swc index

    // initialize
    for (int i = 0; i < nodelist.size(); ++i) {
        D[i]        = (i==0)?true:false;
        I2Swc[i]    = -1; // undefined
        L[i]        = -1; // undefined
    }

    NeuronTree nt; // selected output tree
    int maxtreenodes = -1;
    int seedidx;
    bool fisrttime = true; // first time seed from blobs

    while ((seedidx = get_undiscovered(indices, D))!=-1) {

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
                "NeuronChaser\n#author: Miroslav (miroslav.radojevic@gmail.com)\n#params:\n#channel="+QString("%1").arg(PARA.channel)+
                "\n#scal="+QString("%1").arg(PARA.scal)+
                "\n#perc="+QString("%1").arg(PARA.perc)+
                "\n#znccTh="+QString("%1").arg(PARA.znccTh)+
                "\n#Ndir="+QString("%1").arg(PARA.Ndir)+
                "\n#angSig="+QString("%1").arg(PARA.angSig)+
                "\n#Ni="+QString("%1").arg(PARA.Ni)+
                "\n#Ns="+QString("%1").arg(PARA.Ns)+
                "\n#zDist="+QString("%1").arg(PARA.zDist)+
                "\n#---------------------\n#Kskip="+QString("%1").arg(Kskip)+
//                "\n#prune_len="+QString("%1").arg(prune_len)+
                "\n#maxBlobCount="+QString("%1").arg(maxBlobCount)+
                "\n#minSomaSize="+QString("%1").arg(minSomaSize);

        for (int i = 0; i < seed.size(); i++) {
            // enqueue(), add to FIFO structure, http://en.wikipedia.org/wiki/Queue_%28abstract_data_type%29
            vector<int> lnk(2);
            lnk[0] = -1;
            lnk[1] = seed[i];
            boob.enqueue(lnk);

            D[seed[i]] = true; // mark as discovered
            L[seed[i]] = 0; // information on length toowards CP
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
                    L[next] = L[curr] + 1;

                    nextlist.push_back(next);

                }
            }

            if (nextlist.size()==0 && nodelist[curr].nbr.size()>1) { // termination in tree search
//                ntcurr.listNeuron.last().type = Node::END;
//                for (int k = 0; k < L[curr]; ++k)
                    ntcurr.listNeuron.removeLast();
            }
            else if (nextlist.size()>1) {
//                for (int k = 0; k < nextlist.size(); ++k)
//                    L[nextlist[k]] = 1; // reset if they are expanding from CP
            }
        }

        cout << "DONE. " <<  ntcurr.listNeuron.size() << " nodes found." << endl;

        if (ntcurr.listNeuron.size()>maxtreenodes) {
            maxtreenodes = ntcurr.listNeuron.size();
            nt = ntcurr; // keep the one with highest number of nodes in nt
        }
        if (false) { // can be enabled to save the rest
            QString ntcurrswc_name = PARA.inimg_file+"_NeuronChaser_"+QString("%1").arg(ntcurr.listNeuron.size(), 4, 10, QChar('0'))+".swc";
                writeSWC_file(ntcurrswc_name.toStdString().c_str(), ntcurr);
        }

    }

    cout << "\ntree with " << nt.listNeuron.size() << " nodes selected.\n" << endl;

    QString ntswc_name = PARA.inimg_file + "_NeuronChaser.swc";
    writeSWC_file(ntswc_name.toStdString().c_str(), nt);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(ntswc_name.toStdString().c_str()),bmenu);

    return;
}
