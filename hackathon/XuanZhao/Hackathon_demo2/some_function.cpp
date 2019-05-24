#include "some_function.h"
#include <iostream>
#include <queue>
#include <fstream>
#include "resampling.h"


bool isRoot(NeuronTree nt,V3DLONG par)
{
    bool flag=true;
    for(V3DLONG i=0;i<nt.listNeuron.size();++i)
    {
        if(par==nt.listNeuron[i].n)
        {
            flag=false;
        }
    }
    return flag;
}

QVector<QVector<V3DLONG>> getChildren(NeuronTree nt)
{
    QVector<QVector<V3DLONG>> children;
    V3DLONG nt_num=nt.listNeuron.size();
    children=QVector<QVector<V3DLONG>>(nt_num,QVector<V3DLONG>());

    for(V3DLONG i=0;i<nt_num;++i)
    {
        V3DLONG par=nt.listNeuron[i].parent;
        if(isRoot(nt,par)) continue;
        children[nt.hashNeuron.value(par)].push_back(i);
    }
    return children;
    std::cout<<"children"<<endl;
}

block getBlockOfOPoint(NeuronSWC p, int dx, int dy, int dz)
{
    //std::cout<<"in getBlockOfOPoint"<<endl;
    block btmp;
    btmp.o=p;
    int dd;
    if(p.creatmode==0)
    {
        btmp.min_x=p.x-dx/2;
        btmp.max_x=p.x+dx/2;
        btmp.min_y=p.y-dy/2;
        btmp.max_y=p.y+dy/2;
        btmp.min_z=p.z-dz/2;
        btmp.max_z=p.z+dz/2;
    }
    if(p.creatmode==1)
    {
        dd=5;
        btmp.min_x=p.x-dx/2;
        btmp.max_x=p.x+dx/2;
        btmp.min_y=p.y-dy/2;
        btmp.max_y=p.y+dy/2;
        btmp.min_z=p.z-dd;
        btmp.max_z=p.z+dz-dd;
    }
    if(p.creatmode==2)
    {
        dd=5;
        btmp.min_x=p.x-dx/2;
        btmp.max_x=p.x+dx/2;
        btmp.min_y=p.y-dy/2;
        btmp.max_y=p.y+dy/2;
        btmp.min_z=p.z-dz+dd;
        btmp.max_z=p.z+dd;
    }
    if(p.creatmode==3)
    {
        dd=5;
        btmp.min_x=p.x-dx/2;
        btmp.max_x=p.x+dx/2;
        btmp.min_y=p.y-dy+dd;
        btmp.max_y=p.y+dd;
        btmp.min_z=p.z-dz/2;
        btmp.max_z=p.z+dz/2;
    }
    if(p.creatmode==4)
    {
        dd=5;
        btmp.min_x=p.x-dx/2;
        btmp.max_x=p.x+dx/2;
        btmp.min_y=p.y-dd;
        btmp.max_y=p.y+dy-dd;
        btmp.min_z=p.z-dz/2;
        btmp.max_z=p.z+dz/2;
    }
    if(p.creatmode==5)
    {
        dd=5;
        btmp.min_x=p.x-dd;
        btmp.max_x=p.x+dx-dd;
        btmp.min_y=p.y-dy/2;
        btmp.max_y=p.y+dy/2;
        btmp.min_z=p.z-dz/2;
        btmp.max_z=p.z+dz/2;
    }
    if(p.creatmode==6)
    {
        dd=5;
        btmp.min_x=p.x-dx+dd;
        btmp.max_x=p.x+dd;
        btmp.min_y=p.y-dy/2;
        btmp.max_y=p.y+dy/2;
        btmp.min_z=p.z-dz/2;
        btmp.max_z=p.z+dz/2;
    }
    return btmp;

}

