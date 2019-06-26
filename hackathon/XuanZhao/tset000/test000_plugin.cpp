/* test000_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-8 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include "test000_plugin.h"
#include "some_function.h"
#include  "neuron_sim_scores.h"

#include <v3d_interface.h>
#include <basic_surf_objs.h>

using namespace std;
Q_EXPORT_PLUGIN2(test000, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("half_coordinate")
        <<tr("cut_block")
        <<tr("cut_eswc")
        <<tr("compare_eswc")
        <<tr("trans_eswc");
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
    if (menu_name == tr("half_coordinate"))
	{

        //将eswc文件的坐标减半
        NeuronTree nt;
        //const QString eswcfile="D://shucai//18454_051_ZSJ_SYY_YLL_10282018.ano.eswc";
        QString eswcfile;
        eswcfile=QFileDialog::getOpenFileName(parent,QString(QObject::tr("Choose the file")),".","*.eswc");
        nt=readSWC_file(eswcfile);
        int size=nt.listNeuron.size();
        for(int i=0;i<size;++i)
        {
            nt.listNeuron[i].x/=2;
            nt.listNeuron[i].y/=2;
            nt.listNeuron[i].z/=2;
        }
        const NeuronTree tmpt(nt);

        const QString eswcfile0="D://shucai//half.eswc";
        writeESWC_file1(eswcfile0,tmpt);




	}
    else if (menu_name == tr("cut_block"))
	{
        //读分块信息

        vector<block> blockvector;
        //const char* scanData_file="D://shucai//051_x_6788.89_y_6160.88_z_2670.09.marker_tmp_APP2//scanData.txt";
        QString scanData_file0=QFileDialog::getOpenFileName(parent,QString(QObject::tr("Please choose the scanData_file")));
        string scanData_file1=scanData_file0.toStdString();
        const char* scanData_file=scanData_file1.c_str();

        ifstream in;
        in.open(scanData_file,ios::in);
        if(!in.is_open())
        {
            cout<<"Error opening file"<<endl;
            exit(1);
        }
        while(!in.eof())
        {
            block tmp;
            in>>tmp.a.x;
            in>>tmp.a.y;
            in>>tmp.a.z;
            in>>tmp.path;
            in>>tmp.dx;
            in>>tmp.dy;
            in>>tmp.dz;
            blockvector.push_back(tmp);
        }
        //切eswc文件
        NeuronTree nt;
        //const QString eswcfile="D://shucai//half.eswc";
        QString eswcfile;
        eswcfile=QFileDialog::getOpenFileName(parent,QString(QObject::tr("Choose the file")),".","*.eswc");
        nt=readESWC_file(eswcfile);
        int size=nt.listNeuron.size();
        int bs=blockvector.size();
        QString path="D://shucai//18457//";
        QString suffix=".eswc";
        NeuronTree nttmp;

        for(int i=0;i<bs;++i)
        {
            nttmp.listNeuron.clear();
            for(int j=0;j<size;++j)
            {
                if(nt.listNeuron[j].x>blockvector[i].a.x&&nt.listNeuron[j].x<=(blockvector[i].a.x+blockvector[i].dx)
                        &&nt.listNeuron[j].y>blockvector[i].a.y&&nt.listNeuron[j].y<=(blockvector[i].a.y+blockvector[i].dy)
                        &&nt.listNeuron[j].z>blockvector[i].a.z&&nt.listNeuron[j].z<=(blockvector[i].a.z+blockvector[i].dz))
                {
                    nt.listNeuron[j].x-=blockvector[i].a.x;
                    nt.listNeuron[j].y-=blockvector[i].a.y;
                    nt.listNeuron[j].z-=blockvector[i].a.z;
                    nttmp.listNeuron.push_back(nt.listNeuron[j]);
                    nt.listNeuron[j].x+=blockvector[i].a.x;
                    nt.listNeuron[j].y+=blockvector[i].a.y;
                    nt.listNeuron[j].z+=blockvector[i].a.z;
                }

            }
            const QString s=path+"x_"+QString::number((int)blockvector[i].a.x,10)+"_y_"+QString::number((int)blockvector[i].a.y,10)+"_z_"+
                    QString::number((int)blockvector[i].a.z,10)+suffix;
             writeESWC_file1(s,nttmp);
        }

	}
    else if(menu_name == tr("cut_eswc"))
	{
        //将同一block下的eswc文件分割出来

        QStringList eswcfilelist;
        eswcfilelist=QFileDialog::getOpenFileNames(parent,QString(QObject::tr("Choose the file:")));
        int size=eswcfilelist.size();
        cout<<size<<endl;
        for(int i=0;i<size;++i)
        {
            NeuronTree nt;
            NeuronTree nttmp;
            QString suffix=".eswc";
            QString path0=path(eswcfilelist[i]);
            int m=1;
            nt=readESWC_file(eswcfilelist[i]);
            int size0=nt.listNeuron.size();

            vector<int> iOfEswc;

            vector<V3DLONG> index;
            for(int i=0;i<size0;++i)
            {
                index.push_back(nt.listNeuron[i].n);
            }
            //index.push_back(nt.listNeuron[0].n);
            //nttmp.listNeuron.push_back(nt.listNeuron[0]);
            for(int i=0;i<size0;++i)
            {
                if(!isin(nt.listNeuron[i].parent,index))
                {
                    iOfEswc.push_back(i);
                }
            }
            for(int i=iOfEswc.size()-1;i>0;--i)
            {
                nttmp.listNeuron.clear();
                index.clear();
                index.push_back(nt.listNeuron[iOfEswc[i]].n);
                nttmp.listNeuron.push_back(nt.listNeuron[iOfEswc[i]]);
                int j=iOfEswc[i];
                QList<NeuronSWC>::iterator it=nt.listNeuron.begin()+j;
                nt.listNeuron.erase(it);
                for(;isin(nt.listNeuron[j].parent,index);)
                {
                    nttmp.listNeuron.push_back(nt.listNeuron[j]);
                    index.push_back(nt.listNeuron[j].n);
                    QList<NeuronSWC>::iterator it=nt.listNeuron.begin()+j;
                    nt.listNeuron.erase(it);

                }
                const QString s=path0+"_"+QString::number(m,10)+suffix;
                writeESWC_file1(s,nttmp);
                m++;

            }
            const QString s=path0+"_"+QString::number(m,10)+suffix;
            writeESWC_file1(s,nt);
        }
    }else if(menu_name == tr("trans_eswc"))
    {
        NeuronTree nt0;
        QList<ImageMarker> marker;
        QStringList file0,file1;
        file0=QFileDialog::getOpenFileNames(parent,QString(QObject::tr("Choose the manual file:")));
        file1=QFileDialog::getOpenFileNames(parent,QString(QObject::tr("Choose the marker file:")));
        int size_0=file0.size();
        int size_1=file1.size();
        for(int i=0;i<size_0;++i)
        {
            NeuronTree nttmp;
            vector<V3DLONG> index;
            nttmp.listNeuron.clear();
            index.clear();
            nt0=readSWC_file(file0[i]);
            for(int j=0;j<size_1;++j)
            {
                if(path(file0[i])==path(file1[j]))
                {
                    marker=readMarker_file(file1[j]);
                }
            }

            int index0;
            for(int k=0;k<nt0.listNeuron.size();++k)
            {
                if(nt0.listNeuron[k].x==marker[0].x
                        &&nt0.listNeuron[k].y==marker[0].y
                        &&nt0.listNeuron[k].z==marker[0].z)
                {
                    index0=k;
                }
            }
            index.push_back(nt0.listNeuron[index0].n);
            nttmp.listNeuron.push_back(nt0.listNeuron[index0]);
            for(int i=index0;i<nt0.listNeuron.size();++i)
            {
                if(isin(nt0.listNeuron[i].parent,index))
                {
                    index.push_back(nt0.listNeuron[i].n);
                    nttmp.listNeuron.push_back(nt0.listNeuron[i]);
                }
            }
            writeESWC_file(file0[i],nttmp);


        }
    }
    else
    {
        /*NeuronTree nt0,nt1;
        QString file0,file1;
        file0=QFileDialog::getOpenFileName(parent,QString(QObject::tr("Choose the manual file:")));
        file1=QFileDialog::getOpenFileName(parent,QString(QObject::tr("Choose the auto file:")));
        nt0=readESWC_file(file0);
        nt1=readESWC_file(file1);

        const int dx=1,dy=1,dz=1;
        int size1=nt1.listNeuron.size();
        const int size0=nt0.listNeuron.size();
        vector<bool> istrue(size0,false);
        for(int i=0;i<size0;++i)
        {
            for(int j=0;j<size1;++j)
            {
                if(nt1.listNeuron[j].x>(nt0.listNeuron[i].x-dx)&&nt1.listNeuron[j].x<(nt0.listNeuron[i].x+dx)
                        &&nt1.listNeuron[j].y>(nt0.listNeuron[i].y-dy)&&nt1.listNeuron[j].y<(nt0.listNeuron[i].y+dy)
                        &&nt1.listNeuron[j].z>(nt0.listNeuron[i].z-dz)&&nt1.listNeuron[j].z<(nt0.listNeuron[i].z+dz))
                {
                    istrue[i]=true;
                    break;
                }
            }

        }

        int count=0;
        int c=istrue.size();
        for(int i=0;i<c;++i)
        {
            if(istrue[i]==true)
                count++;
        }
        cout<<count<<endl<<c<<endl<<size0<<endl<<(double)count/c<<endl;*/
        const double nm=0.9;
        ofstream out;
        out.open("D://shucai//result.txt",ios::out|ios::app);
        NeuronTree nt0,nt1;
        QStringList file0,file1;
        file0=QFileDialog::getOpenFileNames(parent,QString(QObject::tr("Choose the manual file:")));
        file1=QFileDialog::getOpenFileNames(parent,QString(QObject::tr("Choose the auto file:")));
        int size_0=file0.size();
        int size_1=file1.size();
        for(int i=0;i<size_0;++i)
        {
            nt0=readESWC_file(file0[i]);
            for(int j=0;j<size_1;++j)
            {
                if(path(file0[i])==path(file1[j]))
                {
                    nt1=readESWC_file(file1[j]);
                }
            }
            if(nt1.listNeuron.empty())
            {
                out<<file0[i].toStdString()<<string(" has no autotracing...")<<endl;
                continue;
            }


            const int dx=1,dy=1,dz=1;
            int size1=nt1.listNeuron.size();
            const int size0=nt0.listNeuron.size();
            vector<bool> istrue(size0,false);
            for(int i=0;i<size0;++i)
            {
                for(int j=0;j<size1;++j)
                {
                    if(nt1.listNeuron[j].x>(nt0.listNeuron[i].x-dx)&&nt1.listNeuron[j].x<(nt0.listNeuron[i].x+dx)
                            &&nt1.listNeuron[j].y>(nt0.listNeuron[i].y-dy)&&nt1.listNeuron[j].y<(nt0.listNeuron[i].y+dy)
                            &&nt1.listNeuron[j].z>(nt0.listNeuron[i].z-dz)&&nt1.listNeuron[j].z<(nt0.listNeuron[i].z+dz))
                    {
                        istrue[i]=true;
                        break;
                    }
                }

            }

            int count=0;
            bool b;
            int c=istrue.size();
            for(int i=0;i<c;++i)
            {
                if(istrue[i]==true)
                    count++;
            }
            double rate=(double)count/(double)c;
            cout<<count<<endl<<c<<endl<<size0<<endl<<(double)count/c<<endl;
            string p0=file0[i].toStdString();
            if(rate<nm)
            {
                b=false;
            }else
            {
                b=true;
            }
            out<<p0<<string(" ")<<count<<string(" ")<<size0<<string(" ")<<rate<<string(" ")<<b<<endl;

        }
        for(int i=0;i<size_1;++i)
        {
            nt1=readESWC_file(file1[i]);
            for(int j=0;j<size_0;++j)
            {
                if(path(file1[i])==path(file0[j]))
                {
                    nt0=readESWC_file(file0[j]);
                }
            }
            if(nt0.listNeuron.empty())
            {
                out<<file1[i].toStdString()<<string(" has no manual tracing...")<<endl;
                continue;
            }


            const int dx=1,dy=1,dz=1;
            int size1=nt1.listNeuron.size();
            const int size0=nt0.listNeuron.size();
            vector<bool> istrue(size1,false);
            for(int i=0;i<size1;++i)
            {
                for(int j=0;j<size0;++j)
                {
                    if(nt0.listNeuron[j].x>(nt1.listNeuron[i].x-dx)&&nt0.listNeuron[j].x<(nt1.listNeuron[i].x+dx)
                            &&nt0.listNeuron[j].y>(nt1.listNeuron[i].y-dy)&&nt0.listNeuron[j].y<(nt1.listNeuron[i].y+dy)
                            &&nt0.listNeuron[j].z>(nt1.listNeuron[i].z-dz)&&nt0.listNeuron[j].z<(nt1.listNeuron[i].z+dz))
                    {
                        istrue[i]=true;
                        break;
                    }
                }

            }

            int count=0;
            bool b;
            int c=istrue.size();

            for(int i=0;i<c;++i)
            {
                if(istrue[i]==true)
                    count++;
            }
            double rate=(double)count/(double)c;
            cout<<count<<endl<<c<<endl<<size0<<endl<<(double)count/c<<endl;
            string p0=file1[i].toStdString();
            if(rate<nm)
            {
                b=false;
            }else
            {
                b=true;
            }
            out<<p0<<string(" ")<<count<<string(" ")<<size0<<string(" ")<<rate<<string(" ")<<b<<endl;

        }




    }

}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("cut_block"))
	{
        QString scan_file=infiles[0];
        const char * scandata_file=scan_file.toStdString().c_str();

        ifstream in;
        in.open(scandata_file,ios::in);
        if(! in.is_open())
        {
            fprintf(stderr,"cannot open scandata.txt");
         }

        vector<block> blockvector;
        while(!in.eof())
        {
            block tmp;
            in>>tmp.a.x;
            in>>tmp.a.y;
            in>>tmp.a.z;
            in>>tmp.path;
            in>>tmp.dx;
            in>>tmp.dy;
            in>>tmp.dz;
            blockvector.push_back(tmp);
        }

        QString eswcfile_manual= infiles[1]; // manual result
        NeuronTree nt=readSWC_file(eswcfile_manual);
        int size=nt.listNeuron.size();

        int sf= atoi(inparas[0]);
        printf("the scaling factor is %d.", sf);
        for(int k=0; k<nt.listNeuron.size();k++)
        {
            nt.listNeuron[k].x/=sf;
            nt.listNeuron[k].y/=sf;
            nt.listNeuron[k].z/=sf;

        }

        QString swcfile_auto= infiles[2];
        NeuronTree nt2= readSWC_file(swcfile_auto);

        int bs=blockvector.size();

        QString path= outfiles[0];
        QString prefix= eswcfile_manual.section(QRegExp("[/.]"),-2,-2).trimmed();
        //cout <<"prefix="<<prefix.toStdString()<<endl;
        QString txtfile=path +"/distance.txt";
//        if (QFileInfo(txtfile).exists())
//        {
//            system(qPrintable(QString("rm -rf %1").arg(txtfile.toStdString().c_str())));
//            system(qPrintable(QString("touch %1").arg(txtfile.toStdString().c_str())));
//        }
//        else{
//            system(qPrintable(QString("touch %1").arg(txtfile.toStdString().c_str())));
//        }
        QByteArray txt_ba=txtfile.toLatin1();
        char * txt_file=txt_ba.data();
        //const char * txt_file=txtfile.toStdString().c_str();
        cout<<"txt_file"<<txt_file<<endl;

        NeuronTree nttmp;
        NeuronTree nttmp2;
        bool crop_img=true;

        cout<<"block vector sz="<<bs<<endl;
        for(int i=0;i<bs-1;++i)
        {
            //crop the manual result by block
            nttmp.listNeuron.clear();
            for(int j=0;j<size;++j)
            {
                if(nt.listNeuron[j].x>blockvector[i].a.x&&nt.listNeuron[j].x<=(blockvector[i].a.x+blockvector[i].dx)
                        &&nt.listNeuron[j].y>blockvector[i].a.y&&nt.listNeuron[j].y<=(blockvector[i].a.y+blockvector[i].dy)
                        &&nt.listNeuron[j].z>blockvector[i].a.z&&nt.listNeuron[j].z<=(blockvector[i].a.z+blockvector[i].dz))
                {
                    nt.listNeuron[j].x-=blockvector[i].a.x;
                    nt.listNeuron[j].y-=blockvector[i].a.y;
                    nt.listNeuron[j].z-=blockvector[i].a.z;
                    nttmp.listNeuron.push_back(nt.listNeuron[j]);
                    nt.listNeuron[j].x+=blockvector[i].a.x;
                    nt.listNeuron[j].y+=blockvector[i].a.y;
                    nt.listNeuron[j].z+=blockvector[i].a.z;
                }

            }
            const QString s=path+"/"+prefix+"_x_"+QString::number((int)blockvector[i].a.x,10)+"_y_"+QString::number((int)blockvector[i].a.y,10)+"_z_"+
                    QString::number((int)blockvector[i].a.z,10)+".eswc";
             writeESWC_file1(s,nttmp);

            //crop the auto result by block

                 nttmp2.listNeuron.clear();
                 for(int j=0;j<nt2.listNeuron.size();++j)
                 {
                     if(nt2.listNeuron[j].x>blockvector[i].a.x&&nt2.listNeuron[j].x<=(blockvector[i].a.x+blockvector[i].dx)
                             &&nt2.listNeuron[j].y>blockvector[i].a.y&&nt2.listNeuron[j].y<=(blockvector[i].a.y+blockvector[i].dy)
                             &&nt2.listNeuron[j].z>blockvector[i].a.z&&nt2.listNeuron[j].z<=(blockvector[i].a.z+blockvector[i].dz))
                     {
                         nt2.listNeuron[j].x-=blockvector[i].a.x;
                         nt2.listNeuron[j].y-=blockvector[i].a.y;
                         nt2.listNeuron[j].z-=blockvector[i].a.z;
                         nttmp2.listNeuron.push_back(nt2.listNeuron[j]);
                         nt2.listNeuron[j].x+=blockvector[i].a.x;
                         nt2.listNeuron[j].y+=blockvector[i].a.y;
                         nt2.listNeuron[j].z+=blockvector[i].a.z;
                     }

                 }

                 if(crop_img)
                 {
                     //QString swcpath=QString::fromStdString(blockvector[i].path);

                     QString dirpath=scan_file.section("scan",0,0);
                     cout<<"dirpath"<<dirpath.toStdString().c_str()<<endl;
                     QString xyz_suffix="x_"+QString::number((int)blockvector[i].a.x,10)+"_y_"+QString::number((int)blockvector[i].a.y,10)+"_z_"+
                             QString::number((int)blockvector[i].a.z,10)+".v3draw";
                     QString imgpath=dirpath+xyz_suffix;
                     QString imgsavepath= path+"/"+prefix+"_x_"+QString::number((int)blockvector[i].a.x,10)+"_y_"+QString::number((int)blockvector[i].a.y,10)+"_z_"+
                             QString::number((int)blockvector[i].a.z,10)+".v3draw";
                     system(qPrintable(QString("cp %1 %2").arg(imgpath.toStdString().c_str()).arg(imgsavepath.toStdString().c_str())));

                 }


                 if(nttmp2.listNeuron.size()>0 && nttmp.listNeuron.size()>0)
                 {
                     const QString s2=path+"/"+prefix+"_x_"+QString::number((int)blockvector[i].a.x,10)+"_y_"+QString::number((int)blockvector[i].a.y,10)+"_z_"+
                             QString::number((int)blockvector[i].a.z,10)+"_auto.swc";
                     writeESWC_file1(s2,nttmp2);




                     // now calculate distance between nttmp and nttmp2
                     NeuronDistSimple tmp_score=neuron_score_rounding_nearest_neighbor(&nttmp,&nttmp2,0,2);
                     cout<<"\nDistance between neuron 1 "<<qPrintable(s)<<" and neuron 2 "<<qPrintable(s2)<<" is: "<<endl;
                     cout<<"entire-structure-average (from neuron 1 to 2) = "<<tmp_score.dist_12_allnodes <<endl;
                     cout<<"entire-structure-average (from neuron 2 to 1)= "<<tmp_score.dist_21_allnodes <<endl;
                     cout<<"average of bi-directional entire-structure-averages = "<<tmp_score.dist_allnodes <<endl;
                     cout<<"differen-structure-average = "<<tmp_score.dist_apartnodes<<endl;
                     cout<<"percent of different-structure (from neuron 1 to 2) = "<<tmp_score.percent_12_apartnodes<<"%"<<endl<<endl;
                     cout<<"percent of different-structure (from neuron 2 to 1) = "<<tmp_score.percent_21_apartnodes<<"%"<<endl<<endl;
                     cout<<"percent of different-structure (average) = "<<tmp_score.percent_apartnodes<<"%"<<endl<<endl;

                     ofstream myfile;
                     cout<<"\n txt_file="<<txt_file<<endl;
                     myfile.open (txt_file, ios::out|ios::app);
                     myfile << "input1 = ";
                     myfile << s.toStdString().c_str();
                     myfile << "\ninput2 = ";
                     myfile << s2.toStdString().c_str();
                     myfile << "\nentire-structure-average (from neuron 1 to 2) = ";
                     myfile << tmp_score.dist_12_allnodes;
                     myfile << "\nentire-structure-average (from neuron 2 to 1) = ";
                     myfile << tmp_score.dist_21_allnodes;
                     myfile << "\naverage of bi-directional entire-structure-averages = ";
                     myfile << tmp_score.dist_allnodes;
                     myfile << "\ndifferen-structure-average = ";
                     myfile << tmp_score.dist_apartnodes;
                     myfile << "\npercent of different-structure (from neuron 1 to 2) = ";
                     myfile << tmp_score.percent_12_apartnodes;
                     myfile << "\npercent of different-structure (from neuron 2 to 1) = ";
                     myfile << tmp_score.percent_21_apartnodes;
                     myfile << "\npercent of different-structure (average)= ";
                     myfile << tmp_score.percent_apartnodes;
                     myfile << "\n \n";

                     myfile.close();



                 }






        }



        return true;






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

