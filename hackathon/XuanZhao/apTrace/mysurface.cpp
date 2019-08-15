#include "mysurface.h"

double assemblePoint::r = 10;

bool simplePoint::getNbSimplePoint(vector<simplePoint> &nbsimplepoints, int mode)
{
    for(int i=-1;i<=1;++i)
        for(int j=-1;j<=1;++j)
            for(int k=-1;k<=1;++k)
            {
                int dvalue=abs(i)+abs(j)+abs(k);
                V3DLONG tmpx=x+i;
                V3DLONG tmpy=y+j;
                V3DLONG tmpz=z+k;
                if(mode==1)
                {
                    if(dvalue==1)
                    {
                        simplePoint tmp_p(tmpx,tmpy,tmpz);
                        nbsimplepoints.push_back(tmp_p);
                    }
                }
                else
                {
                    if(dvalue!=0)
                    {
                        simplePoint tmp_p(tmpx,tmpy,tmpz);
                        nbsimplepoints.push_back(tmp_p);
                    }
                }

            }
    return true;
}


double assemblePoint::getIntensity(vector<vector<vector<unsigned char> > > &image)
{
    if(size==0)
        return 0;
    double intensity=0;
    for(int i=0;i<sps.size();++i)
    {
        intensity+=(double)image[sps[i].x].at(sps[i].y).at(sps[i].z);
    }
    return intensity/size;
}

bool assemblePoint::assemble(vector<vector<vector<unsigned char> > > &image, vector<vector<vector<int> > > &mask, long long *sz)
{
    int mode = 2;

    double r = assemblePoint::r;

    vector<simplePoint> queue;
    for(int i=0; i<this->sps.size();++i)
    {
        queue.push_back(this->sps[i]);
    }

    while(!queue.empty())
    {
        simplePoint tmp_p = queue.front();
        queue.erase(queue.begin());

        vector<simplePoint> nbsimplepoints;
        tmp_p.getNbSimplePoint(nbsimplepoints,mode);

        for(int i=0; i<nbsimplepoints.size(); ++i)
        {
            V3DLONG x = nbsimplepoints[i].x;
            V3DLONG y = nbsimplepoints[i].y;
            V3DLONG z = nbsimplepoints[i].z;

            if(x>=0&&x<=sz[0]-1&&y>=0&&y<=sz[1]-1&&z>=0&&z<=sz[2]-1
                    &&(mask[x].at(y).at(z)==0))
            {
                double dx=this->x-x;
                double dy=this->y-y;
                double dz=this->z-z;

                unsigned char o = image[tmp_p.x].at(tmp_p.y).at(tmp_p.z);
                unsigned char n = image[x].at(y).at(z);
                if((o-n)>10)
                {
                    if(/*abs(dx)<=r&&abs(dy)<=r&&abs(dz)<=r*/true)
                    {
                        queue.push_back(nbsimplepoints[i]);
                        this->sps.push_back(nbsimplepoints[i]);
                        this->size++;
                        if(dx>0)
                            this->dx1=(this->dx1>abs(dx))?this->dx1:abs(dx);
                        else
                            this->dx0=(this->dx0>abs(dx))?this->dx0:abs(dx);
                        if(dy>0)
                            this->dy1=(this->dy1>abs(dy))?this->dy1:abs(dy);
                        else
                            this->dy0=(this->dy0>abs(dy))?this->dy0:abs(dy);
                        if(dx>0)
                            this->dz1=(this->dz1>abs(dz))?this->dz1:abs(dz);
                        else
                            this->dz0=(this->dz0>abs(dz))?this->dz0:abs(dz);
                        mask[x].at(y).at(z) = 1;
                    }
                }
                else
                {
                    mask[x].at(y).at(z) = 1;
                }
            }
        }
    }

    this->intensity = this->getIntensity(image);

    return true;
}

