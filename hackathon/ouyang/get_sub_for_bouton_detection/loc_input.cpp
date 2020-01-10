#include <iostream>
#include "loc_input.h"
using namespace std;

loc_input::loc_input(QWidget *parent) : QDialog(parent)
{


    x_slider_label = new QLabel("1");
    y_slider_label = new QLabel("1");

    x_coordinate = 1;
    y_coordinate = 1;
    z_coordinate = 1;

    //*************
    x_location_spinbox = new QSpinBox();
    x_location_spinbox->setRange(-1000,2000);
    x_location_spinbox->setValue(x_coordinate);

    y_location_spinbox = new QSpinBox();
    y_location_spinbox->setRange(-1000,2000);
    y_location_spinbox->setValue(y_coordinate);

    z_location_spinbox = new QSpinBox();
    z_location_spinbox->setRange(-1000,2000);
    z_location_spinbox->setValue(z_coordinate);


    x_slider = new QSlider();
    x_slider->setRange(-1000,2000);
    x_slider->setValue(1);
    x_slider->setOrientation(Qt::Horizontal);
    x_slider->setTickPosition(QSlider::TicksAbove);

    y_slider = new QSlider();
    y_slider->setRange(-1000,2000);
    y_slider->setValue(1);
    y_slider->setOrientation(Qt::Horizontal);
    y_slider->setTickPosition(QSlider::TicksAbove);

    confirm_button=new QPushButton();
    confirm_button->setText("Save");


    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(new QLabel("x location"),0,0);
    layout->addWidget(x_location_spinbox, 0,1,1,5);
    layout->addWidget(x_slider, 1,0,1,5);

    layout->addWidget(new QLabel("y location"),2,0);
    layout->addWidget(y_location_spinbox, 2,1,1,5);
    layout->addWidget(y_slider, 3,0,1,5);



    layout->addWidget(confirm_button,10,0);
//
//    this->
    this->setLayout(layout);

    QObject::connect(x_location_spinbox, SIGNAL(valueChanged(int)), x_slider, SLOT(setValue(int)));
    QObject::connect(y_location_spinbox, SIGNAL(valueChanged(int)), y_slider, SLOT(setValue(int)));
    QObject::connect(x_slider, SIGNAL(valueChanged(int)), x_location_spinbox, SLOT(setValue(int)));
    QObject::connect(y_slider, SIGNAL(valueChanged(int)), y_location_spinbox, SLOT(setValue(int)));
    QObject::connect(x_location_spinbox, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanger(int)));
    QObject::connect(y_location_spinbox, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanger(int)));
    connect(confirm_button, SIGNAL(clicked()), this, SLOT(save()));

}

void loc_input::setV3DPluginCallback2(V3DPluginCallback2 *callback2)
{
    if(callback2==NULL)
    {cout<<"input a wrong V3DPluginCallback2"<<endl;
        v3d_msg("input a wrong V3DPluginCallback2");
        return;}
    callback=callback2;
    cout<<"catch callback success"<<endl;

    curwin = callback->currentImageWindow();
    p4DImage = callback->getImage(curwin);
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();
    x_location_spinbox->setRange(-sz[0],2*sz[0]);
    x_location_spinbox->setValue(x_coordinate);
    y_location_spinbox->setRange(-sz[1],2*sz[1]);
    y_location_spinbox->setValue(y_coordinate);
    z_location_spinbox->setRange(-sz[2],2*sz[2]);
    z_location_spinbox->setValue(z_coordinate);

    x_slider->setRange(-sz[0],2*sz[0]);
    x_slider->setValue(x_coordinate);

    y_slider->setRange(-sz[1],2*sz[1]);
    y_slider->setValue(y_coordinate);


}

void loc_input::show_area()
{
    V3DLONG xb = x_slider->value();
    V3DLONG xe = x_slider->value()+640;
    V3DLONG yb = y_slider->value();
    V3DLONG ye = y_slider->value()+640;
    LocationSimple markerLeftUp,markerRightUp,markerLeftDown,markerRightDown;
    markerLeftUp.x = xb; markerLeftUp.y = yb;
    markerRightUp.x = xe; markerRightUp.y = yb;
    markerLeftDown.x = xb; markerLeftDown.y = ye;
    markerRightDown.x = xe; markerRightDown.y = ye;
    area.clear();
    area.push_back(markerLeftUp);
    area.push_back(markerRightUp);
    area.push_back(markerLeftDown);
    area.push_back(markerRightDown);
    callback->setLandmark(callback->currentImageWindow(),area);
    callback->pushObjectIn3DWindow(callback->currentImageWindow());
}

