/* Tip_Detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-3-15 : by ChaoWang
 */
#include "ray_shooting.h"
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include "Tip_Detection_plugin.h"
#include <iostream>
#define PI 3.1415926
Q_EXPORT_PLUGIN2(Tip_Detection, Tracing);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

void reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
 
QStringList Tracing::menulist() const
{
	return QStringList() 
		<<tr("tip_detection")
		<<tr("about");
}

QStringList Tracing::funclist() const
{
	return QStringList()
		<<tr("tip_detection")
		<<tr("help");
}

void Tracing::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tip_detection"))
	{    
        int flag = tipdetection(callback,parent);
        if(flag != -1)
        v3d_msg(tr("tip detection completed!"));

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by ChaoWang, 2018-3-15"));
	}
}

bool Tracing::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("tip_detection"))
	{
		return true;
	}
	else if (func_name == tr("help"))
	{
		printHelp();
		return true;
	}
	
}

int tipdetection(V3DPluginCallback2 &callback, QWidget *parent)
{
	// 1 - Obtain the current 4D image pointer
	v3dhandle curwin = callback.currentImageWindow();
	if(!curwin)
	{
		v3d_msg("No image is open.");
		return -1;
	}
	Image4DSimple *p4DImage = callback.getImage(curwin);

		// 2 - Ask for parameters
	//     asking for the subject channel

	NeuronTree nt;

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
    numbers_2d_spinbox->setRange(1,4096);
    numbers_2d_spinbox->setValue(numbers_2d);
            
    QSpinBox * thres_2d_spinbox = new QSpinBox();
    thres_2d_spinbox->setRange(-1, 255);
    thres_2d_spinbox->setValue(thres_2d);
 
    QSpinBox * length_2d_spinbox = new QSpinBox();
    length_2d_spinbox->setRange(1,p4DImage->getXDim());
    length_2d_spinbox->setValue(length_2d);

	QSpinBox * angle_2d_spinbox = new QSpinBox();
    angle_2d_spinbox->setRange(1,180);
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

	/*
	bool ok;
	int numbers_2d = 1;
	numbers_2d = QInputDialog::getInteger(parent, "2D Ray-shooting Rays Number", "Set the rays number:", 64, 1,5000, 1, &ok);
	if (!ok) return -1;

	//      asking for threshold
	int thres_2d = 0;
	thres_2d = QInputDialog::getInteger(parent, "2D Threshold", "Set 2d rays threshold:", 45, 1, 255, 1, &ok);
	if (!ok) return -1;


		//      asking for 2d rays length
	int length_2d = 1;
	length_2d = QInputDialog::getInteger(parent, "2D  Ray-shooting Rays Length", "Set Rays Length:", 8, 1, 30, 1, &ok);
	if (!ok) return -1;

	
			//      asking for 2d angle threshold
	int angle_2d = 1;
	angle_2d = QInputDialog::getInteger(parent, "2D  Ray-shooting Rays Length", "Set 2d angle threshold:", 90, 1, 180, 1, &ok);
	if (!ok) return -1;
	cout<<"angle_2d"<<angle_2d<<endl;

			//      asking for neighbor slice number
	int slice_number = 0;
	if(p4DImage->getZDim() > 1)
	{
		slice_number = QInputDialog::getInteger(parent, "2D  Ray-shooting neighbor slice number", "Set 2d neighbor slice number:", 3, 0, p4DImage->getZDim()/2, 1, &ok);
		if (!ok) return -1;
		cout<<"slice_number"<<slice_number<<endl;
	}

				//   asking for 2d count threshold
	double count_2d_thre = 0;
	count_2d_thre = QInputDialog::getDouble(parent, "2D  Ray-shooting Count numbers", "Set 2d count threshold:",0.3, 0, 1, 1, &ok);
	if (!ok) return -1;
	cout<<"count_2d_thre"<<count_2d_thre<<endl;
	*/

	// 2 - get edge point
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


	cout<<"create 3D_ray"<<endl;
	ray_shooting_3D(ray_X, ray_Y, ray_Z);
	cout<<"create 3D_ray success"<<endl;

	/*
	while(1)
	{
		int testx,testy,testz,swap;
		testx = QInputDialog::getInteger(parent, "testx", "testx:", 212, 1, 512, 1, &ok)-1;
		testy = QInputDialog::getInteger(parent, "testy", "testy", 350, 1, 512, 1, &ok)-1;
		testz = QInputDialog::getInteger(parent, "testz", "testz", 20, 1, 512, 1, &ok)-1;

		swap = testy;
		testy = 511 - testx;
		testx = swap;

		cout<<endl;
		cout<<(int)p4DImage->getValueUINT8(testx,testy,testz,0)<<endl;

		int test_point[3] = {testx,testy,testz};

		rayinten_2D(testx,testy,testz, numbers_2d ,length_2d,thres_2d, ray_x, ray_y, p4DImage, count_2d, max_ang_2d);
		cout<<"count_2d"<<endl<<count_2d<<endl<<"max_ang_2d"<<endl<<max_ang_2d<<endl;
		
		get_slice_max(testx,testy,testz, numbers_2d ,length_2d,thres_2d, ray_x, ray_y, p4DImage, count_2d, max_ang_2d);
		cout<<"max_count_2d"<<endl<<count_2d<<endl<<"max_max_ang_2d"<<endl<<max_ang_2d<<endl;


		rayinten_3D(test_point, rayintensity, ray_X, ray_Y, ray_Z, p4DImage, count_3d, max_ang_3d);
		cout<<"count_3d"<<endl<<count_3d<<endl<<"max_ang_3d"<<endl<<max_ang_3d<<endl;
	}
	*/

	cout<<"begin cycle"<<endl;

	LandmarkList curlist;
	LocationSimple s;

    vector<vector<int> > candidate_tip;
	//vector<int> one_tip(3);
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
								curlist << s;
							}
							}

						}
					}
			}
		}

	for(int i=0;i<curlist.size();i++)
		for(int j=i+1;j<curlist.size();j++)
	{
	
		{
			if(square(curlist[j].x-curlist[i].x)+square(curlist[j].y-curlist[i].y)+square(curlist[j].z-curlist[i].z)<150)
				{
					curlist.removeAt(j);
					j = j - 1;
				}
		}
	}

    callback.setLandmark(curwin, curlist);


	return 1;
}


