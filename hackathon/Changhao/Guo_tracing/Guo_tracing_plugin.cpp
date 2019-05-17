/* Guo_tracing_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-3-26 : by Guochanghao
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include <iostream>
#include <stdio.h>
#include "vn.h"
#include "my_surf_objs.h"
#include "Guo_tracing_plugin.h"
#include "v3d_curvetracepara.h"
#include "v3dneuron_gd_tracing.h"
#include "neuron_tools.h"
#include "fastmarching_dt.h"
#include "new_ray-shooting.h"

Q_EXPORT_PLUGIN2(Guo_tracing, Guo_tracingPlugin);

using namespace std;


double dist(MyMarker a, LocationSimple b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

double dist(LocationSimple a, MyMarker b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}

double dist(LocationSimple a, LocationSimple b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y) + (a.z - b.z)*(a.z - b.z));
}



QStringList Guo_tracingPlugin::menulist() const
{
	return QStringList() 
        <<tr("LinkOnePointToSoma")
        <<tr("reconstruction_func")
        <<tr("tipTracingAddingBranch")
        <<tr("missedTipsTracing")
        <<tr("test")
		<<tr("about");
}

QStringList Guo_tracingPlugin::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void Guo_tracingPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("LinkOnePointToSoma"))
    {
        input_PARA p;
//        p.landmarks = callback.getLandmark(callback.currentImageWindow());
//        for(V3DLONG i = 0; i < p.landmarks.size(); i++)
//        {
//            if(p.landmarks.at(i).x<1)p.landmarks[i].x=1;
//            if(p.landmarks.at(i).y<1)p.landmarks[i].y=1;
//            if(p.landmarks.at(i).z<1)p.landmarks[i].z=1;

//        }
//        callback.setLandmark(callback.currentImageWindow(), p.landmarks);
//        callback.updateImageWindow(callback.currentImageWindow());
        if(!p.initialize_2(callback)) //here will initialize the image pointer, bounding box, etc.
            return;

        // fetch parameters from dialog
        if (!p.dialog())
            return;
        int flag = LinkOnePointToSoma(callback,parent,p);
        if(flag)
        {
            cout<<endl;
            cout<<"******************************"<<endl;
            cout<<"*LinkOnePointToSoma finished!*"<<endl;
            cout<<"******************************"<<endl;
        }
        else
            return;

    }
    else if (menu_name == tr("tracing_v1"))
	{
        input_PARA p;
        if(!p.initialize_2(callback)) //here will initialize the image pointer, bounding box, etc.
            return;

        // fetch parameters from dialog
        if (!p.dialog())
            return;
        int flag = reconstruction_func(callback,parent,p);
        if(flag)
        {
            cout<<endl;
            cout<<"*******************************"<<endl;
            cout<<"*reconstruction_func finished!*"<<endl;
            cout<<"*******************************"<<endl;
        }
        else
            return;

	}
    else if (menu_name == tr("tipTracingAddingBranch"))
    {
        input_PARA p;
        if(!p.initialize_2(callback)) //here will initialize the image pointer, bounding box, etc.
            return;

        // fetch parameters from dialog
        if (!p.dialog())
            return;
        int flag = tipTracingAddingBranch(callback,parent,p);
        if(flag)
        {
            cout<<endl;
            cout<<"**********************************"<<endl;
            cout<<"*tipTracingAddingBranch finished!*"<<endl;
            cout<<"**********************************"<<endl;
        }
        else
            return;

    }
    else if(menu_name == tr("missedTipsTracing"))
    {
        input_PARA p;
        if(!p.initialize_2(callback)) //here will initialize the image pointer, bounding box, etc.
            return;

        // fetch parameters from dialog
        if (!p.dialog())
            return;
        int flag = missedTipsTracing(callback, parent, p);
        if(flag)
        {
            cout<<endl;
            cout<<"****************"<<endl;
            cout<<"*test finished!*"<<endl;
            cout<<"****************"<<endl;
        }
        else
            return;
    }
    else if(menu_name == tr("test"))
    {
        input_PARA p;
        if(!p.initialize_2(callback)) //here will initialize the image pointer, bounding box, etc.
            return;

        // fetch parameters from dialog
        if (!p.dialog())
            return;
        int flag = test(callback, parent, p);
        if(flag)
        {
            cout<<endl;
            cout<<"****************"<<endl;
            cout<<"*test finished!*"<<endl;
            cout<<"****************"<<endl;
        }
        else
            return;
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Guochanghao, 2019-3-26"));
	}
}

bool Guo_tracingPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("tracing_func"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        reconstruction_func(callback,parent,PARA);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of Guo_tracing tracing **** \n");
		printf("vaa3d -x Guo_tracing -f tracing_func -i <inimg_file> -p <channel> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

int LinkOnePointToSoma(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &p)
{
    unsigned char* data1d = 0;
    v3dhandle curwin;
    Image4DSimple* p4DImage;
    V3DLONG nx,ny,nz,nc,channel;
    V3DLONG sz[4];
    ImagePixelType datatype = p.p4dImage->getDatatype();

    if(datatype==V3D_UINT8)
    {
        cout<<"datatype==V3D_UINT8"<<endl;
        curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return 0;
        }

        p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return 0;
        }


        data1d = p4DImage->getRawData();
        nx = p4DImage->getXDim();
        ny = p4DImage->getYDim();
        nz = p4DImage->getZDim();
        nc = p4DImage->getCDim();

        bool ok1;

        if(nc==1)
        {
            channel=1;
            ok1=true;
        }
        else
        {
            channel = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, nc, 1, &ok1);
        }

        if(!ok1)
            return 0;

        sz[0] = nx;
        sz[1] = ny;
        sz[2] = nz;
        sz[3] = nc;


        p.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,p.inimg_file.toStdString().c_str(), data1d, sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",p.inimg_file.toStdString().c_str());
            return 0;
        }
        if(p.channel < 1 || p.channel > sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return 0;
        }
        nx = sz[0];
        ny = sz[1];
        nz = sz[2];
        nc = sz[3];
        channel = p.channel;
    }


    //main neuron reconstruction code
    v3d_msg("start to preprocessing.\n", 0);

    QElapsedTimer timer1;
    timer1.start();


//    simple_saveimage_wrapper(callback,p.inimg_file,data1d,in_sz,datatype);
    cout<<"nx:"<<nx<<"  ny:"<<ny<<"  nz:"<<nz<<"  nc:"<<nc<<"  datatype:"<<datatype<<endl<<endl;
    V3DLONG img2Dsize = nx*ny;
    V3DLONG img3Dsize = nx*ny*nz;

    if (p.xc1>=p.xc0 && p.yc1>=p.yc0 && p.zc1>=p.zc0 &&
        p.xc0>=0 && p.xc1<p.p4dImage->getXDim() &&
        p.yc0>=0 && p.yc1<p.p4dImage->getYDim() &&
        p.zc0>=0 && p.zc1<p.p4dImage->getZDim())
    {
        cout<<"Image size is not wrong."<<endl;
    }
    else
    {
        v3d_msg("Somehow invalid volume box info is detected. Ignore it. But check your Vaa3D program.");
        return false;
    }

    //always just use the buffer data
    unsigned char * data1d_copy;
    try{data1d_copy=new unsigned char [img3Dsize];}
    catch(...) {v3d_msg("cannot allocate memory for data1d_copy."); return 0;}
    for(V3DLONG i=0; i < img3Dsize; i++){ data1d_copy[i]=data1d[i];}

    LandmarkList point = getCalcuMarker(p.landmarks);
    LocationSimple point0;
    if(point.size()==0)
    {
        v3d_msg("No point in the image. Please input one point location.\n(Note: If the location comes from *.marker file, you should -1 in x, y, z location by youself and input it.)");
        //set update the dialog
        float x_coordinate=nx/2;
        float y_coordinate=ny/2;
        float z_coordinate=nz/2;


        QDialog * dialog = new QDialog();


        if(p4DImage->getZDim() > 1)
                dialog->setWindowTitle("3D neuron image");
        else
                dialog->setWindowTitle("2D neuron image");
        QGridLayout * layout = new QGridLayout();

        QDoubleSpinBox * x_location_spinbox = new QDoubleSpinBox();
        x_location_spinbox->setRange(1,nx);
        x_location_spinbox->setDecimals(3);  // 精度
        x_location_spinbox->setSingleStep(0.005); // 步长
        x_location_spinbox->setValue(x_coordinate);

        QDoubleSpinBox * y_location_spinbox = new QDoubleSpinBox();
        y_location_spinbox->setRange(1,ny);
        y_location_spinbox->setDecimals(3);  // 精度
        y_location_spinbox->setSingleStep(0.005); // 步长
        y_location_spinbox->setValue(y_coordinate);

        QDoubleSpinBox * z_location_spinbox = new QDoubleSpinBox();
        z_location_spinbox->setRange(1,nz);
        z_location_spinbox->setDecimals(3);  // 精度
        z_location_spinbox->setSingleStep(0.005); // 步长
        z_location_spinbox->setValue(z_coordinate);


        layout->addWidget(new QLabel("x location"),1,0);
        layout->addWidget(x_location_spinbox, 1,1,1,8);

        layout->addWidget(new QLabel("y location"),2,0);
        layout->addWidget(y_location_spinbox, 2,1,1,8);

        layout->addWidget(new QLabel("z location"),3,0);
        layout->addWidget(z_location_spinbox, 3,1,1,8);


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
            v3d_msg("No point in the image and no point input. Exit!");
            return 0;
        }


        //get the dialog return values
        x_coordinate=x_location_spinbox->value();
        y_coordinate=y_location_spinbox->value();
        z_coordinate=z_location_spinbox->value();
        point0.x = x_coordinate;
        point0.y = y_coordinate;
        point0.z = z_coordinate;
        printf("original location:[%f %f %f]\n",point0.x,point0.y,point0.z);
        point0.x = floor(point0.x+0.5);
        point0.y = floor(point0.y+0.5);
        point0.z = floor(point0.z+0.5);
        printf("floor location:[%f %f %f]\n",point0.x,point0.y,point0.z);
    }
    else
    {
        cout<<"point.size:"<<point.size()<<endl;
        printf("original location:[%f %f %f]\n",point.at(0).x,point.at(0).y,point.at(0).z);
        point0 = point.at(0);
        point0.x = floor(point0.x+0.5);
        point0.y = floor(point0.y+0.5);
        point0.z = floor(point0.z+0.5);
        printf("floor location:[%f %f %f]\n",point0.x,point0.y,point0.z);

    }


    unsigned char ****p4d = 0;
    unsigned char *img_new = 0;
    img_new = p4DImage->getRawDataAtChannel(0);
    try {new4dpointer(p4d, nx, ny, nz, nc, img_new);}
    catch (...)
    {
        fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
        return 0;
    }


    CurveTracePara trace_para;
    trace_para.channo = p.channel;
    trace_para.sp_graph_resolution_step = 1; //no need to downsample further, 2013-02-10. //if change to 1 then make it more accurate, by PHC, 2013-02-09 //ds_step;
    trace_para.b_deformcurve = false; //true; //b_usedshortestpathonly;
    trace_para.b_postMergeClosebyBranches = false; //true; //b_mergeCloseBranches;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = true; //b_postTrim;
    trace_para.b_pruneArtifactBranches = true; //b_pruneArtifactBranches;
    trace_para.sp_graph_background = 1;
    if (p.bkg_thresh==0)
        trace_para.sp_graph_background = 0;
    if (trace_para.sp_graph_background)
    {
        trace_para.imgTH = p.bkg_thresh;
    }
    else
    {
        trace_para.imgTH = 0;
    }

    float *phi = 0;
    LocationSimple SOMA;


    if(1)
    {
        cout<<"Start detecting cellbody"<<endl;
        fastmarching_dt_XY(data1d_copy, phi, nx, ny, nz, 2, p.bkg_thresh);

        if (!phi)
        {
            v3d_msg("Fail to run the GSDT code. Exit.", 0);
            return 0;
        }

        V3DLONG sz0 = nx;
        V3DLONG sz1 = ny;
        V3DLONG sz2 = nz;
        V3DLONG sz01 = sz0 * sz1;
        V3DLONG tol_sz = sz01 * sz2;

        V3DLONG max_loc = 0;
        double max_val = phi[0];
        for(V3DLONG i = 0; i < tol_sz; i++)
        {
            if(phi[i] > max_val)
            {
                max_val = phi[i];
                max_loc = i;
            }
        }
        ImageMarker max_marker(max_loc % sz0, max_loc % sz01 / sz0, max_loc / sz01);

        SOMA.x = max_marker.x;
        SOMA.y = max_marker.y;
        SOMA.z = max_marker.z;
        cout<<endl<<"-----------SOMA location:["<<SOMA.x<<" "<<SOMA.y<<" "<<SOMA.z<<"]--------------"<<endl;
    }

    double weight_xy_z = 1;
    NeuronTree nt;

    nt = LinkOnePoint(p4d, sz, SOMA, point0, trace_para, weight_xy_z);



    //Output

    QString swc_name = p.inimg_file + "_LinkOnePointToSoma.swc";
    nt.name = "Guo_tracing";
    writeSWC_file(swc_name.toStdString().c_str(),nt);



//    Image4DSimple * new4DImage = new Image4DSimple();
//    new4DImage->setData((unsigned char *)data1d_copy, nx, ny, nz, nc, datatype);
//    v3dhandle newwin = callback.newImageWindow("data1d_copy");
//    callback.setImage(newwin, new4DImage);
//    callback.updateImageWindow(newwin);


    return 1;
}

int reconstruction_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &p)
{
    unsigned char* data1d = 0;
    v3dhandle curwin;
    Image4DSimple* p4DImage;
    V3DLONG nx,ny,nz,nc,channel;
    V3DLONG sz[4];
    ImagePixelType datatype = p.p4dImage->getDatatype();

    if(datatype==V3D_UINT8)
    {
        cout<<"datatype==V3D_UINT8"<<endl;
        curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return 0;
        }

        p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return 0;
        }


        data1d = p4DImage->getRawData();
        nx = p4DImage->getXDim();
        ny = p4DImage->getYDim();
        nz = p4DImage->getZDim();
        nc = p4DImage->getCDim();

        bool ok1;

        if(nc==1)
        {
            channel=1;
            ok1=true;
        }
        else
        {
            channel = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, nc, 1, &ok1);
        }

        if(!ok1)
            return 0;

        sz[0] = nx;
        sz[1] = ny;
        sz[2] = nz;
        sz[3] = nc;


        p.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,p.inimg_file.toStdString().c_str(), data1d, sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",p.inimg_file.toStdString().c_str());
            return 0;
        }
        if(p.channel < 1 || p.channel > sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return 0;
        }
        nx = sz[0];
        ny = sz[1];
        nz = sz[2];
        nc = sz[3];
        channel = p.channel;
    }


    //main neuron reconstruction code
    v3d_msg("start to preprocessing.\n", 0);

    QElapsedTimer timer1;
    timer1.start();


//    simple_saveimage_wrapper(callback,p.inimg_file,data1d,in_sz,datatype);
    cout<<"nx:"<<nx<<"  ny:"<<ny<<"  nz:"<<nz<<"  nc:"<<nc<<"  datatype:"<<datatype<<endl<<endl;
    V3DLONG img2Dsize = nx*ny;
    V3DLONG img3Dsize = nx*ny*nz;

    if (p.xc1>=p.xc0 && p.yc1>=p.yc0 && p.zc1>=p.zc0 &&
        p.xc0>=0 && p.xc1<p.p4dImage->getXDim() &&
        p.yc0>=0 && p.yc1<p.p4dImage->getYDim() &&
        p.zc0>=0 && p.zc1<p.p4dImage->getZDim())
    {
        cout<<"Image size is not wrong."<<endl;
    }
    else
    {
        v3d_msg("Somehow invalid volume box info is detected. Ignore it. But check your Vaa3D program.");
        return false;
    }

    //always just use the buffer data
    unsigned char * data1d_copy;
    try{data1d_copy=new unsigned char [img3Dsize];}
    catch(...) {v3d_msg("cannot allocate memory for data1d_copy."); return 0;}
    for(V3DLONG i=0; i < img3Dsize; i++){ data1d_copy[i]=data1d[i];}

    LandmarkList BranchAndTip = getCalcuMarker(p.landmarks);

    if(BranchAndTip.size()==0)
    {
        v3d_msg("NO BranchAndTip! BranchAndTip.size is 0! Please add points!");
    }
    else
    {
        cout<<"BranchAndTip.size:"<<BranchAndTip.size()<<endl;
    }

    unsigned char ****p4d = 0;
    unsigned char *img_new = 0;
    img_new = p4DImage->getRawDataAtChannel(0);
    try {new4dpointer(p4d, nx, ny, nz, nc, img_new);}
    catch (...)
    {
        fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
        return 0;
    }


    CurveTracePara trace_para;
    trace_para.channo = p.channel;
    trace_para.sp_graph_resolution_step = 1; //no need to downsample further, 2013-02-10. //if change to 1 then make it more accurate, by PHC, 2013-02-09 //ds_step;
    trace_para.b_deformcurve = false; //true; //b_usedshortestpathonly;
    trace_para.b_postMergeClosebyBranches = false; //true; //b_mergeCloseBranches;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = true; //b_postTrim;
    trace_para.b_pruneArtifactBranches = true; //b_pruneArtifactBranches;
    trace_para.sp_graph_background = 1;
    if (p.bkg_thresh==0)
        trace_para.sp_graph_background = 0;
    if (trace_para.sp_graph_background)
    {
        trace_para.imgTH = p.bkg_thresh;
    }
    else
    {
        trace_para.imgTH = 0;
    }

    float *phi = 0;
    LocationSimple SOMA;
    LocationSimple p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11;


    if(0)
    {
        cout<<"Start detecting cellbody"<<endl;
        //should always use UINT8 here as indata has been converted to uint8
        fastmarching_dt_XY(data1d_copy, phi, nx, ny, nz, 2, p.bkg_thresh);

        if (!phi)
        {
            v3d_msg("Fail to run the GSDT code. Exit.", 0);
            return 0;
        }

        V3DLONG sz0 = nx;
        V3DLONG sz1 = ny;
        V3DLONG sz2 = nz;
        V3DLONG sz01 = sz0 * sz1;
        V3DLONG tol_sz = sz01 * sz2;

        V3DLONG max_loc = 0;
        double max_val = phi[0];
        for(V3DLONG i = 0; i < tol_sz; i++)
        {
            if(phi[i] > max_val)
            {
                max_val = phi[i];
                max_loc = i;
            }
        }
        ImageMarker max_marker(max_loc % sz0, max_loc % sz01 / sz0, max_loc / sz01);

        SOMA.x = max_marker.x;
        SOMA.y = max_marker.y;
        SOMA.z = max_marker.z;
        cout<<endl<<"-----------SOMA location:["<<SOMA.x<<" "<<SOMA.y<<" "<<SOMA.z<<"]--------------"<<endl;
    }

    switch (BranchAndTip.size()-1)
    {
    case 11:p11 = BranchAndTip[11];
    case 10:p10 = BranchAndTip[10];
    case 9:p9 = BranchAndTip[9];
    case 8:p8 = BranchAndTip[8];
    case 7:p7 = BranchAndTip[7];
    case 6:p6 = BranchAndTip[6];
    case 5:p5 = BranchAndTip[5];
    case 4:p4 = BranchAndTip[4];
    case 3:p3 = BranchAndTip[3];
    case 2:p2 = BranchAndTip[2];
    case 1:p1 = BranchAndTip[1];
    case 0:p0 = BranchAndTip[0];

        break;

    }


    vector<LocationSimple> pp;pp.push_back(p1);
    double weight_xy_z = 1;
    NeuronTree nt,tem_nt;
    v3d_msg("666",0);
    if(1)
    {
        v3d_msg("LinkOnePoint",0);
//        nt = LinkOnePoint(p4d, sz, SOMA, p0, trace_para, weight_xy_z);
        tem_nt = LinkOnePoint(p4d, sz, p0, p1, trace_para, weight_xy_z);
        nt = merge_two_neuron(nt,tem_nt);
        tem_nt = LinkOnePoint(p4d, sz, p1, p2, trace_para, weight_xy_z);
        nt = merge_two_neuron(nt,tem_nt);
        tem_nt = LinkOnePoint(p4d, sz, p2, p3, trace_para, weight_xy_z);
        nt = merge_two_neuron(nt,tem_nt);
//        tem_nt = LinkOnePoint(p4d, sz, p2, p4, trace_para, weight_xy_z);
//        nt = merge_two_neuron(nt,tem_nt);
//        tem_nt = LinkOnePoint(p4d, sz, p1, p5, trace_para, weight_xy_z);
//        nt = merge_two_neuron(nt,tem_nt);
//        tem_nt = tracing111(p4d, sz, p5, p6, trace_para, weight_xy_z);if(tem_nt.listNeuron.size()==0)v3d_msg("2");
//        nt = merge_two_neuron(nt,tem_nt);
//        tem_nt = tracing111(p4d, sz, p6, p7, trace_para, weight_xy_z);if(tem_nt.listNeuron.size()==0)v3d_msg("3");
//        nt = merge_two_neuron(nt,tem_nt);
//        tem_nt = LinkOnePoint(p4d, sz, p7, p8, trace_para, weight_xy_z);
//        nt = merge_two_neuron(nt,tem_nt);
//        tem_nt = LinkOnePoint(p4d, sz, p7, p9, trace_para, weight_xy_z);
//        nt = merge_two_neuron(nt,tem_nt);
//        tem_nt = LinkOnePoint(p4d, sz, p6, p10, trace_para, weight_xy_z);
//        nt = merge_two_neuron(nt,tem_nt);
//        tem_nt = LinkOnePoint(p4d, sz, p5, p11, trace_para, weight_xy_z);
//        nt = merge_two_neuron(nt,tem_nt);
    }
    else
    {
//        nt0 = v3dneuron_GD_tracing(p4d, sz, p0, pp, trace_para, weight_xy_z);
    }

//    nt = LinkOnePoint(p4d, sz, p0, p1, trace_para, weight_xy_z);

    //Output

    QString swc_name = p.inimg_file + "0_Guo_tracing.swc";
    nt.name = "Guo_tracing";
    writeSWC_file(swc_name.toStdString().c_str(),nt);
//    swc_name = p.inimg_file + "1_Guo_tracing.swc";
//    nt1.name = "Guo_tracing";
//    writeSWC_file(swc_name.toStdString().c_str(),nt1);
//    swc_name = p.inimg_file + "2_Guo_tracing.swc";
//    nt2.name = "Guo_tracing";
//    writeSWC_file(swc_name.toStdString().c_str(),nt2);




    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)data1d_copy, nx, ny, nz, nc, datatype);
    v3dhandle newwin = callback.newImageWindow("data1d_copy");
    callback.setImage(newwin, new4DImage);
    callback.updateImageWindow(newwin);
//    callback.setSWC(newwin,nt0);
//    callback.setSWC(newwin,nt1);
//    callback.setSWC(newwin,nt2);
//    QMainWindow windowNew;windowNew.

    return 1;
}


int tipTracingAddingBranch(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &p)
{

    unsigned char* data1d = 0;
    v3dhandle curwin;
    Image4DSimple* p4DImage;
    V3DLONG nx,ny,nz,nc,channel;
    V3DLONG sz[4];
    ImagePixelType datatype = p.p4dImage->getDatatype();

    if(datatype==V3D_UINT8)
    {
        cout<<"datatype==V3D_UINT8"<<endl;
        curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return 0;
        }

        p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return 0;
        }


        data1d = p4DImage->getRawData();
        nx = p4DImage->getXDim();
        ny = p4DImage->getYDim();
        nz = p4DImage->getZDim();
        nc = p4DImage->getCDim();

        bool ok1;

        if(nc==1)
        {
            channel=1;
            ok1=true;
        }
        else
        {
            channel = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, nc, 1, &ok1);
        }

        if(!ok1)
            return 0;

        sz[0] = nx;
        sz[1] = ny;
        sz[2] = nz;
        sz[3] = nc;


        p.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,p.inimg_file.toStdString().c_str(), data1d, sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",p.inimg_file.toStdString().c_str());
            return 0;
        }
        if(p.channel < 1 || p.channel > sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return 0;
        }
        nx = sz[0];
        ny = sz[1];
        nz = sz[2];
        nc = sz[3];
        channel = p.channel;
    }


    //main neuron reconstruction code
    v3d_msg("start to preprocessing.\n", 0);

    QElapsedTimer timer1;
    timer1.start();


//    simple_saveimage_wrapper(callback,p.inimg_file,data1d,in_sz,datatype);
    cout<<"nx:"<<nx<<"  ny:"<<ny<<"  nz:"<<nz<<"  nc:"<<nc<<"  datatype:"<<datatype<<endl<<endl;
    V3DLONG img2Dsize = nx*ny;
    V3DLONG img3Dsize = nx*ny*nz;

    if (p.xc1>=p.xc0 && p.yc1>=p.yc0 && p.zc1>=p.zc0 &&
        p.xc0>=0 && p.xc1<p.p4dImage->getXDim() &&
        p.yc0>=0 && p.yc1<p.p4dImage->getYDim() &&
        p.zc0>=0 && p.zc1<p.p4dImage->getZDim())
    {
        cout<<"Image size is not wrong."<<endl;
    }
    else
    {
        v3d_msg("Somehow invalid volume box info is detected. Ignore it. But check your Vaa3D program.");
        return false;
    }

    //always just use the buffer data
    unsigned char * data1d_copy;
    try{data1d_copy=new unsigned char [img3Dsize];}
    catch(...) {v3d_msg("cannot allocate memory for data1d_copy."); return 0;}
    for(V3DLONG i=0; i < img3Dsize; i++){ data1d_copy[i]=data1d[i];}

    LandmarkList built_inPointSet = getCalcuMarker(p.landmarks);

    if(built_inPointSet.size()==0)
    {
        v3d_msg("NO built_inPointSet! built_inPointSet.size is 0! Please add points! Exit.");
        return 0;
    }
    else
    {
        cout<<"built_inPointSet.size:"<<built_inPointSet.size()<<endl;
    }

    unsigned char ****p4d = 0;
    unsigned char *img_new = 0;
    img_new = p4DImage->getRawDataAtChannel(0);
    try {new4dpointer(p4d, nx, ny, nz, nc, img_new);}
    catch (...)
    {
        fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
        return 0;
    }


    CurveTracePara trace_para;
    trace_para.channo = p.channel;
    trace_para.sp_graph_resolution_step = 1; //no need to downsample further, 2013-02-10. //if change to 1 then make it more accurate, by PHC, 2013-02-09 //ds_step;
    trace_para.b_deformcurve = false; //true; //b_usedshortestpathonly;
    trace_para.b_postMergeClosebyBranches = false; //true; //b_mergeCloseBranches;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = true; //b_postTrim;
    trace_para.b_pruneArtifactBranches = true; //b_pruneArtifactBranches;
    trace_para.sp_graph_background = 1;
    if (p.bkg_thresh==0)
        trace_para.sp_graph_background = 0;
    if (trace_para.sp_graph_background)
    {
        trace_para.imgTH = p.bkg_thresh;
    }
    else
    {
        trace_para.imgTH = 0;
    }

    QFileInfo info(p.inimg_file);
    QString baseName = info.baseName();
    QString canonicalPath = info.canonicalPath();
    float *phi = 0;
    LocationSimple SOMA;

    double weight_xy_z = 1;
    NeuronTree nt,tem_nt;

    //define the color
    RGBA8 red, green, blue, yellow, pink, orange, darkOrange, purple;
    red.r=255;          red.g=0;            red.b=0;
    green.r=0;          green.g=255;        green.b=0;
    blue.r=0;           blue.g=0;           blue.b=255;
    yellow.r=255;       yellow.g=255;       yellow.b=0;
    pink.r=255;         pink.g=105;         pink.b=180;
    orange.r=255;       orange.g=165;       orange.b=0;
    darkOrange.r=255;   darkOrange.g=100;   darkOrange.b=0;
    purple.r=255;       purple.g=0;         purple.b=255;

    vector<LocationSimple> tipPointSet;
    LandmarkList branchPointSet, trunkPointSet, somaPointSet;
    for(V3DLONG i = 0; i < built_inPointSet.size(); i++)
    {
        if(built_inPointSet.at(i).comments=="tip")//(0->trunk,-1->soma,1->branch,2->tip)
        {
            tipPointSet.push_back(built_inPointSet.at(i));
        }
        else if(built_inPointSet.at(i).comments=="branch")
        {
            branchPointSet.push_back(built_inPointSet.at(i));
        }
        else if(built_inPointSet.at(i).comments=="soma")
        {
            somaPointSet.push_back(built_inPointSet.at(i));
        }
        else if(built_inPointSet.at(i).comments=="trunk")
        {
            trunkPointSet.push_back(built_inPointSet.at(i));
        }
        else
        {   v3d_msg("built_inPointSet.comments contain invalid type! Please check *.marker file. Exit.");return 0;}
    }

    if(built_inPointSet.size()!=(tipPointSet.size()+branchPointSet.size()+trunkPointSet.size()+somaPointSet.size()))
    {v3d_msg("built_inPointSet.comments contain invalid type! Please check *.marker file");return 0;}



    if(built_inPointSet.size()==somaPointSet.size()){
        cout<<"*******************************************"<<endl;
        cout<<"*built_inPointSet only contain 'somaPoint'*"<<endl;
        cout<<"*******************************************"<<endl;}
    else if(built_inPointSet.size()==tipPointSet.size()){
        cout<<"******************************************"<<endl;
        cout<<"*built_inPointSet only contain 'tipPoint'*"<<endl;
        cout<<"******************************************"<<endl;}
    else if(built_inPointSet.size()==branchPointSet.size()){
        cout<<"*********************************************"<<endl;
        cout<<"*built_inPointSet only contain 'branchPoint'*"<<endl;
        cout<<"*********************************************"<<endl;}
    else if(built_inPointSet.size()==trunkPointSet.size()){
        cout<<"********************************************"<<endl;
        cout<<"*built_inPointSet only contain 'trunkPoint'*"<<endl;
        cout<<"********************************************"<<endl;}
    else if(built_inPointSet.size()==(branchPointSet.size()+tipPointSet.size())){
        cout<<"***************************************************"<<endl;
        cout<<"*built_inPointSet contain 'branchPoint + tipPoint'*"<<endl;
        cout<<"***************************************************"<<endl;}
    else if(built_inPointSet.size()==(somaPointSet.size()+tipPointSet.size())){
        cout<<"*************************************************"<<endl;
        cout<<"*built_inPointSet contain 'somaPoint + tipPoint'*"<<endl;
        cout<<"*************************************************"<<endl;}
    else if(built_inPointSet.size()==(somaPointSet.size()+branchPointSet.size())){
        cout<<"****************************************************"<<endl;
        cout<<"*built_inPointSet contain 'somaPoint + branchPoint'*"<<endl;
        cout<<"****************************************************"<<endl;}
    else if(built_inPointSet.size()==(somaPointSet.size()+trunkPointSet.size())){
        cout<<"***************************************************"<<endl;
        cout<<"*built_inPointSet contain 'somaPoint + trunkPoint'*"<<endl;
        cout<<"***************************************************"<<endl;}
    else if(built_inPointSet.size()==(branchPointSet.size()+tipPointSet.size()+somaPointSet.size())){
        cout<<"***************************************************************"<<endl;
        cout<<"*built_inPointSet contain 'somaPoint + branchPoint + tipPoint'*"<<endl;
        cout<<"***************************************************************"<<endl;}
    else if(built_inPointSet.size()==(branchPointSet.size()+tipPointSet.size()+somaPointSet.size()+trunkPointSet.size())){
        cout<<"****************************************************************************"<<endl;
        cout<<"*built_inPointSet contain 'somaPoint + branchPoint + tipPoint + trunkPoint'*"<<endl;
        cout<<"****************************************************************************"<<endl;}
    else {v3d_msg("Yet do not support program with such pointType situation. Exit.");}



    if(trunkPointSet.size()!=0)
    {   v3d_msg("Yet do not support program with trunkPoint. Exit."); return 0;}

    if(somaPointSet.size()!=0)
    {
        if(somaPointSet.size()==1)
            SOMA = somaPointSet.takeFirst();
        else
        {   v3d_msg("Yet do not support with somaPoint.size>1. Exit."); return 0;}
    }
    else//when somaPointSet.size()==0
    {
        if(1)
        {
            cout<<"Start detecting cellbody"<<endl;
            fastmarching_dt_XY(data1d_copy, phi, nx, ny, nz, 2, p.bkg_thresh);

            if (!phi)
            {
                v3d_msg("Fail to run the GSDT code. Exit.", 0);
                return 0;
            }

            V3DLONG sz0 = nx;
            V3DLONG sz1 = ny;
            V3DLONG sz2 = nz;
            V3DLONG sz01 = sz0 * sz1;
            V3DLONG tol_sz = sz01 * sz2;

            V3DLONG max_loc = 0;
            double max_val = phi[0];
            for(V3DLONG i = 0; i < tol_sz; i++)
            {
                if(phi[i] > max_val)
                {
                    max_val = phi[i];
                    max_loc = i;
                }
            }
            ImageMarker max_marker(max_loc % sz0, max_loc % sz01 / sz0, max_loc / sz01);

            SOMA.x = max_marker.x;
            SOMA.y = max_marker.y;
            SOMA.z = max_marker.z;
            SOMA.comments = "soma";
            cout<<endl<<"-----------SOMA location:["<<SOMA.x<<" "<<SOMA.y<<" "<<SOMA.z<<"]--------------"<<endl;
            QString somafilename = baseName+"_SOMA.marker";
            saveSoma(somafilename, SOMA);
        }
        else//for test
        {
//            SOMA.x = 901; SOMA.y = 739; SOMA.z = 36; SOMA.comments = "soma";//img = 140918c9.tif
            SOMA.x = 869.000; SOMA.y = 1156.000; SOMA.z = 10.000; SOMA.comments = "soma";//img = fruitfly3.tif

            cout<<endl<<"-----------SOMA location:["<<SOMA.x<<" "<<SOMA.y<<" "<<SOMA.z<<"]--------------"<<endl;
        }
    }

    vector<MyMarker*> swc;
    QString tempname;
    QString swc_name;

//    cout<<"canonicalPath:"<<canonicalPath.toStdString()<<endl<<"baseName:"<<baseName.toStdString()<<endl;
    if(1)//1 for test, 0 for nomal use
    {
//        tempname = "C:\\Users\\Administrator\\Desktop\\";
        tempname = "C:\\Users\\yufuhao\\Desktop\\140918c9_SomaToAllTips_Guo_tracing.swc";
    }
    else
    {
        nt = LinkPoints(callback, p4d, sz, SOMA, tipPointSet, trace_para, weight_xy_z);
        cout<<endl<<"nt.size:"<<nt.listNeuron.size()<<endl;

//        QString swc_name = p.inimg_file + "_readafter_Guo_tracing.swc";

        swc_name = canonicalPath + "\/" + baseName + "_SomaToAllTips_Guo_tracing.swc";
        nt.name = "Guo_tracing";
        writeSWC_file(swc_name.toStdString().c_str(),nt);
        tempname.append(swc_name);
    }
//    double margin = 1;
//    ComputemaskImage(nt,data1d_copy,nx,ny,nz,margin);
//    Image4DSimple * new4DImage = new Image4DSimple();
//    new4DImage->setData((unsigned char *)data1d_copy, nx, ny, nz, p4DImage->getCDim(), p4DImage->getDatatype());
//    v3dhandle newwin = callback.newImageWindow();
//    callback.setImage(newwin, new4DImage);
//    callback.updateImageWindow(newwin);



    if(tempname==""){v3d_msg("readSWC_file failed!"); return 0;}
    swc=readSWC_file1(tempname.toStdString());//check the middle result of the tracing above here whether missing the branchPath
    tem_nt = vectorofMyMarker2NeuronTree(swc);
    LandmarkList swcBranchPoint = FromTreeGetBranch(tem_nt);//the BranchPoint of swc are red
    LandmarkList missedBranchPointSet; missedBranchPointSet.clear();//missedBranchPointSet contains the nearest missedBranchPoints
    LandmarkList missedTipPointSet; missedTipPointSet.clear();//missedTipPointSet contains the nearest missedTipPoints
    LandmarkList matchSuccessSet; matchSuccessSet.clear();//matchSuccessSet contains the nearest matchSuccess Points
    v3d_msg(QString("swc.size:%1").arg(swc.size()),0);
    v3d_msg(QString("tem_nt.size:%1").arg(tem_nt.listNeuron.size()),0);
    v3d_msg(QString("swcBranchPoint.size:%1").arg(swcBranchPoint.size()),0);
    for(V3DLONG i = 0; i < branchPointSet.size(); i++)
    {
        vector<MyMarker> candidateBranch; candidateBranch.clear();
        for(V3DLONG j = 0; j < swc.size(); j++)
        {
            if(dist(*swc.at(j), branchPointSet.at(i))<p.b_thresh)//if the distance is close to real branchPoint, the swc point is defined as candidateBranch
            {
                branchPointSet[i].color = pink;//if branchPointSet matches the swcPoint successfully, its color turns pink
                swc[j]->type=13;//the matched swcPoints' color turn pink and save it to candidateBranch
                candidateBranch.push_back(*swc.at(j));
            }

        }

        v3d_msg(QString("candidateBranch.size:%1").arg(candidateBranch.size()),0);

        if(candidateBranch.size()==0)//if NO swcPoint nearby real branchPoint, this branchPoint needs tracing
        {
            LocationSimple missedBranchPoint = branchPointSet.at(i);
            missedBranchPoint.color = darkOrange;
            missedBranchPoint.comments.clear();
            missedBranchPoint.comments = "missTwoBranch";//missTwoBranch
            missedBranchPointSet.push_back(missedBranchPoint);
        }
        else
        {
            LandmarkList matchSuccess; matchSuccess.clear();
            vector<V3DLONG> m_ind;
            for(V3DLONG k = 0; k < candidateBranch.size(); k++)//check whether candidateBranch already has a branchPath, if not, tracing the missing branchPath
            {
                for(V3DLONG m = 0; m < swcBranchPoint.size(); m++)
                {
                    if(dist(candidateBranch.at(k), swcBranchPoint.at(m))<2)//it should have been "="(equal), but set "<threshold" for the robust precision
                    {
                        //branchPoint already has two traced BranchPaths, do nothing
                        v3d_msg(QString("match success!"),0);
                        LocationSimple tempMarker;
                        tempMarker = MyMarker2LocationSimple(candidateBranch.at(k));
                        tempMarker.color = green;
                        matchSuccess.push_back(tempMarker);
                        m_ind.push_back(m);
                        break;
                    }
                    else{/*v3d_msg(QString("NO match!"),0);*/}
                }
            }
            if(matchSuccess.size()!=0)
            {
                LocationSimple min_matchSuccess = matchSuccess.at(0);
                double min = dist(matchSuccess.at(0), swcBranchPoint.at(m_ind.at(0)));
                for(V3DLONG w = 0; w < matchSuccess.size(); w++)
                {
                    if(min > dist(matchSuccess.at(w), swcBranchPoint.at(m_ind.at(w))))
                    {
                        min = dist(matchSuccess.at(w), swcBranchPoint.at(m_ind.at(w)));
                        min_matchSuccess = matchSuccess.at(w);
                    }
                }
                matchSuccessSet.push_back(min_matchSuccess);
            }


            cout<<"matchSuccess.size:"<<matchSuccess.size()<<endl;

            if(matchSuccess.size()==0)//No swcBranchPoint in candidateBranch
            {
                //find the nearest swcPoint and using it as a startPoint to add one branchPath tracing

                LocationSimple missedBranchPoint = MyMarker2LocationSimple(candidateBranch.at(0));
                missedBranchPoint.comments = "missOneBranch";
                missedBranchPoint.color = orange;
                double min = dist(candidateBranch.at(0), missedBranchPoint);
                for(V3DLONG n = 0; n < candidateBranch.size(); n++)
                {
                    if(min>dist(candidateBranch.at(n), branchPointSet.at(i)))
                    {
                        min = dist(candidateBranch.at(n), branchPointSet.at(i));
                        missedBranchPoint = MyMarker2LocationSimple(candidateBranch.at(n));
                        missedBranchPoint.comments.clear();
                        missedBranchPoint.comments = "missOneBranch";
                        missedBranchPoint.color = orange;
                    }
                }
                missedBranchPointSet.push_back(missedBranchPoint);


            }
            else {/*do nothing*/}
        }

    }
    nt=vectorofMyMarker2NeuronTree(swc);


    LandmarkList missedBranchPointA, missedBranchPointB;
    for(V3DLONG i = 0; i < missedBranchPointSet.size(); i++)
    {
        if(missedBranchPointSet.at(i).comments == "missOneBranch")
        {
            missedBranchPointA.push_back(missedBranchPointSet.at(i));
        }
        else if(missedBranchPointSet.at(i).comments == "missTwoBranch")
        {
            missedBranchPointB.push_back(missedBranchPointSet.at(i));
        }
        else
        {
            printf("invalid missedBranchPointSet.comments:%s\n",missedBranchPointSet.at(i).comments);
            v3d_msg("missedBranchPoint.comments is invalid! Please check missedBranchPoint.comments");
            return 0;
        }
    }
    cout<<"missedBranchPointSet.size:"<<missedBranchPointSet.size()<<endl;
    cout<<"missedBranchPointA.size:"<<missedBranchPointA.size()<<endl;
    cout<<"missedBranchPointB.size:"<<missedBranchPointB.size()<<endl;


    for(V3DLONG i = 0; i < tipPointSet.size(); i++)
    {
        vector<MyMarker> candidateTip; candidateTip.clear();
        for(V3DLONG j = 0; j < swc.size(); j++)
        {
            if(dist(*swc.at(j), tipPointSet.at(i))<p.t_thresh)//if the distance is close to real branchPoint, the swc point is defined as candidateTip
            {
                tipPointSet[i].color = pink;//if tipPointSet matches the swcPoint successfully, its color turns pink
                swc[j]->type=13;//the matched swcPoints' color turn pink and save it to candidateTip
                candidateTip.push_back(*swc.at(j));
            }
        }

        v3d_msg(QString("candidateTip.size:%1").arg(candidateTip.size()),0);

        if(candidateTip.size()==0)//if NO swcPoint nearby real tipPoint, this tipPoint needs tracing
        {
            LocationSimple missedTipPoint = tipPointSet.at(i);
            missedTipPoint.color = darkOrange;
            missedTipPointSet.push_back(missedTipPoint);
        }
        else
        {
        }
    }

    v3d_msg(QString("missedTipPointSet.size:%1").arg(missedTipPointSet.size()),0);

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)data1d_copy, nx, ny, nz, nc, datatype);
    v3dhandle newwin = callback.newImageWindow("data1d_copy");
    callback.setImage(newwin, new4DImage);
    callback.updateImageWindow(newwin);
    displayMarker(callback, newwin, missedBranchPointSet);//mergeLandmark(missedTipPointSet,mergeLandmark(missedBranchPointSet, matchSuccessSet)));
    //missedTipPointSet

    return 1;
}



