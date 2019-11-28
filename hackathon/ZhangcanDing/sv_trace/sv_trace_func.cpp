#include "sv_trace_func.h"

bool superpoint::getIndexs(vector<V3DLONG> &indexs,V3DLONG* sz)
{
    //cout<<"in getIndexs"<<endl;
    indexs.clear();
    for(int i=-rz;i<=rz;++i)
        for(int j=-ry;j<=ry;++j)
            for(int k=-rx;k<=rx;++k)
            {
                double tmpx=x+rx;
                double tmpy=y+ry;
                double tmpz=z+rz;
                superpoint tmp(tmpx,tmpy,tmpz);
                V3DLONG tmp_index=tmp.getIndex(sz);
                //cout<<tmp_index<<endl;
                indexs.push_back(tmp_index);
            }
    return true;
}

double superpoint::get_Intensity(unsigned char* pdata,V3DLONG* sz)
{
    //cout<<"in get_Intensity"<<endl;
    double intensity=0;
    vector<V3DLONG> indexs;
    indexs.clear();
    this->getIndexs(indexs,sz);
    //cout<<"indexs.size:"<<indexs.size()<<endl;
    for(int i=0;i<indexs.size();++i)
    {
        //cout<<pdata[indexs[i]]<<endl;
        intensity+=pdata[indexs[i]];
    }
    return intensity/indexs.size();
}

bool superpoint::get_Radius(unsigned char* pdata,V3DLONG* sz,double thres)
{
    int d=sz[2];
    for(int i=0;i<d;++i)
    {
        int dx=this->rx+i;
        int dy=this->ry+i;
        int dz=this->rz+i;
        superpoint tmp(this->x,this->y,this->z,dx,dy,dz);
        tmp.intensity=tmp.get_Intensity(pdata,sz);
        if(tmp.intensity<thres||tmp.x+tmp.rx>=sz[0]-1||tmp.y+tmp.ry>=sz[1]-1||tmp.z+tmp.rz>=sz[2]-1||tmp.x-tmp.rx<=0||tmp.y-tmp.ry<=0||tmp.z-tmp.rz<=0)
        {
            this->rx=tmp.rx;
            this->ry=tmp.ry;
            this->rz=tmp.rz;
            this->intensity=tmp.intensity;
            break;
        }

    }
    return true;
}

bool superpoint::get_Radius_2(unsigned char* pdata,V3DLONG* sz,double thres)
{
    int d=sz[2];
    for(int i=0;i<d;++i)
    {
        int dx=this->rx+i;
        int dy=this->ry+i;
        int dz=this->rz+i;
        int count_b=0,count_f=0;
        for(int i=-dz;i<=dz;++i)
            for(int j=-dy;j<=dy;++j)
                for(int k=-dx;k<=dx;++k)
                {
                    count_f++;
                    superpoint tmp(this->x+i,this->y+j,this->z+k);
                    V3DLONG index=tmp.getIndex(sz);
                    if(pdata[index]<thres)
                    {
                        count_b++;
                    }
                }
        if((double)count_b/count_f>0.1||this->x+dx>=sz[0]-1||this->y+dy>=sz[1]-1||this->z+dz>=sz[2]-1||this->x-dx<=0||this->y-dy<=0||this->z-dz<=0)
        {
            this->rx=dx;
            this->ry=dy;
            this->rz=dz;
            this->intensity=this->get_Intensity(pdata,sz);
            break;
        }
    }
    return true;
}

bool superpoint::get_Direction(unsigned pdata,V3DLONG* sz)
{
    return true;
}

bool sv_tracer::init_superpoints(vector<superpoint> &superpoints,unsigned char* pdata,V3DLONG* sz,double thres)
{
    superpoints.clear();
    for(int i=1;i<sz[2]-1;++i)
        for(int j=1;j<sz[1]-1;++j)
            for(int k=1;k<sz[0]-1;++k)
            {
                double x=k,y=j,z=i;
                int rx=1,ry=1,rz=1;
                superpoint tmp(x,y,z,rx,ry,rz);
                tmp.intensity=tmp.get_Intensity(pdata,sz);
                //cout<<"...intensity:"<<tmp.intensity<<endl;
                if(tmp.intensity>thres)
                {
                    superpoints.push_back(tmp);
                }
            }
    return true;
}

bool sv_tracer::init_real_points(vector<superpoint> &old,vector<superpoint> &realpoints,unsigned char* pdata,V3DLONG* sz,double thres)
{
    V3DLONG num_sz=sz[0]*sz[1]*sz[2];
    vector<int> mask=vector<int>(num_sz,0);
    multimap<double,superpoint> super_map;
    cout<<"old_size:"<<old.size()<<endl;
    for(V3DLONG i=0;i<old.size();++i)
    {
        super_map.insert(pair<double,superpoint>(old[i].intensity,old[i]));
    }
    multimap<double,superpoint>::reverse_iterator it=super_map.rbegin();

    cout<<"map_size:"<<super_map.size()<<endl;

    int count=0;

    while(it!=super_map.rend())
    {
        //cout<<"the>>"<<count<<"..."<<endl;
        vector<V3DLONG> indexs;
        superpoint tmp=it->second;
        cout<<tmp.x<<" "<<tmp.y<<" "<<tmp.z<<endl;
        tmp.get_Radius_2(pdata,sz,thres);
        tmp.getIndexs(indexs,sz);

        int b_mask=0;
        for(int i=0;i<indexs.size();++i)
        {
            if(mask[indexs[i]]==1)
            {
                b_mask++;
            }
        }
        if((double)b_mask/indexs.size()<0.001)
        {
            for(int i=0;i<indexs.size();++i)
            {
                mask[indexs[i]]=1;
            }
            realpoints.push_back(tmp);
        }

        count++;
        it++;

    }
    return true;
}

bool sv_tracer::writeSuperpoints(const QString markerfile, vector<superpoint> &superpoints)
{
    QList<ImageMarker> markers;
    for(int i=0;i<superpoints.size();++i)
    {
        ImageMarker m;
        m.x=superpoints[i].x+0.5;
        m.y=superpoints[i].y+0.5;
        m.z=superpoints[i].z+0.5;
        m.color.r=255;
        m.color.g=0;
        m.color.b=0;
        m.radius=superpoints[i].rx;
        markers.push_front(m);
    }
    writeMarker_file(markerfile,markers);
    return true;
}
