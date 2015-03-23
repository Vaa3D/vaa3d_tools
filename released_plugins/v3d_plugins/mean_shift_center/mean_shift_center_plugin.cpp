/* Mean_Shift_Center_plugin.cpp
 * Search for center using mean-shift
 * 2015-3-4 : by Yujie Li
 */
 

#include "mean_shift_center_plugin.h"


using namespace std;
Q_EXPORT_PLUGIN2(mean_shift_center,mean_shift_plugin );
//static mean_shift_dialog *dialog=0;
//static ray_shoot_dialog *r_dialog=0;

QStringList mean_shift_plugin::menulist() const
{
	return QStringList() 
        <<tr("mean_shift")
        <<tr("mean_shift_with_constraints")
        <<tr("ray_shoot")
        <<tr("gradient_distance_transform + mean_shift_with_constraints")
        <<tr("all_method_comparison")
		<<tr("about");
}

QStringList mean_shift_plugin::funclist() const
{
	return QStringList()
        <<tr("mean_shift_center_finder")
		<<tr("help");
}

void mean_shift_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("mean_shift"))
	{
        mean_shift_dialog *dialog=new mean_shift_dialog(&callback,0);
        dialog->core();
        //dialog->create_sphere();
	}
    else if (menu_name == tr("mean_shift_with_constraints"))
    {
        mean_shift_dialog *dialog=new mean_shift_dialog(&callback,2); //methodcode=2 mean_shift with constraints
        dialog->core();
        //dialog->create_sphere();
    }
    else if (menu_name==tr("ray_shoot"))
	{
        ray_shoot_dialog *r_dialog=new ray_shoot_dialog(&callback);
        r_dialog->core();
    }
    else if (menu_name==tr("gradient_distance_transform + mean_shift_with_constraints"))
    {
        gradient_transform_dialog *g_dialog=new gradient_transform_dialog(&callback);
        g_dialog->core();
    }
    else if (menu_name==tr("all_method_comparison"))
    {
        all_method_comp(&callback);
//        QString myfile="comparision.marker";
//        FILE * fp_open = fopen(myfile.toLatin1(), "w");
//        fprintf(fp_open,"##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n");
//        LandmarkList LList_m;

//        //method 1:mean_shift
//        mean_shift_dialog *dialog=new mean_shift_dialog(&callback,0);
//        LList_m=dialog->core();
//        for(int i=0; i<LList_m.size(); i++){
//            fprintf(fp_open,"%5.3f,%5.3f,%5.3f,%1d,%1d,%s,%s,%d,%d,%d\n",LList_m.at(i).x,
//                    LList_m.at(i).y,LList_m.at(i).z,0,1,"ms","",196,0,0);
//        }
//        //method 2: mean_shift_constraints
//        mean_shift_dialog *dialog1=new mean_shift_dialog(&callback,2); //methodcode=2 mean_shift with constraints
//        LList_m.clear();
//        LList_m=dialog1->core();
//        for(int i=0; i<LList_m.size(); i++){
//            fprintf(fp_open,"%5.3f,%5.3f,%5.3f,%1d,%1d,%s,%s,%d,%d,%d\n",LList_m.at(i).x,
//                    LList_m.at(i).y,LList_m.at(i).z,0,1,"ms_contr","",0,196,0);
//        }
//        fclose(fp_open);

    }
    else{
    QMessageBox::about(0,"center_finder","The <b>center finder</b> provides several ways to find the center of mass.<p>"
                       "<b>mean_shift </b>calculates the mean in a sphere around each of the user-input markers and shifts mean till the center converges.<br>"
                   "<b>mean_shift_with_constraint</b> adds two conditions to mean_shift. 1) The new center can only move to a location of "
                   "higher intensity. 2) If the new mean is smaller than the previous mean, search window radius will decrease by one. <br>"
                   "<b>ray shoot</b> casts 48 rays toward different angles from each of the marker to find the cell edge and then the geometric center.<br> "
                   "<b>gradient_distance_transform + mean_shift_with_constraints</b> first performs gradient distance transform in a cube around "
                   "each of the marker and then conducts mean shift with constraint on the area."
             "<p>For further questions, please contact Yujie Li at yujie.jade@gmail.com)<p>");
	}
}