int missedTipsTracing(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &p)
{
//    vector<MyMarker*> swc;
//    QString tempname="C:\\Users\\Administrator\\Desktop\\140918c9.tifSomaToAllTips_Guo_tracing.swc";
//    swc=readSWC_file1(tempname.toStdString());//check the middle result of the tracing above here whether missing the branchPath
//    NeuronTree tem_nt = vectorofMyMarker2NeuronTree(swc);
//    LandmarkList swcBranchPointSet;
//    swcBranchPointSet = FromTreeGetBranch(tem_nt);

//    v3dhandle curwin = callback.currentImageWindow();
//    displayMarker(callback, curwin, swcBranchPointSet);

    unsigned char* data1d = 0;
    v3dhandle curwin;
    Image4DSimple* p4DImage;
    V3DLONG nx,ny,nz,nc,channel;
    V3DLONG sz[4];
    ImagePixelType datatype = p.p4dImage->getDatatype();

    if(datatype==V3D_UINT8)
    {
        cout<<"datatype==V3D_UINT8"<<endl;
        curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return 0;
        }

        p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return 0;
        }


        data1d = p4DImage->getRawData();
        nx = p4DImage->getXDim();
        ny = p4DImage->getYDim();
        nz = p4DImage->getZDim();
        nc = p4DImage->getCDim();

        bool ok1;

        if(nc==1)
        {
            channel=1;
            ok1=true;
        }
        else
        {
            channel = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, nc, 1, &ok1);
        }

        if(!ok1)
            return 0;

        sz[0] = nx;
        sz[1] = ny;
        sz[2] = nz;
        sz[3] = nc;


        p.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,p.inimg_file.toStdString().c_str(), data1d, sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",p.inimg_file.toStdString().c_str());
            return 0;
        }
        if(p.channel < 1 || p.channel > sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return 0;
        }
        nx = sz[0];
        ny = sz[1];
        nz = sz[2];
        nc = sz[3];
        channel = p.channel;
    }


    //main neuron reconstruction code
    v3d_msg("start to preprocessing.\n", 0);

    QElapsedTimer timer1;
    timer1.start();


