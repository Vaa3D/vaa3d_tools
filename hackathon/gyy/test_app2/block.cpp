#include "block.h"

void seg_neuron(block &b)
{
    QString path1 = QFileDialog::getOpenFileName(parent, QString(QObject::tr("open file")), "/home/gyy/Desktop");
    NeuronTree nt = readSWC_file(path1);
    V3DLONG size_nt = nt.listNeuron.size();
    V3DLONG max_n = size_nt, min_n = 1;
    vector<NeuronTree> vec_tree;
    for(V3DLONG i = 0; i < size_nt; i ++)
    {
        if(nt.listNeuron[i].parent<min_n && nt.listNeuron[i].parent>max_n)
        {
            NeuronTree nt1;
            nt1.listNeuron.push_back(nt.listNeuron[i]);
            vec_tree.push_back(nt1);
        }
        else
        {
            vec_tree.back().listNeuron.push_back(nt.listNeuron[i]);
        }
    }
    for(V3DLONG j = 0; j < vec_tree.size(); j++)
    {
        QString path2 = "/home/balala/Desktop/data";
        writeESWC_file(vec_tree[j], path2);
    }

}
