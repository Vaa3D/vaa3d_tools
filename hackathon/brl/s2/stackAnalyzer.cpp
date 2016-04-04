#include "stackAnalyzer.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include <fstream>
#include <iostream>
#include <sstream>

#include "../../../hackathon/zhi/neuronassembler_plugin_creator/sort_swc.h"
#include <boost/lexical_cast.hpp>

using namespace std;
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define INF 1E10
template <class T> T pow2(T a)
{
    return a*a;

}

bool export_list2file(vector<MyMarker*> & outmarkers, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    QFile qf(fileOpenName);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QString info;
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
           info = buf;
           myfile<< info.remove('\n') <<endl;
        }

    }

    map<MyMarker*, int> ind;
    for(int i = 0; i < outmarkers.size(); i++) ind[outmarkers[i]] = i+1;

    for(V3DLONG i = 0; i < outmarkers.size(); i++)
    {
        MyMarker * marker = outmarkers[i];
        int parent_id;
        if(marker->parent == 0) parent_id = -1;
        else parent_id = ind[marker->parent];
        myfile<<i+1<<" "<<marker->type<<" "<<marker->x<<" "<<marker->y<<" "<<marker->z<<" "<<marker->radius<<" "<<parent_id<<"\n";
    }

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<outmarkers.size()<<endl;
    return true;
};


StackAnalyzer::StackAnalyzer(V3DPluginCallback2 &callback)
{
    cb = &callback;
    channel = QString("Ch2");
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
        filterList.append(QString("*").append(channel).append("*.tif"));
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

        outputStream<< (int) total4DImage->getOriginX()<<" "<< (int) total4DImage->getOriginY()<<" "<<swcString<<" "<< (int) x<<" "<< (int) y<<"\n";

        V3DLONG mysz[4];
        mysz[0] = total4DImage->getXDim();
        mysz[1] = total4DImage->getYDim();
        mysz[2] = total4DImage->getZDim();
        mysz[3] = total4DImage->getCDim();
        QString imageSaveString = saveDirString;

        imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".v3draw");
        simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, V3D_UINT16);

        //        if(total1dData) {delete []total1dData; total1dData = 0;}

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

     //   Image4DSimple* total4DImage = new Image4DSimple;

        total4DImage->setData((unsigned char*)total1dData_8bit, x, y, nFrames, 1, V3D_UINT8);

        //convert to 8bit image using 1percentage saturation
//        double apercent = 0.01;
//        if(isSoma) apercent = 0.05;
//        V3DLONG maxvv = ceil(p_vmax+1);
//        double *hist = 0;
//        try
//        {
//            hist = new double [maxvv];
//        }
//        catch (...)
//        {
//            qDebug() << "fail to allocate"; return;
//            v3d_msg("Fail to allocate memory in proj_general_scaleandconvert28bit_1percentage().\n");
//            return;
//        }

//        for (V3DLONG i=0;i<maxvv;i++)
//        {
//            hist[i] = 0;
//        }
//        //find the histogram
//        for (V3DLONG i=0;i<tunits;i++)
//        {
//            hist[total1dData[i]] += 1;
//        }
//        qDebug() << "Histogram computed.";

//        //compute the CDF
//        for (V3DLONG i=1;i<maxvv;i++)
//        {
//            hist[i] += hist[i-1];
//        }
//        for (V3DLONG i=0;i<maxvv;i++)
//        {
//            hist[i] /= hist[maxvv-1];
//        }
//        //now search for the intensity thresholds
//        double lowerth, upperth; lowerth = upperth = 0;
//        for (V3DLONG i=0;i<maxvv-1;i++) //not the most efficient method, but the code should be readable
//        {
//            if (hist[i]<apercent && hist[i+1]>apercent)
//                lowerth = i;
//            if (hist[i]<1-apercent && hist[i+1]>1-apercent)
//                upperth = i;
//        }

//        //real rescale of intensity
//        scaleintensity(total4DImage,0, lowerth, upperth, double(0), double(255));

