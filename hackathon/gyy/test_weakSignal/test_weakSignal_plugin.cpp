/* test_weakSignal_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-18 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "test_weakSignal_plugin.h"
#include <iostream>
#include <QFileDialog>
using namespace std;
Q_EXPORT_PLUGIN2(test_weakSignal, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("weak_signal")
        <<tr("cross")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("weak_signal"))
    {
        //v3d_msg("To be implemented.");
        QString path_swc = QFileDialog::getOpenFileName(parent, QString(QObject::tr("open file swc:")));
        NeuronTree nt_swc = readSWC_file(path_swc);
        //QFileInfo fileInfo_swc(path_swc);
        QString path_eswc = QFileDialog::getOpenFileName(parent, QString(QObject::tr("open file eswc:")));
        NeuronTree nt_eswc = readSWC_file(path_eswc);
        QFileInfo fileInfo_eswc(path_eswc);
        V3DLONG size_swc = nt_swc.listNeuron.size();
        V3DLONG size_eswc = nt_eswc.listNeuron.size();
        QList<ImageMarker> markers;
        //V3DLONG size_max = (size_swc>size_eswc)?size_swc:size_eswc;
        //V3DLONG size_min = (size_swc<size_eswc)?size_swc:size_eswc;
        V3DLONG k=0, count=10;

        cout<<"size_swc="<<size_swc<<endl;
        cout<<"size_eswc="<<size_eswc<<endl;
        float dt_min[size_eswc];
        //if(fileInfo_eswc.baseName()==fileInfo_swc.baseName())
        for(V3DLONG i = 1; i <= size_eswc; i ++)
        {
            dt_min[i] = (nt_eswc.listNeuron[i].x-nt_swc.listNeuron[1].x)*(nt_eswc.listNeuron[i].x-nt_swc.listNeuron[1].x)
                    +(nt_eswc.listNeuron[i].y-nt_swc.listNeuron[1].y)*(nt_eswc.listNeuron[i].y-nt_swc.listNeuron[1].y)
                    +(nt_eswc.listNeuron[i].z-nt_swc.listNeuron[1].z)*(nt_eswc.listNeuron[i].z-nt_swc.listNeuron[1].z);
            for(V3DLONG j = 1; j < size_swc; j ++)
            {
                if(((nt_eswc.listNeuron[i].x-nt_swc.listNeuron[j].x)*(nt_eswc.listNeuron[i].x-nt_swc.listNeuron[j].x)
                    +(nt_eswc.listNeuron[i].y-nt_swc.listNeuron[j].y)*(nt_eswc.listNeuron[i].y-nt_swc.listNeuron[j].y)
                    +(nt_eswc.listNeuron[i].z-nt_swc.listNeuron[j].z)*(nt_eswc.listNeuron[i].z-nt_swc.listNeuron[j].z))<dt_min[i])
                {
                    dt_min[i]=(nt_eswc.listNeuron[i].x-nt_swc.listNeuron[j].x)*(nt_eswc.listNeuron[i].x-nt_swc.listNeuron[j].x)
                            +(nt_eswc.listNeuron[i].y-nt_swc.listNeuron[j].y)*(nt_eswc.listNeuron[i].y-nt_swc.listNeuron[j].y)
                            +(nt_eswc.listNeuron[i].z-nt_swc.listNeuron[j].z)*(nt_eswc.listNeuron[i].z-nt_swc.listNeuron[j].z);
                }
            }
            //cout<<"dt_min"<<dt_min[i]<<endl;
            if(dt_min[i]>4)
            {
                k++;
                if(k>=count)
                {
                    ImageMarker marker;
                    marker.x = nt_eswc.listNeuron[i-10].x;
                    marker.y = nt_eswc.listNeuron[i-10].y;
                    marker.z = nt_eswc.listNeuron[i-10].z;
                    markers.push_back(marker);
                    cout<<"markers0: x:"<<marker.x<<" y:"<<marker.y<<" z:"<<marker.z<<endl;
                    k=0;
                }
                else
                    k = 0;
            }
        }
        if(size_eswc>size_swc)
        {
            ImageMarker marker;
            marker.x = nt_eswc.listNeuron[size_swc-1].x;
            marker.y = nt_eswc.listNeuron[size_swc-1].y;
            marker.z = nt_eswc.listNeuron[size_swc-1].z;
            markers.push_back(marker);
            cout<<"markers1: x:"<<marker.x<<" y:"<<marker.y<<" z:"<<marker.z<<endl;
        }
        else
        {
            ImageMarker marker;
            marker.x = nt_eswc.listNeuron[size_eswc-1].x;
            marker.y = nt_eswc.listNeuron[size_eswc-1].y;
            marker.z = nt_eswc.listNeuron[size_eswc-1].z;
            markers.push_back(marker);
            cout<<"markers1: x:"<<marker.x<<" y:"<<marker.y<<" z:"<<marker.z<<endl;

        }

    cout<<"markers.size()="<<markers.size()<<endl;
    QString path_marker = "/home/balala/Desktop/markers/"+fileInfo_eswc.baseName()+".marker";
    if(!writeMarker_file(path_marker,markers))
        return ;
        //}
	}

    else if (menu_name == tr("cross"))
	{
        //v3d_msg("To be implemented.");
        QString path_swc = QFileDialog::getOpenFileName(parent, QString(QObject::tr("open file swc:")));
        NeuronTree nt_swc = readSWC_file(path_swc);
        //QFileInfo fileInfo_swc(path_swc);
        QString path_eswc = QFileDialog::getOpenFileName(parent, QString(QObject::tr("open file eswc:")));
        NeuronTree nt_eswc = readSWC_file(path_eswc);
        QFileInfo fileInfo_eswc(path_eswc);
        //V3DLONG size_swc = nt_swc.listNeuron.size();
        //V3DLONG size_eswc = nt_eswc.listNeuron.size();
        vector<vector<V3DLONG> >childs1, childs2;
        childs1 = vector<vector<V3DLONG> >(nt_swc.listNeuron.size(), vector<V3DLONG>());
        childs2 = vector<vector<V3DLONG> >(nt_eswc.listNeuron.size(), vector<V3DLONG>());
        nt_swc.hashNeuron.clear();
        nt_eswc.hashNeuron.clear();
        for(V3DLONG i = 0; i < nt_swc.listNeuron.size(); i ++)
        {
            nt_swc.hashNeuron.insert(nt_swc.listNeuron[i].n, i);
        }
        for(V3DLONG i = 0; i < nt_eswc.listNeuron.size(); i ++)
        {
            nt_eswc.hashNeuron.insert(nt_eswc.listNeuron[i].n, i);
        }
        for(V3DLONG k = 0; k < nt_eswc.listNeuron.size(); k ++)
        {
            if(nt_eswc.listNeuron[k].pn<0) continue;
            childs2[nt_eswc.hashNeuron.value(nt_eswc.listNeuron[k].pn)].push_back(k);
        }
        for(V3DLONG k = 0; k < nt_swc.listNeuron.size(); k ++)
        {
            if(nt_swc.listNeuron[k].pn<0) continue;
            childs1[nt_swc.hashNeuron.value(nt_swc.listNeuron[k].pn)].push_back(k);
        }
        vector<NeuronSWC> bif_eswc, bif_swc;
        vector<NeuronSWC> tip_eswc, tip_swc;
        for(V3DLONG i = 0; i < nt_eswc.listNeuron.size(); i ++)
        {
            if(childs2[i].size()==2)
                bif_eswc.push_back(nt_eswc.listNeuron[i]);
            if(childs2[i].size()==0)
                tip_eswc.push_back(nt_eswc.listNeuron[i]);
        }
        for(V3DLONG i = 0; i < nt_swc.listNeuron.size(); i ++)
        {
            if(childs1[i].size()==2)
                bif_swc.push_back(nt_swc.listNeuron[i]);
            if(childs1[i].size()==0)
                tip_swc.push_back(nt_swc.listNeuron[i]);
        }
        QList<ImageMarker> markers;
        if(bif_eswc.size()<bif_swc.size())
        {
            for(V3DLONG i = 0; i < bif_swc.size(); i ++)
            {
                for(V3DLONG j = 0; j <bif_eswc.size(); j ++)
                {
                    if(bif_swc[i]==2 && childs2[j].size()==1 && ((bif_swc[i].x-nt_eswc.listNeuron[j].x)*(bif_swc[i].x-nt_eswc.listNeuron[j].x)+(bif_swc[i].y-nt_eswc.listNeuron[j].y)*(bif_swc[i].y-nt_eswc.listNeuron[j].y)+(bif_swc[i].z-nt_eswc.listNeuron[j].z)*(bif_swc[i].z-nt_eswc.listNeuron[j].z))<25)
                    {
                        ImageMarker marker;
                        marker.x = bif_swc[i].x;
                        marker.y = bif_swc[i].y;
                        marker.z = bif_swc[i].z;
                        markers.push_back(marker);
                    }
                }
            }
        }
        QString path_marker="/home/balala/Desktop/markers/" + fileInfo_eswc.baseName().append("markers.marker");
        if(writeMarker_file(path_marker,markers));
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-18"));
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

