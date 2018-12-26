
#include "get_tip_block.h"
#include <stdio.h>
#include <iostream>
#include "qlist.h"
#include <algorithm>
#include <iterator>
#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define MIN_DIST 2

double marker_dist(MyMarker a, MyMarker b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}
using namespace std;

void get_terminal(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString image_file=infiles.at(0);
    QString swc_file=infiles.at(1);
    QString output_dir=outfiles.at(0);
    QString output_apo;
    XYZ block_size=XYZ(100,100,20);

    if(outfiles.size()>1)
    {
        output_apo=outfiles.at(1);
    }
    printf("welcome to use get_termial\n");
    NeuronTree nt = readSWC_file(swc_file);
    if(!output_dir.endsWith("/")){
        output_dir = output_dir+"/";
    }

    // Find tips
    QList<int> tip_list = get_tips(nt, false);
    cout<<"Number_of_tips\t"<<qPrintable(swc_file)<<"\t"<<tip_list.size()<<endl;
    int numtip=tip_list.size();
    // Crop tip-centered regions one by one
    block zcenter_block; // This is a block centered at (0,0,0)
    zcenter_block.small = 0-block_size/2;
    zcenter_block.large = block_size/2;
    QList<QString> output_suffix;
    output_suffix.append(QString("tif"));
    output_suffix.append(QString("swc"));
    printf("welcome to use get_termial\n");
    for(int i=0; i<tip_list.size(); i++){
        int tipnum=i;
        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
        qDebug()<<node.n;
        if(node.type > 5){continue;}
        // create a tip-centered block
        block crop_block = offset_block(zcenter_block, XYZ(node.x, node.y, node.z));
        crop_block.name = QString::number(i);
        XYZ tip=XYZ(node.x, node.y, node.z);
        // crop swc
        QString output_swc = output_dir+crop_block.name+".swc";
        crop_swc(swc_file, output_swc, crop_block);
        // crop image
        crop_img(image_file, crop_block, output_dir, callback, QString(".tif"),output_swc,tipnum,tip);
        //my_saveANO(output_dir, crop_block.name, output_suffix);
    }
    return;
}

void get2d_label_image(NeuronTree nt_crop_sorted,V3DLONG mysz[4],unsigned char * data1d_crop,V3DPluginCallback2 & callback,QString output_format,int tipnum,XYZ tip)
{
   V3DLONG pagesz = mysz[0]*mysz[1]*mysz[2];
   unsigned char* data1d_mask = 0;
   data1d_mask = new unsigned char [pagesz];
   memset(data1d_mask,0,pagesz*sizeof(unsigned char));
   double margin=1;//by PHC 20170531
   ComputemaskImage(nt_crop_sorted, data1d_mask, mysz[0], mysz[1], mysz[2], margin);
   //QString labelSaveString = pathname + ".v3draw_label.tif";
   //simple_saveimage_wrapper(callback, labelSaveString.toLatin1().data(),(unsigned char *)data1d_mask, mysz, 1);

   V3DLONG stacksz =mysz[0]*mysz[1];
   unsigned char *image_mip=0;
   image_mip = new unsigned char [stacksz];
   unsigned char *label_mip=0;
   label_mip = new unsigned char [stacksz];

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
   data1d_2D = new unsigned char [3*stacksz];
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

   QString mipoutpuut = output_format +QString("%1").arg(tipnum)+"_"+QString("%1_%2_%3").arg(tip.x).arg(tip.y).arg(tip.z)+ "_mip.tif";
   simple_saveimage_wrapper(callback,mipoutpuut.toStdString().c_str(),data1d_2D,mysz,1);
   if(data1d_crop) {delete [] data1d_crop; data1d_crop=0;}
   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
   if(image_mip) {delete [] image_mip; image_mip=0;}
   if(label_mip) {delete [] label_mip; label_mip=0;}
   //listNeuron.clear();
}


void get_2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
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
    cout<<"+++++++++++"<<endl;
    QStringList list=input_swc.split("/");
    QString flag=list[6]; QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
    QString flag1=list1[0];
