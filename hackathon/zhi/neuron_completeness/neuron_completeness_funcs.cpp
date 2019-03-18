/* neuron_completeness_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-2-25 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include "neuron_completeness_funcs.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"

QStringList importSWCFileList(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";


    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}


double minDist(QList<NeuronSWC> & neuron1, QList<NeuronSWC> & neuron2)
{
    double dis=MAX_DOUBLE;
    for(int i=0;i<neuron1.size();i++)
    {
        for(int j=0;j<neuron2.size();j++)
        {
            if(NTDIS(neuron1[i],neuron2[j])<dis)
                dis = NTDIS(neuron1[i],neuron2[j]);
        }
    }
    return dis;
}

void calComplete(NeuronTree nt, QList<NEURON_METRICS> & scores)
{
    QList<NeuronSWC> sorted_neuron;
    SortSWC(nt.listNeuron, sorted_neuron ,VOID, 0);

    LandmarkList markerlist;

    QHash<int,int> map_type;
    QMultiMap<int, QList<NeuronSWC> > multi_neurons;
    int first = 0;
    for(V3DLONG i=0; i<sorted_neuron.size();i++)
    {
        if(sorted_neuron.at(i).pn == -1)
        {
            if(i!=0)
            {
                multi_neurons.insert(i-first,sorted_neuron.mid(first,i-first));
            }
            first = i;

        }
        if(!map_type.count(sorted_neuron.at(i).type))
        {
            map_type[sorted_neuron.at(i).type] = 1;
        }
    }
    multi_neurons.insert(sorted_neuron.size()-first,sorted_neuron.mid(first,sorted_neuron.size()-first));
    QList<double> dist;
    QVector<QVector<double> > matrix_dis(multi_neurons.size(),QVector<double>(multi_neurons.size(),MAX_DOUBLE));
    int row =0;
    for (QMultiMap<int, QList<NeuronSWC> >::iterator it1 = multi_neurons.end()-1; it1 != multi_neurons.begin()-1; --it1)
    {
        QList<NeuronSWC> neuron1= it1.value();
        double minD = MAX_DOUBLE;
        int col=0;
        for (QMultiMap<int, QList<NeuronSWC> >::iterator it2 = multi_neurons.end()-1; it2 != multi_neurons.begin()-1; --it2)
        {
            if(it1 != it2)
            {
                if(matrix_dis[row][col] != MAX_DOUBLE)
                    minD = MIN(minD,matrix_dis[row][col]);
                else
                {
                    QList<NeuronSWC> neuron2 = it2.value();
                    double cur_dis = minDist(neuron1,neuron2);
                    matrix_dis[row][col] = cur_dis;
                    matrix_dis[col][row] = cur_dis;
                    minD = MIN(minD,cur_dis);
                }
            }
            col++;
        }
        row++;
        dist.push_back(minD);
        if(minD>2 && minD!=MAX_DOUBLE)
        {
            LocationSimple t;
            t.x = neuron1.at(0).x;
            t.y = neuron1.at(0).y;
            t.z = neuron1.at(0).z;
            t.color.r = 0;t.color.g = 0; t.color.b = 255;
            markerlist.push_back(t);
        }
    }

    NEURON_METRICS tmp;
    tmp.numTrees = multi_neurons.size();
    tmp.numTypes = map_type.size();
    tmp.numSegs = markerlist.size();
    scores.push_back(tmp);
    return;
}

void exportComplete(NeuronTree nt,QList<NeuronSWC>& sorted_neuron, LandmarkList& markerlist, QMultiMap<int, QList<NeuronSWC> >& multi_neurons,QHash<int,int>& map_type,QList<double>& dist)
{
    SortSWC(nt.listNeuron, sorted_neuron ,VOID, 0);

    int first = 0;
    int cur_type = -1;
    for(V3DLONG i=0; i<sorted_neuron.size();i++)
    {
        if(sorted_neuron.at(i).pn == -1)
        {
            if(i!=0)
            {
                multi_neurons.insert(i-first,sorted_neuron.mid(first,i-first));
            }
            first = i;

        }
        if(!map_type.count(sorted_neuron.at(i).type))
        {
            map_type[sorted_neuron.at(i).type] = 1;
//            if(sorted_neuron.at(i).type!=1 && sorted_neuron.at(i).type!=2 && sorted_neuron.at(i).type!=3)
//            {
//                LocationSimple t;
//                t.x = sorted_neuron.at(i).x;
//                t.y = sorted_neuron.at(i).y;
//                t.z = sorted_neuron.at(i).z;
//                t.color.r = 255;t.color.g = 0; t.color.b = 0;
//                markerlist.push_back(t);
//            }
        }/*else
        {
            if(sorted_neuron.at(i).type!=1 && sorted_neuron.at(i).type!=2 && sorted_neuron.at(i).type!=3 && sorted_neuron.at(i).type !=cur_type)
            {
                LocationSimple t;
                t.x = sorted_neuron.at(i).x;
                t.y = sorted_neuron.at(i).y;
                t.z = sorted_neuron.at(i).z;
                t.color.r = 255;t.color.g = 0; t.color.b = 0;
                markerlist.push_back(t);
            }
        }*/
        cur_type = sorted_neuron.at(i).type;
    }
    multi_neurons.insert(sorted_neuron.size()-first,sorted_neuron.mid(first,sorted_neuron.size()-first));
    QVector<QVector<double> > matrix_dis(multi_neurons.size(),QVector<double>(multi_neurons.size(),MAX_DOUBLE));
    int row =0;
    QMultiMap<double, LocationSimple> greenSeg_list;
    for (QMultiMap<int, QList<NeuronSWC> >::iterator it1 = multi_neurons.end()-1; it1 != multi_neurons.begin()-1; --it1)
    {
        QList<NeuronSWC> neuron1= it1.value();
        double minD = MAX_DOUBLE;
        int col=0;
        for (QMultiMap<int, QList<NeuronSWC> >::iterator it2 = multi_neurons.end()-1; it2 != multi_neurons.begin()-1; --it2)
        {
            if(it1 != it2)
            {
                if(matrix_dis[row][col] != MAX_DOUBLE)
                    minD = MIN(minD,matrix_dis[row][col]);
                else
                {
                    QList<NeuronSWC> neuron2 = it2.value();
                    double cur_dis = minDist(neuron1,neuron2);
                    matrix_dis[row][col] = cur_dis;
                    matrix_dis[col][row] = cur_dis;
                    minD = MIN(minD,cur_dis);
                }
            }
            col++;
        }
        row++;
        dist.push_back(minD);
        LocationSimple t;
        t.x = neuron1.at(0).x;
        t.y = neuron1.at(0).y;
        t.z = neuron1.at(0).z;
        if (minD>2 && minD!=MAX_DOUBLE)
        {
            greenSeg_list.insert(minD,t);
        }
    }
    for(QMultiMap<double, LocationSimple>::iterator it = greenSeg_list.end()-1; it != greenSeg_list.begin()-1; --it)
    {
        LocationSimple t = it.value();
        t.color.r = 0;t.color.g = 255; t.color.b = 0;t.comments = QString("gap: %1").arg(QString::number(it.key(),'f',2)).toStdString();
        markerlist.push_back(t);
    }
    return;

}