//    simple_saveimage_wrapper(callback,p.inimg_file,data1d,in_sz,datatype);
    cout<<"nx:"<<nx<<"  ny:"<<ny<<"  nz:"<<nz<<"  nc:"<<nc<<"  datatype:"<<datatype<<endl<<endl;
    V3DLONG img2Dsize = nx*ny;
    V3DLONG img3Dsize = nx*ny*nz;

    if (p.xc1>=p.xc0 && p.yc1>=p.yc0 && p.zc1>=p.zc0 &&
        p.xc0>=0 && p.xc1<p.p4dImage->getXDim() &&
        p.yc0>=0 && p.yc1<p.p4dImage->getYDim() &&
        p.zc0>=0 && p.zc1<p.p4dImage->getZDim())
    {
        cout<<"Image size is not wrong."<<endl;
    }
    else
    {
        v3d_msg("Somehow invalid volume box info is detected. Ignore it. But check your Vaa3D program.");
        return false;
    }

    //always just use the buffer data
    unsigned char * data1d_copy;
    try{data1d_copy=new unsigned char [img3Dsize];}
    catch(...) {v3d_msg("cannot allocate memory for data1d_copy."); return 0;}
    for(V3DLONG i=0; i < img3Dsize; i++){ data1d_copy[i]=data1d[i];}

    unsigned char ****p4d = 0;
    unsigned char *img_new = 0;
    img_new = p4DImage->getRawDataAtChannel(0);
    try {new4dpointer(p4d, nx, ny, nz, nc, img_new);}
    catch (...)
    {
        fprintf (stderr, "Fail to create a 4D pointer for the image data. Exit. \n");
        return 0;
    }


    CurveTracePara trace_para;
    trace_para.channo = p.channel;
    trace_para.sp_graph_resolution_step = 1; //no need to downsample further, 2013-02-10. //if change to 1 then make it more accurate, by PHC, 2013-02-09 //ds_step;
    trace_para.b_deformcurve = false; //true; //b_usedshortestpathonly;
    trace_para.b_postMergeClosebyBranches = false; //true; //b_mergeCloseBranches;
    trace_para.b_3dcurve_width_from_xyonly = true;
    trace_para.b_post_trimming = true; //b_postTrim;
    trace_para.b_pruneArtifactBranches = true; //b_pruneArtifactBranches;
    trace_para.sp_graph_background = 1;
    if (p.bkg_thresh==0)
        trace_para.sp_graph_background = 0;
    if (trace_para.sp_graph_background)
    {
        trace_para.imgTH = p.bkg_thresh;
    }
    else
    {
        trace_para.imgTH = 0;
    }

    //define the color
    RGBA8 red, green, blue, yellow, pink, orange, darkOrange, purple;
    red.r=255;          red.g=0;            red.b=0;
    green.r=0;          green.g=255;        green.b=0;
    blue.r=0;           blue.g=0;           blue.b=255;
    yellow.r=255;       yellow.g=255;       yellow.b=0;
    pink.r=255;         pink.g=105;         pink.b=180;
    orange.r=255;       orange.g=165;       orange.b=0;
    darkOrange.r=255;   darkOrange.g=100;   darkOrange.b=0;
    purple.r=255;       purple.g=0;         purple.b=255;

    float *phi = 0;
    LocationSimple SOMA;

    double weight_xy_z = 1;
    NeuronTree nt,tem_nt;

    LandmarkList missedTipPointSet = getCalcuMarker(p.landmarks);
    LandmarkList calibrated_tipPoint = calibrate_tipPoints(callback, missedTipPointSet, 15, 20);
    v3d_msg(QString("calibrated_tipPoint.size:%1").arg(calibrated_tipPoint.size()));

    if(1)
    {
        cout<<"Start detecting cellbody"<<endl;
        fastmarching_dt_XY(data1d_copy, phi, nx, ny, nz, 2, p.bkg_thresh);

        if (!phi)
        {
            v3d_msg("Fail to run the GSDT code. Exit.", 0);
            return 0;
        }

        V3DLONG sz0 = nx;
        V3DLONG sz1 = ny;
        V3DLONG sz2 = nz;
        V3DLONG sz01 = sz0 * sz1;
        V3DLONG tol_sz = sz01 * sz2;

        V3DLONG max_loc = 0;
        double max_val = phi[0];
        for(V3DLONG i = 0; i < tol_sz; i++)
        {
            if(phi[i] > max_val)
            {
                max_val = phi[i];
                max_loc = i;
            }
        }
        ImageMarker max_marker(max_loc % sz0, max_loc % sz01 / sz0, max_loc / sz01);

        SOMA.x = max_marker.x;
        SOMA.y = max_marker.y;
        SOMA.z = max_marker.z;
        cout<<endl<<"-----------SOMA location:["<<SOMA.x<<" "<<SOMA.y<<" "<<SOMA.z<<"]--------------"<<endl;
        printf("SOMA:[%f %f %f]",SOMA.x,SOMA.y,SOMA.z);
    }
    else
    {
        SOMA.x = 901; SOMA.y = 739; SOMA.z = 36; //img = 140918c9.tif
    }