//    QString flag=input_swc.right(input_swc.length()-43);
//    QString flag1=flag.left(flag.length()-4);
//    //printf("______________:%s\n",output_2d_dir.data());
    qDebug()<<input_image;
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
   QString mipoutpuut = output_2d_dir +flag1+".tif";
   simple_saveimage_wrapper(callback,mipoutpuut.toStdString().c_str(),(unsigned char *)data1d_2D,mysz,1);
   if(data1d_crop) {delete [] data1d_crop; data1d_crop=0;}
   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
   if(image_mip) {delete [] image_mip; image_mip=0;}
   if(label_mip) {delete [] label_mip; label_mip=0;}
   //listNeuron.clear();
}
void get_tip_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString input_swc=infiles.at(0);
    QString input_image=infiles.at(1);
    //QString input_radius=inparas.at(0);
    QString output_2d_dir=outfiles.at(0);
    if(!output_2d_dir.endsWith("/")){
        output_2d_dir = output_2d_dir+"/";
    }
    int radius=5;
    if (inparas.size()==1)
    {
        int tmp=atoi(inparas.at(0));
        if (tmp>1 && tmp<=20)
        {
            radius=tmp;
            qDebug()<<"mean shift search window radius is set to: "<<tmp;
        }
        else
            v3d_msg("The parameter of window radius is not valid, the program will use default value of 5",0);
    }

    QStringList list=input_swc.split("/");
    QString flag1=(list[6].split("."))[0];//the id in list should not be added 1 to the exact string
//    QString flag=input_swc.right(input_swc.length()-43);
//    QString flag1=flag.left(flag.length()-4);
    //printf("______________:%s\n",output_2d_dir.data());
    qDebug()<<input_swc;
    qDebug("name:%s",qPrintable(flag1));
    //1.read croped swc.file
    NeuronTree nt_crop_sorted=readSWC_file(input_swc);
    QList<NeuronSWC> nt_crop_swc=nt_crop_sorted.listNeuron;
    //2.read croped tif.file
    Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(input_image) ));
    int nChannel = p4dImage->getCDim();
    long mysz[4];
    mysz[0] = p4dImage->getXDim();
    mysz[1] = p4dImage->getYDim();
    mysz[2] = p4dImage->getZDim();
    mysz[3] = nChannel;
    cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
    unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);
    //printf("+++++++++++:%p\n",p4dImage);
    // 3.Find tip node: and make tip node to the center of tip image signal
    int tip_id = find_tip(nt_crop_sorted, mysz[0], mysz[1], mysz[2]);
    MyMarker tip,center;
    cout<<"tip id in input swc:"<<tip_id<<endl;
    vector<MyMarker> all_cube_markers;
    double ave_signal;
    tip.x=ceil(nt_crop_sorted.listNeuron.at(tip_id).x);
    tip.y=ceil(nt_crop_sorted.listNeuron.at(tip_id).y);
    tip.z=ceil(nt_crop_sorted.listNeuron.at(tip_id).z);
    cout<<"tip location x:"<<tip.x<<" y:"<<tip.y<<" z:"<<tip.z<<endl;
    //3.1 delete fake tip with weak signal(sample num. 002_10 swc. file)
    //to be implemented
    //3.2 return average signal back to nodes in cube
    //int radius=5;
    all_cube_markers=get_in_circle_nodes(tip,radius);//R=5
    cout<<"cube size(number of pixels):"<<all_cube_markers.size()<<endl;
    ave_signal=get_circle_signal(all_cube_markers,data1d_crop,mysz[0],mysz[1],mysz[2]);
    cout<<"average signal in cube:"<<ave_signal<<endl;
//    unsigned char *result;
//    long total_sz = mysz[0] * mysz[1] * mysz[2];
//    result = new unsigned char [total_sz];
//    if (!return_signal_in_circle_nodes(radius,ave_signal,mysz,data1d_crop,tip)) return;
      node_and_id max_info;
      max_info=return_signal_in_circle_nodes(radius,ave_signal,mysz,data1d_crop,tip);
      center=node_to_center(max_info,nt_crop_swc,mysz,data1d_crop);
      QList<NeuronSWC> output_swc;
      NeuronTree result_tree;
      output_swc=change_tip_xyz(nt_crop_swc,tip_id,center);
      result_tree=neuronlist_2_neurontree(output_swc);
      QString swc_name = output_2d_dir+flag1+"."+QString ("tip_reset.swc");
      writeSWC_file(swc_name,result_tree);
      qDebug()<<swc_name;
