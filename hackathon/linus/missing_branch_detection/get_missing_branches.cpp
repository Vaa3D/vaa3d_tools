
#include "get_missing_branches.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include "qlist.h"
#include "openSWCDialog.h"
#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"
#include "../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define PI 3.14159265359
using namespace std;

double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2)
{
    double xx = s1.x-s2.x;
    double yy = s1.y-s2.y;
    double zz = s1.z-s2.z;
    return (xx*xx+yy*yy+zz*zz);
};

//float median(float arr[], int n){
//    //sort the array
//    sort(arr, arr + n);
//    if(n % 2 == 0)
//        return (arr[n/2 - 1] + arr[n/2])/2;
//    return arr[n/2];
//}

double median(vector<unsigned int> scores)
{
  size_t size = scores.size();

  if (size == 0)
  {
    return 0;  // Undefined, really.
  }
  else
  {
    sort(scores.begin(), scores.end());
    if (size % 2 == 0)
    {
      return (scores[size / 2 - 1] + scores[size / 2]) / 2;
    }
    else
    {
      return scores[size / 2];
    }
  }
}

double median(vector<double> scores)
{
  size_t size = scores.size();

  if (size == 0)
  {
    return 0;  // Undefined, really.
  }
  else
  {
    sort(scores.begin(), scores.end());
    if (size % 2 == 0)
    {
      return (scores[size / 2 - 1] + scores[size / 2]) / 2;
    }
    else
    {
      return scores[size / 2];
    }
  }
}

double StandardDeviation(vector<unsigned int> samples)
{
     int size = samples.size();

     double variance = 0;
     double t = samples[0];
     for (int i = 1; i < size; i++)
     {
          t += samples[i];
          double diff = ((i + 1) * samples[i]) - t;
          variance += (diff * diff) / ((i + 1.0) *i);
     }

     return sqrt(variance / (size - 1));
}

//LandmarkList get_missing_branches_menu(V3DPluginCallback2 &callback, QWidget *parent, Image4DSimple * p4DImage, QList<NeuronSWC> neuron)
LandmarkList get_missing_branches(V3DPluginCallback2 &callback, QWidget *parent, unsigned char * & data1d_crop, V3DLONG mysz[4], ImagePixelType datatype, QList<NeuronSWC> neuron, QString IntTreeName)
{
    LandmarkList candidates_m;
    LandmarkList candidates_mf;

    // Get Neuron
    /*OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return candidates_m;
    NeuronTree nt = openDlg->nt;
    QList<NeuronSWC> neuron = nt.listNeuron;
    */

    // Get Image Data
//    int nChannel = p4DImage->getCDim();

//    V3DLONG mysz[4];
//    mysz[0] = p4DImage->getXDim();
//    mysz[1] = p4DImage->getYDim();
//    mysz[2] = p4DImage->getZDim();
//    mysz[3] = nChannel;
//    cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
//    unsigned char *data1d_crop=p4DImage->getRawDataAtChannel(nChannel);
//    printf("+++++++++++:%p\n",p4DImage);

    vector<long> ids;
    vector<long> parents;
    // Reorder tree ids so that neuron.at(i).n=i+1
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        ids.push_back(neuron.at(i).n);
    }
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        neuron[i].n=i+1;
        if(neuron.at(i).pn !=-1)
        {
            neuron[i].pn=find(ids.begin(), ids.end(),neuron.at(i).pn) - ids.begin()+1;
            parents.push_back(neuron.at(i).pn);
        }
    }

    // Get foreground points in a shell around SWC
    float meanint = 0;
    vector<unsigned int> medianint;
//    V3DLONG nzpixels = 0;
//    float maxint = 0;
    float maxintim = 0;
    vector <struct XYZ> candidates;
    vector <struct XYZ> locswc;
    vector <double> radii;
    vector <unsigned int> treeint;