//LandmarkList2vectorLocationSimple(missedTipPointSet)

    nt = LinkPoints(callback, p4d, sz, SOMA, LandmarkList2vectorLocationSimple(calibrated_tipPoint), trace_para, weight_xy_z);

    vector<MyMarker*> swc;
    QString tempname;
    QString swc_name;
    swc_name = p.inimg_file + "_missedTipsTracing_Guo_tracing.swc";
    nt.name = "Guo_tracing";
    writeSWC_file(swc_name.toStdString().c_str(),nt);
    displayMarker(callback, curwin, calibrated_tipPoint);


    return 1;

}

int test(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &p)
{
    unsigned char* data1d = 0;
    v3dhandle curwin;
    Image4DSimple* p4DImage;
    V3DLONG nx,ny,nz,nc,channel;
    V3DLONG sz[4];
    ImagePixelType datatype = p.p4dImage->getDatatype();

    if(datatype==V3D_UINT8)
    {
        cout<<"datatype==V3D_UINT8"<<endl;
        curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return 0;
        }

        p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return 0;
        }


        data1d = p4DImage->getRawData();
        nx = p4DImage->getXDim();
        ny = p4DImage->getYDim();
        nz = p4DImage->getZDim();
        nc = p4DImage->getCDim();

        bool ok1;

        if(nc==1)
        {
            channel=1;
            ok1=true;
        }
        else
        {
            channel = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, nc, 1, &ok1);
        }

        if(!ok1)
            return 0;

        sz[0] = nx;
        sz[1] = ny;
        sz[2] = nz;
        sz[3] = nc;


        p.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        if (!simple_loadimage_wrapper(callback,p.inimg_file.toStdString().c_str(), data1d, sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",p.inimg_file.toStdString().c_str());
            return 0;
        }
        if(p.channel < 1 || p.channel > sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return 0;
        }
        nx = sz[0];
        ny = sz[1];
        nz = sz[2];
        nc = sz[3];
        channel = p.channel;
    }


    //main neuron reconstruction code
    v3d_msg("start to preprocessing.\n", 0);

    QElapsedTimer timer1;
    timer1.start();


