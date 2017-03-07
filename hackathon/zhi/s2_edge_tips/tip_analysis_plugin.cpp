/* tip_analysis_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-11-3 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "tip_analysis_plugin.h"

#include <boost/lexical_cast.hpp>
#include "../AllenNeuron_postprocessing/sort_swc_IVSCC.h"

#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/eswc_converter/eswc_converter_func.h"


using namespace std;
Q_EXPORT_PLUGIN2(tip_analysis, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("analysis")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}

void analysisSmartScan(QString fileWithData);
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


void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("analysis"))
	{
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                "",
                QObject::tr("Supported file (*.txt *.TXT)"
                    ));
        if(fileOpenName.isEmpty())
            return;
        analysisSmartScan(fileOpenName);
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2016-11-3"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void analysisSmartScan(QString fileWithData)
{
    ifstream ifs(fileWithData.toLatin1());
    string info_swc;
    int offsetX, offsetY,sizeX, sizeY;
    string swcfilename;
    vector<MyMarker*> outswc,inputswc;
    int offsetX_min = 10000000,offsetY_min = 10000000,offsetX_max = -10000000,offsetY_max =-10000000;

    QString fileSaveName = fileWithData + "_tips.swc";


    while(ifs && getline(ifs, info_swc))
    {
        std::istringstream iss(info_swc);
        iss >> offsetX >> offsetY >> swcfilename >> sizeX >> sizeY;
        if(offsetX < offsetX_min) offsetX_min = offsetX;
        if(offsetY < offsetY_min) offsetY_min = offsetY;
        if(offsetX > offsetX_max) offsetX_max = offsetX;
        if(offsetY > offsetY_max) offsetY_max = offsetY;

        QString folderpath = QFileInfo(fileWithData).absolutePath();
        QString swcfilepath = folderpath + '/' + QFileInfo(QString::fromStdString(swcfilename)).completeBaseName() + ".swc";

        inputswc = readSWC_file(swcfilepath.toStdString());
        for(V3DLONG d = 0; d < inputswc.size(); d++)
        {
            if( inputswc[d]->x < 0.05*  sizeX || inputswc[d]->x > 0.95 *  sizeX || inputswc[d]->y < 0.05 * sizeY || inputswc[d]->y > 0.95* sizeY)
            {
                inputswc[d]->type = 1;
            }else
                inputswc[d]->type = 2;

            inputswc[d]->x = inputswc[d]->x + offsetX;
            inputswc[d]->y = inputswc[d]->y + offsetY;
            outswc.push_back(inputswc[d]);
        }
    }
    ifs.close();

    for(V3DLONG i = 0; i < outswc.size(); i++)
    {
        outswc[i]->x = outswc[i]->x - offsetX_min;
        outswc[i]->y = outswc[i]->y - offsetY_min;
    }

    saveSWC_file(fileSaveName.toStdString().c_str(), outswc);
    NeuronTree nt = readSWC_file(fileSaveName);
    NeuronTree nt_sort = SortSWC_pipeline(nt.listNeuron,VOID, 100000);
    NeuronTree nt_sort_prune = pruneswc(nt_sort,5);
    NeuronTree nt_sort_prune_2nd = pruneswc(nt_sort_prune,5);
    NeuronTree nt_sort_prune_sort = SortSWC_pipeline(nt_sort_prune_2nd.listNeuron,VOID,10);

    QVector<QVector<V3DLONG> > childs;
    V3DLONG neuronNum = nt_sort_prune_sort.listNeuron.size();
    childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

    for (V3DLONG i=0;i<neuronNum;i++)
    {
        V3DLONG par = nt_sort_prune_sort.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt_sort_prune_sort.hashNeuron.value(par)].push_back(i);
    }

    vector<V3DLONG> segment_id, segment_layer;
    segment_id, segment_layer;
    if (!swc2eswc(nt_sort_prune_sort,segment_id, segment_layer))
    {
        v3d_msg("Cannot convert swc to eswc.\n");
        return;
    }

    QList<NeuronSWC> list = nt_sort_prune_sort.listNeuron;
    QString AllmarkerfileName = fileWithData + "_tips.marker";
    QString edgeMarkerFileName = fileWithData + "_edgeTips.marker";
    QList<ImageMarker> tips_marker;
    QList<ImageMarker> edge_tips_markers;
    int edge_num = 0;

    for (int i=0;i<list.size();i++)
    {
        ImageMarker t;
        if (childs[i].size() ==0)
        {
            t.x = list.at(i).x;
            t.y = list.at(i).y;
            t.z = list.at(i).z;
            bool flag = false;
            for (int j=0;j<list.size();j++)
            {
                double dis = sqrt(pow2(t.x - list.at(j).x) + pow2(t.y - list.at(j).y) + pow2(t.z - list.at(j).z));
                if(dis < 10.0 && segment_id[i]!= segment_id[j])
                {
                   flag = true;
                   break;
                }
            }
            if(flag) continue;
            tips_marker.append(t);
            if(list.at(i).type == 1){
                edge_tips_markers.append(t);
                edge_num++;}
        }
    }
    double ratio =  (double)edge_num/tips_marker.size();

//    double angThr=cos((180-30)/180*M_PI);

//    connectall(&nt_final, neuron_final_connected, 1, 1, 1, angThr, 30, 1, 0, -1);
    writeMarker_file(AllmarkerfileName, tips_marker);
    writeMarker_file(edgeMarkerFileName, edge_tips_markers);
    export_list2file(nt_sort_prune_sort.listNeuron, fileSaveName,fileSaveName);
    v3d_msg(QString("edge (%1) to tip (%2) ratio is %3,").arg(edge_num).arg(tips_marker.size()).arg((double)ratio));
}