bool mean_shift_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("mean_shift_center_finder"))
    {
        mean_shift_center(callback, input, output);
	}
	else if (func_name == tr("help"))
	{
        printHelp();
	}
	else return false;

	return true;
}

void mean_shift_plugin::all_method_comp(V3DPluginCallback2 *callback)
{
    v3dhandle curwin;
    V3DLONG sz_img[4];
    unsigned char *image1Dc_in;
    LandmarkList LList,LList_final;
    vector<V3DLONG> poss_landmark;
    vector<float> mass_center;
    ImagePixelType pixeltype;
    mean_shift_fun mean_shift_obj;

    if (!load_data(callback,image1Dc_in,LList,pixeltype,sz_img,curwin))
        return;
    V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

    if(pixeltype==1)//V3D_UNIT8
    {
        mean_shift_obj.pushNewData<unsigned char>((unsigned char*)image1Dc_in, sz_img);
    }

    else if (pixeltype == 2) //V3D_UINT16;
    {
        mean_shift_obj.pushNewData<unsigned short>((unsigned short*)image1Dc_in, sz_img);
        convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else if(pixeltype == 4) //V3D_FLOAT32;
    {
        mean_shift_obj.pushNewData<float>((float*)image1Dc_in, sz_img);
        convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
    }
    else
    {
       QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
       return;
    }

    QDialog *mydialog_para=new QDialog;
    QGridLayout *layout2=new QGridLayout;
    mydialog_para->setWindowTitle("Parameter setting");
    QLabel *label_m1=new QLabel;
    label_m1->setText("mean_shift w/o constraints");
    QLabel *label_radius=new QLabel;
    label_radius->setText("Search window radius:");
    QSpinBox *para_radius=new QSpinBox;
    para_radius->setRange(2,30);
    para_radius->setValue(15);
    layout2->addWidget(label_m1,0,0,1,2);
    layout2->addWidget(label_radius,1,0,1,1);
    layout2->addWidget(para_radius,1,1,1,1);
    QFrame *line1=new QFrame;
    line1->setFrameShape(QFrame::HLine);
    layout2->addWidget(line1,2,0,1,2);

    QLabel *label_m2=new QLabel;
    label_m2->setText("ray_shoot");
    QLabel *label_bg=new QLabel;
    label_bg->setText("Background threshold:");
    QSpinBox *para_bg=new QSpinBox;
    para_bg->setRange(0,255);
    para_bg->setValue(70);
    layout2->addWidget(label_m2,3,0,1,2);
    layout2->addWidget(label_bg,4,0,1,1);
    layout2->addWidget(para_bg,4,1,1,1);
    QFrame *line2=new QFrame;
    line2->setFrameShape(QFrame::HLine);
    //line2->setFrameShadow(QFrame::Shadow);
    layout2->addWidget(line2,5,0,1,2);

    QLabel *label_m3=new QLabel;
    label_m3->setText("gradient transform");
    QLabel *label_g_bg=new QLabel;
    label_g_bg->setText("Background threshold (0-255):");
    QSpinBox *para_g_bg=new QSpinBox;
    para_g_bg->setRange(0,255);
    para_g_bg->setValue(70);

    QLabel *label_cnn=new QLabel;
    label_cnn->setText("Connection type (1-3):");
    QSpinBox *para_cnn=new QSpinBox;
    para_cnn->setRange(1,3);
    para_cnn->setValue(1);

    QLabel *label_z=new QLabel;
    label_z->setText("z_thickness:");
    QSpinBox *para_z=new QSpinBox;
    para_z->setRange(1,10);
    para_z->setValue(1);

    int min_dim=MIN(sz_img[0],sz_img[1]);
    if (sz_img[2]<min_dim) min_dim=sz_img[2];
    QLabel *label_tran_win=new QLabel;
    label_tran_win->setText("Gradient distance transform half window size:");
    QSpinBox *para_tran_win=new QSpinBox;
    para_tran_win->setRange(10,min_dim/2);
    para_tran_win->setValue(40);

    QLabel *label_search_r=new QLabel;
    label_search_r->setText("Mean shift search window radius (2-30):");
    QSpinBox *para_search_r=new QSpinBox;
    para_search_r->setRange(2,30);
    para_search_r->setValue(15);

    layout2->addWidget(label_m3,6,0,1,2);
    layout2->addWidget(label_g_bg,7,0,1,1);
    layout2->addWidget(para_g_bg,7,1,1,1);
    layout2->addWidget(label_cnn,8,0,1,1);
    layout2->addWidget(para_cnn,8,1,1,1);
    layout2->addWidget(label_z,9,0,1,1);
    layout2->addWidget(para_z,9,1,1,1);
    layout2->addWidget(label_tran_win,10,0,1,1);
    layout2->addWidget(para_tran_win,10,1,1,1);
    layout2->addWidget(label_search_r,11,0,1,1);
    layout2->addWidget(para_search_r,11,1,1,1);

    QPushButton *button_p_ok=new QPushButton;
    button_p_ok->setText("Ok");
    button_p_ok->setFixedWidth(100);
    QPushButton *button_p_cancel=new QPushButton;
    button_p_cancel->setText("Cancel");
    button_p_cancel->setFixedWidth(100);
    layout2->addWidget(button_p_ok,12,0,1,1);
    layout2->addWidget(button_p_cancel,12,1,1,1);
    connect(button_p_ok,SIGNAL(clicked()),mydialog_para,SLOT(accept()));
    connect(button_p_cancel,SIGNAL(clicked()),mydialog_para,SLOT(reject()));

    mydialog_para->setLayout(layout2);
    int ms_windowradius=15; //mean_shift
    int rs_bg_thr=70; //ray_shoot
    int gt_bg_thr=70; //gradient transform
    int connectiontype=1;
    int z_thickness=1;
    int transform_half_window=40;
    int gt_window_radius=15;

    mydialog_para->exec();
    if (mydialog_para->result()==QDialog::Accepted){
        ms_windowradius=para_radius->value();
        rs_bg_thr=para_bg->value();
        gt_bg_thr=para_g_bg->value();
        connectiontype=para_cnn->value();
        z_thickness=para_z->value();
        transform_half_window=para_tran_win->value();
        gt_window_radius=para_search_r->value();
    }
    else
        return;
    poss_landmark.clear();
    poss_landmark=landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);

    QString myfile="comparision.marker";
    FILE * fp_open = fopen(myfile.toLatin1(), "w");
    fprintf(fp_open,"##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n");


    for (int j=0;j<poss_landmark.size();j++)
    {
        //original landmark
        LList[j].name="ori";
        LList[j].color.r=255; LList[j].color.g=LList[j].color.b=0;
        LList_final.append(LList.at(j));

        //mean_shift
        mass_center=mean_shift_obj.mean_shift_center(poss_landmark[j],ms_windowradius);
        LocationSimple tmp(mass_center[0]+1.0,mass_center[1]+1.0,mass_center[2]+1.0);
        tmp.color.r=170; tmp.color.g=0; tmp.color.b=255;
        tmp.name="ms";
        LList_final.append(tmp);
        fprintf(fp_open,"%5.3f,%5.3f,%5.3f,%1d,%1d,%s,%s,%d,%d,%d\n",mass_center[0]+1.0,
                mass_center[1]+1.0,mass_center[2]+1.0,0,1,"ms","",170,0,255);

        //mean_shift_constraints
        mass_center=mean_shift_obj.mean_shift_with_constraint(poss_landmark[j],ms_windowradius);
        LocationSimple tmp1(mass_center[0]+1.0,mass_center[1]+1.0,mass_center[2]+1.0);
        tmp1.color.r=0; tmp1.color.g=170; tmp1.color.b=255;
        tmp1.name="ms_c";
        LList_final.append(tmp1);
        fprintf(fp_open,"%5.3f,%5.3f,%5.3f,%1d,%1d,%s,%s,%d,%d,%d\n",mass_center[0]+1,
                mass_center[1]+1,mass_center[2]+1,0,1,"ms_c","",0,170,255);
        //ray shoot
        mass_center=mean_shift_obj.ray_shoot_center(poss_landmark[j],rs_bg_thr,j);
        LocationSimple tmp2 (mass_center[0]+1.0,mass_center[1]+1.0,mass_center[2]+1.0);
        tmp2.color.r=0; tmp2.color.g=170; tmp2.color.b=127;
        tmp2.name="ray";
        LList_final.append(tmp2);
        fprintf(fp_open,"%5.3f,%5.3f,%5.3f,%1d,%1d,%s,%s,%d,%d,%d\n",mass_center[0]+1.0,
                mass_center[1]+1.0,mass_center[2]+1.0,0,1,"ray","",0,170,127);
        //gradient
        float *outimg1d=0;
        mass_center=mean_shift_obj.gradient_transform(outimg1d,poss_landmark[j],gt_bg_thr,connectiontype,
                                          z_thickness,transform_half_window,gt_window_radius);
        LocationSimple tmp3(mass_center[0]+1.5,mass_center[1]+1.5,mass_center[2]+1.5);
        tmp3.color.r=255; tmp3.color.g=255; tmp3.color.b=0;
        tmp3.name="gradient";
        LList_final.append(tmp3);

        fprintf(fp_open,"%5.3f,%5.3f,%5.3f,%1d,%1d,%s,%s,%d,%d,%d\n",mass_center[0]+1,
                mass_center[1]+1,mass_center[2]+1,0,1,"gradient","",255,255,0);

    }

    fclose(fp_open);
    callback->setLandmark(curwin,LList_final);
    callback->updateImageWindow(curwin);
    //callback->open3DWindow(curwin);
    callback->pushObjectIn3DWindow(curwin);
    v3d_msg("Computation complete. Markers loaded");
}

