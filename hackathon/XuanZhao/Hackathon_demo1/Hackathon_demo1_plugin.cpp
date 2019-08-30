/* Hackathon_demo1_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-16 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include <QString>
#include <iostream>
#include <fstream>
#include <QFile>
#include "Hackathon_demo1_plugin.h"

#include <basic_surf_objs.h>
#include <v3d_interface.h>

#include "some_class.h"
#include "some_function.h"


using namespace std;
Q_EXPORT_PLUGIN2(Hackathon_demo1, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("getTif")
		<<tr("menu2")
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
    if (menu_name == tr("getTif"))
	{
        PARA_DEMO1 p;
        if(!p.DEMO1())
        {
            return;
        }

        ofstream out;
        out.open("D://shucai//blocks.txt",ios::out|ios::app);

        blockTree bt;
        //const int dx=512,dy=512,dz=128;
        NeuronTree nt;
        QString path0="D://shucai//swc_manual//";
        QString suffix1=".tif";
        QString suffix2=".marker";
        QString suffix3=".eswc";


        QString eswcfile;
        eswcfile=QFileDialog::getOpenFileName(parent,QString(QObject::tr("Choose the file")),".");
        nt=readSWC_file(eswcfile);

        for(int i=0;i<nt.listNeuron.size();++i)
        {
            nt.listNeuron[i].on=false;
        }

        QString dir0;
        dir0=QFileDialog::getExistingDirectory(parent);

        bt=getBlockTree(nt,p.dx,p.dy,p.dz);

        for(int i=0;i<bt.blocklist.size();++i)
        {


            const string dir=dir0.toStdString();
            unsigned char* imgblock;
            imgblock=callback.getSubVolumeTeraFly(dir,bt.blocklist[i].min_x,bt.blocklist[i].max_x,
                                                  bt.blocklist[i].min_y,bt.blocklist[i].max_y,
                                                  bt.blocklist[i].min_z,bt.blocklist[i].max_z);

            QString si0=path0+QString::number(bt.blocklist[i].n,10)+"_x_"+QString::number((qlonglong)bt.blocklist[i].o.x,10)+"_y_"+QString::number((qlonglong)bt.blocklist[i].o.y,10)+"_z_"+QString::number((qlonglong)bt.blocklist[i].o.z,10)+suffix1;
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
            marker.x=bt.blocklist[i].o.x-bt.blocklist[i].min_x;
            marker.y=bt.blocklist[i].o.y-bt.blocklist[i].min_y;
            marker.z=bt.blocklist[i].o.z-bt.blocklist[i].min_z;
            markers.push_back(marker);
            QString si1=path0+QString::number(bt.blocklist[i].n,10)+"_x_"+QString::number((qlonglong)bt.blocklist[i].o.x,10)+"_y_"+QString::number((qlonglong)bt.blocklist[i].o.y,10)+"_z_"+QString::number((qlonglong)bt.blocklist[i].o.z,10)+suffix2;
            const QString si2(si1);
            writeMarker_file(si2,markers);

            NeuronTree nttmp;

            for(int j=0;j<nt.listNeuron.size();++j)
            {
                if(nt.listNeuron[j].x>bt.blocklist[i].min_x
                        &&nt.listNeuron[j].x<=bt.blocklist[i].max_x
                        &&nt.listNeuron[j].y>bt.blocklist[i].min_y
                        &&nt.listNeuron[j].y<=bt.blocklist[i].max_y
                        &&nt.listNeuron[j].z>bt.blocklist[i].min_z
                        &&nt.listNeuron[j].z<=bt.blocklist[i].max_z)
                {
                    nt.listNeuron[j].x-=bt.blocklist[i].min_x;
                    nt.listNeuron[j].y-=bt.blocklist[i].min_y;
                    nt.listNeuron[j].z-=bt.blocklist[i].min_z;

                    nttmp.listNeuron.push_back(nt.listNeuron[j]);
                    std:cout<<nt.listNeuron[j].x<<" "<<nt.listNeuron[j].y<<" "<<nt.listNeuron[j].z<<endl;
                    nt.listNeuron[j].x+=bt.blocklist[i].min_x;
                    nt.listNeuron[j].y+=bt.blocklist[i].min_y;
                    nt.listNeuron[j].z+=bt.blocklist[i].min_z;
                }
            }

            QString si3=path0+QString::number(bt.blocklist[i].n,10)+"_x_"+QString::number((qlonglong)bt.blocklist[i].o.x,10)+"_y_"+QString::number((qlonglong)bt.blocklist[i].o.y,10)+"_z_"+QString::number((qlonglong)bt.blocklist[i].o.z,10)+suffix3;
            const QString si4(si3);

            writeESWC_file(si4,nttmp);

            out<<bt.blocklist[i].n<<" "<<bt.blocklist[i].min_x<<" "
              <<bt.blocklist[i].max_x<<" "<<bt.blocklist[i].min_y<<" "
             <<bt.blocklist[i].max_y<<" "<<bt.blocklist[i].min_z<<" "
            <<bt.blocklist[i].max_z<<" "<<bt.blocklist[i].parent<<endl;


        }

	}
	else if (menu_name == tr("menu2"))
	{

        ofstream ou;
        ou.open("D://shucai//cross.txt",ios::app|ios::out);

        ofstream ou0;
        ou0.open("D://shucai//weak_signal.txt",ios::app|ios::out);

        ou.clear();
        ou0.clear();

        /*QString filename="cross.txt";
        QFile file(filename);
        if(!file.open(QIODevice::ReadWrite|QIODevice::Append))
        {
            //QMessageBox::warning(this,"file write","can't open",QMessageBox::Yes);
            std::cout<<"cannot open"<<endl;
            exit(1);

        }*/

        QStringList files0,files1;
        files0=QFileDialog::getOpenFileNames(parent,QString(QObject::tr("Choose the manual file:")));
        files1=QFileDialog::getOpenFileNames(parent,QString(QObject::tr("Choose the auto file:")));
        int size_0=files0.size();
        int size_1=files1.size();
        for(int i=0;i<size_0;++i)
        {
            std::cout<<files0[i].toStdString()<<endl;
            QFileInfo eswcfile(files0[i]);
            NeuronTree nt0,nt1;
            nt0.listNeuron.clear();nt1.listNeuron.clear();
            nt0=readSWC_file(files0[i]);
            for(int j=0;j<size_1;++j)
            {
                QFileInfo swcfile(files1[j]);
                if(eswcfile.baseName()==swcfile.baseName())
                {
                    nt1=readSWC_file(files1[j]);
                }
            }
            if(nt1.listNeuron.empty())
            {
                std::cout<<"no file"<<endl;
                continue;
            }

            QVector<QVector<V3DLONG>> childs0;
            QVector<QVector<V3DLONG>> childs1;
            for(int i=0;i<childs0.size();++i)
            {
                childs0[i].clear();
            }
            childs0.clear();
            for(int i=0;i<childs1.size();++i)
            {
                childs1[i].clear();
            }
            childs1.clear();


            QVector<NeuronSWC> bifs0;
            QVector<NeuronSWC> bifs1;

            bifs0.clear();
            bifs1.clear();

            V3DLONG nt0Num=nt0.listNeuron.size();
            V3DLONG nt1Num=nt1.listNeuron.size();
            childs0=QVector<QVector<V3DLONG>>(nt0Num,QVector<V3DLONG>());
            childs1=QVector<QVector<V3DLONG>>(nt1Num,QVector<V3DLONG>());

            for(V3DLONG i=1;i<nt0Num;++i)
            {
                V3DLONG par=nt0.listNeuron[i].parent;
                if(par<0) continue;
                childs0[nt0.hashNeuron.value(par)].push_back(i);
            }
            for(V3DLONG i=1;i<nt1Num;++i)
            {
                V3DLONG par=nt1.listNeuron[i].parent;
                if(par<0) continue;
                childs1[nt1.hashNeuron.value(par)].push_back(i);
            }

            std::cout<<"000"<<endl;

            for(V3DLONG i=0;i<nt0Num;++i)
            {
                if(childs0[i].size()>=2)
                {
                    bifs0.push_back(nt0.listNeuron[i]);
                }
            }
            for(V3DLONG i=0;i<nt1Num;++i)
            {
                if(childs1[i].size()>=2)
                {
                    bifs1.push_back(nt1.listNeuron[i]);
                }
            }

            std::cout<<"111"<<endl;

            QVector<NeuronSWC> tmp;
            tmp.clear();
            for(int i=0;i<bifs1.size();++i)
            {
                bool flag=false;
                for(int j=0;j<bifs0.size();++j)
                {
                    if(bifs1[i].x>(bifs0[j].x-1)
                            &&bifs1[i].x<=(bifs0[j].x+1)
                            &&bifs1[i].y>(bifs0[j].y-1)
                            &&bifs1[i].y<=(bifs0[j].y+1)
                            &&bifs1[i].z>(bifs0[j].z-1)
                            &&bifs1[i].z<=(bifs0[j].z+1))
                    {
                        flag=true;
                    }
                }
                if(flag==false)
                {
                    tmp.push_back(bifs1[i]);
                }
                flag=false;
            }

            std::cout<<"222"<<endl;

            QVector<NeuronSWC> result;
            result.clear();

            for(int i=0;i<tmp.size();++i)
            {
                for(int j=0;j<nt0.listNeuron.size();++j)
                {
                    if(tmp[i].x>(nt0.listNeuron[j].x-10)
                            &&tmp[i].x<=(nt0.listNeuron[j].x+10)
                            &&tmp[i].y>(nt0.listNeuron[j].y-10)
                            &&tmp[i].y<=(nt0.listNeuron[j].y+10)
                            &&tmp[i].z>(nt0.listNeuron[j].z-10)
                            &&tmp[i].z<=(nt0.listNeuron[j].z+10))
                    {
                        result.push_back(tmp[i]);
                    }
                }
            }

            std::cout<<"333"<<endl;



            /*if(result.size()>0){
                QTextStream out(&file);
                out<<eswcfile.fileName()<<" ";
                for(int i=0;i<result.size();++i)
                {
                    out<<result[i].x<<" "<<result[i].y<<" "<<result[i].z<<" ";
                }

                out<<endl;

            }*/

            std::cout<<"444"<<endl;

            ou<<eswcfile.fileName().toStdString()<<" ";
            for(int i=0;i<result.size();++i)
            {
                ou<<result[i].x<<" "<<result[i].y<<" "<<result[i].z<<" ";
            }
            ou<<endl;

            ImageMarker marker;//,tmp;
            QList<ImageMarker> markers;
            for(int i=0;i<result.size();++i)
            {
                marker.x=result[i].x;
                marker.y=result[i].y;
                marker.z=result[i].z;
                markers.push_back(marker);
            }
            //tmp=markers[0];
            /*marker=markers[i];
            for(int i=1;i<markers.size();++i)
            {
                if(marker==markers[i])
                {
                    markers
                }
            }*/

            QString file0=eswcfile.absoluteDir().path()+eswcfile.baseName()+QString(".marker");
            QString file1(file0);
            writeMarker_file(file1,markers);

            const int dx=5,dy=5,dz=5;

            bool flag=false;
            int count0=0;
            int count1=0;

            for(int j=0;j<nt0.listNeuron.size();++j)
            {
                for(int k=0;k<nt1.listNeuron.size();++k)
                {
                    if(nt0.listNeuron[j].x>(nt1.listNeuron[k].x-dx)&&nt0.listNeuron[j].x<(nt1.listNeuron[k].x+dx)
                            &&nt0.listNeuron[j].y>(nt1.listNeuron[k].y-dy)&&nt0.listNeuron[j].y<(nt1.listNeuron[k].y+dy)
                            &&nt0.listNeuron[j].z>(nt1.listNeuron[k].z-dz)&&nt0.listNeuron[j].z<(nt1.listNeuron[k].z+dz))
                    {
                        flag=true;
                    }

                }
                if(flag==true)
                {
                    count0++;
                }
                flag=false;


            }
            flag=false;
            for(int j=0;j<nt1.listNeuron.size();++j)
            {
                for(int k=0;k<nt0.listNeuron.size();++k)
                {
                    if(nt1.listNeuron[j].x>(nt0.listNeuron[k].x-dx)&&nt1.listNeuron[j].x<(nt0.listNeuron[k].x+dx)
                            &&nt1.listNeuron[j].y>(nt0.listNeuron[k].y-dy)&&nt1.listNeuron[j].y<(nt0.listNeuron[k].y+dy)
                            &&nt1.listNeuron[j].z>(nt0.listNeuron[k].z-dz)&&nt1.listNeuron[j].z<(nt0.listNeuron[k].z+dz))
                    {
                        flag=true;
                    }
                }

                if(flag==true)
                {
                    count1++;
                }
                flag=false;

            }
            double c0=(double)count0/nt0.listNeuron.size();
            double c1=(double)count1/nt1.listNeuron.size();
            bool isweak=false;
            if(c0<0.7)
            {
                isweak=true;
            }else
            {
                isweak=false;
            }

            ou0<<eswcfile.baseName().toStdString()<<" "<<count0<<" "<<nt0.listNeuron.size()
              <<" "<<c0<<" "<<count1<<" "<<nt1.listNeuron.size()<<" "<<c1<<" "<<isweak<<endl;





        }
        ou.close();
        ou0.close();
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-16"));
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

