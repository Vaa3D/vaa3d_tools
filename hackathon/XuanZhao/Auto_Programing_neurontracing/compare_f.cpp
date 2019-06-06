#include "compare_f.h"
#include "cut_image_f.h"
#include "resampling.h"
#include <v3d_interface.h>
#include <iostream>


void select_cross(const QString dir)
{
    QDir path(dir);
    QStringList filter;
    filter<<"*eswc";
    QStringList eswcfiles=path.entryList(filter);

    QDir cross(dir);
    cross.cdUp();
    const QString cross_s=cross.absolutePath()+"/"+"cross";
    QDir cross_0;
    cross_0.mkdir(cross_s);
    for(int i=0;i<eswcfiles.size();++i)
    {
        QFileInfo eswcfile(eswcfiles[i]);
        NeuronTree e_nt,s_nt;
        QString eswcfile_0=dir+"/"+eswcfile.baseName()+".eswc";
        e_nt=readSWC_file(eswcfile_0);
        QString swcfile=dir+"/"+eswcfile.baseName()+".swc";
        s_nt=readSWC_file(swcfile);


        QVector<QVector<V3DLONG>> child_e;
        QVector<QVector<V3DLONG>> child_s;

        QVector<NeuronSWC> bif_e;
        QVector<NeuronSWC> bif_s;

        bif_e.clear();
        bif_s.clear();

        V3DLONG e_Num=e_nt.listNeuron.size();
        V3DLONG s_Num=s_nt.listNeuron.size();
        child_e=QVector<QVector<V3DLONG>>(e_Num,QVector<V3DLONG>());
        child_s=QVector<QVector<V3DLONG>>(s_Num,QVector<V3DLONG>());

        for(V3DLONG i=0;i<e_Num;++i)
        {
            V3DLONG par=e_nt.listNeuron[i].parent;
            if(isRoot(e_nt,par)) continue;
            child_e[e_nt.hashNeuron.value(par)].push_back(i);
        }
        for(V3DLONG i=0;i<s_Num;++i)
        {
            V3DLONG par=s_nt.listNeuron[i].parent;
            if(isRoot(s_nt,par)) continue;
            child_s[s_nt.hashNeuron.value(par)].push_back(i);
        }


        std::cout<<"000"<<endl;

        for(V3DLONG i=0;i<e_Num;++i)
        {
            if(child_e[i].size()>=2)
            {
                bif_e.push_back(e_nt.listNeuron[i]);
            }
        }
        for(V3DLONG i=0;i<s_Num;++i)
        {
            if(child_s[i].size()>=2)
            {
                bif_s.push_back(s_nt.listNeuron[i]);
            }
        }

        std::cout<<"111"<<endl;

        QVector<NeuronSWC> tmp;
        tmp.clear();
        for(int i=0;i<bif_s.size();++i)
        {
            bool flag=false;
            for(int j=0;j<bif_e.size();++j)
            {
                if(bif_s[i].x>(bif_e[j].x-1)
                        &&bif_s[i].x<=(bif_e[j].x+1)
                        &&bif_s[i].y>(bif_e[j].y-1)
                        &&bif_s[i].y<=(bif_e[j].y+1)
                        &&bif_s[i].z>(bif_e[j].z-1)
                        &&bif_s[i].z<=(bif_e[j].z+1))
                {
                    flag=true;
                }
            }
            if(flag==false)
            {
                tmp.push_back(bif_s[i]);
            }
            flag=false;
        }

        std::cout<<"222"<<endl;

        QVector<NeuronSWC> result;
        result.clear();

        for(int i=0;i<tmp.size();++i)
        {
            for(int j=0;j<e_nt.listNeuron.size();++j)
            {
                if(tmp[i].x>(e_nt.listNeuron[j].x-10)
                        &&tmp[i].x<=(e_nt.listNeuron[j].x+10)
                        &&tmp[i].y>(e_nt.listNeuron[j].y-10)
                        &&tmp[i].y<=(e_nt.listNeuron[j].y+10)
                        &&tmp[i].z>(e_nt.listNeuron[j].z-10)
                        &&tmp[i].z<=(e_nt.listNeuron[j].z+10))
                {
                    result.push_back(tmp[i]);
                }
            }
        }

        std::cout<<"333"<<endl;

        if(!result.isEmpty())
        {
            const QString tiffile=dir+"/"+eswcfile.baseName()+".tif";
            const QString markerfile=dir+"/"+eswcfile.baseName()+".marker";
            const QString new_tiffile=cross_s+"/"+eswcfile.baseName()+".tif";
            const QString new_markerfile=cross_s+"/"+eswcfile.baseName()+".marker";
            const QString new_eswcfile=cross_s+"/"+eswcfile.baseName()+".eswc";
            const QString new_swcfile=cross_s+"/"+eswcfile.baseName()+".swc";
            QFile::copy(tiffile,new_tiffile);
            QFile::copy(markerfile,new_markerfile);
            QFile::copy(swcfile,new_swcfile);
            QFile::copy(eswcfile_0,new_eswcfile);


        ImageMarker marker;
        QList<ImageMarker> markers;
        for(int i=0;i<result.size();++i)
        {
            marker.x=result[i].x;
            marker.y=result[i].y;
            marker.z=result[i].z;
            markers.push_back(marker);
        }


        QString file0=cross.absolutePath()+"/"+eswcfile.baseName()+QString(".marker");
        QString file1(file0);
        writeMarker_file(file1,markers);

        }
/*
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



    */

    }
}