void mean_shift_plugin::mean_shift_center(V3DPluginCallback2 & callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
    mean_shift_fun fun_obj;
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if ((infiles.size()!=2))
    {
        qDebug()<<"ERROR: please set input and output! "<<infiles.size()<<":"<<outfiles.size();
        return;
    }
    if (inparas.size() != 0 && inparas.size() != 1)
    {
        qDebug()<<"ERROR: please give the parameter of window radius (0-50) or use the default value of 10! "
               <<inparas.size();
        return;
    }


    QString qs_input_image(infiles[0]);
    QString qs_input_mark(infiles[1]);
    image_data=0;
    int intype=0;
    int windowradius=10;

    if (inparas.size()==1)
    {
        int tmp=atoi(inparas.at(0));
        if (tmp>0 && tmp<=50)
        windowradius=tmp;
        else
            v3d_msg("The parameter of window radius is not valid, the program will use default value of 10",0);
    }

    if (!qs_input_image.isEmpty())
    {
        if (!simple_loadimage_wrapper(callback, qs_input_image.toStdString().c_str(), image_data, sz_img, intype))
        {
            qDebug()<<"Loading error";
            return;
        }
        if (sz_img[3]>3)
        {
            sz_img[3]=3;
            v3d_msg("More than 3 channels were loaded."
                                     " The first 3 channel will be applied for analysis.",0);
            return;
        }

        V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        if(intype!=1)
        {
            if (intype == 2) //V3D_UINT16;
            {
                convert2UINT8((unsigned short*)image_data, image_data, size_tmp);
            }
            else if(intype == 4) //V3D_FLOAT32;
            {
                convert2UINT8((float*)image_data, image_data, size_tmp);
            }
            else
            {
                v3d_msg("Currently this program only supports UINT8, UINT16, and FLOAT32 data type.",0);
                return;
            }
        }
    }
    fun_obj.pushNewData(image_data,sz_img);

    if (qs_input_mark.isEmpty())
        return;
    FILE * fp = fopen(qs_input_mark.toAscii(), "r");
    if (!fp)
    {
        qDebug()<<"Can not open the file to load the landmark points.\n";
        return;
    }
    else
    {
        fclose(fp);
    }

    QList <LocationSimple> tmpList = readPosFile_usingMarkerCode(qs_input_mark.toAscii()); //revised on 090725 to use the unique interface

    if (tmpList.count()<=0)
    {
        v3d_msg("Did not find any valid row/record of the markers. Thus do not overwrite the current landmarks if they exist.\n",0);
        return;
    }
    qDebug()<<"tmpList size:"<<tmpList.count();

    vector<V3DLONG> poss_landmark;
    poss_landmark=landMarkList2poss(tmpList, sz_img[0], sz_img[0]*sz_img[1]);
    QList <LocationSimple> LList_new_center;
    vector<float> mass_center;

    for (int j=0;j<poss_landmark.size();j++)
    {
        mass_center=fun_obj.mean_shift_center(poss_landmark[j],windowradius);
                //calc_mean_shift_center(poss_landmark[j],windowradius);
        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        LList_new_center.append(tmp);
    }

    qDebug()<<"LList_new_center_size:"<<LList_new_center.size();
    //Write data in the file
    QString qs_output = outfiles.empty() ? qs_input_image + "_out.marker" : QString(outfiles[0]);

    FILE * fp_1 = fopen(qs_output.toAscii(), "w");
    if (!fp_1)
    {
        qDebug()<<"can not open the file to save the landmark points.\n";
        return;
    }

    fprintf(fp_1, "#x, y, z, radius, shape, name, comment\n");
    for (int i=0;i<LList_new_center.size(); i++)
    {
        fprintf(fp_1, "%ld,%ld,%ld,%ld,%ld,%s,%s\n",
                V3DLONG(LList_new_center.at(i).x), V3DLONG(LList_new_center.at(i).y), V3DLONG(LList_new_center.at(i).z),
                V3DLONG(LList_new_center.at(i).radius), V3DLONG(LList_new_center.at(i).shape),
                LList_new_center.at(i).name.c_str(), LList_new_center.at(i).comments.c_str());
        qDebug()<<"Input one line";
    }

    fclose(fp_1);

    if (image_data!=0) {delete []image_data; image_data=0;}

}


