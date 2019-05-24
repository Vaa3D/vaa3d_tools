/* Hackathon_demo2_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-20 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <queue>
#include "Hackathon_demo2_plugin.h"

#include "some_class.h"
#include "some_function.h"
#include "resampling.h"


using namespace std;
Q_EXPORT_PLUGIN2(Hackathon_demo2, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("get tips and eswc")
        <<tr("trans")
        <<tr("gui")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("get tips and eswc")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("get tips and eswc"))
	{
        PARA_DEMO2 p;
        if(!p.demo2_dialog())
        {
            return;
        }

        ofstream out;
        out.open("D://xinsucai//block.txt",ios::out|ios::app);

        blockTree bt;
        NeuronTree nt,nt0;

        QString suffix1=".tif";
        QString suffix2=".marker";
        QString suffix3=".swc";
        QString suffix4=".eswc";

        QStringList eswcfiles;
        eswcfiles=QFileDialog::getOpenFileNames(parent,QString(QObject::tr("Choose the eswcfile or swcfile")),".");

        QString dir0;
        dir0=QFileDialog::getExistingDirectory();

        QString path=QFileDialog::getExistingDirectory(parent,QString(QObject::tr("Choose the dir of saving")));
        QString path0=path+"//";

        for(int i=0;i<eswcfiles.size();++i)
        {
            nt0.listNeuron.clear();
            nt.listNeuron.clear();
            nt0=readSWC_file(eswcfiles[i]);
            double step=2.0;
            nt=resample(nt0,step);

            QFileInfo eswcfile(eswcfiles[i]);

            bt.blocklist.clear();
            bt=getBlockTree(nt,p.dx,p.dy,p.dz);

            for(int j=0;j<bt.blocklist.size();++j)
            {
                const string dir=dir0.toStdString();
                unsigned char* imgblock;
                imgblock=callback.getSubVolumeTeraFly(dir,bt.blocklist[j].min_x,bt.blocklist[j].max_x,
                                                      bt.blocklist[j].min_y,bt.blocklist[j].max_y,
                                                      bt.blocklist[j].min_z,bt.blocklist[j].max_z);
                QString si0=path0+eswcfile.baseName()+"_"+QString::number(bt.blocklist[j].n,10)+"_x_"+QString::number((qlonglong)bt.blocklist[j].o.x,10)+"_y_"+QString::number((qlonglong)bt.blocklist[j].o.y,10)+"_z_"+QString::number((qlonglong)bt.blocklist[j].o.z,10)+suffix1;
                const char* si=si0.toStdString().c_str();

                V3DLONG sz0[4]={p.dx,p.dy,p.dz,1};
                int datatype=1;
                if(!simple_saveimage_wrapper(callback,si,imgblock,sz0,datatype))
                {
                    std::cout<<"000"<<endl;
                }

                delete imgblock;

                ImageMarker marker;
                QList<ImageMarker> markers;
                markers.clear();
                marker.x=bt.blocklist[j].o.x-bt.blocklist[j].min_x;
                marker.y=bt.blocklist[j].o.y-bt.blocklist[j].min_y;
                marker.z=bt.blocklist[j].o.z-bt.blocklist[j].min_z;
                markers.push_back(marker);
                QString si1=path0+eswcfile.baseName()+"_"+QString::number(bt.blocklist[j].n,10)+"_x_"+QString::number((qlonglong)bt.blocklist[j].o.x,10)+"_y_"+QString::number((qlonglong)bt.blocklist[j].o.y,10)+"_z_"+QString::number((qlonglong)bt.blocklist[j].o.z,10)+suffix2;
                const QString si2(si1);
                writeMarker_file(si2,markers);

                NeuronTree nttmp;

                for(int k=0;k<nt.listNeuron.size();++k)
                {
                    if(nt.listNeuron[k].x>bt.blocklist[j].min_x
                            &&nt.listNeuron[k].x<=bt.blocklist[j].max_x
                            &&nt.listNeuron[k].y>bt.blocklist[j].min_y
                            &&nt.listNeuron[k].y<=bt.blocklist[j].max_y
                            &&nt.listNeuron[k].z>bt.blocklist[j].min_z
                            &&nt.listNeuron[k].z<=bt.blocklist[j].max_z)
                    {
                        nt.listNeuron[k].x-=bt.blocklist[j].min_x;
                        nt.listNeuron[k].y-=bt.blocklist[j].min_y;
                        nt.listNeuron[k].z-=bt.blocklist[j].min_z;

                        nttmp.listNeuron.push_back(nt.listNeuron[k]);

                        nt.listNeuron[k].x+=bt.blocklist[j].min_x;
                        nt.listNeuron[k].y+=bt.blocklist[j].min_y;
                        nt.listNeuron[k].z+=bt.blocklist[j].min_z;
                    }
                }
                /*
                QVector<QVector<V3DLONG>> children=getChildren(nt);
                V3DLONG index=0;
                for(V3DLONG j=0;j<nt.listNeuron.size();++j)
                {
                    if(nttmp0.listNeuron[j].x==marker[0].x
                            &&nttmp0.listNeuron[j].y==marker[0].y
                            &&nttmp0.listNeuron[j].z==marker[0].z)
                    {
                        index=j;
                    }
                }

                queue<NeuronSWC> qswcs;
                NeuronSWC o=nt.listNeuron[index];
                NeuronSWC ptmp;
                qswcs.push(o);
                while(!qswcs.empty())
                {
                    ptmp=qswcs.front();
                    qswcs.pop();
                    if(children[nttmp0.hashNeuron.value(ptmp.n)].size()>0)
                    {
                        for(int i=0;i<children[nt.hashNeuron.value(ptmp.n)].size();++i)
                        {
                            qswcs.push(nt.listNeuron[children[nt.hashNeuron.value(ptmp.n)][i]]);
                        }
                    }
                    nttmp.listNeuron.push_back(ptmp);
                    //std::cout<<ptmp.n<<endl;
                }
                */


                QString si3=path0+eswcfile.baseName()+"_"+QString::number(bt.blocklist[j].n,10)+"_x_"+QString::number((qlonglong)bt.blocklist[j].o.x,10)+"_y_"+QString::number((qlonglong)bt.blocklist[j].o.y,10)+"_z_"+QString::number((qlonglong)bt.blocklist[j].o.z,10)+suffix4;
                const QString si4(si3);

                writeESWC_file(si4,nttmp);
                //writeSWC_file(si4,nttmp);

                out<<eswcfile.baseName().toStdString()<<" "
                   <<bt.blocklist[j].n<<" "<<bt.blocklist[j].min_x<<" "
                  <<bt.blocklist[j].max_x<<" "<<bt.blocklist[j].min_y<<" "
                 <<bt.blocklist[j].max_y<<" "<<bt.blocklist[j].min_z<<" "
                <<bt.blocklist[j].max_z<<" "<<bt.blocklist[j].parent<<endl;

            }

        }
	}
    else if (menu_name == tr("trans"))
	{
        NeuronTree nt;
        QList<ImageMarker> marker;
        QStringList eswcfiles;
        eswcfiles=QFileDialog::getOpenFileNames(parent,QString(QObject::tr("Choose the manual file:")));
        int size=eswcfiles.size();
        for(int i=0;i<size;++i)
        {
            NeuronTree nttmp;
            nttmp.listNeuron.clear();
            nt.listNeuron.clear();
            nt=readSWC_file(eswcfiles[i]);
            QFileInfo eswcfile(eswcfiles[i]);
            QString markerpath=eswcfile.dir().absolutePath()+'/'+eswcfile.baseName()+".marker";
            marker=readMarker_file(markerpath);

            QVector<QVector<V3DLONG>> children=getChildren(nt);

            for(int i=0;i<nt.listNeuron.size();++i)
            {
                for(int j=0;j<children[i].size();++j)
                {
                    std::cout<<children[i][j]<<"    ";
                }
                std::cout<<endl;
            }


            V3DLONG index=0;
            for(V3DLONG j=0;j<nt.listNeuron.size();++j)
            {
                if(nt.listNeuron[j].x==marker[0].x
                        &&nt.listNeuron[j].y==marker[0].y
                        &&nt.listNeuron[j].z==marker[0].z)
                {
                    index=j;
                }
            }
            std::cout<<index<<endl;

            queue<NeuronSWC> qswcs;
            NeuronSWC o=nt.listNeuron[index];
            NeuronSWC ptmp;
            qswcs.push(o);
            while(!qswcs.empty())
            {
                ptmp=qswcs.front();
                qswcs.pop();
                if(children[nt.hashNeuron.value(ptmp.n)].size()>0)
                {
                    for(int i=0;i<children[nt.hashNeuron.value(ptmp.n)].size();++i)
                    {
                        qswcs.push(nt.listNeuron[children[nt.hashNeuron.value(ptmp.n)][i]]);
                    }
                }
                nttmp.listNeuron.push_back(ptmp);
                std::cout<<ptmp.n<<endl;
            }
            QString savepath=eswcfile.dir().absolutePath()+'/'+eswcfile.baseName()+"_single"+".eswc";
            writeESWC_file(savepath,nttmp);
        }
	}
    else if(menu_name == tr("gui"))
    {
        PARA_DEMO2 p;

        p.parent=parent;

        if(!p.demo2_dialog())
        {
            return;
        }
        for(int i=0;i<p.eswcfiles.size();++i)
        {
            cout<<p.eswcfiles[i].toStdString()<<endl;
        }

        /*if(!work(p,callback))
        {
            return;
        }*/
        work(p,callback);

    }
    else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-20"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("get tips and eswc"))
    {/*
        QDir eswcdir=(infiles.empty())?QString(""):QString(infiles[0]);
        QString brain=(infiles.size()==2)?infiles[1]:"";
        if(eswcdir.path()=="")
        {
            std::cout<<"no files"<<endl;
            return 1;
        }
        if(brain=="")
        {
            std::cout<<"no brain"<<endl;
            return 1;
        }
        QStringList filters;
        filters<<"*.eswc"<<"*.swc";
        eswcdir.setNameFilters(filters);
        const QList<QString> eswcfiles=eswcdir.entryList();

        ofstream out;
        out.open("D://xinsucai//block.txt",ios::out|ios::app);

        blockTree bt;
        NeuronTree nt,nt0;

        QString suffix1=".tif";
        QString suffix2=".marker";
        QString suffix3=".swc";
        QString suffix4=".eswc";

        PARA_DEMO2 p;

        for(int i=0;i<eswcfiles.size();++i)
        {
            nt0.listNeuron.clear();
            nt.listNeuron.clear();
            nt0=readSWC_file(eswcfiles[i]);
            double step=2.0;
            nt=resample(nt0,step);

            QFileInfo eswcfile(eswcfiles[i]);

            bt.blocklist.clear();
            bt=getBlockTree(nt,p.dx,p.dy,p.dz);

            for(int j=0;j<bt.blocklist.size();++j)
            {
                const string dir=p.brainpath.toStdString();
                unsigned char* imgblock;
                imgblock=callback.getSubVolumeTeraFly(dir,bt.blocklist[j].min_x,bt.blocklist[j].max_x,
                                                      bt.blocklist[j].min_y,bt.blocklist[j].max_y,
                                                      bt.blocklist[j].min_z,bt.blocklist[j].max_z);
                QString si0=path0+eswcfile.baseName()+"_"+QString::number(bt.blocklist[j].n,10)+"_x_"+QString::number((qlonglong)bt.blocklist[j].o.x,10)+"_y_"+QString::number((qlonglong)bt.blocklist[j].o.y,10)+"_z_"+QString::number((qlonglong)bt.blocklist[j].o.z,10)+suffix1;
                const char* si=si0.toStdString().c_str();

                V3DLONG sz0[4]={p.dx,p.dy,p.dz,1};
                int datatype=1;
                if(!simple_saveimage_wrapper(callback,si,imgblock,sz0,datatype))
                {
                    std::cout<<"000"<<endl;
                }

                delete imgblock;

                ImageMarker marker;
                QList<ImageMarker> markers;
                markers.clear();
                marker.x=bt.blocklist[j].o.x-bt.blocklist[j].min_x;
                marker.y=bt.blocklist[j].o.y-bt.blocklist[j].min_y;
                marker.z=bt.blocklist[j].o.z-bt.blocklist[j].min_z;
                markers.push_back(marker);
                QString si1=path0+eswcfile.baseName()+"_"+QString::number(bt.blocklist[j].n,10)+"_x_"+QString::number((qlonglong)bt.blocklist[j].o.x,10)+"_y_"+QString::number((qlonglong)bt.blocklist[j].o.y,10)+"_z_"+QString::number((qlonglong)bt.blocklist[j].o.z,10)+suffix2;
                const QString si2(si1);
                writeMarker_file(si2,markers);

                NeuronTree nttmp;

                for(int k=0;k<nt.listNeuron.size();++k)
                {
                    if(nt.listNeuron[k].x>bt.blocklist[j].min_x
                            &&nt.listNeuron[k].x<=bt.blocklist[j].max_x
                            &&nt.listNeuron[k].y>bt.blocklist[j].min_y
                            &&nt.listNeuron[k].y<=bt.blocklist[j].max_y
                            &&nt.listNeuron[k].z>bt.blocklist[j].min_z
                            &&nt.listNeuron[k].z<=bt.blocklist[j].max_z)
                    {
                        nt.listNeuron[k].x-=bt.blocklist[j].min_x;
                        nt.listNeuron[k].y-=bt.blocklist[j].min_y;
                        nt.listNeuron[k].z-=bt.blocklist[j].min_z;

                        nttmp.listNeuron.push_back(nt.listNeuron[k]);

                        nt.listNeuron[k].x+=bt.blocklist[j].min_x;
                        nt.listNeuron[k].y+=bt.blocklist[j].min_y;
                        nt.listNeuron[k].z+=bt.blocklist[j].min_z;
                    }
                }



                QString si3=path0+eswcfile.baseName()+"_"+QString::number(bt.blocklist[j].n,10)+"_x_"+QString::number((qlonglong)bt.blocklist[j].o.x,10)+"_y_"+QString::number((qlonglong)bt.blocklist[j].o.y,10)+"_z_"+QString::number((qlonglong)bt.blocklist[j].o.z,10)+suffix4;
                const QString si4(si3);

                writeESWC_file(si4,nttmp);
                //writeSWC_file(si4,nttmp);

                out<<eswcfile.baseName().toStdString()<<" "
                   <<bt.blocklist[j].n<<" "<<bt.blocklist[j].min_x<<" "
                  <<bt.blocklist[j].max_x<<" "<<bt.blocklist[j].min_y<<" "
                 <<bt.blocklist[j].max_y<<" "<<bt.blocklist[j].min_z<<" "
                <<bt.blocklist[j].max_z<<" "<<bt.blocklist[j].parent<<endl;

            }

        }



    }


*/




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

