/* get_sub_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-10-16 : by guochanghao
 */
 
#include "v3d_message.h"
#include <vector>
#include "get_sub_plugin.h"
#include <iostream>
#include "basic_surf_objs.h"
#include "my_surf_objs.h"
#include <QFileInfo>
#include "loc_input.h"

using namespace std;
Q_EXPORT_PLUGIN2(get_sub, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("get_sub_block_auto")
        <<tr("get_sub_block")
        <<tr("get_sub_by_marker_as_LeftUpCorner")
        <<tr("get_sub_by_input_LeftUpCorner")
        <<tr("get_sub_by_show_boundary")
		<<tr("get_sub_terafly")
        <<tr("get_sub_terafly_and_swc")
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
    if (menu_name == tr("get_sub_block_auto"))
    {
        int flag = get_sub_auto(callback,parent);
        if (flag == 1)
        {
            cout<<endl;
            cout<<"******************************"<<endl;
            cout<<"*get_sub_block_auto finished!*"<<endl;
            cout<<"******************************"<<endl;
        }
        else
        {   v3d_msg("menu flag != 1"); return;}
    }
    else if (menu_name == tr("get_sub_block"))
    {
        int flag = get_sub(callback,parent);
        if (flag == 1)
        {
            cout<<endl;
            cout<<"*************************"<<endl;
            cout<<"*get_sub_block finished!*"<<endl;
            cout<<"*************************"<<endl;
        }
        else
        {   v3d_msg("menu flag != 1"); return;}
    }
    else if (menu_name == tr("get_sub_by_marker_as_LeftUpCorner"))
    {
        int flag = get_sub_by_marker_as_LeftUpCorner(callback,parent);
        if (flag == 1)
        {
            cout<<endl;
            cout<<"*********************************************"<<endl;
            cout<<"*get_sub_by_marker_as_LeftUpCorner finished!*"<<endl;
            cout<<"*********************************************"<<endl;
        }
        else
        {   v3d_msg("menu flag != 1"); return;}
    }
    else if (menu_name == tr("get_sub_by_input_LeftUpCorner"))
    {
        int flag = get_sub_by_input_LeftUpCorner(callback,parent);
        if (flag == 1)
        {
            cout<<endl;
            cout<<"******************************"<<endl;
            cout<<"*get_sub_by_input_LeftUpCorner finished!*"<<endl;
            cout<<"******************************"<<endl;
        }
        else
        {   v3d_msg("menu flag != 1"); return;}
    }
    else if (menu_name == tr("get_sub_by_show_boundary"))
    {
        int flag = get_sub_by_show_boundary(callback,parent);
        if (flag == 1)
        {
            cout<<endl;
            cout<<"************************************"<<endl;
            cout<<"*get_sub_by_show_boundary finished!*"<<endl;
            cout<<"************************************"<<endl;
        }
        else
        {   v3d_msg("menu flag != 1"); return;}
    }
	else if (menu_name == tr("get_sub_terafly"))
	{
        int flag = get_sub_terafly(callback,parent);
        if (flag == 1)
        {
            cout<<endl;
            cout<<"***************************"<<endl;
            cout<<"*get_sub_terafly finished!*"<<endl;
            cout<<"***************************"<<endl;
        }
        else
        {   v3d_msg("menu flag != 1"); return;}
	}
    else if (menu_name == tr("get_sub_terafly_and_swc"))
    {
        int flag = get_sub_terafly_and_swc(callback,parent);
        if (flag == 1)
        {
            cout<<endl;
            cout<<"************************************"<<endl;
            cout<<"*get_sub_terafly_and_swc finished!*"<<endl;
            cout<<"************************************"<<endl;
        }
        else
        {   v3d_msg("menu flag != 1"); return;}
    }
    else
	{
        v3d_msg(tr("1.get_sub_block_auto:\nAutomatically save all sub-images in the same dir. Each marker is a center of a sub-image.\n"
                   "2.get_sub_block:\nMannually save each sub-image in its own dir. Each marker is a center of a sub-image.\n"
                   "3.get_sub_by_marker_as_LeftUpCorner:\nAutomaticlly save all sub-images in the same dir. Each marker is a LeftUpCorner of a sub-image.\n"
                   "4.get_sub_by_input_LeftUpCorner:\ninput one point's pozition. The point is a LeftUpCorner of the sub-image.\n"
                   "5.get_sub_by_show_boundary:\nYou could see the four corners of the sub-image that will be obtained.\n"
                   "6.get_sub_terafly:\nFor teraFly mode. You could get sub-images. Each marker is a center of a sub-image\n"
                   "7.get_sub_terafly_and_swc:\nFor teraFly mode. You could get sub-images and sub-swcs at the same time. Each marker is a center of a pair of sub-image and sub-swc.\n"
			"Developed by guochanghao, 2018-10-16"));
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

LandmarkList QList_ImageMarker2LandmarkList(QList <ImageMarker> markers)
{
    LandmarkList marker_l;
    for(V3DLONG i=0;i<markers.size();i++)
    {
        LocationSimple temp;
        temp.x=markers.at(i).x;
        temp.y=markers.at(i).y;
        temp.z=markers.at(i).z;
        temp.z=markers.at(i).z;
        temp.radius=markers.at(i).radius;
        temp.shape=PxLocationMarkerShape(markers.at(i).shape);
        temp.name=markers.at(i).name.toStdString();
        temp.color.r=markers.at(i).color.r;
        temp.color.g=markers.at(i).color.g;
        temp.color.b=markers.at(i).color.b;
        marker_l.push_back(temp);
    }
    return marker_l;

}

bool saveSWC_file1(string swc_file, vector<MyMarker*> & outMarker1s)
{
    if(swc_file.find_last_of(".dot") == swc_file.size() - 1) return saveDot_file(swc_file, outMarker1s);
    map<MyMarker*, int> ind;
    ofstream ofs(swc_file.c_str());
    if(ofs.fail())
    {
        cout<<"open swc file error"<<endl;
        return false;
    }
    ofs<<"#name "<<swc_file<<endl;
    ofs<<"##n,type,x,y,z,radius,parent"<<endl;
    for(int i = 0; i < outMarker1s.size(); i++) ind[outMarker1s[i]] = i+1;

    for(int i = 0; i < outMarker1s.size(); i++)
    {
        MyMarker * marker = outMarker1s[i];
        int parent_id;
        if(marker->parent == 0) parent_id = -1;
        else
        {
            parent_id = ind[marker->parent];
            if(parent_id==0)
            {   parent_id = -1;}
        }
        ofs<<i+1<<" "<<marker->type<<" "<<marker->x<<" "<<marker->y<<" "<<marker->z<<" "<<marker->radius<<" "<<parent_id<<endl;
    }
    ofs.close();
    return true;
}
bool writeSWC_file1(const QString& filename, const NeuronTree& nt)
{
    QString curFile = filename;
    if (curFile.trimmed().isEmpty()) //then open a file dialog to choose file
    {
        curFile = QFileDialog::getSaveFileName(0,
                                               "Select a SWC file to save the neuronal or relational data... ",
                                               ".swc",
                                               QObject::tr("Neuron structure file (*.swc);;(*.*)"
                                                           ));
        if (curFile.isEmpty()) //note that I used isEmpty() instead of isNull
            return false;
    }

    FILE * fp = fopen(curFile.toLatin1(), "wt");
    if (!fp)
    {
        return false;
    }
    fprintf(fp, "#name %s\n", qPrintable(nt.name.trimmed()));
    fprintf(fp, "#comment %s\n", qPrintable(nt.comment.trimmed()));
    fprintf(fp, "##n,type,x,y,z,radius,parent\n");
    NeuronSWC * p_pt=0;
    for (int i=0;i<nt.listNeuron.size(); i++)
    {
        p_pt = (NeuronSWC *)(&(nt.listNeuron.at(i)));
        fprintf(fp, "%ld %d %f %f %f %f %ld\n",
                p_pt->n, p_pt->type, p_pt->x, p_pt->y, p_pt->z, p_pt->r, p_pt->pn);
    }

    fclose(fp);
    return true;
}
vector<MyMarker*> readSWC_file1(string swc_file)
{
    vector<MyMarker*> swc;
    ifstream ifs(swc_file.c_str());
    if(ifs.fail())
    {
        cout<<"open swc file : "<< swc_file <<" error"<<endl;
        return swc;
    }
    map<int, MyMarker*> marker_map;
    map<MyMarker*, int> parid_map;
    while(ifs.good())
    {
        if(ifs.peek() == '#'){ifs.ignore(1000,'\n'); continue;}
        MyMarker *  marker = new MyMarker;
        int my_id = -1 ; ifs >> my_id;
        if(my_id == -1) break;
        if(marker_map.find(my_id) != marker_map.end())
        {
            cerr<<"Duplicate Node. This is a graph file. Please read is as a graph."<<endl; return vector<MyMarker*>();
        }
        marker_map[my_id] = marker;

        ifs>> marker->type;
        ifs>> marker->x;
        ifs>> marker->y;
        ifs>> marker->z;
        ifs>> marker->radius;
        int par_id = -1; ifs >> par_id;

        parid_map[marker] = par_id;
        swc.push_back(marker);
    }
    ifs.close();
    vector<MyMarker*>::iterator it = swc.begin();
    while(it != swc.end())
    {
        MyMarker * marker = *it;
        marker->parent = marker_map[parid_map[marker]];
        it++;
    }
    return swc;
}
bool vectorMyMarkerX2vectorMyMarker(vector<MyMarkerX*> &inputMarkers, vector<MyMarker*> & outMarker1s)
{
    string swc_file="temp_for_eswc2swc.swc";
    cout<<"marker num = "<<inputMarkers.size()<<", save swc file to "<<swc_file<<endl;
    map<MyMarkerX*, int> ind;
    ofstream ofs(swc_file.c_str());

    if(ofs.fail())
    {
        cout<<"open swc file error"<<endl;
        return false;
    }

    ofs<<"##n,type,x,y,z,radius,parent,seg_id,seg_level,feature"<<endl;
    for(int i = 0; i < inputMarkers.size(); i++) ind[inputMarkers[i]] = i+1;

    for(int i = 0; i < inputMarkers.size(); i++)
    {
        MyMarkerX * marker = inputMarkers[i];
        int parent_id;
        if(marker->parent == 0) parent_id = -1;
        else parent_id = ind[(MyMarkerX *) marker->parent];
        ofs<<i+1<<" "<<marker->type<<" "<<marker->x<<" "<<marker->y<<" "<<marker->z<<" "<<marker->radius<<" "<<parent_id<<endl;
    }
    ofs.close();

//    outMarker1s = readSWC_file1(swc_file);

    readSWC_file(swc_file,outMarker1s);
    remove((char*)swc_file.c_str());
    return true;
}

vector<MyMarker*>  NeuronTree2vectorofMyMarker(NeuronTree nt)
{
    vector<MyMarker*> swc;
    QString tempname="tempname_for_transfer_1.swc";
    writeSWC_file1(tempname,nt);
//    swc=readSWC_file1(tempname.toStdString());
    readSWC_file(tempname.toStdString(),swc);
    remove(tempname.toUtf8().data());

    return swc;

}


QList <ImageMarker> LandmarkListQList_ImageMarker(LandmarkList marker_l)
{
    QList <ImageMarker> markers;
    for(V3DLONG i=0;i<marker_l.size();i++)
    {
        ImageMarker temp;
        temp.x=marker_l.at(i).x;
        temp.y=marker_l.at(i).y;
        temp.z=marker_l.at(i).z;
        temp.z=marker_l.at(i).z;
        temp.comment = QString::fromStdString(marker_l.at(i).comments);
        temp.radius=marker_l.at(i).radius;
        temp.shape=marker_l.at(i).shape;
        temp.name=QString::fromStdString(marker_l.at(i).name);
        temp.color.r=marker_l.at(i).color.r;
        temp.color.g=marker_l.at(i).color.g;
        temp.color.b=marker_l.at(i).color.b;
        markers.push_back(temp);
    }
    return markers;
}

//程序将以每个marker为中心点，截取一个半径分别为x_radius，y_radius，z_radius的子图。
//步骤：打开一张图，点上marker，输入参数x_length，y_length，z_length，选择保存子图的目录。
int get_sub_auto(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No image is open.");
        return -1;
    }

    Image4DSimple *p4DImage = callback.getImage(curwin);

    //************the default radius of the sub block
    V3DLONG x_radius = 16;
    V3DLONG y_radius = 16;
    V3DLONG z_radius = 16;

    //************set update the dialog
    QDialog * dialog = new QDialog();

    if(p4DImage->getZDim() > 1)
        dialog->setWindowTitle("3D neuron image");
    else
        dialog->setWindowTitle("2D neuron image");

    //*************set spinbox
    QSpinBox *x_radius_spinbox = new QSpinBox();
    x_radius_spinbox->setRange(0,p4DImage->getXDim()*3);
    x_radius_spinbox->setValue(x_radius);
    QSpinBox *y_radius_spinbox = new QSpinBox();
    y_radius_spinbox->setRange(0,p4DImage->getYDim()*3);
    y_radius_spinbox->setValue(y_radius);
    QSpinBox *z_radius_spinbox = new QSpinBox();
    z_radius_spinbox->setRange(0,p4DImage->getZDim()*15);
    if(p4DImage->getZDim()==1)
    {    z_radius_spinbox->setValue(0);}
    else
    {    z_radius_spinbox->setValue(z_radius);}

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(new QLabel("x radius of the subblock"),0,0);
    layout->addWidget(x_radius_spinbox, 0,1,1,5);
    layout->addWidget(new QLabel("y radius of the subblock"),1,0);
    layout->addWidget(y_radius_spinbox, 1,1,1,5);
    layout->addWidget(new QLabel("z radius of the subblock"),2,0);
    layout->addWidget(z_radius_spinbox, 2,1,1,5);


    QHBoxLayout *hbox2 = new QHBoxLayout();
    QPushButton *ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton *cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,6,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    //**************run the dialog
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

    //***************get the dialog return values
    x_radius = x_radius_spinbox->value();
    y_radius = y_radius_spinbox->value();
    z_radius = z_radius_spinbox->value();

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }
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

    unsigned char *sub = 0;

    //***************get the location of a marker
    LandmarkList marker = callback.getLandmark(curwin);
    if(marker.isEmpty())
    {
        v3d_msg("No marker is found.Please select in the image.");
        return -1;
    }


    QString outimg_dir = "";
    QString file_name = QString(p4DImage->getFileName());
    QFileInfo info(file_name);
    QString default_name = info.baseName()+"_sub";
    outimg_dir = QFileDialog::getExistingDirectory(0,
                                  "Save as a Tiff file " );
    V3DLONG im_cropped_sz[4];
    for(V3DLONG m=0;m<marker.size();m++)
    {
        LocationSimple t;
        t.x = marker[m].x;
        t.y = marker[m].y;
        t.z = marker[m].z;

        QString tmpstr = "";    
        tmpstr.append("_x").append(QString("%1").arg(marker[m].x));
        tmpstr.append("_y").append(QString("%1").arg(marker[m].y));
        tmpstr.append("_z").append(QString("%1").arg(marker[m].z));
        QString outimg_file = outimg_dir + "\\" + default_name + tmpstr +".tif";

        if(nz!=1)
        {
            V3DLONG xb = t.x-x_radius;
            V3DLONG xe = t.x+x_radius;
            V3DLONG yb = t.y-y_radius;
            V3DLONG ye = t.y+y_radius;
            V3DLONG zb = t.z-z_radius;
            V3DLONG ze = t.z+z_radius;


            im_cropped_sz[0] = xe-xb+1;
            im_cropped_sz[1] = ye-yb+1;
            im_cropped_sz[2] = ze-zb+1;
            im_cropped_sz[3] = 1;

            try {sub = new unsigned char [im_cropped_sz[0]*im_cropped_sz[1]*im_cropped_sz[2]*im_cropped_sz[3]];}
            catch(...)  {v3d_msg("cannot allocate memory for sub."); return false;}

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
            t.x = marker[m].x;
            t.y = marker[m].y;
            t.z = 1;

            V3DLONG xb = t.x-x_radius;
            V3DLONG xe = t.x+x_radius;
            V3DLONG yb = t.y-y_radius;
            V3DLONG ye = t.y+y_radius;
            V3DLONG zb = 1;
            V3DLONG ze = 1;


            im_cropped_sz[0] = xe-xb+1;
            im_cropped_sz[1] = ye-yb+1;
            im_cropped_sz[2] = ze-zb+1;
            im_cropped_sz[3] = 1;

            try {sub = new unsigned char [im_cropped_sz[0]*im_cropped_sz[1]*im_cropped_sz[2]*im_cropped_sz[3]];}
            catch(...)  {v3d_msg("cannot allocate memory for sub."); return false;}

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
        }



        //v3d_msg("Get a subblock!!");
        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)sub,im_cropped_sz,1);
        //v3d_msg(QString("The center of subblock is (%1 %2 %3)").arg(t.x).arg(t.y).arg(t.z));
    }


