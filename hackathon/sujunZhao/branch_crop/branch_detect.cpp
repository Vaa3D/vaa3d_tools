#include "v3d_message.h"
#include <vector>
#include "branch_crop_plugin.h"
#include <stdio.h>
#include <iostream>
#include <v3d_interface.h>
//#include "../get_terminal.h"
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define PI 3.14159265359
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
using namespace std;

void get_branches(V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    //if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    //if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

//    QString image_file=infiles.at(0);
//    QString swc_file = infiles.at(1);
//    QString output_dir=outfiles.at(0);
//    XYZ block_size=XYZ(100,100,20);
    QString swc_file = infiles.at(0);

    printf("welcome to use get_branch\n");
    NeuronTree nt = readSWC_file(swc_file);
//    if(!output_dir.endsWith("/")){
//        output_dir = output_dir+"/";
//    }
    QString cell_name = swc_file.right(swc_file.size()-swc_file.lastIndexOf("/")-1);
    cell_name = cell_name.left(cell_name.indexOf("."));

    // remove duplicated nodes
    SortSWC(nt.listNeuron, sorted_neuron ,VOID, 0);
    // Find branch points
    QList<int> branch_list = get_branch_points(sorted_neuron, false);
//    cout<<"Number_of_tips\t"<<qPrintable(swc_file)<<"\t"<<tip_list.size()<<endl;
    //    // Crop tip-centered regions one by one
    //    block zcenter_block; // This is a block centered at (0,0,0)
    //    zcenter_block.small = 0-block_size/2;
    //    zcenter_block.large = block_size/2;
    //    QList<QString> output_suffix;
    //    output_suffix.append(QString("nrrd"));
    //    output_suffix.append(QString("swc"));

    //    for(int i=0; i<tip_list.size(); i++){
    ////        if(i>0){break;}
    //        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
    //        qDebug()<<node.n;
    //        if(node.type > 5){continue;}
    //        // create a tip-centered block
    //        XYZ shift;
    //        shift.x = (int)node.x;
    //        shift.y = (int)node.y;
    //        shift.z = (int)node.z;

    //        block crop_block = offset_block(zcenter_block, shift);
    //        crop_block.name = cell_name + "_"+QString::number(i);
    //        // crop image
    //        qDebug()<<crop_block.name;
    //        qDebug()<<crop_block.small.x<<crop_block.small.y<<crop_block.small.z;
    //        qDebug()<<crop_block.large.x<<crop_block.large.y<<crop_block.large.z;

    //        crop_img(image_file, crop_block, output_dir, callback, QString(".nrrd"));
    //        // crop swc
    //        QString output_swc = output_dir+crop_block.name+".swc";
    //        crop_swc(swc_file, output_swc, crop_block);
    ////        my_saveANO(output_dir, crop_block.name, output_suffix);
    //    }
      return;
}

QList<int> get_branch_points(NeuronTree nt, bool include_root){
    QList<int> branch_list;
    QList<int> tip_list;
    QList<int> alln;
    QList<int> plist;
    int N=nt.listNeuron.size();
    map<int, int> t;
    vector< vector<int> > childlist;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        //qDebug() << nt.listNeuron.at(i).n << nt.listNeuron.at(i).pn;
        plist.append(nt.listNeuron.at(i).pn);
        alln.append(nt.listNeuron.at(i).n);
        t.insert(pair<int,int>(plist.at(i),0));
        if(include_root & nt.listNeuron.at(i).pn == -1){
            tip_list.append(i);
        }
    }
    for(int i=0; i<N; i++){

        t.at(plist.at(i)) = t.at(plist.at(i))+1;
        if((plist.count(plist.at(i))>1)&(t.at(plist.at(i)) == 1)){
            branch_list.push_back(i);
        }
    }
    // tips
    for(int i=0; i<N; i++){
        if(plist.count(nt.listNeuron.at(i).n)==0){tip_list.append(i);}
    }

    // branch points pruning, remove length < 10um
    // 1. based on distances
    int n_branch = branch_list.size();
    for(int i=0;i<n_branch;i++){

    }

    return branch_list;
}

