#include "imageprocess.h"

#include <math.h>

#include <iostream>

#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"

//#include <thread>
#include <vector>



//void convertDataThread::run(int i, int j){
//    m[i] = j;
//}

void enhanceImage(unsigned char* data1d, V3DLONG* sz){
    V3DLONG totalSz = sz[0]*sz[1]*sz[2];

    for(int i=0; i<totalSz; i++){
        data1d[i] = pow(data1d[i]/255.0,2/3.0)*255;
    }
}

void get_2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QStringList input_swcs;
//    QString input_swc=infiles.at(0);
    for(int i=0 ;i<infiles.size(); ++i){
        QString input_swc = infiles.at(i);
        input_swcs.append(input_swc);
    }

    QString input_image=inparas.at(0);
//    QString output_2d_dir=outfiles.at(0);
//    if(!output_2d_dir.endsWith("/")){
//        output_2d_dir = output_2d_dir+"/";
//    }
    cout<<"+++++++++++"<<endl;
//    QStringList list=input_swc.split(".");
    QStringList list=input_image.split(".");
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
//    qDebug()<<input_swc;
    qDebug("number:%s",qPrintable(mipoutput));
//    NeuronTree nt_crop_sorted=readSWC_file(input_swc);

    vector<NeuronTree> nts_crop_sorted;

    for(int i=0; i<input_swcs.size(); ++i){
        NeuronTree nt_crop_sorted=readSWC_file(input_swcs[i]);
        for(int i=0; i<nt_crop_sorted.listNeuron.size(); i++){
            nt_crop_sorted.listNeuron[i].r = 1;
        }
        nts_crop_sorted.push_back(nt_crop_sorted);
    }

//    for(int i=0; i<nt_crop_sorted.listNeuron.size(); i++){
//        nt_crop_sorted.listNeuron[i].r = 1;
//    }
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

   vector<unsigned char*> data1d_masks;

   for(int i =0 ; i<nts_crop_sorted.size(); ++i){
       qDebug()<<"i: "<<i<<" mask image";
       unsigned char* data1d_mask = 0;
       data1d_mask = new unsigned char [pagesz];
       memset(data1d_mask,0,pagesz*sizeof(unsigned char));
       double margin=0;//by PHC 20170531
       QList<int> mark_others;
       ComputemaskImage(nts_crop_sorted[i], data1d_mask, mysz[0], mysz[1], mysz[2],margin, mark_others,false);

       data1d_masks.push_back(data1d_mask);
   }


   //QString labelSaveString = pathname + ".v3draw_label.tif";
   //simple_saveimage_wrapper(callback, labelSaveString.toLatin1().data(),(unsigned char *)data1d_mask, mysz, 1);

   V3DLONG stacksz =mysz[0]*mysz[1];
   unsigned char *image_mip=0;
   image_mip = new unsigned char [stacksz];//2D orignal image

   vector<unsigned char*> labels_mip;
   for(int i=0; i<data1d_masks.size(); ++i){
       unsigned char *label_mip=0;
       label_mip = new unsigned char [stacksz];//2D annotation
       labels_mip.push_back(label_mip);
   }


   for(V3DLONG iy = 0; iy < mysz[1]; iy++)
   {
       V3DLONG offsetj = iy*mysz[0];
       for(V3DLONG ix = 0; ix < mysz[0]; ix++)
       {
           int max_mip = 0;
           vector<int> max_labels;
           for(int i=0; i<labels_mip.size(); i++){
               int max_label = 0;
               max_labels.push_back(max_label);
           }

           for(V3DLONG iz = 0; iz < mysz[2]; iz++)
           {
               V3DLONG offsetk = iz*mysz[1]*mysz[0];
               if(data1d_crop[offsetk + offsetj + ix] >= max_mip)
               {
                   image_mip[iy*mysz[0] + ix] = data1d_crop[offsetk + offsetj + ix];
                   max_mip = data1d_crop[offsetk + offsetj + ix];
               }
               for(int i=0; i<labels_mip.size(); ++i){
                   if(data1d_masks[i][offsetk + offsetj + ix] >= max_labels[i])
                   {
                       labels_mip[i][iy*mysz[0] + ix] = data1d_masks[i][offsetk + offsetj + ix];
                       max_labels[i] = data1d_masks[i][offsetk + offsetj + ix];
                   }
               }

           }
       }
   }

   qDebug()<<"label end";

   unsigned char* data1d_2D = 0;
   data1d_2D = new unsigned char [3*stacksz*2];//3 channels image, 2 block
//channel 1
   for(V3DLONG i=0; i<mysz[0]; i++){
       for(V3DLONG j=0; j<mysz[1]; j++){
           int index = j*mysz[0]*2 + i;
           int mipIndex = j*mysz[0] + i;
           data1d_2D[index] = image_mip[mipIndex];
       }
   }
   for(V3DLONG i=mysz[0]; i<mysz[0]*2; i++){
       for(V3DLONG j=0; j<mysz[1]; j++){
           int index = j*mysz[0]*2 + i;
           int indexChannel0 = j*mysz[0]*2 + i;
           int indexChannel1 = j*mysz[0]*2 + i + stacksz*2;
           int indexChannel2 = j*mysz[0]*2 + i + stacksz*4;

           int mipIndex = j*mysz[0] + i - mysz[0];
           if(labels_mip.size()>0){
               data1d_2D[indexChannel0] = (labels_mip[0][mipIndex] == 255) ? 255 : image_mip[mipIndex];
               data1d_2D[indexChannel1] = (labels_mip[0][mipIndex] == 255) ? 0 : image_mip[mipIndex];
               data1d_2D[indexChannel2] = (labels_mip[0][mipIndex] == 255) ? 0 : image_mip[mipIndex];
           }else{
               data1d_2D[index] = image_mip[mipIndex];
           }

       }
   }
