#include "stackAnalyzer.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include <fstream>
#include <iostream>
#include <sstream>

#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"
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
}


StackAnalyzer::StackAnalyzer(V3DPluginCallback2 &callback)
{
    cb = &callback;
    channel = QString("Ch2");
    redThreshold = 0;
    redAlpha = 1.0;
    lipofuscinMethod = 0; // lipofuscinMethod   = 0 :  subtract using alpha value to scale red channel
    //                    = 1 :  set green channel to zero wherever red channel is above redThreshold
    globalMaxBlockSize = 180;
    globalMinBlockSize = 100;
    radius = 20;
}

void StackAnalyzer::updateChannel(QString newChannel){
    channel = newChannel;
}

void StackAnalyzer::updateRedThreshold(int rThresh){
    redThreshold = rThresh;

}

void StackAnalyzer::updateRedAlpha(float rAlpha){
    redAlpha=rAlpha;
}

void StackAnalyzer::updateLipoMethod(int lipoMethod){
    lipofuscinMethod = lipoMethod;
}

void StackAnalyzer::updateGlobalMinMaxBlockSizes(int newMinBlockSize, int newMaxBlockSize){
    globalMinBlockSize = newMinBlockSize;
    globalMaxBlockSize = newMaxBlockSize;
}

void StackAnalyzer::updateSearchRadius(double inputRadius){
    radius = inputRadius;
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
        //use this to id the number of images in the stack (in one channel)
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
                emit loadingDone(total4DImage_mip);
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

        imageSaveString.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.baseName()).append(channel).append(".ome.tif.v3draw");
        simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData, mysz, V3D_UINT16);

        emit loadingDone(total4DImage_mip);


    }else{  //initial string is not readable
        qDebug()<<QString("invalid image path: ").append(latestString);
    }
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

    QString lastImagepath = folderpath + "/" +   QFileInfo(swcfilepath2).baseName().append(".ome.tif").append(channel).append(".v3draw");
    qDebug()<<lastImagepath;
    unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    int datatype;
    if(!simple_loadimage_wrapper(*cb, lastImagepath.toStdString().c_str(), data1d, in_sz, datatype))
    {
        cerr<<"load image "<<lastImagepath.toStdString()<<" error!"<<endl;
        emit combinedSWC(fileSaveName);
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

                if(dis < radius && flag[j] ==0)
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

NeuronTree StackAnalyzer::generate_crossing_swc(Image4DSimple* total4DImage)
{
    NeuronTree nt_result;

    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    //set center node
    NeuronSWC S;
    S.n 	= 1;
    S.type 	= 2;
    S.x 	= total4DImage->getXDim()/2;
    S.y 	= total4DImage->getYDim()/2;
    S.z 	= total4DImage->getZDim()/2;
    S.r 	= 1;
    S.pn    = -1;
    listNeuron.append(S);
    hashNeuron.insert(S.n, listNeuron.size()-1);

    //set left node
    S.n 	= 2;
    S.x 	= 0;
    S.pn    = 1;
    listNeuron.append(S);
    hashNeuron.insert(S.n, listNeuron.size()-1);

    //set right node
    S.n 	= 3;
    S.x 	= total4DImage->getXDim()-1;
    listNeuron.append(S);
    hashNeuron.insert(S.n, listNeuron.size()-1);

    //set up node
    S.n 	= 4;
    S.x 	= total4DImage->getXDim()/2;
    S.y 	= 0;
    listNeuron.append(S);
    hashNeuron.insert(S.n, listNeuron.size()-1);

    //set down node
    S.n 	= 5;
    S.y 	= total4DImage->getYDim()-1;
    listNeuron.append(S);
    hashNeuron.insert(S.n, listNeuron.size()-1);

    nt_result.n = -1;
    nt_result.on = true;
    nt_result.listNeuron = listNeuron;
    nt_result.hashNeuron = hashNeuron;

    return nt_result;
}

void StackAnalyzer::ada_win_finding(LandmarkList tips,LocationSimple tileLocation,LandmarkList *newTargetList,QList<LandmarkList> *newTipsList,Image4DSimple* total4DImage,int max_block_size,int direction, float overlap, int min_block_size){
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
        adaptive_size = (max_y - min_y)*(1.0+3.0*overlap);

    }else
    {
        for(int i = 0; i<tips.size();i++)
        {
            if(tips.at(i).x <= min_x) min_x = tips.at(i).x;
            if(tips.at(i).x >= max_x) max_x = tips.at(i).x;
        }
        adaptive_size = (max_x - min_x)*(1.0+3.0*overlap);
    }

    if(adaptive_size <= globalMinBlockSize) adaptive_size = globalMinBlockSize;
    if(adaptive_size >= max_block_size) adaptive_size = max_block_size;

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
}

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