bool assemblePoint::renewXYZ(vector<vector<vector<unsigned char> > > &image)
{
    double x_new,y_new,z_new;

    double average_m=0;
    double sum_m=0;
    double m=0;


    for(int i=0;i<this->sps.size();++i)
    {
        average_m+=(double)image[this->sps[i].x].at(this->sps[i].y).at(this->sps[i].z);
    }
    average_m/=this->size;


    for(int i=0;i<this->sps.size();++i)
    {
        m=(double)image[this->sps[i].x].at(this->sps[i].y).at(this->sps[i].z);//-average_m;
        sum_m+=m;
        x_new+=m*this->sps[i].x;
        y_new+=m*this->sps[i].y;
        z_new+=m*this->sps[i].z;
    }


    if(size>0)
    {
        x_new/=sum_m;
        y_new/=sum_m;
        z_new/=sum_m;
    }
    else
    {
        x_new=this->x;
        y_new=this->y;
        z_new=this->z;
    }

    double d_x=this->x-x_new;
    double d_y=this->y-y_new;
    double d_z=this->z-z_new;

    this->dx0+=d_x;
    this->dx1-=d_x;
    this->dy0+=d_y;
    this->dy1-=d_y;
    this->dz0+=d_z;
    this->dz1-=d_z;

    this->x=x_new;
    this->y=y_new;
    this->z=z_new;

    return true;
}

bool assemblePoint::getSurfacePoints()
{
    int min_x=IN,min_y=IN,min_z=IN,max_x=0,max_y=0,max_z=0;
    for(int i=0; i<sps.size(); ++i)
    {
        min_x = (min_x>sps[i].x) ? sps[i].x : min_x;
        max_x = (max_x<sps[i].x) ? sps[i].x : max_x;
        min_y = (min_y>sps[i].y) ? sps[i].y : min_y;
        max_y = (max_y<sps[i].y) ? sps[i].y : max_y;
        min_z = (min_z>sps[i].z) ? sps[i].z : min_z;
        max_z = (max_z<sps[i].z) ? sps[i].z : max_z;
    }
    for(int i=0; i<sps.size(); ++i)
    {
        if(sps[i].x==min_x||sps[i].x==max_x||sps[i].y==min_y||sps[i].y==max_y||sps[i].z==min_z||sps[i].z==max_z)
        {
            surface_sps.push_back(sps[i]);
        }
    }
    return true;
}

bool assemblePoint::getNbPointsIndex(vector<assemblePoint> &apoints, vector<int> &nbpointsindex)
{
    double d = 10;
    V3DLONG x_min = this->x - this->dx0*d;
    V3DLONG x_max = this->x + this->dx0*d;
    V3DLONG y_min = this->y - this->dy0*d;
    V3DLONG y_max = this->y + this->dy0*d;
    V3DLONG z_min = this->z - this->dz0*d;
    V3DLONG z_max = this->z + this->dz0*d;

    for(int i=0; i<apoints.size(); ++i)
    {
        if(apoints[i].x>=x_min&&apoints[i].x<=x_max
                &&apoints[i].y>=y_min&&apoints[i].y<=y_max
                &&apoints[i].z>=z_min&&apoints[i].z<=z_max
                &&apoints[i].x!=this->x)
        {
            nbpointsindex.push_back(i);
        }
    }
    return true;
}

double assemblePoint::getDistance(assemblePoint &other)
{
    double d = 0;
    double d_d = this->dire*other.dire;
    d_d = -d_d;
    double d_e = sqrt((this->x-other.x)*(this->x-other.x)+(this->y-other.y)*(this->y-other.y)+(this->z*5-other.z*5)*(this->z*5-other.z*5));
    double d_h = 1;
    d = d_d*d_e*d_h;
    return d;
}

