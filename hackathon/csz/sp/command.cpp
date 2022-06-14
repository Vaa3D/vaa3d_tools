#include "command.h"

void Command::readCmdTxt(QString CmdTxt)
{
//    vector<vector<char *>> datacmd;
    if(iput==0){
        qDebug()<<"You must define the input path";
        return;
    }
    QFile qf(CmdTxt);
    qDebug()<<CmdTxt;
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        v3d_msg(QString("open file [%1] failed!").arg(CmdTxt));
        //return datacmd;
        return;
    }

    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space
        if (buf[0]=='#')
        {
            continue;
        }

        QStringList qsl = QString(buf).trimmed().split(" ");
        if (qsl.size()==0)   continue;

//        for(auto &q:qsl){
//            qDebug()<<q;
//        }
        this->convertcmd(qsl);
        //qDebug()<<"------------";
    }
}

void Command::convertcmd(QStringList txtline)
{
    QString input=QString(iput);
    vector<vector<char *>> dataflowarg;
    dataflowarg.clear();
    vector<char *> funcparas;
    funcparas.clear();
    //QString output=QString(otput);

    char *ipimg=(input+"\\"+txtline[1]).toLatin1().data();

    QString suffix=txtline[1].split(".")[txtline[1].split(".").size()-1];

    Image4DSimple *nimg=new Image4DSimple;
    if(suffix=="tiff"||suffix=="v3draw"||suffix=="v3dpbd")
        nimg=mcallback->loadImage(ipimg);
    NeuronTree *nt=new NeuronTree;
    if(suffix=="swc"||suffix=="eswc")
        *nt=readSWC_file(input+"\\"+txtline[1]);
    DataFlow *ndtf=new DataFlow;
    ndtf->push_img(nimg);
    ndtf->push_swc(nt);

    ndtf->dataname.append(txtline[1]);
    ndtf->otdataname.append(txtline[2]);

    dtf.emplace_back(ndtf);
    otresult.emplace_back(txtline[3]);

    for(int i=4;i<txtline.size();i++){
        qDebug()<<txtline[i];
        char *para=new char(txtline[i].size()+1);
        for(int j=0;j<=txtline[i].size();j++){
            para[j]=txtline[i].toLatin1().data()[j];
        }
        para[txtline[i].size()]='\0';
        //qDebug()<<"csz debug "<<para;
        if(txtline[i][0]>='a'&&txtline[i][0]<='z'){         //DataFlowArg[0] is empty.
            dataflowarg.emplace_back(funcparas);
            funcparas.clear();
            funcparas.emplace_back(para);
            if(i==txtline.size()-1)
                dataflowarg.emplace_back(funcparas);
            continue;
        }
        funcparas.emplace_back(para);
        if(i==txtline.size()-1)
            dataflowarg.emplace_back(funcparas);
    }
    DataFlowCmd.emplace_back(dataflowarg);
}

void Command::acceptcmd(const V3DPluginArgList &input, V3DPluginArgList &output)
{
    iput=(*(vector<char*> *)(input.at(0).p)).at(0);
    otput=(*(vector<char*> *)(output.at(0).p)).at(0);

//    paralist=(*(vector<char*> *)(input.at(1).p));
//    qDebug()<<inputlist[0];
//    qDebug()<<outputlist[0];
//    otpath=QString(outputlist[0]);
//    vector<char *> funcparas;
//    for(int i=2;i<paralist.size();i++){
//        if(paralist[i][0]>='a'&&paralist[i][0]<='z'){         //DataFlowArg[0] is empty.
//            DataFlowCmd.push_back(funcparas);
//            funcparas.clear();
//            funcparas.push_back(paralist[i]);
//            if(i==paralist.size()-1)
//                DataFlowCmd.push_back(funcparas);
//            continue;
//        }
//        funcparas.push_back(paralist[i]);
//        if(i==paralist.size()-1)
//            DataFlowCmd.push_back(funcparas);
//    }
}