void StackAnalyzer::startTracing(QString latestString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation, QString saveDirString, bool useGSDT, bool isSoma, bool isAdaptive, int methodChoice, int tileStatus){

    QDir saveDir(saveDirString);
    if (tileStatus!=-1){
        qDebug()<<"loadScan input: "<<latestString;
        qDebug()<<"isAdaptive "<<isAdaptive;
        qDebug()<<"methodChoice "<<methodChoice;
        qDebug()<<"tileStatus "<<tileStatus;
    }
    QList<LandmarkList> newTipsList;
    LandmarkList newTargetList;

    QFileInfo imageFileInfo = QFileInfo(latestString);
    Image4DSimple* total4DImage = new Image4DSimple;
    Image4DSimple* total4DImage_mip = new Image4DSimple;
    QString imageSaveString = saveDirString;
    QString swcString = saveDirString;


    QString v3drawFile;


    if (tileStatus==1){ // already scanned, already have .swc file
        QString othercopy = saveDirString;
        v3drawFile = imageFileInfo.completeBaseName();
        othercopy.append(QDir::separator()).append(v3drawFile).append(".swc");
        qDebug()<<"othercopy "<<othercopy;
        imageSaveString.append(QDir::separator()).append(v3drawFile.append(".v3draw")); // .v3draw file must exist [ don't do anything stupid to make this not true!]
        swcString = othercopy;


    } else if (tileStatus == 0){ // hasn't been loaded or traced yet
        imageSaveString.append(QDir::separator()).append("x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(channel).append(".v3draw");
        swcString.append(QDir::separator()).append("x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(channel).append(".swc");
    }else if (tileStatus == -1){ // waiting for loading and tracing to finish
        tileLocation.name = latestString.toStdString();
        imageSaveString = latestString;
        newTargetList.append(tileLocation);
        newTipsList.append(inputRootList);
        total4DImage_mip->deleteRawDataAndSetPointerToNull();

        emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave,imageSaveString, tileStatus);
        return;
    }



    qDebug()<<"swcString= "<<swcString;
    qDebug()<<"imageSaveString= "<<imageSaveString;


    QFileInfo imInfo(imageSaveString);



    if(!imInfo.exists()){ // image file isn't readable, this should ONLY HAPPEN if it hasn't been converted to v3draw,
        //  so  we need to   read in the new image data, save to .v3draw and prepare pointer to pass to tracing methods


        QStringList fileList;
        Image4DSimple * pNewImage = cb->loadImage(latestString.toLatin1().data());
        QDir imageDir =  imageFileInfo.dir();
        QStringList filterList;
        filterList.append(QString("*").append("Ch1").append("*.tif"));
        imageDir.setNameFilters(filterList);
        QStringList fileList1 = imageDir.entryList();

        QStringList filterList2;
        filterList2.append(QString("*").append("Ch2").append("*.tif"));
        imageDir.setNameFilters(filterList2);
        QStringList fileList2 = imageDir.entryList();

        if (channel=="Ch1"){
            fileList = fileList1;
        }else if (channel=="Ch2"){
            fileList = fileList2;
        }

        //use this to id the number of images in the stack (in one channel?!)
        V3DLONG x = pNewImage->getXDim();
        V3DLONG y = pNewImage->getYDim();
        V3DLONG nFrames = fileList1.length();

        V3DLONG tunits = x*y*nFrames;
        unsigned short int * total1dData = new unsigned short int [tunits];
        unsigned short int * total1dData_mip= new unsigned short int [x*y];
        for(V3DLONG i =0 ; i < x*y; i++)
            total1dData_mip[i] = 0;
        V3DLONG totalImageIndex = 0;
        double p_vmax=0;
        qDebug()<<channel;
        for (int f=0; f<nFrames; f++){
            //qDebug()<<fileList[f];
            if (channel=="G-R") {
                Image4DSimple * pNewImage1 = cb->loadImage(imageDir.absoluteFilePath(fileList1[f]).toLatin1().data());
                Image4DSimple * pNewImage2 = cb->loadImage(imageDir.absoluteFilePath(fileList2[f]).toLatin1().data());

                if (pNewImage1->valid()){
                    unsigned short int * data1d1 = 0;
                    data1d1 = new unsigned short int [x*y];
                    data1d1 = (unsigned short int*)pNewImage1->getRawData();
                    unsigned short int * data1d2 = 0;
                    data1d2 = new unsigned short int [x*y];
                    data1d2 = (unsigned short int*)pNewImage2->getRawData();
                    if (lipofuscinMethod==0){  // Green - alpha*Red
                        for (V3DLONG i = 0; i< (x*y); i++)
                        {
                            if (data1d1[i] >= data1d2[i]){
                                total1dData[totalImageIndex]= 0;

                            }else{
                                float tmp = (float)(data1d2[i])-(float)(data1d1[i])*redAlpha;

                                if (tmp<0) total1dData[totalImageIndex]=0;
                                else total1dData[totalImageIndex]= (unsigned short int) tmp;

                            }
                            if(total1dData[totalImageIndex] > p_vmax) p_vmax = total1dData[totalImageIndex];
                            if(total1dData_mip[i] < total1dData[totalImageIndex]) total1dData_mip[i] = total1dData[totalImageIndex];
                            totalImageIndex++;
                        }

                    } else if (lipofuscinMethod==1){ //obscuration

                        for (V3DLONG i = 0; i< (x*y); i++)
                        {
                            if ((data1d1[i] >= data1d2[i])|(data1d1[i]>redThreshold)){
                                total1dData[totalImageIndex]= 0;

                            }else{
                                total1dData[totalImageIndex]= data1d2[i];

                            }
                            if(total1dData[totalImageIndex] > p_vmax) p_vmax =total1dData[totalImageIndex];
                            if(total1dData_mip[i] < total1dData[totalImageIndex]) total1dData_mip[i] = total1dData[totalImageIndex];
                            totalImageIndex++;
                        }



                    }
                    if(data1d1) {delete []data1d1; data1d1 = 0;}
                    if(data1d2) {delete []data1d2; data1d2 = 0;}

                }else{
                    qDebug()<<imageDir.absoluteFilePath(fileList[f])<<" failed!";
                }

            }else{

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

        }

        total4DImage->setData((unsigned char*)total1dData, x, y, nFrames, 1, V3D_UINT16);
        total4DImage_mip->setData((unsigned char*)total1dData_mip, x, y, 1, 1, V3D_UINT16);
        total4DImage->setOriginX(tileLocation.x);
        total4DImage->setOriginY(tileLocation.y);

        if (tileStatus==0){
            QString scanDataFileString = saveDirString;
            scanDataFileString.append("/").append("scanData.txt");
            qDebug()<<scanDataFileString;
            QFile saveTextFile;
            saveTextFile.setFileName(scanDataFileString);// add currentScanFile
            if (!saveTextFile.isOpen()){
                if (!saveTextFile.open(QIODevice::Text|QIODevice::Append  )){
                    qDebug()<<"unable to save file!";
                    total4DImage_mip->deleteRawDataAndSetPointerToNull();
                    emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave,imageSaveString, tileStatus);
                    return;}     }
            QTextStream outputStream;
            outputStream.setDevice(&saveTextFile);


            outputStream<< (int) total4DImage->getOriginX()<<" "<< (int) total4DImage->getOriginY()<<" "<<swcString<<" "<< (int) x<<" "<< (int) y<<" "<< (int) methodChoice<<" "<< (int) isAdaptive<<"\n";

            saveTextFile.close();

        }

        V3DLONG mysz[4];
        mysz[0] = total4DImage->getXDim();
        mysz[1] = total4DImage->getYDim();
        mysz[2] = total4DImage->getZDim();
        mysz[3] = total4DImage->getCDim();

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
            total4DImage_mip->deleteRawDataAndSetPointerToNull();
            emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave, imageSaveString, tileStatus);

            return;
        }
        double dn = pow(2.0, double(5));
        for (V3DLONG i=0;i<tunits;i++)
        {
            double tmp = (double)(total1dData[i]) / dn;
            if (tmp>255) total1dData_8bit[i] = 255;
            else if (tmp<1) total1dData_8bit[i] = 0;
            else
                total1dData_8bit[i] = (unsigned char)(tmp);
        }

        total4DImage->setData((unsigned char*)total1dData_8bit, x, y, nFrames, 1, V3D_UINT8);
        simple_saveimage_wrapper(*cb, imageSaveString.toLatin1().data(),(unsigned char *)total1dData_8bit, mysz, V3D_UINT8);
    }else{ // image file IS readable:

        unsigned char* data1d = 0;
        total4DImage->deleteRawDataAndSetPointerToNull();
        V3DLONG in_sz[4];
        int datatype = 0;
        if (!simple_loadimage_wrapper(*cb,imageSaveString.toLatin1().data(), data1d, in_sz, datatype))
        {
            qDebug()<<"unable to load file in stackAnalyzer::startTracing";
            qDebug()<<imageSaveString;
            total4DImage_mip->deleteRawDataAndSetPointerToNull();

            emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave, imageSaveString, tileStatus);
            return;
        }
        total4DImage->setData((unsigned char*)data1d, in_sz[0], in_sz[1], in_sz[2], 1, V3D_UINT8);

    }
    total4DImage->setOriginX(tileLocation.x);
    total4DImage->setOriginY(tileLocation.y);

    tileLocation.name = imageSaveString.toStdString(); // this is critical-  the outgoing tileLocation will be linked back to the .v3draw file location which is either
    // the new .v3draw file that was just read in for the first time
    // OR
    // the .v3draw file that matches the location information of this tile's .swc file

    if(methodChoice ==-1)
    {
        methodChoice = methodSelection(total4DImage,inputRootList, background, isSoma);//select different methods
    }

    qDebug()<<"=== immediately before tracing =====";
    qDebug()<<"isAdaptive "<<isAdaptive;
    qDebug()<<"methodChoice "<<methodChoice;
    qDebug()<<"tileLocation.name "<<QString::fromStdString(tileLocation.name);


    if(isAdaptive)
    {
        if(methodChoice == 2){
            APP2Tracing_adaptive(total4DImage, total4DImage_mip, swcString, overlap, background, interrupt, inputRootList, tileLocation, saveDirString,useGSDT, isSoma,imageSaveString,tileStatus);
            return;
        }
        else{
            SubtractiveTracing_adaptive(latestString,imageSaveString, total4DImage, total4DImage_mip, swcString,overlap, background, interrupt,  inputRootList, tileLocation, saveDirString,useGSDT, isSoma, methodChoice,tileStatus);
            return;
        }
    }
    else
    {
        if(methodChoice == 2){
            APP2Tracing(total4DImage, total4DImage_mip, swcString, overlap, background, interrupt, inputRootList, useGSDT, isSoma, tileLocation,imageSaveString,tileStatus);
            return;
        }    else{
            SubtractiveTracing(latestString,imageSaveString, total4DImage, total4DImage_mip, swcString,overlap, background, interrupt,  inputRootList, tileLocation, saveDirString,useGSDT, isSoma, methodChoice,tileStatus);
            return;}

    }

}