//channel 2
   for(V3DLONG i=0; i<mysz[0]; i++){
       for(V3DLONG j=0; j<mysz[1]; j++){
           int index = j*mysz[0]*2 + i + stacksz*2;
           int mipIndex = j*mysz[0] + i;
           data1d_2D[index] = image_mip[mipIndex];
       }
   }
   for(V3DLONG i=mysz[0]; i<mysz[0]*2; i++){
       for(V3DLONG j=0; j<mysz[1]; j++){
//           int index = j*mysz[0]*2 + i+ stacksz*2;
           int indexChannel0 = j*mysz[0]*2 + i;
           int indexChannel1 = j*mysz[0]*2 + i + stacksz*2;
           int indexChannel2 = j*mysz[0]*2 + i + stacksz*4;

           int mipIndex = j*mysz[0] + i - mysz[0];
           if(labels_mip.size()>1){
               if(labels_mip[1][mipIndex] == 255){
                   data1d_2D[indexChannel0] = 0;
                   data1d_2D[indexChannel1] = 255;
                   data1d_2D[indexChannel2] = 0;
               }else{
                   data1d_2D[indexChannel0] = (labels_mip[0][mipIndex] == 255) ? data1d_2D[indexChannel0] : image_mip[mipIndex];
                   data1d_2D[indexChannel1] = (labels_mip[0][mipIndex] == 255) ? data1d_2D[indexChannel1] : image_mip[mipIndex];
                   data1d_2D[indexChannel2] = (labels_mip[0][mipIndex] == 255) ? data1d_2D[indexChannel2] : image_mip[mipIndex];
               }

           }

       }
   }
//channel 3
   for(V3DLONG i=0; i<mysz[0]; i++){
       for(V3DLONG j=0; j<mysz[1]; j++){
           int index = j*mysz[0]*2 + i + stacksz*4;
           int mipIndex = j*mysz[0] + i;
           data1d_2D[index] = image_mip[mipIndex];
       }
   }
   for(V3DLONG i=mysz[0]; i<mysz[0]*2; i++){
       for(V3DLONG j=0; j<mysz[1]; j++){
//           int index = j*mysz[0]*2 + i + stacksz*4;
           int indexChannel0 = j*mysz[0]*2 + i;
           int indexChannel1 = j*mysz[0]*2 + i + stacksz*2;
           int indexChannel2 = j*mysz[0]*2 + i + stacksz*4;

           int mipIndex = j*mysz[0] + i - mysz[0];
           if(labels_mip.size()>2){
               if(labels_mip[2][mipIndex] == 255){
                   data1d_2D[indexChannel0] = 0;
                   data1d_2D[indexChannel1] = 0;
                   data1d_2D[indexChannel2] = 255;
               }else{
                   if(labels_mip[1][mipIndex] == 255 || labels_mip[0][mipIndex] == 255){
                       data1d_2D[indexChannel0] = data1d_2D[indexChannel0];
                       data1d_2D[indexChannel1] = data1d_2D[indexChannel1];
                       data1d_2D[indexChannel2] = data1d_2D[indexChannel2];
                   }else{
                       data1d_2D[indexChannel0] = image_mip[mipIndex];
                       data1d_2D[indexChannel1] = image_mip[mipIndex];
                       data1d_2D[indexChannel2] = image_mip[mipIndex];
                   }

               }
           }
       }
   }

   qDebug()<<"MIP end";

//   for(V3DLONG i=0; i<stacksz; i++)
//       data1d_2D[i] = image_mip[i];

//   for(V3DLONG i=0; i<stacksz; i++)
//   {
//       data1d_2D[i+stacksz] = (label_mip[i] ==255) ? 255: image_mip[i];
//   }
//   for(V3DLONG i=0; i<stacksz; i++)
//       data1d_2D[i+2*stacksz] = image_mip[i];

   mysz[0] = mysz[0]*2;
   mysz[2] = 1;
   mysz[3] = 3;
//   QString mipoutput = output_2d_dir +"result"+".tiff";
   simple_saveimage_wrapper(callback,mipoutput.toStdString().c_str(),(unsigned char *)data1d_2D,mysz,1);
   if(data1d_crop) {delete [] data1d_crop; data1d_crop=0;}
//   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   for(int i=0; i<data1d_masks.size(); i++){
       unsigned char* data1d_mask = data1d_masks[i];
       if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   }
   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
   if(image_mip) {delete [] image_mip; image_mip=0;}
//   if(label_mip) {delete [] label_mip; label_mip=0;}

   for(int i=0; i<labels_mip.size(); i++){
       unsigned char* label_mip = labels_mip[i];
       if(label_mip) {delete [] label_mip; label_mip=0;}
   }

   //listNeuron.clear();
}

