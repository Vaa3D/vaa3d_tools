#include "trans_class.h"

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

