void StackAnalyzer::APP2Tracing(Image4DSimple* total4DImage, Image4DSimple* total4DImage_mip, QString swcString, float overlap, int background, bool interrupt, LandmarkList inputRootList, bool useGSDT, bool isSoma, LocationSimple tileLocation, QString tileSaveString,int tileStatus)
{
    QList<LandmarkList> newTipsList;
    LandmarkList newTargetList;

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
        vector<MyMarker*> tileswc_file;
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
        if (tileStatus==1) markerSaveString.append("D");
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
        NeuronSWC curr = list.at(i);
        LocationSimple newTip;
        if( curr.x < 0.05*  p.p4dImage->getXDim() || curr.x > 0.95 *  p.p4dImage->getXDim() || curr.y < 0.05 * p.p4dImage->getYDim() || curr.y > 0.95* p.p4dImage->getYDim())
        {
            newTip.x = curr.x + p.p4dImage->getOriginX();
            newTip.y = curr.y + p.p4dImage->getOriginY();
            newTip.z = curr.z + p.p4dImage->getOriginZ();
        }
        if( curr.x < 0.05* p.p4dImage->getXDim())
        {
            tip_left.push_back(newTip);
        }
        if (curr.x > 0.95 * p.p4dImage->getXDim())
        {
            tip_right.push_back(newTip);
        }
        if (curr.y < 0.05 * p.p4dImage->getYDim())
        {
            tip_up.push_back(newTip);
        }
        if (curr.y > 0.95*p.p4dImage->getYDim())
        {
            tip_down.push_back(newTip);
        }
    }

    LocationSimple newTarget;

    if(tip_left.size()>0)
    {
        newTipsList.push_back(tip_left);
        newTarget.x = -floor((1.0-overlap)*p.p4dImage->getXDim());
        newTarget.y = 0;
        newTarget.z = 0;
        newTargetList.push_back(newTarget);
    }
    if(tip_right.size()>0)
    {
        newTipsList.push_back(tip_right);
        newTarget.x = floor((1.0-overlap)*p.p4dImage->getXDim());
        newTarget.y = 0;
        newTarget.z = 0;
        newTargetList.push_back(newTarget);
    }
    if(tip_up.size()>0)
    {
        newTipsList.push_back(tip_up);
        newTarget.x = 0;
        newTarget.y = -floor((1.0-overlap)*p.p4dImage->getYDim());
        newTarget.z = 0;
        newTargetList.push_back(newTarget);
    }
    if(tip_down.size()>0)
    {
        newTipsList.push_back(tip_down);
        newTarget.x = 0;
        newTarget.y = floor((1.0-overlap)*p.p4dImage->getYDim());
        newTarget.z = 0;
        newTargetList.push_back(newTarget);
    }

    if (!newTargetList.empty())
    {
        for (int i = 0; i<newTargetList.length(); i++)
        {
            newTargetList[i].x = newTargetList[i].x+p.p4dImage->getOriginX();
            newTargetList[i].y= newTargetList[i].y+p.p4dImage->getOriginY();
            newTargetList[i].z =(1.0-overlap)*newTargetList[i].z+p.p4dImage->getOriginZ();
            newTargetList[i].ev_pc1 = tileLocation.ev_pc1;
            newTargetList[i].ev_pc2= tileLocation.ev_pc2;
            newTargetList[i].ave = tileLocation.ave;
            newTargetList[i].mcenter = tileLocation.mcenter;
            newTargetList[i].name = tileLocation.name;
        }
    }

    if (!imageLandmarks.isEmpty()){
        qDebug()<<"set landmark group";

    }

    QList<ImageMarker> tipsToSave;
    QString markerSaveString2;
    markerSaveString2 = swcString;
    if (tileStatus==1) markerSaveString2.append("D");
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
    emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave,tileSaveString, tileStatus);


}

void StackAnalyzer::APP2Tracing_adaptive(Image4DSimple* total4DImage,  Image4DSimple* total4DImage_mip, QString swcString, float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation, QString saveDirString, bool useGSDT, bool isSoma, QString tileSaveString, int tileStatus)
{
    QString finaloutputswc = saveDirString + ("/s2.swc");
    ifstream ifs_swc(finaloutputswc.toStdString().c_str());
    vector<MyMarker*> finalswc;

    if(ifs_swc)
        finalswc = readSWC_file(finaloutputswc.toStdString());

    QList<LandmarkList> newTipsList;
    LandmarkList newTargetList;

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
        if (tileStatus==1) markerSaveString.append("D");
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
        NeuronSWC curr = list.at(i);
        LocationSimple newTip;
        bool check_tip = false;

        if( curr.x < 0.05*  p.p4dImage->getXDim() || curr.x > 0.95 *  p.p4dImage->getXDim() || curr.y < 0.05 * p.p4dImage->getYDim() || curr.y > 0.95* p.p4dImage->getYDim())
        {
            newTip.x = curr.x + p.p4dImage->getOriginX();
            newTip.y = curr.y + p.p4dImage->getOriginY();
            newTip.z = curr.z + p.p4dImage->getOriginZ();

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
        }
        if (curr.x > 0.95 * p.p4dImage->getXDim())
        {
            tip_right.push_back(newTip);
        }
        if (curr.y < 0.05 * p.p4dImage->getYDim())
        {
            tip_up.push_back(newTip);
        }
        if (curr.y > 0.95*p.p4dImage->getYDim())
        {
            tip_down.push_back(newTip);
        }
    }
    if (!newTargetList.empty())
    {
        for (int i = 0; i<newTargetList.length(); i++)
        {
            newTargetList[i].ave = tileLocation.ave;
            newTargetList[i].mcenter = tileLocation.mcenter;
            newTargetList[i].name = tileLocation.name;
        }
    }
    if(tip_left.size()>0)
    {
        QList<LandmarkList> group_tips_left = group_tips(tip_left,100,1);
        for(int i = 0; i < group_tips_left.size();i++)
            ada_win_finding(group_tips_left.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,globalMaxBlockSize,1,overlap, globalMinBlockSize);
    }
    if(tip_right.size()>0)
    {
        QList<LandmarkList> group_tips_right = group_tips(tip_right,100,2);
        for(int i = 0; i < group_tips_right.size();i++)
            ada_win_finding(group_tips_right.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,globalMaxBlockSize,2,overlap, globalMinBlockSize);
    }
    if(tip_up.size()>0)
    {
        QList<LandmarkList> group_tips_up = group_tips(tip_up,100,3);
        for(int i = 0; i < group_tips_up.size();i++)
            ada_win_finding(group_tips_up.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,globalMaxBlockSize,3,overlap, globalMinBlockSize);

    }
    if(tip_down.size()>0)
    {
        QList<LandmarkList> group_tips_down = group_tips(tip_down,100,4);
        for(int i = 0; i < group_tips_down.size();i++)
            ada_win_finding(group_tips_down.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,globalMaxBlockSize,4,overlap, globalMinBlockSize);
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

    if (!imageLandmarks.isEmpty()){
        qDebug()<<"set landmark group";

    }

    QList<ImageMarker> tipsToSave;
    QString markerSaveString2;
    markerSaveString2 = swcString;
    if (tileStatus==1) markerSaveString2.append("D");
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
    emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave, tileSaveString, tileStatus);
}