void get_2d_image2(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QStringList input_swcs;

    for(int i=0 ;i<infiles.size(); ++i){
        QString input_swc = infiles.at(i);
        input_swcs.append(input_swc);
    }

    QString input_image = inparas.at(0);

    int lineWidth = 1;
    if (inparas.size() >= 2) {
        lineWidth = atof(inparas.at(1));
    }

    cout<<"+++++++++++"<<endl;
//    QStringList list=input_swc.split(".");
    QStringList list=input_image.split(".");
    list.pop_back();
    QString mipoutput = list.join(".") + "_2d.tif";

    if(outfiles.size() >= 1) {
        mipoutput = outfiles[0];
    }

    qDebug()<<input_image;

    qDebug("number:%s",qPrintable(mipoutput));

    vector<NeuronTree> nts_crop_sorted;

    for(int i=0; i<input_swcs.size(); ++i){
        NeuronTree nt_crop_sorted=readSWC_file(input_swcs[i]);
        for(int i=0; i<nt_crop_sorted.listNeuron.size(); i++){
            nt_crop_sorted.listNeuron[i].r = lineWidth;
        }
        nts_crop_sorted.push_back(nt_crop_sorted);
    }

    qDebug()<<"start to load img";

    V3DLONG mysz[4] = {0,0,0,0};
    unsigned char *data1d_crop = 0;
    int datatype = 1;
    simple_loadimage_wrapper(callback,input_image.toStdString().c_str(),data1d_crop,mysz,datatype);

    qDebug()<<"-----------end------------";

    enhanceImage(data1d_crop,mysz);

   V3DLONG pagesz = mysz[0]*mysz[1]*mysz[2];

   vector<unsigned char*> data1d_masks;

   for(int i =0 ; i<nts_crop_sorted.size(); ++i){
       qDebug()<<"i: "<<i<<" mask image";
       unsigned char* data1d_mask = 0;
       data1d_mask = new unsigned char [pagesz];
       memset(data1d_mask,0,pagesz*sizeof(unsigned char));
       double margin=0;//by PHC 20170531
       QList<int> mark_others;
       ComputemaskImage(nts_crop_sorted[i], data1d_mask, mysz[0], mysz[1], mysz[2],margin, mark_others,false);

       data1d_masks.push_back(data1d_mask);
   }


   //QString labelSaveString = pathname + ".v3draw_label.tif";
   //simple_saveimage_wrapper(callback, labelSaveString.toLatin1().data(),(unsigned char *)data1d_mask, mysz, 1);

   V3DLONG stacksz =mysz[0]*mysz[1];
   unsigned char *image_mip=0;
   image_mip = new unsigned char [stacksz];//2D orignal image

   vector<unsigned char*> labels_mip;
   for(int i=0; i<data1d_masks.size(); ++i){
       unsigned char *label_mip=0;
       label_mip = new unsigned char [stacksz];//2D annotation
       labels_mip.push_back(label_mip);
   }


   for(V3DLONG iy = 0; iy < mysz[1]; iy++)
   {
       V3DLONG offsetj = iy*mysz[0];
       for(V3DLONG ix = 0; ix < mysz[0]; ix++)
       {
           int max_mip = 0;
           vector<int> max_labels;
           for(int i=0; i<labels_mip.size(); i++){
               int max_label = 0;
               max_labels.push_back(max_label);
           }

           for(V3DLONG iz = 0; iz < mysz[2]; iz++)
           {
               V3DLONG offsetk = iz*mysz[1]*mysz[0];
               if(data1d_crop[offsetk + offsetj + ix] >= max_mip)
               {
                   image_mip[iy*mysz[0] + ix] = data1d_crop[offsetk + offsetj + ix];
                   max_mip = data1d_crop[offsetk + offsetj + ix];
               }
               for(int i=0; i<labels_mip.size(); ++i){
                   if(data1d_masks[i][offsetk + offsetj + ix] >= max_labels[i])
                   {
                       labels_mip[i][iy*mysz[0] + ix] = data1d_masks[i][offsetk + offsetj + ix];
                       max_labels[i] = data1d_masks[i][offsetk + offsetj + ix];
                   }
               }

           }
       }
   }

   qDebug()<<"label end";

   unsigned char* data1d_2D = 0;
   int count = labels_mip.size() + 1;
   data1d_2D = new unsigned char [3*stacksz*count];//3 channels image, count block

   for(int c=0; c<3; ++c){
       for(int i=0; i<mysz[0]; ++i){
           for(int j=0; j<mysz[1]; ++j){
               int index = j*mysz[0]*count + i + stacksz*count*c;
               int mipIndex = j*mysz[0] + i;
               data1d_2D[index] = image_mip[mipIndex];
           }
       }
   }
   static unsigned char colortable[][3]={
   {255,   0,    0},
   {  0, 255,    0},
   {40, 210,  240},
   {255, 255,    0},
   {255,   0,  255},
   {255, 128,    0},
   {  0, 255,  128},
   {128,   0,  255},
   {128, 255,    0},
   {  0, 128,  255},
   {255,   0,  128},
   {128,   0,    0},
   {  0, 128,    0},
   {  0,   0,  128},
   {128, 128,    0},
   {  0, 128,  128},
   {128,   0,  128},
   {255, 128,  128},
   {128, 255,  128},
   {128, 128,  255},
   };

   for(int s=0; s<labels_mip.size(); ++s){
       for(int c=0; c<3; ++c){
           for(int i=0; i<mysz[0]; ++i){
               for(int j=0; j<mysz[1]; ++j){
                   int index = j*mysz[0]*count + (s+1)*mysz[0] + i + stacksz*count*c;
                   int mipIndex = j*mysz[0] + i;
                   data1d_2D[index] = (labels_mip[s][mipIndex] == 255) ? colortable[s][c] : image_mip[mipIndex];
               }
           }
       }
   }

   qDebug()<<"MIP end";

   mysz[0] = mysz[0]*count;
   mysz[2] = 1;
   mysz[3] = 3;
//   QString mipoutput = output_2d_dir +"result"+".tiff";
   simple_saveimage_wrapper(callback,mipoutput.toStdString().c_str(),(unsigned char *)data1d_2D,mysz,1);
   if(data1d_crop) {delete [] data1d_crop; data1d_crop=0;}
//   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   for(int i=0; i<data1d_masks.size(); i++){
       unsigned char* data1d_mask = data1d_masks[i];
       if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   }
   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
   if(image_mip) {delete [] image_mip; image_mip=0;}
//   if(label_mip) {delete [] label_mip; label_mip=0;}

   for(int i=0; i<labels_mip.size(); i++){
       unsigned char* label_mip = labels_mip[i];
       if(label_mip) {delete [] label_mip; label_mip=0;}
   }

   //listNeuron.clear();
}

