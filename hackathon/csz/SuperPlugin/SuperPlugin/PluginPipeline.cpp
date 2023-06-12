#include <PluginPipeline.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>


using namespace std;

QReadWriteLock PluginPipeLine::lockfile = QReadWriteLock();
QMutex PluginPipeLine::debug = QMutex();
QMutex PluginPipeLine::debug1 = QMutex();

PluginPipeLine::PluginPipeLine(V3DPluginCallback2 &callback)
{
    _callback=&callback;
    outsourcework=nullptr;
    connectdomaincount=px=py=pz=0;
    startready=false;
    SWCtoConcat.clear();
    connect(this,SIGNAL(readytoconcat()),this,SLOT(ConcatSWC()));
    connect(this,SIGNAL(concatdone(QString,PluginPipeLine*)),this,SLOT(getLandMarkList(QString,PluginPipeLine*)));
//    connect(TaskManage::get_instance(),SIGNAL(preprocess(QString)),this,SLOT(tracingapp2(QString)));

}

PluginPipeLine::~PluginPipeLine()
{

}

void PluginPipeLine::registerMemory(void *mem, int type)
{
    pointer np;
    np.p=mem;
    np.type=type;
    np.done=false;
    memory.append(np);
    int index=memory.size()-1;
    Mem2Pointer.insert(mem,index);
}

void PluginPipeLine::releaseMemory(void *mem)
{
    int index=Mem2Pointer[mem];
    int type=memory[index].type;
    void *p=memory[index].p;
    bool done=memory[index].done;
    if(done){
        switch(type){
        case 0:{
            char* cp=(char*)p;
            memory.removeAt(index);
            Mem2Pointer.remove(mem);
            delete cp;
            cp=nullptr;
            break;
        }
        case 1:{
            vector<char*>* vcp=(vector<char*>*)p;
            memory.removeAt(index);
            Mem2Pointer.remove(mem);
            delete vcp;
            vcp=nullptr;
            break;
        }
        }
    }
}

void PluginPipeLine::registerTaskMemory()
{

}