void StackAnalyzer::SubtractiveTracing(QString latestString,QString imageSaveString, Image4DSimple* total4DImage, Image4DSimple* total4DImage_mip,QString swcString,float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation, QString saveDirString,bool useGSDT, bool isSoma, int methodChoice, int tileStatus)
{
    QFileInfo imageFileInfo = QFileInfo(latestString);
    QList<LandmarkList> newTipsList;
    LandmarkList newTargetList;

    NeuronTree nt;
    NeuronTree nt_most;

    bool alreadyBeenTraced = tileStatus==1;

    QString swcMOST = saveDirString;

    if(methodChoice ==0){
        swcMOST.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(channel).append(".v3draw_MOST.swc");
    }else if(methodChoice ==1){
        swcMOST.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(channel).append(".v3draw_neutube.swc");
    }else if(methodChoice==3){
        swcMOST.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(channel).append(".v3draw_debug.swc");
    }


    if(!alreadyBeenTraced)
    {
        if(methodChoice == 3)
        {
            if (background<150){ nt = generate_crossing_swc(total4DImage); }
            export_list2file(nt.listNeuron, swcMOST,swcMOST);

        }else{

            int seed_win = 10;
            int slip_win = 10;

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

            if(methodChoice == 0)
            {
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

                qDebug()<<"starting most";
            }else if (methodChoice == 1)
            {
                arg_para.push_back("1");
                arg_para.push_back("1");
                full_plugin_name = "neuTube";
                func_name =  "neutube_trace";
                qDebug()<<"starting neutube";
            }

            qDebug()<<"rootlist size "<<QString::number(inputRootList.size());
            arg.p = (void *) & arg_para; input << arg;

            if(!cb->callPluginFunc(full_plugin_name,func_name,input,output))
            {

                qDebug()<<("Can not find the tracing plugin!\n");
                total4DImage_mip->deleteRawDataAndSetPointerToNull();

                emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave, imageSaveString, tileStatus);
                return;
            }
        }
    }


    if (alreadyBeenTraced) {
        swcMOST = swcString;
        if (methodChoice==0){
            swcMOST.chop(4);
            swcMOST.append(".v3draw_MOST.swc");
        }else if(methodChoice==1){
            swcMOST.chop(4);
            swcMOST.append(".v3draw_neutube.swc");
        }else if(methodChoice==3){
        swcMOST.chop(4);
        swcMOST.append(".v3draw_debug.swc");
    }
    }

    qDebug()<<"reading SWC file ... "<<swcMOST;
    nt_most= readSWC_file(swcMOST);

    if(nt_most.listNeuron.size()<1){
        qDebug()<<"zero size listNeuron!!";
        export_list2file(nt.listNeuron, swcString,swcMOST);
        emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave, imageSaveString, tileStatus);
        return;
    }

    if(!alreadyBeenTraced)
    {
        nt = sort_eliminate_swc(nt_most,inputRootList,total4DImage,isSoma);
        export_list2file(nt.listNeuron, swcString,swcMOST);
    }else
    {
        sAMutex.lock();
        NeuronTree nt_tile = readSWC_file(swcString);
        LandmarkList inputRootList_pruned = eliminate_seed(nt_tile,inputRootList,total4DImage);
        if(inputRootList_pruned.size()<1)
        {
            sAMutex.unlock();
            emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave, imageSaveString, tileStatus);
            return;
        }else
        {
            nt = sort_eliminate_swc(nt_most,inputRootList_pruned,total4DImage,isSoma);
            combine_list2file(nt.listNeuron, swcString);
        }
        sAMutex.unlock();
    }


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
        outputMarker.x = inputRootList.at(i).x- (float) total4DImage->getOriginX(); ;
        outputMarker.y = inputRootList.at(i).y- (float) total4DImage->getOriginY(); ;
        outputMarker.z = inputRootList.at(i).z - (float) total4DImage->getOriginZ();;
        seedsToSave.append(outputMarker);

    }

    QString markerSaveString;
    markerSaveString = swcString;
    if(seedsToSave.size()>=1)
        markerSaveString.append(QString("%1_%2_%3_x%4_y%5").arg(seedsToSave.at(0).x).arg(seedsToSave.at(0).y).arg(seedsToSave.at(0).z).arg((int)tileLocation.pixmax).arg((int)tileLocation.pixval));
    else
        markerSaveString.append("0_x%1_y%2").arg((int)tileLocation.pixmax).arg((int)tileLocation.pixval);
    markerSaveString.append("_initial.marker");
    writeMarker_file(markerSaveString, seedsToSave);

    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);
    }

    QString finaloutputswc = saveDirString + ("/s2.swc");
    QString finaloutputswc_left = saveDirString + ("/s2_nontraced.swc");

    ifstream ifs_swc(finaloutputswc.toStdString().c_str());
    NeuronTree finalswc;
    NeuronTree finalswc_left;
    vector<QList<NeuronSWC> > nt_list;
    vector<QList<NeuronSWC> > nt_list_left;

    if(ifs_swc)
    {
        finalswc = readSWC_file(finaloutputswc);
        nt_list.push_back(finalswc.listNeuron);
        finalswc_left = readSWC_file(finaloutputswc_left);
        nt_list_left.push_back(finalswc_left.listNeuron);
    }

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

            for(V3DLONG j = 0; j < finalswc.listNeuron.size(); j++ )
            {
                double dis = sqrt(pow2(newTip.x - finalswc.listNeuron.at(j).x) + pow2(newTip.y - finalswc.listNeuron.at(j).y) + pow2(newTip.z - finalswc.listNeuron.at(j).z));
                if(dis < 10)
                {
                    check_tip = true;
                    break;
                }
            }
        }

        if(check_tip) continue;
        if( curr.x < 0.05* total4DImage->getXDim())
        {
            tip_left.push_back(newTip);
        }
        if (curr.x > 0.95 * total4DImage->getXDim())
        {
            tip_right.push_back(newTip);
        }
        if (curr.y < 0.05 * total4DImage->getYDim())
        {
            tip_up.push_back(newTip);
        }
        if (curr.y > 0.95*total4DImage->getYDim())
        {
            tip_down.push_back(newTip);
        }

    }
    LocationSimple newTarget;
    if(tip_left.size()>0)
    {
        newTipsList.push_back(tip_left);
        newTarget.x = -floor((1.0-overlap)*total4DImage->getXDim());
        newTarget.y = 0;
        newTarget.z = 0;
        newTargetList.push_back(newTarget);
    }
    if(tip_right.size()>0)
    {
        newTipsList.push_back(tip_right);
        newTarget.x = floor((1.0-overlap)*total4DImage->getXDim());
        newTarget.y = 0;
        newTarget.z = 0;
        newTargetList.push_back(newTarget);
    }
    if(tip_up.size()>0)
    {
        newTipsList.push_back(tip_up);
        newTarget.x = 0;
        newTarget.y = -floor((1.0-overlap)*total4DImage->getYDim());
        newTarget.z = 0;
        newTargetList.push_back(newTarget);
    }
    if(tip_down.size()>0)
    {
        newTipsList.push_back(tip_down);
        newTarget.x = 0;
        newTarget.y = floor((1.0-overlap)*total4DImage->getYDim());
        newTarget.z = 0;
        newTargetList.push_back(newTarget);
    }

    if (!newTargetList.empty())
    {
        for (int i = 0; i<newTargetList.length(); i++)
        {
            newTargetList[i].x = newTargetList[i].x+total4DImage->getOriginX();
            newTargetList[i].y = newTargetList[i].y+total4DImage->getOriginY();
            newTargetList[i].z = newTargetList[i].z+total4DImage->getOriginZ();
            newTargetList[i].ev_pc1 = tileLocation.ev_pc1;
            newTargetList[i].ev_pc2 = tileLocation.ev_pc2;
            newTargetList[i].mcenter = tileLocation.mcenter;
            newTargetList[i].ave = tileLocation.ave;
            newTargetList[i].name =imageSaveString.toStdString();
            newTargetList[i].pixmax = tileLocation.x;
            newTargetList[i].pixval = tileLocation.y;
        }
    }

    NeuronTree nt_traced = readSWC_file(swcString);
    NeuronTree nt_left = neuron_sub(nt_most, nt_traced);
    if(ifs_swc)
    {
        for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
        {
            NeuronSWC curr = nt.listNeuron.at(i);
            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                nt.listNeuron[i].type = 1;
            }else
                nt.listNeuron[i].type = 2;

            nt.listNeuron[i].x = curr.x + total4DImage->getOriginX();
            nt.listNeuron[i].y = curr.y + total4DImage->getOriginY();
            nt.listNeuron[i].z = curr.z + total4DImage->getOriginZ();
        }
        nt_list.push_back(nt.listNeuron);
        QList<NeuronSWC> finalswc_updated;
        if (combine_linker(nt_list, finalswc_updated))
        {
            export_list2file(finalswc_updated, finaloutputswc,finaloutputswc);
        }

        for(V3DLONG i = 0; i < nt_left.listNeuron.size(); i++)
        {
            NeuronSWC curr = nt_left.listNeuron.at(i);
            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                nt_left.listNeuron[i].type = 1;
            }else
                nt_left.listNeuron[i].type = 2;

            nt_left.listNeuron[i].x = curr.x + total4DImage->getOriginX();
            nt_left.listNeuron[i].y = curr.y + total4DImage->getOriginY();
            nt_left.listNeuron[i].z = curr.z + total4DImage->getOriginZ();
        }

        if(nt_left.listNeuron.size()>0)
        {
            if(tileStatus!=1)
            {
                QList<NeuronSWC> nt_left_sorted;
                if(SortSWC(nt_left.listNeuron, nt_left_sorted,VOID, 0))
                    nt_list_left.push_back(nt_left_sorted);
                QList<NeuronSWC> finalswc_left_updated_added;
                if (combine_linker(nt_list_left, finalswc_left_updated_added))
                {
                    export_list2file(finalswc_left_updated_added, finaloutputswc_left,finaloutputswc_left);
                }
            }else
            {
                NeuronTree finalswc_left_updated_minus = neuron_sub(finalswc_left, nt);
                export_list2file(finalswc_left_updated_minus.listNeuron, finaloutputswc_left,finaloutputswc_left);
            }
        }
    }
    else
    {
        for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
        {
            NeuronSWC curr = nt.listNeuron.at(i);
            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                nt.listNeuron[i].type = 1;
            }else
                nt.listNeuron[i].type = 2;

            nt.listNeuron[i].x = curr.x + total4DImage->getOriginX();
            nt.listNeuron[i].y = curr.y + total4DImage->getOriginY();
            nt.listNeuron[i].z = curr.z + total4DImage->getOriginZ();

        }
        export_list2file(nt.listNeuron, finaloutputswc,finaloutputswc);

        for(V3DLONG i = 0; i < nt_left.listNeuron.size(); i++)
        {
            NeuronSWC curr = nt_left.listNeuron.at(i);
            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                nt_left.listNeuron[i].type = 1;
            }else
                nt_left.listNeuron[i].type = 2;

            nt_left.listNeuron[i].x = curr.x + total4DImage->getOriginX();
            nt_left.listNeuron[i].y = curr.y + total4DImage->getOriginY();
            nt_left.listNeuron[i].z = curr.z + total4DImage->getOriginZ();
        }
        if(nt_left.listNeuron.size()>0)
        {
            QList<NeuronSWC> nt_left_sorted;
            if(SortSWC(nt_left.listNeuron, nt_left_sorted,VOID, 0))
                export_list2file(nt_left_sorted, finaloutputswc_left,finaloutputswc_left);
        }
        else
        {
            export_list2file(nt_left.listNeuron, finaloutputswc_left,finaloutputswc_left);
        }
    }

    QList<ImageMarker> tipsToSave;
    QString markerSaveString2;
    markerSaveString2 = swcString;
    for (int i =0; i<newTipsList.length(); i++){
        LandmarkList iList = newTipsList[i];
        for (int j = 0; j<iList.length();j++){
            ImageMarker markerIJ;
            markerIJ.x = iList[j].x-total4DImage->getOriginX();
            markerIJ.y = iList[j].y-total4DImage->getOriginY();
            markerIJ.z = iList[j].z-total4DImage->getOriginZ();
            tipsToSave.append(markerIJ);
        }
    }

    if(tipsToSave.size()>=1)
        markerSaveString2.append(QString("%1_%2_%3").arg(tipsToSave.at(0).x).arg(tipsToSave.at(0).y).arg(tipsToSave.at(0).z));
    else
        markerSaveString2.append("0");
    markerSaveString2.append("_final.marker");

    writeMarker_file(markerSaveString2, tipsToSave);
    emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave, imageSaveString, tileStatus);
}