bool assemblePoint::getDirection(unsigned char* data1d, long long *sz)
{
    ImageCtrl c(data1d,sz);
    unsigned char*** data3d = c.get3ddata();

    double* vec1 = new double[3];
    double* vec2 = new double[3];
    double* vec3 = new double[3];
    int datatype = 1;
    int channo = 1;
    LocationSimple pt;
    pt.x = this->x;
    pt.y = this->y;
    pt.z = this->z;
    pt.radius = int(dx0*4+0.5);

    qDebug()<<"r: "<<pt.radius<<endl;
    double sigma1,sigma2,sigma3;
    compute_rgn_stat_new(pt,data3d,channo,sz[0],sz[1],sz[2],sz[3],datatype,vec1,vec2,vec3,sigma1,sigma2,sigma3);

    qDebug()<<"vec1: "<<vec1[0]<<" "<<vec1[1]<<" "<<vec1[2]<<endl
           <<"vec2: "<<vec2[0]<<" "<<vec2[1]<<" "<<vec2[2]<<endl
          <<"vec3: "<<vec3[0]<<" "<<vec3[1]<<" "<<vec3[2]<<endl;
    this->dire = direction(vec1[0],vec1[1],vec1[2]);

    if(vec1) delete[] vec1;
    if(vec2) delete[] vec2;
    if(vec3) delete[] vec3;

    for(int i=0; i<sz[2]; ++i)
    {
        for(int j=0; j<sz[1]; ++j)
        {
            delete[] data3d[i][j];
        }
        delete[] data3d[i];
    }
    delete[] data3d;

    return true;
 }

bool assemblePoint::showDirection(QList<ImageMarker> &markers)
{
    double mx,my,mz;
    for(int i=1; i<10; ++i)
    {
        mx = x+i*this->dire.x;
        my = y+i*this->dire.y;
        mz = z+i*this->dire.z;
        ImageMarker m(mx,my,mz);
        m.color.r = 0;
        m.color.g = 0;
        m.color.b = 255;
        m.radius = 1;
        markers.push_back(m);

        mx = x-i*this->dire.x;
        my = y-i*this->dire.y;
        mz = z-i*this->dire.z;
        m.x = mx;
        m.y = my;
        m.z = mz;
        m.color.r = 0;
        m.color.g = 0;
        m.color.b = 255;
        m.radius = 1;
        markers.push_back(m);

    }
    return true;
}



bool apTracer::initialAsseblePoint(vector<assemblePoint> &assemblePoints, vector<vector<vector<unsigned char> > > &image, long long *sz, double thres)
{
    int mode = 2;

    vector<vector<vector<int> > > mask=vector<vector<vector<int> > >(sz[0],vector<vector<int> >(sz[1],vector<int>(sz[2],0)));

    for(V3DLONG i=0;i<sz[0];++i)
        for(V3DLONG j=0;j<sz[1];++j)
            for(V3DLONG k=0;k<sz[2];++k)
            {
                if(image[i].at(j).at(k)<thres)
                    mask[i].at(j).at(k)=1;
            }

    vector<assemblePoint> iniassemblepoints;

    for(V3DLONG i=0; i<sz[0]; ++i)
    {
        for(V3DLONG j=0; j<sz[1]; ++j)
        {
            for(V3DLONG k=0; k<sz[2]; ++k)
            {
                if(mask[i].at(j).at(k)==0)
                {
                    simplePoint p(i,j,k);
                    assemblePoint a_p;
                    a_p.sps.push_back(p);
                    a_p.x = i;
                    a_p.y = j;
                    a_p.z = k;
                    a_p.size = 1;
                    a_p.intensity = a_p.getIntensity(image);
                    iniassemblepoints.push_back(a_p);
                }
            }
        }
    }

    multimap<double,assemblePoint> intensitymap;
    for(int i=0; i<iniassemblepoints.size(); ++i)
    {
        intensitymap.insert(pair<double,assemblePoint>(iniassemblepoints[i].intensity,iniassemblepoints[i]));
    }

    multimap<double,assemblePoint>::reverse_iterator it = intensitymap.rbegin();

    while(it!=intensitymap.rend())
    {
        assemblePoint tmp = it->second;

        bool e = false;

        if(mask[tmp.x].at(tmp.y).at(tmp.z))
            e = true;

        if(e==true)
        {
            it++;
            continue;
        }

        for(int i=0; i<tmp.sps.size(); ++i)
        {
            mask[tmp.sps[i].x].at(tmp.sps[i].y).at(tmp.sps[i].z)=1;
        }

        tmp.assemble(image,mask,sz);

        if(tmp.size>20)
        {
            assemblePoints.push_back(tmp);
        }

        it++;
    }
    for(int i=0;i<assemblePoints.size();++i)
    {
        assemblePoints[i].renewXYZ(image);
        qDebug()<<i<<" size: "<<assemblePoints[i].size<<" intensity: "<<assemblePoints[i].intensity<<endl;
    }

    return true;

}