//    simple_saveimage_wrapper(callback,p.inimg_file,data1d,in_sz,datatype);
    cout<<"nx:"<<nx<<"  ny:"<<ny<<"  nz:"<<nz<<"  nc:"<<nc<<"  datatype:"<<datatype<<endl<<endl;
    V3DLONG img2Dsize = nx*ny;
    V3DLONG img3Dsize = nx*ny*nz;

    if (p.xc1>=p.xc0 && p.yc1>=p.yc0 && p.zc1>=p.zc0 &&
        p.xc0>=0 && p.xc1<p.p4dImage->getXDim() &&
        p.yc0>=0 && p.yc1<p.p4dImage->getYDim() &&
        p.zc0>=0 && p.zc1<p.p4dImage->getZDim())
    {
        cout<<"Image size is not wrong."<<endl;
    }
    else
    {
        v3d_msg("Somehow invalid volume box info is detected. Ignore it. But check your Vaa3D program.");
        return false;
    }

    //always just use the buffer data
    unsigned char * data1d_copy;
    try{data1d_copy=new unsigned char [img3Dsize];}
    catch(...) {v3d_msg("cannot allocate memory for data1d_copy."); return 0;}
    for(V3DLONG i=0; i < img3Dsize; i++)
    {
//        if(data1d[i]<10)data1d_copy[i]=0;
//        else{data1d_copy[i]=255*(log(double(1 + 100*data1d[i]/255))/log(double(100+1)));}
        data1d_copy[i]=data1d[i];
    }


    unsigned char * image_mip;
    try{image_mip=new unsigned char [img2Dsize];}
    catch(...) {v3d_msg("cannot allocate memory for image_mip."); return 0;}



    QList <V3dR_MainWindow *> list_3dviewer = callback.getListAll3DViewers();
    if (list_3dviewer.size() < 1)
    {
      v3d_msg("Please open  a SWC file from the main menu first! list_3dviewer");
      return false;
    }
    V3dR_MainWindow *surface_win = list_3dviewer[0];
    if (!surface_win){
      v3d_msg("Please open up a SWC file from the main menu first!");
      return false;
    }
    //    cout<<"read SWC from 3d Viewer  "<<endl;
    QList<NeuronTree> * mTreeList = callback.getHandleNeuronTrees_Any3DViewer(surface_win);
    if(mTreeList->size()<=0)
    {
      return false;
    }
    NeuronTree nt=mTreeList->at(mTreeList->size()-1);
    double margin = 1;
    LandmarkList missedBranchPoint = callback.getLandmark(curwin);v3d_msg(QString("missedBranchPoint.size:%1").arg(missedBranchPoint.size()));
    ComputemaskImage(callback,nt,data1d_copy,nx,ny,nz,margin,missedBranchPoint);