//    vector <int> nrnint;
    for(V3DLONG id=0; id<neuron.size(); id++)
    {
        locswc.push_back(XYZ(neuron.at(id).x,neuron.at(id).y,neuron.at(id).z));
        radii.push_back(neuron.at(id).radius);
        V3DLONG nodex = neuron.at(id).x;
        V3DLONG nodey = neuron.at(id).y;
        V3DLONG nodez = neuron.at(id).z;
        struct XYZ treep = XYZ(nodex,nodey,nodez);
        treeint.push_back(data1d_crop[V3DLONG(treep.z*mysz[0]*mysz[1]+treep.y*mysz[0]+treep.x)]);
        neuron[id].fea_val.push_back(data1d_crop[V3DLONG(treep.z*mysz[0]*mysz[1]+treep.y*mysz[0]+treep.x)]);
    }
    NeuronTree nt;
    nt.listNeuron = neuron;
    writeESWC_file(IntTreeName,nt);
//    for(V3DLONG id=0; id<neuron.size(); id++)
//    {
//        meanint = meanint + data1d_crop[V3DLONG(neuron.at(id).z*mysz[0]*mysz[1]+neuron.at(id).y*mysz[0]+neuron.at(id).x)];
////        qDebug() << "Mean intensity in swc" << meanint;
//        qDebug() << "Intensity in swc" << data1d_crop[V3DLONG(neuron.at(id).z*mysz[0]*mysz[1]+neuron.at(id).y*mysz[0]+neuron.at(id).x)];
//    }
//    meanint = meanint/neuron.size();
//    qDebug() << "Mean intensity in swc" << meanint;
//    for(V3DLONG id=0; id<neuron.size(); id++)
//    {
//        if(maxint<data1d_crop[V3DLONG(neuron.at(id).z*mysz[0]*mysz[1]+neuron.at(id).y*mysz[0]+neuron.at(id).x)]) maxint = data1d_crop[V3DLONG(neuron.at(id).z*mysz[0]*mysz[1]+neuron.at(id).y*mysz[0]+neuron.at(id).x)] ;
////        qDebug() << "Mean intensity in swc" << meanint;
//    }
//    qDebug() << "Max intensity in swc" << maxint;
//    for(V3DLONG imi=1; imi<V3DLONG(mysz[2]*mysz[0]*mysz[1]); imi++)
//    {
//        if(0 < data1d_crop[imi]) qDebug() << "Intensity in image" << float(data1d_crop[imi]);
//        if(maxintim < data1d_crop[imi]) maxintim = float(data1d_crop[imi]);
//    }
//    qDebug() << "Max intensity in image" << maxintim;
    for(V3DLONG imi=1; imi<V3DLONG(mysz[2]*mysz[0]*mysz[1]); imi++)
    {
//        if(0 < data1d_crop[imi]) qDebug() << "Intensity in image" << float(data1d_crop[imi]);
        if(2 < data1d_crop[imi])
        {
            medianint.push_back(int(data1d_crop[imi]));
//            nzpixels++;
//            meanint = meanint + float(data1d_crop[imi]);
            if(maxintim < data1d_crop[imi]) maxintim = float(data1d_crop[imi]);
        }
    }
//    meanint = meanint/nzpixels;
//    qDebug() << "Mean non-zero intensity in image" << meanint;
    meanint = median(medianint);  
//    double stdev = StandardDeviation(medianint);
    qDebug() << "Median non-zero intensity in image" << meanint;

//    double imgave,imgstd;
//    V3DLONG total_size=mysz[0]*mysz[1]*mysz[2];
//    mean_and_std(data1d_crop,total_size,imgave,imgstd);
//    //double td= (imgstd<10) ? 10:imgstd;
//    double bkg_thresh= imgave+0.7*imgstd;//+15;
//    qDebug() << "Background threshold" << bkg_thresh;

    float mediantreeint = median(treeint);
    qDebug() << "Median tree intensity" << mediantreeint;

    float mediantreerad = median(radii);
    qDebug() << "Median tree radius" << mediantreerad;