//        //free space
//        if (hist) {delete []hist; hist=0;}

        PARA_APP2 p;
        p.is_gsdt = useGSDT;
        p.is_coverage_prune = true;
        p.is_break_accept = false;
        p.bkg_thresh = background;
        p.length_thresh = 5;
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

        list<string> infostring;
        string tmpstr; QString qtstr;
        tmpstr =  qPrintable( qtstr.prepend("## NeuronCrawler_APP2")); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.channel).prepend("#channel = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.bkg_thresh).prepend("#bkg_thresh = ") ); infostring.push_back(tmpstr);

        tmpstr =  qPrintable( qtstr.setNum(p.length_thresh).prepend("#length_thresh = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.SR_ratio).prepend("#SR_ratio = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.is_gsdt).prepend("#is_gsdt = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.is_break_accept).prepend("#is_gap = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.cnn_type).prepend("#cnn_type = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.b_256cube).prepend("#b_256cube = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.b_RadiusFrom2D).prepend("#b_radiusFrom2D = ") ); infostring.push_back(tmpstr);


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

                bool flag = false;
                if(tileswc_file.size()>0)
                {
                    for(V3DLONG dd = 0; dd < tileswc_file.size();dd++)
                    {
                        double dis = sqrt(pow2(RootNewLocation.x - tileswc_file.at(dd)->x) + pow2(RootNewLocation.y - tileswc_file.at(dd)->y) + pow2(RootNewLocation.z - tileswc_file.at(dd)->z));
                        if(dis < 10.0)
                        {
                           flag = true;
                           break;
                        }
                    }
                }

                if(!flag)
                {
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
            }
            saveSWC_file(swcString.toStdString().c_str(), tileswc_file,infostring);
        //    export_list2file(tileswc_file, swcString,p.outswc_file);
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

void StackAnalyzer::loadScan_adaptive(QString latestString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation , QString saveDirString, bool useGSDT, bool isSoma){
    qDebug()<<"loadScan input: "<<latestString;
    qDebug()<<"overlap input:"<< QString::number(overlap);
    //QString latestString = getFileString();

    // Zhi:  this is a stack on AIBSDATA/MAT
    // modify as needed for your local path!

   // latestString =QString("/data/mat/BRL/testData/ZSeries-02232016-0931-2797/ZSeries-02232016-0931-2797_Cycle00001_Ch1_000001.ome.tif");
   // saveDirString = "/opt/zhi/Desktop/super_plugin_test/2016_02_05_Fri_13_52/2016_03_25_Fri_09_35";
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
        filterList.append(QString("*").append(channel).append("*.tif"));
        imageDir.setNameFilters(filterList);
        QStringList fileList = imageDir.entryList();

        //get the parent dir and the list of ch1....ome.tif files
        //use this to id the number of images in the stack (in one channel?!)
        V3DLONG x = pNewImage->getXDim();
        V3DLONG y = pNewImage->getYDim();
        V3DLONG nFrames = fileList.length();

        tileLocation.ev_pc1 = x;
        tileLocation.ev_pc2 = y;


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

        outputStream<< (int) total4DImage->getOriginX()<<" "<< (int) total4DImage->getOriginY()<<" "<<swcString<<" "<< (int) x<<" "<< (int) y<<"\n";

        V3DLONG mysz[4];
        mysz[0] = total4DImage->getXDim();
        mysz[1] = total4DImage->getYDim();
        mysz[2] = total4DImage->getZDim();
        mysz[3] = total4DImage->getCDim();
        QString imageSaveString = saveDirString;

        imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".v3draw");
        simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, V3D_UINT16);

        QString finaloutputswc = saveDirString + ("/app2_adaptive.swc");
        ifstream ifs_swc(finaloutputswc.toStdString().c_str());
        vector<MyMarker*> finalswc;

        if(ifs_swc)
           finalswc = readSWC_file(finaloutputswc.toStdString());

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

        PARA_APP2 p;
        p.is_gsdt = useGSDT;
        p.is_coverage_prune = true;
        p.is_break_accept = false;
        p.bkg_thresh = background;
        p.length_thresh = 5;
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

        list<string> infostring;
        string tmpstr; QString qtstr;
        tmpstr =  qPrintable( qtstr.prepend("## NeuronCrawler_APP2")); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.channel).prepend("#channel = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.bkg_thresh).prepend("#bkg_thresh = ") ); infostring.push_back(tmpstr);

        tmpstr =  qPrintable( qtstr.setNum(p.length_thresh).prepend("#length_thresh = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.SR_ratio).prepend("#SR_ratio = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.is_gsdt).prepend("#is_gsdt = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.is_break_accept).prepend("#is_gap = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.cnn_type).prepend("#cnn_type = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.b_256cube).prepend("#b_256cube = ") ); infostring.push_back(tmpstr);
        tmpstr =  qPrintable( qtstr.setNum(p.b_RadiusFrom2D).prepend("#b_radiusFrom2D = ") ); infostring.push_back(tmpstr);


        LandmarkList imageLandmarks;
        vector<MyMarker*> tileswc_file;


        if(inputRootList.size() <1)
        {
            p.outswc_file =swcString;
            proc_app2(*cb, p, versionStr);
            tileswc_file = readSWC_file(p.outswc_file.toStdString());

        }
        else
        {

            int maxRootListSize;
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

            for(int i = 0; i < maxRootListSize; i++)
            {
                p.outswc_file =swcString + (QString::number(i)) + (".swc");
                LocationSimple RootNewLocation;
                RootNewLocation.x = inputRootList.at(i).x - p.p4dImage->getOriginX();
                RootNewLocation.y = inputRootList.at(i).y - p.p4dImage->getOriginY();
                RootNewLocation.z = inputRootList.at(i).z - p.p4dImage->getOriginZ();

                bool flag = false;
                if(tileswc_file.size()>0)
                {
                    for(V3DLONG dd = 0; dd < tileswc_file.size();dd++)
                    {
                        double dis = sqrt(pow2(RootNewLocation.x - tileswc_file.at(dd)->x) + pow2(RootNewLocation.y - tileswc_file.at(dd)->y) + pow2(RootNewLocation.z - tileswc_file.at(dd)->z));
                        if(dis < 10.0)
                        {
                           flag = true;
                           break;
                        }
                    }
                }

                if(!flag)
                {
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
            }
            saveSWC_file(swcString.toStdString().c_str(), tileswc_file,infostring);
        //    export_list2file(tileswc_file, swcString,p.outswc_file);
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
                bool check_tip = false;

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

                    for(V3DLONG j = 0; j < finalswc.size(); j++ )
                    {
                        double dis = sqrt(pow2(newTip.x - finalswc.at(j)->x) + pow2(newTip.y - finalswc.at(j)->y) + pow2(newTip.z - finalswc.at(j)->z));
                        if(dis < 10)
                        {
                            check_tip = true;
                            break;
                        }
                    }
                }
                if(check_tip) continue;
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
        LandmarkList newTargetList;


        if(tip_left.size()>0)
        {
            QList<LandmarkList> group_tips_left = group_tips(tip_left,128,1);
            for(int i = 0; i < group_tips_left.size();i++)
                ada_win_finding(group_tips_left.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,256,1,overlap);
        }
        if(tip_right.size()>0)
        {
            QList<LandmarkList> group_tips_right = group_tips(tip_right,128,2);
            for(int i = 0; i < group_tips_right.size();i++)
                ada_win_finding(group_tips_right.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,256,2,overlap);
        }
        if(tip_up.size()>0)
        {
            QList<LandmarkList> group_tips_up = group_tips(tip_up,128,3);
            for(int i = 0; i < group_tips_up.size();i++)
                ada_win_finding(group_tips_up.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,256,3,overlap);

        }
        if(tip_down.size()>0)
        {
            QList<LandmarkList> group_tips_down = group_tips(tip_down,128,4);
            for(int i = 0; i < group_tips_down.size();i++)
                ada_win_finding(group_tips_down.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,256,4,overlap);
        }

        if(ifs_swc)
        {
            for(V3DLONG i = 0; i < tileswc_file.size(); i++)
            {
                tileswc_file[i]->x = tileswc_file[i]->x + total4DImage->getOriginX();
                tileswc_file[i]->y = tileswc_file[i]->y + total4DImage->getOriginY();
                tileswc_file[i]->z = tileswc_file[i]->z + total4DImage->getOriginZ();

                finalswc.push_back(tileswc_file[i]);
            }
            saveSWC_file(finaloutputswc.toStdString().c_str(), finalswc);
        }
        else
        {
            for(V3DLONG i = 0; i < tileswc_file.size(); i++)
            {
                tileswc_file[i]->x = tileswc_file[i]->x + total4DImage->getOriginX();
                tileswc_file[i]->y = tileswc_file[i]->y + total4DImage->getOriginY();
                tileswc_file[i]->z = tileswc_file[i]->z + total4DImage->getOriginZ();
            }
            saveSWC_file(finaloutputswc.toStdString().c_str(), tileswc_file);
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
        filterList.append(QString("*").append(channel).append("*.tif"));
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
        for (int f=0; f<nFrames; f++){
            //status(fileList[f]);
            Image4DSimple * pNewImage = cb->loadImage(imageDir.absoluteFilePath(fileList[f]).toLatin1().data());
            if (pNewImage->valid()){
                unsigned short int * data1d = 0;
                data1d = new unsigned short int [x*y];
                data1d = (unsigned short int*)pNewImage->getRawData();
                for (V3DLONG i = 0; i< (x*y); i++){
                    total1dData[totalImageIndex]= data1d[i];
                    if(total1dData_mip[i] < data1d[i]) total1dData_mip[i] = data1d[i];

                    totalImageIndex++;
                }
            }else{
                qDebug()<<QString(imageDir.absoluteFilePath(fileList[f])).append(" failed!");
            }

        }


        Image4DSimple* total4DImage = new Image4DSimple;
        total4DImage->setData((unsigned char*)total1dData, x, y, nFrames, 1, V3D_UINT16);
        total4DImage->setFileName(imageFileInfo.absoluteFilePath().toLatin1().data());

        Image4DSimple* total4DImage_mip = new Image4DSimple;
        total4DImage_mip->setData((unsigned char*)total1dData_mip, x, y, 1, 1, V3D_UINT16);


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

        outputStream<< (int) total4DImage->getOriginX()<<" "<< (int) total4DImage->getOriginY()<<" "<<swcString<<" "<< (int) x<<" "<< (int) y<<"\n";

        V3DLONG mysz[4];
        mysz[0] = total4DImage->getXDim();
        mysz[1] = total4DImage->getYDim();
        mysz[2] = total4DImage->getZDim();
        mysz[3] = total4DImage->getCDim();
        QString imageSaveString = saveDirString;

        imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.baseName()).append(".ome.tif.v3draw");
        simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, V3D_UINT16);

        emit loadingDone(total4DImage_mip);


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
    //fileWithData = "/opt/zhi/Desktop/super_plugin_test/2016_03_25_Fri_15_23/scanData.txt";

    ifstream ifs(fileWithData.toLatin1());
    string info_swc;
    int offsetX, offsetY,sizeX, sizeY;
    string swcfilepath;
    vector<MyMarker*> outswc;
    int node_type = 1;
    int offsetX_min = 10000000,offsetY_min = 10000000,offsetX_max = -10000000,offsetY_max =-10000000;
    int origin_x,origin_y;
    while(ifs && getline(ifs, info_swc))
    {
        std::istringstream iss(info_swc);
        iss >> offsetX >> offsetY >> swcfilepath >> sizeX >> sizeY;
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

       // saveSWC_file(fileSaveName.toStdString().c_str(), outswc);
        export_list2file(outswc, fileSaveName,QString::fromStdString(swcfilepath));

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
        iss >> offsetX >> offsetY >> swcfilepath >> sizeX >> sizeY;
        QString imagefilepath = QFileInfo(QString::fromStdString(swcfilepath)).completeBaseName() + ".v3draw";

//        unsigned char * data1d = 0;
//        V3DLONG in_sz[4];
//        int datatype;
//        QString imagefilepath_abs = folderpath + "/" + imagefilepath;
//        if(!simple_loadimage_wrapper(*cb, imagefilepath_abs.toStdString().c_str(), data1d, in_sz, datatype))
//        {
//            cerr<<"load image "<<imagefilepath_abs.toStdString()<<" error!"<<endl;
//            return;
//        }
//        if(data1d) {delete []data1d; data1d=0;}


        imagefilepath.append(QString("   ( %1, %2, 0) ( %3, %4, %5)").arg(offsetX - origin_x).arg(offsetY- origin_y).arg(sizeX -1 + offsetX - origin_x).arg(sizeY - 1 + offsetY - origin_y).arg(in_sz[2]-1));
        myfile << imagefilepath.toStdString();
        myfile << "\n";
    }
    myfile.flush();
    myfile.close();
    ifs_2nd.close();
    emit combinedSWC(fileSaveName);
}

void StackAnalyzer::loadScan_MOST(QString latestString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation, QString saveDirString, bool useGSDT, bool isSoma)
{
    qDebug()<<"loadScan input: "<<latestString;
    qDebug()<<"overlap input:"<< QString::number(overlap);
    //QString latestString = getFileString();

    // Zhi:  this is a stack on AIBSDATA/MAT
    // modify as needed for your local path!

    int seed_win = 10;
    int slip_win = 10;
    //isSoma = true;
    //latestString =QString("/data/mat/BRL/testData/ZSeries-02242016-1111-3022/ZSeries-02242016-1111-3022_Cycle00001_Ch1_000001.ome.tif");
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
        filterList.append(QString("*").append(channel).append("*.tif"));
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

        outputStream<< (int) total4DImage->getOriginX()<<" "<< (int) total4DImage->getOriginY()<<" "<<swcString<<" "<< (int) x<<" "<< (int) y<<"\n";

        V3DLONG mysz[4];
        mysz[0] = total4DImage->getXDim();
        mysz[1] = total4DImage->getYDim();
        mysz[2] = total4DImage->getZDim();
        mysz[3] = total4DImage->getCDim();
        QString imageSaveString = saveDirString;

//        imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".v3draw");
//        simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, V3D_UINT16);

        //        if(total1dData) {delete []total1dData; total1dData = 0;}

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

     //   Image4DSimple* total4DImage = new Image4DSimple;

        total4DImage->setData((unsigned char*)total1dData_8bit, x, y, nFrames, 1, V3D_UINT8);

        imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".v3draw");
        simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData_8bit, mysz, V3D_UINT8);

        //convert to 8bit image using 1percentage saturation
//        double apercent = 0.01;
//        if(isSoma) apercent = 0.05;
//        V3DLONG maxvv = ceil(p_vmax+1);
//        double *hist = 0;
//        try
//        {
//            hist = new double [maxvv];
//        }
//        catch (...)
//        {
//            qDebug() << "fail to allocate"; return;
//            v3d_msg("Fail to allocate memory in proj_general_scaleandconvert28bit_1percentage().\n");
//            return;
//        }

//        for (V3DLONG i=0;i<maxvv;i++)
//        {
//            hist[i] = 0;
//        }
//        //find the histogram
//        for (V3DLONG i=0;i<tunits;i++)
//        {
//            hist[total1dData[i]] += 1;
//        }
//        qDebug() << "Histogram computed.";

//        //compute the CDF
//        for (V3DLONG i=1;i<maxvv;i++)
//        {
//            hist[i] += hist[i-1];
//        }
//        for (V3DLONG i=0;i<maxvv;i++)
//        {
//            hist[i] /= hist[maxvv-1];
//        }
//        //now search for the intensity thresholds
//        double lowerth, upperth; lowerth = upperth = 0;
//        for (V3DLONG i=0;i<maxvv-1;i++) //not the most efficient method, but the code should be readable
//        {
//            if (hist[i]<apercent && hist[i+1]>apercent)
//                lowerth = i;
//            if (hist[i]<1-apercent && hist[i+1]>1-apercent)
//                upperth = i;
//        }

//        //real rescale of intensity
//        scaleintensity(total4DImage,0, lowerth, upperth, double(0), double(255));

//        //free space
//        if (hist) {delete []hist; hist=0;}

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

        string S_background_th = boost::lexical_cast<string>(background);
        char* C_background_th = new char[S_background_th.length() + 1];
        strcpy(C_background_th,S_background_th.c_str());
        arg_para.push_back(C_background_th);

        string S_seed_win = boost::lexical_cast<string>(seed_win);
        char* C_seed_win = new char[S_seed_win.length() + 1];
        strcpy(C_seed_win,S_seed_win.c_str());
        arg_para.push_back(C_seed_win);

        string S_slip_win = boost::lexical_cast<string>(slip_win);
        char* C_slip_win = new char[S_slip_win.length() + 1];
        strcpy(C_slip_win,S_slip_win.c_str());
        arg_para.push_back(C_slip_win);

        full_plugin_name = "mostVesselTracer";  func_name =  "MOST_trace";
        arg.p = (void *) & arg_para; input << arg;


        qDebug()<<"starting most";
        qDebug()<<"rootlist size "<<QString::number(inputRootList.size());

//        list<string> infostring;
//        string tmpstr; QString qtstr;
//        tmpstr =  qPrintable( qtstr.prepend("## NeuronCrawler_MOST")); infostring.push_back(tmpstr);
//        tmpstr =  qPrintable( qtstr.setNum(0).prepend("#channel = ") ); infostring.push_back(tmpstr);
//        tmpstr =  qPrintable( qtstr.setNum(bkg_thresh).prepend("#bkg_thresh = ") ); infostring.push_back(tmpstr);

//        tmpstr =  qPrintable( qtstr.setNum(seed_win).prepend("#seed_win = ") ); infostring.push_back(tmpstr);
//        tmpstr =  qPrintable( qtstr.setNum(slip_win).prepend("#slip_win = ") ); infostring.push_back(tmpstr);


        if(!cb->callPluginFunc(full_plugin_name,func_name,input,output))
        {

             printf("Can not find the tracing plugin!\n");
             return;
        }

        NeuronTree nt_most;
        QString swcMOST = saveDirString;
        swcMOST.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".v3draw_MOST.swc");

        nt_most = readSWC_file(swcMOST);


        LandmarkList imageLandmarks;
        int maxRootListSize;
        maxRootListSize = inputRootList.size();

        QList<ImageMarker> seedsToSave;
        for (int i = 0; i<maxRootListSize; i++){
            LocationSimple RootNewLocation;
            ImageMarker outputMarker;
            RootNewLocation.x = inputRootList.at(i).x - total4DImage->getOriginX();
            RootNewLocation.y = inputRootList.at(i).y - total4DImage->getOriginY();
            RootNewLocation.z = inputRootList.at(i).z - total4DImage->getOriginZ();
            imageLandmarks.append(RootNewLocation);
            outputMarker.x = inputRootList.at(i).x  - total4DImage->getOriginX();
            outputMarker.y = inputRootList.at(i).y - total4DImage->getOriginY();
            outputMarker.z = inputRootList.at(i).z - total4DImage->getOriginZ();
            seedsToSave.append(outputMarker);

        }

        QString markerSaveString;
        markerSaveString = swcString;
        markerSaveString.append(".marker");
        writeMarker_file(markerSaveString, seedsToSave);

        NeuronTree nt;
        nt = sort_eliminate_swc(nt_most,inputRootList,total4DImage,isSoma);

        export_list2file(nt.listNeuron, swcString,swcMOST);

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
            NeuronSWC curr = list.at(i);
            LocationSimple newTip;
            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                newTip.x = curr.x + total4DImage->getOriginX();
                newTip.y = curr.y + total4DImage->getOriginY();
                newTip.z = curr.z + total4DImage->getOriginZ();
            }
            if( curr.x < 0.05* total4DImage->getXDim())
            {
                tip_left.push_back(newTip);
            }else if (curr.x > 0.95 * total4DImage->getXDim())
            {
                tip_right.push_back(newTip);
            }else if (curr.y < 0.05 * total4DImage->getYDim())
            {
                tip_up.push_back(newTip);
            }else if (curr.y > 0.95*total4DImage->getYDim())
            {
                tip_down.push_back(newTip);
            }

        }

        QList<LandmarkList> newTipsList;
        LocationSimple newTarget;

        if(tip_left.size()>0)
        {
            newTipsList.push_back(tip_left);
            newTarget.x = -total4DImage->getXDim();
            newTarget.y = 0;
            newTarget.z = 0;
            newTargetList.push_back(newTarget);
        }
        if(tip_right.size()>0)
        {
            newTipsList.push_back(tip_right);
            newTarget.x = total4DImage->getXDim();
            newTarget.y = 0;
            newTarget.z = 0;
            newTargetList.push_back(newTarget);
        }
        if(tip_up.size()>0)
        {
            newTipsList.push_back(tip_up);
            newTarget.x = 0;
            newTarget.y = -total4DImage->getYDim();
            newTarget.z = 0;
            newTargetList.push_back(newTarget);
        }
        if(tip_down.size()>0)
        {
            newTipsList.push_back(tip_down);
            newTarget.x = 0;
            newTarget.y = total4DImage->getYDim();
            newTarget.z = 0;
            newTargetList.push_back(newTarget);
        }

        if (!newTargetList.empty())
        {
            for (int i = 0; i<newTargetList.length(); i++)
            {
                newTargetList[i].x = (1.0-overlap)*newTargetList[i].x+total4DImage->getOriginX();
                newTargetList[i].y=(1.0-overlap)*newTargetList[i].y+total4DImage->getOriginY();
                newTargetList[i].z =(1.0-overlap)*newTargetList[i].z+total4DImage->getOriginZ();
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

void StackAnalyzer::loadScan_MOST_adaptive(QString latestString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation, QString saveDirString, bool useGSDT, bool isSoma)
{
    qDebug()<<"loadScan input: "<<latestString;
    qDebug()<<"overlap input:"<< QString::number(overlap);
    //QString latestString = getFileString();

    // Zhi:  this is a stack on AIBSDATA/MAT
    // modify as needed for your local path!

    int seed_win = 10;
    int slip_win = 10;

    //debug...
//    isSoma = true;
//    latestString =QString("/data/mat/BRL/testData/ZSeries-02232016-0931-2797/ZSeries-02232016-0931-2797_Cycle00001_Ch1_000001.ome.tif");
//    saveDirString = "/opt/zhi/Desktop/super_plugin_test/2016_02_05_Fri_13_52/2016_03_25_Fri_09_35";


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
        filterList.append(QString("*").append(channel).append("*.tif"));
        imageDir.setNameFilters(filterList);
        QStringList fileList = imageDir.entryList();

        //get the parent dir and the list of ch1....ome.tif files
        //use this to id the number of images in the stack (in one channel?!)
        V3DLONG x = pNewImage->getXDim();
        V3DLONG y = pNewImage->getYDim();
        V3DLONG nFrames = fileList.length();


        tileLocation.ev_pc1 = x;
        tileLocation.ev_pc2 = y;

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

        outputStream<< (int) total4DImage->getOriginX()<<" "<< (int) total4DImage->getOriginY()<<" "<<swcString<<" "<< (int) x<<" "<< (int) y<<"\n";

        V3DLONG mysz[4];
        mysz[0] = total4DImage->getXDim();
        mysz[1] = total4DImage->getYDim();
        mysz[2] = total4DImage->getZDim();
        mysz[3] = total4DImage->getCDim();
        QString imageSaveString = saveDirString;

//        imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".v3draw");
//        simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, V3D_UINT16);

        //        if(total1dData) {delete []total1dData; total1dData = 0;}

        //convert to 8bit image using 8 shiftnbits

        QString finaloutputswc = saveDirString + ("/most_adaptive.swc");
        ifstream ifs_swc(finaloutputswc.toStdString().c_str());
        vector<MyMarker*> finalswc;

        if(ifs_swc)
           finalswc = readSWC_file(finaloutputswc.toStdString());

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

     //   Image4DSimple* total4DImage = new Image4DSimple;

        total4DImage->setData((unsigned char*)total1dData_8bit, x, y, nFrames, 1, V3D_UINT8);

        imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".v3draw");
        simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData_8bit, mysz, V3D_UINT8);

        //convert to 8bit image using 1percentage saturation
//        double apercent = 0.01;
//        if(isSoma) apercent = 0.05;
//        V3DLONG maxvv = ceil(p_vmax+1);
//        double *hist = 0;
//        try
//        {
//            hist = new double [maxvv];
//        }
//        catch (...)
//        {
//            qDebug() << "fail to allocate"; return;
//            v3d_msg("Fail to allocate memory in proj_general_scaleandconvert28bit_1percentage().\n");
//            return;
//        }

//        for (V3DLONG i=0;i<maxvv;i++)
//        {
//            hist[i] = 0;
//        }
//        //find the histogram
//        for (V3DLONG i=0;i<tunits;i++)
//        {
//            hist[total1dData[i]] += 1;
//        }
//        qDebug() << "Histogram computed.";

//        //compute the CDF
//        for (V3DLONG i=1;i<maxvv;i++)
//        {
//            hist[i] += hist[i-1];
//        }
//        for (V3DLONG i=0;i<maxvv;i++)
//        {
//            hist[i] /= hist[maxvv-1];
//        }
//        //now search for the intensity thresholds
//        double lowerth, upperth; lowerth = upperth = 0;
//        for (V3DLONG i=0;i<maxvv-1;i++) //not the most efficient method, but the code should be readable
//        {
//            if (hist[i]<apercent && hist[i+1]>apercent)
//                lowerth = i;
//            if (hist[i]<1-apercent && hist[i+1]>1-apercent)
//                upperth = i;
//        }

//        //real rescale of intensity
//        scaleintensity(total4DImage,0, lowerth, upperth, double(0), double(255));

//        //free space
//        if (hist) {delete []hist; hist=0;}

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

        string S_background_th = boost::lexical_cast<string>(background);
        char* C_background_th = new char[S_background_th.length() + 1];
        strcpy(C_background_th,S_background_th.c_str());
        arg_para.push_back(C_background_th);

        string S_seed_win = boost::lexical_cast<string>(seed_win);
        char* C_seed_win = new char[S_seed_win.length() + 1];
        strcpy(C_seed_win,S_seed_win.c_str());
        arg_para.push_back(C_seed_win);

        string S_slip_win = boost::lexical_cast<string>(slip_win);
        char* C_slip_win = new char[S_slip_win.length() + 1];
        strcpy(C_slip_win,S_slip_win.c_str());
        arg_para.push_back(C_slip_win);

        full_plugin_name = "mostVesselTracer";  func_name =  "MOST_trace";
        arg.p = (void *) & arg_para; input << arg;


        qDebug()<<"starting most";
        qDebug()<<"rootlist size "<<QString::number(inputRootList.size());

//        list<string> infostring;
//        string tmpstr; QString qtstr;
//        tmpstr =  qPrintable( qtstr.prepend("## NeuronCrawler_MOST")); infostring.push_back(tmpstr);
//        tmpstr =  qPrintable( qtstr.setNum(0).prepend("#channel = ") ); infostring.push_back(tmpstr);
//        tmpstr =  qPrintable( qtstr.setNum(bkg_thresh).prepend("#bkg_thresh = ") ); infostring.push_back(tmpstr);

//        tmpstr =  qPrintable( qtstr.setNum(seed_win).prepend("#seed_win = ") ); infostring.push_back(tmpstr);
//        tmpstr =  qPrintable( qtstr.setNum(slip_win).prepend("#slip_win = ") ); infostring.push_back(tmpstr);


        if(!cb->callPluginFunc(full_plugin_name,func_name,input,output))
        {

             printf("Can not find the tracing plugin!\n");
             return;
        }

        NeuronTree nt_most;
        QString swcMOST = saveDirString;
        swcMOST.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(".v3draw_MOST.swc");

        nt_most = readSWC_file(swcMOST);

        if(nt_most.size()<1)
            return;

        NeuronTree nt = sort_eliminate_swc(nt_most,inputRootList,total4DImage,isSoma);
        export_list2file(nt.listNeuron, swcString,swcMOST);

        LandmarkList imageLandmarks;
        int maxRootListSize;
        maxRootListSize = inputRootList.size();

        QList<ImageMarker> seedsToSave;
        for (int i = 0; i<maxRootListSize; i++){
            LocationSimple RootNewLocation;
            ImageMarker outputMarker;
            RootNewLocation.x = inputRootList.at(i).x - total4DImage->getOriginX();
            RootNewLocation.y = inputRootList.at(i).y - total4DImage->getOriginY();
            RootNewLocation.z = inputRootList.at(i).z - total4DImage->getOriginZ();
            imageLandmarks.append(RootNewLocation);
            outputMarker.x = inputRootList.at(i).x  - total4DImage->getOriginX();
            outputMarker.y = inputRootList.at(i).y - total4DImage->getOriginY();
            outputMarker.z = inputRootList.at(i).z - total4DImage->getOriginZ();
            seedsToSave.append(outputMarker);

        }

        QString markerSaveString;
        markerSaveString = swcString;
        markerSaveString.append(".marker");
        writeMarker_file(markerSaveString, seedsToSave);



        LandmarkList tip_left;
        LandmarkList tip_right;
        LandmarkList tip_up ;
        LandmarkList tip_down;
        QList<NeuronSWC> list = nt.listNeuron;
        for (V3DLONG i=0;i<list.size();i++)
        {
            NeuronSWC curr = list.at(i);
            LocationSimple newTip;
            bool check_tip = false;

            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                newTip.x = curr.x + total4DImage->getOriginX();
                newTip.y = curr.y + total4DImage->getOriginY();
                newTip.z = curr.z + total4DImage->getOriginZ();

                for(V3DLONG j = 0; j < finalswc.size(); j++ )
                {
                    double dis = sqrt(pow2(newTip.x - finalswc.at(j)->x) + pow2(newTip.y - finalswc.at(j)->y) + pow2(newTip.z - finalswc.at(j)->z));
                    if(dis < 10)
                    {
                        check_tip = true;
                        break;
                    }
                }

                if(check_tip) continue;
                if( curr.x < 0.05* total4DImage->getXDim())
                {
                    tip_left.push_back(newTip);
                }else if (curr.x > 0.95 * total4DImage->getXDim())
                {
                    tip_right.push_back(newTip);
                }else if (curr.y < 0.05 * total4DImage->getYDim())
                {
                    tip_up.push_back(newTip);
                }else if (curr.y > 0.95*total4DImage->getYDim())
                {
                    tip_down.push_back(newTip);
                }
            }
        }

        QList<LandmarkList> newTipsList;
        LandmarkList newTargetList;


        if(tip_left.size()>0)
        {
            QList<LandmarkList> group_tips_left = group_tips(tip_left,50,1);
            for(int i = 0; i < group_tips_left.size();i++)
                ada_win_finding(group_tips_left.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,tileLocation.ev_pc1,1,overlap);
        }
        if(tip_right.size()>0)
        {
            QList<LandmarkList> group_tips_right = group_tips(tip_right,50,2);
            for(int i = 0; i < group_tips_right.size();i++)
                ada_win_finding(group_tips_right.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,tileLocation.ev_pc1,2,overlap);
        }
        if(tip_up.size()>0)
        {
            QList<LandmarkList> group_tips_up = group_tips(tip_up,50,3);
            for(int i = 0; i < group_tips_up.size();i++)
                ada_win_finding(group_tips_up.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,tileLocation.ev_pc1,3,overlap);

        }
        if(tip_down.size()>0)
        {
            QList<LandmarkList> group_tips_down = group_tips(tip_down,50,4);
            for(int i = 0; i < group_tips_down.size();i++)
                ada_win_finding(group_tips_down.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,tileLocation.ev_pc1,4,overlap);
        }

        vector<MyMarker*> tileswc_file = readSWC_file(swcString.toStdString());

        if(ifs_swc)
        {
            for(V3DLONG i = 0; i < tileswc_file.size(); i++)
            {
                tileswc_file[i]->x = tileswc_file[i]->x + total4DImage->getOriginX();
                tileswc_file[i]->y = tileswc_file[i]->y + total4DImage->getOriginY();
                tileswc_file[i]->z = tileswc_file[i]->z + total4DImage->getOriginZ();

                finalswc.push_back(tileswc_file[i]);
            }
            saveSWC_file(finaloutputswc.toStdString().c_str(), finalswc);
        }
        else
        {
            for(V3DLONG i = 0; i < tileswc_file.size(); i++)
            {
                tileswc_file[i]->x = tileswc_file[i]->x + total4DImage->getOriginX();
                tileswc_file[i]->y = tileswc_file[i]->y + total4DImage->getOriginY();
                tileswc_file[i]->z = tileswc_file[i]->z + total4DImage->getOriginZ();
            }
            saveSWC_file(finaloutputswc.toStdString().c_str(), tileswc_file);
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
void StackAnalyzer::updateChannel(QString inputChannel){
    channel = inputChannel;
}

NeuronTree StackAnalyzer::sort_eliminate_swc(NeuronTree nt,LandmarkList inputRootList,Image4DSimple* total4DImage,bool isSoma)
{
    NeuronTree nt_result;
    QList<NeuronSWC> neuron_sorted;

    if (!SortSWC(nt.listNeuron, neuron_sorted,VOID, 10))
    {
        v3d_msg("fail to call swc sorting function.",0);
        return nt_result;
    }

    V3DLONG neuronNum = neuron_sorted.size();
    V3DLONG *flag = new V3DLONG[neuronNum];
    if(isSoma)
    {
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            flag[i] = 1;
        }
    }
    else
    {
        for (V3DLONG i=0;i<neuronNum;i++)
        {
            flag[i] = 0;
        }

        int root_index = -1;
        for(V3DLONG i = 0; i<inputRootList.size();i++)
        {
            int marker_x = inputRootList.at(i).x - total4DImage->getOriginX();
            int marker_y = inputRootList.at(i).y - total4DImage->getOriginY();
            int marker_z = inputRootList.at(i).z - total4DImage->getOriginZ();

            for(V3DLONG j = 0; j<neuronNum;j++)
            {
                NeuronSWC curr = neuron_sorted.at(j);
                if(curr.pn < 0) root_index = j;
                double dis = sqrt(pow2(marker_x - curr.x) + pow2(marker_y - curr.y) + pow2(marker_z - curr.z));

                if(dis < 20 && flag[j] ==0)
                {
                    flag[root_index] = 1;
                    V3DLONG d;
                    for( d = root_index+1; d < neuronNum; d++)
                    {
                        if(neuron_sorted.at(d).pn < 0)
                            break;
                        else
                            flag[d] = 1;

                    }
                    break;
                }

            }
        }
    }

    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    //set node
    NeuronSWC S;
    for (V3DLONG i=0;i<neuron_sorted.size();i++)
    {
        if(flag[i] == 1)
        {
            NeuronSWC curr = neuron_sorted.at(i);
            S.n 	= curr.n;
            S.type 	= curr.type;
            S.x 	= curr.x;
            S.y 	= curr.y;
            S.z 	= curr.z;
            S.r 	= curr.r;
            S.pn 	= curr.pn;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
        }
    }
    nt_result.n = -1;
    nt_result.on = true;
    nt_result.listNeuron = listNeuron;
    nt_result.hashNeuron = hashNeuron;
    if(flag) {delete[] flag; flag = 0;}

    return nt_result;
}