void getSwcL0Image(QString swcPath, QString brainPath, QString outPath, int times, V3DPluginCallback2 & callback){

    NeuronTree nt = readSWC_file(swcPath);

//    V3DLONG* sz = 0;
//    callback.getDimTeraFly(brainPath.toStdString(),sz);

//    qDebug()<<"sz0: "<<sz[0]<<" sz1: "<<sz[1]<<" sz2: "<<sz[2];

    size_t minX = INT_MAX, minY = INT_MAX, minZ = INT_MAX;
    size_t maxX = 0, maxY = 0, maxZ = 0;
    for(int i=0; i<nt.listNeuron.size(); i++){
        NeuronSWC s = nt.listNeuron[i];
        if(minX>s.x) minX = s.x;
        if(maxX<s.x) maxX = s.x;
        if(minY>s.y) minY = s.y;
        if(maxY<s.y) maxY = s.y;
        if(minZ>s.z) minZ = s.z;
        if(maxZ<s.z) maxZ = s.z;
    }

    minX -= 20, minY -= 20, minZ -= 10;
    maxX += 20, maxY += 20, maxZ += 10;
    if(minX<0) minX = 0;
    if(minY<0) minY = 0;
    if(minZ<0) minZ = 0;

    minX /= times, minY /= times, minZ /= times;
    minX /= times, minY /= times, minZ /= times;

//    if(maxX>sz[0]-1) maxX = sz[0]-1;
//    if(maxY>sz[1]-1) maxY = sz[1]-1;
//    if(maxZ>sz[2]-1) maxZ = sz[2]-1;

    std::cout<<QString::number(minX).toStdString().c_str()<<" "<<QString::number(maxX).toStdString().c_str()<<" "
           <<QString::number(minY).toStdString().c_str()<<" "<<QString::number(maxY).toStdString().c_str()<<" "
          <<QString::number(minZ).toStdString().c_str()<<" "<<QString::number(maxZ).toStdString().c_str();

    minX = 1024, maxX = 1225, minY = 1024, maxY = 1225, minZ = 255, maxZ = 512;

    unsigned char* pdata = 0;
    pdata = callback.getSubVolumeTeraFly(brainPath.toStdString(),minX,maxX,minY,maxY,minZ,maxZ);
    if(pdata == NULL){
        qDebug()<<"pdata is null";
    }else{
        qDebug()<<"pdata is not null";
    }

    qDebug()<<"-----------got pdata--------- ";

    V3DLONG outSZ[4] = {maxX-minX+1,maxY-minY+1,maxZ-minZ+1,1};

    qDebug()<<outSZ[0]<<" "<<outSZ[1]<<" "<<outSZ[2];

    int dataType = 1;

    V3DLONG tolSZ = outSZ[0]*outSZ[1]*outSZ[2];
    for(int i=0 ;i<tolSZ;){
        std::cout<<(int)pdata[i]<<" ";
        i += 1000;
    }

    Image4DSimple* img = new Image4DSimple();
    img->setData(pdata,outSZ[0],outSZ[1],outSZ[2],outSZ[3],V3D_UINT8);
    bool a = callback.saveImage(img,(char*)outPath.toStdString().c_str());
    qDebug()<<"a: "<<a;

    if(img){
        delete img;
        img = 0;
    }
//    if(sz){
//        delete[] sz;
//        sz = 0;
//    }

}

void convertData(QString path, V3DPluginCallback2& callback){
    QFileInfoList dirList1 = QDir(path).entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
    for(int i=0; i<dirList1.size(); i++){
        qDebug()<<"dir1: "<<dirList1[i].absoluteFilePath();
        QDir dir1 = QDir(dirList1[i].absoluteFilePath());
        QFileInfoList dirList2 = dir1.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
        for(int j=0; j<dirList2.size(); j++){
            qDebug()<<"dir2: "<<dirList2[j].absoluteFilePath();
            QDir dir2 = QDir(dirList2[j].absoluteFilePath());
            QFileInfoList dirList3 = dir2.entryInfoList(QDir::Files);
            for(int k=0; k<dirList3.size(); k++){
                qDebug()<<"file: "<<dirList3[k].absoluteFilePath();
                QString file = dirList3[k].absoluteFilePath();
                _convertData(file,callback);
            }
//            break;
        }
//        break;
    }

}
void downSampleData(QString path, int times, V3DPluginCallback2 &callback){
    QFileInfoList dirList1 = QDir(path).entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
    for(int i=0; i<dirList1.size(); i++){
        qDebug()<<"dir1: "<<dirList1[i].absoluteFilePath();
        QDir dir1 = QDir(dirList1[i].absoluteFilePath());
        QFileInfoList dirList2 = dir1.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
        for(int j=0; j<dirList2.size(); j++){
            qDebug()<<"dir2: "<<dirList2[j].absoluteFilePath();
            QDir dir2 = QDir(dirList2[j].absoluteFilePath());
            QFileInfoList fileList = dir2.entryInfoList(QDir::Files);
            for(int k=0; k<fileList.size(); k++){
                qDebug()<<"file: "<<fileList[k].absoluteFilePath();
                QString file = fileList[k].absoluteFilePath();
                _downSampleData(file,times,callback);
//                break;
            }
//            break;
        }
//        break;
    }
}

void _downSampleData(QString tifPath,int times,V3DPluginCallback2 &callback){
    unsigned char* pdata = 0;
    V3DLONG sz[4] = {0,0,0,0};
    int dataType = 0;
    simple_loadimage_wrapper(callback,tifPath.toStdString().c_str(),pdata,sz,dataType);
    qDebug()<<"sz0: "<<sz[0]<<" sz1: "<<sz[1]<<" sz2: "<<sz[2]<<" sz3: "<<sz[3];
    qDebug()<<"dataType: "<<dataType;
    resample3dimg_interp(pdata,sz,times,times,times,1);
//    downsample3dimg_1dpt(pdata,sz,times);
    simple_saveimage_wrapper(callback,tifPath.toStdString().c_str(),pdata,sz,1);
    if(pdata){
        delete[] pdata;
        pdata = 0;
    }
}

void _convertData(QString tifPath, V3DPluginCallback2 &callback){
    unsigned char* pdata = 0;
    V3DLONG sz[4] = {0,0,0,0};
    int dataType = 0;
    simple_loadimage_wrapper(callback,tifPath.toStdString().c_str(),pdata,sz,dataType);
    qDebug()<<"sz0: "<<sz[0]<<" sz1: "<<sz[1]<<" sz2: "<<sz[2]<<" sz3: "<<sz[3];
    qDebug()<<"dataType: "<<dataType;
    if(dataType == 1){
        if(pdata){
            delete[] pdata;
            pdata = 0;
        }
        return;
    }
    unsigned short* sdata = (unsigned short*)pdata;
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
//                    for(int q=0; q<tolSZ;){
//                        std::cout<<sdata[q]<<" ";
//                        q+=1000;
//                    }
    unsigned char* data = new unsigned char[tolSZ];
    for(int i=0; i<tolSZ; i++){
        int t = ((double)sdata[i]/4080.0)*255;
        if(t>255)
            t = 255;
        if(t<0)
            t = 0;
        data[i] = t;
    }
    simple_saveimage_wrapper(callback,tifPath.toStdString().c_str(),data,sz,1);
    if(data){
        delete[] data;
        data = 0;
    }
    if(pdata){
        delete[] pdata;
        pdata = 0;
    }
}