//    QString mipoutput = output_2d_dir +flag1+"."+"average.tif";
//    qDebug("number:%s",qPrintable(mipoutput));
//    simple_saveimage_wrapper(callback,mipoutput.toStdString().c_str(),data1d_crop,mysz,1);
}

double get_circle_signal(vector<MyMarker> allmarkers, unsigned char * data1d,long sz0, long sz1, long sz2)
{

    double signal = 0;
    double in_block_ct = 0;
    long sz01 = sz0 * sz1;
    long total_sz = sz0 * sz1 * sz2;

    for(int i=0; i<allmarkers.size();i++){
        int x = allmarkers[i].x;
        int y = allmarkers[i].y;
        int z = allmarkers[i].z;
        int id = z*sz01+y*sz0+x;
        if(id<total_sz){
            signal += data1d[id];
            //cout<<"this:"<<id<<" id's signal indensity:"<<data1d[id]<<endl;
            in_block_ct += 1;
        }

    }
    //cout<<"total signal:"<<signal<<endl;
    if(in_block_ct>0)
    {
        signal = signal / in_block_ct;
    }

    return signal;

}
int find_tip(NeuronTree nt, long sz0, long sz1, long sz2)
{
    // Return the node at center of the image as tip node
    MyMarker center = MyMarker((sz0-1)/2, (sz1-1)/2, (sz2-1)/2);
    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    for(int i=0; i<nt.listNeuron.size(); i++){
        MyMarker node=MyMarker(nt.listNeuron.at(i).x, nt.listNeuron.at(i).y, nt.listNeuron.at(i).z);
        if(marker_dist(center, node)<MIN_DIST & childs[i].size()==0) {
            return i;
        }
        else if(marker_dist(center, node)<MIN_DIST+1 & childs[i].size()==0) {
            return i;
        }
        else if(marker_dist(center, node)<MIN_DIST+2 & childs[i].size()==0) {
            return i;
        }
    }
    printf("No tip found!\n");
}

QList<NeuronSWC> change_tip_xyz(QList<NeuronSWC>input_swc,int tip_node,MyMarker center){

    QList<NeuronSWC> output_swc;
  for(int i=0;i<input_swc.size();i++){
    if(tip_node!=i){
    NeuronSWC s;
    s.x=input_swc.at(i).x;
    s.y=input_swc.at(i).y;
    s.z=input_swc.at(i).z;
    s.type=input_swc.at(i).type;
    s.radius=input_swc.at(i).radius;
    s.pn=input_swc.at(i).pn;
    s.n=input_swc.at(i).n;
    output_swc.append(s);}
    else{
        NeuronSWC s;
        s.x=center.x;
        s.y=center.y;
        s.z=center.z;
        s.type=input_swc.at(i).type;
        s.radius=input_swc.at(i).radius;
        s.pn=input_swc.at(i).pn;
        s.n=input_swc.at(i).n;
        output_swc.append(s);
    }
  }
  return output_swc;
}

NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist){
    NeuronTree new_tree;
    QList<NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
//    qDebug()<<"creating new neuronlist";
    for (int i = 0; i < neuronlist.size(); i++)
    {
        NeuronSWC node=neuronlist.at(i);
        NeuronSWC S;
        S.n 	= node.n;
        S.type 	= node.type;
        S.x 	= node.x;
        S.y 	= node.y;
        S.z 	= node.z;
        S.r 	= node.r;
        S.pn 	= node.pn;
        listNeuron.append(S);
        hashNeuron.insert(S.n, i);
    }

    new_tree.listNeuron = listNeuron;
    new_tree.hashNeuron = hashNeuron;
    return new_tree;
}

vector<MyMarker> get_in_circle_nodes(MyMarker center_marker, double circle_radius){
     //get nodes in cube
    vector<MyMarker> all_cube_markers;
     double length=circle_radius*2;//R=5pixels
     double total_node_num=length*length*length;
     double cube_bx=center_marker.x-circle_radius;
     double cube_by=center_marker.y-circle_radius;
     double cube_bz=center_marker.z-circle_radius;
     cout<<"start node location:"<<endl<<cube_bx<<endl<<cube_by<<endl<<cube_bz<<endl;
     for (int i=0;i<length;i++){
          MyMarker node;
          for(int j=0;j<length;j++){
               for(int k=0;k<length;k++){
                   node.x=cube_bx+i;node.y=cube_by+j;node.z=cube_bz+k;
                   //cout<<"+++++++++++++"<<i<<j<<k<<endl;
                   all_cube_markers.push_back(node);}
          }
         }
     return all_cube_markers;
}