vector<NeuronSWC> getTPointOfBlock(NeuronTree &nt, block b)
{
    QVector<QVector<V3DLONG>> children=getChildren(nt);

    vector<NeuronSWC> tips;
    NeuronSWC o=b.o;
    //NeuronSWC& ptmp;
    queue<NeuronSWC> qswcs;
    qswcs.push(o);
    while(!qswcs.empty())
    {
        std::cout<<"111"<<endl;
        NeuronSWC ptmp=qswcs.front();
        std::cout<<"222"<<endl;
        qswcs.pop();
        std::cout<<"000"<<endl;

        if(ptmp.x>b.min_x&&ptmp.x<=b.max_x
                &&ptmp.y>b.min_y&&ptmp.y<=b.max_y
                &&ptmp.z>b.min_z&&ptmp.z<=b.max_z)
        {
            if(children[nt.hashNeuron.value(ptmp.n)].size()>0)
            {
                for(int i=0;i<children[nt.hashNeuron.value(ptmp.n)].size();++i)
                {
                    qswcs.push(nt.listNeuron[children[nt.hashNeuron.value(ptmp.n)][i]]);
                }
            }
            std::cout<<qswcs.size()<<endl;
            if(children[nt.hashNeuron.value(ptmp.n)].size()==0)
            {
                continue;
            }else
            {
                NeuronSWC ptmp1=nt.listNeuron[children[nt.hashNeuron.value(ptmp.n)][0]];
                if(ptmp1.z>b.max_z&&b.o.creatmode!=2)
                {
                    ptmp.creatmode=1;
                    tips.push_back(ptmp);
                }
                if(ptmp1.z<b.min_z&&b.o.creatmode!=1)
                {
                    ptmp.creatmode=2;
                    tips.push_back(ptmp);
                }
                if(ptmp1.y<b.min_y&&b.o.creatmode!=4)
                {
                    ptmp.creatmode=3;
                    tips.push_back(ptmp);
                }
                if(ptmp1.y>b.max_y&&b.o.creatmode!=3)
                {
                    ptmp.creatmode=4;
                    tips.push_back(ptmp);
                }
                if(ptmp1.x>b.max_x&&b.o.creatmode!=6)
                {
                    ptmp.creatmode=5;
                    tips.push_back(ptmp);
                }
                if(ptmp1.x<b.min_x&&b.o.creatmode!=5)
                {
                    ptmp.creatmode=6;
                    tips.push_back(ptmp);
                }
            }

        }

    }
    return tips;
}

blockTree getBlockTree(NeuronTree &nt, int dx, int dy, int dz)
{
    NeuronSWC o;
    for(V3DLONG i=0;i<nt.listNeuron.size();++i)
    {
        if(nt.listNeuron[i].parent==-1)
        {
            o=nt.listNeuron[i];
            break;
        }
    }

    o.creatmode=0;
    NeuronSWC ptmp;
    blockTree bt;
    block btmp;
    vector<NeuronSWC> ptmps,pzs;

    btmp=getBlockOfOPoint(o,dx,dy,dz);
    btmp.n=0;
    btmp.parent=-1;
    std::cout<<"getTPointOfBlock..."<<endl;
    ptmps=getTPointOfBlock(nt,btmp);
    std::cout<<"getTPointOfBlock out"<<endl;
    btmp.tips.assign(ptmps.begin(),ptmps.end());
    pzs.insert(pzs.end(),ptmps.begin(),ptmps.end());

    bt.blocklist.push_back(btmp);
    int count=0;
    while(!pzs.empty())
    {
        ptmps.clear();
        ptmp=*(pzs.end()-1);
        pzs.pop_back();
        btmp.tips.clear();
        btmp=getBlockOfOPoint(ptmp,dx,dy,dz);
        btmp.n=bt.blocklist.size();
        for(int i=0;i<bt.blocklist.size();++i)
        {
            for(int j=0;j<bt.blocklist[i].tips.size();++j)
            {
                if(btmp.o.x==bt.blocklist[i].tips[j].x
                        &&btmp.o.y==bt.blocklist[i].tips[j].y
                        &&btmp.o.z==bt.blocklist[i].tips[j].z)
                {
                    btmp.parent=bt.blocklist[i].n;
                }
            }
        }
        ptmps=getTPointOfBlock(nt,btmp);
        btmp.tips.assign(ptmps.begin(),ptmps.end());
        if(ptmps.size()!=0)
        {
            pzs.insert(pzs.end(),ptmps.begin(),ptmps.end());
        }
        bt.blocklist.push_back(btmp);
        std::cout<<pzs.size()<<endl<<count<<endl;
        count++;
    }

    if(pzs.empty())
    {
        std::cout<<"pzs empty"<<endl;
    }

    return bt;

}