void joinImage(QString tifDir, QString outPath,int times, V3DPluginCallback2& callback){
//    int gx = 0, gy = 0, gz = 0;
//    vector<vector<int> > oris = vector<vector<int> >();
    qDebug()<<"-------in joinImage---------";

    int gx = tifDir.split("x")[1].toInt();
    int gy = tifDir.split("x")[0].split("(")[1].toInt();
    int gz = tifDir.split("x")[2].split(")")[0].toInt();



    int t = 16/times;
    qDebug()<<"t: "<<t;
    gx /= t, gy /= t, gz /= t;

//    gx += 1, gy += 1, gz += 1;

    unsigned int tolSZ = gx*gy*gz;

    qDebug()<<"tolSZ: "<<tolSZ<<" gx: "<<gx<<" gy: "<<gy<<" gz: "<<gz;


    qDebug()<<"start to allocate";
    unsigned char* data = new unsigned char[tolSZ];
    if(!data){
        qDebug()<<"allocate memory failed";
    }else {
        qDebug()<<"allocate memory successed";
    }

    memset(data,0,tolSZ*sizeof(unsigned char));




    QFileInfoList dirList1 = QDir(tifDir).entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
    for(int i=0; i<dirList1.size(); i++){
//        qDebug()<<"dir1: "<<dirList1[i].absoluteFilePath();
        QDir dir1 = QDir(dirList1[i].absoluteFilePath());
        QFileInfoList dirList2 = dir1.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
        for(int j=0; j<dirList2.size(); j++){
//            qDebug()<<"dir2: "<<dirList2[j].absoluteFilePath();
            QDir dir2 = QDir(dirList2[j].absoluteFilePath());
            QFileInfoList fileList = dir2.entryInfoList(QDir::Files);
            for(int k=0; k<fileList.size(); k++){
//                qDebug()<<"file: "<<fileList[k].absoluteFilePath();
                QString file = fileList[k].absoluteFilePath();

                QString fileName = fileList[k].baseName();
//                qDebug()<<"fileName: "<<fileName;

                int ox = fileName.split("_")[1].toInt();
                int oy = fileName.split("_")[0].toInt();
                int oz = fileName.split("_")[2].toInt();

                ox /= 160;
                oy /= 160;
                oz /= 160;

                qDebug()<<"ox: "<<ox<<" oy: "<<oy<<" oz: "<<oz;

                unsigned char* pdata = 0;
                V3DLONG sz[4] = {0,0,0,0};
                int dataType = 1;
                simple_loadimage_wrapper(callback,file.toStdString().c_str(),pdata,sz,dataType);

                for(int z=0; z<sz[2]; z++){
                    for(int y=0; y<sz[1]; y++){
                        for(int x=0; x<sz[0]; x++){
                            int pIndex = z*sz[0]*sz[1] + y*sz[0] + x;
                            int dz = (z+oz)>(gz-1)? gz-1 : z+oz;
                            int dy = (y+oy)>(gy-1)? gy-1 : y+oy;
                            int dx = (x+ox)>(gx-1)? gx-1 : x+ox;
                            unsigned int index = dz*gx*gy + dy*gx + dx;
//                            qDebug()<<"index: "<<index;
                            data[index] = pdata[pIndex];
//                            if(data[index]>20){
//                                qDebug()<<index<<" : "<<(int)data[index];
//                            }

                        }
                    }
                }

                if(pdata){
                    delete[] pdata;
                    pdata = 0;
                }
            }
        }
    }


    V3DLONG outSZ[4];
    outSZ[0] = gx;
    outSZ[1] = gy;
    outSZ[2] = gz;
    outSZ[3] = 1;
    bool a = simple_saveimage_wrapper(callback,outPath.toStdString().c_str(),data,outSZ,1);
    qDebug()<<"a: "<<a;
    qDebug()<<"save image end";

    if(data){
        qDebug()<<"delete data";
        delete[] data;
        data = 0;
    }

}


void joinImage2(QString tifDir, QString outPath,int resolution, V3DPluginCallback2& callback){
//    int gx = 0, gy = 0, gz = 0;
//    vector<vector<int> > oris = vector<vector<int> >();
    qDebug()<<"-------in joinImage---------";

    unsigned long long gx = tifDir.split("x")[1].toInt();
    unsigned long long gy = tifDir.split("x")[0].split("(")[1].toInt();
    unsigned long long gz = tifDir.split("x")[2].split(")")[0].toInt();



    int t = resolution;
    qDebug()<<"t: "<<t;
//    gx /= t, gy /= t, gz /= t;

//    gx += 1, gy += 1, gz += 1;

    unsigned long long tolSZ = gx*gy*gz;

    qDebug()<<"tolSZ: "<<tolSZ<<" gx: "<<gx<<" gy: "<<gy<<" gz: "<<gz;


    qDebug()<<"start to allocate";
    unsigned char* data = new unsigned char[tolSZ];
    if(!data){
        qDebug()<<"allocate memory failed";
    }else {
        qDebug()<<"allocate memory successed";
    }

    memset(data,0,tolSZ*sizeof(unsigned char));




    QFileInfoList dirList1 = QDir(tifDir).entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
    for(int i=0; i<dirList1.size(); i++){
//        qDebug()<<"dir1: "<<dirList1[i].absoluteFilePath();
        QDir dir1 = QDir(dirList1[i].absoluteFilePath());
        QFileInfoList dirList2 = dir1.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot);
        for(int j=0; j<dirList2.size(); j++){
//            qDebug()<<"dir2: "<<dirList2[j].absoluteFilePath();
            QDir dir2 = QDir(dirList2[j].absoluteFilePath());
            QFileInfoList fileList = dir2.entryInfoList(QDir::Files);
            for(int k=0; k<fileList.size(); k++){
//                qDebug()<<"file: "<<fileList[k].absoluteFilePath();
                QString file = fileList[k].absoluteFilePath();

                QString fileName = fileList[k].baseName();
//                qDebug()<<"fileName: "<<fileName;

                int ox = fileName.split("_")[1].toInt();
                int oy = fileName.split("_")[0].toInt();
                int oz = fileName.split("_")[2].toInt();

                ox /= (t*10);
                oy /= (t*10);
                oz /= (t*10);

                qDebug()<<"ox: "<<ox<<" oy: "<<oy<<" oz: "<<oz;

                unsigned char* pdata = 0;
                V3DLONG sz[4] = {0,0,0,0};
                int dataType = 1;
                simple_loadimage_wrapper(callback,file.toStdString().c_str(),pdata,sz,dataType);
                qDebug()<<"dataType: "<<dataType;

                qDebug()<<"sz012: "<<sz[0]<<" "<<sz[1]<<" "<<sz[2];

                unsigned short* sdata = (unsigned short*)pdata;

                for(int z=0; z<sz[2]; z++){
                    for(int y=0; y<sz[1]; y++){
                        for(int x=0; x<sz[0]; x++){
                            int pIndex = z*sz[0]*sz[1] + y*sz[0] + x;
                            unsigned long long dz = (z+oz)>(gz-1)? gz-1 : z+oz;
                            unsigned long long dy = (y+oy)>(gy-1)? gy-1 : y+oy;
                            unsigned long long dx = (x+ox)>(gx-1)? gx-1 : x+ox;

                            unsigned long long index = dz*gx*gy + dy*gx + dx;
                            if(index>tolSZ || index<0){
//                                qDebug()<<"dz dy dx"<<dz<<" "<<dy<<" "<<dx;
//                                qDebug()<<"index: "<<index;
                                continue;
                            }
//                            qDebug()<<"index: "<<index;
                            unsigned char tmp = 0;
                            if(dataType == 1){
                                tmp = pdata[pIndex];
                            }else if(dataType == 2){

                                int t = ((double)sdata[pIndex]/4096.0)*255 + 0.5;
                                tmp = t;
                                if(t<0) tmp = 0;
                                if(t>255) tmp = 255;
                            }
                            data[index] = tmp;
//                            if(data[index]>20){
//                                qDebug()<<index<<" : "<<(int)data[index];
//                            }

                        }
                    }
                }

                if(pdata){
                    delete[] pdata;
                    pdata = 0;
                }
            }
        }
    }


    V3DLONG outSZ[4];
    outSZ[0] = gx;
    outSZ[1] = gy;
    outSZ[2] = gz;
    outSZ[3] = 1;
    bool a = simple_saveimage_wrapper(callback,outPath.toStdString().c_str(),data,outSZ,1);
    qDebug()<<"a: "<<a;
    qDebug()<<"save image end";

    if(data){
        qDebug()<<"delete data";
        delete[] data;
        data = 0;
    }

}