//    unsigned char * res;
//    try{res=new unsigned char [img2Dsize];}
//    catch(...) {v3d_msg("cannot allocate memory for res."); return 0;}
//    Closeoperations(image_mip, res, ny, nx);
//    Openoperations(image_mip, res, ny, nx);

/** filter  **/
//    float * data1d_filtered=0;
//    double sigma = 3;
//    gaussian_filter(data1d_copy, sz, 7, 7, 7, channel, sigma, data1d_filtered);

/** adaptive thresholding  **/
//    float sum0 =0.0;
//    V3DLONG count0=0;
//    for(V3DLONG i = 0; i < img3Dsize; i++)
//    {
//        sum0 += data1d_filtered[i]; count0++;
//    }
//    float adaptThresh0 = sum0/count0;
//    float adaptThresh;
//    float sum_low=0.0, sum_high=0.0;
//    float average_low=0.0, average_high=0.0;
//    V3DLONG count_low=0, count_high=0;
//    for(V3DLONG i = 0; i < img3Dsize; i++)
//    {
//        if(data1d_filtered[i] > adaptThresh0){sum_high += data1d_filtered[i]; count_high++;}
//        else{sum_low += data1d_filtered[i]; count_low++;}
//    }
//    if((count_low+count_high)==img3Dsize)
//    {
//        if(count_low&&count_high)
//        {
//            average_high = sum_high/count_high;
//            average_low = sum_low/count_low;
//            adaptThresh = (average_high+average_low)/2;
//        }
//        else if(count_low==0)
//        {
//            adaptThresh = sum_high/count_high;
//        }
//        else
//        {
//            adaptThresh = sum_low/count_low;
//        }
//    }
//    else{v3d_msg("count_low+count_high!=img3Dsize");}
//    float err = abs(adaptThresh-adaptThresh0);
//    cout<<endl<<"0  adaptThresh:"<<adaptThresh<<" adaptThresh0:"<<adaptThresh0<<endl;