void work(PARA_DEMO2& p, V3DPluginCallback2 &callback)
{
    /*if(!p.demo2_dialog())
    {
        return false;
    }*/
    std::cout<<"in work"<<endl;
    ofstream out;
    out.open("D://xinsucai//block.txt",ios::out|ios::app);

    blockTree bt;
    NeuronTree nt,nt0;

    QString suffix1=".tif";
    QString suffix2=".marker";
    QString suffix3=".swc";
    QString suffix4=".eswc";

    QString path=p.savepath;
    QString path0=path+"//";
    std::cout<<p.brainpath.toStdString()<<endl;
    std::cout<<p.savepath.toStdString()<<endl;
    std::cout<<p.dx<<" "<<p.dy<<" "<<p.dz<<endl;
    std::cout<<p.singleTree<<endl;
    std::cout<<p.eswcfiles.size()<<endl;

    for(int i=0;i<p.eswcfiles.size();++i)
    {
        std::cout<<"in echo"<<endl;
        nt0.listNeuron.clear();
        nt.listNeuron.clear();
        nt0=readSWC_file(p.eswcfiles[i]);
        double step=2.0;
        nt=resample(nt0,step);

        QFileInfo eswcfile(p.eswcfiles[i]);

        bt.blocklist.clear();
        bt=getBlockTree(nt,p.dx,p.dy,p.dz);
        std::cout<<"after getBlockTree"<<endl;

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

            std::cout<<"in for"<<endl;

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

            if(p.singleTree==true)
            {
                NeuronTree nttmp0;
                nttmp0.listNeuron.clear();

                QVector<QVector<V3DLONG>> children=getChildren(nttmp);

                V3DLONG index=0;
                for(V3DLONG j=0;j<nttmp.listNeuron.size();++j)
                {
                    if(nttmp.listNeuron[j].x==marker.x
                            &&nttmp.listNeuron[j].y==marker.y
                            &&nttmp.listNeuron[j].z==marker.z)
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
                    if(children[nttmp.hashNeuron.value(ptmp.n)].size()>0)
                    {
                        for(int i=0;i<children[nttmp.hashNeuron.value(ptmp.n)].size();++i)
                        {
                            qswcs.push(nt.listNeuron[children[nt.hashNeuron.value(ptmp.n)][i]]);
                        }
                    }
                    nttmp0.listNeuron.push_back(ptmp);
                    //std::cout<<ptmp.n<<endl;
                }
                writeESWC_file(si4,nttmp0);

            }else
            {
                writeESWC_file(si4,nttmp);
            }




            //writeSWC_file(si4,nttmp);

            out<<eswcfile.baseName().toStdString()<<" "
               <<bt.blocklist[j].n<<" "<<bt.blocklist[j].min_x<<" "
              <<bt.blocklist[j].max_x<<" "<<bt.blocklist[j].min_y<<" "
             <<bt.blocklist[j].max_y<<" "<<bt.blocklist[j].min_z<<" "
            <<bt.blocklist[j].max_z<<" "<<bt.blocklist[j].parent<<endl;
        }

    }

    //return true;

}
