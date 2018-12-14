/* retype_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-03-13 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "retype_swc_plugin.h"

#include "basic_surf_objs.h"
#include <iostream>
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"

#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
#define NTDOT(a,b) ((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)
#define angle(a,b,c) (acos((((b).x-(a).x)*((c).x-(a).x)+((b).y-(a).y)*((c).y-(a).y)+((b).z-(a).z)*((c).z-(a).z))/(NTDIS(a,b)*NTDIS(a,c)))*180.0/3.14159265359)

using namespace std;
Q_EXPORT_PLUGIN2(retype_swc, retype_swc);
bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    QFile qf(fileOpenName);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(fileOpenName));
#endif
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

    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
};

template <class T> T pow2(T a)
{
    return a*a;

}
 
QStringList retype_swc::menulist() const
{
	return QStringList() 
		<<tr("retype")
		<<tr("about");
}

QStringList retype_swc::funclist() const
{
	return QStringList()
		<<tr("help");
}

void retype_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("retype"))
    {
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(fileOpenName.isEmpty())
            return;
        int type;
        NeuronTree nt;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
             nt = readSWC_file(fileOpenName);

             QVector<QVector<V3DLONG> > childs;
             V3DLONG neuronNum = nt.listNeuron.size();
             childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );
             for (V3DLONG i=0;i<neuronNum;i++)
             {
                 V3DLONG par = nt.listNeuron[i].pn;
                 if (par<0) continue;
                 childs[nt.hashNeuron.value(par)].push_back(i);
             }

             //assign all sub_trees
             QVector<int> visit(nt.listNeuron.size(),0);
             for(int i=1; i<nt.listNeuron.size();i++)
             {
                 if(NTDIS(nt.listNeuron.at(i),nt.listNeuron.at(0))<=40)
                     visit[i]=1;

             }

             for(int i=0; i<nt.listNeuron.size();i++)
             {
                 NeuronSWC current = nt.listNeuron[i];
                 bool boundary=false;
                 if(current.x<=3 || current.y<=3 || current.z<=3 || current.x>=128-4 ||
                         current.y>=128-4 || current.z>=128-4)
                     boundary=true;

                 if((current.radius>=8 || boundary)&& visit[i]==0)
                 {
                     QQueue<int> q;
                     visit[i]=1;
                     q.push_back(i);
                     while(!q.empty())
                     {
                         int current = q.front(); q.pop_front();
                         nt.listNeuron[current].type = 255;
                         for(int j=0; j<childs[current].size();j++)
                         {
                             if(visit[childs[current].at(j)]==0)
                             {
                                 visit[childs[current].at(j)]=1;
                                 q.push_back(childs[current].at(j));
                             }
                         }
                     }
                     q.clear();
                 }
             }

            QString fileDefaultName = fileOpenName+QString("_retyped.swc");
            //write new SWC to file
            QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                    fileDefaultName,
                    QObject::tr("Supported file (*.swc)"
                        ";;Neuron structure	(*.swc)"
                        ));
            if (!export_list2file(nt.listNeuron,fileSaveName,fileOpenName))
            {
                v3d_msg("fail to write the output swc file.");
                return;
            }

        }
	}
	else
	{
        v3d_msg(tr("This is a plugin to retype the swc file (the noot node type to be 1, and the rest to be defined by the user). "
			"Developed by Zhi Zhou, 2014-03-13"));
    }
}

bool retype_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, paras, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) paras = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("retype"))
    {
        cout<<"Welcome to swc retyping plugin"<<endl;
        if(infiles.empty())
        {
            cerr<<"Need input swc file"<<endl;
            return false;
        }

        QString  inswc_file =  infiles[0];
        int k=0;
        int type = (paras.size() >= k+1) ? atoi(paras[k]) : 3;  k++;//0;

        QString  outswc_file;
        if(!outfiles.empty())
            outswc_file = outfiles[0];
        else
            outswc_file = inswc_file + "_retyped.swc";

        cout<<"inswc_file = "<<inswc_file.toStdString().c_str()<<endl;
        cout<<"node type = "<<type<<endl;
        cout<<"outswc_file = "<<outswc_file.toStdString().c_str()<<endl;

        NeuronTree nt;
        nt = readSWC_file(inswc_file);
        for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
        {
            if(nt.listNeuron[i].parent == -1)
                nt.listNeuron[i].type = 1;
            else
                nt.listNeuron[i].type = type;
        }
        if (!export_list2file(nt.listNeuron,outswc_file,inswc_file))
        {
            v3d_msg("fail to write the output swc file.");
            return false;
        }
    }else if (func_name == tr("retype_reference"))
    {
        if(infiles.size()<2)
        {
            cerr<<"Need two input swc files"<<endl;
            return false;
        }

        if(outfiles.empty())
        {
            cerr<<"Need output file name"<<endl;
            return false;
        }

        NeuronTree nt_original = readSWC_file(QString(infiles[0]));
        NeuronTree nt_refined = readSWC_file(QString(infiles[1]));

        for(V3DLONG i=0; i<nt_refined.listNeuron.size();i++)
        {
            if(nt_refined.listNeuron.at(i).type == 6)
            {
                for(V3DLONG j=0; j<nt_original.listNeuron.size();j++)
                {
                    if(NTDIS(nt_refined.listNeuron[i], nt_original.listNeuron[j])<1)
                    {
                        nt_refined.listNeuron[i].type = nt_original.listNeuron[j].type;
                        break;
                    }
                }

            }
        }

        writeESWC_file(QString(outfiles[0]),nt_refined);
    }
    else if (func_name == tr("help"))
    {
        cout<<"Usage : v3d -x dllname -f retype -i <inswc_file> -o <outswc_file> -p <type>"<<endl;
        cout<<endl;
    }
	else return false;

	return true;
}

