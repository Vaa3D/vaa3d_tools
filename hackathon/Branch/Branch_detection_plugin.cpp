/* Branch_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-5-20 : by Chao Wang
 */
#include <iostream>
#include "basic_surf_objs.h"
#include "v3d_message.h"
#include <vector>
#include "Branch_detection_plugin.h"
#include "new_ray-shooting.h"
#include "../zhi/APP2_large_scale/my_surf_objs.h"
#define PI 3.1415926
using namespace std;
Q_EXPORT_PLUGIN2(Branch_detection, TestPlugin)
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("Branch_detection")
        <<tr("creat_ray-shooting_model")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
        if (menu_name == tr("Branch_detection"))
       {
		v3d_msg("To be implemented.");
                int flag=branch_detection(callback,parent);
                if (flag==1)
                {
                    v3d_msg(tr("branch detection completed!"));
                    v3d_msg(tr("Chao Wang is 棒棒哒!"));
                 }
                 else
                {

                      v3d_msg(tr("branch detection not completed!"));
                 }


       }
        else if (menu_name == tr("creat_ray-shooting_model"))
        {
            int flag= raymodel(callback,parent);
            if(flag=1)
            v3d_msg("Chao wang ray-shooting model creat.");
        }

	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Chao Wang, 2018-5-20"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}