void StackAnalyzer::ada_win_finding(LandmarkList tips,LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList,Image4DSimple* total4DImage,int block_size,int direction, float overlap)
{
    newTipsList->push_back(tips);
    float min_y = INF, max_y = -INF;
    float min_x = INF, max_x = -INF;
    double adaptive_size;

    if(direction == 1 || direction == 2)
    {
        for(int i = 0; i<tips.size();i++)
        {
            if(tips.at(i).y <= min_y) min_y = tips.at(i).y;
            if(tips.at(i).y >= max_y) max_y = tips.at(i).y;
        }
        adaptive_size = (max_y - min_y)*1.2;

    }else
    {
        for(int i = 0; i<tips.size();i++)
        {
            if(tips.at(i).x <= min_x) min_x = tips.at(i).x;
            if(tips.at(i).x >= max_x) max_x = tips.at(i).x;
        }
        adaptive_size = (max_x - min_x)*1.2;
    }

    if(adaptive_size <= 50) adaptive_size = 50;
    if(adaptive_size >= block_size) adaptive_size = block_size;

    LocationSimple newTarget;

    if(direction == 1)
    {
        newTarget.x = -floor(adaptive_size*(1.0-overlap)) + tileLocation.x;
        newTarget.y = floor((min_y + max_y -adaptive_size)/2.0 - total4DImage->getOriginY()) + tileLocation.y;

    }else if(direction == 2)
    {
        newTarget.x = tileLocation.x + tileLocation.ev_pc1 - floor(adaptive_size*overlap);
        newTarget.y = floor((min_y + max_y -adaptive_size)/2.0 - total4DImage->getOriginY()) + tileLocation.y;

    }else if(direction == 3)
    {
        newTarget.x = floor((min_x + max_x - adaptive_size)/2) - total4DImage->getOriginX() + tileLocation.x;
        newTarget.y = -floor(adaptive_size*(1.0-overlap)) + tileLocation.y;
    }else
    {
        newTarget.x = floor((min_x + max_x - adaptive_size)/2) - total4DImage->getOriginX() + tileLocation.x;
        newTarget.y = tileLocation.y + tileLocation.ev_pc2 - floor(adaptive_size*overlap);
    }
    newTarget.z = total4DImage->getOriginZ();
    newTarget.ev_pc1 = adaptive_size;
    newTarget.ev_pc2 = adaptive_size;

    newTargetList->push_back(newTarget);
    return;
}

