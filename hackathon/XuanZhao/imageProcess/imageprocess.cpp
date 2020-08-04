#include "imageprocess.h"

#include <math.h>

#include <iostream>

#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"

#include <thread>
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
    for(int i=0; i<nt_crop_sorted.listNeuron.size(); i++){
        nt_crop_sorted.listNeuron[i].r = 1;
    }
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
           int mipIndex = j*mysz[0] + i - mysz[0];
           data1d_2D[index] = (label_mip[mipIndex] == 255) ? 0 : image_mip[mipIndex];
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
           int index = j*mysz[0]*2 + i+ stacksz*2;
           int mipIndex = j*mysz[0] + i - mysz[0];
           data1d_2D[index] = (label_mip[mipIndex] == 255) ? 255 : image_mip[mipIndex];
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
           int index = j*mysz[0]*2 + i + stacksz*4;
           int mipIndex = j*mysz[0] + i - mysz[0];
           data1d_2D[index] = (label_mip[mipIndex] == 255) ? 0 : image_mip[mipIndex];
       }
   }

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
   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
   if(image_mip) {delete [] image_mip; image_mip=0;}
   if(label_mip) {delete [] label_mip; label_mip=0;}
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
























