
#include "get_tip_block.h"
#include <stdio.h>
#include <iostream>
#include "qlist.h"
#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
using namespace std;

void get_terminal(QString image_file, QString swc_file, QString output_dir, XYZ block_size, V3DPluginCallback2 & callback)
{
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
   //separate two situations of tip image into different folders




   QString mipoutpuut = output_format +QString("%1").arg(tipnum)+"_"+QString("%1_%2_%3").arg(tip.x).arg(tip.y).arg(tip.z)+ "_mip.tif";
   simple_saveimage_wrapper(callback,mipoutpuut.toStdString().c_str(),data1d_2D,mysz,1);
   //printf("welcome to use crop_img+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
   if(data1d_crop) {delete [] data1d_crop; data1d_crop=0;}
   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
   if(image_mip) {delete [] image_mip; image_mip=0;}
   if(label_mip) {delete [] label_mip; label_mip=0;}
   //listNeuron.clear();
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
    cout<<"Demo1:\t ./vaa3d -x ML_get_sample -f get_ML_sample -i original image & input .swcfile -o output image dir.\n";

}