//Histogram Equalization
void HE(unsigned char *data1d, long long *sz){
    V3DLONG* HA = new V3DLONG[256];
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    memset(HA,0,256*sizeof(V3DLONG));
    for(int i=0; i<tolSZ; i++){
        HA[data1d[i]]++;
    }
    for(int i=1; i<256; i++){
        HA[i] += HA[i-1];
        qDebug()<<i<<" "<<HA[i];
    }

    for(int i=0; i<tolSZ; i++){
        int tmp = HA[data1d[i]]*256/(double)tolSZ;
//        qDebug()<<tmp;
        if(tmp>255)
            tmp = 255;
        if(tmp<0)
            tmp = 0;
        data1d[i] = (unsigned char)tmp;
    }
    if(HA){
        delete[] HA;
        HA = 0;
    }
}

void getColorMask(vector<double> &colorMask, double colorSigma){
    for(int i=0; i<256; ++i){
        double colorDiff = exp(-(i*i)/(2*colorSigma*colorSigma));
        colorMask.push_back(colorDiff);
    }
}

//void getGaussianMask(float* &mask, V3DLONG* kernelSZ, double spaceSigmaXY, double spaceSigmaZ){
//    V3DLONG tolSZ = kernelSZ[0]*kernelSZ[1]*kernelSZ[2];
//    mask = new float[tolSZ];
//    V3DLONG kernelSZ01 = kernelSZ[0]*kernelSZ[1];
//    int centerX = (kernelSZ[0] - 1) / 2;
//    int centerY = (kernelSZ[1] - 1) / 2;
//    int centerZ = (kernelSZ[2] - 1) / 2;
//    double x,y,z;
//    double sum = 0;

//    for(int k=0; k<kernelSZ[2]; ++k){
//        z = pow(k-centerZ,2);
//        for(int j=0; j<kernelSZ[1]; ++j){
//            y = pow(j-centerY,2);
//            for(int i=0; i<kernelSZ[0]; ++i){
//                x = pow(i-centerX,2);
//                double g = exp(-(x+y)/(2*spaceSigmaXY*spaceSigmaXY)-z/(2*spaceSigmaZ*spaceSigmaZ));
//                qDebug()<<g;
//                V3DLONG index = k*kernelSZ01 + j*kernelSZ[0] + i;
//                mask[index] = g;
//                sum += g;
//            }
//        }
//    }

//    for(int i=0; i<tolSZ; i++){
//        mask[i] /= sum;
//    }
//}

void getGaussianMask(float* &mask, V3DLONG* kernelSZ, double spaceSigmaXY, double spaceSigmaZ){
    V3DLONG tolSZ = kernelSZ[0]*kernelSZ[1]*kernelSZ[2];
    mask = new float[tolSZ];
    V3DLONG kernelSZ01 = kernelSZ[0]*kernelSZ[1];
    int centerX = (kernelSZ[0] - 1) / 2;
    int centerY = (kernelSZ[1] - 1) / 2;
    int centerZ = (kernelSZ[2] - 1) / 2;
    double x,y,z;
    double sum = 0;

    for(int k=0; k<kernelSZ[2]; ++k){
        z = pow(k-centerZ,2);
        for(int j=0; j<kernelSZ[1]; ++j){
            y = pow(j-centerY,2);
            for(int i=0; i<kernelSZ[0]; ++i){
                x = pow(i-centerX,2);
                double g =0;
                if(spaceSigmaZ>0) g = exp(-(x+y)/(2*spaceSigmaXY*spaceSigmaXY)-z/(2*spaceSigmaZ*spaceSigmaZ));
                else g = exp(-(x+y)/(2*spaceSigmaXY*spaceSigmaXY));
                qDebug()<<g;
                V3DLONG index = k*kernelSZ01 + j*kernelSZ[0] + i;
                mask[index] = g;
                sum += g;
            }
        }
    }

    for(int i=0; i<tolSZ; i++){
        mask[i] /= sum;
    }
}