void PluginPipeLine::CropRawImage(QString ImageName,QVector<long> &boundingbox)
{
    assert(boundingbox.size()==6);
    char *imgname=new char[ImageName.length()+1];
    imgname[ImageName.length()]='\0';
    strcpy(imgname,ImageName.toStdString().c_str());
    registerMemory((void*)imgname,0);

    Image4DSimple *totalimg=_callback->loadImage(imgname);
    LimitBoundary(boundingbox,totalimg);

    FinalFileName=FileManage::get_instance()->GenerateFinalSwcName(ImageName);

    std::cout<<imgname<<'\n';
    QString OutputName=FileManage::get_instance()->GenerateImageName(boundingbox,ImageName);
    QVector<float> info;
    for(int i=0;i<6;i++){
        info.append(boundingbox[i]);
    }
    FileManage::get_instance()->CreateFileInfomation(OutputName,info);
    char *outimgname=new char[OutputName.length()+1];
    outimgname[OutputName.length()]='\0';
    strcpy(outimgname,OutputName.toStdString().c_str());
    registerMemory((void*)outimgname,0);

    QString XS=QString::number(boundingbox[0]);
    char *xs=new char[XS.length()+1];
    xs[XS.length()]='\0';
    strcpy(xs,XS.toStdString().c_str());
    registerMemory((void*)xs,0);
    mxs=boundingbox[0];

    QString YS=QString::number(boundingbox[1]);
    char *ys=new char[YS.length()+1];
    ys[YS.length()]='\0';
    strcpy(ys,YS.toStdString().c_str());
    registerMemory((void*)ys,0);
    mys=boundingbox[1];

    QString ZS=QString::number(boundingbox[2]);
    char *zs=new char[ZS.length()+1];
    zs[ZS.length()]='\0';
    strcpy(zs,ZS.toStdString().c_str());
    registerMemory((void*)zs,0);
    mzs=boundingbox[2];

    QString XE=QString::number(boundingbox[3]-1);
    char *xe=new char[XE.length()+1];
    xe[XE.length()]='\0';
    strcpy(xe,XE.toStdString().c_str());
    registerMemory((void*)xe,0);
    mxe=boundingbox[3];

    QString YE=QString::number(boundingbox[4]-1);
    char *ye=new char[YE.length()+1];
    ye[YE.length()]='\0';
    strcpy(ye,YE.toStdString().c_str());
    registerMemory((void*)ye,0);
    mye=boundingbox[4];

    QString ZE=QString::number(boundingbox[5]-1);
    char *ze=new char[ZE.length()+1];
    ze[ZE.length()]='\0';
    strcpy(ze,ZE.toStdString().c_str());
    registerMemory((void*)ze,0);
    mze=boundingbox[5];


    mboundingbox=boundingbox;


    vector<char*> *input=new vector<char*>();
    vector<char*> *inputpara=new vector<char*>();
    vector<char*> *output=new vector<char*>();
    input->push_back(imgname);
    output->push_back(outimgname);
    inputpara->push_back(xs);
    inputpara->push_back(xe);
    inputpara->push_back(ys);
    inputpara->push_back(ye);
    inputpara->push_back(zs);
    inputpara->push_back(ze);
    inputpara->push_back("1");

    qDebug()<<xs<<xe<<ys<<ye<<zs<<ze;
    registerMemory((void*)input,1);
    registerMemory((void*)inputpara,1);
    registerMemory((void*)output,1);
    CallPlugin *cp;
    cp=new CallPlugin(*_callback,"cropped3DImageSeries","crop3d_raw");
    cp->AssignPara(input,0);
    cp->AssignPara(inputpara,1);
    cp->AssignPara(output,2);
    cp->type=3;
    qDebug()<<OutputName;


//    QString cpadd = QString::number(reinterpret_cast<quintptr>(cp), 16);
//    QString ppadd = QString::number(reinterpret_cast<quintptr>(this), 16);
//    SpLog::debug("ppcrop:"+cpadd+" "+ppadd+" "+cp->getOutput()->at(0));

    TaskManage::get_instance()->addTask(cp,this);

}

void PluginPipeLine::ConvertFileFormat(QString ImageName,PluginPipeLine* p)
{
    if(p!=this)
        return;
    CallPlugin *cp;
    cp=new CallPlugin(*_callback,"convert_file_format","convert_format");
    char *_input=qstring2char(ImageName);
    char *_output=qstring2char(ImageName+".tiff");
    std::cout<<_input<<'\n';
    std::cout<<_output<<'\n';
    vector<char*> *input=new vector<char*>();
    vector<char*> *output=new vector<char*>();
    registerMemory((void*)input,1);

    registerMemory((void*)output,1);
    vector<char*> *inputpara=new vector<char*>();
    input->push_back(_input);
    output->push_back(_output);
    inputpara->push_back("1");
    cp->AssignPara(input,0);
    cp->AssignPara(inputpara,1);
    cp->AssignPara(output,2);
    cp->type=1;




    TaskManage::get_instance()->addTask(cp,this);
}

void PluginPipeLine::tracingapp2(QString name,PluginPipeLine* p)
{
    if(p!=this)
        return;
    CallPlugin *cp;
    cp=new CallPlugin(*_callback,"vn2","app2");
    char *_input=qstring2char(name);
    char *_output=qstring2char(name+"_app2.swc");
    std::cout<<_input<<'\n';
    std::cout<<_output<<'\n';
    vector<char*> *input=new vector<char*>();
    vector<char*> *output=new vector<char*>();
    vector<char*> *inputpara=new vector<char*>();
    input->push_back(_input);
    output->push_back(_output);
    inputpara->push_back("NULL");
    inputpara->push_back("0");
    inputpara->push_back("1");

    cp->AssignPara(input,0);
    cp->AssignPara(inputpara,1);
    cp->AssignPara(output,2);
    cp->type=4;


//    QString cpadd = QString::number(reinterpret_cast<quintptr>(cp), 16);
//    QString ppadd = QString::number(reinterpret_cast<quintptr>(this), 16);
//    SpLog::debug("ppapp2:"+cpadd+" "+ppadd+" "+cp->getOutput()->at(0));

    TaskManage::get_instance()->addTask(cp,this);


}