node_and_id return_signal_in_circle_nodes(int circle_radius, double ave_signal,long mysz[4],unsigned char * data1d,MyMarker center_marker){
   //get max node x,y,z,whose indensity is more than average
    node_and_id all_roi_nodes;
    vector<MyMarker> max_list;
    unsigned char * result=0;
    double length=2*circle_radius;
    double cube_bx=center_marker.x-circle_radius;
    double cube_by=center_marker.y-circle_radius;
    double cube_bz=center_marker.z-circle_radius;
    long sz01 = mysz[0] * mysz[1];
    long total_sz = mysz[0] * mysz[1] * mysz[2];
    cout<<"input block size:"<<total_sz<<endl;
    result = new unsigned char [total_sz];
    int num=0;
    for (int i=0;i<length;i++){
          MyMarker node;
          for(int j=0;j<length;j++){
               for(int k=0;k<length;k++){
                   node.x=cube_bx+i;node.y=cube_by+j;node.z=cube_bz+k;
                   int id = node.z*sz01+node.y*mysz[0]+node.x;
                   //printf("id in image signal arr:%d\n",id);
                   //cout<<"node location:"<<endl<<node.x<<endl<<node.y<<endl<<node.z<<endl;
                   for (long cc=0;cc<total_sz;cc++){
                       if(cc!=id) result[cc]=data1d[cc];
                       else{
                            if (id<=total_sz){
                            double min=0;double max=1;
                            if(data1d[id]<=ave_signal) result[id]=min;
                            if(data1d[id]>ave_signal) {result[id]=max;num+=1;
                                all_roi_nodes.all_nodes.push_back(node);
                                all_roi_nodes.all_id.push_back(id);
                            }//get the foreground node's id,it is wroten for generating an 0-1 image firstly
                            //result[id] = (data1d[id]<=ave_signal) ? min:max;
                            //cout<<"max_id size:"<<max_list.size()<<endl;
                            }
                       }}
               }
          }
         }
    cout<<"total max num :"<<num<<endl;
    return all_roi_nodes;
}


MyMarker node_to_center(node_and_id all_max_nodes,QList<NeuronSWC> input_swc,long mysz[4],unsigned char * data1d){

    vector<MyMarker> nodes=all_max_nodes.all_nodes;
    vector<int> ids=all_max_nodes.all_id;
    node_and_id info;
//    for(int i=0;i<ids.size();i++){

//       cout<<"id:"<<ids.at(i)<<endl;
//    }
    sort(nodes.begin(),nodes.end());
    vector<int> id_numofneibs,total_signal;
    for(int i=0;i<nodes.size();i++){
        vector<int> result;
        info=get_26_neib_id(nodes.at(i),mysz,data1d);
        //cout<<"27 pixels' id(size):"<<neibs.size()<<endl;
        sort(info.all_id.begin(),info.all_id.end());
        set_intersection(ids.begin(),ids.end(),info.all_id.begin(),info.all_id.end(),back_inserter(result));
        int num_inters=result.size();
        total_signal.push_back(info.total_signal);
        //cout<<"total signal size:"<<info.total_signal<<endl;
        id_numofneibs.push_back(num_inters);
        result.clear();
    }
     //method 1. using neibs total num who is in signal to define center location
//    vector<int>::iterator max=std::max_element(id_numofneibs.begin(),id_numofneibs.end());
//    int pos=distance(id_numofneibs.begin(),max);
//    MyMarker center=nodes.at(pos);
//    cout<<"original id position:"<<pos<<endl<<"max neibs size num of all cube nodes:"<<id_numofneibs.at(pos)<<endl;
//    cout<<"x:"<<center.x<<endl<<"y:"<<center.y<<endl<<"z:"<<center.z<<endl;
//    return center;
    //method 2.using 27 pixels' total signal to define center location
    //sort(total_signal.begin(),total_signal.end());
    vector<int>::iterator max=std::max_element(total_signal.begin(),total_signal.end());
    int pos=distance(total_signal.begin(),max);
    MyMarker center=nodes.at(pos);
    cout<<"original id position:"<<pos<<endl<<"max total 27 pixel signal of all cube nodes:"<<total_signal.at(pos)<<endl;
    cout<<"x:"<<center.x<<endl<<"y:"<<center.y<<endl<<"z:"<<center.z<<endl;
    return center;
}