int branch_detection(V3DPluginCallback2 &callback, QWidget *parent)
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
                    // asking for the subject channel



            int ray_numbers_2d = 128;
            int thres_2d = 45;
            int ray_length_2d = 16;


            //set update the dialog
            QDialog * dialog = new QDialog();


            if(p4DImage->getZDim() > 1)
                    dialog->setWindowTitle("3D neuron image tip point detection Based on Ray-shooting algorithm");
            else
                    dialog->setWindowTitle("2D neuron image tip point detection Based on Ray-shooting algorithm");

        QGridLayout * layout = new QGridLayout();

        QSpinBox * ray_numbers_2d_spinbox = new QSpinBox();
        ray_numbers_2d_spinbox->setRange(1,1000);
        ray_numbers_2d_spinbox->setValue(ray_numbers_2d);

        QSpinBox * thres_2d_spinbox = new QSpinBox();
        thres_2d_spinbox->setRange(-1, 255);
        thres_2d_spinbox->setValue(thres_2d);

        QSpinBox * ray_length_2d_spinbox = new QSpinBox();
        ray_length_2d_spinbox->setRange(1,p4DImage->getXDim());
        ray_length_2d_spinbox->setValue(ray_length_2d);


        layout->addWidget(new QLabel("ray numbers"),0,0);
        layout->addWidget(ray_numbers_2d_spinbox, 0,1,1,5);

        layout->addWidget(new QLabel("intensity threshold"),1,0);
        layout->addWidget(thres_2d_spinbox, 1,1,1,5);

        layout->addWidget(new QLabel("ray length"),2,0);
        layout->addWidget(ray_length_2d_spinbox, 2,1,1,5);


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
        ray_numbers_2d = ray_numbers_2d_spinbox->value();
        thres_2d = thres_2d_spinbox->value();
        ray_length_2d = ray_length_2d_spinbox->value();


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

       unsigned char* datald=0;
       V3DLONG pagesz_mip = sz[0]*sz[1];

       datald = p4DImage->getRawData();


 //      cout<<"find 2D cureve points "<<endl;
 //      int slice=sz[2];
 //      for(long j=0;j<slice;j++)
 //          {
  //               for(long k=0;k<sz[0];k++)
 //                       for(long i=0;i<sz[1];i++) 在此处加上高曲率点检测代码


       vector<vector<float> > ray_x(ray_numbers_2d,vector<float>(ray_length_2d)), ray_y(ray_numbers_2d,vector<float>(ray_length_2d));
      // float  rayintensity[512][8], ray_X[512][16], ray_Y[512][16], ray_Z[512][16];
       //int count_2d;

       cout<<"begin"<<endl;

       cout<<"create 2D_ray"<<endl;

       float ang = 2*PI/ray_numbers_2d;
       float x_dis, y_dis;

       for(int i = 0; i < ray_numbers_2d; i++)
       {
           x_dis = cos(ang*(i+1));
           y_dis = sin(ang*(i+1));
           for(int j = 0; j<ray_length_2d; j++)
               {
                   ray_x[i][j] = x_dis*(j+1);
                   ray_y[i][j] = y_dis*(j+1);
               }
       }

       cout<<"create 2D_ray success"<<endl;



       cout<<"mip"<<endl;
       V3DLONG M=sz[1]; //y
       V3DLONG N=sz[0]; //x
       V3DLONG P=sz[2]; //z
       V3DLONG pagesz=M*N*P;
       v3d_msg(QString("sz[0] is %1,sz[1] is %2, sz[2] is %3").arg(N).arg(M).arg(P));
       unsigned char *image_mip=0;
       try{image_mip=new unsigned char [pagesz];}
       catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}
       for(V3DLONG iy = 0; iy < M; iy++)
       {
           V3DLONG offsetj = iy*N;
           for(V3DLONG ix = 0; ix < N; ix++)
           {
               int max_mip = 0;
               for(V3DLONG iz = 0; iz < P; iz++)
               {
                   V3DLONG offsetk = iz*M*N;
                   if(datald[offsetk + offsetj + ix] >= max_mip)
                   {
                       image_mip[iy*N + ix] = datald[offsetk + offsetj + ix];
                       max_mip = datald[offsetk + offsetj + ix];
                      // v3d_msg(QString("max_mip is %1").arg(max_mip));
                   }
               }
           }
       }
       cout<<"mip was complete "<<endl;

        cout<<"segment"<<endl;
        unsigned char *image_binary=0;
        try{image_binary=new unsigned char [pagesz];}
        catch(...) {v3d_msg("cannot allocate memory for image_binary."); return 0;}
        for(V3DLONG i = 0; i < pagesz_mip; i++)
        {
            if(image_mip[i] > thres_2d)
                image_binary[i] = 255;
            else
                image_binary[i] = 0;
        }
        cout<<"segment was complete "<<endl;


        cout<<"begin circle "<<endl;

        LandmarkList curlist;
        LocationSimple s;


       for(V3DLONG k=0;k<sz[0];k++)
       {
           for(V3DLONG i=0;i<sz[1];i++)
           {

                   int project_value=get_2D_ValueUINT8(k, i,image_binary,sz[0],sz[1]);
                   //v3d_msg(QString("project_value is %1").arg(project_value));

                   if(project_value > thres_2d)
                       {

                           bool flag=rayinten_2D(k,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, image_binary,sz[0],sz[1]);
                           //v3d_msg(QString("flag is %1").arg(flag));
                           if (flag==true)
                           {


                               {
                                   s.x = k + 1;
                                   s.y = i + 1;
                                   s.z = (sz[2]/2);  // because this is 2D branch detection , z轴坐标位于中间
                                   s.radius = 1;
                                   s.color = random_rgba8(255);
                                   curlist << s;
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
       //v3d_msg(QString("project_value is %1").arg(project_value));
       callback.setLandmark(curwin, curlist);

       if(image_binary) {delete []image_binary; image_binary = 0;}
       if(image_mip) {delete []image_mip; image_mip = 0;}

       return 1;
   }

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

int raymodel(V3DPluginCallback2 &callback, QWidget *parent)
{
    // 1 - Obtain the current 4D image pointer
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No image is open.");
        return -1;
    }
    Image4DSimple *p4DImage = callback.getImage(curwin);

    V3DLONG sz[3];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();


    float x_point=0,y_point=0,z_point=0;
    int ray_numble=64;
    int length_numble=8;

    QDialog * dialog = new QDialog();


    QGridLayout * layout = new QGridLayout();

    QSpinBox * X_point_spinbox = new QSpinBox();
    X_point_spinbox->setRange(1,sz[0]);
    X_point_spinbox->setValue(x_point);

    QSpinBox * Y_point_spinbox = new QSpinBox();
    Y_point_spinbox->setRange(1, sz[1]);
    Y_point_spinbox->setValue(y_point);

    QSpinBox * Z_point_spinbox = new QSpinBox();
    Z_point_spinbox->setRange(1,sz[2]);
    Z_point_spinbox->setValue(z_point);

    QSpinBox * number_ray_spinbox = new QSpinBox();
    number_ray_spinbox->setRange(1,1000);
    number_ray_spinbox->setValue(ray_numble);

    QSpinBox * length_ray_spinbox = new QSpinBox();
    length_ray_spinbox->setRange(1,100);
    length_ray_spinbox->setValue(length_numble);

//    if(p4DImage->getZDim() > 1)
//    {
//        QSpinBox * slice_number_spinbox = new QSpinBox();
//        slice_number_spinbox->setRange(0, p4DImage->getZDim()/2);
//        slice_number_spinbox->setValue(slice_number);

//        layout->addWidget(new QLabel("slice number"),5,0);
//        layout->addWidget(slice_number_spinbox, 5,1,1,5);

//        slice_number = slice_number_spinbox->value();
//    }

    layout->addWidget(new QLabel("x laction "),0,0);
    layout->addWidget(X_point_spinbox, 0,1,1,5);

    layout->addWidget(new QLabel("y location"),1,0);
    layout->addWidget(Y_point_spinbox, 1,1,1,5);

    layout->addWidget(new QLabel("z location"),2,0);
    layout->addWidget(Z_point_spinbox, 2,1,1,5);

    layout->addWidget(new QLabel("ray_numble of model"),3,0);
    layout->addWidget(number_ray_spinbox, 3,1,1,5);

    layout->addWidget(new QLabel("length_numbers of model"),4,0);
    layout->addWidget(length_ray_spinbox, 4,1,1,5);

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
    x_point = X_point_spinbox->value();
    y_point = Y_point_spinbox->value();
    z_point = Z_point_spinbox->value();
    ray_numble = number_ray_spinbox->value();
    length_numble = length_ray_spinbox->value();

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }


    cout<<"complete this dialog"<<endl;

    cout<<"start the ray-shooting model"<<endl;

    vector<float> x_lac;
    vector<float> y_lac;
    float ang = 2*PI/ray_numble;

    for(int i=0;i<ray_numble;i++)
    {
        float x_dis=cos(ang*(i+1));
        float y_dis=sin(ang*(i+1));
        for(int j=0;j<length_numble;j++)
        {
             x_lac.push_back(x_point+(j+1)*x_dis);
             y_lac.push_back(y_point+(j+1)*y_dis);
        }
        //v3d_msg(QString("y is %1").arg(y_lac[i]));
    }

    cout<<"start save swc"<<endl;
    NeuronTree nt;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    for(V3DLONG i=0;i<x_lac.size();i++)
    {
        NeuronSWC s;
        s.x=x_lac[i];
        s.y = y_lac[i];
        s.z = sz[2]/2;
        s.radius = 1;
        s.type = 3;
        listNeuron.append(s);
        hashNeuron.insert(s.n, listNeuron.size()-1);
    }
    nt.n = -1;
    nt.on = true;
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;

    QString raynkdh = QString ("ray-shooting.swc");
    writeSWC_file(raynkdh,nt);

   // v3d_msg(QString("okokookokokoko"));
}