QList<LandmarkList> StackAnalyzer::group_tips(LandmarkList tips,int min_size, int direction)
{
    QList<LandmarkList> groupTips;

   //bubble sort
   if(direction == 1 || direction == 2)
   {
       for(int i = 0; i < tips.size();i++)
       {
           for(int j = 0; j < tips.size();j++)
           {
               if(tips.at(i).y < tips.at(j).y)
                   tips.swap(i,j);
           }
       }

       LandmarkList eachGroupList;
       eachGroupList.push_back(tips.at(0));
       for(int d = 0; d < tips.size()-1; d++)
       {
           if(tips.at(d+1).y - tips.at(d).y < min_size)
           {
               eachGroupList.push_back(tips.at(d+1));
           }
           else
           {
               groupTips.push_back(eachGroupList);
               eachGroupList.erase(eachGroupList.begin(),eachGroupList.end());
               eachGroupList.push_back(tips.at(d+1));
           }
       }
       groupTips.push_back(eachGroupList);
   }else
   {
       for(int i = 0; i < tips.size();i++)
       {
           for(int j = 0; j < tips.size();j++)
           {
               if(tips.at(i).x < tips.at(j).x)
                   tips.swap(i,j);
           }
       }

       LandmarkList eachGroupList;
       eachGroupList.push_back(tips.at(0));
       for(int d = 0; d < tips.size()-1; d++)
       {
           if(tips.at(d+1).x - tips.at(d).x < min_size)
           {
               eachGroupList.push_back(tips.at(d+1));
           }
           else
           {
               groupTips.push_back(eachGroupList);
               eachGroupList.erase(eachGroupList.begin(),eachGroupList.end());
               eachGroupList.push_back(tips.at(d+1));

           }
       }
       groupTips.push_back(eachGroupList);
   }
   return groupTips;
}

bool StackAnalyzer::combine_list2file(QList<NeuronSWC> & lN, QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::Text|QIODevice::Append))
        return false;
    QTextStream myfile(&file);
    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
};

LandmarkList StackAnalyzer::eliminate_seed(NeuronTree nt,LandmarkList inputRootList,Image4DSimple* total4DImage)
{
    LandmarkList inputRootList_pruned;
    V3DLONG neuronNum = nt.listNeuron.size();


    for(V3DLONG i = 0; i<inputRootList.size();i++)
    {
        int marker_x = inputRootList.at(i).x - total4DImage->getOriginX();
        int marker_y = inputRootList.at(i).y - total4DImage->getOriginY();
        int marker_z = inputRootList.at(i).z - total4DImage->getOriginZ();

        bool flag = false;
        for(V3DLONG j = 0; j<neuronNum;j++)
        {
            NeuronSWC curr = nt.listNeuron.at(j);
            double dis = sqrt(pow2(marker_x - curr.x) + pow2(marker_y - curr.y) + pow2(marker_z - curr.z));

            if(dis < 20)
            {
                flag = true;
                break;
            }

        }
        if(!flag)
            inputRootList_pruned.push_back(inputRootList.at(i));

    }
    return inputRootList_pruned;
}