void PluginPipeLine::getLandMarkList(QString name,PluginPipeLine* p)
{
    if(p!=this)
        return;
    LandmarkList *newTargetList=new LandmarkList();
    LocationSimple tileLocation;
    tileLocation.x=mxs;
    tileLocation.y=mys;
    tileLocation.z=mzs;
    tileLocation.ev_pc1=mxe-mxs;
    tileLocation.ev_pc2=mye-mys;
    tileLocation.ev_pc3=mze-mzs;
    LandmarkList tip_left;
    LandmarkList tip_right;
    LandmarkList tip_up ;
    LandmarkList tip_down;
    LandmarkList tip_out;
    LandmarkList tip_in;
    QFile ifs_swctemp(name);
    if(!ifs_swctemp.exists())
        return;
//    emit regionDone(mxs,mxe,mys,mye,mzs,mze);

    QFile ifs_swc(FinalFileName);
    vector<MyMarker*> finalswc;
    if(ifs_swc.exists()){
        lockfile.lockForRead();
        finalswc=readSWC_file(FinalFileName.toStdString());
        lockfile.unlock();
    }

    NeuronTree nt = readSWC_file(name);
    QList<NeuronSWC> list = nt.listNeuron;
    for (V3DLONG i=0;i<list.size();i++)
    {
            NeuronSWC curr = list.at(i);
            LocationSimple newTip;

            bool check_tip = false;
            if( curr.x < 0.05*  (mxe-mxs) || curr.x > 0.95 *  (mxe-mxs) || curr.y < 0.05 * (mye-mys) || curr.y > 0.95* (mye-mys)
                   || curr.z < 0.05*  (mze-mzs) || curr.z > 0.95 *  (mze-mzs))
            {
                newTip.x = curr.x + mxs;
                newTip.y = curr.y + mys;
                newTip.z = curr.z + mzs;
                newTip.radius = curr.r;

                for(V3DLONG j = 0; j < finalswc.size(); j++ )
                {
                    double dis = sqrt((newTip.x - finalswc.at(j)->x)*(newTip.x - finalswc.at(j)->x) + (newTip.y - finalswc.at(j)->y)*(newTip.y - finalswc.at(j)->y) + (newTip.z - finalswc.at(j)->z)*(newTip.z - finalswc.at(j)->z));
                    if(dis < 2*finalswc.at(j)->radius || dis < 20) //can be changed
                    {
                        check_tip = true;
                        break;
                    }
                }
            }
            if(check_tip) continue;
            if( curr.x < 0.05* (mxe-mxs))
            {
                tip_left.push_back(newTip);
            }else if (curr.x > 0.95 * (mxe-mxs))
            {
                tip_right.push_back(newTip);
            }else if (curr.y < 0.05 * (mye-mys))
            {
                tip_up.push_back(newTip);
            }else if (curr.y > 0.95*(mye-mys))
            {
                tip_down.push_back(newTip);
            }else if (curr.z < 0.05 * (mze-mzs))
            {
                tip_out.push_back(newTip);
            }else if (curr.z > 0.95*(mze-mzs))
            {
                tip_in.push_back(newTip);
            }
    }
    if(tip_left.size()>0)
    {
        QList<LandmarkList> group_tips_left = group_tips(tip_left,512,1);
        for(int i = 0; i < group_tips_left.size();i++)
            ada_win_finding_3D(group_tips_left.at(i),tileLocation,newTargetList,128,1);
    }
    if(tip_right.size()>0)
    {
        QList<LandmarkList> group_tips_right = group_tips(tip_right,512,2);
        for(int i = 0; i < group_tips_right.size();i++)
            ada_win_finding_3D(group_tips_right.at(i),tileLocation,newTargetList,128,2);
    }
    if(tip_up.size()>0)
    {
        QList<LandmarkList> group_tips_up = group_tips(tip_up,512,3);
        for(int i = 0; i < group_tips_up.size();i++)
            ada_win_finding_3D(group_tips_up.at(i),tileLocation,newTargetList,128,3);
    }
    if(tip_down.size()>0)
    {
        QList<LandmarkList> group_tips_down = group_tips(tip_down,512,4);
        for(int i = 0; i < group_tips_down.size();i++)
            ada_win_finding_3D(group_tips_down.at(i),tileLocation,newTargetList,128,4);
    }

    if(tip_out.size()>0)
    {
        QList<LandmarkList> group_tips_out = group_tips(tip_out,512,5);
        for(int i = 0; i < group_tips_out.size();i++)
            ada_win_finding_3D(group_tips_out.at(i),tileLocation,newTargetList,128,5);
    }

    if(tip_in.size()>0)
    {
        QList<LandmarkList> group_tips_in = group_tips(tip_in,512,6);
        for(int i = 0; i < group_tips_in.size();i++)
            ada_win_finding_3D(group_tips_in.at(i),tileLocation,newTargetList,128,6);
    }

    debug.lock();
    QFile file(DebugPath+"/output"+PluginRunStartTime+".txt");
    if (!file.open(QIODevice::Append | QIODevice::Text))
            v3d_msg(file.errorString());
    QTextStream out(&file);
    out <<this<<","<<mxs<<","<<mxe<<","<<mys<<","<<mye<<","<<mzs<<","<<mze<<","<<(mxs+mxe)/2<<","<<(mys+mye)/2<<","<<(mzs+mze)/2<<","<<name<<"\n";
    file.close();
    debug.unlock();

    lockfile.lockForWrite();
    if(ifs_swc.exists()){
        finalswc=readSWC_file(FinalFileName.toStdString());
    }
    if(ifs_swc.exists()){
        vector<MyMarker*> tileswc_file = readSWC_file(name.toStdString());
        for(int i=0;i<tileswc_file.size();i++){
            tileswc_file[i]->x+=mxs;
            tileswc_file[i]->y+=mys;
            tileswc_file[i]->z+=mzs;
            finalswc.push_back(tileswc_file[i]);
        }
        saveSWC_file(FinalFileName.toStdString().c_str(),finalswc);
    }else{   
        vector<MyMarker*> tileswc_file = readSWC_file(name.toStdString());
        for(int i=0;i<tileswc_file.size();i++){
            tileswc_file[i]->x+=mxs;
            tileswc_file[i]->y+=mys;
            tileswc_file[i]->z+=mzs;
        }
        saveSWC_file(FinalFileName.toStdString().c_str(),tileswc_file);   
    }
    lockfile.unlock();

    emit sendLandmark(*newTargetList);
}

