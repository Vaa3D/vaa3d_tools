#include "stackAnalyzer.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))


StackAnalyzer::StackAnalyzer(V3DPluginCallback2 &callback)
{
    cb = &callback;
}

void StackAnalyzer::loadScan(QString latestString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation , QString saveDirString, bool useGSDT, bool isSoma){
    qDebug()<<"loadScan input: "<<latestString;
    qDebug()<<"overlap input:"<< QString::number(overlap);
    //QString latestString = getFileString();

    // Zhi:  this is a stack on AIBSDATA/MAT
    // modify as needed for your local path!

    //  latestString =QString("/data/mat/BRL/testData/ZSeries-01142016-0940-048/ZSeries-01142016-0940-048_Cycle00001_Ch2_000001.ome.tif");
    //LandmarkList inputRootList;
    qDebug()<<"loadScan input: "<<latestString;
    //   LocationSimple testroot;
    //   testroot.x = 10;testroot.y = 31;testroot.z = 101;inputRootList.push_back(testroot);
    //   testroot.x = 205;testroot.y = 111;testroot.z = 102;inputRootList.push_back(testroot);


    QFileInfo imageFileInfo = QFileInfo(latestString);
    if (imageFileInfo.isReadable()){
        //  v3dhandle newwin = cb->newImageWindow();
        Image4DSimple * pNewImage = cb->loadImage(latestString.toLatin1().data());
        QDir imageDir =  imageFileInfo.dir();
        QStringList filterList;
        filterList.append(QString("*Ch2*.tif"));
        imageDir.setNameFilters(filterList);
        QStringList fileList = imageDir.entryList();

        //get the parent dir and the list of ch1....ome.tif files
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
        for (int f=0; f<nFrames; f++){
            //qDebug()<<fileList[f];
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


        //convert to 8bit image using 8 shiftnbits
        //        unsigned char * total1dData_8bit = 0;
        //        try
        //        {
        //            total1dData_8bit = new unsigned char [tunits];
        //        }
        //        catch (...)
        //        {
        //            v3d_msg("Fail to allocate memory in total1dData_8bit.\n");
        //            return;
        //        }
        //        double dn = pow(2.0, double(5));
        //        for (V3DLONG i=0;i<tunits;i++)
        //        {
        //            double tmp = (double)(total1dData[i]) / dn;
        //            if (tmp>255) total1dData_8bit[i] = 255;
        //            else
        //                total1dData_8bit[i] = (unsigned char)(tmp);
        //        }

        //        Image4DSimple* total4DImage = new Image4DSimple;
        //        total4DImage->setData((unsigned char*)total1dData_8bit, x, y, nFrames, 1, V3D_UINT8);

        //new code starts here:

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

        outputStream<< (int) total4DImage->getOriginX()<<" "<< (int) total4DImage->getOriginY()<<" "<<swcString<<"\n";

        V3DLONG mysz[4];
        mysz[0] = total4DImage->getXDim();
        mysz[1] = total4DImage->getYDim();
        mysz[2] = total4DImage->getZDim();
        mysz[3] = total4DImage->getCDim();
        QString imageSaveString = saveDirString;

        imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".v3draw");
        simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, V3D_UINT16);

        //        if(total1dData) {delete []total1dData; total1dData = 0;}

        //convert to 8bit image using 1percentage saturation
        double apercent = 0.01;
        if(isSoma) apercent = 0.05;
        V3DLONG maxvv = ceil(p_vmax+1);
        double *hist = 0;
        try
        {
            hist = new double [maxvv];
        }
        catch (...)
        {
            qDebug() << "fail to allocate"; return;
            v3d_msg("Fail to allocate memory in proj_general_scaleandconvert28bit_1percentage().\n");
            return;
        }

        for (V3DLONG i=0;i<maxvv;i++)
        {
            hist[i] = 0;
        }
        //find the histogram
        for (V3DLONG i=0;i<tunits;i++)
        {
            hist[total1dData[i]] += 1;
        }
        qDebug() << "Histogram computed.";

        //compute the CDF
        for (V3DLONG i=1;i<maxvv;i++)
        {
            hist[i] += hist[i-1];
        }
        for (V3DLONG i=0;i<maxvv;i++)
        {
            hist[i] /= hist[maxvv-1];
        }
        //now search for the intensity thresholds
        double lowerth, upperth; lowerth = upperth = 0;
        for (V3DLONG i=0;i<maxvv-1;i++) //not the most efficient method, but the code should be readable
        {
            if (hist[i]<apercent && hist[i+1]>apercent)
                lowerth = i;
            if (hist[i]<1-apercent && hist[i+1]>1-apercent)
                upperth = i;
        }

        //real rescale of intensity
        scaleintensity(total4DImage,0, lowerth, upperth, double(0), double(255));

        //free space
        if (hist) {delete []hist; hist=0;}

        PARA_APP2 p;
        p.is_gsdt = useGSDT;
        p.is_coverage_prune = true;
        p.is_break_accept = false;
        p.bkg_thresh = background;
        p.length_thresh = 20;
        p.cnn_type = 2;
        p.channel = 0;
        p.SR_ratio = 3.0/9.9;
        p.b_256cube = 1;
        p.b_RadiusFrom2D = true;
        p.b_resample = 1;
        p.b_intensity = 0;
        p.b_brightfiled = 0;
        p.b_menu = interrupt; //if set to be "true", v3d_msg window will show up.

        p.p4dImage = total4DImage;
        p.xc0 = p.yc0 = p.zc0 = 0;
        p.xc1 = p.p4dImage->getXDim()-1;
        p.yc1 = p.p4dImage->getYDim()-1;
        p.zc1 = p.p4dImage->getZDim()-1;
        QString versionStr = "v2.621";

        qDebug()<<"starting app2";
        qDebug()<<"rootlist size "<<QString::number(inputRootList.size());
        LandmarkList imageLandmarks;

        if(inputRootList.size() <1)
        {
            p.outswc_file =swcString;
            proc_app2(*cb, p, versionStr);
        }
        else
        {




            // eliminate markers within some  pixels of any other marker
            //  well... this does that- getting rid of all markers in clusters!
            // I need some kind of mean-shift on the guys within a certain radius or keep track of
            // each cluster separately.
            //            LandmarkList betterRootList;
            //            for (int i=0;i<inputRootList.length();i++){
            //                float minr2 = 10;
            //                int timesThrough =0;
            //                int notej = -1;
            //                for (int j =0; j<inputRootList.length(); j++){

            //                    float r2ij = (inputRootList.at(i).x - inputRootList.at(j).x)*(inputRootList.at(i).x - inputRootList.at(j).x) +
            //                            (inputRootList.at(i).y - inputRootList.at(j).y)* (inputRootList.at(i).y - inputRootList.at(j).y)+
            //                            (inputRootList.at(i).z - inputRootList.at(j).z)*(inputRootList.at(i).z - inputRootList.at(j).z);
            //                    if ((r2ij<minr2 )&&(i!=j)) {
            //                        minr2=r2ij;
            //                        qDebug()<<r2ij;

            //                    }

            //                }
            //                if (minr2==10){
            //                    betterRootList.append(inputRootList.at(i));
            //                }
            //            }
            //            inputRootList.clear();
            //            inputRootList = betterRootList;


            vector<MyMarker*> tileswc_file;
            int maxRootListSize;// kludge here to make it through debugging. need some more filters on the swc outputs and marker inputs
            //            if (inputRootList.size()>16){
            //                maxRootListSize = 16;
            //            }else{
            maxRootListSize = inputRootList.size();

            QList<ImageMarker> seedsToSave;
            for (int i = 0; i<maxRootListSize; i++){
                LocationSimple RootNewLocation;
                ImageMarker outputMarker;
                RootNewLocation.x = inputRootList.at(i).x - p.p4dImage->getOriginX();
                RootNewLocation.y = inputRootList.at(i).y - p.p4dImage->getOriginY();
                RootNewLocation.z = inputRootList.at(i).z - p.p4dImage->getOriginZ();
                imageLandmarks.append(RootNewLocation);
                outputMarker.x = inputRootList.at(i).x  - p.p4dImage->getOriginX();
                outputMarker.y = inputRootList.at(i).y - p.p4dImage->getOriginY();
                outputMarker.z = inputRootList.at(i).z - p.p4dImage->getOriginZ();
                seedsToSave.append(outputMarker);

            }
            QString markerSaveString;
            markerSaveString = swcString;
            markerSaveString.append(".marker");
            writeMarker_file(markerSaveString, seedsToSave);

            // are ZERO markers here with no coordinates? or do they come back to stackAnalyzer from s2UI??
            for(int i = 0; i < maxRootListSize; i++)
            {
                p.outswc_file =swcString + (QString::number(i)) + (".swc");
                LocationSimple RootNewLocation;
                RootNewLocation.x = inputRootList.at(i).x - p.p4dImage->getOriginX();
                RootNewLocation.y = inputRootList.at(i).y - p.p4dImage->getOriginY();
                RootNewLocation.z = inputRootList.at(i).z - p.p4dImage->getOriginZ();

                p.landmarks.push_back(RootNewLocation);
                proc_app2(*cb, p, versionStr);
                p.landmarks.clear();
                vector<MyMarker*> inputswc = readSWC_file(p.outswc_file.toStdString());
                qDebug()<<"ran app2";
                for(V3DLONG d = 0; d < inputswc.size(); d++)
                {
                    tileswc_file.push_back(inputswc[d]);
                }
            }
            saveSWC_file(swcString.toStdString().c_str(), tileswc_file);
        }


        NeuronTree nt;
        nt = readSWC_file(swcString);
        QVector<QVector<V3DLONG> > childs;
        V3DLONG neuronNum = nt.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = nt.listNeuron[i].pn;
            if (par<0) continue;
            childs[nt.hashNeuron.value(par)].push_back(i);
        }

        LandmarkList newTargetList;
        LandmarkList tip_left;
        LandmarkList tip_right;
        LandmarkList tip_up ;
        LandmarkList tip_down;
        QList<NeuronSWC> list = nt.listNeuron;
        for (V3DLONG i=0;i<list.size();i++)
        {
            if (childs[i].size()==0)
            {
                NeuronSWC curr = list.at(i);
                LocationSimple newTip;
                if( curr.x < 0.05*  p.p4dImage->getXDim() || curr.x > 0.95 *  p.p4dImage->getXDim() || curr.y < 0.05 * p.p4dImage->getYDim() || curr.y > 0.95* p.p4dImage->getYDim())
                {
                    V3DLONG node_pn = getParent(i,nt);// Zhi, what if there's no parent?
                    V3DLONG node_pn_2nd;
                    if( list.at(node_pn).pn < 0)
                    {
                        node_pn_2nd = node_pn;
                    }
                    else
                    {
                        node_pn_2nd = getParent(node_pn,nt);
                    }

                    newTip.x = list.at(node_pn_2nd).x + p.p4dImage->getOriginX();
                    newTip.y = list.at(node_pn_2nd).y + p.p4dImage->getOriginY();
                    newTip.z = list.at(node_pn_2nd).z + p.p4dImage->getOriginZ();
                }
                if( curr.x < 0.05* p.p4dImage->getXDim())
                {
                    tip_left.push_back(newTip);
                }else if (curr.x > 0.95 * p.p4dImage->getXDim())
                {
                    tip_right.push_back(newTip);
                }else if (curr.y < 0.05 * p.p4dImage->getYDim())
                {
                    tip_up.push_back(newTip);
                }else if (curr.y > 0.95*p.p4dImage->getYDim())
                {
                    tip_down.push_back(newTip);
                }
            }
        }

        QList<LandmarkList> newTipsList;
        LocationSimple newTarget;

        if(tip_left.size()>0)
        {
            newTipsList.push_back(tip_left);
            newTarget.x = -p.p4dImage->getXDim();
            newTarget.y = 0;
            newTarget.z = 0;
            newTargetList.push_back(newTarget);
        }
        if(tip_right.size()>0)
        {
            newTipsList.push_back(tip_right);
            newTarget.x = p.p4dImage->getXDim();
            newTarget.y = 0;
            newTarget.z = 0;
            newTargetList.push_back(newTarget);
        }
        if(tip_up.size()>0)
        {
            newTipsList.push_back(tip_up);
            newTarget.x = 0;
            newTarget.y = -p.p4dImage->getYDim();
            newTarget.z = 0;
            newTargetList.push_back(newTarget);
        }
        if(tip_down.size()>0)
        {
            newTipsList.push_back(tip_down);
            newTarget.x = 0;
            newTarget.y = p.p4dImage->getYDim();
            newTarget.z = 0;
            newTargetList.push_back(newTarget);
        }




        if (!newTargetList.empty())
        {
            for (int i = 0; i<newTargetList.length(); i++)
            {
                newTargetList[i].x = (1.0-overlap)*newTargetList[i].x+p.p4dImage->getOriginX();
                newTargetList[i].y=(1.0-overlap)*newTargetList[i].y+p.p4dImage->getOriginY();
                newTargetList[i].z =(1.0-overlap)*newTargetList[i].z+p.p4dImage->getOriginZ();
            }
        }
        saveTextFile.close();
        //    cb->setImage(newwin, total4DImage);
        //cb->open3DWindow(newwin);
        //    cb->setSWC(newwin,nt);

        if (!imageLandmarks.isEmpty()){
            //        cb->setLandmark(newwin,imageLandmarks);
            //        cb->pushObjectIn3DWindow(newwin);
            qDebug()<<"set landmark group";

        }

        //   cb->pushObjectIn3DWindow(newwin);
        //    cb->updateImageWindow(newwin);

        QList<ImageMarker> tipsToSave;
        QString markerSaveString2;
        markerSaveString2 = swcString;
        markerSaveString2.append("final.marker");
        for (int i =0; i<newTipsList.length(); i++){
            LandmarkList iList = newTipsList[i];
            for (int j = 0; j<iList.length();j++){
                ImageMarker markerIJ;
                markerIJ.x = iList[j].x;
                markerIJ.y = iList[j].y;
                markerIJ.z = iList[j].z;

                tipsToSave.append(markerIJ);
            }

        }
        writeMarker_file(markerSaveString2, tipsToSave);
        emit analysisDone(newTipsList, newTargetList, total4DImage_mip);

    }else{
        qDebug()<<"invalid image";
    }
}


