#include "tip_rayshooting.h"
#include "v3d_message.h"
#include "stackutil.h"
#include <iostream>
#include <cmath>

using namespace std;

#define PI 3.1415926

float square(float x){return x*x;}

void ray_shooting(int m, int n,vector<vector<float>> ray_x,vector<vector<float>> ray_y)
	{

		int num = n-1;
		float ang = 2*PI/m;
		float x_dis, y_dis;

		for(int i = 0; i < m; i++)
			{
				x_dis = cos(ang*(i+1));
				y_dis = sin(ang*(i+1));
			for(int j = 0; j<n; j++)
				{
					ray_x[i][j] = x_dis*(j+1);
					ray_y[i][j] = y_dis*(j+1);
				}
			}
	}
 
void rayinten_2D(int point_x,int point_y,int point_z,int m,int n,int threshold, vector<vector<float>> ray_x,vector<vector<float>> ray_y, Image4DSimple *p4DImage, int &count, float &max_ang)
{
	int point[3] = {point_x,point_y,point_z};
	count = max_ang = 0;  //初始化输出，因为这里使用的引用，不初始化的话会变成累加
	if(point[2] < 0||point[2] >= p4DImage->getZDim())
		return;
	int  point_coordinate[3];
	float pixe = 0.0;
	vector<int> indd(m),ind1(m);
	

	for(int i = 0; i < m; i++)
		{ 
		for(int j = 0; j<n; j++)
			{
				 pixe = 0.0; //为2D的ray像素点赋初值
				//if((point[0]+ray_x[i][j] >= 0) && (point[1]+ray_y[i][j]) >= 0 && (point[0]+ray_x[i][j]) < p4DImage->getXDim() &&  (point[1]+ray_y[i][j]) < p4DImage->getYDim())
				{  
					//pixe = p4DImage->getValueUINT8(point[0]+ray_x[i][j], point[1]+ray_y[i][j], point[2], 0);
					pixe = interp_2d(point[0]+ray_x[i][j], point[1]+ray_y[i][j], point[2],p4DImage);
					if(pixe > threshold)
						ind1[i]++;
				}
			}
		}

	
	for (int i = 0;i < m;i++)
	{
		if (ind1[i] >= n/2)
			{
				indd[count] = i;
				count++;
			}
	}
	
	float dis=0,max_dis=0; 
	if (count > 1)
		{
			for (int i = 0;i < count - 1;i++)
			{
				for (int j = i+1;j < count;j++)
				{
						dis = min(indd[j] - indd[i],m-(indd[j] - indd[i]));
					if (dis > max_dis)
						max_dis=dis;
				}
			}

			max_ang = max_dis / m;
		}
	else if (count == 1)
		max_ang = 1/m;
	else 
		{
			count = 0;
			max_ang = 0;
		}		
}

void ray_shooting_3D(float ray_X[512][8], float ray_Y[512][8], float ray_Z[512][8])
	{	

		for(int len = 1; len <= 8; len++)
		{
			int mm = 0;	
			for(int n = 1; n <= 16; n++ )
				for(int k = 0; k <=31; k++ )
					{
						{
							ray_X[mm][len] = len * sin(PI * n/16) * cos(PI * k / 16);
							ray_Y[mm][len] = len * sin(PI * n/16) * sin(PI * k / 16);
							ray_Z[mm][len] = len * cos(PI * n/16);
							mm++;
						}
					}
				}
		}