// find other branch points in the same cropped block
vector< vector<int> > get_close_points(NeuronTree nt,vector<int> a){
    vector< vector<int> > neighbours;
    int n=a.size();
    for(int i=0; i<n; i++){
        vector<int> cp;
        NeuronSWC node1 = nt.listNeuron.at(a.at(i));
        int min_x = 50;
        int min_y = 50;
        int min_z = 10;
        for(int j=0; j<n; j++){
            NeuronSWC node2 = nt.listNeuron.at(a.at(j));
            if(i != j){
                if(abs(node1.x-node2.x) < min_x){
                    min_x = ceil(abs(node1.x-node2.x));
                }
                if(abs(node1.y-node2.y) < min_y){
                    min_y = ceil(abs(node1.y-node2.y));
                }
                if(abs(node1.z-node2.z) < min_z){
                    min_z = ceil(abs(node1.z-node2.z));
                }
           }
        }
        if(min_x<25){
            min_x = 25;
        }
        if(min_y<25){
            min_y = 25;
        }
        if(min_z<5){
            min_z = 5;
        }
        cp.push_back(min_x);
        cp.push_back(min_y);
        cp.push_back(min_z);
        neighbours.push_back(cp);
    }
    return neighbours;
}


XYZ offset_XYZ(XYZ input, XYZ offset){
    input.x += offset.x;
    input.y += offset.y;
    input.z += offset.z;
    return input;
}

void crop_swc(QString input_swc, QString output_swc, block crop_block)
{

//        crop_img(image_file, crop_block, output_dir, callback, QString(".nrrd"));
//        // crop swc
//        QString output_swc = output_dir+crop_block.name+".swc";
//        crop_swc(swc_file, output_swc, crop_block);
////        my_saveANO(output_dir, crop_block.name, output_suffix);
//    }
    return;
}

void get2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString input_swc=infiles.at(0);
    QString input_image=inparas.at(0);
    QString output_2d_dir=outfiles.at(0);
    if(!output_2d_dir.endsWith("/")){
        output_2d_dir = output_2d_dir+"/";
    }
    QStringList list=input_swc.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
    QString flag1=list1.first();//dont need to match list index
    //printf("______________:%s\n",output_2d_dir.data());
    qDebug()<<input_swc;
    qDebug("number:%s",qPrintable(flag1));
    NeuronTree nt_crop_sorted=readSWC_file(input_swc);
    Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(input_image) ));
    int nChannel = p4dImage->getCDim();

    V3DLONG mysz[4];
    mysz[0] = p4dImage->getXDim();
    mysz[1] = p4dImage->getYDim();
    mysz[2] = p4dImage->getZDim();
    mysz[3] = nChannel;
    cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
    unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);
    //printf("+++++++++++:%p\n",p4dImage);

   V3DLONG pagesz = mysz[0]*mysz[1]*mysz[2];
   unsigned char* data1d_mask = 0;
   data1d_mask = new unsigned char [pagesz];
   memset(data1d_mask,0,pagesz*sizeof(unsigned char));
   double margin=0;//by PHC 20170531
   ComputemaskImage(nt_crop_sorted, data1d_mask, mysz[0], mysz[1], mysz[2], margin);
   //QString labelSaveString = pathname + ".v3draw_label.tif";
   //simple_saveimage_wrapper(callback, labelSaveString.toLatin1().data(),(unsigned char *)data1d_mask, mysz, 1);

   V3DLONG stacksz =mysz[0]*mysz[1];
   unsigned char *image_mip=0;
   image_mip = new unsigned char [stacksz];//2D orignal image
   unsigned char *label_mip=0;
   label_mip = new unsigned char [stacksz];//2D annotation
   for(V3DLONG iy = 0; iy < mysz[1]; iy++)
   {
       V3DLONG offsetj = iy*mysz[0];
       for(V3DLONG ix = 0; ix < mysz[0]; ix++)
       {
           int max_mip = 0;
           int max_label = 0;
           for(V3DLONG iz = 0; iz < mysz[2]; iz++)
           {
               V3DLONG offsetk = iz*mysz[1]*mysz[0];
               if(data1d_crop[offsetk + offsetj + ix] >= max_mip)
               {
                   image_mip[iy*mysz[0] + ix] = data1d_crop[offsetk + offsetj + ix];
                   max_mip = data1d_crop[offsetk + offsetj + ix];
               }
               if(data1d_mask[offsetk + offsetj + ix] >= max_label)
               {
                   label_mip[iy*mysz[0] + ix] = data1d_mask[offsetk + offsetj + ix];
                   max_label = data1d_mask[offsetk + offsetj + ix];
               }
           }
       }
   }
   unsigned char* data1d_2D = 0;
   data1d_2D = new unsigned char [3*stacksz];//3 channels image
   for(V3DLONG i=0; i<stacksz; i++)
       data1d_2D[i] = image_mip[i];

   for(V3DLONG i=0; i<stacksz; i++)
   {
       data1d_2D[i+stacksz] = (label_mip[i] ==255) ? 255: image_mip[i];
   }
   for(V3DLONG i=0; i<stacksz; i++)
       data1d_2D[i+2*stacksz] = image_mip[i];

   mysz[2] = 1;
   mysz[3] = 3;
   QString mipoutpuut = output_2d_dir +flag1+"_"+"mip.tif";
   simple_saveimage_wrapper(callback,mipoutpuut.toStdString().c_str(),(unsigned char *)data1d_2D,mysz,1);
   if(data1d_crop) {delete [] data1d_crop; data1d_crop=0;}
   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
   if(image_mip) {delete [] image_mip; image_mip=0;}
   if(label_mip) {delete [] label_mip; label_mip=0;}
   //listNeuron.clear();
}

