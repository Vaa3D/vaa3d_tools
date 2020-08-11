
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

    vector<long> ids;
    vector<long> parents;
    // Reorder tree ids so that neuron.at(i).n=i+1
    for(V3DLONG i=0;i<neuron.size();i++)
    {
        ids.push_back(neuron.at(i).n);
    }
    for(V3DLONG i=0;i<neuron.size();i++)
    {
//        neuron[i].n=i+1;
        if(neuron.at(i).pn !=-1)
        {
//            neuron[i].pn=find(ids.begin(), ids.end(),neuron.at(i).pn) - ids.begin()+1;
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
    qDebug() << "Feature value set.";
    vector <V3DLONG> tipid;
    for(V3DLONG id=0; id<neuron.size(); id++)
    {
        if(find(parents.begin(),parents.end(),neuron.at(id).n)==parents.end()){tipid.push_back(neuron.at(id).n);}
    }

    NeuronTree nt;
    nt.listNeuron = neuron;
    writeESWC_file(IntTreeName,nt);

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

    float mediantreeint = median(treeint);
    qDebug() << "Median tree intensity" << mediantreeint;

    float mediantreerad = median(radii);
    qDebug() << "Median tree radius" << mediantreerad;

//    float mintreerad = *min_element(radii.begin(),radii.end());
//    qDebug() << "Minimum tree radius" << mintreerad;

//    V3DLONG radius = round(20*mediantreerad);
//    qDebug() << "Shell radius for missing branch search" << radius;

    for(V3DLONG id=0; id<neuron.size(); id+=3)
    {
        qDebug() << "Node " << id+1 << " of " << neuron.size();
        qDebug() << "Radius:" << neuron.at(id).radius;
        if(neuron.at(id).radius>5){continue;} // out of image border

        V3DLONG nodex = V3DLONG(neuron.at(id).x);
        V3DLONG nodey = V3DLONG(neuron.at(id).y);
        V3DLONG nodez = V3DLONG(neuron.at(id).z);
//        V3DLONG nodex = V3DLONG(neuron.at(id).x+neuron.at(id).radius);
//        V3DLONG nodey = V3DLONG(neuron.at(id).y+neuron.at(id).radius);
//        V3DLONG nodez = V3DLONG(neuron.at(id).z+neuron.at(id).radius);
//        qDebug() << nodex << nodey << nodez;

        //////////////////////////////////////////////////////////
        V3DLONG radius = round(10*neuron.at(id).radius);
        int windowradius;
        if(radius>10){
            windowradius = int(15);
            radius=15;
        }
        else
        {
            windowradius = int(1*radius);
        }
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
                    else if(minint > double(data1d_crop[offsetkl + offsetjl + i]))
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
        struct XYZ walkp;
        for(int V3DLONG steps=1; steps<5; steps++)
        {
            V3DLONG walknodex = V3DLONG(neuron.at(walkid).x);
            V3DLONG walknodey = V3DLONG(neuron.at(walkid).y);
            V3DLONG walknodez = V3DLONG(neuron.at(walkid).z);
            localswcmv += data1d_crop[V3DLONG(mysz[0]*mysz[1]*walknodez+mysz[0]*walknodey+walknodex)];
            walkp = XYZ(walknodex,walknodey,walknodez);
            walkid = neuron.at(walkid).pn-1;
            nsteps++;
            if(walkid==-1)break;
        }
        if(nsteps>0)localswcmv = localswcmv/nsteps;
        qDebug() << "Adaptive local tree mean int value" << localswcmv;

//        double bkg_thresh = minint + 0.5;
//        double bkg_thresh = minint + 0.05*(localswcmv-minint);//+0.7*sdev); //sdev > 1.36
        double bkg_thresh = 0.3*mv + 0.7*sdev + minint;//+0.7*sdev); //sdev > 1.36
//        double bkg_thresh = (minint+1) *1.2; //sdev > 1.36
//        double bkg_thresh = mv*0.5; //sdev > 1.36
//        double bkg_thresh = 0; //sdev > 1.36
//        if(20*sdev>localswcmv){
//        if(sdev>1.36){
//            bkg_thresh = mv+sdev+minint;//1.4*(1.3*mv+3.5*sdev+minint);//mv+0.7*sdev;//localswcmv+sdev*sdev
//        }
//        else{
//            bkg_thresh = double(minint);//+sdev*5;//+5
//        }
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

        double anglestep = asin(1/double(radius))*2;
//        qDebug() << "Angle step" << anglestep;

        double theta0 = 0;
        double thetaf=PI;
        double phi0 = 0;
        double phif=2*PI;
        if(find(tipid.begin(),tipid.end(),neuron.at(id).n)!=tipid.end()){
//            theta0=0;
//            thetaf=PI;
//            phi0=0;
//            phif=2*PI;
            theta0=0;
            thetaf=2*PI;
            phi0=0;
            phif=PI;
            radius=1.7*radius;
            bkg_thresh=0.5*bkg_thresh;
        }
        else{
//            theta0=0;
//            thetaf=PI;
//            theta0 = acos(double(nodez-walkp.z)/sqrt(double((nodex-walkp.x)*(nodex-walkp.x)+(nodey-walkp.y)*(nodey-walkp.y)+(nodez-walkp.z)*(nodez-walkp.z))))+PI/2-PI/3;
//            theta0 = acos(double(nodez-walkp.z)/(nsteps*3))+PI/2-PI/3;
//            thetaf=theta0+2*PI/3;
//            qDebug() << "Theta 0: " << theta0 << "Theta f: " << thetaf;
//            qDebug() << "node z: " << nodez << "walkp z: " << walkp.z << " nsteps:" << nsteps;
//            phi0=0;
//            phif=2*PI;
            theta0=0;
            thetaf=2*PI;
            phi0=atan(double(nodey-walkp.y)/double(nodex-walkp.x))+PI/2-PI/5;
            phif=phi0+2*PI/5;
            qDebug() << "Phi 0: " << phi0 << "Phi f: " << phif;
            qDebug() << "node y: " << nodey << "walkp y: " << walkp.y << "node x: " << nodex << "walkp x: " << walkp.x << " nsteps:" << nsteps;
            radius=1.5*radius;
        }

        vector <struct XYZ> loccandidates1;
        vector <V3DLONG> loccandidatesint1;

        for(double theta=theta0; theta<=thetaf; theta+=anglestep) // Check the step in function of the radius (arcsin(1/radius))
        {
            //qDebug() << theta;
            for(double phi=phi0; phi<=phif; phi+=anglestep)
            {
                //qDebug() << phi;
                struct XYZ shellp = XYZ(nodex+round(radius*sin(theta)*cos(phi)),nodey+round(radius*sin(theta)*sin(phi)),nodez+round(radius*cos(theta))); // radius*cos(theta)/5
                struct XYZ shellp2 = XYZ(nodex+round(1.5*radius*sin(theta)*cos(phi)),nodey+round(1.5*radius*sin(theta)*sin(phi)),nodez+round(1.1*radius*cos(theta))); // radius*cos(theta)/5
                if((shellp.x<0) || (shellp.x>=mysz[0]) || (shellp.y<0) || (shellp.y>=mysz[1]) || (shellp.z<0) || (shellp.z>=mysz[2])){continue;} // out of image border
                if((shellp2.x<0) || (shellp2.x>=mysz[0]) || (shellp2.y<0) || (shellp2.y>=mysz[1]) || (shellp2.z<0) || (shellp2.z>=mysz[2])){continue;} // out of image border
                for(int i=0; i<neuron.size(); i++)
//                for(int i=1; i<80; i++)
                {
                    //qDebug() << i;
                    float dist = dist_L2(shellp,locswc.at(i));
                    // Checks whether a point is in a shell of radius 20*dendrite radius and if intensity >threshold
                    if(dist < 2*radius && double(data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)])>=bkg_thresh){
                        loccandidates1.push_back(shellp);
                        loccandidatesint1.push_back(V3DLONG(data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]));
                    }
                    if(dist < 3*radius && double(data1d_crop[V3DLONG(shellp2.z*mysz[0]*mysz[1]+shellp2.y*mysz[0]+shellp2.x)])>=bkg_thresh){
                        loccandidates1.push_back(shellp2);
                        loccandidatesint1.push_back(V3DLONG(data1d_crop[V3DLONG(shellp2.z*mysz[0]*mysz[1]+shellp2.y*mysz[0]+shellp2.x)]));
                    }
                }
            }
        }

        vector <struct XYZ> loccandidates;
        vector <V3DLONG> loccandidatesint;
        vector <bool> out;