QVector<long> PluginPipeLine::getBoundingBox(long xs, long ys, long zs, long xe, long ye, long ze)
{
    QVector<long> bb;
    bb.push_back(xs);
    bb.push_back(ys);
    bb.push_back(zs);
    bb.push_back(xe);
    bb.push_back(ye);
    bb.push_back(ze);
    return bb;
}

char *PluginPipeLine::qstring2char(QString text)
{
    char *result=new char[text.length()+1];
    result[text.length()]='\0';
    strcpy(result,text.toStdString().c_str());
    registerMemory((void*)result,0);
    return result;
}

void PluginPipeLine::LimitBoundary(QVector<long> &boundingbox,Image4DSimple* img)
{
    boundingbox[0]=boundingbox[0]<0?0:boundingbox[0];
    boundingbox[1]=boundingbox[1]<0?0:boundingbox[1];
    boundingbox[2]=boundingbox[2]<0?0:boundingbox[2];
    boundingbox[3]=boundingbox[3]>img->getXDim()?img->getXDim():boundingbox[3];
    boundingbox[4]=boundingbox[4]>img->getYDim()?img->getYDim():boundingbox[4];
    boundingbox[5]=boundingbox[5]>img->getZDim()?img->getZDim():boundingbox[5];
}

void PluginPipeLine::OutSourcedTask(QJsonObject &ost)
{

    outsourcework=new OutSource(this);

    outsourcework->asyncPostRequest(ost);
}

