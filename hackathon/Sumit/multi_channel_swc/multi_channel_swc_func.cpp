#include "multi_channel_swc_func.h"
#include <QtGui>

NeuronTree readSWC_file_multichannel(const QString& filename)
{
    NeuronTree nt = readSWC_file(filename);
    QFile qf(filename);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(filename));
#endif
        return nt;
    }
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        //  add #name, #comment
        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
            //found the CHANNELSWC and start reading features
            if (buf[1]=='C'&&buf[2]=='H'&&buf[3]=='A'&&buf[4]=='N'&&buf[5]=='N'&&buf[6]=='E'&&buf[7]=='L'&&buf[8]=='S'&&buf[9]=='W'&&buf[10]=='C'){
                if(! qf.atEnd()) qf.readLine(_buf, sizeof(_buf));
                QStringList qsl;
                //get numbers
                V3DLONG nodeNum=0;
                int feaNum=0;
                int channelNum=0;
                if (buf[0]=='#') qsl = QString(buf+1).split(" ",QString::SkipEmptyParts);
                if(qsl.size()==3){
                    nodeNum=qsl[2].toInt();
                    feaNum=qsl[1].toInt();
                    channelNum=qsl[0].toInt();
                }else{
                    qDebug()<<"ERROR: cannot read features in "<<filename;
                    qDebug()<<"expect [#channelNumber featureNumber rowNumber] after [#CHANNELSWC]";
                    break;
                }
                if(nodeNum!=nt.listNeuron.size()){
                    qDebug()<<"ERROR: the number of rows does not match with the number of nodes in neuron tree";
                    break;
                }
                //skip extra line
                if(! qf.atEnd()) qf.readLine(_buf, sizeof(_buf));
                //start reading features
                V3DLONG i=0;
                for(i=0; i<nodeNum; i++){
                    if(! qf.atEnd()) qf.readLine(_buf, sizeof(_buf));
                    else break;
                    if((buf[0]=='#')) qsl = QString(buf+1).split(" ",QString::SkipEmptyParts);
                    else break;
                    if(qsl.size()!=feaNum){
                        qDebug()<<"WARNING: the number of features in row "<<i+1<<" does not match expectation: "<<feaNum;
                        qDebug()<<"Missing features will be set to 0, extra features will be dumped";
                    }
                    nt.listNeuron[i].fea_val.clear();
                    for(int j=0; j<feaNum; j++){
                        float tmp=0;
                        if(j<qsl.size()) tmp=qsl[j].toFloat();
                        nt.listNeuron[i].fea_val.push_back(tmp);
                    }
                }
                break;
            }
            continue;
        }
    }

    return nt;
}

NeuronTree convert_SWC_to_render_ESWC(const NeuronTree &source)
{
    NeuronTree eswc;
    if(source.listNeuron.size()==0)
        return eswc;
    int chNum=source.listNeuron.at(0).fea_val.size()/2;
    eswc.n=source.n*(chNum+1);
    eswc.on=source.on;
    eswc.selected=source.selected;
    eswc.name=source.name;
    eswc.comment=source.comment;
    eswc.color.r=source.color.r;
    eswc.color.g=source.color.g;
    eswc.color.b=source.color.b;
    eswc.color.a=source.color.a;
    eswc.listNeuron.clear();
    eswc.file     = source.file;
    eswc.editable = source.editable;
    eswc.linemode = source.linemode;
    V3DLONG maxN=0;

    eswc.hashNeuron.clear();
    eswc.listNeuron.clear();
    for(V3DLONG i=0; i<source.listNeuron.size(); i++)
    {
        NeuronSWC S;
        S.n = source.listNeuron[i].n;
        S.type = source.listNeuron[i].type;
        S.x = source.listNeuron[i].x;
        S.y = source.listNeuron[i].y;
        S.z = source.listNeuron[i].z;
        S.r = source.listNeuron[i].r;
        S.pn = source.listNeuron[i].pn;
        S.seg_id = source.listNeuron[i].seg_id;
        S.level = source.listNeuron[i].level;
        S.fea_val.append(-1);
        eswc.listNeuron.append(S);
        eswc.hashNeuron.insert(S.n, eswc.listNeuron.size()-1);
        maxN=MAX(S.n,maxN);
    }
    maxN++;
    for(int cid=0; cid<chNum; cid++){
        for(V3DLONG i=0; i<source.listNeuron.size(); i++)
        {
            NeuronSWC S;
            S.n = source.listNeuron[i].n + (cid+1)*maxN;
            S.type = 6+cid;
            S.x = source.listNeuron[i].x;
            S.y = source.listNeuron[i].y;
            S.z = source.listNeuron[i].z;
            S.r = source.listNeuron[i].r*source.listNeuron.at(i).fea_val.at(cid*2);
            if(source.listNeuron[i].pn>=0)
                S.pn = source.listNeuron[i].pn + (cid+1)*maxN;
            else
                S.pn = source.listNeuron[i].pn;
            S.seg_id = source.listNeuron[i].seg_id;
            S.level = source.listNeuron[i].level;
            S.fea_val.append(source.listNeuron[i].fea_val.at(cid*2+1));
            eswc.listNeuron.append(S);
            eswc.hashNeuron.insert(S.n, eswc.listNeuron.size()-1);
        }
    }

    return eswc;
}