void StackAnalyzer::SubtractiveTracing_adaptive(QString latestString, QString imageSaveString, Image4DSimple* total4DImage, Image4DSimple* total4DImage_mip,QString swcString,float overlap, int background, bool interrupt, LandmarkList inputRootList, LocationSimple tileLocation, QString saveDirString,bool useGSDT, bool isSoma, int methodChoice, int tileStatus)
{
    QFileInfo imageFileInfo = QFileInfo(latestString);
    QList<LandmarkList> newTipsList;
    LandmarkList newTargetList;

    int seed_win = 10;
    int slip_win = 10;

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

    if(methodChoice == 0)
    {
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

        qDebug()<<"starting most";
    }else if (methodChoice == 1)
    {
        arg_para.push_back("1");
        arg_para.push_back("1");
        full_plugin_name = "neuTube";
        func_name =  "neutube_trace";
        qDebug()<<"starting neutube";
    }


    qDebug()<<"rootlist size "<<QString::number(inputRootList.size());

    arg.p = (void *) & arg_para; input << arg;

    if(!cb->callPluginFunc(full_plugin_name,func_name,input,output))
    {

        printf("Can not find the tracing plugin!\n");
        total4DImage_mip->deleteRawDataAndSetPointerToNull();

        emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave, imageSaveString, tileStatus);

        return;
    }
    NeuronTree nt_most;
    QString swcMOST = saveDirString;
    if(methodChoice ==0)
        swcMOST.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(channel).append(".v3draw_MOST.swc");
    else if(methodChoice ==1)
        swcMOST.append("/x_").append(QString::number((int)tileLocation.x)).append("_y_").append(QString::number((int)tileLocation.y)).append("_").append(imageFileInfo.fileName()).append(channel).append(".v3draw_neutube.swc");

    nt_most = readSWC_file(swcMOST);

    if(nt_most.listNeuron.size()<1){
        emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave, imageSaveString, tileStatus);
        return;
    }
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
    if (tileStatus==1) markerSaveString.append("D");
    markerSaveString.append(".marker");
    writeMarker_file(markerSaveString, seedsToSave);



    sAMutex.lock();

    QString finaloutputswc = saveDirString + ("/s2.swc");
    QString finaloutputswc_left = saveDirString + ("/s2_nontraced.swc");

    ifstream ifs_swc(finaloutputswc.toStdString().c_str());
    NeuronTree finalswc;
    NeuronTree finalswc_left;
    vector<QList<NeuronSWC> > nt_list;
    vector<QList<NeuronSWC> > nt_list_left;

    if(ifs_swc)
    {
        finalswc = readSWC_file(finaloutputswc);
        finalswc_left = readSWC_file(finaloutputswc_left);
    }

    LandmarkList tip_left;
    LandmarkList tip_right;
    LandmarkList tip_up ;
    LandmarkList tip_down;
    QList<NeuronSWC> list = nt.listNeuron;
    LandmarkList tip_visited;

    for (V3DLONG i=0;i<list.size();i++)
    {
        NeuronSWC curr = list.at(i);
        LocationSimple newTip;
        bool check_tip = false, check_visited= false;

        if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
        {
            newTip.x = curr.x + total4DImage->getOriginX();
            newTip.y = curr.y + total4DImage->getOriginY();
            newTip.z = curr.z + total4DImage->getOriginZ();

            for(V3DLONG j = 0; j < finalswc.listNeuron.size(); j++ )
            {
                double dis = sqrt(pow2(newTip.x - finalswc.listNeuron.at(j).x) + pow2(newTip.y - finalswc.listNeuron.at(j).y) + pow2(newTip.z - finalswc.listNeuron.at(j).z));
                if(dis < 10)
                {
                    check_tip = true;
                    break;
                }
            }

            if(!check_tip)
            {
                for(V3DLONG j = 0; j < finalswc_left.listNeuron.size(); j++ )
                {
                    double dis = sqrt(pow2(newTip.x - finalswc_left.listNeuron.at(j).x) + pow2(newTip.y - finalswc_left.listNeuron.at(j).y) + pow2(newTip.z - finalswc_left.listNeuron.at(j).z));
                    if(dis < 10)
                    {
                        check_visited = true;
                        tip_visited.push_back(newTip);
                        break;
                    }
                }
            }

            if(check_tip || check_visited) continue;

            if( curr.x < 0.05* total4DImage->getXDim())
            {
                tip_left.push_back(newTip);
            }
            if (curr.x > 0.95 * total4DImage->getXDim())
            {
                tip_right.push_back(newTip);
            }
            if (curr.y < 0.05 * total4DImage->getYDim())
            {
                tip_up.push_back(newTip);
            }
            if (curr.y > 0.95*total4DImage->getYDim())
            {
                tip_down.push_back(newTip);
            }
        }
    }

    if (!newTargetList.empty())
    {
        for (int i = 0; i<newTargetList.length(); i++)
        {
            newTargetList[i].ave = tileLocation.ave;
            newTargetList[i].mcenter = tileLocation.mcenter;
            newTargetList[i].name = imageSaveString.toStdString();
        }
    }

    if(tip_left.size()>0)
    {
        QList<LandmarkList> group_tips_left = group_tips(tip_left,120,1);
        for(int i = 0; i < group_tips_left.size();i++)
            ada_win_finding(group_tips_left.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,globalMaxBlockSize,1,overlap, globalMinBlockSize);
    }
    if(tip_right.size()>0)
    {
        QList<LandmarkList> group_tips_right = group_tips(tip_right,120,2);
        for(int i = 0; i < group_tips_right.size();i++)
            ada_win_finding(group_tips_right.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,globalMaxBlockSize,2,overlap, globalMinBlockSize);
    }
    if(tip_up.size()>0)
    {
        QList<LandmarkList> group_tips_up = group_tips(tip_up,120,3);
        for(int i = 0; i < group_tips_up.size();i++)
            ada_win_finding(group_tips_up.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,globalMaxBlockSize,3,overlap, globalMinBlockSize);

    }
    if(tip_down.size()>0)
    {
        QList<LandmarkList> group_tips_down = group_tips(tip_down,120,4);
        for(int i = 0; i < group_tips_down.size();i++)
            ada_win_finding(group_tips_down.at(i),tileLocation,&newTargetList,&newTipsList,total4DImage,globalMaxBlockSize,4,overlap, globalMinBlockSize);
    }

    NeuronTree nt_traced = readSWC_file(swcString);
    NeuronTree nt_left = neuron_sub(nt_most, nt_traced);

    if(ifs_swc)
    {
        nt_list.push_back(finalswc.listNeuron);
        NeuronTree nt_visited;
        NeuronTree finalswc_left_nonvisited;
        if(tip_visited.size()>0)
        {
            nt_visited = sort_eliminate_swc(finalswc_left,tip_visited,total4DImage,false);
        }
        if(nt_visited.listNeuron.size()>0)
        {
            finalswc_left_nonvisited = neuron_sub(finalswc_left,nt_visited);
            nt_list_left.push_back(finalswc_left_nonvisited.listNeuron);
        }else
            nt_list_left.push_back(finalswc_left.listNeuron);

        for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
        {
            NeuronSWC curr = nt.listNeuron.at(i);
            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                nt.listNeuron[i].type = 1;
            }else
                nt.listNeuron[i].type = 2;

            nt.listNeuron[i].x = curr.x + total4DImage->getOriginX();
            nt.listNeuron[i].y = curr.y + total4DImage->getOriginY();
            nt.listNeuron[i].z = curr.z + total4DImage->getOriginZ();

        }
        nt_list.push_back(nt.listNeuron);
        QList<NeuronSWC> finalswc_updated;
        if (combine_linker(nt_list, finalswc_updated))
        {
            export_list2file(finalswc_updated, finaloutputswc,finaloutputswc);
        }

        if(nt_left.listNeuron.size()>0)
        {
            for(V3DLONG i = 0; i < nt_left.listNeuron.size(); i++)
            {
                NeuronSWC curr = nt_left.listNeuron.at(i);
                if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
                {
                    nt_left.listNeuron[i].type = 1;
                }else
                    nt_left.listNeuron[i].type = 2;

                nt_left.listNeuron[i].x = curr.x + total4DImage->getOriginX();
                nt_left.listNeuron[i].y = curr.y + total4DImage->getOriginY();
                nt_left.listNeuron[i].z = curr.z + total4DImage->getOriginZ();
            }

            NeuronTree nt_left_left = neuron_sub(nt_left, finalswc);
            if(nt_left_left.listNeuron.size()>0)
            {
                QList<NeuronSWC> nt_left_sorted;
                if(SortSWC(nt_left_left.listNeuron, nt_left_sorted,VOID, 0))
                    nt_list_left.push_back(nt_left_sorted);

                QList<NeuronSWC> finalswc_left_updated_added;
                if (combine_linker(nt_list_left, finalswc_left_updated_added))
                {
                    QList<NeuronSWC> finalswc_left_updated_added_sorted;
                    if(SortSWC(finalswc_left_updated_added, finalswc_left_updated_added_sorted,VOID, 10))
                        export_list2file(finalswc_left_updated_added_sorted, finaloutputswc_left,finaloutputswc_left);
                }
            }
        }else if(nt_visited.listNeuron.size()>0)
        {
            export_list2file(finalswc_left_nonvisited.listNeuron, finaloutputswc_left,finaloutputswc_left);
        }

    }
    else
    {
        for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
        {
            NeuronSWC curr = nt.listNeuron.at(i);
            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                nt.listNeuron[i].type = 1;
            }else
                nt.listNeuron[i].type = 2;

            nt.listNeuron[i].x = curr.x + total4DImage->getOriginX();
            nt.listNeuron[i].y = curr.y + total4DImage->getOriginY();
            nt.listNeuron[i].z = curr.z + total4DImage->getOriginZ();

        }
        export_list2file(nt.listNeuron, finaloutputswc,finaloutputswc);

        for(V3DLONG i = 0; i < nt_left.listNeuron.size(); i++)
        {
            NeuronSWC curr = nt_left.listNeuron.at(i);
            if( curr.x < 0.05*  total4DImage->getXDim() || curr.x > 0.95 *  total4DImage->getXDim() || curr.y < 0.05 * total4DImage->getYDim() || curr.y > 0.95* total4DImage->getYDim())
            {
                nt_left.listNeuron[i].type = 1;
            }else
                nt_left.listNeuron[i].type = 2;

            nt_left.listNeuron[i].x = curr.x + total4DImage->getOriginX();
            nt_left.listNeuron[i].y = curr.y + total4DImage->getOriginY();
            nt_left.listNeuron[i].z = curr.z + total4DImage->getOriginZ();
        }

        QList<NeuronSWC> nt_left_sorted;
        if(SortSWC(nt_left.listNeuron, nt_left_sorted,VOID, 0))
            export_list2file(nt_left_sorted, finaloutputswc_left,finaloutputswc_left);
    }

    sAMutex.unlock();




    if (!imageLandmarks.isEmpty()){
        qDebug()<<"set landmark group";
    }
    QList<ImageMarker> tipsToSave;
    QString markerSaveString2;
    markerSaveString2 = swcString;
    if (tileStatus==1) markerSaveString2.append("D");
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



    emit analysisDone(newTipsList, newTargetList, total4DImage_mip, tileLocation.ave, imageSaveString, tileStatus);
}