//    Image4DSimple *new4DImage = new Image4DSimple();
//    new4DImage->setData((unsigned char*)sub, im_cropped_sz[0], im_cropped_sz[1],im_cropped_sz[2], sz[3], p4DImage->getDatatype());
//    v3dhandle newwin = callback.newImageWindow();
//    callback.setImage(newwin, new4DImage);
//    callback.updateImageWindow(newwin);
    if(sub){ delete []sub; sub=0;}

    return 1;

}

//程序将以每个marker为左上角点，截取一个x_length*y_length*z_length的子图。
//步骤：打开一张图，点上marker，输入参数x_length，y_length，z_length，选择保存子图的目录。
int get_sub_by_marker_as_LeftUpCorner(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
            v3d_msg("No image is open.");
            return -1;
    }

    Image4DSimple *p4DImage = callback.getImage(curwin);

    //************the default radius of the sub block
    V3DLONG x_length = 640;
    V3DLONG y_length = 640;
    V3DLONG z_length = 640;

    //************set update the dialog
    QDialog * dialog = new QDialog();

    if(p4DImage->getZDim() > 1)
            dialog->setWindowTitle("3D neuron image");
    else
            dialog->setWindowTitle("2D neuron image");

    //*************set spinbox
    QSpinBox *x_length_spinbox = new QSpinBox();
    x_length_spinbox->setRange(0,p4DImage->getXDim()*3);
    x_length_spinbox->setValue(x_length);
    QSpinBox *y_length_spinbox = new QSpinBox();
    y_length_spinbox->setRange(0,p4DImage->getYDim()*3);
    y_length_spinbox->setValue(y_length);
    QSpinBox *z_length_spinbox = new QSpinBox();
    z_length_spinbox->setRange(0,p4DImage->getZDim()*3);
    z_length_spinbox->setValue(z_length);


    QGridLayout *layout = new QGridLayout();
    layout->addWidget(new QLabel("x length of the subblock"),0,0);
    layout->addWidget(x_length_spinbox, 0,1,1,5);
    layout->addWidget(new QLabel("y length of the subblock"),1,0);
    layout->addWidget(y_length_spinbox, 1,1,1,5);
    layout->addWidget(new QLabel("z length of the subblock"),2,0);
    layout->addWidget(z_length_spinbox, 2,1,1,5);


    QHBoxLayout *hbox2 = new QHBoxLayout();
    QPushButton *ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton *cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,6,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    //**************run the dialog
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

    //***************get the dialog return values
    x_length = x_length_spinbox->value();
    y_length = y_length_spinbox->value();
    z_length = z_length_spinbox->value();

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }
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

    unsigned char *sub = new unsigned char [sz[0]*sz[1]*sz[2]*sz[3]];

    //***************get the location of a marker
    LandmarkList marker = callback.getLandmark(curwin);
    if(marker.isEmpty())
    {
        v3d_msg("No marker is found.Please select in the image.");
        return -1;
    }


    QString outimg_dir = "";
    QString file_name = QString(p4DImage->getFileName());
    QFileInfo info(file_name);
    QString default_name = info.baseName()+"_sub";
    outimg_dir = QFileDialog::getExistingDirectory(0,
                                  "Choose a dir to save file " );
    V3DLONG im_cropped_sz[4];
    for(V3DLONG m=0;m<marker.size();m++)
    {
        QString tmpstr = "";
        tmpstr.setNum(m+1);
        QString outimg_file = outimg_dir + "\\" + default_name + tmpstr +".tif";
        LocationSimple t;
        if(nz!=1)
        {
            t.x = marker[m].x;
            t.y = marker[m].y;
            t.z = marker[m].z;

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
            t.x = marker[m].x;
            t.y = marker[m].y;
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
        }



        //v3d_msg("Get a subblock!!");
        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)sub,im_cropped_sz,1);
        //v3d_msg(QString("The center of subblock is (%1 %2 %3)").arg(t.x).arg(t.y).arg(t.z));
    }


    return 1;

}

