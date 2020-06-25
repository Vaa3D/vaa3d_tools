#include "imageprocess.h"

#include <math.h>

#include <iostream>

#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"


void enhanceImage(unsigned char* data1d, V3DLONG* sz){
    V3DLONG totalSz = sz[0]*sz[1]*sz[2];

    for(int i=0; i<totalSz; i++){
        data1d[i] = sqrt(data1d[i]/255.0)*255;
    }
}

void get_2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString input_swc=infiles.at(0);
    QString input_image=inparas.at(0);
//    QString output_2d_dir=outfiles.at(0);
//    if(!output_2d_dir.endsWith("/")){
//        output_2d_dir = output_2d_dir+"/";
//    }
    cout<<"+++++++++++"<<endl;
    QStringList list=input_swc.split(".");
    list.pop_back();
    QString mipoutput = list.join(".") + "_2d.tif";
//    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
//    //QString flag1=list1.first();

//    QStringList tif_name;
//    for(int k=0;k<4;k++) tif_name.append(flag.split("_")[k]);
//    QString flag1=tif_name.join("_");
//    QString flag=input_swc.right(input_swc.length()-43);
//    QString flag1=flag.left(flag.length()-4);
//    //printf("______________:%s\n",output_2d_dir.data());
    qDebug()<<input_image;
    qDebug()<<input_swc;
    qDebug("number:%s",qPrintable(mipoutput));
    NeuronTree nt_crop_sorted=readSWC_file(input_swc);
//    NeuronTree nt_crop_sorted=revise_radius(nt_crop_sorted1,1);

    qDebug()<<"start to load img";

//    Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(input_image) ));


//    int nChannel = p4dImage->getCDim();

    V3DLONG mysz[4] = {0,0,0,0};
    unsigned char *data1d_crop = 0;
    int datatype = 1;
    simple_loadimage_wrapper(callback,input_image.toStdString().c_str(),data1d_crop,mysz,datatype);
//    mysz[0] = p4dImage->getXDim();
//    mysz[1] = p4dImage->getYDim();
//    mysz[2] = p4dImage->getZDim();
//    mysz[3] = nChannel;
//    cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
//    unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);

    qDebug()<<"-----------end------------";

    enhanceImage(data1d_crop,mysz);
    //printf("+++++++++++:%p\n",p4dImage);

   V3DLONG pagesz = mysz[0]*mysz[1]*mysz[2];
   unsigned char* data1d_mask = 0;
   data1d_mask = new unsigned char [pagesz];
   memset(data1d_mask,0,pagesz*sizeof(unsigned char));
   double margin=0;//by PHC 20170531
   QList<int> mark_others;
   ComputemaskImage(nt_crop_sorted, data1d_mask, mysz[0], mysz[1], mysz[2],margin, mark_others,false);
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
//   QString mipoutput = output_2d_dir +"result"+".tiff";
   simple_saveimage_wrapper(callback,mipoutput.toStdString().c_str(),(unsigned char *)data1d_2D,mysz,1);
   if(data1d_crop) {delete [] data1d_crop; data1d_crop=0;}
   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
   if(image_mip) {delete [] image_mip; image_mip=0;}
   if(label_mip) {delete [] label_mip; label_mip=0;}
   //listNeuron.clear();
}