//    float mintreerad = *min_element(radii.begin(),radii.end());
//    qDebug() << "Minimum tree radius" << mintreerad;

//    V3DLONG radius = round(20*mediantreerad);
//    qDebug() << "Shell radius for missing branch search" << radius;

    for(V3DLONG id=0; id<neuron.size(); id++)
    {
        //qDebug() << id;
        //float theta,phi;
//        V3DLONG walknode,walknode2 = id;
//        struct XYZ invec = XYZ(-1,-1,-1);
//        for (int i=0; i<160; i++)
//        {
//            locswc.push_back(invec);
//        }
//        for (int i=1; i<80; i++)
//        {
//            if(walknode!=-1) walknode = neuron.at(walknode).pn; // Check root case
//            if(walknode!=-1) locswc[80+i] = XYZ(neuron.at(walknode).x,neuron.at(walknode).y,neuron.at(walknode).z);
//            cout << "\r Getting swc node locations..." << flush;
//            if(walknode2!=-1)
//            {
//                long childnodeid = find(parents.begin(),parents.end(),neuron.at(walknode2).n-1) - parents.begin();
//                if(find(parents.begin(),parents.end(),neuron.at(walknode2).n-1)!= parents.end())
//                {
//                    walknode2 = neuron.at(parents.at(childnodeid)).n-1; // Check tip case
//                    if(walknode2!=-1 && 80-i>=0) locswc[80-i] = XYZ(neuron.at(walknode2).x,neuron.at(walknode2).y,neuron.at(walknode2).z);
//                }
//            }
//        }
        //cout << endl;
        //qDebug() << "locswcs obtained";
        qDebug() << "Radius:" << neuron.at(id).radius;
        if(neuron.at(id).radius>7){continue;} // out of image border

        V3DLONG nodex = V3DLONG(neuron.at(id).x+neuron.at(id).radius);
        V3DLONG nodey = V3DLONG(neuron.at(id).y+neuron.at(id).radius);
        V3DLONG nodez = V3DLONG(neuron.at(id).z+neuron.at(id).radius);
//        qDebug() << nodex << nodey << nodez;

        //////////////////////////////////////////////////////////
        V3DLONG radius = round(20*neuron.at(id).radius);
        int windowradius = int(1*radius);
        V3DLONG y_offset=mysz[0];
        V3DLONG z_offset=mysz[0]*mysz[1];
        V3DLONG page_size=mysz[0]*mysz[1]*mysz[2];

        V3DLONG N = mysz[0];
        V3DLONG M = mysz[1];
        V3DLONG P = mysz[2];

//        V3DLONG pos;
//        vector<V3DLONG> coord;

        float total_x,total_y,total_z,v_color,sum_v,v_prev,x,y,z;
        float center_dis=1;
        vector<float> center_float(3,0);

        //coord=pos2xyz(ind, y_offset, z_offset);
        x=(float)nodex;y=(float)nodey;z=(float)nodez;
        //qDebug()<<"x,y,z:"<<x<<":"<<y<<":"<<z<<"ind:"<<ind;

        //qDebug()<<"v_Prev:"<<v_prev;
//        int testCount=0;
    //    int testCount1=0;

//        while (center_dis>=0.5 && testCount<50)
//        {
        total_x=total_y=total_z=sum_v=0;
//        testCount++;

        V3DLONG xb = MAX(x+0.5-windowradius,0);
        V3DLONG xe = MIN(mysz[0]-1,x+0.5+windowradius);
        V3DLONG yb = MAX(y+0.5-windowradius,0);
        V3DLONG ye = MIN(mysz[1]-1,y+0.5+windowradius);
        V3DLONG zb = MAX(z+0.5-windowradius,0);
        V3DLONG ze = MIN(mysz[2]-1,z+0.5+windowradius);

        V3DLONG i,j,k;
        double w;

        //first get the average value
        double x2, y2, z2;
        double rx2 = double(windowradius+1)*(windowradius+1);
        double ry2 = rx2, rz2 = rx2;
        double tmpd;
        double xm=0,ym=0,zm=0, s=0, mv=0, n=0, maxint=0, minint=0;

        s = 0; n = 0;
        for(k=zb; k<=ze; k++)
        {
            V3DLONG offsetkl = k*M*N;
            z2 = k-z; z2*=z2;
            for(j=yb; j<=ye; j++)
            {
                V3DLONG offsetjl = j*N;
                y2 = j-y; y2*=y2;
                tmpd = y2/ry2 + z2/rz2;
                if (tmpd>1.0)
                    continue;

                for(i=xb; i<=xe; i++)
                {
                    x2 = i-x; x2*=x2;
                    if (x2/rx2 + tmpd > 1.0)
                        continue;
                    if(maxint < double(data1d_crop[offsetkl + offsetjl + i]))
                    {
                        maxint = double(data1d_crop[offsetkl + offsetjl + i]);
                    }
                    else
                    {
                        minint = double(data1d_crop[offsetkl + offsetjl + i]);
                    }
                    s += double(data1d_crop[offsetkl + offsetjl + i]);
                    n = n+1;
                }
            }
        }
        if (n!=0)
            mv = s/n;
        else
            mv = 0;

        // get standard deviation
        double var=0.0;
        double ep=0.0;
        double sdev=0.0;
        for(k=zb; k<=ze; k++)
        {
            V3DLONG offsetkl = k*M*N;
            z2 = k-z; z2*=z2;
            for(j=yb; j<=ye; j++)
            {
                V3DLONG offsetjl = j*N;
                y2 = j-y; y2*=y2;
                tmpd = y2/ry2 + z2/rz2;
                if (tmpd>1.0)
                    continue;

                for(i=xb; i<=xe; i++)
                {
                    x2 = i-x; x2*=x2;
                    if (x2/rx2 + tmpd > 1.0)
                        continue;

                    s = double(data1d_crop[offsetkl + offsetjl + i]-mv);
                    var += s*s;
//                        n = n+1;
                }
            }
        }
        var=(var-ep*ep/n)/(n-1);
        sdev=sqrt(var);

        /////////////////////////////////////////////////////
//        }
//        double imgave,imgstd;
//        unsigned char *data1d_crop2=data1d_crop[];
//        V3DLONG total_size=20*20*8;//mysz[0]*mysz[1]*mysz[2];
//        mean_and_std(data1d_crop2,total_size,imgave,imgstd);
        //double td= (imgstd<10) ? 10:imgstd;
//        double bkg_thresh= 1.3*mv+5*sdev;//+15;*0.7
//        double bkg_thresh= meanint+2.8*(mediantreeint-meanint);//+15;*0.7
//        double bkg_thresh = mv + 0.7*(maxint-mv);
//        double bkg_thresh = mv+0.7*sdev+30;
//        double bkg_thresh =  4.5*sqrt(mv*sdev)+60/(sdev*sdev*sdev); //0.5*mv*sdev+mv+0.1*sdev;
//        double bkg_thresh =  0.85*(4.5*sqrt(mv*sdev)+60/(sdev*sdev*sdev))+0.1*data1d_crop[V3DLONG(mysz[0]*mysz[1]*nodez+mysz[0]*nodey+nodex)]; //0.5*mv*sdev+mv+0.1*sdev;
        //double bkg_thresh =  0.77*(0.7*data1d_crop[V3DLONG(mysz[0]*mysz[1]*nodez+mysz[0]*nodey+nodex)]+1*mv*sdev)/2;//10/sdev; //0.5*mv*sdev+mv+0.1*sdev;
//        double bkg_thresh =  1*(data1d_crop[V3DLONG(mysz[0]*mysz[1]*nodez+mysz[0]*nodey+nodex)]-2*sdev);//10/sdev; //0.5*mv*sdev+mv+0.1*sdev;
        V3DLONG walkid = id;
        double localswcmv=0;
        double nsteps=0;
        for(int V3DLONG steps=1; steps<5; steps++)
        {
            V3DLONG walknodex = V3DLONG(neuron.at(walkid).x);
            V3DLONG walknodey = V3DLONG(neuron.at(walkid).y);
            V3DLONG walknodez = V3DLONG(neuron.at(walkid).z);
            localswcmv += data1d_crop[V3DLONG(mysz[0]*mysz[1]*walknodez+mysz[0]*walknodey+walknodex)];
            walkid = neuron.at(walkid).pn;
            nsteps++;
            if(walkid==-1)break;
        }
        if(nsteps>0)localswcmv = localswcmv/nsteps;
        qDebug() << "Adaptive local tree mean int value" << localswcmv;

        double bkg_thresh = 0; //sdev > 1.36
//        if(20*sdev>localswcmv){
        if(sdev>1.36){
            bkg_thresh = mv+sdev+minint;//1.4*(1.3*mv+3.5*sdev+minint);//mv+0.7*sdev;//localswcmv+sdev*sdev
        }
        else{
            bkg_thresh = double(minint)+sdev*5;//+5
        }
//        double bkg_thresh = (mv+localswcmv)/2+1.8*sdev;
//        double bkg_thresh = localswcmv-7+0.2*mv*sdev;
//        double bkg_thresh = data1d_crop[V3DLONG(mysz[0]*mysz[1]*nodez+mysz[0]*nodey+nodex)]-2;
        //        double bkg_thresh =  2*sqrt(mv*sdev)+55/sdev; //0.5*mv*sdev+mv+0.1*sdev;
//        double bkg_thresh =  0.8*mv*sdev+0.2*mv+sdev; //0.5*mv*sdev+mv+0.1*sdev;
//        double bkg_thresh2 =  2.5*(mv+0.7*sdev+30); //0.5*mv*sdev+mv+0.1*sdev;
//        double bkg_thresh = 0.5*mv*sdev+mv+0.1*sdev;//0.6*maxint+mv+0.7*sdev; // might be useful for low signal to noise
//        double bkg_thresh = 0.5*(mv+maxint);//0.6*maxint+mv+0.7*sdev;

        qDebug() << "Adaptive background threshold" << bkg_thresh;
        qDebug() << "Adaptive background mean value" << mv;
        qDebug() << "Adaptive background max value" << maxint;
        qDebug() << "Adaptive background min value" << minint;
        qDebug() << "Adaptive background std dev" << sdev;

        qDebug() << "";

        double anglestep = asin(1/double(radius))*4;
//        qDebug() << "Angle step" << anglestep;

        for(double theta=0; theta<PI; theta+=anglestep) // Check the step in function of the radius (arcsin(1/radius))
        {
            //qDebug() << theta;
            for(double phi=0; phi<2*PI; phi+=anglestep)
            {
                //qDebug() << phi;
                struct XYZ shellp = XYZ(nodex+round(radius*sin(theta)*cos(phi)),nodey+round(radius*sin(theta)*sin(phi)),nodez+round(radius*cos(theta)/3)); // radius*cos(theta)/5
//                struct XYZ shellp2 = XYZ(nodex+round(0.75*radius*sin(theta)*cos(phi)),nodey+round(0.75*radius*sin(theta)*sin(phi)),nodez+round(0.75*radius*cos(theta)/3)); // radius*cos(theta)/5
                if((shellp.x<0) || (shellp.x>=mysz[0]) || (shellp.y<0) || (shellp.y>=mysz[1]) || (shellp.z<0) || (shellp.z>=mysz[2])){continue;} // out of image border
//                if((shellp2.x<0) || (shellp2.x>=mysz[0]) || (shellp2.y<0) || (shellp2.y>=mysz[1]) || (shellp2.z<0) || (shellp2.z>=mysz[2])){continue;} // out of image border
                for(int i=0; i<neuron.size(); i++)
//                for(int i=1; i<80; i++)
                {
                    //qDebug() << i;
                    // Checks whether a point is in a shell of radius r<15 pixels and if intensity >15
//                    if(dist_L2(shellp,locswc.at(i))<15 && data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=bkg_thresh) candidates.push_back(shellp);
//                    if(meanint*3 > mediantreeint)
//                    {
//                        if(dist_L2(shellp,locswc.at(i))<15 && data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=meanint*6) candidates.push_back(shellp);
//                    }
//                    else
//                    {
//                        if(dist_L2(shellp,locswc.at(i))<15 && data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=mediantreeint) candidates.push_back(shellp);
//                    }
                    if(dist_L2(shellp,locswc.at(i))<=radius && double(data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)])>=bkg_thresh) candidates.push_back(shellp);
//                    else if(dist_L2(shellp,locswc.at(i))<radius && double(data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)])>=bkg_thresh2) candidates.push_back(shellp);
//                    if(dist_L2(shellp2,locswc.at(i))<=0.75*radius && double(data1d_crop[V3DLONG(shellp2.z*mysz[0]*mysz[1]+shellp2.y*mysz[0]+shellp2.x)])>=bkg_thresh) candidates.push_back(shellp2);
//                    if(datatype==V3D_UINT8){
//                        if(dist_L2(shellp,locswc.at(i))<radius && data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=bkg_thresh+15) candidates.push_back(shellp);
//                    }
//                    else if(datatype==V3D_UINT16)
//                    {
//                        if(dist_L2(shellp,locswc.at(i))<radius && data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=bkg_thresh+3825) candidates.push_back(shellp);
//                    }
//                    if(datatype==V3D_UINT8){
//                        if(dist_L2(shellp,locswc.at(i))<15 && data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=meanint+30) candidates.push_back(shellp);
//                    }
//                    else if(datatype==V3D_UINT16)
//                    {
//                        if(dist_L2(shellp,locswc.at(i))<15 && data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=meanint+7650) candidates.push_back(shellp);
//                    }
//                                        if(locswc.at(80-i)==invec && locswc.at(80+i)!=invec)
//                    {
//                        if(dist_L2(shellp,locswc.at(80+i))>=20 && data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=40) candidates.push_back(shellp);
//                    }
//                    else if(locswc.at(80+i)==invec && locswc.at(80-i)!=invec)
//                    {
//                        if(dist_L2(shellp,locswc.at(80-i))>=20 && data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=40) candidates.push_back(shellp);
//                    }
//                    else if(locswc.at(80+i)!=invec && locswc.at(80-i)!=invec)
//                    {
//                        if(dist_L2(shellp,locswc.at(80-i))>=20 && dist_L2(shellp,locswc.at(80+i))>=20 && data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=40) candidates.push_back(shellp);//To be checked!!
//                    }
                }
            }
        }
    }
    cout << endl;

