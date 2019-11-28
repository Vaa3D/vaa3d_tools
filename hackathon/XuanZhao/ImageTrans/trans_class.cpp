#include "trans_class.h"
#include <fstream>

void GaussElimination(vector<vector<double> > &A)
{
    int size1=A.size();
    int size2=A[0].size();
    vector<vector<double>> B=vector<vector<double>>(100,vector<double>(100,0));
    for(int i=0;i<size1;++i)
        for(int j=0;j<size2;++j)
            B[i][j]=A[i][j];
    for(int i=0;i<size1-1;++i)
    {
        for(int j=i+1;j<size1;++j)
            for(int k=i;k<size2;++k)
            {
                A[j][k]=A[j][k]-A[i][j]*B[j][i]/B[i][i];
            }
        for(int p=0;p<size1;++p)
            for(int q=0;q<size2;++q)
                B[p][q]=A[p][q];
    }
}

void DecodeMatrix(vector<vector<double> > &A, vector<double> &AN)
{
    int size1=A.size();
    int size2=A[0].size();
    for(int i=size1-1;i>=0;--i)
    {
        double temp=A[i][size2-1];
        for(int k=i;k<size1-1;++k)
        {
            temp=A[i][size2-1]-A[i][k+1]*AN[k+1];
        }
        AN[i]=temp/A[i][i];
    }
}

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

double ImageCtrl::B_cernel(double z)
{
    double fi;
    if(z>-1&&z<1)
    {
        z=fabs(z);
        fi=0.5*z*z*z-z*z+0.666666667;
    }else if((z>-2&&z<=-1)||(z>=1&&z<2))
    {
        z=fabs(z);
        fi=-0.1666666667*z*z*z+z*z-2*z+1.3333333333;
    }else
    {
        fi=0;
    }
    return fi;
}

bool ImageCtrl::Draw_z(vector<vector<vector<unsigned char>>> &image_new,int times)
{
    if(data1d==0)
    {
        cout<<"the image is valid"<<endl;
        return false;
    }

    if(times==0)
    {
        return true;
    }

    vector<vector<vector<unsigned char>>> image;
    this->Data1d_to_3d(image);

    image_new=vector<vector<vector<unsigned char>>>(sz0,vector<vector<unsigned char>>(sz1,vector<unsigned char>((sz2+(sz2-1)*(times-1)),0)));

    vector<vector<double>> cernel=vector<vector<double>>(4,vector<double>(times-1,0));
    for(int i=0;i<4;++i)
    {
        for(int j=1;j<times;++j)
        {
            cernel[i].at(j-1)=this->B_cernel((1-i)+(double)j/times);
        }
    }

    for(V3DLONG i=0;i<sz0;++i)
        for(V3DLONG j=0;j<sz1;++j)
        {
            for(V3DLONG k=0;k<sz2;++k)
            {
                for(int p=1;p<times;++p)
                {
                    if(k==0)
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                        image_new[i].at(j).at(k*times+p)=(unsigned char)floor(image[i].at(j).at(k)*cernel[1].at(p-1)+image[i].at(j).at(k+1)*cernel[2].at(p-1)+image[i].at(j).at(k+2)*cernel[3].at(p-1));
                    }
                    else if(k==sz2-2)
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                        image_new[i].at(j).at(k*times+p)=(unsigned char)floor(image[i].at(j).at(k-1)*cernel[0].at(p-1)+image[i].at(j).at(k)*cernel[1].at(p-1)+image[i].at(j).at(k+1)*cernel[2].at(p-1));
                    }
                    else if(k==sz2-1)
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                    }
                    else
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);


                        double a=(double)(abs(image[i].at(j).at(k)-image[i].at(j).at(k-1))-abs(image[i].at(j).at(k+1)-image[i].at(j).at(k+2)))/G;
                        int n=1;
                        double wl=exp(-a*n);
                        double wr=exp(a*n);
                        image_new[i].at(j).at(k*times+p)=(unsigned char)floor(image[i].at(j).at(k-1)*cernel[0].at(p-1)*wl+image[i].at(j).at(k)*cernel[1].at(p-1)*wl+image[i].at(j).at(k+1)*cernel[2].at(p-1)*wr+image[i].at(j).at(k+2)*cernel[3].at(p-1)*wr);
                    }
                }
            }
        }

    return true;

}

bool ImageCtrl::Draw_z_PureB(vector<vector<vector<unsigned char> > > &image_new, int times)
{
    if(data1d==0)
    {
        cout<<"the image is valid"<<endl;
        return false;
    }

    if(times==0)
    {
        return true;
    }

    vector<vector<vector<unsigned char>>> image;
    this->Data1d_to_3d(image);

    image_new=vector<vector<vector<unsigned char>>>(sz0,vector<vector<unsigned char>>(sz1,vector<unsigned char>((sz2+(sz2-1)*(times-1)),0)));

    vector<vector<double>> cernel=vector<vector<double>>(4,vector<double>(times-1,0));
    for(int i=0;i<4;++i)
    {
        for(int j=1;j<times;++j)
        {
            cernel[i].at(j-1)=this->B_cernel((1-i)+(double)j/times);
        }
    }

    for(V3DLONG i=0;i<sz0;++i)
        for(V3DLONG j=0;j<sz1;++j)
        {
            for(V3DLONG k=0;k<sz2;++k)
            {
                for(int p=1;p<times;++p)
                {
                    if(k==0)
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                        image_new[i].at(j).at(k*times+p)=(unsigned char)floor(image[i].at(j).at(k)*cernel[1].at(p-1)+image[i].at(j).at(k+1)*cernel[2].at(p-1)+image[i].at(j).at(k+2)*cernel[3].at(p-1));
                    }
                    else if(k==sz2-2)
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                        image_new[i].at(j).at(k*times+p)=(unsigned char)floor(image[i].at(j).at(k-1)*cernel[0].at(p-1)+image[i].at(j).at(k)*cernel[1].at(p-1)+image[i].at(j).at(k+1)*cernel[2].at(p-1));
                    }
                    else if(k==sz2-1)
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                    }
                    else
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                        image_new[i].at(j).at(k*times+p)=(unsigned char)floor(image[i].at(j).at(k-1)*cernel[0].at(p-1)+image[i].at(j).at(k)*cernel[1].at(p-1)+image[i].at(j).at(k+1)*cernel[2].at(p-1)+image[i].at(j).at(k+2)*cernel[3].at(p-1));
                    }
                }
            }
        }

    return true;
}

bool ImageCtrl::trans(vector<unsigned char> hdzs, vector<vector<double> > &A)
{
    int size=hdzs.size();
    A=vector<vector<double>>(size,vector<double>(size+1,0));
    for(int i=0;i<size;++i)
    {
        for(int j=0;j<size;++j)
        {
            A[i][j]=pow((double)(i+1),size-j-1);
        }
        A[i][size]=(double)hdzs[i];
    }
    return true;
}