void loc_input::sliderValueChanger(int value)
{
    x_slider_label->setText(QString::number(x_slider->value()));
    y_slider_label->setText(QString::number(y_slider->value()));
    this->show_area();
//    cout<<"sliderValueChanger "<<score_slider->value()<<endl;
//    emit signal_slider_valueChanged(value);
}

void loc_input::save()
{
    v3dhandle curwin = callback->currentImageWindow();
    Image4DSimple *p4DImage = callback->getImage(curwin);

    V3DLONG sz[4];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();
    sz[3] = p4DImage->getCDim();

    unsigned char *data1d=0;
    data1d = p4DImage->getRawData();
    V3DLONG nx=sz[0];
    V3DLONG ny=sz[1];
    V3DLONG nz=sz[2];

    V3DLONG x_length = 640;
    V3DLONG y_length = 640;
    V3DLONG z_length = 1;

    unsigned char *matching_img=0;
    try{matching_img=new unsigned char [nx*ny*nz];}
    catch(...) {v3d_msg("cannot allocate memory for matching_img."); return ;}

    unsigned char *sub = new unsigned char [nx*ny*nz];
    unsigned char *matching_sub = new unsigned char [nx*ny*nz];

    //***************get the location of a marker
    LocationSimple marker;
    marker.x = x_slider->value();
    marker.y = y_slider->value();
    marker.z = 1;

    QString outimg_dir = "";
    QString file_name = QString(p4DImage->getFileName());
    QFileInfo info(file_name);
    QString default_name = info.baseName()+"_sub";
    QStringList fileNameList1 = default_name.split("_");
    QString matching_img_name = "";
    for(V3DLONG i = 0; i < fileNameList1.size(); i++)
    {
        if(fileNameList1.at(i)=="label")
        {
            matching_img_name = info.canonicalPath()+"\\"+fileNameList1.at(0)+"_mip.tif";
            if(!QFileInfo(matching_img_name).isFile())
            {
                v3d_msg("NO image for the label.",0);
                if(matching_img)
                {   delete []matching_img; matching_img=0;}
                break;
            }
            else
            {
                matching_img = callback->loadImage(matching_img_name.toLocal8Bit().data())->getRawData();
            }
            break;
        }
        else if(fileNameList1.at(i)=="mip")
        {
            matching_img_name = info.canonicalPath()+"\\"+fileNameList1.at(0)+"_cross2Dgt_label.tif";
            if(!QFileInfo(matching_img_name).isFile())
            {
                v3d_msg("NO label for the image.",0);
                if(matching_img)
                {   delete []matching_img; matching_img=0;}
                break;
            }
            else
            {
                matching_img = callback->loadImage(matching_img_name.toLocal8Bit().data())->getRawData();
            }
            break;
        }
    }


    if(0)
    {
        outimg_dir = QFileDialog::getExistingDirectory(0,
                                      "Choose a dir to save file " );
    }
    else
    {
        outimg_dir = "C:\\Users\\Administrator\\Desktop\\cross_groundtruth20190619\\mouse_RGC_cross_groundtruth\\sample";
    }


    V3DLONG im_cropped_sz[4];
    LocationSimple t;

    QString outimg_file = outimg_dir + "\\" + default_name +".tif";
    if(nz!=1)
    {
        t.x = marker.x;
        t.y = marker.y;
        t.z = marker.z;

        V3DLONG xb = t.x;
        V3DLONG xe = t.x+x_length-1;
        V3DLONG yb = t.y;
        V3DLONG ye = t.y+y_length-1;
        V3DLONG zb = t.z;
        V3DLONG ze = t.z+z_length-1;


        im_cropped_sz[0] = xe-xb+1;
        im_cropped_sz[1] = ye-yb+1;
        im_cropped_sz[2] = ze-zb+1;
        im_cropped_sz[3] = 1;

        V3DLONG num_size = 0;
        for(V3DLONG i = zb; i <= ze; i++)
        {
            V3DLONG z_location = i*nx*ny;
            for(V3DLONG j = yb; j <= ye; j++)
            {
                V3DLONG y_location = j*nx;
                for(V3DLONG k = xb; k <= xe; k++)
                {
                    if (i<0||i>=nz||j<0||j>=ny||k<0||k>=nx)
                        sub[num_size] = 0;
                    else
                        sub[num_size] = data1d[z_location+y_location+k];
                    num_size++;
                }
            }
        }
    }
    else
    {
        t.x = marker.x;
        t.y = marker.y;
        t.z = 1;

        V3DLONG xb = t.x;
        V3DLONG xe = t.x+x_length-1;
        V3DLONG yb = t.y;
        V3DLONG ye = t.y+y_length-1;
        V3DLONG zb = 1;
        V3DLONG ze = 1;


        im_cropped_sz[0] = xe-xb+1;
        im_cropped_sz[1] = ye-yb+1;
        im_cropped_sz[2] = ze-zb+1;
        im_cropped_sz[3] = 1;

        V3DLONG num_size = 0;
        for(V3DLONG i = zb; i <= ze; i++)
        {
            V3DLONG z_location = 0;
            for(V3DLONG j = yb; j <= ye; j++)
            {
                V3DLONG y_location = j*nx;
                for(V3DLONG k = xb; k <= xe; k++)
                {
                    if (j<0||j>=ny||k<0||k>=nx)
                        sub[num_size] = 0;
                    else
                        sub[num_size] = data1d[z_location+y_location+k];
                    num_size++;
                }
            }
        }
        simple_saveimage_wrapper(*callback, outimg_file.toStdString().c_str(),(unsigned char *)sub,im_cropped_sz,1);
        //v3d_msg(QString("The center of subblock is (%1 %2 %3)").arg(t.x).arg(t.y).arg(t.z));
    }

    if(matching_img!=0)
    {
        QFileInfo matching_info(matching_img_name);
        QString matching_outimg_file = outimg_dir + "\\" + matching_info.baseName() +"_sub.tif";
        if(nz!=1)
        {
            t.x = marker.x;
            t.y = marker.y;
            t.z = marker.z;

            V3DLONG xb = t.x;
            V3DLONG xe = t.x+x_length-1;
            V3DLONG yb = t.y;
            V3DLONG ye = t.y+y_length-1;
            V3DLONG zb = t.z;
            V3DLONG ze = t.z+z_length-1;


            im_cropped_sz[0] = xe-xb+1;
            im_cropped_sz[1] = ye-yb+1;
            im_cropped_sz[2] = ze-zb+1;
            im_cropped_sz[3] = 1;

            V3DLONG num_size = 0;
            for(V3DLONG i = zb; i <= ze; i++)
            {
                V3DLONG z_location = i*nx*ny;
                for(V3DLONG j = yb; j <= ye; j++)
                {
                    V3DLONG y_location = j*nx;
                    for(V3DLONG k = xb; k <= xe; k++)
                    {
                        if (i<0||i>=nz||j<0||j>=ny||k<0||k>=nx)
                            matching_sub[num_size] = 0;
                        else
                            matching_sub[num_size] = matching_img[z_location+y_location+k];
                        num_size++;
                    }
                }
            }
        }
        else
        {
            t.x = marker.x;
            t.y = marker.y;
            t.z = 1;

            V3DLONG xb = t.x;
            V3DLONG xe = t.x+x_length-1;
            V3DLONG yb = t.y;
            V3DLONG ye = t.y+y_length-1;
            V3DLONG zb = 1;
            V3DLONG ze = 1;


            im_cropped_sz[0] = xe-xb+1;
            im_cropped_sz[1] = ye-yb+1;
            im_cropped_sz[2] = ze-zb+1;
            im_cropped_sz[3] = 1;

            V3DLONG num_size = 0;
            for(V3DLONG i = zb; i <= ze; i++)
            {
                V3DLONG z_location = 0;
                for(V3DLONG j = yb; j <= ye; j++)
                {
                    V3DLONG y_location = j*nx;
                    for(V3DLONG k = xb; k <= xe; k++)
                    {
                        if (j<0||j>=ny||k<0||k>=nx)
                            matching_sub[num_size] = 0;
                        else
                            matching_sub[num_size] = matching_img[z_location+y_location+k];
                        num_size++;
                    }
                }
            }
            simple_saveimage_wrapper(*callback, matching_outimg_file.toStdString().c_str(),(unsigned char *)matching_sub,im_cropped_sz,1);
            //v3d_msg(QString("The center of subblock is (%1 %2 %3)").arg(t.x).arg(t.y).arg(t.z));
        }
    }
    else
    {
        v3d_msg("No matching image.",0);
    }

}