/* functions in DOFUNC takes 2 parameters
 * "input" arglist has 2 positions reserved for input and parameter:
 *            input.at(0).p returns a pointer to vector<char*> that pass the arglist following the input option '-i'
 *                                  items are splitted by ' ', which is often used as input data 
 *                                  [required]
 *            input.at(1).p returns a pointer to vector<char*> that pass the arglist following the input option '-p'. 
 *                                  items are splitted by ' ', it is reserved for you to define your own parameters 
 *				    [not required, if '-p' is not specified, input only contains one member]
 * "output" arglist has a size of 1:
 *            output.at(0).p returns a pointer to vector<char*> that pass the arglist following the input option '-o' 
 *                                  items are splitted by ' ', which is often used as output data 
 *                                  [required]
 */


void printHelp()
{
	cout<<"\nThis is a demo plugin to detect tip point in an image. by Keran Lin 2017-04"<<endl;
	cout<<"\nUsage: v3d -x <example_plugin_name> -f tip_detection -i <input_image_file> -o <output_image_file> -p <subject_color_channel> <threshold>"<<endl;
	cout<<"\t -i <input_image_file>                      input 3D image (tif, raw or lsm)"<<endl;
	cout<<"\t -o <output_image_file>                     output image of the thresholded subject channel"<<endl;
	cout<<"\t                                            the  paras must come in this order"<<endl;
	cout<<"\nDemo: v3d -x libexample_debug.dylib -f image_thresholding -i input.tif -o output.tif -p 0 100\n"<<endl;
	return;
}