void PluginPipeLine::define(QString name,int type)
{
    QJsonObject p=packing(name,type);
    OutSourcedTask(p);
}

void PluginPipeLine::predict(QString name,PluginPipeLine* p)
{
    if(p!=this)
        return;
    Solution s;
    QString nameseg=s.seg(name,*_callback);
    if(nameseg.length()==0){
        SpLog::debug(name+" has too little infomation.");
        return;
    }
    Image4DSimple *img=_callback->loadImage(qstring2char(nameseg));

    Delivery delivery=PackImage(img);

    delivery.name=nameseg;
    delivery.IP=AlgorithmPredictIP;
    delivery.Port=AlgorithmPredictPort;
    sendDelivery(delivery);
}

void PluginPipeLine::Advantra(QString name)
{
    CallPlugin *cp;
    cp=new CallPlugin(*_callback,"Advantra","advantra_func");
    char *_input=qstring2char(name);
    char *_output=qstring2char(name+"_Advantra.swc");
    vector<char*> *input=new vector<char*>();
    vector<char*> *output=new vector<char*>();
    vector<char*> *inputpara=new vector<char*>();
    input->push_back(_input);
    output->push_back(_output);
    inputpara->push_back("10");
    inputpara->push_back("0.5");
    inputpara->push_back("0.75");
    inputpara->push_back("10");
    inputpara->push_back("60");
    inputpara->push_back("5");
    inputpara->push_back("5");
    inputpara->push_back("1");
    cp->AssignPara(input,0);
    cp->AssignPara(inputpara,1);
    cp->AssignPara(output,2);
    cp->type=4;

//    QString cpadd = QString::number(reinterpret_cast<quintptr>(cp), 16);
//    QString ppadd = QString::number(reinterpret_cast<quintptr>(this), 16);
//    SpLog::debug("ppadvantra:"+cpadd+" "+ppadd+" "+cp->getOutput()->at(0));

    TaskManage::get_instance()->addTask(cp,this);
}

void PluginPipeLine::neutube(QString name)
{
    CallPlugin *cp;
    cp=new CallPlugin(*_callback,"neutube","neutube_trace");
    char *_input=qstring2char(name);

    vector<char*> *input=new vector<char*>();
    vector<char*> *output=new vector<char*>();
    vector<char*> *inputpara=new vector<char*>();
    input->push_back(_input);

    cp->AssignPara(input,0);
    cp->AssignPara(inputpara,1);
    cp->AssignPara(output,2);
    cp->type=4;

    TaskManage::get_instance()->addTask(cp,this);
}

void PluginPipeLine::entv2s(QString name)
{
    CallPlugin *cp;
    cp=new CallPlugin(*_callback,"EnsembleNeuronTracerV2s","tracing_func");
    char *_input=qstring2char(name);

    vector<char*> *input=new vector<char*>();
    vector<char*> *output=new vector<char*>();
    vector<char*> *inputpara=new vector<char*>();
    input->push_back(_input);

    cp->AssignPara(input,0);
    cp->AssignPara(inputpara,1);
    cp->AssignPara(output,2);
    cp->type=4;

    TaskManage::get_instance()->addTask(cp,this);
}

void PluginPipeLine::entv2n(QString name)
{
    CallPlugin *cp;
    cp=new CallPlugin(*_callback,"EnsembleNeuronTracerV2n","tracing_func");
    char *_input=qstring2char(name);

    vector<char*> *input=new vector<char*>();
    vector<char*> *output=new vector<char*>();
    vector<char*> *inputpara=new vector<char*>();
    input->push_back(_input);

    cp->AssignPara(input,0);
    cp->AssignPara(inputpara,1);
    cp->AssignPara(output,2);
    cp->type=4;

    TaskManage::get_instance()->addTask(cp,this);
}