//程序将以输入坐标为左上角点，截取一个x_length*y_length*z_length的子图。
//步骤：打开一张图，输入一个坐标，输入参数x_length，y_length，z_length，选择保存子图的目录。
int get_sub_by_input_LeftUpCorner(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No image is open.");
        return -1;
    }

    Image4DSimple *p4DImage = callback.getImage(curwin);

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


    //************the default radius of the sub block
    V3DLONG x_coordinate = 1;
    V3DLONG y_coordinate = 1;
    V3DLONG z_coordinate = 1;
    V3DLONG x_length = 640;
    V3DLONG y_length = 640;
    V3DLONG z_length = 640;

    //************set update the dialog
    QDialog * dialog = new QDialog();

    if(p4DImage->getZDim() > 1)
            dialog->setWindowTitle("3D neuron image");
    else
            dialog->setWindowTitle("2D neuron image");

    //*************set spinbox
    QSpinBox * x_location_spinbox = new QSpinBox();
    x_location_spinbox->setRange(1,sz[0]);
    x_location_spinbox->setValue(x_coordinate);

    QSpinBox * y_location_spinbox = new QSpinBox();
    y_location_spinbox->setRange(1,sz[1]);
    y_location_spinbox->setValue(y_coordinate);

    QSpinBox * z_location_spinbox = new QSpinBox();
    z_location_spinbox->setRange(1,sz[2]);
    z_location_spinbox->setValue(z_coordinate);

    QSpinBox *x_length_spinbox = new QSpinBox();
    x_length_spinbox->setRange(0,p4DImage->getXDim()*3);
    x_length_spinbox->setValue(x_length);
    QSpinBox *y_length_spinbox = new QSpinBox();
    y_length_spinbox->setRange(0,p4DImage->getYDim()*3);
    y_length_spinbox->setValue(y_length);
    QSpinBox *z_length_spinbox = new QSpinBox();
    z_length_spinbox->setRange(0,p4DImage->getZDim()*3);
    z_length_spinbox->setValue(1);


    QGridLayout *layout = new QGridLayout();
    layout->addWidget(new QLabel("x location"),0,0);
    layout->addWidget(x_location_spinbox, 0,1,1,5);

    layout->addWidget(new QLabel("y location"),1,0);
    layout->addWidget(y_location_spinbox, 1,1,1,5);

    layout->addWidget(new QLabel("z location"),2,0);
    layout->addWidget(z_location_spinbox, 2,1,1,5);

    layout->addWidget(new QLabel("x length of the subblock"),3,0);
    layout->addWidget(x_length_spinbox, 3,1,1,5);
    layout->addWidget(new QLabel("y length of the subblock"),4,0);
    layout->addWidget(y_length_spinbox, 4,1,1,5);
    layout->addWidget(new QLabel("z length of the subblock"),5,0);
    layout->addWidget(z_length_spinbox, 5,1,1,5);


    QHBoxLayout *hbox2 = new QHBoxLayout();
    QPushButton *ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton *cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,10,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    //**************run the dialog
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

    //***************get the dialog return values
    x_coordinate=x_location_spinbox->value();
    y_coordinate=y_location_spinbox->value();
    z_coordinate=z_location_spinbox->value();
    x_length = x_length_spinbox->value();
    y_length = y_length_spinbox->value();
    z_length = z_length_spinbox->value();

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }

    unsigned char *matching_img=0;
    try{matching_img=new unsigned char [nx*ny*nz];}
    catch(...) {v3d_msg("cannot allocate memory for matching_img."); return 0;}

    unsigned char *sub = new unsigned char [nx*ny*nz];
    unsigned char *matching_sub = new unsigned char [nx*ny*nz];

    //***************get the location of a marker
    LocationSimple marker;
    marker.x = x_coordinate;
    marker.y = y_coordinate;
    marker.z = z_coordinate;

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
            matching_img = callback.loadImage(matching_img_name.toLocal8Bit().data())->getRawData();
        }
        else if(fileNameList1.at(i)=="mip")
        {
            matching_img_name = info.canonicalPath()+"\\"+fileNameList1.at(0)+"_cross2Dgt_label.tif";
            matching_img = callback.loadImage(matching_img_name.toLocal8Bit().data())->getRawData();
        }
    }


    outimg_dir = QFileDialog::getExistingDirectory(0,
                                  "Choose a dir to save file " );
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
        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)sub,im_cropped_sz,1);
        //v3d_msg(QString("The center of subblock is (%1 %2 %3)").arg(t.x).arg(t.y).arg(t.z));
    }

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
        simple_saveimage_wrapper(callback, matching_outimg_file.toStdString().c_str(),(unsigned char *)matching_sub,im_cropped_sz,1);
        //v3d_msg(QString("The center of subblock is (%1 %2 %3)").arg(t.x).arg(t.y).arg(t.z));
    }


    return 1;

}