void select_weaksignal(const QString dir)
{
    QDir path(dir);
    QStringList filter;
    filter<<"*eswc";
    QStringList eswcfiles=path.entryList(filter);

    QDir cross(dir);
    cross.cdUp();
    const QString cross_s=cross.absolutePath()+"/"+"weaksignal";
    QDir cross_0;
    cross_0.mkdir(cross_s);
    for(int i=0;i<eswcfiles.size();++i)
    {
        QFileInfo eswcfile(eswcfiles[i]);
        NeuronTree e_nt,s_nt,s_nt_0;
        QString eswcfile_0=dir+"/"+eswcfile.baseName()+".eswc";
        e_nt=readSWC_file(eswcfile_0);
        QString swcfile=dir+"/"+eswcfile.baseName()+".swc";

        s_nt_0=readSWC_file(swcfile);
        double step=2.0;
        s_nt=resample(s_nt_0,step);

        const int dx=5,dy=5,dz=5;

        bool flag=false;
        int count0=0;

        for(int j=0;j<s_nt.listNeuron.size();++j)
        {
            for(int k=0;k<e_nt.listNeuron.size();++k)
            {
                if(s_nt.listNeuron[j].x>(e_nt.listNeuron[k].x-dx)&&s_nt.listNeuron[j].x<(e_nt.listNeuron[k].x+dx)
                        &&s_nt.listNeuron[j].y>(e_nt.listNeuron[k].y-dy)&&s_nt.listNeuron[j].y<(e_nt.listNeuron[k].y+dy)
                        &&s_nt.listNeuron[j].z>(e_nt.listNeuron[k].z-dz)&&s_nt.listNeuron[j].z<(e_nt.listNeuron[k].z+dz))
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

        double rate=double(count0)/(double)e_nt.listNeuron.size();



        if(rate<0.7)
        {
            const QString tiffile=dir+"/"+eswcfile.baseName()+".tif";
            const QString markerfile=dir+"/"+eswcfile.baseName()+".marker";
            const QString new_tiffile=cross_s+"/"+eswcfile.baseName()+".tif";
            const QString new_markerfile=cross_s+"/"+eswcfile.baseName()+".marker";
            const QString new_eswcfile=cross_s+"/"+eswcfile.baseName()+".eswc";
            const QString new_swcfile=cross_s+"/"+eswcfile.baseName()+".swc";
            QFile::copy(tiffile,new_tiffile);
            QFile::copy(markerfile,new_markerfile);
            QFile::copy(swcfile,new_swcfile);
            QFile::copy(eswcfile_0,new_eswcfile);

        }
    }
}