//        vector <float> dists;

//        V3DLONG maxcdint=*max_element(loccandidatesint1);
        for (V3DLONG cd=0; cd<loccandidates1.size(); cd++)
        {
            out.push_back(0);
        }
        for (V3DLONG cd=0; cd<loccandidates1.size(); cd++)
        {
            float mindist=1000;
            for(V3DLONG j=0; j<neuron.size(); j++)
            {
                float dist=dist_L2(XYZ(neuron.at(j).x,neuron.at(j).y,neuron.at(j).z),loccandidates1.at(cd));
                if(dist<float(radius))
                {
                    out.at(cd) = 1;
                }
                if(dist<mindist)mindist=dist;
            }
            if(out.at(cd)==0)loccandidates.push_back(loccandidates1.at(cd));
            if(out.at(cd)==0)loccandidatesint.push_back(loccandidatesint1.at(cd)/maxint+2*mindist/radius);
        }


        V3DLONG it;
        if(loccandidates.size()>0)
        {
            if(max_element(loccandidatesint.begin(), loccandidatesint.end()) != loccandidatesint.end())
            {
                it = max_element(loccandidatesint.begin(), loccandidatesint.end()) - loccandidatesint.begin();
                candidates.push_back(loccandidates.at(it));
            }
        }
        if(loccandidates.size()>0)
        {
            loccandidates.erase(loccandidates.begin()+it);
            loccandidatesint.erase(loccandidatesint.begin()+it);
            if(max_element(loccandidatesint.begin(), loccandidatesint.end()) != loccandidatesint.end())
            {
                it = max_element(loccandidatesint.begin(), loccandidatesint.end()) - loccandidatesint.begin();
                candidates.push_back(loccandidates.at(it));
            }
        }
        if(loccandidates.size()>0)
        {
            loccandidates.erase(loccandidates.begin()+it);
            loccandidatesint.erase(loccandidatesint.begin()+it);
            if(max_element(loccandidatesint.begin(), loccandidatesint.end()) != loccandidatesint.end())
            {
                it = max_element(loccandidatesint.begin(), loccandidatesint.end()) - loccandidatesint.begin();
                candidates.push_back(loccandidates.at(it));
            }
        }
    }
    qDebug() << "Number of candidates:" << candidates.size();