//    V3DLONG t_num=0;
//    while(err>0.00001)
//    {
//        t_num++;cout<<"t_num:"<<t_num<<"  adaptThresh:"<<adaptThresh<<"  adaptThresh0:"<<adaptThresh0<<endl;
//        adaptThresh0 = adaptThresh;
//        sum_low=0.0; sum_high=0.0; average_low=0.0; average_high=0.0; count_low=0; count_high=0;
//        for(V3DLONG i = 0; i < img3Dsize; i++)
//        {
//            if(data1d_filtered[i] > adaptThresh0){sum_high += data1d_filtered[i]; count_high++;}
//            else{sum_low += data1d_filtered[i]; count_low++;}
//        }
//        if((count_low+count_high)==img3Dsize)
//        {
//            if(count_low&&count_high)
//            {
//                average_high = sum_high/count_high;
//                average_low = sum_low/count_low;
//                adaptThresh = (average_high+average_low)/2;
//            }
//            else if(count_low==0)
//            {
//                adaptThresh = sum_high/img3Dsize;
//            }
//            else
//            {
//                adaptThresh = sum_low/img3Dsize;
//            }
//        }
//        else{v3d_msg("count_low+count_high!=img3Dsize");}
//        err = abs(adaptThresh-adaptThresh0);
//    }

//    cout<<endl<<"adaptThresh:"<<adaptThresh<<endl;
//    for(V3DLONG i = 0; i < img3Dsize; i++)
//    {
//        if(data1d_filtered[i]<adaptThresh)data1d_filtered[i]=0;
//        else{data1d_filtered[i]=255;}
//    }


//    for(V3DLONG i = 0; i < img3Dsize; i++)
//    {
//        if(data1d_filtered[i]<5)data1d_filtered[i]=0;
//        else{data1d_filtered[i]=255*(log(double(1 + 100*data1d_filtered[i]/255))/log(double(100+1)));}
//    }

    simple_saveimage_wrapper(callback, "C:\\Users\\yufuhao\\Desktop\\mask2.tif", (unsigned char *)data1d_copy, sz, p4DImage->getDatatype());

    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)data1d_copy, nx, ny, nz, nc, p4DImage->getDatatype());
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.updateImageWindow(newwin);
    return 1;
}