void StackAnalyzer::loadGridScan(QString latestString,  LocationSimple tileLocation, QString saveDirString){
    QFileInfo imageFileInfo = QFileInfo(latestString);
    QString windowString = latestString;

    if (imageFileInfo.isReadable()){


        Image4DSimple * pNewImage = cb->loadImage(latestString.toLatin1().data());
        QDir imageDir =  imageFileInfo.dir();
        QStringList filterList;
        filterList.append(QString("*Ch2*.tif"));
        imageDir.setNameFilters(filterList);
        QStringList fileList = imageDir.entryList();

        //get the parent dir and the list of ch1....ome.tif files
        //use this to id the number of images in the stack (in one channel?!)
        V3DLONG x = pNewImage->getXDim();
        V3DLONG y = pNewImage->getYDim();
        V3DLONG nFrames = fileList.length();

        V3DLONG tunits = x*y*nFrames;
        unsigned short int * total1dData = new unsigned short int [tunits];
        V3DLONG totalImageIndex = 0;
        for (int f=0; f<nFrames; f++){
            //status(fileList[f]);
            Image4DSimple * pNewImage = cb->loadImage(imageDir.absoluteFilePath(fileList[f]).toLatin1().data());
            if (pNewImage->valid()){
                unsigned short int * data1d = 0;
                data1d = new unsigned short int [x*y];
                data1d = (unsigned short int*)pNewImage->getRawData();
                for (V3DLONG i = 0; i< (x*y); i++){
                    total1dData[totalImageIndex]= data1d[i];
                    totalImageIndex++;
                }
            }else{
                qDebug()<<QString(imageDir.absoluteFilePath(fileList[f])).append(" failed!");
            }

        }


        Image4DSimple* total4DImage = new Image4DSimple;
        total4DImage->setData((unsigned char*)total1dData, x, y, nFrames, 1, V3D_UINT16);
        total4DImage->setFileName(imageFileInfo.absoluteFilePath().toLatin1().data());



        QString swcString = saveDirString;
        swcString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".swc");

        QString scanDataFileString = saveDirString;
        scanDataFileString.append("/").append("scanDataGrid.txt");
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

        outputStream<< (int) total4DImage->getOriginX()<<" "<< (int) total4DImage->getOriginY()<<" "<<swcString<<"\n";

        V3DLONG mysz[4];
        mysz[0] = total4DImage->getXDim();
        mysz[1] = total4DImage->getYDim();
        mysz[2] = total4DImage->getZDim();
        mysz[3] = total4DImage->getCDim();
        QString imageSaveString = saveDirString;

        imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.baseName()).append(".ome.tif.v3draw");
        simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, V3D_UINT16);

        emit loadingDone();


    }else{  //initial string is not readable
        qDebug()<<QString("invalid image path: ").append(latestString);
    }
}