node_and_id get_26_neib_id(MyMarker center_marker,long mysz[4],unsigned char * data1d){

    node_and_id info_27;
    int length=3;//27 cube's length
    double cube_bx=center_marker.x-1;
    double cube_by=center_marker.y-1;
    double cube_bz=center_marker.z-1;
    double signal=0;
    long sz01 = mysz[0] * mysz[1];
    long total_sz=mysz[0] * mysz[1] * mysz[2];
    for (int i=0;i<length;i++){
        MyMarker node;
          for(int j=0;j<length;j++){
               for(int k=0;k<length;k++){
                   node.x=cube_bx+i;node.y=cube_by+j;node.z=cube_bz+k;
                   int id = node.z*sz01+node.y*mysz[0]+node.x;
                   info_27.all_id.push_back(id);
                   if(id<total_sz){
                       signal += data1d[id];
                       //cout<<"this:"<<id<<" id's signal indensity:"<<data1d[id]<<endl;
                   }
                   info_27.total_signal=signal;
               }}}
        //cout<<"27 pixels' id(size):"<<neibs_26_id.size()<<endl;
    return info_27;

}



QList<int> get_tips(NeuronTree nt, bool include_root){
    // whether a node is a tip;
    QList<int> tip_list;
    QList<int> plist;
    QList<int> alln;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        plist.append(nt.listNeuron.at(i).pn);
        alln.append(nt.listNeuron.at(i).n);
        if(include_root & nt.listNeuron.at(i).pn == -1){
            tip_list.append(i);
        }
    }
    for(int i=0; i<N; i++){
        if(plist.count(nt.listNeuron.at(i).n)==0){tip_list.append(i);}
    } 
//    //delete the fake tips(distance between tip and branch node is less than 10)
//    QVector<QVector<V3DLONG> > childs;
//    V3DLONG neuronNum = nt.listNeuron.size();
//    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
//    for (V3DLONG i=0;i<neuronNum;i++)
//    {
//        V3DLONG par = nt.listNeuron[i].pn;
//        if (par<0) continue;
//        childs[nt.hashNeuron.value(par)].push_back(i);
//    }

//    for (int i=0;i<tip_list.size();i++)
//    {

//        int step=tip_list.at(i);
//        while (childs[step].size()<2)
//              {
//                 int stepn=nt.listNeuron.at(step).pn;//stepn is the n but not the i
//                 if (stepn!=-1) step=alln.indexOf(stepn);
//                 else break;
//                 cout<<"the numer of index:"<<step<<endl;
//              }
//        double dis=dist(nt.listNeuron.at(tip_list.at(i)),nt.listNeuron.at(step));
//        if(dis<10) tip_list.removeOne(i);
//        else continue;
//    }
//   cout<<"the number of tips is(after deleting fake tips):"<<tip_list.size()<<endl;
    return(tip_list);
}

XYZ offset_XYZ(XYZ input, XYZ offset){
    input.x += offset.x;
    input.y += offset.y;
    input.z += offset.z;
    return input;
}

block offset_block(block input_block, XYZ offset)
{
    input_block.small = offset_XYZ(input_block.small, offset);
    input_block.large = offset_XYZ(input_block.large, offset);
    return input_block;
}