//    int windowradius = 10; //Check!
//    LandmarkList llist;
//    ImagePixelType pixtype=V3D_UINT16;
//    load_data(&callback,data1d_crop,llist,pixtype,mysz,curwin);
    for (V3DLONG i=0; i<candidates.size(); i++)
    {
        bool out=0;
        LocationSimple candidate;
//        LocationSimple candidatef;
        candidate.x = long(candidates.at(i).x);
        candidate.y = long(candidates.at(i).y);
        candidate.z = long(candidates.at(i).z);

        for(V3DLONG j=0; j<neuron.size(); j++)
        {
//            if(dist_L2(XYZ(neuron.at(j).x,neuron.at(j).y,neuron.at(j).z),XYZ(candidate.x,candidate.y,candidate.z))<double(radius)/3) out = 1;
        }
        if(out==0 && find(candidates_m.begin(),candidates_m.end(),candidate) == candidates_m.end()){
            candidates_m.push_back(candidate);
//            V3DLONG candint = 0;
//            for(V3DLONG exp=0; exp<20; exp++){
//                for(V3DLONG exp2=0; exp2<20; exp2++){
//                    for(V3DLONG exp3=0; exp3<20; exp3++){
//                        struct XYZ explp = XYZ(candidate.x+exp-10,candidate.y+exp2-10,candidate.z+exp3-10);
//                        if((explp.x<0) || (explp.x>=mysz[0]) || (explp.y<0) || (explp.y>=mysz[1]) || (explp.z<0) || (explp.z>=mysz[2])){continue;} // out of image border
//                        if(candint < data1d_crop[V3DLONG(explp.z*mysz[0]*mysz[1]+explp.y*mysz[0]+explp.x)]){
//                            candint = data1d_crop[V3DLONG(explp.z*mysz[0]*mysz[1]+explp.y*mysz[0]+explp.x)];
//                            candidatef.x = explp.x;
//                            candidatef.y = explp.y;
//                            candidatef.z = explp.z;
//                        }
//                    }
//                }
//            }
        }
//        if(find(final_pts.begin(),final_pts.end(),XYZ(candidatef.x,candidatef.y,candidatef.z)) != final_pts.end()){
//            final_pts.push_back(XYZ(candidatef.x,candidatef.y,candidatef.z));
//        }
//        if(find(candidates_f.begin(),candidates_f.end(),candidatef) != candidates_f.end()){
//            candidates_f.push_back(candidatef);
//        }
//        //qDebug() << long(candidates.at(i).x) << long(candidates.at(i).y) << long(candidates.at(i).z);
//        V3DLONG ind = xyz2pos(long(candidates.at(i).x),long(candidates.at(i).y),long(candidates.at(i).z),mysz[1],mysz[2]);
//        vector <float>  centered = calc_mean_shift_center(ind,windowradius,mean_shift_fun::data1Dc_float,mean_shift_fun::sz_image,0);
//        struct XYZ centeredxyz = XYZ(centered[0],centered[1],centered[2]);
//        if(find(final_pts.begin(),final_pts.end(),centeredxyz) != final_pts.end()) final_pts.push_back(centeredxyz);
    }