void PluginPipeLine::most(QString name)
{
    CallPlugin *cp;
    cp=new CallPlugin(*_callback,"mostVesselTracer","MOST_trace");
    char *_input=qstring2char(name);

    vector<char*> *input=new vector<char*>();
    vector<char*> *output=new vector<char*>();
    vector<char*> *inputpara=new vector<char*>();
    input->push_back(_input);

    cp->AssignPara(input,0);
    cp->AssignPara(inputpara,1);
    cp->AssignPara(output,2);
    cp->type=4;

    TaskManage::get_instance()->addTask(cp,this);
}

void PluginPipeLine::mst(QString name)
{
    CallPlugin *cp;
    cp=new CallPlugin(*_callback,"neurontracing_mst","trace_mst");
    char *_input=qstring2char(name);

    vector<char*> *input=new vector<char*>();
    vector<char*> *output=new vector<char*>();
    vector<char*> *inputpara=new vector<char*>();
    input->push_back(_input);

    cp->AssignPara(input,0);
    cp->AssignPara(inputpara,1);
    cp->AssignPara(output,2);
    cp->type=4;

    TaskManage::get_instance()->addTask(cp,this);
}

void PluginPipeLine::DetachConnectedDomain(QString name)
{
    CallPlugin *cp;
    cp=new CallPlugin(*_callback,"DetachConnectedDomain","seg");
    char *_input=qstring2char(name);

    vector<char*> *input=new vector<char*>();
    vector<char*> *output=new vector<char*>();
    vector<char*> *inputpara=new vector<char*>();
    input->push_back(_input);

    cp->AssignPara(input,0);
    cp->AssignPara(inputpara,1);
    cp->AssignPara(output,2);
    cp->type=2;

//    QString cpadd = QString::number(reinterpret_cast<quintptr>(cp), 16);
//    QString ppadd = QString::number(reinterpret_cast<quintptr>(this), 16);
//    SpLog::debug("ppadvantra:"+cpadd+" "+ppadd+" "+cp->getOutput()->at(0));

    TaskManage::get_instance()->addTask(cp,this);
}

void PluginPipeLine::selectAlgorithm(QString name, int method, PluginPipeLine *p)
{
    if(p!=this)
        return;


    tempSWCname=name+".swc";
    Solution s;
    QStringList detached=s.detach(name,*_callback);
    for(int i=0;i<detached.size();i++){
        SpLog::debug(detached[i]);
        if(method==app2id){
            tracingapp2(detached[i],p);
        }else if(method==advantraid){
            Advantra(detached[i]);
        }
        connectdomaincount++;
    }

}

void PluginPipeLine::submitSWC(QString swcname, PluginPipeLine *p)
{
    if(p!=this||SWCtoConcat.contains(swcname))
        return;
    SWCtoConcat.insert(swcname);
    SpLog::debug("concat: "+swcname+" "+QString::number(SWCtoConcat.size())+" "+QString::number(connectdomaincount));
    if(SWCtoConcat.size()==connectdomaincount){
        emit readytoconcat();
    }
}

void PluginPipeLine::ConcatSWC()
{
    vector<MyMarker*> finalswc;
    QSet<QString>::const_iterator it;
    for (it = SWCtoConcat.constBegin(); it != SWCtoConcat.constEnd(); ++it){
        vector<MyMarker*> tileswc_file = readSWC_file(it->toStdString());
        if(tileswc_file.size()==0)
            continue;
        for(int i=0;i<tileswc_file.size();i++){
            finalswc.push_back(tileswc_file[i]);
        }
    }
    saveSWC_file(tempSWCname.toStdString().c_str(),finalswc);
    emit concatdone(tempSWCname,this);
}

void PluginPipeLine::sendDelivery(Delivery delivery)
{
    if(!outsourcework){
        outsourcework=new OutSource(this);
        connect(outsourcework,SIGNAL(nextOperation(QString,int,PluginPipeLine*)),this,SLOT(selectAlgorithm(QString,int,PluginPipeLine*)));
        connect(outsourcework,SIGNAL(startpoint(int,int,int,PluginPipeLine*)),this,SLOT(ObtainStartPoint(int,int,int,PluginPipeLine*)));
    }

    outsourcework->sendDelivery(delivery);

}