QList <LocationSimple> mean_shift_plugin::readPosFile_usingMarkerCode(const char * posFile) //last update 090725
{
    QList <LocationSimple> coordPos;
    QList <ImageMarker> tmpList = readMarker_file(posFile);

    if (tmpList.count()<=0)
        return coordPos;

    coordPos.clear();
    for (int i=0;i<tmpList.count();i++)
    {
        LocationSimple pos;
        pos.x = tmpList.at(i).x;
        pos.y = tmpList.at(i).y;
        pos.z = tmpList.at(i).z;
        pos.radius = tmpList.at(i).radius;
        pos.shape = (PxLocationMarkerShape)(tmpList.at(i).shape);
        pos.name = (string)(qPrintable(tmpList.at(i).name));
        pos.comments = (string)(qPrintable(tmpList.at(i).comment));

        coordPos.append(pos);
    }

    return coordPos;
}

void mean_shift_plugin::printHelp()
{
    printf("\nmean_shift_center_finder -needs two input files- 1) image 2) marker file   -optional parameter:search window radius\n");
    printf("Usage v3d -x mean_shift_center_finder -f mean_shift_center_finder -i <input.v3draw> <input.v3draw.marker> [-p <int>(0-50)] [-o <output_image.marker>]\n");
}

QList <ImageMarker> mean_shift_plugin::readMarker_file(const QString & filename)
{
    QList <ImageMarker> tmp_list;

    QFile qf(filename);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(filename),0);