//    cout << endl;

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

        if(out==0 && find(candidates_m.begin(),candidates_m.end(),candidate) == candidates_m.end()){
            candidates_m.push_back(candidate);
        }
    }

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
            V3DLONG radius=round(neuron.at(j).radius*10);
            if(dist_L2(XYZ(neuron.at(j).x,neuron.at(j).y,neuron.at(j).z),XYZ(candidates_m.at(i).x,candidates_m.at(i).y,candidates_m.at(i).z))<float(radius)*0.9) out.at(i) = 1;
//can be useful for small blocks                if(dist_L2(XYZ(neuron.at(j).x,neuron.at(j).y,neuron.at(j).z),XYZ(final_pts.at(i).x,final_pts.at(i).y,final_pts.at(i).z))>90) out.at(i) = 1;
//            if(dist_L2(XYZ(neuron.at(j).x,neuron.at(j).y,neuron.at(j).z),XYZ(candidates_m.at(i).x,candidates_m.at(i).y,candidates_m.at(i).z))>50*radius) out.at(i) = 1;
        }
        for (V3DLONG j=i+1; j<candidates_m.size(); j++)
        {
            if(dist_L2(XYZ(candidates_m.at(j).x,candidates_m.at(j).y,candidates_m.at(j).z),XYZ(candidates_m.at(i).x,candidates_m.at(i).y,candidates_m.at(i).z))<float(round(neuron.at(j).radius*10*0.8)))
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
