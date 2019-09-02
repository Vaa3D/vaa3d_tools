#include "imagectrl.h"

bool ImageCtrl::Cor_to_Index(V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG &index)
{
    index=z*sz0*sz1+y*sz0+x;
    if(index>=sz0*sz1*sz2||index<0)
        return false;
    else
        return true;
}


bool ImageCtrl::Index_to_Cor(V3DLONG &x,V3DLONG &y,V3DLONG &z,V3DLONG index)
{
    z=index/(sz0*sz1);
    y=(index%(sz0*sz1))/sz0;
    x=index%sz0;
    if(z<0||y<0||x<0||z>=sz2||y>=sz1||x>=sz0)
        return false;
    else
        return true;
}

bool ImageCtrl::Data1d_to_3d(vector<vector<vector<unsigned char> > > &image)
{
    for(V3DLONG i=0;i<image.size();++i)
    {
        for(V3DLONG j=0;j<image[i].size();++j)
        {
            image[i].at(j).clear();
        }
        image[i].clear();
    }
    image.clear();
    for(V3DLONG i=0;i<sz0;++i)
    {
        vector<vector<unsigned char>> yz;
        for(V3DLONG ii=0;ii<yz.size();++ii)
        {
            yz[ii].clear();
        }
        yz.clear();
        for(V3DLONG j=0;j<sz1;++j)
        {
            vector<unsigned char> thick;
            thick.clear();
            for(V3DLONG k=0;k<sz2;++k)
            {
                V3DLONG index;
                this->Cor_to_Index(i,j,k,index);
                thick.push_back(data1d[index]);
            }
            yz.push_back(thick);
        }
        image.push_back(yz);
    }
    return true;
}

bool ImageCtrl::Data3d_to_1d(vector<vector<vector<unsigned char> > > &image)
{
    sz0=image.size();
    if(sz0==0) return false;
    sz1=image[0].size();
    if(sz1==0) return false;
    sz2=image[0].at(0).size();
    if(sz2==0) return false;

    V3DLONG sz_num=sz0*sz1*sz2;
    if(data1d) delete[] data1d;

    data1d=new unsigned char[sz_num];
    for(V3DLONG i=0;i<sz0;++i)
        for(V3DLONG j=0;j<sz1;++j)
            for(V3DLONG k=0;k<sz2;++k)
            {
                V3DLONG index;
                this->Cor_to_Index(i,j,k,index);
                data1d[index]=image[i].at(j).at(k);
            }
    return true;
}

double ImageCtrl::getMeanIntensity()
{
    V3DLONG sz_num = sz0*sz1*sz2;
    double sum = 0;
    for(V3DLONG i=0; i<sz_num; ++i)
    {
        sum += data1d[i];
    }

    return sum/sz_num;
}

bool ImageCtrl::corrode(vector<vector<vector<unsigned char> > > &image,int mode)
{
    sz0=image.size();
    if(sz0==0) return false;
    sz1=image[0].size();
    if(sz1==0) return false;
    sz2=image[0].at(0).size();
    if(sz2==0) return false;

    qDebug()<<"---------------------------------------";

    double thres =  this->getMode();
    vector<vector<vector<int> > > mask=vector<vector<vector<int> > >(sz0,vector<vector<int> >(sz1,vector<int>(sz2,0)));
    for(V3DLONG i=0;i<sz0;++i)
        for(V3DLONG j=0;j<sz1;++j)
            for(V3DLONG k=0;k<sz2;++k)
            {
                if(image[i].at(j).at(k)<thres)
                    mask[i].at(j).at(k)=1;
            }

    for(V3DLONG i=0; i<sz0; ++i)
    {
        for(V3DLONG j=0; j<sz1; ++j)
        {
            for(V3DLONG k=0; k<sz2; ++k)
            {
                if(mask[i].at(j).at(k)==0)
                {
                    bool e = true;
                    simplePoint p(i,j,k);
                    vector<simplePoint> nbpoints;
                    p.getNbSimplePoint(nbpoints,mode);
//                    qDebug()<<"size: "<<nbpoints.size()<<endl;
                    for(int i=0;i<nbpoints.size();++i)
                    {
                        if(nbpoints[i].x<0)
                            nbpoints[i].x=-nbpoints[i].x;
                        if(nbpoints[i].y<0)
                            nbpoints[i].y=-nbpoints[i].y;
                        if(nbpoints[i].z<0)
                            nbpoints[i].z=-nbpoints[i].z;
                        if(nbpoints[i].x>=sz0)
                            nbpoints[i].x-=2;
                        if(nbpoints[i].y>=sz1)
                            nbpoints[i].y-=2;
                        if(nbpoints[i].z>=sz2)
                            nbpoints[i].z-=2;
                        if(mask[nbpoints[i].x].at(nbpoints[i].y).at(nbpoints[i].z)==1)
                            e = false;
                    }
                    if(e==false)
                    {
                        mask[i].at(j).at(k)=2;
                    }
                }
            }
        }
    }

    for(V3DLONG i=0;i<sz0;++i)
        for(V3DLONG j=0;j<sz1;++j)
            for(V3DLONG k=0;k<sz2;++k)
            {
                if(mask[i].at(j).at(k)==2||mask[i].at(j).at(k)==1)
                    image[i].at(j).at(k)=0;
            }

    V3DLONG sz_num=sz0*sz1*sz2;
    if(data1d) delete[] data1d;

    data1d=new unsigned char[sz_num];
    for(V3DLONG i=0;i<sz0;++i)
        for(V3DLONG j=0;j<sz1;++j)
            for(V3DLONG k=0;k<sz2;++k)
            {
                V3DLONG index;
                this->Cor_to_Index(i,j,k,index);
                data1d[index]=image[i].at(j).at(k);
            }
    return true;


}