bool apTracer::writeAsseblePoints(const QString markerfile, vector<assemblePoint> &assemblepoints)
{
    QList<ImageMarker> markers;
    for(int i=0;i<assemblepoints.size();++i)
    {
        ImageMarker m;
        m.x=assemblepoints[i].x+1.5;
        m.y=assemblepoints[i].y+1.5;
        m.z=assemblepoints[i].z+1.5;
        m.color.r=255;
        m.color.g=0;
        m.color.b=0;
        m.radius=1;
        markers.push_front(m);
    }
    writeMarker_file(markerfile,markers);
    return true;
}

bool apTracer::aptrace(vector<assemblePoint> &assemblePoints, vector<vector<vector<unsigned char> > > &image, NeuronTree &nt, long long *sz)
{
    for(int i=0; i<assemblePoints.size(); ++i)
    {
        assemblePoints[i].getSurfacePoints();
    }
    for(int i=0; i<assemblePoints.size(); ++i)
    {
        int count = 0;

        vector<simplePoint> tmp,tmp_child;
        tmp.clear();
        for(int k=0; k<assemblePoints[i].surface_sps.size(); ++k)
        {
            simplePoint t = assemblePoints[i].surface_sps[k];
            if(true)
                tmp.push_back(assemblePoints[i].surface_sps[k]);
        }


        for(int j=0; j<assemblePoints.size(); ++j)
        {
            tmp_child.clear();
            if(i!=j)
            {
                for(int k=0; k<assemblePoints[j].surface_sps.size(); ++k)
                {
                    simplePoint t = assemblePoints[j].surface_sps[k];
                    if(true)
                        tmp_child.push_back(assemblePoints[j].surface_sps[k]);
                }

                int same_count = 0;

                for(int k=0; k<tmp.size(); ++k)
                {
                    bool e = false;
                    for(int m=0; m<tmp_child.size(); ++m)
                    {
                        int d = abs(tmp[k].x-tmp_child[m].x)+abs(tmp[k].y-tmp_child[m].y)+abs(tmp[k].z-tmp_child[m].z);
//                        if(d<10)
//                        {
//                            qDebug()<<"d: "<<d<<endl;
//                        }

                        if(d<=3)
                        {
                            e = true;
                        }
                    }
                    if(e==true)
                    {
                        same_count++;
                    }
                }

                qDebug()<<i<<" "<<j<<" : "<<endl;

                qDebug()<<"same_count: "<<same_count<<endl;

                if((double)same_count/tmp.size()>0.2)
                {
                    count++;
                    if(count==1)
                    {
                        NeuronSWC p,par;
                        par.n = i*3;
                        par.x = assemblePoints[i].x;
                        par.y = assemblePoints[i].y;
                        par.z = assemblePoints[i].z;
                        par.parent = -1;
                        p.n = i*3+1;
                        p.x = assemblePoints[j].x;
                        p.y = assemblePoints[j].y;
                        p.z = assemblePoints[j].z;
                        p.parent = par.n;
                        nt.listNeuron.push_back(par);
                        nt.listNeuron.push_back(p);
                    }
                    else if(count==2)
                    {
                        NeuronSWC child;
                        child.n = i*3+2;
                        child.x = assemblePoints[j].x;
                        child.y = assemblePoints[j].y;
                        child.z = assemblePoints[j].z;
                        child.parent = i*3;
                        nt.listNeuron.push_back(child);
                        break;
                    }
                }
            }

        }
    }

    return true;

}