void PluginPipeLine::deletethis()
{
    this->deleteLater();
}

void PluginPipeLine::getstartpoint(QString name,QString IP,QString Port)
{
    if(!outsourcework){
        outsourcework=new OutSource(this);
        connect(outsourcework,SIGNAL(nextOperation(QString,int,PluginPipeLine*)),this,SLOT(selectAlgorithm(QString,int,PluginPipeLine*)));
        connect(outsourcework,SIGNAL(startpoint(int,int,int,PluginPipeLine*)),this,SLOT(ObtainStartPoint(int,int,int,PluginPipeLine*)));
    }
    Image4DSimple *img=_callback->loadImage(qstring2char(name));

    Delivery delivery=PackImage(img,IP,Port,"",name,"",true,false);
    delivery.name=name;
    delivery.IP=IP;
    delivery.Port=Port;
    outsourcework->sendDelivery(delivery);
}

void PluginPipeLine::ObtainStartPoint(int x, int y, int z, PluginPipeLine *p)
{
    if(p!=this)
        return;
    px=x;
    py=y;
    pz=z;
    startready=true;
    emit finished();
}

vector<int> PluginPipeLine::getCentroid()
{

    vector<int> result;
    result.clear();
    result.push_back(px);
    result.push_back(py);
    result.push_back(pz);
    return result;
}