bool ImageCtrl:: Draw_z_F(vector<vector<vector<unsigned char> > > &image_new, int times)
{
    if(data1d==0)
    {
        cout<<"the image is valid"<<endl;
        return false;
    }

    if(times==0)
    {
        return true;
    }

    vector<vector<vector<unsigned char>>> image;
    this->Data1d_to_3d(image);

    image_new=vector<vector<vector<unsigned char>>>(sz0,vector<vector<unsigned char>>(sz1,vector<unsigned char>((sz2+(sz2-1)*(times-1)),0)));



    for(V3DLONG i=0;i<sz0;++i)
        for(V3DLONG j=0;j<sz1;++j)
        {
            for(V3DLONG k=0;k<sz2;++k)
            {
                for(int p=1;p<times;++p)
                {
                    if(k==0)
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                        vector<vector<double>> A;
                        vector<double> AN=vector<double>(3,0);
                        vector<unsigned char> hdzs;
                        for(int ii=0;ii<3;++ii)
                        {
                            hdzs.push_back(image[i].at(j).at(k+ii));
                        }
                        this->trans(hdzs,A);
                        GaussElimination(A);
                        DecodeMatrix(A,AN);
                        image_new[i].at(j).at(k*times+p)=(unsigned char)floor(AN[0]*pow(((double)p/times+1),2)+AN[1]*((double)p/times+1)+AN[2]);
                    }
                    else if(k==sz2-2)
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                        vector<vector<double>> A;
                        vector<double> AN=vector<double>(3,0);
                        vector<unsigned char> hdzs;
                        for(int ii=-1;ii<2;++ii)
                        {
                            hdzs.push_back(image[i].at(j).at(k+ii));
                        }
                        this->trans(hdzs,A);
                        GaussElimination(A);
                        DecodeMatrix(A,AN);
                        image_new[i].at(j).at(k*times+p)=(unsigned char)floor(AN[0]*pow(((double)p/times+2),2)+AN[1]*((double)p/times+2)+AN[2]);

                    }
                    else if(k==sz2-1)
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                    }
                    else
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);

                        vector<vector<double>> A;
                        vector<double> AN=vector<double>(4,0);
                        vector<unsigned char> hdzs;
                        for(int ii=-1;ii<3;++ii)
                        {
                            hdzs.push_back(image[i].at(j).at(k+ii));
                        }
                        this->trans(hdzs,A);
                        GaussElimination(A);
                        DecodeMatrix(A,AN);
                        image_new[i].at(j).at(k*times+p)=(unsigned char)floor(AN[0]*pow(((double)p/times+2),3)+AN[1]*pow(((double)p/times+2),2)+AN[2]*((double)p/times+2)+AN[3]);

                    }
                }
            }
        }

    return true;
}

bool ImageCtrl::Draw_z_S(vector<vector<vector<unsigned char> > > &image_new, int times)
{
    if(data1d==0)
    {
        cout<<"the image is valid"<<endl;
        return false;
    }

    if(times==0)
    {
        return true;
    }

    vector<vector<vector<unsigned char>>> image;
    this->Data1d_to_3d(image);

    image_new=vector<vector<vector<unsigned char>>>(sz0,vector<vector<unsigned char>>(sz1,vector<unsigned char>((sz2+(sz2-1)*(times-1)),0)));

    vector<vector<double>> cernel=vector<vector<double>>(4,vector<double>(times-1,0));
    for(int i=0;i<4;++i)
    {
        for(int j=1;j<times;++j)
        {
            cernel[i].at(j-1)=this->B_cernel((1-i)+(double)j/times);
        }
    }

    for(V3DLONG i=0;i<sz0;++i)
        for(V3DLONG j=0;j<sz1;++j)
        {
            for(V3DLONG k=0;k<sz2;++k)
            {
                for(int p=1;p<times;++p)
                {
                    if(k==0)
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                        image_new[i].at(j).at(k*times+p)=(unsigned char)floor(image[i].at(j).at(k)*cernel[1].at(p-1)+image[i].at(j).at(k+1)*cernel[2].at(p-1)+image[i].at(j).at(k+2)*cernel[3].at(p-1));
                    }
                    else if(k==sz2-2)
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                        image_new[i].at(j).at(k*times+p)=(unsigned char)floor(image[i].at(j).at(k-1)*cernel[0].at(p-1)+image[i].at(j).at(k)*cernel[1].at(p-1)+image[i].at(j).at(k+1)*cernel[2].at(p-1));
                    }
                    else if(k==sz2-1)
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);
                    }
                    else
                    {
                        image_new[i].at(j).at(k*times)=image[i].at(j).at(k);


                        double a=(double)(abs(image[i].at(j).at(k)-image[i].at(j).at(k-1))-abs(image[i].at(j).at(k+1)-image[i].at(j).at(k+2)))/G;
                        int n=1;
                        double wl=exp(-a*n);
                        double wr=exp(a*n);
                        image_new[i].at(j).at(k*times+p)=(unsigned char)floor(image[i].at(j).at(k-1)*cernel[0].at(p-1)*wl+image[i].at(j).at(k)*cernel[1].at(p-1)*wl+image[i].at(j).at(k+1)*cernel[2].at(p-1)*wr+image[i].at(j).at(k+2)*cernel[3].at(p-1)*wr);
                    }
                }
            }
        }

    vector<vector<vector<unsigned char>>> image_tmp=vector<vector<vector<unsigned char>>>(sz0,vector<vector<unsigned char>>(sz1,vector<unsigned char>((sz2+(sz2-1)*(times-1)),0)));

    for(V3DLONG i=0;i<sz0;++i)
        for(V3DLONG j=0;j<sz1;++j)
        {
            for(V3DLONG k=0;k<sz2-1;++k)
            {
                image_tmp[i].at(j).at(k*times)=image_new[i].at(j).at(k*times);
                for(int p=1;p<times;++p)
                {
                    if(i==0)
                    {
                        if(j==0)
                        {
                            image_tmp[i].at(j).at(k*times+p)=(unsigned char)(double)floor((image_new[i+1].at(j).at(k*times+p)+image_new[i].at(j+1).at(k*times+p)+image_new[i].at(j).at(k*times+p)*3)/5);
                        }
                        else if(j==sz1-1)
                        {
                            image_tmp[i].at(j).at(k*times+p)=(unsigned char)floor((double)(image_new[i+1].at(j).at(k*times+p)+image_new[i].at(j-1).at(k*times+p)+image_new[i].at(j).at(k*times+p)*3)/5);
                        }
                        else
                        {
                            image_tmp[i].at(j).at(k*times+p)=(unsigned char)floor((double)(image_new[i+1].at(j).at(k*times+p)+image_new[i].at(j-1).at(k*times+p)+image_new[i].at(j+1).at(k*times+p)+image_new[i].at(j).at(k*times+p)*3)/6);
                        }
                    }
                    else if(i==sz0-1)
                    {
                        if(j==0)
                        {
                            image_tmp[i].at(j).at(k*times+p)=(unsigned char)floor((double)(image_new[i-1].at(j).at(k*times+p)+image_new[i].at(j+1).at(k*times+p)+image_new[i].at(j).at(k*times+p)*3)/5);
                        }
                        else if(j==sz1-1)
                        {
                            image_tmp[i].at(j).at(k*times+p)=(unsigned char)floor((double)(image_new[i-1].at(j).at(k*times+p)+image_new[i].at(j-1).at(k*times+p)+image_new[i].at(j).at(k*times+p)*3)/5);
                        }
                        else
                        {
                            image_tmp[i].at(j).at(k*times+p)=(unsigned char)floor((double)(image_new[i-1].at(j).at(k*times+p)+image_new[i].at(j-1).at(k*times+p)+image_new[i].at(j+1).at(k*times+p)+image_new[i].at(j).at(k*times+p)*3)/6);
                        }
                    }
                    else
                    {
                        if(j==0)
                        {
                            image_tmp[i].at(j).at(k*times+p)=(unsigned char)floor((double)(image_new[i+1].at(j).at(k*times+p)+image_new[i-1].at(j).at(k*times+p)+image_new[i].at(j+1).at(k*times+p)+image_new[i].at(j).at(k*times+p)*3)/6);
                        }
                        else if(j==sz1-1)
                        {
                            image_tmp[i].at(j).at(k*times+p)=(unsigned char)floor((double)(image_new[i+1].at(j).at(k*times+p)+image_new[i-1].at(j).at(k*times+p)+image_new[i].at(j-1).at(k*times+p)+image_new[i].at(j).at(k*times+p)*3)/6);
                        }
                        else
                        {
                            image_tmp[i].at(j).at(k*times+p)=(unsigned char)floor((double)(image_new[i+1].at(j).at(k*times+p)+image_new[i-1].at(j).at(k*times+p)+image_new[i].at(j-1).at(k*times+p)+image_new[i].at(j+1).at(k*times+p)+image_new[i].at(j).at(k*times+p)*4)/8);
                        }
                    }
                }

                image_tmp[i].at(j).at((sz2-1)*times)=image_new[i].at(j).at((sz2-1)*times);
            }
        }

    for(V3DLONG i=0;i<sz0;++i)
        for(V3DLONG j=0;j<sz1;++j)
            for(V3DLONG k=0;k<image_new[0].at(0).size();++k)
            {
                image_new[i].at(j).at(k)=image_tmp[i].at(j).at(k);
            }





    return true;
}

