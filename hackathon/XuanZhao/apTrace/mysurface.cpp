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

direction simplePoint::getDirection(unsigned char*** data3d, long long *sz)
{

    double* vec1 = new double[3];
    double* vec2 = new double[3];
    double* vec3 = new double[3];
    int datatype = 1;
    int channo = 1;
    LocationSimple pt;
    pt.x = this->x;
    pt.y = this->y;
    pt.z = this->z;
    pt.radius = 5;

    double sigma1,sigma2,sigma3;
    compute_rgn_stat_new(pt,data3d,channo,sz[0],sz[1],sz[2],sz[3],datatype,vec1,vec2,vec3,sigma1,sigma2,sigma3);

    this->dire = direction(vec1[0],vec1[1],vec1[2]);

    if(vec1) delete[] vec1;
    if(vec2) delete[] vec2;
    if(vec3) delete[] vec3;

    return this->dire;

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

bool assemblePoint::assemble(vector<vector<vector<unsigned char> > > &image, unsigned char*** data3d, vector<vector<vector<int> > > &mask, long long *sz)
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
                direction d = nbsimplepoints[i].getDirection(data3d,sz);
                double dx=this->x-x;
                double dy=this->y-y;
                double dz=this->z-z;

                unsigned char o = image[tmp_p.x].at(tmp_p.y).at(tmp_p.z);
                unsigned char n = image[x].at(y).at(z);
                if(d*tmp_p.dire>(sqrt(3)/2)&&(o-n)>4)
                {
                    if(abs(dx)<=r&&abs(dy)<=r&&abs(dz)<=r)
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
    double d = 5;
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
    d_d = exp(-d_d);
    double d_e = sqrt((this->x-other.x)*(this->x-other.x)+(this->y-other.y)*(this->y-other.y)+(this->z*5-other.z*5)*(this->z*5-other.z*5));
    double d_h = 1;
    d = d_d*d_e*d_h;
    return d;
}

bool assemblePoint::getDirection(unsigned char ***data3d, long long *sz)
{
    double* vec1 = new double[3];
    double* vec2 = new double[3];
    double* vec3 = new double[3];
    int datatype = 1;
    int channo = 1;
    LocationSimple pt;
    pt.x = this->x;
    pt.y = this->y;
    pt.z = this->z;
    pt.radius = 5;

    qDebug()<<"r: "<<pt.radius<<endl;
    double sigma1,sigma2,sigma3;
    compute_rgn_stat_new(pt,data3d,channo,sz[0],sz[1],sz[2],sz[3],datatype,vec1,vec2,vec3,sigma1,sigma2,sigma3);

    qDebug()<<"______________________________________________________"<<endl;
    qDebug()<<this->x+1.5<<" "<<this->y+1.5<<this->z+1.5<<endl;
    qDebug()<<"vec1: "<<vec1[0]<<" "<<vec1[1]<<" "<<vec1[2]<<endl
           <<"vec2: "<<vec2[0]<<" "<<vec2[1]<<" "<<vec2[2]<<endl
          <<"vec3: "<<vec3[0]<<" "<<vec3[1]<<" "<<vec3[2]<<endl
         <<"sigma1: "<<sigma1<<" sigma2: "<<sigma2<<" sigma3: "<<sigma3<<endl;
    this->dire = direction(vec1[0],vec1[1],vec1[2]);

    if(vec1) delete[] vec1;
    if(vec2) delete[] vec2;
    if(vec3) delete[] vec3;

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

bool assemblePoint::meanShift(vector<vector<vector<unsigned char> > > &image, double r, V3DLONG* sz)
{
    double d = IN;

    while(d>0.2)
    {
        V3DLONG xb = x - r; if (xb<0) xb = 0; else if (xb >= sz[0]) xb = sz[0] - 1;
        V3DLONG xe = x + r; if (xe<0) xe = 0; else if (xe >= sz[0]) xe = sz[0] - 1;
        V3DLONG yb = y - r; if (yb<0) yb = 0; else if (yb >= sz[1]) yb = sz[1] - 1;
        V3DLONG ye = y + r; if (ye<0) ye = 0; else if (ye >= sz[1]) ye = sz[1] - 1;
        V3DLONG zb = z - r; if (zb<0) zb = 0; else if (zb >= sz[2]) zb = sz[2] - 1;
        V3DLONG ze = z + r; if (ze<0) ze = 0; else if (ze >= sz[2]) ze = sz[2] - 1;

        V3DLONG i, j, k;
        double w;

        //first get the center of mass
        double xm = 0, ym = 0, zm = 0, s = 0, mv = 0;
        for (k = zb; k <= ze; k++)
        {
            for (j = yb; j <= ye; j++)
            {
                for (i = xb; i <= xe; i++)
                {
                    w = double(image[i][j][k]);
                    xm += w*i;
                    ym += w*j;
                    zm += w*k;
                    s += w;
                }
            }
        }
        if(s>0)
        {
            xm /= s; ym /= s; zm /= s;
            mv = s / (double(ze - zb + 1)*(ye - yb + 1)*(xe - xb + 1));
        }

        d = distance_two_point(simplePoint(x,y,z),simplePoint(xm,ym,zm));
        x = xm; y = ym; z = zm;
    }

    return true;

}



bool apTracer::initialAssemblePoint(vector<assemblePoint> &assemblePoints, vector<vector<vector<unsigned char> > > &image, unsigned char*** data3d, long long *sz, double thres)
{
    int mode = 2;

//    qDebug()<<"__________________________________"<<endl;
//    qDebug()<<"in initia"<<endl;

    vector<vector<vector<int> > > mask=vector<vector<vector<int> > >(sz[0],vector<vector<int> >(sz[1],vector<int>(sz[2],0)));

    for(V3DLONG i=0;i<sz[0];++i)
        for(V3DLONG j=0;j<sz[1];++j)
            for(V3DLONG k=0;k<sz[2];++k)
            {
                if(image[i].at(j).at(k)<thres)
                    mask[i].at(j).at(k)=1;
            }

    vector<assemblePoint> iniassemblepoints;

//    qDebug()<<__LINE__<<" END mask "<<endl;

    for(V3DLONG i=0; i<sz[0]; ++i)
    {
        for(V3DLONG j=0; j<sz[1]; ++j)
        {
            for(V3DLONG k=0; k<sz[2]; ++k)
            {
                if(mask[i].at(j).at(k)==0)
                {
                    simplePoint p(i,j,k);
                    p.getDirection(data3d,sz);
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
//    qDebug()<<__LINE__<<" END initial "<<endl;

    multimap<double,assemblePoint> intensitymap;
    for(int i=0; i<iniassemblepoints.size(); ++i)
    {
        intensitymap.insert(pair<double,assemblePoint>(iniassemblepoints[i].intensity,iniassemblepoints[i]));
    }

    multimap<double,assemblePoint>::reverse_iterator it = intensitymap.rbegin();

    int count = 0;

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

        tmp.assemble(image,data3d,mask,sz);

        if(tmp.size>20)
        {
            assemblePoints.push_back(tmp);
        }

        if(tmp.size>20)
        {
            QList<ImageMarker> markers;
            for(int j=0;j<tmp.sps.size();++j)
            {
                ImageMarker m;
                m.x = tmp.sps[j].x + 1.5;
                m.y = tmp.sps[j].y + 1.5;
                m.z = tmp.sps[j].z + 1.5;
                m.color.r = 255;
                m.color.g = 0;
                m.color.b = 0;
                m.radius = 1;
                markers.push_back(m);
            }
            QString markerfile = "C://Users//BrainCenter//Desktop//cross_example//cross_negative_positive//111//" + QString::number(count) +".marker";
            writeMarker_file(markerfile,markers);
        }

        count++;

        it++;
    }
    for(int i=0;i<assemblePoints.size();++i)
    {
        assemblePoints[i].renewXYZ(image);
//        assemblePoints[i].meanShift(image,2.0,sz);
        qDebug()<<i<<" size: "<<assemblePoints[i].size<<" intensity: "<<assemblePoints[i].intensity<<endl;
    }

    return true;

}

bool apTracer::writeAssemblePoints(const QString markerfile, vector<assemblePoint> &assemblepoints)
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


bool apTracer::trace(vector<assemblePoint> &assemblePoints, vector<vector<vector<unsigned char> > > &image, unsigned char*** data3d, NeuronTree &nt, V3DLONG* sz)
{

    int size = assemblePoints.size();
    vector<int> plist = vector<int>(size,-2);

    vector<int> tipsindex;
    this->findTips(assemblePoints,tipsindex,image,sz);

    vector<int> flag = vector<int>(size,0);
    vector<double> pi = vector<double>(size,0);

    map<assemblePoint,int> indexmap;
    for(int i=0; i<size; ++i)
    {
        assemblePoints[i].getDirection(data3d,sz);
        indexmap[assemblePoints[i]] = i;
    }

    multimap<double,assemblePoint> pimap;

    qDebug()<<tipsindex.size()<<endl;

    for(int i=0; i<assemblePoints.size(); ++i)
    {
        qDebug()<<"________________________________________________"<<endl;
        qDebug()<<"the tipindex "<<i<<" : statrt..."<<endl;

        int index = i;
        if(flag[index]==0)
        {
            plist[index] = -1;
            flag[index] = 2;
            pi[index] = 0;
            pimap.insert(pair<double,assemblePoint>(pi[index],assemblePoints[index]));



            int count = 1;

            while(!pimap.empty())
            {


                assemblePoint p = pimap.begin()->second;
                int p_index = indexmap[p];
                pimap.erase(pimap.begin());
                flag[p_index] = 2;

                qDebug()<<count<<" point had be decided*******************************  index: "<<p_index<<endl;

                if(plist[p_index]!=-1)
                {
                    int par_index = plist[p_index];
                    assemblePoints[p_index].dire = direction(assemblePoints[p_index].x-assemblePoints[par_index].x,assemblePoints[p_index].y
                                                             -assemblePoints[par_index].y,assemblePoints[p_index].z-assemblePoints[par_index].z);
                    assemblePoints[p_index].dire.norm_dir();
                }

                vector<int> indexs;
                p.getNbPointsIndex(assemblePoints,indexs);
                qDebug()<<"get nbpoints_________"<<"size: "<<indexs.size()<<endl;

                int nbsize = indexs.size();

                for(int j=0; j<indexs.size(); ++j)
                {
                    if(flag[indexs[j]]!=2)
                    {
                        nbsize++;
                    }
                }

                for(int j=0; j<indexs.size(); ++j)
                {
                    qDebug()<<"the nbpoint "<<indexs[j]<<" is caculating..."<<endl;

                    assemblePoint& t = assemblePoints[indexs[j]];
                    t.dire = direction(t.x-assemblePoints[p_index].x,t.y-assemblePoints[p_index].y,t.z-assemblePoints[p_index].z);
                    t.dire.norm_dir();

                    double dist = t.getDistance(assemblePoints[p_index]);


                    double dot = t.dire*assemblePoints[p_index].dire;

                    if(nbsize>1)
                    {
                        segment a;
                        NeuronSWC child,par;
                        child.x = t.x; child.y = t.y; child.z = t.z;
                        par.x = assemblePoints[p_index].x;
                        par.y = assemblePoints[p_index].y;
                        par.z = assemblePoints[p_index].z;
                        a.points.push_back(par);
                        a.points.push_back(child);
                        double mean,std;
                        a.getSegMeanStdIntensity(image,mean,std);
                        qDebug()<<"mean: "<<mean<<" std: "<<std<<endl;


                        if(count==1)
                        {
                            dot=abs(dot);
                        }

                        if(std>5&&dot<sqrt(3)/2)
                        {
                            dist *= std;
                        }
                    }


                    qDebug()<<indexs[j]<<" dist : "<<dist<<" dot: "<<dot<<endl;

                    if(dist<8||nbsize==1)
                    {
                        if(flag[indexs[j]]==0)
                        {
                            pi[indexs[j]] = dist;
                            flag[indexs[j]] = 1;
                            plist[indexs[j]] = p_index;
                            pimap.insert(pair<double,assemblePoint>(pi[indexs[j]],t));
                            qDebug()<<"first assign____ "<<indexs[j]<<" dist: "<<dist<<" par: "<<p_index<<endl;
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
                                qDebug()<<"second assign____ "<<indexs[j]<<" dist: "<<dist<<" par: "<<p_index<<endl;
                            }
                        }
                    }
                }



                count++;
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


bool apTracer::direc_trace(vector<assemblePoint> &assemblePoints, vector<vector<vector<unsigned char> > > &image, vector<segment> &v_segment, long long *sz)
{

    qDebug()<<"in direc_trace............."<<endl;

    const double anglethres = sqrt(2)/2;

    vector<int> flag = vector<int>(assemblePoints.size(),0);

//    map<assemblePoint,int> indexmap;

//    for(int i=0; i<assemblePoints.size(); ++i)
//    {
//        indexmap[assemblePoints[i]] = i;
//    }

    for(int i=0; i<assemblePoints.size(); ++i)
    {
        vector<int> nbpointsindex;
        assemblePoints[i].getNbPointsIndex(assemblePoints,nbpointsindex);

        qDebug()<<__LINE__<<i<<endl;

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
            int index = dmap.begin()->second;
            flag[i]++;
            flag[index]++;

            NeuronSWC par,child;
            par.n = i*2;
            par.x = assemblePoints[i].x;
            par.y = assemblePoints[i].y;
            par.z = assemblePoints[i].z;
            par.r = assemblePoints[i].dz0;
            par.type = 3;
            par.parent = -1;


            child.n = i*2+1;
            child.x = assemblePoints[index].x;
            child.y = assemblePoints[index].y;
            child.z = assemblePoints[index].z;
            child.r = assemblePoints[index].dz0;
            child.type = 3;
            child.parent = par.n;

            segment segtmp;
            segtmp.points.push_back(par);
            segtmp.points.push_back(child);
            segtmp.getHeadPoint();
            segtmp.getTailPoint();
            segtmp.getHeadAngle();
            segtmp.getTailAngle();
            segtmp.getLength();

            if(flag[i]==1&&flag[index]==1)
            {
                v_segment.push_back(segtmp);
            }
            else if(flag[i]==2&&flag[index]==2)
            {
                flag[i]--;
                flag[index]--;
            }
            else if(flag[i]==2||flag[index]==2)
            {
                int tarindex = (flag[i]==2) ? i : index;
                for(int j=0; j<v_segment.size(); ++j)
                {
                    if(v_segment[j].headpoint.x==assemblePoints[tarindex].x
                            &&v_segment[j].headpoint.y==assemblePoints[tarindex].y
                            &&v_segment[j].headpoint.z==assemblePoints[tarindex].z)
                    {
                        direction d1 = v_segment[j].headangle;
                        direction d2 = direction(assemblePoints[tarindex].x-v_segment[j].headpoint.x,
                                                 assemblePoints[tarindex].y-v_segment[j].headpoint.y,
                                                 assemblePoints[tarindex].z-v_segment[j].headpoint.z);
                        if(d1*d2>PI/2)
                        {
                            NeuronSWC par;
                            par.n = i*2;
                            par.x = assemblePoints[tarindex].x;
                            par.y = assemblePoints[tarindex].y;
                            par.z = assemblePoints[tarindex].z;
                            par.r = assemblePoints[tarindex].dz0;
                            par.type = 3;
                            par.parent = -1;
                            v_segment[j].points[0].parent = par.n;
                            v_segment[j].points.insert(v_segment[j].points.begin(),par);
                            v_segment[j].getHeadPoint();
                            v_segment[j].getHeadAngle();
                            v_segment[j].getLength();
                        }
                        else
                        {
                            double length_new = distance_two_point(assemblePoints[tarindex],v_segment[j].headpoint);
                            double length_old = v_segment[j].length;
                            if(length_new>length_old)
                            {
                                for(int k=0; k<v_segment[j].points.size()-1; ++k)
                                {
                                    v_segment[j].points.pop_back();
                                }
                                NeuronSWC par;
                                par.n = i*2;
                                par.x = assemblePoints[tarindex].x;
                                par.y = assemblePoints[tarindex].y;
                                par.z = assemblePoints[tarindex].z;
                                par.r = assemblePoints[tarindex].dz0;
                                par.type = 3;
                                par.parent = -1;
                                v_segment[j].points[0].parent = par.n;
                                v_segment[j].points.insert(v_segment[j].points.begin(),par);
                                v_segment[j].getHeadPoint();
                                v_segment[j].getHeadAngle();
                                v_segment[j].getTailPoint();
                                v_segment[j].getTailAngle();
                                v_segment[j].getLength();
                            }
                        }
                    }
                    else if(v_segment[j].tailpoint.x==assemblePoints[tarindex].x
                            &&v_segment[j].tailpoint.y==assemblePoints[tarindex].y
                            &&v_segment[j].tailpoint.z==assemblePoints[tarindex].z)
                    {
                        direction d1 = v_segment[j].tailangle;
                        direction d2 = direction(assemblePoints[tarindex].x-v_segment[j].tailpoint.x,
                                                 assemblePoints[tarindex].y-v_segment[j].tailpoint.y,
                                                 assemblePoints[tarindex].z-v_segment[j].tailpoint.z);
                        if(d1*d2>PI/2)
                        {
                            NeuronSWC child;
                            child.n = i*2;
                            child.x = assemblePoints[tarindex].x;
                            child.y = assemblePoints[tarindex].y;
                            child.z = assemblePoints[tarindex].z;
                            child.r = assemblePoints[tarindex].dz0;
                            child.type = 3;
                            child.parent = v_segment[j].tailpoint.n;
                            v_segment[j].points.push_back(child);
                            v_segment[j].getTailPoint();
                            v_segment[j].getTailAngle();
                            v_segment[j].getLength();
                        }
                        else
                        {
                            double length_new = distance_two_point(assemblePoints[tarindex],v_segment[j].tailpoint);
                            double length_old = v_segment[j].length;
                            if(length_new>length_old)
                            {
                                for(int k=0; k<v_segment[j].points.size()-1; ++k)
                                {
                                    v_segment[j].points.erase(v_segment[j].points.begin());
                                }
                                NeuronSWC child;
                                child.n = i*2;
                                child.x = assemblePoints[tarindex].x;
                                child.y = assemblePoints[tarindex].y;
                                child.z = assemblePoints[tarindex].z;
                                child.r = assemblePoints[tarindex].dz0;
                                child.type = 3;
                                child.parent = v_segment[j].tailpoint.n;
                                v_segment[j].points.push_back(child);
                                v_segment[j].getHeadPoint();
                                v_segment[j].getHeadAngle();
                                v_segment[j].getTailPoint();
                                v_segment[j].getTailAngle();
                                v_segment[j].getLength();
                            }
                        }
                    }
                }

                flag[tarindex]--;
            }

        }

    }

    qDebug()<<"delete......"<<endl;

    vector<assemblePoint> atmp;

    for(int i=0; i<assemblePoints.size(); ++i)
    {
        if(flag[i]==0)
        {
            atmp.push_back(assemblePoints[i]);
        }
    }
    assemblePoints.clear();
    assemblePoints.assign(atmp.begin(),atmp.end());

    qDebug()<<"end trace......"<<endl;

    return true;
}

bool apTracer::connectPointandSegment(vector<assemblePoint> &assemblePoints, vector<segment> &v_segment)
{
    const double anglethres = sqrt(3)/2;

    int count = 0;
    for(int i=0; i<v_segment.size(); ++i)
    {
        for(int j=0; j<v_segment[i].points.size(); ++j)
        {
            count++;
        }
    }

    qDebug()<<"assemble size: "<<assemblePoints.size()<<endl;

    for(int i=0; i<assemblePoints.size(); ++i)
    {
        map<double,pair<int,bool>> dmap;

        NeuronSWC a;
        a.x = assemblePoints[i].x;
        a.y = assemblePoints[i].y;
        a.z = assemblePoints[i].z;
        for(int j=0; j<v_segment.size(); ++j)
        {
            double d1=IN,d2=IN;

            NeuronSWC n1 = v_segment[j].headpoint;
            NeuronSWC n2 = v_segment[j].points[1];
            direction h = direction(a.x-n1.x,a.y-n1.y,a.z-n1.z);
            h.norm_dir();
            if(h*v_segment[j].headangle>anglethres)
            {
                d1 = p_to_line<NeuronSWC,direction>(a,n1,n2);
            }

            NeuronSWC n3 = v_segment[j].tailpoint;
            NeuronSWC n4 = v_segment[j].points[v_segment[j].points.size()-2];
            direction t = direction(a.x-n3.x,a.y-n3.y,a.z-n3.z);
            t.norm_dir();
            if(t*v_segment[j].tailangle>anglethres)
            {
                d2 = p_to_line<NeuronSWC,direction>(a,n3,n4);
            }

            if(d1<d2)
            {
                if(d1<IN)
                {
                    dmap[d1] = pair<int,bool>(j,true);
                }
            }
            else
            {
                if(d2<IN)
                {
                    dmap[d2] = pair<int,bool>(j,false);
                }
            }
        }

        qDebug()<<"damp size: "<<dmap.size()<<endl;

        if(!dmap.empty())
        {
            bool flag = dmap.begin()->second.second;
            int index = dmap.begin()->second.first;

            if(flag==true)
            {
                NeuronSWC par;
                par.n = count+i;
                par.x = assemblePoints[i].x;
                par.y = assemblePoints[i].y;
                par.z = assemblePoints[i].z;
                par.r = assemblePoints[i].dz0;
                par.type = 3;
                par.parent = -1;

                v_segment[index].points[0].parent = par.n;
                v_segment[index].points.insert(v_segment[index].points.begin(),par);
                v_segment[index].getHeadPoint();
                v_segment[index].getHeadAngle();
            }
            else
            {
                NeuronSWC child;
                child.n = count+i;
                child.x = assemblePoints[i].x;
                child.y = assemblePoints[i].y;
                child.z = assemblePoints[i].z;
                child.r = assemblePoints[i].dz0;
                child.type = 3;
                child.parent = v_segment[index].tailpoint.n;

                v_segment[index].points.push_back(child);
                v_segment[index].getTailPoint();
                v_segment[index].getTailAngle();
            }
        }

    }
    return true;
}

bool apTracer::connectSegment(vector<segment> &v_segment)
{
    return true;
}


NeuronSWC segment::getHeadPoint()
{
    if(points.size()>0)
    {
        headpoint = points[0];
        return headpoint;
    }
    else
    {
        return NeuronSWC();
    }
}

NeuronSWC segment::getTailPoint()
{
    int size = points.size();
    if(size>0)
    {
        tailpoint = points[size-1];
        return tailpoint;
    }
    else
    {
        return NeuronSWC();
    }
}

direction segment::getHeadAngle()
{
    if(points.size()>1)
    {
        headangle = direction(points[0].x-points[1].x,points[0].y-points[1].y,points[0].z-points[1].z);
        headangle.norm_dir();
        return headangle;
    }
    else
    {
        return direction();
    }

}

direction segment::getTailAngle()
{
    int size = points.size();
    if(size>1)
    {
        tailangle = direction(points[size-1].x-points[size-2].x,points[size-1].y-points[size-2].y,points[size-1].z-points[size-2].z);
        return tailangle;
    }
    else
    {
        return direction();
    }
}

double segment::getLength()
{
    int size = points.size();
    if(size<2)
    {
        length = 0;
    }
    else
    {
        for(int i=0; i<size-1; ++i)
        {
            length+=distance_two_point(points[i],points[i+1]);
        }
    }
    return length;
}

bool segment::getSegMeanStdIntensity(vector<vector<vector<unsigned char> > > &image, double &mean, double &std)
{
    vector<simplePoint> listpoint;
    if(points.size()==0)
    {
        mean = 0;
        std = 0;
    }
    else if(points.size()==1)
    {
        mean = image[(int)(points[0].x+0.5)].at((int)(points[0].y+0.5)).at((int)(points[0].z+0.5));
        std = 0;
    }
    else
    {
//        qDebug()<<"in getsegmeanstd.........."<<endl;
        for(int i=0; i<points.size()-1; ++i)
        {
            direction d = direction(points[i+1].x-points[i].x,points[i+1].y-points[i].y,points[i+1].z-points[i].z);
            d.norm_dir();
            double l = distance_two_point(points[i+1],points[i]);
            double l0 = 0;
            listpoint.push_back(simplePoint((int)(points[i].x+0.5),(int)(points[i].y+0.5),(int)(points[i].z+0.5)));
            int count = 1;
            int lastx = (int)(points[i].x+0.5);
            int lasty = (int)(points[i].y+0.5);
            int lastz = (int)(points[i].z+0.5);
            while(l0<l)
            {
                int x = (int)(points[i].x+d.x*count+0.5);
                int y = (int)(points[i].y+d.y*count+0.5);
                int z = (int)(points[i].z+d.z*count+0.5);
                if(x!=lastx||y!=lasty||z!=lastz)
                {
                    listpoint.push_back(simplePoint(x,y,z));
                    lastx = x;
                    lasty = y;
                    lastz = z;
                }
                l0 = distance_two_point(points[i],simplePoint(x,y,z));
                count++;
            }
        }
        for(int i=0;i<listpoint.size();++i)
        {
            mean += image[listpoint[i].x].at(listpoint[i].y).at(listpoint[i].z);
        }
        mean /= listpoint.size();
        for(int i=0;i<listpoint.size();++i)
        {
            std += (image[listpoint[i].x].at(listpoint[i].y).at(listpoint[i].z)-mean)*(image[listpoint[i].x].at(listpoint[i].y).at(listpoint[i].z)-mean);
        }
        std = sqrt(std);
        std /= listpoint.size();

    }

    return true;
}

