void crop_img(QString image, block crop_block, QString outputdir_img, V3DPluginCallback2 & callback, QString output_format,QString input_swc,int tipnum,XYZ tip)
{
    printf("welcome to use crop_img\n");
    if(output_format.size()==0){output_format=QString(".tiff");}

    V3DLONG *in_zz;
    if(!callback.getDimTeraFly(image.toStdString(), in_zz))
    {
        v3d_msg("Cannot load terafly images.",0);
        return;
    }
    // 1. When cropping, ranges must be integers
    // pixels at large values won't be included, so set large.x/y/z as large.x/y/z + 1
    XYZ small=XYZ(crop_block.small);
    XYZ large=XYZ(crop_block.large);
    small.x = floor(small.x);
    small.y = floor(small.y);
    small.z = floor(small.z);
    large.x = ceil(large.x)+1;
    large.y = ceil(large.y)+1;
    large.z = ceil(large.z)+1;

    // 2. Crop image. image is stored as 1d array. 2 parameters needed for cropping:
    // 2.1. 'cropped_image' is a pointer to the beginning of the region of interest
    unsigned char * cropped_image = 0;
    qDebug()<<small.x<<small.y<<small.z<<large.x<<large.y<<large.z;
    cropped_image = callback.getSubVolumeTeraFly(image.toStdString(),
                                                 small.x, large.x,
                                                 small.y, large.y,
                                                 small.z, large.z);
    // 2.2. 'in_sz' sets the size of the region.
    V3DLONG in_sz[4];
    in_sz[0] = large.x-small.x;
    in_sz[1] = large.y-small.y;
    in_sz[2] = large.z-small.z;
    in_sz[3] = in_zz[3];   // channel information

    // 3. Save image
    QString saveName = outputdir_img + "/" + crop_block.name + output_format;
    const char* fileName = saveName.toAscii();
    simple_saveimage_wrapper(callback, fileName, cropped_image, in_sz, 1);

    NeuronTree nt_crop_sorted;
    nt_crop_sorted=readSWC_file(input_swc);
    get2d_label_image(nt_crop_sorted,in_sz,cropped_image,callback,outputdir_img,tipnum,tip);

    return;
}

void crop_swc(QString input_swc, QString output_swc, block crop_block)
{

    printf("welcome to use crop_swc\n");
    QString cmd = "vaa3d -x preprocess -f crop_swc_cuboid -i " + input_swc + " -o " +output_swc  + " -p "
             + "\""
             + "#a " + QString::number(crop_block.small.x)
             + " #b " + QString::number(crop_block.small.y)
             + " #c " + QString::number(crop_block.small.z)
             + " #d " + QString::number(crop_block.large.x)
             + " #e " + QString::number(crop_block.large.y)
             + " #f " + QString::number(crop_block.large.z)
             + " #r " + QString::number(crop_block.small.x)
             + " #s " + QString::number(crop_block.small.y)
             + " #t " + QString::number(crop_block.small.z)
             + "\"";
    qDebug()<<cmd;
    system(qPrintable(cmd));
    return;
}

bool my_saveANO(QString ano_dir, QString fileNameHeader, QList<QString> suffix)
{
    if(!ano_dir.endsWith('/')){ano_dir = ano_dir+'/';}
    FILE * fp=0;
    fp = fopen((char *)qPrintable(ano_dir+fileNameHeader+QString(".ano")), "wt");
    if (!fp)
    {
        v3d_msg("Fail to open file to write.");
        return false;
    }
    for(int i=0; i<suffix.size(); i++)
    {
        QString tail = suffix.at(i);
        if(!tail.startsWith('.')){tail = '.'+tail;}
        QString header;
        if(tail.endsWith("SWC") || tail.endsWith("swc")){header = "SWCFILE=";}
        else if(tail.endsWith("APO") || tail.endsWith("apo")){header = "APOFILE=";}
        else{header = "RAWIMG=";}
        fprintf(fp, qPrintable(header+fileNameHeader+tail));
        fprintf(fp, "\n");
    }
    if(fp){fclose(fp);}
    return true;
}

void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"This plugin for cropping block of tips"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_ML_sample\n";
    cout<<"-i<file name>:\t\t input .tif file\n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f get_ML_sample -i original image input .swc file -o output image dir.\n";

}
void printHelp1(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    //2.
    cout<<"This plugin for generet 2D images"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_2D_sample\n";
    cout<<"-i<file name>:\t\t input .swc and .tiff file\n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f get_2D_sample -i original swc -p input .image -o output image dir.\n";
    //3.
    cout<<"This plugin for getting image signal in tip"<<endl;
    cout<<"usage:\n";
    cout<<"-f<func name>:\t\t get_ML_sample\n";
    cout<<"-i<file name>:\t\t input .swc file\n";
    cout<<"-p<file name>:\t\t input image file(tif,nrrd,v3draw)\n";
    cout<<"-o<file name>:\t\t ouput dir\n";
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f get_tip_sample -i <original swc> <input .image> -p <radius> -o <output swc.file dir>\n";

}
