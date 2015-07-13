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

// input parameter default values (suggeted to user in menu)
static long     channel = 1;
static int      scal    = 10;
static int      perc    = 80;
static float    znccTh  = 0.75;
static int      Ndir    = 11;
static float    angSig  = 60;
//static float    gcsSig  = 3;
static int      Ni      = 30;
static int      Ns      = 10;
static float    zDist   = 1.0;
static int      saveMidres = 0;

static int nrInputParams = 10;

// not necessary to tune
static unsigned char imgTh = 1; // low end of the intensity range
static int prune_len = 3; // those that reach endpoint after 1 step are removed from the reconstrction to clean the tree

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

int get_undiscovered(vector<int> indices, vector<bool> * discovered) {

//    cout << "get_undiscovered()" << endl;

    for (int i = 0; i < indices.size(); i++) {  // go through node indices sorted by the correlation value

        if (indices[i]!=0) {

            // check if there are any undiscovered here and if there are stop and return the node index
            for (int j = 0; j < discovered[indices[i]].size(); j++) {
                if (!discovered[indices[i]][j]) { // get out as soon as there is at leas one undiscovered
                    return indices[i];
                }
            }

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
    printf("\ninimg:\t%d x %d x %d \n", N, M, P);

    // tracer
    BTracer btrcr(PARA.Ni, PARA.Ns, PARA.scal, P==1, PARA.zDist);

    if (PARA.saveMidres) {
        btrcr.save_templates(callback, PARA.inimg_file); // templates used for zncc()
    }

////////////////////////////////////////////////////////////////////////////////////////////////////

    cout << "\nfg. score..."<<flush;

    unsigned char * scr = new unsigned char [size]; // foreground score (size is usually huge for 3d stacks so that synamic allocation is chosen instead of static as static allocation would break for large arrays)
    for (long var = 0; var < size; ++var) scr[var] = 0;

    int range_xy = round(PARA.scal/2.0);
    int range_z  = (P==1)?0:round((PARA.scal/2.0)/zDist); // range_z is 0 for 2d image   zDist

    // xy

    unsigned char * nbhood_xy = new unsigned char [(2*range_xy+1)*(2*range_xy+1)];

    vector<int> dx_xy;
    vector<int> dy_xy;
    vector<int> dz_xy;

    for (int dx = -range_xy; dx <= range_xy; ++dx) {
        for (int dy = -range_xy; dy <= range_xy; ++dy) {
            dx_xy.push_back(dx);
            dy_xy.push_back(dy);
            dz_xy.push_back(0);
        }
    }

    // xz (in 3d)

    unsigned char * nbhood_xz = new unsigned char [(2*range_xy+1)*(2*range_z +1)];

    vector<int> dx_xz;
    vector<int> dy_xz;
    vector<int> dz_xz;

    for (int dx = -range_xy; dx <= range_xy; ++dx) {
        for (int dz = -range_z; dz <= range_z; ++dz) {
            dx_xz.push_back(dx);
            dy_xz.push_back(0);
            dz_xz.push_back(dz);
        }
    }

    // yz (in 3d)

    unsigned char * nbhood_yz = new unsigned char [(2*range_xy+1)*(2*range_z +1)];

    vector<int> dx_yz;
    vector<int> dy_yz;
    vector<int> dz_yz;

    for (int dy = -range_xy; dy <= range_xy; ++dy) {
        for (int dz = -range_z; dz <= range_z; ++dz) {
            dx_yz.push_back(0);
            dy_yz.push_back(dy);
            dz_yz.push_back(dz);
        }
    }

    unsigned char m05_xy, m95_xy, m05_xz, m95_xz, m05_yz, m95_yz;

    for(long i = 0; i < size; i++) {

        int x  = i % N;
        int z  = i / (N*M);
        int y  = i/N-z*M; // (M-1) - (i/N-z*M);

        if (x>=range_xy && x<N-range_xy && y>=range_xy && y<M-range_xy && ((P==1)?true:(z>=range_z && z<P-range_z)) && data1d[i]>imgTh ) {

            // xy
            for (int k = 0; k < dx_xy.size(); ++k) {
                nbhood_xy[k] = data1d[(z+dz_xy[k])*N*M+(y+dy_xy[k])*N+(x+dx_xy[k])];
            }

            m05_xy = quantile(nbhood_xy, dx_xy.size(), 5,  100);
            m95_xy = quantile(nbhood_xy, dx_xy.size(), 95, 100);

            if (P>1){
                // xz, yz
                for (int k = 0; k < dx_xz.size(); ++k) {
                    nbhood_xz[k] = data1d[(z+dz_xz[k])*N*M+(y+dy_xz[k])*N+(x+dx_xz[k])];
                    nbhood_yz[k] = data1d[(z+dz_yz[k])*N*M+(y+dy_yz[k])*N+(x+dx_yz[k])];
                }

                m05_xz = quantile(nbhood_xz, dx_xz.size(), 5,  100);
                m95_xz = quantile(nbhood_xz, dx_xz.size(), 95, 100);

                m05_yz = quantile(nbhood_yz, dx_yz.size(), 5,  100);
                m95_yz = quantile(nbhood_yz, dx_yz.size(), 95, 100);
            }


            scr[z*N*M+y*N+x] = (P>1)?maximum(m95_xy-m05_xy, m95_xz-m05_xz, m95_yz-m05_yz):(m95_xy-m05_xy);

        }
        else // score set to zero if nbhood went out of the image boundaries
            scr[z*N*M+y*N+x] = 0;

    }

    delete nbhood_xy; nbhood_xy = 0;
    delete nbhood_xz; nbhood_xz = 0;
    delete nbhood_yz; nbhood_yz = 0;

    cout << "done."<< endl;

    if (PARA.saveMidres) { // save scr
        QString of = PARA.inimg_file + "_scr.tif";
        simple_saveimage_wrapper(callback, of.toStdString().c_str(), scr, in_sz, V3D_UINT8);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// percentile threshold on score
    ///

    unsigned char * scr1 = new unsigned char[size];
//    unsigned char scrmin = scr[0];
//    unsigned char scrmax = scr[0];
    for (long ii = 0; ii < size; ++ii) {
        scr1[ii]  = scr[ii];
//        if (scr[ii]>scrmax) scrmax = scr[ii];
//        if (scr[ii]<scrmin) scrmin = scr[ii];
    }

    unsigned char th = quantile(scr1, size, PARA.perc, 100);
    delete scr1; scr1 = 0;

    cout << PARA.perc << "th = " << (int)th << endl;
//    printf("th=%d\n", th);
//    cout << "scrmin = " << (int)scrmin << endl;
//    printf("scrmin=%d\n", scrmin);
//    cout << "scrmax = " << (int)scrmax << endl;
//    printf("scrmax=%d\n", scrmax);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // tagmap define - this one will bookmark the pixels of the stack:
    // 0    : background
    // -1   : foreground
    // -2   : mask this foreground location from calculating
    // 1    : node<1>
    // 2    : node<2>
    // 3    : node<3>
    // ...
    // tag will correspond to the node index in the list
    // node belonging to guidepoint or trace

    int * tagmap = new int[size];
//    int tagmap[size]; // tagmap can be allocated statically in principle but the size of the array is huge so that it cannot be allocated, unless dynamically
    for (long iii = 0; iii < size; ++iii) {tagmap[iii] = 0;} // initialize tagmap with 0

    cout << "\ngenerate fg. map..." << flush;

    int rang_xy = PARA.scal;
    int rang_z = round(PARA.scal/zDist);

    // assign those that are in the foreground with -1
    for (long i = 0; i < size; ++i) {

        int x  = i % N;
        int z  = i / (N*M);
        int y  = i/N-z*M;

//        cout << x << ", " << y << ", " << z << " -- " << (int)(scr[z*N*M+y*N+x]) << " > " << (int)th << "  :::  " << (scr[z*N*M+y*N+x]>th) << endl;
//        if (tagmap[z*N*M+y*N+x]==-1) continue;

        if (x>=rang_xy && x<N-rang_xy && y>=rang_xy && y<M-rang_xy && (P==1)?true:(z>=rang_z && z<P-rang_z)) {

            if (scr[z*N*M+y*N+x]>th) { // assign to all the points

                // first solution
//                for (int xx = x-rang_xy; xx <= x+rang_xy; ++xx) {
//                    for (int yy = y-rang_xy; yy <= y+rang_xy; ++yy) {
//                        for (int zz = z-rang_z; zz <= z+rang_z; ++zz) {
//                            if (
//                                    pow(xx-x,2)/pow(rang_xy,2)+
//                                    pow(yy-y,2)/pow(rang_xy,2)+
//                                    pow(zz-z,2)/pow(rang_z,2)<=1) {
//                                tagmap[zz*N*M+yy*N+xx] = -1; // -1 is tagging the foreground location
//                            }
//                        }
//                    }
//                }

                // avoid dilating now, do it after the points are extracted
                tagmap[z*N*M+y*N+x] = -1;

            }

        }

    }

    delete scr; scr = 0;

    cout << "done."<< endl;

    if (PARA.saveMidres) {
        // save int* tagmap as set of .txt images (uint8 is not enough to save the range and saving float image regular way did not work)
        cout << "exporting tagmap (1: fg-bg): ";
        for (int zz = 0; zz < P; ++zz) {
            cout << zz << ", "<<flush;
            QString of = PARA.inimg_file + "_tagmap_fgbg_"+QString("%1").arg(zz, 4, 10, QChar('0'))+".txt";
            ofstream myfile;
            myfile.open (of.toStdString().c_str());
            for (int yy = 0; yy < M; ++yy) {
                for (int xx = 0; xx < N; ++xx) {
                    myfile << tagmap[zz*(N*M)+yy*N+xx] << " ";
                }
                myfile << endl;
            }
            myfile.close();
        }
        cout << "done exporting." << endl;
    }




    // count foreground locs for array allocation
    long cnt_fg = 0;
    for (long i = 0; i < size; ++i) {if (tagmap[i]==-1) cnt_fg++;}

    cout << setprecision(4) << cnt_fg/1000.0 << "k fg. locations, at scale=" << PARA.scal << endl;
//if (true) {cout << "EXIT()"<<endl; return;}
////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////

    cout << "\nGPNT CALCULATION..." << endl;

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
    nodelist.push_back(dummynode); // nodes start from index 1 (0 index would mix with background tag)

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

//        cout << vx_corr[k] << "; " << vy_corr[k] << "; " << vz_corr[k] << "; " << endl; // << sqrt(vx_corr[k]*vx_corr[k]+vy_corr[k]*vy_corr[k]+vz_corr[k]*vz_corr[k])

    }

    float curr_zncc;
    float curr_gcsstd;

    int rx = PARA.scal; // search for local max radius
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

                                    if (tagmap[zn*N*M+yn*N+xn]==-1) { // not in background, not in other region and would not overlap

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

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// dilate foreground so that the traces are allowed
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    cout << "\ndilate tagmap..." << flush;

    int * tagmap_temp = new int[size]; // temporary map for dilatation, will bi initialized with the current copy of tagmap
    for (int i = 0; i < size; ++i) tagmap_temp[i] = tagmap[i];

    int dil_xy = ceil( PARA.scal/1.0);
    int dil_z  = ceil((PARA.scal/1.0)/zDist);

    // sphere offsets
    vector<int> dxoffset;
    vector<int> dyoffset;
    vector<int> dzoffset;

    for (int dx = -dil_xy; dx <= dil_xy; ++dx) {
        for (int dy = -dil_xy; dy <= dil_xy; ++dy) {
            for (int dz = -dil_z; dz <= dil_z; ++dz) {
                if (pow(dx,2)/pow(dil_xy,2)+pow(dy,2)/pow(dil_xy,2)+pow(dz,2)/pow(dil_z,2)<=1) {
                    dxoffset.push_back(dx);
                    dyoffset.push_back(dy);
                    dzoffset.push_back(dz);
                }
            }
        }
    }
//    cout << dxoffset.size() << " offsets" << endl;

    // take tagmap copy, go through all of copy's non-zero elements and dilate with -1s, substituting zeros with -1s in original tagmap
    for (long i = 0; i < size; ++i) {

        int x  = i % N;
        int z  = i / (N*M);
        int y  = i/N-z*M;

        if (tagmap_temp[z*N*M+y*N+x]==0) continue; // skip those that are zero

        if (x>=dil_xy && x<N-dil_xy && y>=dil_xy && y<M-dil_xy && z>=dil_z && z<P-dil_z) { // is in image

            for (int k = 0; k < dxoffset.size(); ++k) {
                int curridx = (z+dzoffset[k])*N*M + (y+dyoffset[k])*N + (x+dxoffset[k]);
                if (tagmap[curridx]==0) tagmap[curridx] = -1;
            }

//            for (int xx = x-dil_xy; xx <= x+dil_xy; ++xx) {
//                for (int yy = y-dil_xy; yy <= y+dil_xy; ++yy) {
//                    for (int zz = z-dil_z; zz <= z+dil_z; ++zz) {
//                        if (pow(xx-x,2)/pow(dil_xy,2)+pow(yy-y,2)/pow(dil_xy,2)+pow(zz-z,2)/pow(dil_z,2)<=1) { // is in sphere
//                            if (tagmap[zz*N*M+yy*N+xx] == 0) {
//                                tagmap[zz*N*M+yy*N+xx] = -1; // tag the neighbourhood as foreground
//                            }
//                        }
//                    }
//                }
//            }

        }

    }

    delete tagmap_temp; tagmap_temp = 0;

    cout << "done."<< endl;

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

    cout << setprecision(4) << cnt_fg/1000.0 << "k locations, " << flush;
    cout << setprecision(4) << (cnt_skipped/(float)cnt_fg)*100.0 << "% skipped" << endl;
    cout << setprecision(2) << znccmin << " zncc min, " << flush;
    cout << setprecision(2) << znccmax << " zncc max" << endl;

    // new tagmap after calculating
    long cnt_new_fg = 0;
    for (long ii = 0; ii < size; ++ii) {
        if (tagmap[ii]!=0) cnt_new_fg++;
    }
    cout << setprecision(4) << cnt_new_fg/1000.0 << "k after expanding." << endl;

////////////////////////////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < cnt_fg; ++i) {delete i2xyz[i]; delete i2calcs[i];}
    delete i2xyz; i2xyz = 0;
    delete i2calcs; i2calcs = 0;
    delete xyz2i; xyz2i = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////

    if (PARA.saveMidres) {
        // save int* tagmap as set of .txt images (uint8 is not enough to save the range and saving float image regular way did not work)
        cout << "exporting tagmap (2: gpnts): ";
        for (int zz = 0; zz < P; ++zz) {
            cout << zz << ", "<<flush;
            QString of = PARA.inimg_file + "_tagmap_gpnts_"+QString("%1").arg(zz, 4, 10, QChar('0'))+".txt";
            ofstream myfile;
            myfile.open (of.toStdString().c_str());
            for (int yy = 0; yy < M; ++yy) {
                for (int xx = 0; xx < N; ++xx) {
                    myfile << tagmap[zz*(N*M)+yy*N+xx] << " ";
                }
                myfile << endl;
            }
            myfile.close();
        }
        cout << "done exporting." << endl;
    }

    if (PARA.saveMidres) { // save guidepoint nodes into swc

        NeuronTree gpntswc;

        for (int ni = 1; ni < nodelist.size(); ++ni) { // first element of the nodelist will be 0

            NeuronSWC nn;
            nn.n = nn.nodeinseg_id = ni;
            nn.type = 3;
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

////////////////////////////////////////////////////////////////////////////////////////////////////

    // arrange guidepoint nodes by correlation score (start chasing from the one with highest correlation)
    // std::sort(nodelist.begin(), nodelist.end(), compareNode); // highest correlation first
    // tags have been assigned to the index of the node in the list and changing the position in the list with sorting will deteriorate the tagmap
    // sort indices and trace from the top of the sorted original indices
    // sorted base od corr score

    vector<int> indices(nodelist.size()); // nr nodes before the tracing starts
    for (int i = 0; i < indices.size(); ++i) indices.at(i) = i; // std::iota does the same
    sort(indices.begin(), indices.end(), CompareIndicesByNodeVectorCorrValues(&nodelist));

////////////////////////////////////////////////////////////////////////////////////////////////////


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

////////////////////////////////////////////////////////////////////////////////////////////////////

    // chasing... constrained tracing limited with tagmaps, starts from the highest corr guidepoint and grows the map
    // final output is the list of linked nodes  (save individual nodes)
    cout << "CHASING... " << endl;

    for (int ni = 0; ni < indices.size(); ++ni) {

        // print process
        if (ni!=0 && ni%(indices.size()/10)==0) cout << ni/(indices.size()/10)*10 << "%\t" << flush;

        for (int vsgn = -1; vsgn <= 1; vsgn+=2) { // -1, 1 direction

            int tt_i    = indices.at(ni); // get the nodelist index
            if (tt_i == 0) continue; // node with index 0 is dummy node in nodelist

            float tt_x  = nodelist.at(tt_i).x;
            float tt_y  = nodelist.at(tt_i).y;
            float tt_z  = nodelist.at(tt_i).z;
            float tt_vx = nodelist.at(tt_i).vx;
            float tt_vy = nodelist.at(tt_i).vy;
            float tt_vz = nodelist.at(tt_i).vz;
            float tt_r  = nodelist.at(tt_i).r;

            vector<int> outcome_tags = btrcr.trace(tt_x, tt_y, tt_z,
                        vsgn*tt_vx, vsgn*tt_vy, vsgn*tt_vz, tt_r,
                        data1d, N, M, P,
                        PARA.angSig,
                        tagmap, tt_i, false); // each trace will use updated tagmap // PARA.gcsSig,

//            cout << "\n" << ni << ".th/" << indices.size() << " :\t nodelist idx " << tt_i << "(" << vsgn << "):\t" << flush;
//            for (int var = 0; var < outcome_tags.size(); ++var) {cout << outcome_tags[var] << " " << flush;}
//            cout << endl;

            // there will be at least one member in the vector
            if (outcome_tags[0]>0 || outcome_tags[0]==-2) { // if the trace will be added

//                cout << "add [0 -- " << btrcr.node_cnt <<"] nodes, nodelist has last index " << nodelist.size()-1 <<" "<< endl;

                for (int ti = 0; ti <= btrcr.node_cnt; ++ti) {

                    // add node to the nodelist
                    Node gpntnode(btrcr.xc[ti], btrcr.yc[ti], btrcr.zc[ti], btrcr.rc[ti], Node::AXON); // reminder!! tracer's r will be gcsstd
                    nodelist.push_back(gpntnode); // reminder!! nodelist will have r as gcsstd
                    int nodetag = nodelist.size()-1;

//                    cout << "\nappend index " << nodetag << endl;

                    // add bidirectional linking
                    if (ti==0) {
                        // towards previous
                        nodelist[nodetag].nbr.push_back(tt_i);
                        nodelist[tt_i].nbr.push_back(nodetag);
//                        cout << ti <<" "<< nodetag << "<->" << tt_i << " " << flush;

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
//                            cout << ti <<" "<< nodetag << "<->" << outcome_tags[k1] << " " <<flush;

                            // radius equalization
                            float ravg = 0.5*(nodelist[outcome_tags[k1]].r + nodelist[nodetag].r);
                            nodelist[outcome_tags[k1]].r = ravg;
                            nodelist[nodetag].r          = ravg;

                        }

                    }

                    if (ti>0) { // previous will exist
                        nodelist[nodetag].nbr.push_back(nodetag-1);
                        nodelist[nodetag-1].nbr.push_back(nodetag);
//                        cout << ti <<" "<< nodetag << "<->" << nodetag-1 <<" " << flush;
                    }

                    // fill the tagmap with current nodetag - the same way it was in guidepoint extraction
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
                                if (xfill>=0 && xfill<N && yfill>=0 && yfill<M && zfill>=0 && zfill<P) { // is in image

                                    // overwriting those tags makes sense, if it is 0+ don't overwrite already tagged ones
                                    float x2 = pow(xfill-x_sph,2);
                                    float y2 = pow(yfill-y_sph,2);
                                    float z2 = pow(zfill-z_sph,2);

                                    bool isinelipse = x2/pow(rxy_sph,2)+y2/pow(rxy_sph,2)+z2/pow(rz_sph,2)<=1;

                                    if (isinelipse) {
                                        int currenttag = tagmap[zfill*N*M+yfill*N+xfill];
                                        if (currenttag==-1 || currenttag==-2 || currenttag==0) {
                                            //
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
//        cout << "node " << ni << " : " << nodelist.at(ni).nbr.size() << " nbrs ---> " << flush;
//        for (int i = 0; i < nodelist[ni].nbr.size(); ++i) {cout << nodelist[ni].nbr[i] << " " << flush;}
        sort(nodelist.at(ni).nbr.begin(), nodelist.at(ni).nbr.end());
        nodelist.at(ni).nbr.erase(unique(nodelist.at(ni).nbr.begin(), nodelist.at(ni).nbr.end()), nodelist.at(ni).nbr.end());
//        cout << "\nshort   -- " << nodelist.at(ni).nbr.size() << endl;
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
        // save int* tagmap as set of .txt images (uint8 is not enough to save the range and saving float image regular way did not work)
        cout << "exporting tagmap (3: chases): ";
        for (int zz = 0; zz < P; ++zz) {
            cout << zz << ", "<<flush;
            QString of = PARA.inimg_file + "_tagmap_chases_"+QString("%1").arg(zz, 4, 10, QChar('0'))+".txt";
            ofstream myfile;
            myfile.open (of.toStdString().c_str());
            for (int yy = 0; yy < M; ++yy) {
                for (int xx = 0; xx < N; ++xx) {
                    myfile << tagmap[zz*(N*M)+yy*N+xx] << " ";
                }
                myfile << endl;
            }
            myfile.close();
        }
        cout << "done exporting." << endl;
    }

    delete tagmap; tagmap = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////

    /** traverse through the connected nodes to get the tree out - BFS
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

    // will essentially convert nodelist with all its linking (2 directional connections)into NeuronTree which contains the list of neuron nodes
    // each node (NeuronSWC) will have 1 parent
    // BFS needs Queue data structure that is implemented in class BfsQueue
    // queue accumualtes the linkages between the nodes, where the linkage is described as vector with 2 node indexes vector<int> ~ [curr_node_idx, next_node_idx]
    // since knowing just the node index in the queue is not enough, need to know the index of the mother node to initialize the trace in some direction
    // discovered is array of vector<bool> that will bookkeep labels of the discovered adjacent node pairs, necessary for BFS
    // the reason for keeping two values is that each time we need the mother index and so for each trace element, including the first node of the trace

    // nodelist datastructure:
    // nodelist[0] dummy node
    // nodelist[1] Node instance (tag 1) with connections to other neighbours stored in nbr
    // nodelist[2] Node instance (tag 2)
    // nodelist[3] Node instance (tag 3)
    // nodelist[4] ....

    vector<bool> discovered[nodelist.size()]; // for book-keeping the discovered linkings between nodes (use nodelist to create it)
    for (int ni = 1; ni < nodelist.size(); ++ni) {
        for (int j = 0; j < nodelist[ni].nbr.size(); ++j) {
            discovered[ni].push_back(false);
        }
    }

    NeuronTree nt;
    int max_tree_nodes = -1;
    int seed_idx; // seed index

    while ( (seed_idx = get_undiscovered(indices, discovered))!=-1) { // seeds are undiscovered nodes

        // generate tree for this seed

        // reset and initialize map to tag every node with corresponding swc index
        int nodelist2swc[nodelist.size()]; // so that each index nodelist index can accomodate cooresponding swc reconstruction index
        for (int k = 0; k < nodelist.size(); ++k) nodelist2swc[k] = -1;

        BfsQueue< vector<int> > boob; // queue for the bfs
        vector<int> seed; // list of seed indexeses of nodes that will initialize the tree genesis
        seed.push_back(seed_idx); // 1 seed, can have more (if somas are added)

        NeuronTree ntcurr;
        ntcurr.name =
                "NeuronChaser\n#params:\n#channel="+QString("%1").arg(PARA.channel)+
                "\n#scal="+QString("%1").arg(PARA.scal)+
                "\n#perc="+QString("%1").arg(PARA.perc)+
                "\n#znccTh="+QString("%1").arg(PARA.znccTh)+
                "\n#Ndir="+QString("%1").arg(PARA.Ndir)+
                "\n#angSig="+QString("%1").arg(PARA.angSig)+
                "\n#Ni="+QString("%1").arg(PARA.Ni)+
                "\n#Ns="+QString("%1").arg(PARA.Ns)+
                "\n#zDist="+QString("%1").arg(PARA.zDist)+
                "\n#imgTh="+QString("%1").arg(imgTh)+
                "\n#prune_len="+QString("%1").arg(prune_len);

        for (int i = 0; i < seed.size(); i++) {

            int curr = seed[i];
            int next = INT_MIN;

            NeuronSWC nseed;
            nseed.n = nseed.nodeinseg_id = nodelist2swc[curr] = ntcurr.listNeuron.size()+1;
            nseed.type = Node::NOTHING;
            nseed.x = nodelist[curr].x;
            nseed.y = nodelist[curr].y;
            nseed.z = nodelist[curr].z;
            nseed.r = btrcr.gcsstd2rad * nodelist[curr].r;
            nseed.parent = -1;
            ntcurr.listNeuron.append(nseed);

            // add the neighbors to the queue and label them as discovered
            for (int j = 0; j <nodelist[curr].nbr.size(); j++) {

                next = nodelist[curr].nbr[j];

                // enqueue(), add to FIFO structure, http://en.wikipedia.org/wiki/Queue_%28abstract_data_type%29
                vector<int> lnk(2);
                lnk[0] = curr;
                lnk[1] = next;
                boob.enqueue(lnk);

                discovered[curr][j] = true;
                int pos = find(nodelist[next].nbr.begin(), nodelist[next].nbr.end(), curr) - nodelist[next].nbr.begin();
                discovered[next][pos] = true;

            }
        }

        while (boob.hasItems()) {

            // dequeue(), take from FIFO structure, http://en.wikipedia.org/wiki/Queue_%28abstract_data_type%29
            vector<int> takeit = boob.dequeue();
            int prev = takeit[0];  // next neighbour at the time it was added to the queue becomes current
            int curr = takeit[1];

            // auxilliary values (store branch indexes and branch length) used for removing the tail branches
            vector<int> bchidxs; bchidxs.clear();
            int bchlen = 0;

            bool nadd = nodelist2swc[curr]==-1;

            if (nadd) {

                bchlen++;
                bchidxs.push_back(curr);

                NeuronSWC n0;
                n0.n = n0.nodeinseg_id = nodelist2swc[curr] = ntcurr.listNeuron.size()+1;
                n0.type = Node::AXON;
                n0.x = nodelist[curr].x;
                n0.y = nodelist[curr].y;
                n0.z = nodelist[curr].z;
                n0.r = btrcr.gcsstd2rad * nodelist[curr].r;
                n0.parent = nodelist2swc[prev];
                ntcurr.listNeuron.append(n0);
            }

            int count_undiscovered = 0;

            while (((count_undiscovered = count_vals(discovered[curr], false)) == 1) && nadd) {

                prev = curr;
                int pos = find(discovered[curr].begin(), discovered[curr].end(), false) - discovered[curr].begin();
                curr = nodelist[curr].nbr[pos];

                // curr->prev
                pos = find(nodelist[curr].nbr.begin(), nodelist[curr].nbr.end(), prev) - nodelist[curr].nbr.begin();
                discovered[curr][pos] = true;
                // prev->curr
                pos = find(nodelist[prev].nbr.begin(), nodelist[prev].nbr.end(), curr) - nodelist[prev].nbr.begin();
                discovered[prev][pos] = true;

                nadd = nodelist2swc[curr]==-1;

                if (nadd) {

                    bchlen++;
                    bchidxs.push_back(curr);

                    NeuronSWC n1;
                    n1.n = n1.nodeinseg_id = nodelist2swc[curr] = ntcurr.listNeuron.size()+1;
                    n1.type = Node::AXON;
                    n1.x = nodelist[curr].x;
                    n1.y = nodelist[curr].y;
                    n1.z = nodelist[curr].z;
                    n1.r = btrcr.gcsstd2rad * nodelist[curr].r;
                    n1.parent = nodelist2swc[prev];
                    ntcurr.listNeuron.append(n1);

                }

            }

            int count_new_branches = 0;
            for (int i = 0; i < discovered[curr].size(); i++) {
                if (!discovered[curr][i]) { // if it was not discovered

                    count_new_branches++;

                    int next = nodelist[curr].nbr[i]; // take the index of the node that was not discovered

                    vector<int> lnk(2);
                    lnk[0] = curr;
                    lnk[1] = next;
                    boob.enqueue(lnk);

                    discovered[curr][i] = true;
                    int pos = find(nodelist[next].nbr.begin(), nodelist[next].nbr.end(), curr) - nodelist[next].nbr.begin();
                    discovered[next][pos] = true;
                }
            }

//            if (count_new_branches!=count_undiscovered) cout << "WRONG"<<endl;

            // pruning : cancel those that were up to prune_len
            if ((count_undiscovered==0 || !nadd) && bchlen<=prune_len) {

                // when nadd=0 bchlen is always 0 so it's ok to place but maybe redundant as it does not affect the reduction
                // also bchlen is redundant, bchidxs is enough to have
                // remove the node from neurontree list and remove it's corresponding image map index
                for (int ii = 1; ii <= bchlen; ++ii) {
//                    cout << "PRUNE: bchlen=" << bchlen <<"\t"<<flush;
//                    cout << "undiscovered"<< count_undiscovered << "\t" << flush;
//                    cout << "NADD =" << nadd << "\t" << flush;
//                    cout << ii <<"# remove, ntree size="<< ntcurr.listNeuron.size() <<" , nodelist2swc[" << bchidxs.back() << "] = -1"<<flush;
                    nodelist2swc[bchidxs.back()] = -1; // reset what was set while traversing this branch
                    bchidxs.pop_back();
                    ntcurr.listNeuron.removeLast();
//                    cout<<" after = "<<ntcurr.listNeuron.size()<<endl;

                }
            }

        }

        cout << "done. " <<  ntcurr.listNeuron.size() << " nodes found." << endl;

        if (ntcurr.listNeuron.size()>max_tree_nodes) {
            max_tree_nodes = ntcurr.listNeuron.size();
            nt = ntcurr; // keep the one with highest number of nodes in nt
        }
//        else {
            // save ntcurr just in case for completeness
            if (false) {
                QString ntcurrswc_name = PARA.inimg_file + "_NeuronChaser_"+QString("%1").arg(ntcurr.listNeuron.size(), 4, 10, QChar('0'))+".swc";
                cout << ntcurrswc_name.toStdString().c_str() << " to save " << endl;
                writeSWC_file(ntcurrswc_name.toStdString().c_str(), ntcurr);
            }
//        }

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