bool apTracer::findTips(vector<assemblePoint> &apoints, vector<int> &tipsindex, vector<vector<vector<unsigned char> > > &image, long long *sz)
{
    double min_x=IN,min_y=IN,min_z=IN,max_x=0,max_y=0,max_z=0;
    for(int i=0; i<apoints.size(); ++i)
    {
        min_x = (min_x>apoints[i].x) ? apoints[i].x : min_x;
        max_x = (min_x<apoints[i].x) ? apoints[i].x : max_x;
        min_y = (min_y>apoints[i].y) ? apoints[i].y : min_y;
        max_y = (min_y<apoints[i].y) ? apoints[i].y : max_y;
        min_z = (min_z>apoints[i].z) ? apoints[i].z : min_z;
        max_z = (min_z<apoints[i].z) ? apoints[i].z : max_z;
    }

    for(int i=0; i<apoints.size(); ++i)
    {
        if(apoints[i].x==min_x||apoints[i].x==max_x||apoints[i].y==min_y||apoints[i].y==max_y||apoints[i].z==min_z||apoints[i].z==max_z)
        {
            tipsindex.push_back(i);
        }
    }

    return true;
}


bool apTracer::trace(vector<assemblePoint> &assemblePoints,vector<vector<vector<unsigned char> > > &image,NeuronTree &nt,V3DLONG* sz)
{
    ImageCtrl c;
    c.Data3d_to_1d(image);
    unsigned char* pdata =c.getdata();

    int size = assemblePoints.size();
    vector<int> plist = vector<int>(size,-2);

    vector<int> tipsindex;
    this->findTips(assemblePoints,tipsindex,image,sz);

    vector<int> flag = vector<int>(size,0);
    vector<double> pi = vector<double>(size,0);

    map<assemblePoint,int> indexmap;
    for(int i=0; i<size; ++i)
    {
        assemblePoints[i].getDirection(pdata,sz);
        indexmap[assemblePoints[i]] = i;
    }

    multimap<double,assemblePoint> pimap;

    qDebug()<<tipsindex.size()<<endl;

    for(int i=0; i<tipsindex.size(); ++i)
    {
        int index = tipsindex[i];
        if(flag[index]==0)
        {
            plist[index] = -1;
            flag[index] = 2;
            pi[index] = 0;
            pimap.insert(pair<double,assemblePoint>(pi[index],assemblePoints[index]));
            assemblePoints[index].dire = direction();

            while(!pimap.empty())
            {
                assemblePoint p = pimap.begin()->second;
                int p_index = indexmap[p];
                pimap.erase(pimap.begin());
                flag[p_index] = 2;

                if(plist[p_index]!=-1)
                {
                    int par_index = plist[p_index];
                    assemblePoints[p_index].dire = direction(assemblePoints[p_index].x-assemblePoints[par_index].x,assemblePoints[p_index].y
                                                             -assemblePoints[par_index].y,assemblePoints[p_index].z-assemblePoints[par_index].z);
                    assemblePoints[p_index].dire.norm_dir();
                }

                vector<int> indexs;
                p.getNbPointsIndex(assemblePoints,indexs);
                qDebug()<<"size: "<<indexs.size()<<endl;
                for(int j=0; j<indexs.size(); ++j)
                {
                    assemblePoint& t = assemblePoints[indexs[j]];
                    t.dire = direction(t.x-assemblePoints[p_index].x,t.y-assemblePoints[p_index].y,t.z-assemblePoints[p_index].z);
                    t.dire.norm_dir();

                    double dist = t.getDistance(assemblePoints[p_index]);

                    qDebug()<<j<<" dist : "<<dist<<" dot: "<<t.dire*assemblePoints[p_index].dire<<endl;

                    if(t.dire*assemblePoints[p_index].dire<(1/2))
                    {
                        dist =IN;
                    }

                    if(dist<IN)
                    {
                        if(flag[indexs[j]]==0)
                        {
                            pi[indexs[j]] = dist;
                            flag[indexs[j]] = 1;
                            plist[indexs[j]] = p_index;
                            pimap.insert(pair<double,assemblePoint>(pi[indexs[j]],t));
                        }
                        else if(flag[indexs[j]]==1)
                        {
                            if(pi[indexs[j]]>dist)
                            {
                                multimap<double,assemblePoint>::iterator it = pimap.begin();
                                while(it!=pimap.end())
                                {
                                    assemblePoint s = it->second;
                                    if(indexmap[s]==indexs[j])
                                        break;
                                    it++;
                                }
                                pimap.erase(it);
                                pi[indexs[j]] = dist;
                                pimap.insert(pair<double,assemblePoint>(pi[indexs[j]],t));
                                plist[indexs[j]] = p_index;
                            }
                        }
                    }
                }

            }
        }
    }

    NeuronSWC node;
    for(int i=0; i<size; ++i)
    {
        node.n = i;
        node.parent = plist[i];
        node.x = assemblePoints[i].x+0.5;
        node.y = assemblePoints[i].y+0.5;
        node.z = assemblePoints[i].z+0.5;
        node.r = assemblePoints[i].dz0;
        nt.listNeuron.push_back(node);
    }

    return true;

}