void bilateralfilter(unsigned char* src, unsigned char* &dst, V3DLONG* sz, V3DLONG* kernelSZ, double spaceSigmaXY, double spaceSigmaZ, double colorSigma){
    qDebug()<<"-----bilateralfilter-------";
    vector<double> colorMask = vector<double>();
    float* spaceMask = 0;
    getColorMask(colorMask,colorSigma);
    qDebug()<<"get color mask end";
    getGaussianMask(spaceMask,kernelSZ,spaceSigmaXY,spaceSigmaZ);
    qDebug()<<"get mask end";

    V3DLONG tolKernelSZ = kernelSZ[0]*kernelSZ[1]*kernelSZ[2];
    float* mask = new float[tolKernelSZ];
    int centerX = (kernelSZ[0] - 1) / 2;
    int centerY = (kernelSZ[1] - 1) / 2;
    int centerZ = (kernelSZ[2] - 1) / 2;

    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    dst = new unsigned char[tolSZ];
    V3DLONG tolNewSZ = (sz[0]+centerX*2)*(sz[1]+centerY*2)*(sz[2]+centerZ*2);
    unsigned char* srcNew = new unsigned char[tolNewSZ];
    for(int k=0; k<sz[2]+centerZ*2; ++k){
        for(int j=0; j<sz[1]+centerY*2; ++j){
            for(int i=0; i<sz[0]+centerX*2; ++i){
                V3DLONG index = k*(sz[0]+centerX*2)*(sz[1]+centerY*2) + j*(sz[0]+centerX*2) + i;
                V3DLONG srcX = i - centerX;
                V3DLONG srcY = j - centerY;
                V3DLONG srcZ = k - centerZ;
                if(srcX<0) srcX = 0; if(srcX>=sz[0]) srcX = sz[0]-1;
                if(srcY<0) srcY = 0; if(srcY>=sz[1]) srcY = sz[1]-1;
                if(srcZ<0) srcZ = 0; if(srcZ>=sz[2]) srcZ = sz[2]-1;
                V3DLONG srcIndex = srcZ*sz[0]*sz[1] + srcY*sz[0] +srcX;
                srcNew[index] = src[srcIndex];
            }
        }
    }
    qDebug()<<"-------start--------";

    for(int k=centerZ; k<sz[2]+centerZ; ++k){
        for(int j=centerY; j<sz[1]+centerY; ++j){
            for(int i=centerX; i<sz[0]+centerX; ++i){
                double sum = 0;
                double grayDiff = 0;
                double spaceColorSum = 0;

                for(int kk=-centerZ; kk<=centerZ; ++kk){
                    for(int jj=-centerY; jj<=centerY; ++jj){
                        for(int ii=-centerX; ii<=centerX; ++ii){
                            V3DLONG index = k*(sz[0]+centerX*2)*(sz[1]+centerY*2) + j*(sz[0]+centerX*2) + i;
                            int centerPix = srcNew[index];
                            V3DLONG curIndex = (k+kk)*(sz[0]+centerX*2)*(sz[1]+centerY*2) + (j+jj)*(sz[0]+centerX*2) + (i+ii);
                            int pix = srcNew[curIndex];
                            grayDiff = abs(centerPix-pix);
                            double colorWeight = colorMask[grayDiff];
                            V3DLONG maskIndex = (kk+centerZ)*kernelSZ[0]*kernelSZ[1] + (jj+centerY)*kernelSZ[0] + (ii+centerX);
                            mask[maskIndex] = colorWeight*spaceMask[maskIndex];
                            spaceColorSum += mask[maskIndex];
                        }
                    }
                }

                for(int ii=0; ii<tolKernelSZ; ++ii){
                    mask[ii] /= spaceColorSum;
                }

                for(int kk=-centerZ; kk<=centerZ; ++kk){
                    for(int jj=-centerY; jj<=centerY; ++jj){
                        for(int ii=-centerX; ii<=centerX; ++ii){
                            V3DLONG curIndex = (k+kk)*(sz[0]+centerX*2)*(sz[1]+centerY*2) + (j+jj)*(sz[0]+centerX*2) + (i+ii);
                            V3DLONG maskIndex = (kk+centerZ)*kernelSZ[0]*kernelSZ[1] + (jj+centerY)*kernelSZ[0] + (ii+centerX);
                            sum += srcNew[curIndex]*mask[maskIndex];
                        }
                    }
                }

                if(sum<0)
                    sum = 0;
                if(sum>255)
                    sum = 255;
                V3DLONG dstIndex = (k-centerZ)*sz[0]*sz[1] + (j-centerY)*sz[0] + (i-centerX);
//                qDebug()<<sum;
                dst[dstIndex] = (unsigned char)sum;

            }
        }
    }

    colorMask.clear();
    if(spaceMask){
        delete[] spaceMask;
        spaceMask = 0;
    }
    if(mask){
        delete[] mask;
        mask = 0;
    }
    if(srcNew){
        delete[] srcNew;
        srcNew = 0;
    }

}

void im_roll(unsigned char * src, unsigned char * dst, V3DLONG* sz, int* d_roll)
{
//    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
//    dst = new unsigned char[tolSZ];

    int centerX = d_roll[0];
    int centerY = d_roll[1];
    int centerZ = d_roll[2];

    for(V3DLONG k=centerZ; k<sz[2]+centerZ; ++k){
        for(V3DLONG j=centerY; j<sz[1]+centerY; ++j){
            for(V3DLONG i=centerX; i<sz[0]+centerX; ++i){

                V3DLONG x = i, y=j, z=k;

                x = (x<0)?0:x;
                y = (y<0)?0:y;
                z = (z<0)?0:z;

                x = (x>=sz[0])?sz[0]-1:x;
                y = (y>=sz[1])?sz[1]-1:y;
                z = (z>=sz[2])?sz[2]-1:z;
                V3DLONG index = z*sz[0]*sz[1] + y*sz[0] + x;
                V3DLONG index1 = (k-centerZ)*sz[0]*sz[1] + (j-centerY)*sz[0] + i-centerX;
                dst[index1] = src[index];
            }
        }
    }
}

