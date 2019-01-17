
#include "get_missing_branches.h"
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include "qlist.h"
#include "openSWCDialog.h"
#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"
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

void get_missing_branches_menu(V3DPluginCallback2 &callback, QWidget *parent, Image4DSimple * p4DImage)
{
    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return;

    // Get Neuron
    NeuronTree nt = openDlg->nt;
    QList<NeuronSWC> neuron = nt.listNeuron;

    // Get Image Data
    int nChannel = p4DImage->getCDim();

    V3DLONG mysz[4];
    mysz[0] = p4DImage->getXDim();
    mysz[1] = p4DImage->getYDim();
    mysz[2] = p4DImage->getZDim();
    mysz[3] = nChannel;
    cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
    unsigned char *data1d_crop=p4DImage->getRawDataAtChannel(nChannel);
    printf("+++++++++++:%p\n",p4DImage);

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
    long radius = 40;
    vector <struct XYZ> candidates;
    for(V3DLONG id=0; id<neuron.size(); id++)
    {
        //qDebug() << id;
        //float theta,phi;
        vector <struct XYZ> locswc;
        V3DLONG walknode,walknode2 = id;
        struct XYZ invec = XYZ(-1,-1,-1);
        for (int i=0; i<160; i++)
        {
            locswc.push_back(invec);
        }
        for (int i=1; i<80; i++)
        {
            if(walknode!=-1) walknode = neuron.at(walknode).pn; // Check root case
            if(walknode!=-1) locswc[80+i] = XYZ(neuron.at(walknode).x,neuron.at(walknode).y,neuron.at(walknode).z);
            cout << "\r Getting swc node locations..." << flush;
            if(walknode2!=-1)
            {
                long childnodeid = find(parents.begin(),parents.end(),neuron.at(walknode2).n-1) - parents.begin();
                if(find(parents.begin(),parents.end(),neuron.at(walknode2).n-1)!= parents.end())
                {
                    walknode2 = neuron.at(parents.at(childnodeid)).n-1; // Check tip case
                    if(walknode2!=-1 && 80-i>=0) locswc[80-i] = XYZ(neuron.at(walknode2).x,neuron.at(walknode2).y,neuron.at(walknode2).z);
                }
            }
        }
        //cout << endl;
        //qDebug() << "locswcs obtained";
        long nodex = neuron.at(id).x;
        long nodey = neuron.at(id).y;
        long nodez = neuron.at(id).z;
        //qDebug() << nodex << nodey << nodez;
        for(double theta=0; theta<PI; theta+=0.1) // Check the step in function of the radius
        {
            //qDebug() << theta;
            for(double phi=0; phi<2*PI; phi+=0.1)
            {
                //qDebug() << phi;
                struct XYZ shellp = XYZ(nodex+radius*sin(theta)*cos(phi),nodey+radius*sin(theta)*sin(phi),nodez+radius*cos(theta)/5);
                if((shellp.x<0) || (shellp.x>=mysz[0]) || (shellp.y<0) || (shellp.y>=mysz[1]) || (shellp.z<0) || (shellp.z>=mysz[2])){continue;} // out of image border
                for(int i=1; i<80; i++)
                {
                    //qDebug() << i;
                    // Checks whether a point is in a shell of radius 20<r<40 pixels and if intensity >40
                    if(locswc.at(80-i)==invec && locswc.at(80+i)!=invec)
                    {
                        if(dist_L2(shellp,locswc.at(80+i))>=20 && data1d_crop[long(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=40) candidates.push_back(shellp);
                    }
                    else if(locswc.at(80+i)==invec && locswc.at(80-i)!=invec)
                    {
                        if(dist_L2(shellp,locswc.at(80-i))>=20 && data1d_crop[long(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=40) candidates.push_back(shellp);
                    }
                    else if(locswc.at(80+i)!=invec && locswc.at(80-i)!=invec)
                    {
                        if(dist_L2(shellp,locswc.at(80-i))>=20 && dist_L2(shellp,locswc.at(80+i))>=20 && data1d_crop[long(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=40) candidates.push_back(shellp);//To be checked!!
                    }
                }
            }
        }
    }
    cout << endl;

    for (V3DLONG i=0; i<candidates.size(); i++)
    {
        qDebug() << long(candidates.at(i).x) << long(candidates.at(i).y) << long(candidates.at(i).z);
    }
    qDebug() << "Candidates vector size is:" << candidates.size();
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