bool ImageCtrl::SaveImage(QString filename,V3DPluginCallback2 &callback)
{
    V3DLONG sz[4]={sz0,sz1,sz2,sz3};
    int datatype=1;
    simple_saveimage_wrapper(callback,filename.toStdString().c_str(),data1d,sz,datatype);
    return true;
}

unsigned char* ImageCtrl::getdata()
{
    unsigned char* pdata=0;
    V3DLONG sz_num = sz0*sz1*sz2;
    pdata = new unsigned char[sz_num];
    for(V3DLONG i=0; i<sz_num; ++i)
    {
        pdata[i] = data1d[i];
    }
    return pdata;
}

unsigned char*** ImageCtrl::get3ddata()
{
    unsigned char*** indata3d;
    indata3d = new unsigned char**[sz2];//(unsigned char***)malloc(sz2*sizeof(unsigned char**));
    for(int i=0; i<sz2; ++i)
    {
        indata3d[i] = new unsigned char*[sz1];//(unsigned char**)malloc(sz1*sizeof(unsigned char*));
        for(int j=0; j<sz1; ++j)
        {
            indata3d[i][j] = new unsigned char[sz0];//(unsigned char*)malloc(sz0*sizeof(unsigned char));
        }
    }

    for(int z=0; z<sz2; ++z)
        for(int y=0; y<sz1; ++y)
            for(int x=0; x<sz0; ++x)
            {
                indata3d[z][y][x] = data1d[z*sz1*sz0+y*sz0+x];
            }
    return indata3d;
}

bool ImageCtrl::getSegImage(vector<assemblePoint> &assemblePoints)
{
    V3DLONG sz_num = sz0*sz1*sz2;
    vector<int> flag = vector<int>(sz_num,0);
    for(int i=0; i<assemblePoints.size(); ++i)
    {
        for(int j=0; j<assemblePoints[i].sps.size(); ++j)
        {
            V3DLONG index;
            this->Cor_to_Index(assemblePoints[i].sps[j].x,assemblePoints[i].sps[j].y,assemblePoints[i].sps[j].z,index);
            data1d[index] = 100;
            flag[index] = 1;
        }
    }
    for(V3DLONG i=0; i<sz_num; ++i)
    {
        if(flag[i]==0)
            data1d[i] = 0;
    }

    return true;
}

bool ImageCtrl::histogram()
{
    V3DLONG sz_num = sz0*sz1*sz2;
    vector<int> count = vector<int>(256,0);

    map<int,int> countmap;

    for(int i=0; i<sz_num; ++i)
    {
        count[data1d[i]]++;
    }
    for(int i=0; i<count.size(); ++i)
    {
        if(count[i]!=0)
        {
            qDebug()<<i<<" count: "<<count[i]<<endl;
        }
        countmap[count[i]] = i;
    }

    for(int i=0; i<sz_num; ++i)
    {
        if(data1d[i]<=countmap.rbegin()->second)
        {
            data1d[i] = 0;
        }
    }

    return true;
}