void bilateralfilter2(unsigned char* src, unsigned char* &dst, V3DLONG* sz, V3DLONG* kernelSZ, double spaceSigmaXY, double spaceSigmaZ, double colorSigma){
    qDebug()<<"-----bilateralfilter-------";

    if (!src || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 || kernelSZ[0]<=0 || kernelSZ[1]<=0 || kernelSZ[2]<=0)
    {
        v3d_msg("Invalid parameters to bilateral_filter().", 0);
        return;
    }

    int centerX = (kernelSZ[0] - 1) / 2;
    int centerY = (kernelSZ[1] - 1) / 2;
    int centerZ = (kernelSZ[2] - 1) / 2;

    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];

    vector<double> colorMask = vector<double>();
    getColorMask(colorMask, colorSigma);
    float* spaceMask = 0;
    getGaussianMask(spaceMask, kernelSZ, spaceSigmaXY, spaceSigmaZ);
    dst = new unsigned char[tolSZ];
    unsigned char* srcNew = new unsigned char[tolSZ];

    double* wgt_sum = new double[tolSZ];

    for (V3DLONG i=0; i<tolSZ; i++)
    {
        srcNew[i] = 0;
        wgt_sum[i] = 0;
    }

    qDebug()<<"-------start--------";

    for(int kk=-centerZ; kk<=centerZ; ++kk){
        for(int jj=-centerY; jj<=centerY; ++jj){
            for(int ii=-centerX; ii<=centerX; ++ii){

                V3DLONG maskIndex = (kk+centerZ)*kernelSZ[0]*kernelSZ[1] + (jj+centerY)*kernelSZ[0] + (ii+centerX);

                int d_roll[3]={ii, jj, kk};
//                srcNew = 0;
                im_roll(src, srcNew, sz, d_roll);
                for(int ip=0; ip<tolSZ; ++ip)
                {
                    double tw = colorMask[abs(srcNew[ip]-src[ip])]*spaceMask[maskIndex];
                    dst[ip] += tw*srcNew[ip];
                    wgt_sum[ip] += tw;
                }
            }
        }
    }

    for (V3DLONG i=0; i<tolSZ; i++)
    {
        dst[i] = dst[i]/wgt_sum[i];
    }

    if(srcNew){
        delete[] srcNew;
        srcNew = 0;
    }
}


void changeContrast(unsigned char* data1d, V3DLONG* sz){
    double imageMean = 0, imageStd = 0;
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    mean_and_std(data1d,tolSZ,imageMean,imageStd);
    if(imageMean>30){
        return;
    }
    double th1 = imageMean + imageStd*2;
    double th2 = imageMean + imageStd*10;
    double tmp;
    for(V3DLONG i=0; i<tolSZ; i++){
        if(data1d[i]<th1){
            tmp = (data1d[i]/th1)*80;
            data1d[i] = (unsigned char)tmp;
        }else if (data1d[i]<th2) {
            tmp = ((data1d[i]-80)/(200-80))*(200-80) + 80;
            data1d[i] = (unsigned char)tmp;
        }else{
            tmp = ((data1d[i]-200)/(255-200))*(255-200) + 200;
            if(tmp>255)
                tmp = 255;
            data1d[i] = (unsigned char)tmp;
        }
    }
}

void changeContrast2(unsigned char* data1d, V3DLONG* sz, double percentDown, double percentUp){
    qDebug()<<"in changeContrast2";
    double th1,th2;
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    V3DLONG* hist = new V3DLONG[256];
    memset(hist,0,256*sizeof(V3DLONG));
    qDebug()<<"set 0";
    for(V3DLONG i=0; i<tolSZ; i++){
        hist[data1d[i]]++;
    }
    qDebug()<<"hist end";
    V3DLONG count=0;
    for(int i=0; i<256; i++){
        count += hist[i];
        if(count/(double)tolSZ>percentDown/100.0){
            th1 = i;
            break;
        }
    }
    count = 0;
    for(int i=255; i>=0; i--){
        count += hist[i];
        if(count/(double)tolSZ>(1-percentUp/100.0)){
            th2 = i;
            break;
        }
    }
    qDebug()<<"th1,th2: "<<th1<<" "<<th2;

    double tmp;
    for(V3DLONG i=0; i<tolSZ; i++){
        if(data1d[i]<=th1){
            tmp = 0;
            data1d[i] = (unsigned char)tmp;
        }else if(data1d[i]<th2){
            tmp = ((data1d[i]-th1)/(th2-th1))*255;
            data1d[i] = (unsigned char)tmp;
        }else {
            tmp = 255;
            data1d[i] = (unsigned char)tmp;
        }
    }

    if(hist){
        delete[] hist;
        hist = 0;
    }
}

void convertDataTo0_255(unsigned char *data1d, long long *sz){
    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    double iMin = INT_MAX;
    double iMax = 0;
    for(V3DLONG i=0; i<tolSZ; i++){
        if(data1d[i]>iMax){
            iMax = data1d[i];
        }
        if(data1d[i]<iMin){
            iMin = data1d[i];
        }
    }

    for(V3DLONG i =0; i<tolSZ; i++){
        double tmp = ((data1d[i]-iMin)/(iMax-iMin))*255;
        if(tmp>255) tmp = 255;
        data1d[i] = (unsigned char) tmp;
    }
}

void removeAbnormalLine(unsigned char* data1d, V3DLONG* sz){

    V3DLONG tolSZ = sz[0]*sz[1]*sz[2];
    V3DLONG sz01 = sz[0]*sz[1];

    unsigned imageMin = 255;
    for(V3DLONG i=0; i<tolSZ; i++){
        if(imageMin>data1d[i]){
            imageMin = data1d[i];
        }
    }

    for(int z=0; z<sz[2]; z++){
        for(int y=0; y<sz[1]; y++){
            double zyMean = 0;
            for(int x=0; x<sz[0]; x++){
                V3DLONG index = z*sz01 + y*sz[0] + x;
                zyMean += data1d[index];
            }
            if(sz[0]>0){
                zyMean /= (double)sz[0];
            }
            if(zyMean>64){
                for(int x=0; x<sz[0]; x++){
                    V3DLONG index = z*sz[0]*sz[1] + y*sz[0] + x;
                    data1d[index] = imageMin;
                }
            }
        }
    }
}




