// check missing branches
double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2)
{
       double xx = s1.x-s2.x;
       double yy = s1.y-s2.y;
       double zz = s1.z-s2.z;
       return (xx*xx+yy*yy+zz*zz);
};

LandmarkList get_missing_branches_menu(V3DPluginCallback2 &callback, QWidget *parent, Image4DSimple * p4DImage)
{
    LandmarkList candidates_m;

    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return candidates_m;

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
    V3DLONG radius = 40;
    vector <struct XYZ> candidates;
    vector <struct XYZ> locswc;
    vector <struct XYZ> final_pts;
    for(V3DLONG id=0; id<neuron.size(); id++)
    {
        locswc.push_back(XYZ(neuron.at(id).x,neuron.at(id).y,neuron.at(id).z));
    }
    for(V3DLONG id=0; id<neuron.size(); id++)
    {
        V3DLONG nodex = neuron.at(id).x;
        V3DLONG nodey = neuron.at(id).y;
        V3DLONG nodez = neuron.at(id).z;
        //qDebug() << nodex << nodey << nodez;
        for(double theta=0; theta<PI; theta+=0.8) // Check the step in function of the radius
        {
            //qDebug() << theta;
            for(double phi=0; phi<2*PI; phi+=0.8)
            {
                //qDebug() << phi;
                struct XYZ shellp = XYZ(nodex+round(radius*sin(theta)*cos(phi)),nodey+round(radius*sin(theta)*sin(phi)),nodez+round(radius*cos(theta)/5));
                if((shellp.x<0) || (shellp.x>=mysz[0]) || (shellp.y<0) || (shellp.y>=mysz[1]) || (shellp.z<0) || (shellp.z>=mysz[2])){continue;} // out of image border
                for(int i=0; i<neuron.size(); i++)
//                for(int i=1; i<80; i++)
                {
                    //qDebug() << i;
                    // Checks whether a point is in a shell of radius 20<r<40 pixels and if intensity >40
                    if(dist_L2(shellp,locswc.at(i))>=20 && data1d_crop[V3DLONG(shellp.z*mysz[0]*mysz[1]+shellp.y*mysz[0]+shellp.x)]>=40) candidates.push_back(shellp);
                }
            }
        }
    }
    cout << endl;
    for (V3DLONG i=0; i<candidates.size(); i++)
    {
        LocationSimple candidate;
        candidate.x = candidates.at(i).x;
        candidate.y = candidates.at(i).y;
        candidate.z = candidates.at(i).z;

        candidates_m.push_back(candidate);
    }
    qDebug() << "Candidates vector size is:" << candidates.size();
    return candidates_m;
}