//    for (V3DLONG i=0; i<final_pts.size(); i++)
//    {
//        qDebug() << long(final_pts.at(i).x) << long(final_pts.at(i).y) << long(final_pts.at(i).z);
//    }

    vector <bool> out;
    for (V3DLONG i=0; i<candidates_m.size(); i++)
    {
        out.push_back(0);
    }
    for (V3DLONG i=0; i<candidates_m.size(); i++)
    {
//                    bool out=0;
//                    bool first=1;
        for(V3DLONG j=0; j<neuron.size(); j++)
        {
            V3DLONG radius=round(neuron.at(j).radius*20);
            if(dist_L2(XYZ(neuron.at(j).x,neuron.at(j).y,neuron.at(j).z),XYZ(candidates_m.at(i).x,candidates_m.at(i).y,candidates_m.at(i).z))<double(radius)*0.8) out.at(i) = 1;
//can be useful for small blocks                if(dist_L2(XYZ(neuron.at(j).x,neuron.at(j).y,neuron.at(j).z),XYZ(final_pts.at(i).x,final_pts.at(i).y,final_pts.at(i).z))>90) out.at(i) = 1;
//            if(dist_L2(XYZ(neuron.at(j).x,neuron.at(j).y,neuron.at(j).z),XYZ(candidates_m.at(i).x,candidates_m.at(i).y,candidates_m.at(i).z))>50*radius) out.at(i) = 1;
        }
        for (V3DLONG j=i+1; j<candidates_m.size(); j++)
        {
            if(dist_L2(XYZ(candidates_m.at(j).x,candidates_m.at(j).y,candidates_m.at(j).z),XYZ(candidates_m.at(i).x,candidates_m.at(i).y,candidates_m.at(i).z))<double(40/3))
            {
                if(out.at(i)==0 && data1d_crop[V3DLONG(candidates_m.at(i).z*mysz[0]*mysz[1]+candidates_m.at(i).y*mysz[0]+candidates_m.at(i).x)] >= data1d_crop[V3DLONG(candidates_m.at(j).z*mysz[0]*mysz[1]+candidates_m.at(j).y*mysz[0]+candidates_m.at(j).x)])
                {
                    out.at(j) = 1;
                }
                else if(out.at(j)==0 && data1d_crop[V3DLONG(candidates_m.at(i).z*mysz[0]*mysz[1]+candidates_m.at(i).y*mysz[0]+candidates_m.at(i).x)] < data1d_crop[V3DLONG(candidates_m.at(j).z*mysz[0]*mysz[1]+candidates_m.at(j).y*mysz[0]+candidates_m.at(j).x)])
                {
                    out.at(i) = 1;
                }
            }
        }
        if(out.at(i)==0)candidates_mf.push_back(candidates_m.at(i));
    }
    qDebug() << "Candidates vector size is:" << candidates.size();