QList<LandmarkList> PluginPipeLine::group_tips(LandmarkList tips, int block_size, int direction)
{
    QList<LandmarkList> groupTips;

   //bubble sort
   if(direction == 1 || direction == 2 || direction == 5 || direction == 6)
   {
       for(int i = 0; i < tips.size();i++)
       {
           for(int j = 0; j < tips.size();j++)
           {
               if(tips.at(i).y < tips.at(j).y)
                   tips.swapItemsAt(i,j);
           }
       }

       LandmarkList eachGroupList;
       eachGroupList.push_back(tips.at(0));
       for(int d = 0; d < tips.size()-1; d++)
       {
           if(tips.at(d+1).y - tips.at(d).y < block_size)
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
                   tips.swapItemsAt(i,j);
           }
       }

       LandmarkList eachGroupList;
       eachGroupList.push_back(tips.at(0));
       for(int d = 0; d < tips.size()-1; d++)
       {
           if(tips.at(d+1).x - tips.at(d).x < block_size)
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

bool PluginPipeLine::ada_win_finding_3D(LandmarkList tips, LocationSimple tileLocation, LandmarkList *newTargetList, int block_size, int direction)
{
    double overlap = 0.1;

    float min_x = INF, max_x = -INF;
    float min_y = INF, max_y = -INF;
    float min_z = INF, max_z = -INF;


    double adaptive_size_x,adaptive_size_y,adaptive_size_z;
    double max_r = -INF;

    if(direction == 1 || direction == 2)
    {
        for(int i = 0; i<tips.size();i++)
        {
            if(tips.at(i).y <= min_y) min_y = tips.at(i).y;
            if(tips.at(i).y >= max_y) max_y = tips.at(i).y;

            if(tips.at(i).z <= min_z) min_z = tips.at(i).z;
            if(tips.at(i).z >= max_z) max_z = tips.at(i).z;

            if(tips.at(i).radius >= max_r) max_r = tips.at(i).radius;
        }

        adaptive_size_y = (max_y - min_y)*1.2;
        adaptive_size_z = (max_z - min_z)*1.2;
        adaptive_size_x = adaptive_size_y;

    }else if(direction == 3 || direction == 4)
    {
        for(int i = 0; i<tips.size();i++)
        {
            if(tips.at(i).x <= min_x) min_x = tips.at(i).x;
            if(tips.at(i).x >= max_x) max_x = tips.at(i).x;

            if(tips.at(i).z <= min_z) min_z = tips.at(i).z;
            if(tips.at(i).z >= max_z) max_z = tips.at(i).z;

            if(tips.at(i).radius >= max_r) max_r = tips.at(i).radius;
        }

        adaptive_size_x = (max_x - min_x)*1.2;
        adaptive_size_z = (max_z - min_z)*1.2;
        adaptive_size_y = adaptive_size_x;
    }else
    {
        for(int i = 0; i<tips.size();i++)
        {
            if(tips.at(i).x <= min_x) min_x = tips.at(i).x;
            if(tips.at(i).x >= max_x) max_x = tips.at(i).x;

            if(tips.at(i).y <= min_y) min_y = tips.at(i).y;
            if(tips.at(i).y >= max_y) max_y = tips.at(i).y;

            if(tips.at(i).radius >= max_r) max_r = tips.at(i).radius;
        }

        adaptive_size_x = (max_x - min_x)*1.2;
        adaptive_size_y = (max_y - min_y)*1.2;
        adaptive_size_z = adaptive_size_x;
    }

    adaptive_size_x = (adaptive_size_x <= 128) ? 128 : adaptive_size_x;
    adaptive_size_y = (adaptive_size_y <= 128) ? 128 : adaptive_size_y;
    adaptive_size_z = (adaptive_size_z <= 128) ? 128 : adaptive_size_z;


    adaptive_size_x = (adaptive_size_x >= block_size) ? block_size : adaptive_size_x;
    adaptive_size_y = (adaptive_size_y >= block_size) ? block_size : adaptive_size_y;
    adaptive_size_z = (adaptive_size_z >= block_size) ? block_size : adaptive_size_z;

    LocationSimple newTarget;
    if(direction == 1)
    {
        newTarget.x = -floor(adaptive_size_x*(1.0-overlap)) + tileLocation.x;
        newTarget.y = floor((min_y + max_y - adaptive_size_y)/2);
        newTarget.z = floor((min_z + max_z - adaptive_size_z)/2);
    }else if(direction == 2)
    {
        newTarget.x = tileLocation.x + tileLocation.ev_pc1 - floor(adaptive_size_x*overlap);
        newTarget.y = floor((min_y + max_y - adaptive_size_y)/2);
        newTarget.z = floor((min_z + max_z - adaptive_size_z)/2);

    }else if(direction == 3)
    {
        newTarget.x = floor((min_x + max_x - adaptive_size_x)/2);
        newTarget.y = -floor(adaptive_size_y*(1.0-overlap)) + tileLocation.y;
        newTarget.z = floor((min_z + max_z - adaptive_size_z)/2);
    }else if(direction == 4)
    {
        newTarget.x = floor((min_x + max_x - adaptive_size_x)/2);
        newTarget.y = tileLocation.y + tileLocation.ev_pc2 - floor(adaptive_size_y*overlap);
        newTarget.z = floor((min_z + max_z - adaptive_size_z)/2);
    }else if(direction == 5)
    {
        newTarget.x = floor((min_x + max_x - adaptive_size_x)/2);
        newTarget.y = floor((min_y + max_y - adaptive_size_y)/2);
        newTarget.z = -floor(adaptive_size_z*(1.0-overlap)) + tileLocation.z;
    }else if(direction == 6)
    {
        newTarget.x = floor((min_x + max_x - adaptive_size_x)/2);
        newTarget.y = floor((min_y + max_y - adaptive_size_y)/2);
        newTarget.z = tileLocation.z + tileLocation.ev_pc3 - floor(adaptive_size_z*overlap);
    }


   // v3d_msg(QString("zmin is %1, zmax is %2, z is %3, z_winsize is %4").arg(min_z).arg(max_z).arg(tileLocation.z).arg(adaptive_size_z));
    newTarget.ev_pc1 = adaptive_size_x;
    newTarget.ev_pc2 = adaptive_size_y;
    newTarget.ev_pc3 = adaptive_size_z;

    newTarget.radius = max_r;

    newTargetList->push_back(newTarget);
    return true;
}

QJsonObject PluginPipeLine::packing(QString path, int type)
{
    QJsonObject pack;
    pack.insert("path",path);
    pack.insert("type",type);
    return pack;
}