int get_sub_by_show_boundary(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No image is open.");
        return -1;
    }

    loc_input *s=new loc_input();

    s->setV3DPluginCallback2(&callback);

    s->show();

    return 1;

}

int get_sub(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if(!curwin)
    {
            v3d_msg("No image is open.");
            return -1;
    }

    Image4DSimple *p4DImage = callback.getImage(curwin);

    //************the default radius of the sub block
    V3DLONG x_radius = 64;
    V3DLONG y_radius = 64;
    V3DLONG z_radius = 16;

    //************set update the dialog
    QDialog * dialog = new QDialog();

    if(p4DImage->getZDim() > 1)
        dialog->setWindowTitle("3D neuron image");
    else
        dialog->setWindowTitle("2D neuron image");

    //*************set spinbox
    QSpinBox *x_radius_spinbox = new QSpinBox();
    x_radius_spinbox->setRange(0,p4DImage->getXDim()*3);
    x_radius_spinbox->setValue(x_radius);
    QSpinBox *y_radius_spinbox = new QSpinBox();
    y_radius_spinbox->setRange(0,p4DImage->getYDim()*3);
    y_radius_spinbox->setValue(y_radius);
    QSpinBox *z_radius_spinbox = new QSpinBox();
    z_radius_spinbox->setRange(0,p4DImage->getZDim()*3);
    if(p4DImage->getZDim()==1)
    {    z_radius_spinbox->setValue(0);}
    else
    {    z_radius_spinbox->setValue(z_radius);}

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(new QLabel("x radius of the subblock"),0,0);
    layout->addWidget(x_radius_spinbox, 0,1,1,5);
    layout->addWidget(new QLabel("y radius of the subblock"),1,0);
    layout->addWidget(y_radius_spinbox, 1,1,1,5);
    layout->addWidget(new QLabel("z radius of the subblock"),2,0);
    layout->addWidget(z_radius_spinbox, 2,1,1,5);

    QHBoxLayout *hbox2 = new QHBoxLayout();
    QPushButton *ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton *cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,6,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    //**************run the dialog
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

    //***************get the dialog return values
    x_radius = x_radius_spinbox->value();
    y_radius = y_radius_spinbox->value();
    z_radius = z_radius_spinbox->value();

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }
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


    //***************get the location of a marker
    LandmarkList marker = callback.getLandmark(curwin);
    if(marker.isEmpty())
    {
        v3d_msg("No marker is found.Please select in the image.");
        return -1;
    }


    V3DLONG im_cropped_sz[4];
    for(V3DLONG m=0;m<marker.size();m++)
    {
        LocationSimple t;
        t.x = marker[m].x;
        t.y = marker[m].y;
        t.z = marker[m].z;

        QString tmpstr = "";
        tmpstr.append("_x").append(QString("%1").arg(marker[m].x));
        tmpstr.append("_y").append(QString("%1").arg(marker[m].y));
        tmpstr.append("_z").append(QString("%1").arg(marker[m].z));
        QString outimg_file = "";
        QString file_name = QString(p4DImage->getFileName());
        QFileInfo info(file_name);
        QString default_name = info.baseName()+"_sub"+tmpstr+".tif";
        outimg_file = QFileDialog::getSaveFileName(0,
                                      "Save as a Tiff file ",
                                      default_name,
                                      QObject::tr("Tiff  (*.tif);;(*.*)"
                                      ));

        unsigned char *sub=0;
        if(nz!=1)
        {
            V3DLONG xb = t.x-x_radius;
            V3DLONG xe = t.x+x_radius;
            V3DLONG yb = t.y-y_radius;
            V3DLONG ye = t.y+y_radius;
            V3DLONG zb = t.z-z_radius;
            V3DLONG ze = t.z+z_radius;


            im_cropped_sz[0] = xe-xb+1;
            im_cropped_sz[1] = ye-yb+1;
            im_cropped_sz[2] = ze-zb+1;
            im_cropped_sz[3] = 1;

            try {sub = new unsigned char [im_cropped_sz[0]*im_cropped_sz[1]*im_cropped_sz[2]*im_cropped_sz[3]];}
            catch(...)  {v3d_msg("cannot allocate memory for sub."); return false;}

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
            t.x = marker[m].x;
            t.y = marker[m].y;
            t.z = 1;

            V3DLONG xb = t.x-x_radius;
            V3DLONG xe = t.x+x_radius;
            V3DLONG yb = t.y-y_radius;
            V3DLONG ye = t.y+y_radius;
            V3DLONG zb = 1;
            V3DLONG ze = 1;


            im_cropped_sz[0] = xe-xb+1;
            im_cropped_sz[1] = ye-yb+1;
            im_cropped_sz[2] = ze-zb+1;
            im_cropped_sz[3] = 1;

            try {sub = new unsigned char [im_cropped_sz[0]*im_cropped_sz[1]*im_cropped_sz[2]*im_cropped_sz[3]];}
            catch(...)  {v3d_msg("cannot allocate memory for sub."); return false;}

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
        }

        //v3d_msg("Get a subblock!!");
        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)sub,im_cropped_sz,1);
        //v3d_msg(QString("The center of subblock is (%1 %2 %3)").arg(t.x).arg(t.y).arg(t.z));
        if(sub){ delete []sub; sub=0;}
    }