#endif
        return tmp_list;
    }

    V3DLONG k=0;
    while (! qf.atEnd())
    {
        char curline[2000];
        qf.readLine(curline, sizeof(curline));
        k++;
        {
            if (curline[0]=='#' || curline[0]=='x' || curline[0]=='X' || curline[0]=='\0') continue;

            QStringList qsl = QString(curline).trimmed().split(",");
            int qsl_count=qsl.size();
            if (qsl_count<3)   continue;

            ImageMarker S;

            S.x = qsl[0].toFloat();
            S.y = qsl[1].toFloat();
            S.z = qsl[2].toFloat();
            S.radius = (qsl_count>=4) ? qsl[3].toInt() : 0;
            S.shape = (qsl_count>=5) ? qsl[4].toInt() : 1;
            S.name = (qsl_count>=6) ? qPrintable(qsl[5].trimmed()) : "";
            S.comment = (qsl_count>=7) ? qPrintable(qsl[6].trimmed()) : "";

            S.color = random_rgba8(255);
            if (qsl_count>=8) S.color.r = qsl[7].toUInt();
            if (qsl_count>=9) S.color.g = qsl[8].toUInt();
            if (qsl_count>=10) S.color.b = qsl[9].toUInt();

            S.type = (S.x==-1 || S.y==-1 || S.z==-1) ? 0 : 2;

            S.on = true; //listLoc[i].on;        //true;
            S.selected = false;

            tmp_list.append(S);
        }
    }

    return tmp_list;
}
