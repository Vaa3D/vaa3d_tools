/* Mean_Shift_Center_plugin.cpp
 * Search for center using mean-shift
 * 2015-3-4 : by Yujie Li
 */
 
#include "v3d_message.h"
#include <vector>
#include "mean_shift_center_plugin.h"
#include "mean_shift_dialog.h"

using namespace std;
Q_EXPORT_PLUGIN2(mean_shift_center,mean_shift_plugin );
static mean_shift_dialog *dialog=0;

QStringList mean_shift_plugin::menulist() const
{
	return QStringList() 
        <<tr("mean_shift_center_finder")
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
    if (menu_name == tr("mean_shift_center_finder"))
	{
        dialog=new mean_shift_dialog(&callback);
        dialog->setWindowTitle("mean_shift_center_finder");
        dialog->show();
	}
	else
	{
        v3d_msg(tr("<p>The <b>Mean shift center finder</b> performs searches around each of the user-input markers and "
                   "returns the location of local maxima of intensity as the new marker in the output window.<p>"
			"Developed by Yujie Li, 2015-3-4"));
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


void mean_shift_plugin::mean_shift_center(V3DPluginCallback2 & callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
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
        fclose(fp); //since I will open the file and close it in the function below, thus close it now
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
    vector<V3DLONG> mass_center;

    for (int j=0;j<poss_landmark.size();j++)
    {
        mass_center=calc_mean_shift_center(poss_landmark[j],windowradius);
        LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
        LList_new_center.append(tmp);
    }

    qDebug()<<"LList_new_center_size:"<<LList_new_center.size();
    //Write data in the file
    QString qs_output = outfiles.empty() ? qs_input_image + "_out.marker" : QString(outfiles[0]);

    FILE * fp_1 = fopen(qs_output.toAscii(), "w");
    if (!fp_1)
    {
        qDebug()<<"Control point saving error,"
                                 "can not open the file to save the landmark points.\n";
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

vector<V3DLONG> mean_shift_plugin::calc_mean_shift_center(V3DLONG ind,int windowradius)
{

    V3DLONG y_offset=sz_img[0];
    V3DLONG z_offset=sz_img[0]*sz_img[1];

    V3DLONG x,y,z,pos;
    vector<V3DLONG> coord;

    coord=pos2xyz(ind, y_offset, z_offset);
    x=coord[0];y=coord[1];z=coord[2];
    float total_x,total_y,total_z,v_color,sum_v;
    float center_dis=1;
    vector<V3DLONG> center(3,0);
    V3DLONG page_size=sz_img[0]*sz_img[1]*sz_img[2];

    //find out the channel with the maximum intensity for the marker
    v_color=image_data[ind];
    int channel=0;
    for (int j=1;j<sz_img[3];j++)
    {
        if (image_data[ind+page_size*j]>v_color)
        {
            v_color=image_data[ind+page_size*j];
            channel=j;
        }
    }

    while (center_dis>=1)
    {
        total_x=total_y=total_z=sum_v=0;

        for(V3DLONG dx=MAX(x-windowradius,0); dx<=MIN(sz_img[0]-1,x+windowradius); dx++){
            for(V3DLONG dy=MAX(y-windowradius,0); dy<=MIN(sz_img[1]-1,y+windowradius); dy++){
                for(V3DLONG dz=MAX(z-windowradius,0); dz<=MIN(sz_img[2]-1,z+windowradius); dz++){
                    pos=xyz2pos(dx,dy,dz,y_offset,z_offset);
                    double tmp=(dx-x)*(dx-x)+(dy-y)*(dy-y)
                         +(dz-z)*(dz-z);
                    double distance=sqrt(tmp);
                    if (distance>windowradius) continue;
                    v_color=image_data[pos+page_size*channel];
//                    v_color=image_data[pos];
//                    for (int j=1;j<sz_img[3];j++)
//                    v_color=MAX(v_color,image_data[pos+sz_img[0]*sz_img[1]*sz_img[2]*j]);

                    total_x=v_color*dx+total_x;
                    total_y=v_color*dy+total_y;
                    total_z=v_color*dz+total_z;
                    sum_v=sum_v+v_color;
                 }
             }
         }
            qDebug()<<"v_color:"<<v_color<<":"<<"total xyz:"<<total_x<<":"<<total_y<<":"<<total_z<<":"<<sum_v;
            center[0]=total_x/sum_v;
            center[1]=total_y/sum_v;
            center[2]=total_z/sum_v;
        float tmp_1=(center[0]-x)*(center[0]-x)+(center[1]-y)*(center[1]-y)
                    +(center[2]-z)*(center[2]-z);
        center_dis=sqrt(tmp_1);
        qDebug()<<"center distance:"<<center_dis<<":"<<center[0]<<":"<<center[1]<<":"<<center[2];
        x=center[0]; y=center[1]; z=center[2];
    }
    return center;
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

QList <ImageMarker> readMarker_file(const QString & filename)
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
