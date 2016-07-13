#include "threadedTracer.h"

ThreadedTracer::ThreadedTracer(V3DPluginCallback2 &callback,QString latestString,  LocationSimple tileLocation, QString saveDirString,  QString channel, int tileNumber){
        cb = &callback;
        this->latestString = latestString;
        this->tileLocation = tileLocation;
        this->saveDirString = saveDirString;
        this->channel = channel;
        this->tileNumber = tileNumber;
}

void ThreadedTracer::run(){

qDebug()<<"loadScan input: "<<latestString;
//QString latestString = getFileString();
//    latestString =QString("/data/mat/BRL/DATA_FOR_GROUP/testDataForZhi/ZSeries-06092016-1407-8470/ZSeries-06092016-1407-8470_Cycle00001_Ch1_000001.ome.tif");
//    bool isAdaptive = 1;
//    int methodChoice = 1;

//LandmarkList inputRootList;

qDebug()<<tileNumber;

QList<LandmarkList> newTipsList;
LandmarkList newTargetList;

QFileInfo imageFileInfo = QFileInfo(latestString);
if (imageFileInfo.isReadable()){
    Image4DSimple * pNewImage = cb->loadImage(latestString.toLatin1().data());
    QDir imageDir =  imageFileInfo.dir();
    QStringList filterList;
    filterList.append(QString("*").append("Cycle%1").arg(tileNumber,5,10,QLatin1Char('0')).append("_Ch").append(channel).append("*.tif"));
    qDebug()<<"filterlist.first()"<<filterList.first();
    imageDir.setNameFilters(filterList);
    QStringList fileList = imageDir.entryList();

    //use this to id the number of images in the stack (in one channel?!)
    V3DLONG x = pNewImage->getXDim();
    V3DLONG y = pNewImage->getYDim();
    V3DLONG nFrames = fileList.length();

    V3DLONG tunits = x*y*nFrames;
    unsigned short int * total1dData = new unsigned short int [tunits];
    unsigned short int * total1dData_mip= new unsigned short int [x*y];
    for(V3DLONG i =0 ; i < x*y; i++)
        total1dData_mip[i] = 0;
    V3DLONG totalImageIndex = 0;
    double p_vmax=0;
    qDebug()<<"nFrames = "<<nFrames;
    for (int f=0; f<nFrames; f++){
        qDebug()<<fileList[f];
        Image4DSimple * pNewImage = cb->loadImage(imageDir.absoluteFilePath(fileList[f]).toLatin1().data());
        if (pNewImage->valid()){
            unsigned short int * data1d = 0;
            data1d = new unsigned short int [x*y];
            data1d = (unsigned short int*)pNewImage->getRawData();
            for (V3DLONG i = 0; i< (x*y); i++)
            {
                total1dData[totalImageIndex]= data1d[i];
                if(data1d[i] > p_vmax) p_vmax = data1d[i];
                if(total1dData_mip[i] < data1d[i]) total1dData_mip[i] = data1d[i];
                totalImageIndex++;
            }
            if(data1d) {delete []data1d; data1d = 0;}
        }else{
            qDebug()<<imageDir.absoluteFilePath(fileList[f])<<" failed!";
        }
    }

    Image4DSimple* total4DImage = new Image4DSimple;
    total4DImage->setData((unsigned char*)total1dData, x, y, nFrames, 1, V3D_UINT16);

    Image4DSimple* total4DImage_mip = new Image4DSimple;
    total4DImage_mip->setData((unsigned char*)total1dData_mip, x, y, 1, 1, V3D_UINT16);


    QString swcString = saveDirString;
    swcString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".swc");


    QString scanDataFileString = saveDirString;
    scanDataFileString.append("/").append("scanData.txt");
    qDebug()<<scanDataFileString;
    QFile saveTextFile;
    saveTextFile.setFileName(scanDataFileString);// add currentScanFile
    if (!saveTextFile.isOpen()){
        if (!saveTextFile.open(QIODevice::Text|QIODevice::Append  )){
            qDebug()<<"unable to save file!";
            return;}     }
    QTextStream outputStream;
    outputStream.setDevice(&saveTextFile);
    total4DImage->setOriginX(tileLocation.x);
    total4DImage->setOriginY(tileLocation.y);

    qDebug()<<total4DImage->getOriginX();


    outputStream<< (int) total4DImage->getOriginX()<<" "<< (int) total4DImage->getOriginY()<<" "<<swcString<<" "<< (int) x<<" "<< (int) y<<" "<<"\n";

    saveTextFile.close();
    V3DLONG mysz[4];
    mysz[0] = total4DImage->getXDim();
    mysz[1] = total4DImage->getYDim();
    mysz[2] = total4DImage->getZDim();
    mysz[3] = total4DImage->getCDim();

    tileLocation.ev_pc1 = (double) total4DImage->getXDim();
    tileLocation.ev_pc2 = (double) total4DImage->getYDim();


    QString imageSaveString = saveDirString;


    // add bit of code to image green - red channels. This will require an additional argument or another signal/slot combination to monitor the value of a combobox in the GUI...
    // add button to do || nT on mXtls




    //convert to 8bit image using 8 shiftnbits
    unsigned char * total1dData_8bit = 0;
    try
    {
        total1dData_8bit = new unsigned char [tunits];
    }
    catch (...)
    {
        v3d_msg("Fail to allocate memory in total1dData_8bit.\n");

        return;
    }
    double dn = pow(2.0, double(5));
    for (V3DLONG i=0;i<tunits;i++)
    {
        double tmp = (double)(total1dData[i]) / dn;
        if (tmp>255) total1dData_8bit[i] = 255;
        else
            total1dData_8bit[i] = (unsigned char)(tmp);
    }




    total4DImage->setData((unsigned char*)total1dData_8bit, x, y, nFrames, 1, V3D_UINT8);
    imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".v3draw");
    simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData_8bit, mysz, V3D_UINT8);
    qDebug()<<"=== immediately before tracing =====";
//-------------
    V3DPluginArgItem arg;
    V3DPluginArgList input;
    V3DPluginArgList output;

    QString full_plugin_name;
    QString func_name;

    arg.type = "random";std::vector<char*> arg_input;
    std:: string fileName_Qstring(imageSaveString.toStdString());char* fileName_string =  new char[fileName_Qstring.length() + 1]; strcpy(fileName_string, fileName_Qstring.c_str());
    arg_input.push_back(fileName_string);
    arg.p = (void *) & arg_input; input<< arg;

    char* char_swcout =  new char[swcString.length() + 1];strcpy(char_swcout, swcString.toStdString().c_str());
    arg.type = "random";std::vector<char*> arg_output;arg_output.push_back(char_swcout); arg.p = (void *) & arg_output; output<< arg;

    arg.type = "random";
    std::vector<char*> arg_para;



    arg_para.push_back("1");
    arg_para.push_back("1");
    full_plugin_name = "neuTube";
    func_name =  "neutube_trace";

    arg.p = (void *) & arg_para; input << arg;

    if(!cb->callPluginFunc(full_plugin_name,func_name,input,output))
    {

         qDebug()<<("Can not find the tracing plugin!\n");

         return;
    }

}else{
    qDebug()<<"invalid image";
}
}