bool apTracer::direc_trace(vector<assemblePoint> &assemblePoints, vector<vector<vector<unsigned char> > > &image, NeuronTree &nt, long long *sz)
{

    const double anglethres = sqrt(2)/2;

    for(int i=0; i<assemblePoints.size(); ++i)
    {
        vector<int> nbpointsindex;
        assemblePoints[i].getNbPointsIndex(assemblePoints,nbpointsindex);

        map<double,int> dmap;
        for(int j=0; j<nbpointsindex.size(); ++j)
        {
            int index = nbpointsindex[j];
            direction doftp = direction(assemblePoints[index].x-assemblePoints[i].x,assemblePoints[index].y-assemblePoints[i].y,assemblePoints[index].z-assemblePoints[i].z);
            doftp.norm_dir();
            double dot1 = abs(assemblePoints[i].dire*assemblePoints[index].dire);
            double dot2 = abs(assemblePoints[i].dire*doftp);
            double dot3 = abs(assemblePoints[index].dire*doftp);

            qDebug()<<"dot1: "<<dot1<<" dot2: "<<dot2<<" dot3: "<<dot3<<endl;

            double d = distance_two_point(assemblePoints[i],assemblePoints[index]);
            if(dot1>anglethres&&dot2>anglethres&&dot3>anglethres
                    &&d<15)
            {
                dmap[d] = index;
            }
        }

        if(!dmap.empty())
        {
            NeuronSWC par,child;
            par.n = i*2;
            par.x = assemblePoints[i].x;
            par.y = assemblePoints[i].y;
            par.z = assemblePoints[i].z;
            par.r = assemblePoints[i].dz0;
            par.parent = -1;

            int index = dmap.begin()->second;
            child.n = i*2+1;
            child.x = assemblePoints[index].x;
            child.y = assemblePoints[index].y;
            child.z = assemblePoints[index].z;
            child.r = assemblePoints[index].dz0;
            child.parent = par.n;

            nt.listNeuron.push_back(par);
            nt.listNeuron.push_back(child);
        }

    }

    return true;
}




