bool ImageCtrl::Draw_S(vector<vector<vector<unsigned char> > > &image_new, int times, int mode)
{
    if(mode==0)
        this->Draw_z(image_new,times);
    if(mode==1)
        this->Draw_z_PureB(image_new,times);
    if(mode==2)
        this->Draw_z_F(image_new,times);
    if(mode==3)
        this->Draw_z_S(image_new,mode);

    return true;
}

bool ImageCtrl::SaveImage(QString filename,V3DPluginCallback2 &callback)
{
    V3DLONG sz[4]={sz0,sz1,sz2,sz3};
    int datatype=1;
    simple_saveimage_wrapper(callback,filename.toStdString().c_str(),data1d,sz,datatype);
    return true;
}

void ImageCtrl::display(vector<vector<vector<unsigned char> > > &image)
{
    for(V3DLONG i=0;i<sz0;++i)
        for(V3DLONG j=0;j<sz1;++j)
        {
            cout<<"for "<<i<<" "<<j<<" :"<<endl;
            for(V3DLONG k=0;k<sz2;++k)
            {
                cout<<(int)image[i].at(j).at(k)<<" ";
            }
            cout<<endl<<endl;
        }
}



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

double superpoint::get_Intensity(vector<vector<vector<unsigned char> > > &image,V3DLONG* sz)
{
    double intensity=0;
    int c=0;
    for(V3DLONG i=-rz;i<=rz;++i)
        for(V3DLONG j=-ry;j<=ry;++j)
            for(V3DLONG k=-rx;k<=rx;++k)
            {
                double tmpx=x+i;
                double tmpy=y+j;
                double tmpz=z+k;
                if(tmpx>sz[0]-1||tmpx<0||tmpy>sz[1]-1||tmpy<0||tmpz>sz[2]-1||tmpz<0)
                    continue;
                intensity+=(double)image[tmpx].at(tmpy).at(tmpz);
                c++;
            }
    return intensity/c;
}

double superpoint::get_BackIntensity(vector<vector<vector<unsigned char> > > &image,V3DLONG* sz)
{
    double back_intensity=0;
    int c=0;
    int d=1;
    for(V3DLONG i=-rz-d;i<=rz+d;++i)
        for(V3DLONG j=-ry-d;j<=ry+d;++j)
            for(V3DLONG k=-rx-d;k<=rx+d;++k)
            {
                double tmpx=x+i;
                double tmpy=y+j;
                double tmpz=z+k;
                if(tmpx>sz[0]-1||tmpx<0||tmpy>sz[1]-1||tmpy<0||tmpz>sz[2]-1||tmpz<0)
                    continue;
                back_intensity+=(double)image[tmpx].at(tmpy).at(tmpz);
                c++;
            }
    return back_intensity/c;
}

bool superpoint:: get_Radius(unsigned char* pdata,V3DLONG* sz,double thres)
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