double ImageCtrl::getMode()
{
    V3DLONG sz_num = sz0*sz1*sz2;
    vector<int> count = vector<int>(256,0);

    map<int,int> countmap;

    for(int i=0; i<sz_num; ++i)
    {
        count[data1d[i]]++;
    }
    for(int i=0; i<count.size(); ++i)
    {
        countmap[count[i]] = i;
    }

    return (double)(countmap.rbegin()->second+0.5);
}

bool ImageCtrl::segment0()
{
    vector<vector<vector<unsigned char> > > image;
    this->Data1d_to_3d(image);
    vector<vector<vector<int> > > mask = vector<vector<vector<int> > >(sz0,vector<vector<int> >(sz1,vector<int>(sz2,0)));

    double mode = this->getMode();

    qDebug()<<"mode: "<<mode<<endl;

    double mean = this->getMeanIntensity();

    qDebug()<<"mean: "<<mean<<endl;

    for(V3DLONG i=0;i<sz0;++i)
        for(V3DLONG j=0;j<sz1;++j)
            for(V3DLONG k=0;k<sz2;++k)
            {
                if(image[i].at(j).at(k)<mode)
                    mask[i].at(j).at(k)=1;
            }


    const int a =16*16;
    vector<int> counts = vector<int>(16,0);

    multimap<int,simplePoint> countmap;


    for(V3DLONG i=0; i<=sz2-16; ++i)
    {
        for(V3DLONG j=0; j<=sz1-16; ++j)
        {
            for(V3DLONG k=0; k<=sz0-16; ++k)
            {
                int count = 0;
                for(int ii=i; ii<i+16; ++ii)
                {
                    for(int jj=j; jj<j+16; ++jj)
                    {
                        for(int kk=k; kk<k+16; ++kk)
                        {
                            if(mask[ii].at(jj).at(kk)==0)
                            {
                                count++;
                            }
                        }
                    }
                }

                for(int p=0; p<16; ++p)
                {
                    if(count>p*a)
                    {
                        counts[p]++;
                    }
                }

                simplePoint p(i,j,k);
                countmap.insert(pair<int,simplePoint>(count,p));
            }
        }
    }

    for(int p=0; p<16; ++p)
    {
        qDebug()<<"counts "<<p<<" : has "<<counts[p]<<" percent: "<<(double)counts[p]/((sz0-15)*(sz1-15)*(sz2-15))<<endl;
    }

    multimap<int,simplePoint>::reverse_iterator it = countmap.rbegin();

    while(it!=countmap.rend())
    {
        double mask0_count = 0;


        simplePoint tmp = it->second;
        int s = 0;

        vector<int> intensity = vector<int>(256,0);

        for(V3DLONG ii=tmp.x; ii<tmp.x+16; ++ii)
        {
            for(V3DLONG jj=tmp.y; jj<tmp.y+16; ++jj)
            {
                for(V3DLONG kk=tmp.z; kk<tmp.z+16; ++kk)
                {
                    s += image[ii][jj][kk];
                    intensity[image[ii][jj][kk]]++;
                    if(mask[ii][jj][kk]==0)
                        mask0_count++;
                }
            }
        }

        s /= a*16;
        mask0_count = (double)mask0_count/(a*16);

        qDebug()<<"mask 0 count: "<<mask0_count<<endl;

        vector<int>::iterator it_i = intensity.begin();
        int count_i = 0;
        int index_i;
        while(it_i!=intensity.end())
        {
            count_i += *it_i;
            if(count_i>(a*1.6*9))
            {
                index_i = it_i - intensity.begin();
                break;
            }
            it_i++;
        }



        if(mask0_count>0.3)
        {
            qDebug()<<"index_i: "<<index_i<<endl;
            for(V3DLONG ii=tmp.x; ii<tmp.x+16; ++ii)
            {
                for(V3DLONG jj=tmp.y; jj<tmp.y+16; ++jj)
                {
                    for(V3DLONG kk=tmp.z; kk<tmp.z+16; ++kk)
                    {
                        if(image[ii][jj][kk]<index_i+0.5)
                        {
                            image[ii][jj][kk] = 0;
                            mask[ii][jj][kk] = 2;
                        }
                    }
                }
            }
        }



        it++;

    }

    this->Data3d_to_1d(image);



    return true;






}