int StackAnalyzer::methodSelection(Image4DSimple* total4DImage,LandmarkList inputRootList, int background, bool isSoma)
{
    int methodChoice = 1;
    if(isSoma)
        methodChoice = 2;
    else
    {
        if(inputRootList.size() < 5)
            methodChoice = 2;
    }
    return methodChoice;

}


template <class T> void StackAnalyzer::gaussian_filter(T* data1d,
                                                       V3DLONG *in_sz,
                                                       unsigned int Wx,
                                                       unsigned int Wy,
                                                       unsigned int Wz,
                                                       unsigned int c,
                                                       double sigma,
                                                       float* &outimg)
{
    if (!data1d || !in_sz || in_sz[0]<=0 || in_sz[1]<=0 || in_sz[2]<=0 || in_sz[3]<=0 || outimg)
    {
        v3d_msg("Invalid parameters to gaussian_filter().", 0);
        return;
    }

    if (outimg)
    {
        v3d_msg("Warning: you have supplied an non-empty output image pointer. This program will force to free it now. But you may want to double check.");
        delete []outimg;
        outimg = 0;
    }

    // for filter kernel
    double sigma_s2 = 0.5/(sigma*sigma); // 1/(2*sigma*sigma)
    double pi_sigma = 1.0/(sqrt(2*3.1415926)*sigma); // 1.0/(sqrt(2*pi)*sigma)

    float min_val = INF, max_val = 0;

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG sc = in_sz[3];
    V3DLONG pagesz = N*M*P;

    //filtering
    V3DLONG offset_init = (c-1)*pagesz;

    //declare temporary pointer
    float *pImage = new float [pagesz];
    if (!pImage)
    {
        printf("Fail to allocate memory.\n");
        return;
    }
    else
    {
        for(V3DLONG i=0; i<pagesz; i++)
            pImage[i] = data1d[i + offset_init];  //first channel data (red in V3D, green in ImageJ)
    }
    //Filtering
    //
    //   Filtering along x
    if(N<2)
    {
        //do nothing
    }
    else
    {
        //create Gaussian kernel
        float  *WeightsX = 0;
        WeightsX = new float [Wx];
        if (!WeightsX)
            return;

        float Half = (float)(Wx-1)/2.0;

        // Gaussian filter equation:
        // http://en.wikipedia.org/wiki/Gaussian_blur
        //   for (unsigned int Weight = 0; Weight < Half; ++Weight)
        //   {
        //        const float  x = Half* float (Weight) / float (Half);
        //         WeightsX[(int)Half - Weight] = WeightsX[(int)Half + Weight] = pi_sigma * exp(-x * x *sigma_s2); // Corresponding symmetric WeightsX
        //    }

        for (unsigned int Weight = 0; Weight <= Half; ++Weight)
        {
            const float  x = float(Weight)-Half;
            WeightsX[Weight] = WeightsX[Wx-Weight-1] = pi_sigma * exp(-(x * x *sigma_s2)); // Corresponding symmetric WeightsX
        }


        double k = 0.;
        for (unsigned int Weight = 0; Weight < Wx; ++Weight)
            k += WeightsX[Weight];

        for (unsigned int Weight = 0; Weight < Wx; ++Weight)
            WeightsX[Weight] /= k;

        printf("\n x dierction");

        for (unsigned int Weight = 0; Weight < Wx; ++Weight)
            printf("/n%f",WeightsX[Weight]);

        //   Allocate 1-D extension array
        float  *extension_bufferX = 0;
        extension_bufferX = new float [N + (Wx<<1)];

        unsigned int offset = Wx>>1;

        //	along x
        const float  *extStop = extension_bufferX + N + offset;

        for(V3DLONG iz = 0; iz < P; iz++)
        {
            for(V3DLONG iy = 0; iy < M; iy++)
            {
                float  *extIter = extension_bufferX + Wx;
                for(V3DLONG ix = 0; ix < N; ix++)
                {
                    *(extIter++) = pImage[iz*M*N + iy*N + ix];
                }

                //   Extend image
                const float  *const stop_line = extension_bufferX - 1;
                float  *extLeft = extension_bufferX + Wx - 1;
                const float  *arrLeft = extLeft + 2;
                float  *extRight = extLeft + N + 1;
                const float  *arrRight = extRight - 2;

                while (extLeft > stop_line)
                {
                    *(extLeft--) = *(arrLeft++);
                    *(extRight++) = *(arrRight--);

                }

                //	Filtering
                extIter = extension_bufferX + offset;

                float  *resIter = &(pImage[iz*M*N + iy*N]);

                while (extIter < extStop)
                {
                    double sum = 0.;
                    const float  *weightIter = WeightsX;
                    const float  *const End = WeightsX + Wx;
                    const float * arrIter = extIter;
                    while (weightIter < End)
                        sum += *(weightIter++) * float (*(arrIter++));
                    extIter++;
                    *(resIter++) = sum;

                    //for rescale
                    if(max_val<*arrIter) max_val = *arrIter;
                    if(min_val>*arrIter) min_val = *arrIter;


                }

            }
        }
        //de-alloc
        if (WeightsX) {delete []WeightsX; WeightsX=0;}
        if (extension_bufferX) {delete []extension_bufferX; extension_bufferX=0;}

    }

    //   Filtering along y
    if(M<2)
    {
        //do nothing
    }
    else
    {
        //create Gaussian kernel
        float  *WeightsY = 0;
        WeightsY = new float [Wy];
        if (!WeightsY)
            return;

        float Half = (float)(Wy-1)/2.0;

        // Gaussian filter equation:
        // http://en.wikipedia.org/wiki/Gaussian_blur
        /* for (unsigned int Weight = 0; Weight < Half; ++Weight)
          {
               const float  y = Half* float (Weight) / float (Half);
               WeightsY[(int)Half - Weight] = WeightsY[(int)Half + Weight] = pi_sigma * exp(-y * y *sigma_s2); // Corresponding symmetric WeightsY
          }*/

        for (unsigned int Weight = 0; Weight <= Half; ++Weight)
        {
            const float  y = float(Weight)-Half;
            WeightsY[Weight] = WeightsY[Wy-Weight-1] = pi_sigma * exp(-(y * y *sigma_s2)); // Corresponding symmetric WeightsY
        }


        double k = 0.;
        for (unsigned int Weight = 0; Weight < Wy; ++Weight)
            k += WeightsY[Weight];

        for (unsigned int Weight = 0; Weight < Wy; ++Weight)
            WeightsY[Weight] /= k;

        //	along y
        float  *extension_bufferY = 0;
        extension_bufferY = new float [M + (Wy<<1)];

        unsigned int offset = Wy>>1;
        const float *extStop = extension_bufferY + M + offset;

        for(V3DLONG iz = 0; iz < P; iz++)
        {
            for(V3DLONG ix = 0; ix < N; ix++)
            {
                float  *extIter = extension_bufferY + Wy;
                for(V3DLONG iy = 0; iy < M; iy++)
                {
                    *(extIter++) = pImage[iz*M*N + iy*N + ix];
                }

                //   Extend image
                const float  *const stop_line = extension_bufferY - 1;
                float  *extLeft = extension_bufferY + Wy - 1;
                const float  *arrLeft = extLeft + 2;
                float  *extRight = extLeft + M + 1;
                const float  *arrRight = extRight - 2;

                while (extLeft > stop_line)
                {
                    *(extLeft--) = *(arrLeft++);
                    *(extRight++) = *(arrRight--);
                }

                //	Filtering
                extIter = extension_bufferY + offset;

                float  *resIter = &(pImage[iz*M*N + ix]);

                while (extIter < extStop)
                {
                    double sum = 0.;
                    const float  *weightIter = WeightsY;
                    const float  *const End = WeightsY + Wy;
                    const float * arrIter = extIter;
                    while (weightIter < End)
                        sum += *(weightIter++) * float (*(arrIter++));
                    extIter++;
                    *resIter = sum;
                    resIter += N;

                    //for rescale
                    if(max_val<*arrIter) max_val = *arrIter;
                    if(min_val>*arrIter) min_val = *arrIter;


                }

            }
        }

        //de-alloc
        if (WeightsY) {delete []WeightsY; WeightsY=0;}
        if (extension_bufferY) {delete []extension_bufferY; extension_bufferY=0;}


    }

    //  Filtering  along z
    if(P<2)
    {
        //do nothing
    }
    else
    {
        //create Gaussian kernel
        float  *WeightsZ = 0;
        WeightsZ = new float [Wz];
        if (!WeightsZ)
            return;

        float Half = (float)(Wz-1)/2.0;

        /* for (unsigned int Weight = 1; Weight < Half; ++Weight)
          {
               const float  z = Half * float (Weight) / Half;
               WeightsZ[(int)Half - Weight] = WeightsZ[(int)Half + Weight] = pi_sigma * exp(-z * z * sigma_s2) ; // Corresponding symmetric WeightsZ
          }*/

        for (unsigned int Weight = 0; Weight <= Half; ++Weight)
        {
            const float  z = float(Weight)-Half;
            WeightsZ[Weight] = WeightsZ[Wz-Weight-1] = pi_sigma * exp(-(z * z *sigma_s2)); // Corresponding symmetric WeightsZ
        }


        double k = 0.;
        for (unsigned int Weight = 0; Weight < Wz; ++Weight)
            k += WeightsZ[Weight];

        for (unsigned int Weight = 0; Weight < Wz; ++Weight)
            WeightsZ[Weight] /= k;

        //	along z
        float  *extension_bufferZ = 0;
        extension_bufferZ = new float [P + (Wz<<1)];

        unsigned int offset = Wz>>1;
        const float *extStop = extension_bufferZ + P + offset;

        for(V3DLONG iy = 0; iy < M; iy++)
        {
            for(V3DLONG ix = 0; ix < N; ix++)
            {

                float  *extIter = extension_bufferZ + Wz;
                for(V3DLONG iz = 0; iz < P; iz++)
                {
                    *(extIter++) = pImage[iz*M*N + iy*N + ix];
                }

                //   Extend image
                const float  *const stop_line = extension_bufferZ - 1;
                float  *extLeft = extension_bufferZ + Wz - 1;
                const float  *arrLeft = extLeft + 2;
                float  *extRight = extLeft + P + 1;
                const float  *arrRight = extRight - 2;

                while (extLeft > stop_line)
                {
                    *(extLeft--) = *(arrLeft++);
                    *(extRight++) = *(arrRight--);
                }

                //	Filtering
                extIter = extension_bufferZ + offset;

                float  *resIter = &(pImage[iy*N + ix]);

                while (extIter < extStop)
                {
                    double sum = 0.;
                    const float  *weightIter = WeightsZ;
                    const float  *const End = WeightsZ + Wz;
                    const float * arrIter = extIter;
                    while (weightIter < End)
                        sum += *(weightIter++) * float (*(arrIter++));
                    extIter++;
                    *resIter = sum;
                    resIter += M*N;

                    //for rescale
                    if(max_val<*arrIter) max_val = *arrIter;
                    if(min_val>*arrIter) min_val = *arrIter;

                }

            }
        }

        //de-alloc
        if (WeightsZ) {delete []WeightsZ; WeightsZ=0;}
        if (extension_bufferZ) {delete []extension_bufferZ; extension_bufferZ=0;}


    }

    outimg = pImage;


    return;
}

NeuronTree StackAnalyzer::neuron_sub(NeuronTree nt_total, NeuronTree nt)
{
    V3DLONG neuronNum = nt_total.listNeuron.size();
    NeuronTree nt_left;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    //set node
    NeuronSWC S;
    for (V3DLONG i=0;i<neuronNum;i++)
    {
        NeuronSWC curr = nt_total.listNeuron.at(i);
        S.n 	= curr.n;
        S.type 	= curr.type;
        S.x 	= curr.x;
        S.y 	= curr.y;
        S.z 	= curr.z;
        S.r 	= curr.r;
        S.pn 	= curr.pn;
        bool flag = false;
        for(V3DLONG j=0;j<nt.listNeuron.size();j++)
        {
            double dis = sqrt(pow2(nt.listNeuron[j].x - curr.x) + pow2(nt.listNeuron[j].y - curr.y) + pow2(nt.listNeuron[j].z - curr.z));
            if(dis < 5)
            {
                flag = true;
                break;
            }
        }
        if(!flag)
        {
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);
        }
    }
    nt_left.n = -1;
    nt_left.on = true;
    nt_left.listNeuron = listNeuron;
    nt_left.hashNeuron = hashNeuron;
    return nt_left;
}


