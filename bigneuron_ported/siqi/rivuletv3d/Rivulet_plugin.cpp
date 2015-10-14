/* Rivulet_plugin.cpp
 * Rivulet algorithm for 3D neuron tracing. 
 * 2015-8-25 : by Siqi Liu, Donghao Zhang
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include <cmath>
#include <iostream>
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"
#include "stackutil.h"
#include "fastmarching_dt.h"
#include "Rivulet_plugin.h"
#include "utils/msfm.h"
#include <iterator>
#include "structs.h"
#include "rk4.h"
#include "marker_radius.h"
#include <ctime>
#include "vn_rivulet.h"
Q_EXPORT_PLUGIN2(Rivulet, RivuletPlugin);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    unsigned char threshold;
    unsigned char stepsize;
    unsigned char gap;
    int dumpbranch;
    float connectrate;
    float percentage;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
float * get_boundary_distance(unsigned char*);
template <class T> unsigned char * charimg(T *speedimg, int NVOX);
V3DLONG sub2ind(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG* in_sz);
Point ind2sub(V3DLONG idx, V3DLONG* in_sz);
template <class T> V3DLONG findmax(T * I, int NVOX, T * max);
double* float2double(float *f, V3DLONG NVOX);
double* distgradient(double* T, V3DLONG* in_sz);
Path shortestpath2(double * T, double *grad, unsigned char *data1d, V3DLONG* in_sz, Point startpoint, Point sourcepoint, unsigned char stepsize, unsigned char gap);
double* c2mat(double* m, V3DLONG* in_sz);
void binarysphere3d(bool* tB, V3DLONG* in_sz, vector<Point> l, vector<float> radius);
float addbranch2tree(vector<swcnode>* tree, Path l, unsigned char connectrate, vector<float> radius, unsigned char *data1d, V3DLONG* in_sz);
void celebrate();
 
QStringList RivuletPlugin::menulist() const
{
	return QStringList() 
		<<tr("tracing")
		<<tr("about");
}

QStringList RivuletPlugin::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void RivuletPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing"))
	{
        bool bmenu = true;
        input_PARA PARA;

        if(callback.getImageWindowList().empty())
        {
            v3d_msg("Oops... No image opened in V3D...");
            return;
        }
        PARA_RIVULET p;
        // fetch parameters from dialog
        if (!p.rivulet_dialog())
            return;
        PARA.threshold = p.threshold;
        PARA.connectrate = p.connectrate;
        PARA.percentage = p.percentage;
        PARA.dumpbranch = p.dumpbranch;
        PARA.gap = p.gap;
        PARA.stepsize = p.stepsize;
        PARA.channel = p.channel;

        reconstruction_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("Rivulet algorithm for 3D neuron tracing. . "
			"Developed by Siqi Liu, Donghao Zhang, 2015-8-25"));
	}
}

bool RivuletPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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
        PARA.threshold = (paras.size() >= k+1) ? atoi(paras[k]) : 10;  k++;
        PARA.stepsize = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        PARA.gap = (paras.size() >= k+1) ? atoi(paras[k]) : 15;  k++;
        PARA.dumpbranch = (paras.size() >= k+1) ? atoi(paras[k]) : 0;  k++;
        PARA.connectrate = (paras.size() >= k+1) ? atof(paras[k]) : 1.5;  k++;
        PARA.percentage = (paras.size() >= k+1) ? atof(paras[k]) : 0.98;  k++;
        if (PARA.percentage > 1.0)
            PARA.percentage = 1.0;
        reconstruction_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {
		printf("**** Usage of Rivulet tracing **** \n");
		printf("vaa3d -x Rivulet -f tracing_func -i <inimg_file> -p <channel> <threshold> <stepsize> <gap> <dumpbranch> <connectrate> <percentage>\n");
        printf("inimg_file\tThe input image\n");
        printf("channel\tData channel for tracing. Start from 1 (default 1).\n");
        printf("threshold\tThe background threshold for segmentation (default 10).\n");
        printf("stepsize\tThe step size of gradient descent tracing (default 1).\n");
        printf("gap\tThe gap allowed for steps on background voxels. Useful for noisy images (default 15)\n");
        printf("dumpbranch\tWhether the unconnected branches will be dumpped. (default 0)\n");
        printf("connectrate\tBranch terminis within connectrate*(radius+3) will be connected to the trunk. (default 1.5)\n");
        printf("percentage\tThe converage expected on the binary image before stopping tracing. (default 0.98)\n");
        printf("outswc_file\tWill be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    // QMessageBox msgBox;
    // if (bmenu)
    // {
    //     msgBox.setText("Rivulet Running...");
    //     msgBox.setIcon(QMessageBox::Information);
    //     msgBox.setStandardButtons(QMessageBox::Ok);
    //     // msgBox.setAutoClose(true);
    //     // msgBox.setTimeout(3); //Closes after three seconds
    //     msgBox.exec();
    // }

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
    cout<<"Start Rivulet"<<endl;
    printf("Image Size: %d, %d, %d\n", in_sz[0], in_sz[1], in_sz[2]);

    // Binarize Image with theshold
    const unsigned char threshold = PARA.threshold;
    const unsigned char stepsize = PARA.stepsize;
    const unsigned char gap = PARA.gap; 
    const unsigned char dumpbranch = PARA.dumpbranch; 
    const unsigned char connectrate = PARA.connectrate; 
    const double percentage = PARA.percentage;
    cout<<"Tracing Parameters:"<<endl;
    printf("threshold:\t%d", threshold);
    printf("stepsize:\t%d", stepsize);
    printf("gap:\t%d", gap);
    printf("dumpbranch:\t%d", dumpbranch);
    printf("connectrate:\t%d", connectrate);
    printf("percentage:\t%f", percentage);

    int NVOX = M * N * P;
    
    // Distance Transform
    float * bdist1d = 0;
    cout<<"DT..."<<endl;
    unsigned char* binary_data1d;

    try
    {
        binary_data1d = new unsigned char[NVOX]; // Make a copy of the original data for radius estimation
    }
    catch(...)  {v3d_msg("Cannot allocate memory for B."); return;}

    std::copy(data1d, data1d + NVOX, binary_data1d);

    // Fast marching distance transform proposed in APP2 
    fastmarching_dt(data1d, bdist1d, N, M, P, 2, threshold);

    // binarize data
    for (int i=0; i<NVOX; i++)
    {
        binary_data1d[i] = binary_data1d[i] > threshold? 1 : 0;
    }

    // Find the source point
    float maxd;
    V3DLONG maxidx = findmax(bdist1d, NVOX, &maxd);
    Point sourcepoint = ind2sub(maxidx, in_sz);

    // Make Speed Image
    for (int i = 0; i < NVOX - 1; i++)
    {   
        float s = pow(bdist1d[i] / maxd, 4);
        bdist1d[i] = s == 0 ? 1e-10 : s;
    }

    // Marching on the Speed Image
    int sp[3];
    sp[0] = (int)sourcepoint.x; sp[1] = (int)sourcepoint.y; sp[2] = (int)sourcepoint.z;
    double* bdist1d_double = float2double(bdist1d, NVOX);
    // printf("Source Point -- x: %d, y: %d, z: %d\n", (int)sourcepoint.x, (int)sourcepoint.y, (int)sourcepoint.z);
    double* oT = msfm(bdist1d_double, in_sz, sp, false, false, false); // Original Timemap

    if (bdist1d_double) {delete [] bdist1d_double; bdist1d_double = 0;}

    double* T = new double[NVOX]; // Timemap for tracing which is going to be erased
    copy(oT, oT + NVOX, T);
    vector<swcnode> tree;
    bool prune = true;

    // Calculate the gradient of the Distance map
    cout<<"Calculating Gradient"<<endl;
    double* grad = distgradient(T, in_sz);
    // cout<<"Gradient Calculated"<<endl;
    bool * B;
    try {B = new bool[NVOX];}
    catch(...)  {v3d_msg("Cannot allocate memory for B."); return;}
    for (int i=0; i<NVOX; i++)
    {
        B[i] = false;
    }

    bool * tB;
    try {tB = new bool[NVOX];}
    catch(...)  {v3d_msg("Cannot allocate memory for tB."); return;}
    for (int i=0; i<NVOX; i++)
    {
        tB[i] = false;
    }
    vector<float> lconfidence;

    // Mask T with 0 where 0 in original image
    for (int i=0; i<NVOX; i++)
    {
        T[i] = binary_data1d[i] ==0 ? 0 : T[i];
    }

    // Start Tracing
    cout<<"Start Tracing"<<endl;
    cout<<"        '.,\n          'b      *\n           '$    #.\n            $:   #:\n            *#  @):\n            :@,@):   ,.**:'\n  ,         :@@*: ..**'\n   '#o.    .:(@'.@*\"'\n      'bq,..:,@@*'   ,*\n      ,p$q8,:@)'  .p*'\n     '    '@@Pp@@*'\n           Y7'.'\n          :@):.\n         .:@:'.\n       .::(@:.  \n"<<endl;
    MyMarker marker; // Only for radius estimation
    std::clock_t start = std::clock();
    while(true)
    {
        cout<<".";
        // cout<<"N Branches:"<<tree.size()<<endl;
        double maxt;
        int maxidx = findmax(T, NVOX, &maxt);
        Point startpoint = ind2sub((V3DLONG)maxidx, in_sz);
        // Trace the shortest path from the farthest point to the source point
        Path l = shortestpath2(T, grad, binary_data1d, in_sz, startpoint, sourcepoint, stepsize, gap);
        // cout<<"after shortestpath2"<<endl;
        int pathlen = l.l.size();
        
        //Get radius of each point from distance transform
        vector<float> radius(pathlen);
        for (int i=0; i< pathlen; i++)
        {
            V3DLONG idx = sub2ind(l.l[i].x, l.l[i].y, l.l[i].z, in_sz);
            // radius[i] = bdist1d[idx] < 1.0 ? 1.0 : bdist1d[idx];
            marker.x = l.l[i].x; marker.y = l.l[i].y; marker.z = l.l[i].z;
            radius[i] = markerRadius(data1d, in_sz, marker, threshold, 2); 
        }

        // Remove traced from the time map
        binarysphere3d(tB, in_sz, l.l, radius);
        tB[maxidx] = 1;
        V3DLONG bsum = 0;
        V3DLONG vsum = 0;

        for (int i=0; i<NVOX; i++)
        {
            T[i] = tB[i] == true ? -1 : T[i];
            B[i] = tB[i] || B[i];
            bsum += B[i] && binary_data1d[i] != 0? 1 : 0;
            vsum += binary_data1d[i];
        }

        // Add l to tree
        if ( !(l.dump && dumpbranch) )
        {
            float conf = addbranch2tree(&tree, l, connectrate, radius, binary_data1d, in_sz);
            lconfidence.push_back(conf);
        }
        else
        {
            // cout<<"Branch dumped"<<endl;
        }

        // Compute the coverage percent
        double percent = (double)bsum / (double)vsum;
        // cout<<"Percent:"<<percent* 100<<endl;
        if (percent >= percentage)
        {
            break;
        }
        
    }

    double duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
    cout<<"Tracing took "<<duration<<" seconds"<<endl;

    // Free up memory
    if (bdist1d) {delete [] bdist1d; bdist1d = 0;}
    if (oT) {delete [] oT; oT = 0;}
    if (T) {delete [] T; T = 0;}
    if (grad) {delete [] grad; grad = 0;}
    if (B) {delete [] B; B = 0;}
    if (binary_data1d) {delete [] binary_data1d; binary_data1d = 0;}

    //Output
    NeuronTree nt;
    QList <NeuronSWC> listNeuron;
    QHash <int, int> hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    NeuronSWC S;
    for (int i = 0; i < tree.size(); i++)
    {
        S.n = tree[i].id;
        S.type = tree[i].type;
        S.x = tree[i].p.x;
        S.y = tree[i].p.y;
        S.z = tree[i].p.z;
        S.r = tree[i].radius;
        S.pn = tree[i].parent;
        listNeuron.append(S);
        hashNeuron.insert(S.n, listNeuron.size() - 1);       
    }
    nt.n = -1;
    nt.on = true;
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;
	QString swc_name = PARA.inimg_file + "_Rivulet.swc";
	nt.name = "Rivulet";
    writeSWC_file(swc_name.toStdString().c_str(),nt);
    celebrate();
    cout<<"δὶς ἐς τὸν αὐτὸν ποταμὸν οὐκ ἂν ἐμβαίης. -- Ἡράκλειτος"<<endl;
    // if (bmenu)
    // {
    //     msgBox.done(0);
    // }

    v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    // v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

    return;
}


template <class T> unsigned char * charimg(T *I, int NVOX)
{
    unsigned char * data1d_uchar;
    try {data1d_uchar = new unsigned char [NVOX];}
    catch(...)  {v3d_msg("cannot allocate memory for data1d_uchar."); return NULL;}

    // Find max
    T max = 0;
    findmax(I, NVOX, &max);

    // Rescale to 0-255
    for (int i = 0; i < NVOX; i++)
    {
       data1d_uchar[i] = (unsigned char)((I[i] / max) * 255);
    }
    
    return data1d_uchar;
}


template <class T> V3DLONG findmax(T * I, int NVOX, T * max)
{ 
    *max = 0;
    V3DLONG maxidx = 0;
    for (int i = 0; i < NVOX; i++)
    {
       if (*max < I[i])
       {
            *max = I[i];
            maxidx = i;
       }
    }
    return maxidx;
}


Point ind2sub(V3DLONG idx, V3DLONG* in_sz)
{
    Point p;
    float xysz = (float)in_sz[0] * (float)in_sz[1];
    p.z = (float)(floor((float)idx / (float)xysz));
    p.y = idx - p.z * xysz;
    p.y = (float)floor((float)p.y / (float)in_sz[0]);
    p.x = (float)(floor(idx - p.z * xysz - p.y * (float)in_sz[0]));
    return p;
}


V3DLONG sub2ind(V3DLONG x, V3DLONG y, V3DLONG z, V3DLONG* in_sz)
{
    V3DLONG xysz = (V3DLONG)(in_sz[0] * in_sz[1]);
    return (V3DLONG)z * xysz + y * in_sz[0] + x;
}


double* float2double(float *f, V3DLONG NVOX)
{
    double* d = 0;
    try {d = new double [NVOX];}
    catch(...)  {v3d_msg("cannot allocate memory for double image."); return NULL;}

    for (int i=0; i<NVOX; i++)
    {
        d[i] = (double) f[i];
    }
    return d;
}


// Change 1d volume storage to column wise (originally row wise)
double* c2mat(double* m, V3DLONG* in_sz)
{
    V3DLONG NVOX = (V3DLONG)((float)in_sz[0] * (float)in_sz[1] * (float)in_sz[2]);
    V3DLONG newsz[3];
    newsz[0] = in_sz[1]; newsz[1] = in_sz[0]; newsz[2] = in_sz[2];
    double * newmat = new double[NVOX];
    for (int x = 0; x < in_sz[0]; x++)
        for (int y = 0; y < in_sz[1]; y++)
            for (int z = 0; z < in_sz[2]; z++)
            {
                int newloc = sub2ind(y, x, z, in_sz);
                int oldloc = sub2ind(x, y, z, newsz);
                newmat[newloc] = m[oldloc];
            }
    return newmat;
}


double* distgradient(double* T, V3DLONG* in_sz)
{
    V3DLONG NVOX = in_sz[0] * in_sz[1] * in_sz[2];
    V3DLONG jin_sz[3]; // matrix for jacobian
    jin_sz[0] = in_sz[0] + 2; jin_sz[1] = in_sz[1] + 2; jin_sz[2] = in_sz[2] + 2;
    int jsz = jin_sz[0] * jin_sz[1] * in_sz[2];
    double* Fx = new double[NVOX];
    std::fill(Fx, Fx+NVOX, 0.0);
    double* Fy = new double[NVOX];
    std::fill(Fy, Fy+NVOX, 0.0);
    double* Fz = new double[NVOX];
    std::fill(Fz, Fz+NVOX, 0.0);
    double* J;
    try{
        J = new double[jsz];
    }
    catch(...)  {v3d_msg("Cannot allocate memory for J."); return NULL;}

    double maxt = 0; 
    findmax(T, NVOX, &maxt);
    std::fill(J, J+jsz, maxt);

    // Assign the center of J to T
    for (int x=0; x<in_sz[0]; x++)
        for (int y=0; y<in_sz[1]; y++)
            for (int z=0; z<in_sz[2]; z++)
            {
                int jloc = sub2ind(x+1, y+1, z+1, jin_sz);
                int tloc = sub2ind(x, y, z, in_sz);
                J[jloc] = T[tloc];
            }

    // Make the neighbour position kernel
    short *Ne[26];
    int ctr = 0;
    for (int i = -1; i <= 1 ; i++)
        for (int j = -1; j <= 1 ; j++)
            for (int k = -1; k <= 1 ; k++)
            {
                Ne[ctr] = new short[3];
                Ne[ctr][0] = i;
                Ne[ctr][1] = j;
                Ne[ctr][2] = k;
                // printf("Ne %d, %d, %d\n", i, j, k);
                ctr++;
            }
    
    for (int i=0; i<26; i++)
        for (int x=0; x<in_sz[0]; x++)
            for (int y=0; y<in_sz[1]; y++)
                for (int z=0; z<in_sz[2]; z++)
                {
                    int jloc = sub2ind(1 + x + Ne[i][0], 1 + y + Ne[i][1], 1 + z + Ne[i][2], jin_sz);
                    int tloc = sub2ind(x, y, z, in_sz);
                    double in = J[jloc];
                    if (in < T[tloc])
                    {
                        T[tloc] = in;
                        float powsum = Ne[i][0] * Ne[i][0] + Ne[i][1] * Ne[i][1] + Ne[i][2] * Ne[i][2];
                        Fx[tloc] = -Ne[i][0] / sqrt(powsum);
                        Fy[tloc] = -Ne[i][1] / sqrt(powsum);
                        Fz[tloc] = -Ne[i][2] / sqrt(powsum);
                    }
                }

    // Change to grad to 1d
    double *grad4d = new double[NVOX*3];
    std::copy(Fx, Fx+NVOX, grad4d);
    std::copy(Fy, Fy+NVOX, grad4d + NVOX);
    std::copy(Fz, Fz+NVOX, grad4d + NVOX + NVOX);
    if (Fx) {delete [] Fx; Fx = 0;}
    if (Fy) {delete [] Fy; Fy = 0;}
    if (Fz) {delete [] Fz; Fz = 0;}
    if (J) {delete [] J; J = 0;}

    return grad4d;
}


float pointdist(Point p1, Point p2)
{
    float d = sqrt(pow((p1.x - p2.x), 2) + 
                          pow((p1.y - p2.y), 2) + 
                          pow((p1.z - p2.z), 2));
    return d;
}


Path shortestpath2(double * T, double *grad, unsigned char *data1d, V3DLONG* in_sz, Point startpoint, Point sourcepoint, unsigned char stepsize, unsigned char gap)
{
    Path path2return;
    int i = 0; // Count movement
    int j = 0; // Count empty steps
    int ifree = 10000; // Reserve memory for the shortest line array
    vector<Point> shortestline(ifree);
    bool merged = false;
    path2return.l = shortestline;

    // Iteratively trace the shortest line array
    while(true)
    {
        V3DLONG startidx = sub2ind((V3DLONG)floor(startpoint.x), (V3DLONG)floor(startpoint.y), (V3DLONG)floor(startpoint.z), in_sz);

        double dist = T[startidx];

        if (dist == -1)
        {
            path2return.merged = true;
            break;
        }

        // Calculate the next point using runge kutta
        Point endpoint = rk4(startpoint, grad, in_sz, stepsize);
        if (endpoint.x < 0 || endpoint.x > in_sz[0] ||
            endpoint.y < 0 || endpoint.y > in_sz[1] ||
            endpoint.z < 0 || endpoint.z > in_sz[2]) break;

        // Calculate the distance to the end point
        float distance2end = pointdist(endpoint, sourcepoint);

        if (distance2end < 2.0 * (float)stepsize || distance2end < 1.0)
        {
            shortestline[i] = sourcepoint;
            // printf("DEBUG: == Reached sourcepoint at %f, %f, %f\n", sourcepoint.x, sourcepoint.y, sourcepoint.z);
            break;
        }

        if (j == gap) 
        {
            path2return.dump = true;
            break;
        }
          
        double movement = 0;
        if (i >= 15)
        {
            Point pback = shortestline[i - 15];
            movement = pointdist(pback, endpoint);
        }
        else
        {
            movement = stepsize + 1;
        }

        int endidx = sub2ind((V3DLONG)floor(endpoint.x), (V3DLONG)floor(endpoint.y), (V3DLONG)floor(endpoint.z), in_sz);
        if (data1d[endidx] == 0)
        {
            j++; 
        }
        else
        {
            j=0;
        }

        if (movement < stepsize) 
        {
            break;
        }

        
        startpoint = endpoint;
        if (startpoint.x != startpoint.x || startpoint.y != startpoint.y || startpoint.z != startpoint.z) // Check NaN
        {
            break;
        }

        shortestline[i] = endpoint;
        // printf("Trace-Point: %d - %f, %f, %f\n", i, endpoint.x, endpoint.y, endpoint.z); 
        i++; // Count number of iterations
    }

    shortestline.erase(shortestline.begin() + i, shortestline.end());
    path2return.l = shortestline;
    return path2return;
}


float constrain(float x, float low, float high)
{
    if (x < low)
        return low;
    if (x > high)
        return high;
    return x;
}


vector<Point> neighbours3d(float x, float y, float z, float radius)
{
    vector<Point> neighbours;
    // Return the coordinates of neighbours within a radius
    for (float xgv = x - radius; xgv <= x + radius; xgv++)
        for (float ygv = y - radius; ygv <= y + radius; ygv++)
            for (float zgv = z - radius; zgv <= z + radius; zgv++)
            {
                Point p; p.x = xgv; p.y = ygv; p.z = zgv;
                neighbours.push_back(p);
            }
    return neighbours;
}


void binarysphere3d(bool* tB, V3DLONG* in_sz, vector<Point> l, vector<float> radius)
{
    for (int i=0; i<l.size(); i++)
    {
        // printf("binarysphere- Point -%d- x:%f, y:%f, z:%f, radius: %f\n", i, l[i].x, l[i].y, l[i].z, radius[i]);
        vector<Point> neighbours = neighbours3d(l[i].x, l[i].y, l[i].z, radius[i]);
        for (int n=0; n<neighbours.size(); n++)
        {
            float x = constrain(neighbours[n].x, 0.0, in_sz[0]-1);
            float y = constrain(neighbours[n].y, 0.0, in_sz[1]-1);
            float z = constrain(neighbours[n].z, 0.0, in_sz[2]-1);
            // printf("neigh x:%f.2, y:%f.2, z:%f.2\n", x, y, z);
            V3DLONG ind = sub2ind((V3DLONG)x, (V3DLONG)y, (V3DLONG)z, in_sz);
            tB[ind] = true;
        }
    }
}


V3DLONG findmindist(Point p, vector<swcnode> tree, double* m)
{
    *m = 1e10;
    V3DLONG idx = -1;
    for (int i=0; i<tree.size(); i++)
    {
        double dist = sqrt(pow(p.x - tree[i].p.x, 2) + pow(p.y - tree[i].p.y, 2) + pow(p.z - tree[i].p.z, 2));
        if (*m > dist)
        {
            *m = dist; 
            idx = i;
        }
    }

    return idx;
}


float addbranch2tree(vector<swcnode>* tree, Path l, unsigned char connectrate, vector<float> radius, unsigned char *data1d, V3DLONG* in_sz)
{
    float confidence = 0;
    int pathlen = l.l.size();
    if (pathlen < 4)
    {
        // cout<<"Dump a branch short length:"<<pathlen<<endl;
        return confidence;
    }
    int NVOX = (int)((float)in_sz[0] * (float)in_sz[1] * (float)in_sz[2]);
    int vsum = 0;

    // Compute the confidence
    for (int i=0; i<pathlen; i++)
    {
        Point p = l.l[i];
        V3DLONG ind = sub2ind((V3DLONG)floor(p.x), (V3DLONG)floor(p.y), 
                              (V3DLONG)floor(p.z), in_sz);
        if (data1d[ind] > 0) 
        {
            vsum++;
        }
    }

    confidence = (float)vsum / (float)pathlen; 
    if(confidence < 0.5)
    {
        // cout<<"Dump with confidence: "<<confidence<<" length: "<<l.l.size()<<endl;
        return confidence;
    }

    assert(pathlen == radius.size());

    if (tree->size() == 0) 
    {
        for (int i=0; i<pathlen; i++)
        {
            swcnode node;
            node.id = i+1;
            node.type = 2;
            node.p.x = l.l[i].x;
            node.p.y = l.l[i].y;
            node.p.z = l.l[i].z;
            node.radius = (int)radius[i];
            node.parent = i+2;
            (*tree).push_back( node );
        }
        (*tree).back().parent = -1; // Root node
    }
    else
    {
        vector<swcnode> newtree(pathlen);
        Point termini1 = l.l[pathlen - 1];
        Point termini2 = l.l[0];
        // Get pairwise distance between the termini and tree nodes 
        double mind1, mind2;
        V3DLONG idx1 = findmindist(termini1, *tree, &mind1); 
        V3DLONG idx2 = findmindist(termini2, *tree, &mind2); 

        // Sort internal relationship
        for (int i=0; i<pathlen; i++)
        {
            newtree[i].id = tree->back().id + i + 1;
            newtree[i].type = 2;
            newtree[i].p.x = l.l[i].x;
            newtree[i].p.y = l.l[i].y;
            newtree[i].p.z = l.l[i].z;
            newtree[i].radius = radius[i];
            if (i != pathlen - 1)
                newtree[i].parent = tree->back().id + i + 2;
            else
                newtree[i].parent = -2;

        }

        if ( mind1 < ((*tree)[idx1].radius + 3) * connectrate || 
             mind1 < (newtree.back().radius + 3) * connectrate && l.merged)
        {
            newtree.back().parent = (*tree)[idx1].id; // Connect to the tree parent
        }
        else
        {
            newtree.back().parent = -2; // Remain unconnected!!
        }

        if ( mind2 < ((*tree)[idx2].radius + 3) * connectrate || 
             mind2 < (newtree.back().radius + 3) * connectrate && l.merged)
        {
            // newtree[0].parent = (*tree)[idx2].id; // Connect to the tree parent
            (*tree)[idx2].parent = newtree[0].id; // Connect to the tree parent
        }
        else
        {
            newtree[0].parent = -2; // Remain unconnected!!
        }

        vector<swcnode> backuptree = *tree;
        tree->clear();
        tree->reserve(backuptree.size() + newtree.size()); // Enlarge the tree vector to hold the new branch
        tree->insert(tree->end(), backuptree.begin(), backuptree.end());
        tree->insert(tree->end(), newtree.begin(), newtree.end());

    }

    return confidence;
}

void celebrate()
{
    printf("　　　　　　o　　　o\n　　　　o　＼　　o　　／　o　　　　Boom！\n　　　o　　 o　　 l　　　o　　 o\n　　　　　o-　　 ○　　-o\n　　　o　　 o　　 l　　　o　　 o　　　　　　　　Boom！\n　　　　o　　　/ o　　　　o\n　　　　　／ o　　　o　＼　　　　Boom！\n\n　　　　　　　　 （\n　　　　　　　　　）\n　　　　　　　　 （\n　　　　　　　　 （\n　　　　　　　　　）\n　　　　　　　　 （\n");
}