void rayinten_3D(int point[3], float rayintensity[512][8], float ray_X[512][8], float ray_Y[512][8], float ray_Z[512][8], Image4DSimple *p4DImage, int &count, float &max_ang)
	{
		count = max_ang = 0;
		if(point[2] < 0||point[2] >= p4DImage->getZDim())
			return;

		int pixe, ind1[512] = {0};
		
		for (int i = 0; i < 512; i++)
			for (int j = 0; j < 8; j ++)
			{
				{
					pixe=0;
					if((point[0]+ray_X[i][j] >= 0) && ((point[1]+ray_Y[i][j]) >= 0) && ((point[0]+ray_X[i][j]) < p4DImage->getXDim()) &&  ((point[1]+ray_Y[i][j]) < p4DImage->getYDim()) 
						 && ((point[2]+ray_Z[i][j]) >= 0) &&  (point[2]+ray_Z[i][j]) < p4DImage->getZDim())
					{
						pixe = p4DImage->getValueUINT8(point[0] + ray_X[i][j], point[1] + ray_Y[i][j], point[2] + ray_Z[i][j],0);
						if(pixe > 255*0.18)
						ind1[i]++;
					}
				}
			}

		for (int i = 0; i < 512; i++)
		{
			if (ind1[i] > 8/2)
			{
				ind1[count] = i;
				count++;
			}
	}

		float dis;
		int ind1_i, ind1_j;
		if (count > 1)
		{
			for (int i = 0; i < count; i++)
				for ( int j = i+1; j <count; j++)
				{
					{
						ind1_i = ind1[i];
						ind1_j = ind1[j];
						dis = (ray_X[ind1_i][0]*ray_X[ind1_j][0] + ray_Y[ind1_i][0]*ray_Y[ind1_j][0] + ray_Z[ind1_i][0]*ray_Z[ind1_j][0])/
							sqrt(ray_X[ind1_i][0]*ray_X[ind1_i][0] + ray_Y[ind1_i][0]*ray_Y[ind1_i][0] + ray_Z[ind1_i][0]*ray_Z[i][0]) * sqrt(ray_X[ind1_j][0]*ray_X[ind1_j][0] + ray_Y[ind1_j][0]*ray_Y[ind1_j][0] + ray_Z[ind1_j][0]*ray_Z[ind1_j][0]);
						//dis = acos(dis) / PI * 180;
						if (dis < max_ang)
						{
							max_ang = dis;
						}
					}
				}
		}
		else
		{
			max_ang = 180;
		}

	}


float interp_2d(float point_x,float point_y,int point_z, Image4DSimple *p4DImage)
{
	float result;

	if(point_x < 0||point_y < 0||point_x > p4DImage->getXDim()-1||point_y > p4DImage->getYDim()-1)
		return 0.0;
	else if(point_x < 1||point_y < 1||point_x > p4DImage->getXDim()-2||point_y > p4DImage->getYDim()-2)
	{	
		result = p4DImage->getValueUINT8(point_x,point_y,point_z,0);
		return result;
	}
	else
	{
		int y1 = p4DImage->getValueUINT8(ceil(point_x), ceil(point_y), point_z,0);
		int y0 = p4DImage->getValueUINT8(floor(point_x), ceil(point_y), point_z,0);
		int x1 = p4DImage->getValueUINT8(ceil(point_x), floor(point_y), point_z,0);
		int x0 = p4DImage->getValueUINT8(floor(point_x), floor(point_y), point_z,0);

		result = x0*(point_x-floor(point_x))*(point_y-floor(point_y))+x1*(ceil(point_x)-point_x)*(point_y-floor(point_y))
			+y0*(point_x-floor(point_x))*(ceil(point_y)-point_y)+y1*(ceil(point_x)-point_x)*(ceil(point_y)-point_y);
		return result;
	}
}

bool get_slice_flag(int i,int j,int k,int nembers_2d,int length_2d,double angle_2d,double count_2d_thre,int slice_number,int threshold, vector<vector<float>> ray_x,vector<vector<float>> ray_y, Image4DSimple *p4DImage, int &count_2d, float &max_ang_2d)
{
	max_ang_2d = 0.0;
	count_2d = 0;

	float   max_ang_2d_slice1,max_ang_2d_slice2;
	int count_2d_slice1,count_2d_slice2;

	rayinten_2D(i,j,k, nembers_2d ,length_2d,threshold, ray_x, ray_y, p4DImage, count_2d, max_ang_2d);

	for(int slice = 1; slice<= slice_number;slice++)
	{

		rayinten_2D(i,j,k - slice, nembers_2d ,length_2d,threshold, ray_x, ray_y, p4DImage, count_2d_slice1, max_ang_2d_slice1);
		rayinten_2D(i,j,k + slice, nembers_2d ,length_2d,threshold, ray_x, ray_y, p4DImage, count_2d_slice2, max_ang_2d_slice2);
		if(count_2d_slice1 < count_2d_thre*nembers_2d&&max_ang_2d_slice1*360 <= angle_2d&&count_2d_slice2 < count_2d_thre*nembers_2d&&max_ang_2d_slice2*360 <= angle_2d)
			continue;
		else
			return false;

	}

	return true;
}