//    Image4DSimple *new4DImage = new Image4DSimple();
//    new4DImage->setData((unsigned char*)sub, im_cropped_sz[0], im_cropped_sz[1],im_cropped_sz[2], sz[3], p4DImage->getDatatype());
//    v3dhandle newwin = callback.newImageWindow();
//    callback.setImage(newwin, new4DImage);
//    callback.updateImageWindow(newwin);
    return 1;

}

//程序可以在terafly模式下挖出子图
//步骤：打开一张terafly图。在terafly窗口点上marker。选择保存子图的目录。将以marker为中心挖出一个以x_radius，y_radius，z_radius为半径的子图。
int get_sub_terafly(V3DPluginCallback2 &callback,QWidget *parent)
{
    QString inimg_file = callback.getPathTeraFly();
    LandmarkList terafly_landmarks = callback.getLandmarkTeraFly();

    int x_radius = 512;
    int y_radius = 512;
    int z_radius = 50;
    //************set update the dialog
    QDialog * dialog = new QDialog();

    //*************set spinbox
    QSpinBox *x_radius_spinbox = new QSpinBox();
    x_radius_spinbox->setRange(0,10000);
    x_radius_spinbox->setValue(x_radius);
    QSpinBox *y_radius_spinbox = new QSpinBox();
    y_radius_spinbox->setRange(0,10000);
    y_radius_spinbox->setValue(y_radius);
    QSpinBox *z_radius_spinbox = new QSpinBox();
    z_radius_spinbox->setRange(0,10000);
    z_radius_spinbox->setValue(z_radius);

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(new QLabel("x radius of the subblock"),0,0);
    layout->addWidget(x_radius_spinbox, 0,1,1,5);
    layout->addWidget(new QLabel("y radius of the subblock"),1,0);
    layout->addWidget(y_radius_spinbox, 1,1,1,5);
    layout->addWidget(new QLabel("z radius of the subblock"),2,0);
    layout->addWidget(z_radius_spinbox, 2,1,1,5);


    QHBoxLayout *hbox2 = new QHBoxLayout();
    QPushButton *ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton *cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,6,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    //**************run the dialog
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

    //***************get the dialog return values
    x_radius = x_radius_spinbox->value();
    y_radius = y_radius_spinbox->value();
    z_radius = z_radius_spinbox->value();

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }
    for(V3DLONG m=0;m<terafly_landmarks.size();m++)
    {
        LocationSimple t;
        t.x = terafly_landmarks[m].x;
        t.y = terafly_landmarks[m].y;
        t.z = terafly_landmarks[m].z;

        V3DLONG im_cropped_sz[4];

        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        double l_x = x_radius;
        double l_y = y_radius;
        double l_z = z_radius;

        V3DLONG xb = t.x-l_x;
        V3DLONG xe = t.x+l_x-1;
        V3DLONG yb = t.y-l_y;
        V3DLONG ye = t.y+l_y-1;
        V3DLONG zb = t.z-l_z;
        V3DLONG ze = t.z+l_z-1;
        pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
        im_cropped_sz[0] = xe-xb+1;
        im_cropped_sz[1] = ye-yb+1;
        im_cropped_sz[2] = ze-zb+1;
        im_cropped_sz[3] = 1;

        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}


        im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,
                                                  yb,ye+1,zb,ze+1);

        QString tmpstr = "";
        tmpstr.append("_x").append(QString("%1").arg(terafly_landmarks[m].x));
        tmpstr.append("_y").append(QString("%1").arg(terafly_landmarks[m].y));
        tmpstr.append("_z").append(QString("%1").arg(terafly_landmarks[m].z));
        QString outimg_file = "";
        QString file_name = inimg_file;
        QFileInfo info(file_name);
        QString default_name = info.baseName()+"_TeraSub"+tmpstr+".tif";
        outimg_file = QFileDialog::getSaveFileName(0,
                                      "Save as a Tiff file ",
                                      default_name,
                                      QObject::tr("Tiff  (*.tif);;(*.*)"
                                      ));

        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }
    return 1;

}