bool superpoint::get_Radius(vector<vector<vector<unsigned char> > > &image,V3DLONG* sz, double thres)
{
    int d=image.size();
    int z_s=5;
    for(int i=0;i<d;++i)
    {
        int dx=this->rx+i;
        int dy=this->ry+i;
        int dz=this->rz+i/z_s;
        superpoint tmp(this->x,this->y,this->z,dx,dy,dz);
        tmp.intensity=tmp.get_Intensity(image,sz);
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

bool superpoint::get_Radius_2(vector<vector<vector<unsigned char> > > &image,V3DLONG* sz, double thres)
{
    int d=image.size();
    int z_s=5;
    for(int i=0;i<d;++i)
    {
        int dx=this->rx+i;
        int dy=this->ry+i;
        int dz=this->rz+i/z_s;
        int count_b=0,count_f=0;
        for(int i=-dz;i<=dz;++i)
            for(int j=-dy;j<=dy;++j)
                for(int k=-dx;k<=dx;++k)
                {
                    count_f++;
                    double tmpx=x+k;
                    double tmpy=y+j;
                    double tmpz=z+i;
                    if((double)image[tmpx].at(tmpy).at(tmpz)<(thres))
                    {
                        count_b++;
                    }
                }

        if((double)count_b/count_f>0.01||this->x+dx>=sz[0]-1||this->y+dy>=sz[1]-1||this->z+dz>=sz[2]-1||this->x-dx<=0||this->y-dy<=0||this->z-dz<=0)
        {
            this->rx=dx;
            this->ry=dy;
            this->rz=dz;
//            cout<<"x: "<<this->x+dx<<" "<<this->x-dx<<"y: "<<this->y+dy<<" "<<this->y-dy<<"z: "<<this->z+dz<<" "<<this->z-dz<<endl;
            this->intensity=this->get_Intensity(image,sz);
//            cout<<"222222222222222222222"<<endl;
            break;
        }

    }
    return true;
}

bool superpoint::get_Direction(unsigned pdata,V3DLONG* sz)
{
    return true;
}

bool superpoint::get_Direction(vector<vector<vector<unsigned char> > > &image, long long *sz)
{
    int d=15;//暂定

    vector<double> Vdirection=vector<double>(13,0);
    vector<int> count=vector<int>(13,0);
    for(int i=-rx-d;i<=rx+d;++i)
    {
        V3DLONG tmpx=this->x+i;
        if(tmpx>sz[0]-1||tmpx<0)
            continue;
        V3DLONG tmpy=this->y;
        V3DLONG tmpz=this->z;
        Vdirection[0]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[0]++;
    }
    Vdirection[0]/=count[0];

    for(int i=-ry-d;i<=ry+d;++i)
    {
        V3DLONG tmpx=this->x;
        V3DLONG tmpy=this->y+i;
        if(tmpy>sz[1]-1||tmpy<0)
            continue;
        V3DLONG tmpz=this->z;
        Vdirection[1]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[1]++;
    }
    Vdirection[1]/=count[1];

    for(int i=-rz-d;i<=rz+d;++i)
    {
        V3DLONG tmpx=this->x;
        V3DLONG tmpy=this->y;
        V3DLONG tmpz=this->z+i;
        if(tmpz>sz[2]-1||tmpz<0)
            continue;
        Vdirection[2]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[2]++;
    }
    Vdirection[2]/=count[2];

    for(int i=rx-d;i<=rx+d;++i)
    {
        V3DLONG tmpx=this->x+i;
        V3DLONG tmpy=this->y+i;
        if(tmpx>sz[0]-1||tmpx<0||tmpy>sz[1]-1||tmpy<0)
            continue;
        V3DLONG tmpz=this->z;
        Vdirection[3]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[3]++;
    }
    Vdirection[3]/=count[3];

    for(int i=rx-d;i<=rx+d;++i)
    {
        V3DLONG tmpx=this->x+i;
        V3DLONG tmpy=this->y-i;
        if(tmpx>sz[0]-1||tmpx<0||tmpy>sz[1]-1||tmpy<0)
            continue;
        V3DLONG tmpz=this->z;
        Vdirection[4]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[4]++;
    }
    Vdirection[4]/=count[4];

    for(int i=rx-d;i<=rx+d;++i)
    {
        V3DLONG tmpx=this->x+i;
        V3DLONG tmpy=this->y;
        V3DLONG tmpz=this->z+i;
        if(tmpx>sz[0]-1||tmpx<0||tmpz>sz[2]-1||tmpz<0)
            continue;
        Vdirection[5]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[5]++;
    }
    Vdirection[5]/=count[5];

    for(int i=rx-d;i<=rx+d;++i)
    {
        V3DLONG tmpx=this->x+i;
        V3DLONG tmpy=this->y;
        V3DLONG tmpz=this->z-i;
        if(tmpx>sz[0]-1||tmpx<0||tmpz>sz[2]-1||tmpz<0)
            continue;
        Vdirection[6]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[6]++;
    }
    Vdirection[6]/=count[6];

    for(int i=ry-d;i<=ry+d;++i)
    {
        V3DLONG tmpx=this->x;
        V3DLONG tmpy=this->y+i;
        V3DLONG tmpz=this->z+i;
        if(tmpy>sz[1]-1||tmpy<0||tmpz>sz[2]-1||tmpz<0)
            continue;
        Vdirection[7]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[7]++;
    }
    Vdirection[7]/=count[7];

    for(int i=ry-d;i<=ry+d;++i)
    {
        V3DLONG tmpx=this->x;
        V3DLONG tmpy=this->y+i;
        V3DLONG tmpz=this->z-i;
        if(tmpy>sz[1]-1||tmpy<0||tmpz>sz[2]-1||tmpz<0)
            continue;
        Vdirection[8]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[8]++;
    }
    Vdirection[8]/=count[8];

    for(int i=rx-d;i<=rx+d;++i)
    {
        V3DLONG tmpx=this->x+i;
        V3DLONG tmpy=this->y+i;
        V3DLONG tmpz=this->z+i;
        if(tmpx>sz[0]-1||tmpx<0||tmpy>sz[1]-1||tmpy<0||tmpz>sz[2]-1||tmpz<0)
            continue;
        Vdirection[9]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[9]++;
    }
    Vdirection[9]/=count[9];

    for(int i=rx-d;i<=rx+d;++i)
    {
        V3DLONG tmpx=this->x-i;
        V3DLONG tmpy=this->y+i;
        V3DLONG tmpz=this->z+i;
        if(tmpx>sz[0]-1||tmpx<0||tmpy>sz[1]-1||tmpy<0||tmpz>sz[2]-1||tmpz<0)
            continue;
        Vdirection[10]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[10]++;
    }
    Vdirection[10]/=count[10];

    for(int i=rx-d;i<=rx+d;++i)
    {
        V3DLONG tmpx=this->x+i;
        V3DLONG tmpy=this->y-i;
        V3DLONG tmpz=this->z+i;
        if(tmpx>sz[0]-1||tmpx<0||tmpy>sz[1]-1||tmpy<0||tmpz>sz[2]-1||tmpz<0)
            continue;
        Vdirection[11]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[11]++;
    }
    Vdirection[11]/=count[11];

    for(int i=rx-d;i<=rx+d;++i)
    {
        V3DLONG tmpx=this->x+i;
        V3DLONG tmpy=this->y+i;
        V3DLONG tmpz=this->z-i;
        if(tmpx>sz[0]-1||tmpx<0||tmpy>sz[1]-1||tmpy<0||tmpz>sz[2]-1||tmpz<0)
            continue;
        Vdirection[12]+=(double)image[tmpx].at(tmpy).at(tmpz);
        count[12]++;
    }
    Vdirection[12]/=count[12];

    multimap<double,int> dmap;
    for(int i=0;i<Vdirection.size();++i)
    {
        dmap.insert(pair<double,int>(Vdirection[i],i));
    }
    multimap<double,int>::reverse_iterator it=dmap.rbegin();
    int index=it->second;

    switch (index) {
    case 0:
        this->direc=direction(1,0,0);
        break;
    case 1:
        this->direc=direction(0,1,0);
        break;
    case 2:
        this->direc=direction(0,0,1);
        break;
    case 3:
        this->direc=direction(A1,A1,0);
        break;
    case 4:
        this->direc=direction(A1,-A1,0);
        break;
    case 5:
        this->direc=direction(A1,0,A1);
        break;
    case 6:
        this->direc=direction(A1,0,-A1);
        break;
    case 7:
        this->direc=direction(0,A1,A1);
        break;
    case 8:
        this->direc=direction(0,A1,-A1);
        break;
    case 9:
        this->direc=direction(A2,A2,A2);
        break;
    case 10:
        this->direc=direction(-A2,A2,A2);
        break;
    case 11:
        this->direc=direction(A2,-A2,A2);
        break;
    case 12:
        this->direc=direction(A2,A2,-A2);
        break;
    default:
        break;
    }

    return true;


}

bool superpoint::get_nb_points_index(vector<superpoint> &v_points, vector<int> &n_points_index)
{
    double d=5;
    V3DLONG x_min=this->x-this->rx*d;
    V3DLONG x_max=this->x+this->rx*d;
    V3DLONG y_min=this->y-this->ry*d;
    V3DLONG y_max=this->y+this->ry*d;
    V3DLONG z_min=this->z-this->rz*d;
    V3DLONG z_max=this->z+this->rz*d;
    for(int i=0;i<v_points.size();++i)
    {
        if(v_points[i].x>=x_min&&v_points[i].x<=x_max
                &&v_points[i].y>=y_min&&v_points[i].y<=y_max
                &&v_points[i].z>=z_min&&v_points[i].z<=z_max)
        {
            n_points_index.push_back(i);
        }
    }
    return true;
}

bool sv_tracer::init_superpoints(vector<superpoint> &superpoints,unsigned char* pdata,V3DLONG* sz,double thres)
{
    superpoints.clear();
    for(int i=3;i<sz[2]-3;++i)
        for(int j=3;j<sz[1]-3;++j)
            for(int k=3;k<sz[0]-3;++k)
            {
                double x=k,y=j,z=i;
                int rx=3,ry=3,rz=3;
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

double superpoint::get_distance(superpoint &other)
{
    double d=0;
    double d_d=this->direc*other.direc;
    d_d=exp(-d_d);
    double d_e=sqrt((this->x-other.x)*(this->x-other.x)+(this->y-other.y)*(this->y-other.y)+(this->z*5-other.z*5)*(this->z*5-other.z*5));

    double d_h=exp(-this->intensity);
    d=d_d*d_e*d_h;
    return d;
}

bool sv_tracer::init_superpoints(vector<superpoint> &superpoints, vector<vector<vector<unsigned char> > > &image, long long *sz, double thres)
{
    superpoints.clear();
    for(int i=1;i<sz[2]-1;++i)
        for(int j=1;j<sz[1]-1;++j)
            for(int k=1;k<sz[0]-1;++k)
            {
                double x=k,y=j,z=i;
                int rx=1,ry=1,rz=1;
                superpoint tmp(x,y,z,rx,ry,rz);
                tmp.intensity=tmp.get_Intensity(image,sz);

                tmp.back_intensity=tmp.get_BackIntensity(image,sz);
//                cout<<"intensity: "<<tmp.intensity<<"  back_intensity: "<<tmp.back_intensity<<endl;
                tmp.rate=tmp.intensity/tmp.back_intensity;
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
//        cout<<"the>>"<<count<<"..."<<endl;
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

bool sv_tracer::init_real_points(vector<superpoint> &old, vector<superpoint> &realpoints, vector<vector<vector<unsigned char> > > &image, long long *sz, double thres)
{
    //V3DLONG num_sz=sz[0]*sz[1]*sz[2];
    //vector<int> mask=vector<int>(num_sz,0);
    vector<vector<vector<int>>> mask=vector<vector<vector<int>>>(image.size(),vector<vector<int>>(image[0].size(),vector<int>(image[0].at(0).size(),0)));

    multimap<double,superpoint> super_map;
    cout<<"old_size:"<<old.size()<<endl;
    for(V3DLONG i=0;i<old.size();++i)
    {
        //super_map.insert(pair<double,superpoint>(old[i].intensity,old[i]));
        super_map.insert(pair<double,superpoint>(old[i].rate,old[i]));
    }
    multimap<double,superpoint>::reverse_iterator it=super_map.rbegin();

    cout<<"map_size:"<<super_map.size()<<endl;

    int count=0;

    while(it!=super_map.rend())
    {
//        cout<<"the>>"<<count<<"..."<<endl;
        //vector<V3DLONG> indexs;
        superpoint tmp=it->second;
        //cout<<tmp.x<<" "<<tmp.y<<" "<<tmp.z<<endl;
//        tmp.get_Radius_2(image,sz,thres);
        //tmp.getIndexs(indexs,sz);
//        cout<<"!!!!!!!!!!!!!!!"<<endl;

        int b_mask=0,size=0;
//        for(int i=0;i<indexs.size();++i)
//        {
//            if(mask[indexs[i]]==1)
//            {
//                b_mask++;
//            }
//        }
        for(int i=-tmp.rx;i<=tmp.rx;++i)
            for(int j=-tmp.ry;j<=tmp.ry;++j)
                for(int k=-tmp.rz;k<=tmp.rz;++k)
                {
                    V3DLONG tmpx=tmp.x+i;
                    V3DLONG tmpy=tmp.y+j;
                    V3DLONG tmpz=tmp.z+k;
                    size++;
                    if(mask[tmpx].at(tmpy).at(tmpz)==1)
                    {
                        b_mask++;
                    }
                }


        if((double)b_mask/size==0)
        {
            for(int i=-tmp.rx;i<=tmp.rx;++i)
                for(int j=-tmp.ry;j<=tmp.ry;++j)
                    for(int k=-tmp.rz;k<=tmp.rz;++k)
                    {
                        V3DLONG tmpx=tmp.x+i;
                        V3DLONG tmpy=tmp.y+j;
                        V3DLONG tmpz=tmp.z+k;
                        mask[tmpx].at(tmpy).at(tmpz)=1;
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
        m.x=superpoints[i].x+1.5;
        m.y=superpoints[i].y+1.5;
        m.z=superpoints[i].z+1.5;
        m.color.r=255;
        m.color.g=0;
        m.color.b=0;
        m.radius=superpoints[i].rx;
        markers.push_front(m);
    }
    writeMarker_file(markerfile,markers);
    return true;
}


bool sv_tracer::trace(vector<superpoint> &realpoints, vector<int> &plist, vector<vector<vector<unsigned char> > > &image, long long *sz)
{
    map<superpoint,int> smap;
    multimap<int,superpoint> r_map;

    cout<<"calculate direction..."<<endl;

    int r_size=realpoints.size();
    for(int i=0;i<r_size;++i)
    {
        realpoints[i].get_Direction(image,sz);
        smap[realpoints[i]]=i;
        r_map.insert(pair<int,superpoint>(realpoints[i].rx,realpoints[i]));
    }

    cout<<"end calculate direction..."<<endl;

    vector<int> flag=vector<int>(r_size,0);
    vector<double> pi=vector<double>(r_size,0);



    //vector<superpoint> v_points;

    multimap<double,superpoint> pimap;

    bool s=true;

    int count=1;

    while(s)
    {
        multimap<int,superpoint>::reverse_iterator it=r_map.rbegin();

        while(it!=r_map.rend())
        {
            //cout<<"??????"<<endl;
            superpoint tmp=it->second;
            int p_index=smap[tmp];
            if(flag[p_index]==0)
            {
                plist[p_index]=-1;
                flag[p_index]=2;
                pi[p_index]=0;
                //v_points.push_back(tmp);

                pimap.insert(pair<double,superpoint>(pi[p_index],tmp));
                break;
            }
            it++;
        }
        if(it==r_map.rend())
            s=false;

        int cc=1;

        while(!pimap.empty())
        {
            superpoint p=pimap.begin()->second;

            int p_index=smap[p];
            //v_points.erase(v_points.begin());
            pimap.erase(pimap.begin());
            flag[p_index]=2;

            if(plist[p_index]!=-1)
            {
                int par_index=plist[p_index];
                direction par_p=realpoints[p_index].direc-realpoints[par_index].direc;
                if(par_p*realpoints[p_index].direc<0)
                {
                    //realpoints[p_index].direc.negative();
                    realpoints[p_index].direc=par_p;
                }
            }

            vector<int> indexs;
            p.get_nb_points_index(realpoints,indexs);
            cout<<"index size:"<<indexs.size()<<endl;
            for(int i=0;i<indexs.size();++i)
            {


//                direction p_t=realpoints[indexs[i]].direc-p.direc;
//                if(p_t*realpoints[indexs[i]].direc<0)
//                {
//                    realpoints[indexs[i]].direc.negative();
//                }

                superpoint t=realpoints[indexs[i]];

                //if(t.direc*p.direc==0) continue;
                double dist=t.get_distance(p);

                cout<<"p_index "<<indexs[i]<<" dist: "<<dist<<endl;
                if(flag[indexs[i]]==0)
                {
                    cout<<"======0"<<endl;

                    pi[indexs[i]]=dist;
                    flag[indexs[i]]=1;
                    plist[indexs[i]]=smap[p];
                    pimap.insert(pair<double,superpoint>(pi[indexs[i]],t));
                }
                else if(flag[indexs[i]]==1)
                {
                    if(pi[indexs[i]]>dist)
                    {
                        //multimap<double,superpoint>::iterator it=pimap.find(pi[indexs[i]]);
                        multimap<double,superpoint>::iterator it_a=pimap.begin();
                        for(;it_a!=pimap.end();)
                        {
                            superpoint s=it_a->second;
                            //cout<<"indexs[i]"<<indexs[i]<<endl;
                            if(smap[s]==indexs[i])
                                break;
                            it_a++;
                            //cout<<"!!!!!!!!!!!!!!!!!!!!!!"<<endl;
                        }
                        //it->first=dist;
                        pimap.erase(it_a);
                        pi[indexs[i]]=dist;
                        pimap.insert(pair<double,superpoint>(pi[indexs[i]],t));
                        plist[indexs[i]]=smap[p];

                    }
                }
            }
            cout<<"for cc :"<<cc<<"..."<<endl;
            cc++;

        }

        cout<<"for "<<count<<":..."<<endl;
        count++;


    }


    return true;
}


bool sv_tracer::sv_trace(vector<superpoint> &realpoints, vector<vector<vector<unsigned char> > > &image, NeuronTree &nt, long long *sz)
{
    int size=realpoints.size();
    vector<int> plist=vector<int>(size,-2);
    this->trace(realpoints,plist,image,sz);

    cout<<"end trace..."<<endl;

    NeuronSWC tmp;
/*
    vector<vector<int>> children=vector<vector<int>>(size,vector<int>());
    for(int i=0;i<size;++i)
    {
        if(plist[i]<0) continue;
        children[plist[i]].push_back(i);
    }


    vector<bool> flag=vector<bool>(size,false);

    bool s=true;

    while(s)
    {
        for(int i=0;i<size;++i)
        {
            if(plist[i]==-1&&flag[i]==false)
            {

            }
        }
    }
*/
    for(int i=0;i<size;++i)
    {
        tmp.n=i;
        tmp.parent=plist[i];
        tmp.x=realpoints[i].x+0.5;
        tmp.y=realpoints[i].y+0.5;
        tmp.z=realpoints[i].z+0.5;
        tmp.r=realpoints[i].rx;
        nt.listNeuron.push_back(tmp);
    }

    cout<<"end..."<<endl;

    return true;


}


bool sv_tracer::find_tips(vector<superpoint> &superpoints, vector<superpoint> &tips, vector<vector<vector<unsigned char> > > &image, long long *sz)
{
    int s_size=superpoints.size();

    vector<vector<double>> intens=vector<vector<double>>(s_size,vector<double>(26,0));

//    for(int i=0;i<s_size;++i)
//    {
//        superpoints[i].get_Direction(image,sz);
//    }

    direction dire[13]={direction(1,0,0),direction(0,1,0),direction(0,0,1),direction(1,1,0),direction(1,0,1),direction(0,1,1),direction(1,-1,0),direction(1,0,-1),direction(0,1,-1),direction(1,1,1),direction(-1,1,1),direction(1,-1,1),direction(1,1,-1)};

    for(int i=0;i<s_size;++i)
    {
//        if(superpoints[i].direc.x==A1||superpoints[i].direc.y==A1||superpoints[i].direc.z==A1)
//        {
//            direction d(superpoints[i].direc.x*2,superpoints[i].direc.y*2,superpoints[i].direc.z*2);
//            superpoints[i].direc.d_xyz(d);
//            superpoints[i].direc.round_xyz();
//        }
//        if(superpoints[i].direc.x==A2||superpoints[i].direc.y==A2||superpoints[i].direc.z==A2)
//        {
//            direction d(superpoints[i].direc.x*3,superpoints[i].direc.y*3,superpoints[i].direc.z*3);
//            superpoints[i].direc.d_xyz(d);
//            superpoints[i].direc.round_xyz();
//        }

        int d=10;
        int count=0;

        for(int j=0;j<13;++j)
        {
            for(int k=0;k<d;++k)
            {
                double tmpx=superpoints[i].x+k*dire[j].x;
                double tmpy=superpoints[i].y+k*dire[j].y;
                double tmpz=superpoints[i].z+k*dire[j].z;
                if(tmpx>sz[0]-1||tmpx<0||tmpy>sz[1]-1||tmpy<0||tmpz>sz[2]-1||tmpz<0)
                    continue;
                count++;
                intens[i].at(j*2+0)+=image[tmpx].at(tmpy).at(tmpz);
            }
            intens[i].at(j*2+0)/=count;
            count=0;
            for(int k=0;k<d;++k)
            {
                dire[j].negative();
                double tmpx=superpoints[i].x+k*dire[j].x;
                double tmpy=superpoints[i].y+k*dire[j].y;
                double tmpz=superpoints[i].z+k*dire[j].z;
                if(tmpx>sz[0]-1||tmpx<0||tmpy>sz[1]-1||tmpy<0||tmpz>sz[2]-1||tmpz<0)
                    continue;
                count++;
                intens[i].at(j*2+1)+=image[tmpx].at(tmpy).at(tmpz);
            }
            intens[i].at(j*2+1)/=count;
        }

    }

    for(int i=0;i<s_size;++i)
    {
        double max,min;

        bool flag=true;

        flag=false;
        for(int j=0;j<13;++j)
        {
            if(intens[i].at(j*2+0)>intens[i].at(j*2+1))
            {
                max=intens[i].at(j*2+0);
                min=intens[i].at(j*2+1);
            }
            else
            {
                max=intens[i].at(j*2+1);
                min=intens[i].at(j*2+0);
            }

            cout<<"FOR "<<i<<": "<<dire[j].x<<" "<<dire[j].y<<" "<<dire[j].z<<" "<<"max: "<<max<<" min: "<<min<<endl;
//            cout<<"direction: "<<superpoints[i].direc.x<<" "<<superpoints[i].direc.y<<" "<<superpoints[i].direc.z<<endl;

            if(max>15)
            {
                flag=true;
            }
        }

        if(flag==true)
            tips.push_back(superpoints[i]);

    }

    return true;
}


bool simplePoint::getNbSimplePoint(vector<simplePoint> &nbsimplepoints, int mode)
{
    for(int i=-1;i<=1;++i)
        for(int j=-1;j<=1;++j)
            for(int k=-1;k<=1;++k)
            {
                int dvalue=abs(i)+abs(j)+abs(z);
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

double assemblePoint::getBackIntensity(vector<vector<vector<unsigned char> > > &image, long long *sz)
{
    int d=1;
    double back_intensity=0;
    int c=0;
    for(int i=floor(-dx0-d);i<=ceil(dx0+d);++i)
        for(int j=floor(-dy0-d);j<=ceil(dy0+d);++j)
            for(int k=floor(-dz0-d);k<=ceil(dz0+d);++k)
            {
                double tmpx=round(x)+i;
                double tmpy=round(y)+j;
                double tmpz=round(z)+k;
                if(tmpx>sz[0]-1||tmpx<0||tmpy>sz[1]-1||tmpy<0||tmpz>sz[2]-1||tmpz<0)
                    continue;
                back_intensity+=(double)image[tmpx].at(tmpy).at(tmpz);
                c++;
            }
    return back_intensity/c;
}

bool assemblePoint::assemble(vector<vector<vector<unsigned char> > > &image, vector<vector<vector<int> > > &mask, long long *sz)
{
    int mode=2;

    int max_x,max_y,max_z;
    max_x=max_y=10;
    max_z=5;
    V3DLONG b_x0=x-max_x;
    V3DLONG b_x1=x+max_x;
    V3DLONG b_y0=y-max_y;
    V3DLONG b_y1=y+max_y;
    V3DLONG b_z0=z-max_z;
    V3DLONG b_z1=z+max_z;

    vector<simplePoint> queue;
    for(int i=0;i<this->sps.size();++i)
    {
        queue.push_back(this->sps[i]);
    }

    while(!queue.empty())
    {
        simplePoint tmp_p=queue.front();
        queue.erase(queue.begin());

        vector<simplePoint> nbsimplepoints;
        tmp_p.getNbSimplePoint(nbsimplepoints,mode);
        for(int i=0;i<nbsimplepoints.size();++i)
        {
            V3DLONG x=nbsimplepoints[i].x;
            V3DLONG y=nbsimplepoints[i].y;
            V3DLONG z=nbsimplepoints[i].z;
            if(x>=b_x0&&x<=b_x1&&y>=b_y0&&y<=b_y1&&z>=b_z0&&z<=b_z1
                    &&x>=0&&x<=sz[0]-1&&y>=0&&y<=sz[1]-1&&z>=0&&z<=sz[2]-1
                &&(mask[x].at(y).at(z)==0))
            {
                queue.push_back(nbsimplepoints[i]);
                this->sps.push_back(nbsimplepoints[i]);
                this->size++;
                double dx=this->x-x;
                double dy=this->y-y;
                double dz=this->z-z;
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
                mask[x].at(y).at(z)=1;
            }
        }
    }

    this->intensity=this->getIntensity(image);
    this->back_intensity=this->getBackIntensity(image,sz);
    this->rate=this->intensity/this->back_intensity;

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

    cout<<"average: "<<average_m<<endl;

    for(int i=0;i<this->sps.size();++i)
    {
        m=(double)image[this->sps[i].x].at(this->sps[i].y).at(this->sps[i].z);//-average_m;
        sum_m+=m;
        x_new+=m*this->sps[i].x;
        y_new+=m*this->sps[i].y;
        z_new+=m*this->sps[i].z;
    }

    cout<<"sum: "<<sum_m<<endl;

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

    cout<<"x: "<<x_new<<" y: "<<y_new<<" z: "<<z_new<<endl;

    return true;
}

bool apTracer::initialAsseblePoint(vector<assemblePoint> &assemblepoints, vector<vector<vector<unsigned char> > > &image, long long *sz, double thres)
{
    int mode=2;
    vector<vector<vector<int> > > mask=vector<vector<vector<int> > >(sz[0],vector<vector<int> >(sz[1],vector<int>(sz[2],0)));

    for(V3DLONG i=0;i<sz[0];++i)
        for(V3DLONG j=0;j<sz[1];++j)
            for(V3DLONG k=0;k<sz[2];++k)
            {
                if(image[i].at(j).at(k)<thres)
                    mask[i].at(j).at(k)=1;
            }

    vector<assemblePoint> iniassemblpoints;

    cout<<"first..."<<endl;

    for(V3DLONG i=1;i<sz[0]-1;++i)
        for(V3DLONG j=1;j<sz[1]-1;++j)
            for(V3DLONG k=1;k<sz[2]-1;++k)
            {
                if(mask[i].at(j).at(k)==0)
                {
                    simplePoint p(i,j,k);
                    assemblePoint a_p;
                    a_p.sps.push_back(p);
                    p.getNbSimplePoint(a_p.sps,mode);
                    a_p.x=i;
                    a_p.y=j;
                    a_p.z=k;
                    a_p.size=27;
                    a_p.dx0=1;
                    a_p.dx1=1;
                    a_p.dy0=1;
                    a_p.dy1=1;
                    a_p.dz0=1;
                    a_p.dz1=1;
                    a_p.intensity=a_p.getIntensity(image);
                    a_p.back_intensity=a_p.getBackIntensity(image,sz);
                    a_p.rate=a_p.intensity/a_p.back_intensity;
                    iniassemblpoints.push_back(a_p);
                }
            }

    cout<<"end first..."<<endl;

    multimap<double,assemblePoint> ratemaps;
    for(int i=0;i<iniassemblpoints.size();++i)
    {
        ratemaps.insert(pair<double,assemblePoint>(iniassemblpoints[i].rate,iniassemblpoints[i]));
    }

    cout<<"end map..."<<endl;

    multimap<double,assemblePoint>::reverse_iterator it=ratemaps.rbegin();

    int count=0;

    while(it!=ratemaps.rend())
    {

//        cout<<"count: "<<count<<endl;
        count++;
        assemblePoint tmp=it->second;

        bool e=false;
//        for(int i=0;i<tmp.sps.size();++i)
//        {
//            if(mask[tmp.sps[i].x].at(tmp.sps[i].y).at(tmp.sps[i].z)==1)
//                e=true;
//        }
        if(mask[tmp.x].at(tmp.y).at(tmp.z))
            e=true;

        if(e==true)
        {
            it++;
            continue;
        }

        for(int i=0;i<tmp.sps.size();++i)
        {
            mask[tmp.sps[i].x].at(tmp.sps[i].y).at(tmp.sps[i].z)=1;
        }

        cout<<"end if..."<<endl;
        tmp.assemble(image,mask,sz);


        if(tmp.intensity>30)
        {
            assemblepoints.push_back(tmp);
        }





        it++;
    }

    for(int i=0;i<assemblepoints.size();++i)
    {
        assemblepoints[i].renewXYZ(image);
        cout<<i<<" size: "<<assemblepoints[i].size<<" intensity: "<<assemblepoints[i].intensity<<" back_intensity: "<<assemblepoints[i].back_intensity<<" rate: "<<assemblepoints[i].rate<<endl;
    }
    for(int i=0;i<assemblepoints.size();++i)
    {
        assemblepoints[i].getDirection(image,sz);
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

bool assemblePoint::getDirection(vector<vector<vector<unsigned char> > > &image, long long *sz)
{
    V3DLONG ox = round(this->x);
    V3DLONG oy = round(this->y);
    V3DLONG oz = round(this->z);
    vector<double> intens=vector<double>(26,0);
    direction dire[26] = {direction(1,0,0),direction(0,1,0),direction(0,0,1),direction(1,1,0),direction(1,0,1),direction(0,1,1),direction(1,-1,0),direction(1,0,-1),direction(0,1,-1),direction(1,1,1),direction(-1,1,1),direction(1,-1,1),direction(1,1,-1)
                       ,direction(-1,0,0),direction(0,-1,0),direction(0,0,-1),direction(-1,-1,0),direction(-1,0,-1),direction(0,-1,-1),direction(-1,1,0),direction(-1,0,1),direction(0,-1,1),direction(-1,-1,-1),direction(1,-1,-1),direction(-1,1,-1),direction(-1,-1,1)};
    direction realdire[26] = {direction(1,0,0),direction(0,1,0),direction(0,0,1),direction(A1,A1,0),direction(A1,0,A1),direction(0,A1,A1),direction(A1,-A1,0),direction(A1,0,-A1),direction(0,A1,-A1),direction(A2,A2,A2),direction(-A2,A2,A2),direction(A2,-A2,A2),direction(A2,A2,-A2)
                       ,direction(-1,0,0),direction(0,-1,0),direction(0,0,-1),direction(-A1,-A1,0),direction(-A1,0,-A1),direction(0,-A1,-A1),direction(-A1,A1,0),direction(-A1,0,A1),direction(0,-A1,A1),direction(-A2,-A2,-A2),direction(A2,-A2,-A2),direction(-A2,A2,-A2),direction(-A2,-A2,A2)};

    int d = 20,count = 0;

    for(int i = 0; i<26; ++i)
    {
        for(int j = 0; j<d; ++j)
        {
            V3DLONG tmpx = ox + j*dire[i].x;
            V3DLONG tmpy = oy + j*dire[i].y;
            V3DLONG tmpz = oz + j*dire[i].z;
            if(tmpx>sz[0]-1||tmpx<0||tmpy>sz[1]-1||tmpy<0||tmpz>sz[2]-1||tmpz<0)
                continue;
            count++;
            intens.at(i)+=image[tmpx].at(tmpy).at(tmpz);
        }
        intens.at(i)/=count;
        count=0;
    }
    ofstream out;

    for(int i = 0; i<26; ++i)
    {
        cout<<i<<" : "<<intens[i]<<" ";
    }
    cout<<endl;
    return true;

}

bool apTracer::aptrace(vector<assemblePoint> &assemblePoints, vector<vector<vector<unsigned char> > > &image, NeuronTree &nt, long long *sz)
{
    for(int i=0;i<assemblePoints.size();++i)
    {
        qDebug()<<"----------------------------------------------------"<<endl;
        qDebug()<<i<<" : "<<endl;
        map<double,int> dismap;
        vector<double> mean_intens = vector<double>(assemblePoints.size(),0);
        for(int j=0;j<assemblePoints.size();++j)
        {
            assemblePoint& ori = assemblePoints[i];
            assemblePoint& chi = assemblePoints[j];
            if(i!=j)
            {
                qDebug()<<j<<" : "<<endl;
                double d = distance_two_point(ori,chi);
                qDebug()<<"distance: "<<d<<endl;
                dismap[d] = j;
                int midx1 = static_cast<int>(ori.x+(chi.x-ori.x)/4);
                int midy1 = static_cast<int>(ori.y+(chi.y-ori.y)/4);
                int midz1 = static_cast<int>(ori.z+(chi.z-ori.z)/4);
                simplePoint p1(midx1,midy1,midz1);
                assemblePoint a_p1;
                a_p1.sps.push_back(p1);
//                p1.getNbSimplePoint(a_p1.sps,2);
                a_p1.x = midx1;
                a_p1.y = midy1;
                a_p1.z = midz1;
                a_p1.size = 1;
                a_p1.intensity = a_p1.getIntensity(image);

                int midx2 = static_cast<int>(ori.x+((chi.x-ori.x)/4)*2);
                int midy2 = static_cast<int>(ori.y+((chi.y-ori.y)/4)*2);
                int midz2 = static_cast<int>(ori.z+((chi.z-ori.z)/4)*2);
                simplePoint p2(midx2,midy2,midz2);
                assemblePoint a_p2;
                a_p2.sps.push_back(p2);
//                p2.getNbSimplePoint(a_p2.sps,2);
                a_p2.x = midx2;
                a_p2.y = midy2;
                a_p2.z = midz2;
                a_p2.size = 1;
                a_p2.intensity = a_p2.getIntensity(image);

                int midx3 = static_cast<int>(ori.x+((chi.x-ori.x)/4)*3);
                int midy3 = static_cast<int>(ori.y+((chi.y-ori.y)/4)*3);
                int midz3 = static_cast<int>(ori.z+((chi.z-ori.z)/4)*3);
                simplePoint p3(midx3,midy3,midz3);
                assemblePoint a_p3;
                a_p3.sps.push_back(p3);
//                p3.getNbSimplePoint(a_p3.sps,2);
                a_p3.x = midx3;
                a_p3.y = midy3;
                a_p3.z = midz3;
                a_p3.size = 1;
                a_p3.intensity = a_p3.getIntensity(image);

                qDebug()<<"origin intensity: "<<ori.intensity
                       <<"mid1 intensity: "<<a_p1.intensity
                      <<"mid2 intensity: "<<a_p2.intensity
                     <<"mid3 intensity: "<<a_p3.intensity
                    <<"child intensity: "<<chi.intensity<<endl;
                double intensity_mean =(ori.intensity+a_p1.intensity+a_p2.intensity+a_p3.intensity+chi.intensity)/5;
                mean_intens[j] =intensity_mean;
            }
        }

        map<double,int>::iterator it = dismap.begin();
        int count = 0;
        while(it!=dismap.end())
        {
            qDebug()<<"------------------------------------------------"<<endl;
            qDebug()<<"distance: "<<it->first<<endl;
            int index = it->second;
            assemblePoint& ori = assemblePoints[i];
            assemblePoint& chi = assemblePoints[index];
            double thres_intensity = (ori.intensity+chi.intensity)/2.1;
            if(mean_intens[index]<thres_intensity)
                it++;
            else
            {
                count++;
                if(count==1)
                {
                    qDebug()<<"count: "<<count<<endl;
                    qDebug()<<"mean: "<<mean_intens[index]<<" ori intensity: "<<ori.intensity<<" chi intensity: "<<chi.intensity<<endl;
                    NeuronSWC par,child;
                    par.n = i*3;
                    par.x = ori.x;
                    par.y = ori.y;
                    par.z = ori.z;
                    par.parent = -1;
                    child.n = i*3+1;
                    child.x = chi.x;
                    child.y = chi.y;
                    child.z = chi.z;
                    child.parent = par.n;
                    nt.listNeuron.push_back(par);
                    nt.listNeuron.push_back(child);
                    it++;
                    break;
                }
//                else if(count==2)
//                {
//                    qDebug()<<"count: "<<count<<endl;
//                    qDebug()<<"mean: "<<mean_intens[index]<<" ori intensity: "<<ori.intensity<<" chi intensity: "<<chi.intensity<<endl;
//                    NeuronSWC child2;
//                    child2.n = i*3+2;
//                    child2.x = chi.x;
//                    child2.y = chi.y;
//                    child2.z = chi.z;
//                    child2.parent = i*3;
//                    nt.listNeuron.push_back(child2);
//                    break;
//                }
             }
        }

    }
    return true;
}
