int tipdetection(PARA_APP2 & p)
{
	
	Image4DSimple *p4DImage = p.p4dImage;
	p.landmarks.clear();

		// 2 - Ask for parameters
	//     asking for the subject channel

	int numbers_2d = 64,thres_2d = 45,length_2d = 8,angle_2d = 90,slice_number = 3;
	int count_2d_thre = 33;

	//set update the dialog
    QDialog * dialog = new QDialog();

	//自适应感知输入图像的类型
	if(p4DImage->getZDim() > 1)
		dialog->setWindowTitle("3D neuron image tip point detection Based on Ray-shooting algorithm");
	else
		dialog->setWindowTitle("2D neuron image tip point detection Based on Ray-shooting algorithm");

	QGridLayout * layout = new QGridLayout();

	QSpinBox * numbers_2d_spinbox = new QSpinBox();
    numbers_2d_spinbox->setRange(0,4096);
    numbers_2d_spinbox->setValue(numbers_2d);
            
    QSpinBox * thres_2d_spinbox = new QSpinBox();
    thres_2d_spinbox->setRange(-1, 255);
    thres_2d_spinbox->setValue(thres_2d);
 
    QSpinBox * length_2d_spinbox = new QSpinBox();
    length_2d_spinbox->setRange(0,p4DImage->getXDim());
    length_2d_spinbox->setValue(length_2d);

	QSpinBox * angle_2d_spinbox = new QSpinBox();
    angle_2d_spinbox->setRange(0,180);
    angle_2d_spinbox->setValue(angle_2d);

	QSpinBox * count_2d_spinbox = new QSpinBox();
    count_2d_spinbox->setRange(0,4096);
    count_2d_spinbox->setValue(count_2d_thre);

	if(p4DImage->getZDim() > 1)
	{
		QSpinBox * slice_number_spinbox = new QSpinBox();
		slice_number_spinbox->setRange(0, p4DImage->getZDim()/2);
		slice_number_spinbox->setValue(slice_number);

		layout->addWidget(new QLabel("slice number"),5,0);
		layout->addWidget(slice_number_spinbox, 5,1,1,5);

		slice_number = slice_number_spinbox->value();
	}

    layout->addWidget(new QLabel("ray numbers"),0,0);
    layout->addWidget(numbers_2d_spinbox, 0,1,1,5);

    layout->addWidget(new QLabel("intensity threshold"),1,0);
    layout->addWidget(thres_2d_spinbox, 1,1,1,5);

    layout->addWidget(new QLabel("ray length"),2,0);
    layout->addWidget(length_2d_spinbox, 2,1,1,5);

	layout->addWidget(new QLabel("angle threshold"),3,0);
    layout->addWidget(angle_2d_spinbox, 3,1,1,5);
    
	layout->addWidget(new QLabel("ray numbers threshold"),4,0);
    layout->addWidget(count_2d_spinbox, 4,1,1,5);

	QHBoxLayout * hbox2 = new QHBoxLayout();
    QPushButton * ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton * cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

	layout->addLayout(hbox2,6,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

	//run the dialog

	
    if(dialog->exec() != QDialog::Accepted)
	{
		if (dialog) 
		{
			delete dialog;
			dialog=0;
			cout<<"delete dialog"<<endl;
		}
		return -1;
	}
      

    //get the dialog return values
    numbers_2d = numbers_2d_spinbox->value();
    thres_2d = thres_2d_spinbox->value();
    length_2d = length_2d_spinbox->value();
    angle_2d = angle_2d_spinbox->value();
	count_2d_thre = count_2d_spinbox->value();



    if (dialog) 
	{
		delete dialog;
		dialog=0;
		cout<<"delete dialog"<<endl;
	}

	
	V3DLONG sz[3];
	sz[0] = p4DImage->getXDim();
	sz[1] = p4DImage->getYDim();
	sz[2] = p4DImage->getZDim();

	vector<vector<float> > ray_x(numbers_2d,vector<float>(length_2d)), ray_y(numbers_2d,vector<float>(length_2d));

	//float **ray_x,**ray_y;
	/*
	float** ray_x = (float**)new float(numbers_2d);
	float** ray_y = (float**)new float(numbers_2d);

	for(int i = 0;i<numbers_2d;i++)
	{
		ray_x[i] = new float(length_2d);
		ray_y[i] = new float(length_2d);
	}
	*/

	float   max_ang_2d,max_ang_3d,rayintensity[512][8], ray_X[512][8], ray_Y[512][8], ray_Z[512][8];
	int count_2d,count_3d;

	cout<<"begin"<<endl;

	cout<<"create 2D_ray"<<endl;
	//ray_shooting(numbers_2d ,length_2d,ray_x,ray_y);
	float ang = 2*PI/numbers_2d;
	float x_dis, y_dis;

	for(int i = 0; i < numbers_2d; i++)
	{
		x_dis = cos(ang*(i+1));
		y_dis = sin(ang*(i+1));
		for(int j = 0; j<length_2d; j++)
			{
				ray_x[i][j] = x_dis*(j+1);
				ray_y[i][j] = y_dis*(j+1);
			}
	}

	cout<<"create 2D_ray success"<<endl;

	/*
	cout<<"create 3D_ray"<<endl;
	ray_shooting_3D(ray_X, ray_Y, ray_Z);
	cout<<"create 3D_ray success"<<endl;
	*/


	cout<<"begin cycle"<<endl;

	LocationSimple s;

	vector<double> angle;

	for(long k=0;k<sz[2];k++)
	{
		for(long i=0;i<sz[0];i++)
		{
			for(long j=0;j<sz[1];j++)
			{
				int pie=p4DImage->getValueUINT8(i,j,k,0);   //这里千万注意getValueUINT8返回的是一个char变量，要转换为它所对应的ASCII码
			
				if(pie > 20&&pie < 80)
					{
						int point[3]={i,j,k};

						bool flag = get_slice_flag(i,j,k, numbers_2d ,length_2d,angle_2d,count_2d_thre,slice_number,thres_2d,ray_x, ray_y, p4DImage, count_2d, max_ang_2d);

						if (flag&&count_2d <= count_2d_thre*numbers_2d&&(count_2d > 0.08*numbers_2d)&&(max_ang_2d*360 < angle_2d))
						{
					//		rayinten_3D(point, rayintensity, ray_X, ray_Y, ray_Z, p4DImage, count_3d, max_ang_3d);
					//		if (count_3d < 192 && count_3d > 10 && max_ang_3d > -0.766 && max_ang_3d < 1)
						//	candidate_tip.push_back({i,j,k});
							{
								s.x = i + 1;  //注意这里需要给坐标加一，因为V3D坐标是从1开始而不是0
								s.y = j + 1;
								s.z = k + 1;
								s.radius = 1;
								s.color = random_rgba8(255);//为标记点的随机分配颜色
								p.landmarks << s;
							}
							}

						}
					}
			}
		}

	for(int i=0;i<p.landmarks.size();i++)
		for(int j=i+1;j<p.landmarks.size();j++)
	{
	
		{
			if(square(p.landmarks[j].x-p.landmarks[i].x)+square(p.landmarks[j].y-p.landmarks[i].y)+square(p.landmarks[j].z-p.landmarks[i].z)<150)
				{
					p.landmarks.removeAt(j);
					j = j - 1;
				}
		}
	}
	
	if(p.landmarks.empty())
		return 0;

	return 1;
}