//    qDebug() << "Final points vector size is:" << candidates_m.size();
    qDebug() << "Final points vector size is:" << candidates_mf.size();
    return candidates_mf;
//    return candidates_f;
//    long RR = 40; //radius spherical coordinates shell around swc
//    for(int id=0; id<neuron.size(); id++)
//    {
//        if(neuron.at(id).pn != -1)
//        {
//            float vx = neuron.at(id).x - neuron.at(neuron.at(id).pn-1).x;
//            float vy = neuron.at(id).y - neuron.at(neuron.at(id).pn-1).y;
//            float vz = neuron.at(id).z - neuron.at(neuron.at(id).pn-1).z;
//            qDebug() << "Swc node vector:" << vx << vy << vz;

//            // Get spherical coordinates of unit vector
//            struct XYZ vec = XYZ(vx,vy,vz);
//            float R = norm(vec);
//            struct XYZ uvec = normalize(vec);
//            qDebug() << "Unit vector:" << uvec.x << uvec.y << uvec.z;

//            float theta = acos(uvec.z);
//            float phi = atan(uvec.y/uvec.x);

//            // Get z coordinates
//            long minz,maxz,xin,yin;
//            if(vz>=0)
//            {
//                minz = neuron.at(neuron.at(id).pn-1).z;
//                maxz = neuron.at(id).z;
//                xin = neuron.at(neuron.at(id).pn-1).x;
//                yin = neuron.at(neuron.at(id).pn-1).y;
//            }
//            else
//            {
//                maxz = neuron.at(neuron.at(id).pn-1).z;
//                minz = neuron.at(id).z;
//                xin = neuron.at(id).x;
//                yin = neuron.at(id).y;
//            }

//            qDebug() << "Radius:" << R;
//            for(long zz=minz; zz<maxz; zz++) //To be rotated (by now theta=0 phi=0)
//            {
//                long xx = xin + (zz-minz)*sin(theta);
//                for(double alpha=0; alpha<2*PI; alpha+=0.1)
//                {
//                    long yy = RR*sin(alpha);
//                    xx = RR*cos(alpha);
//                    qDebug() << "Cartesian coordinates:" << xx << yy << zz;
//                }
//            }
//        }
//    }

}

void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"This plugin detects missing branches based on current swc."<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_ML_sample\n";
    cout<<"-i<file name>:\t\t input .tif file\n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x missing_branch_detection -f missing_branch_detection -i input.image input.swcfile -o output.image.\n";

}