void StackAnalyzer::processStack(Image4DSimple InputImage){

}

void StackAnalyzer::processSmartScan(QString fileWithData){
    // zhi add code to generate combined reconstruction from .txt file
    // at location filewith data
    qDebug()<<"caught filename "<<fileWithData;
    bool gridMode = false;
    if (fileWithData.contains("Grid")){ gridMode = true;}
    qDebug()<<gridMode;
    // fileWithData = "/opt/zhi/Desktop/test_xiaoxiao/2016_02_08_Mon_15_08/scanData.txt";

    ifstream ifs(fileWithData.toLatin1());
    string info_swc;
    int offsetX, offsetY;
    string swcfilepath;
    vector<MyMarker*> outswc;
    int node_type = 1;
    int offsetX_min = 10000000,offsetY_min = 10000000,offsetX_max = -10000000,offsetY_max =-10000000;
    int origin_x,origin_y;
    while(ifs && getline(ifs, info_swc))
    {
        std::istringstream iss(info_swc);
        iss >> offsetX >> offsetY >> swcfilepath;
        if(offsetX < offsetX_min) offsetX_min = offsetX;
        if(offsetY < offsetY_min) offsetY_min = offsetY;
        if(offsetX > offsetX_max) offsetX_max = offsetX;
        if(offsetY > offsetY_max) offsetY_max = offsetY;

        if(node_type == 1)
        {
            origin_x = offsetX;
            origin_y = offsetY;
        }
        if (!gridMode){
            vector<MyMarker*> inputswc = readSWC_file(swcfilepath);;
            for(V3DLONG d = 0; d < inputswc.size(); d++)
            {
                inputswc[d]->x = inputswc[d]->x + offsetX;
                inputswc[d]->y = inputswc[d]->y + offsetY;
                inputswc[d]->type = node_type;
                outswc.push_back(inputswc[d]);
            }
            }node_type++;
    }
    ifs.close();


    QString fileSaveName = fileWithData + ".swc";

    if (gridMode){

    }else{
        for(V3DLONG i = 0; i < outswc.size(); i++)
        {
            outswc[i]->x = outswc[i]->x - offsetX_min;
            outswc[i]->y = outswc[i]->y - offsetY_min;
        }

        saveSWC_file(fileSaveName.toStdString().c_str(), outswc);
    }
    //write tc file
    QString folderpath = QFileInfo(fileWithData).absolutePath();
    QString swcfilepath2 = QString::fromStdString(swcfilepath);

    QString lastImagepath = folderpath + "/" +   QFileInfo(swcfilepath2).baseName().append(".ome.tif.v3draw");
    qDebug()<<lastImagepath;
    unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    int datatype;
    if(!simple_loadimage_wrapper(*cb, lastImagepath.toStdString().c_str(), data1d, in_sz, datatype))
    {
        cerr<<"load image "<<lastImagepath.toStdString()<<" error!"<<endl;
        return;
    }
    if(data1d) {delete []data1d; data1d=0;}
    QString tc_name = fileWithData + ".tc";
    ofstream myfile;
    myfile.open(tc_name.toStdString().c_str(), ios::in);
    if (myfile.is_open()==true)
    {
        qDebug()<<"initial file can be opened for reading and will be removed";

        myfile.close();
        remove(tc_name.toStdString().c_str());
    }
    myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
    if (!myfile.is_open()){
        qDebug()<<"file's not really open...";
        emit combinedSWC(fileSaveName);
        return;
    }
    myfile << "# thumbnail file \n";
    myfile << "NULL \n\n";
    myfile << "# tiles \n";
    myfile << node_type-1 << " \n\n";
    myfile << "# dimensions (XYZC) \n";
    myfile << in_sz[0] + offsetX_max - offsetX_min << " " << in_sz[1] + offsetY_max - offsetY_min << " " << in_sz[2] << " " << 1 << " ";
    myfile << "\n\n";
    myfile << "# origin (XYZ) \n";
    myfile << "0.000000 0.000000 0.000000 \n\n";
    myfile << "# resolution (XYZ) \n";
    myfile << "1.000000 1.000000 1.000000 \n\n";
    myfile << "# image coordinates look up table \n";

    ifstream ifs_2nd(fileWithData.toLatin1());
    while(ifs_2nd && getline(ifs_2nd, info_swc))
    {
        std::istringstream iss(info_swc);
        iss >> offsetX >> offsetY >> swcfilepath;
        QString imagefilepath = QFileInfo(QString::fromStdString(swcfilepath)).completeBaseName() + ".v3draw";
        imagefilepath.append(QString("   ( %1, %2, 0) ( %3, %4, %5)").arg(offsetX - origin_x).arg(offsetY- origin_y).arg(in_sz[0]-1 + offsetX - origin_x).arg(in_sz[1]-1 + offsetY - origin_y).arg(in_sz[2]-1));
        myfile << imagefilepath.toStdString();
        myfile << "\n";
    }
    myfile.flush();
    myfile.close();
    ifs_2nd.close();
    emit combinedSWC(fileSaveName);
}
