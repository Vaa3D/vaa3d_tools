#include "function_.h"
#include "qdir.h"

#include "v3d_basicdatatype.h"
#include "color_xyz.h"
#include "v3d_message.h"

using namespace std;

int getNum_file(const QString &dirPath)
{
    QDir dir(dirPath);
    QStringList filter;
    QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir.entryInfoList(QDir::Files));
    int fileNum = fileInfo -> count();
    //foreach(QString subDir, dir.entryList())
    return fileNum;
}


vector<NeuronSWC> tip_detection1(NeuronTree &nt)
{
    V3DLONG size_nt = nt.listNeuron.size();
    vector< vector<V3DLONG> > childs;
    childs = vector< vector<V3DLONG> >(nt.listNeuron.size(), vector<V3DLONG>() );
    nt.hashNeuron.clear();

    for(V3DLONG i = 0; i < size_nt; i++)
    {
        nt.hashNeuron.insert(nt.listNeuron[i].n, i);
    }
    for (V3DLONG i=0; i<nt.listNeuron.size();i++)
    {
        if (nt.listNeuron[i].pn<0) continue;
        childs[nt.hashNeuron.value(nt.listNeuron[i].pn)].push_back(i);

    }
    vector<NeuronSWC> branchpoints,tips;
    int child_num;

    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        NeuronSWC cur = nt.listNeuron[i];
        child_num = childs[i].size();   //need check
        //child_num = childs[nt.listNeuron[i].n].size();
        if(child_num==2)
            branchpoints.push_back(cur);
        else if(child_num==0)
            tips.push_back(cur);
    }
    return tips;
}
