//程序可以在terafly模式下同时挖出子图和子swc
//步骤：打开一张terafly图。如果terafly窗口里没有marker，将提示选择一个marker文件。选择保存子图和子swc的目录。如果terafly窗口里没有swc，将提示选择一个或多个swc或eswc文件。将以marker为中心挖出一个以x_radius，y_radius，z_radius为半径的子图以及swc。
int get_sub_terafly_and_swc(V3DPluginCallback2 &callback,QWidget *parent)
{
    QString inimg_file = callback.getPathTeraFly();
    QString file_name = inimg_file;
    QFileInfo info(file_name);

    LandmarkList terafly_landmarks = callback.getLandmarkTeraFly();

    if(terafly_landmarks.size()==0)
    {
        QString marker_file = "";
        marker_file= QFileDialog::getOpenFileName(0,
                              "No marker in windows. Choose a marker file, please",
                              "",
                              QObject::tr("marker  (*.marker);;(*.*)"
                                ));
        if(marker_file.isEmpty()){  v3d_msg("No marker is found as the block center."); return false;}
        QList <ImageMarker> tmp_built_in_points = readMarker_file(marker_file);
        terafly_landmarks = QList_ImageMarker2LandmarkList(tmp_built_in_points);
    }

    int x_radius = 50;
    int y_radius = 50;
    int z_radius = 50;
    //************set update the dialog
    QDialog * dialog = new QDialog();

    //*************set spinbox
    QSpinBox *x_radius_spinbox = new QSpinBox();
    x_radius_spinbox->setRange(0,10000);
    x_radius_spinbox->setValue(x_radius);
    QSpinBox *y_radius_spinbox = new QSpinBox();
    y_radius_spinbox->setRange(0,10000);
    y_radius_spinbox->setValue(y_radius);
    QSpinBox *z_radius_spinbox = new QSpinBox();
    z_radius_spinbox->setRange(0,10000);
    z_radius_spinbox->setValue(z_radius);

    QGridLayout *layout = new QGridLayout();
    layout->addWidget(new QLabel("x radius of the subblock"),0,0);
    layout->addWidget(x_radius_spinbox, 0,1,1,5);
    layout->addWidget(new QLabel("y radius of the subblock"),1,0);
    layout->addWidget(y_radius_spinbox, 1,1,1,5);
    layout->addWidget(new QLabel("z radius of the subblock"),2,0);
    layout->addWidget(z_radius_spinbox, 2,1,1,5);


    QHBoxLayout *hbox2 = new QHBoxLayout();
    QPushButton *ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton *cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,6,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    //**************run the dialog
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

    //***************get the dialog return values
    x_radius = x_radius_spinbox->value();
    y_radius = y_radius_spinbox->value();
    z_radius = z_radius_spinbox->value();

    if (dialog)
    {
        delete dialog;
        dialog=0;
        cout<<"delete dialog"<<endl;
    }

    QString dir = "";
    dir = QFileDialog::getExistingDirectory(0,
                                  "choose the save dir",
                                  QObject::tr("(*.*)"
                                  ));

    QString all_swc_file = dir + "\\" + info.baseName() +"_all_swc.swc";

    vector<MyMarker*> outMarker1; outMarker1.clear();
    vector<MyMarker*> outMarker2; outMarker2.clear();


    QStringList swc_list;

    NeuronTree nt = callback.getSWCTeraFly();
    int swcInWindows=0;
    if(nt.listNeuron.size()!=0)
    {   swcInWindows=1;}
    else
    {
        swcInWindows=0;
        swc_list=QFileDialog::getOpenFileNames(0,"Choose SWC or ESWC","D:\\",QObject::tr("*.swc;;*.eswc"));
        if(swc_list.isEmpty()){return 0;}
        int input_as_swc;//0 for eswc, 1 for swc
        if(swc_list.at(0).endsWith(".eswc"))
        {   input_as_swc=0;}
        else{   input_as_swc=1;}
        if(input_as_swc)
        {
            for(int i=0;i<swc_list.size();i++)
            {
                if(!swc_list.at(i).endsWith(".swc"))
                {
                    swc_list.removeAt(i);
                }
            }
            for(int i=0;i<swc_list.size();i++)
            {
                cout<<swc_list.at(i).toUtf8().data()<<endl;
            }
            vector<MyMarker*> temp_swc_all1;
            for(int i=0;i<swc_list.size();i++)
            {
                vector<MyMarker*> temp_swc;temp_swc.clear();
                readSWC_file(swc_list.at(i).toStdString(),temp_swc);
                cout<<"i="<<i<<" temp_swc.size:"<<temp_swc.size()<<endl;
                for(int j=0;j<temp_swc.size();j++)
                {
                   temp_swc_all1.push_back(temp_swc.at(j));
                }
            }
            cout<<"temp_swc_all1.size:"<<temp_swc_all1.size()<<endl;
            outMarker1=temp_swc_all1;
            cout<<"outMarker1.size:"<<outMarker1.size()<<endl;
            QString all_swc_file = dir + "\\" + info.baseName() +"_all_swc.swc";
            saveSWC_file1(all_swc_file.toStdString(),outMarker1);

            vector<MyMarker*> temp_swc_all2;
            for(int i=0;i<swc_list.size();i++)
            {
                vector<MyMarker*> temp_swc;temp_swc.clear();
                readSWC_file(swc_list.at(i).toStdString(),temp_swc);
                cout<<"i="<<i<<" temp_swc.size:"<<temp_swc.size()<<endl;
                for(int j=0;j<temp_swc.size();j++)
                {
                   temp_swc_all2.push_back(temp_swc.at(j));
                }
            }
            cout<<"temp_swc_all2.size:"<<temp_swc_all2.size()<<endl;
            outMarker2=temp_swc_all2;
        }
        else
        {
            //0 for eswc
            for(int i=0;i<swc_list.size();i++)
            {
                if(!swc_list.at(i).endsWith(".eswc"))
                {
                    swc_list.removeAt(i);
                }
            }
            for(int i=0;i<swc_list.size();i++)
            {
                cout<<swc_list.at(i).toUtf8().data()<<endl;
            }
            vector<MyMarkerX*> temp_swc_all1;
            for(int i=0;i<swc_list.size();i++)
            {
                vector<MyMarkerX*> temp_eswc;temp_eswc.clear();
                readESWC_file(swc_list.at(i).toStdString(),temp_eswc);
                cout<<"i="<<i<<" temp_eswc.size:"<<temp_eswc.size()<<endl;
                for(int j=0;j<temp_eswc.size();j++)
                {
                   temp_swc_all1.push_back(temp_eswc.at(j));
                }

            }
            cout<<"temp_swc_all1.size:"<<temp_swc_all1.size()<<endl;
            vectorMyMarkerX2vectorMyMarker(temp_swc_all1,outMarker1);
            cout<<"outMarker1.size:"<<outMarker1.size()<<endl;
            saveSWC_file1(all_swc_file.toStdString(),outMarker1);

            vector<MyMarkerX*> temp_swc_all2;
            for(int i=0;i<swc_list.size();i++)
            {
                vector<MyMarkerX*> temp_eswc;temp_eswc.clear();
                readESWC_file(swc_list.at(i).toStdString(),temp_eswc);
                cout<<"i="<<i<<" temp_eswc.size:"<<temp_eswc.size()<<endl;
                for(int j=0;j<temp_eswc.size();j++)
                {
                   temp_swc_all2.push_back(temp_eswc.at(j));
                }
            }
            cout<<"temp_swc_all2.size:"<<temp_swc_all2.size()<<endl;
            vectorMyMarkerX2vectorMyMarker(temp_swc_all2,outMarker2);
        }
    }



    double time_counter=1, process1 = 0, entire_time=terafly_landmarks.size();
    for(V3DLONG m=0;m<terafly_landmarks.size();m++)
    {
        double process2 = (time_counter++)*1000.0/entire_time;
        if(process2-process1 >=1)
        {cout<<"\r"<<(int)process2/10.0<<"%   "<<endl; cout.flush(); process1 = process2;}

        LocationSimple t;
        t.x = terafly_landmarks[m].x;
        t.y = terafly_landmarks[m].y;
        t.z = terafly_landmarks[m].z;

        V3DLONG im_cropped_sz[4];

        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        double l_x = x_radius;
        double l_y = y_radius;
        double l_z = z_radius;

        V3DLONG xb = t.x-l_x;
        V3DLONG xe = t.x+l_x-1;
        V3DLONG yb = t.y-l_y;
        V3DLONG ye = t.y+l_y-1;
        V3DLONG zb = t.z-l_z;
        V3DLONG ze = t.z+l_z-1;
        pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
        im_cropped_sz[0] = xe-xb+1;
        im_cropped_sz[1] = ye-yb+1;
        im_cropped_sz[2] = ze-zb+1;
        im_cropped_sz[3] = 1;

        try {im_cropped = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}
//        cout<<"xb:"<<xb<<" xe:"<<xe<<" yb:"<<yb<<" ye:"<<ye<<" zb:"<<zb<<" ze:"<<ze<<endl;

        im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,yb,ye+1,zb,ze+1);

        QString tmpstr = "";
        tmpstr.append("_x").append(QString("%1").arg(terafly_landmarks[m].x));
        tmpstr.append("_y").append(QString("%1").arg(terafly_landmarks[m].y));
        tmpstr.append("_z").append(QString("%1").arg(terafly_landmarks[m].z));

        QString default_name = info.baseName()+"_TeraSub"+tmpstr+".tif";
        QString save_path_img = dir+"/"+default_name;

        simple_saveimage_wrapper(callback, save_path_img.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}


        vector<MyMarker*> swc_sub, swc_notsub; swc_sub.clear(); swc_notsub.clear();
        vector<MyMarker*> swc_tmp; swc_tmp.clear();
        vector<MyMarker*> swc; swc.clear();
        if(swcInWindows)
        {
            swc = NeuronTree2vectorofMyMarker(nt);
            swc_tmp = NeuronTree2vectorofMyMarker(nt);
        }
        else
        {
            swc = outMarker1;
            swc_tmp = outMarker2;
        }
        for (V3DLONG i=0; i<swc.size(); i++)
        {
            MyMarker * curr_m = swc.at(i);
            MyMarker * curr_m_tmp = swc_tmp.at(i);
            if(curr_m->x>=xb&&curr_m->x<=xe&&curr_m->y>=yb&&curr_m->y<=ye&&curr_m->z>=zb&&curr_m->z<=ze)
            {
                curr_m_tmp->x=swc_tmp.at(i)->x;
                curr_m_tmp->y=swc_tmp.at(i)->y;
                curr_m_tmp->z=swc_tmp.at(i)->z;
                swc_sub.push_back(curr_m_tmp);
            }
        }
        QString save_path_swc = dir+"/"+ info.baseName()+"_TeraSub"+tmpstr+".swc";

        saveSWC_file1(save_path_swc.toStdString(),swc_sub);
        readSWC_file(save_path_swc.toStdString(),swc_sub);
        for (V3DLONG i=0; i<swc_sub.size(); i++)
        {
            swc_sub[i]->x -= xb;
            swc_sub[i]->y -= yb;
            swc_sub[i]->z -= zb;
        }
        saveSWC_file1(save_path_swc.toStdString(),swc_sub);

    }
    return 1;

}

