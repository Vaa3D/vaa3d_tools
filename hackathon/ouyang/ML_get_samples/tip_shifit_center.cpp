#include "tip_shift_center.h"



void tip_shift_center(V3DPluginCallback2 & callback, const V3DPluginArgList & input,
                                          V3DPluginArgList & output)
{
    //Input,output check
    V3DLONG sz_img[4];
    unsigned char *image_data;
    int intype;
    LandmarkList LList,LList_new_center;
    mean_shift_fun fun_obj;
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if ((infiles.size()!=2))
    {
        qDebug()<<"ERROR: please provide image and marker file! "<<infiles.size();
        return;
    }
    if (inparas.size() != 0 && inparas.size() != 1)
    {
        qDebug()<<"ERROR: please set the parameter of window radius (2-30) or leave it blank"
                  " and the program will use the default value of 15!  :" <<inparas.size();
        return;
    }

    //check parameter
    int windowradius=15;
    if (inparas.size()==1)
    {
        int tmp=atoi(inparas.at(0));
        if (tmp>1 && tmp<=30)
        {
            windowradius=tmp;
            qDebug()<<"mean shift search window radius' is set to: "<<tmp;
        }

        else
            v3d_msg("The parameter of window radius is not valid, the program will use default value of 15",0);
    }

    //load image and markers
    image_data=0;
    LList.clear();
    load_image_marker(callback,input,image_data,LList,intype,sz_img);

    V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
    if(intype==1)
    {
        fun_obj.pushNewData<unsigned char>((unsigned char*)image_data, sz_img);
    }
    else if (intype == 2) //V3D_UINT16;
    {
        fun_obj.pushNewData<unsigned short>((unsigned short*)image_data, sz_img);
        convert2UINT8_meanshift((unsigned short*)image_data, image_data, size_tmp);
    }
    else if(intype == 4) //V3D_FLOAT32;
    {
        fun_obj.pushNewData<float>((float*)image_data, sz_img);
        convert2UINT8_meanshift((float*)image_data, image_data, size_tmp);
    }
    else
    {
        v3d_msg("Currently this program only supports UINT8, UINT16, and FLOAT32 data type.",0);
        return;
    }

    vector<V3DLONG> poss_landmark;
    poss_landmark=landMarkList2poss(LList, sz_img[0], sz_img[0]*sz_img[1]);
    LList_new_center.clear();
    vector<float> mass_center;

    for (int j=0;j<poss_landmark.size();j++)
    {
        //append the original marker in LList_new_center
        LList[j].name="ori";
        LList[j].color.r=255; LList[j].color.g=LList[j].color.b=0;
        LList_new_center.append(LList.at(j));
//        else
//        {
            mass_center=fun_obj.mean_shift_center(poss_landmark[j],windowradius);
            LocationSimple tmp(mass_center[0]+1,mass_center[1]+1,mass_center[2]+1);
            tmp.color.r=170; tmp.color.g=0; tmp.color.b=255;
            tmp.name="ms";
            LList_new_center.append(tmp);
//        }


    }

    qDebug()<<"LList_new_center_size:"<<LList_new_center.size();
    //Write data in the file
    QString qs_input_image(infiles[0]);
    QString qs_output;
    //if (methodcode==2)
    //    qs_output = outfiles.empty() ? qs_input_image + "_ms_c_out.marker" : QString(outfiles[0]);
    //else
        qs_output = outfiles.empty() ? qs_input_image + "_ms_out.marker" : QString(outfiles[0]);
    FILE * fp_1 = fopen(qs_output.toAscii(), "w");
    if (!fp_1)
    {
        qDebug()<<"cannot open the file to save the landmark points.\n";
        return;
    }
    fprintf(fp_1,"#parameters: \n");
    fprintf(fp_1,"#mean shift search window radius: %d\n",windowradius);
    fprintf(fp_1, "#x, y, z, radius, shape, name, comment,color_r,color_g,color_b\n");

    for (int i=0;i<LList_new_center.size(); i++)
    {
        LocationSimple tmp=LList_new_center.at(i);
        fprintf(fp_1, "%.lf,%.lf,%.lf,%.1f,1,",tmp.x,tmp.y,tmp.z,tmp.radius);
        fprintf(fp_1, "%s, ",tmp.name.c_str());
        fprintf(fp_1, " ,%d,%d,%d\n",tmp.color.r,tmp.color.g,tmp.color.b);
    }

    fclose(fp_1);
    if (image_data!=0) {delete []image_data; image_data=0;}

}

void load_image_marker(V3DPluginCallback2 & callback,const V3DPluginArgList & input,
                        unsigned char * & image_data,LandmarkList &LList,int &intype,V3DLONG sz_img[4])
{
    vector<char*> infiles = *((vector<char*> *)input.at(0).p);;

    QString qs_input_image(infiles[0]);
    QString qs_input_mark(infiles[1]);

    if (!qs_input_image.isEmpty())
    {
        if (!simple_loadimage_wrapper(callback, qs_input_image.toStdString().c_str(), image_data, sz_img,intype))
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
    }
    if (qs_input_mark.isEmpty())  return;
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

    LList = readPosFile_usingMarkerCode(qs_input_mark.toAscii());

    if (LList.count()<=0)
    {
        v3d_msg("Did not find any valid markers\n",0);
        return;
    }
    qDebug()<<"LList size:"<<LList.count();
}
void convert2UINT8_meanshift(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz)
{
    unsigned short* pPre = (unsigned short*)pre1d;
    unsigned short max_v=0, min_v = 255;
    for(V3DLONG i=0; i<imsz; i++)
    {
        if(max_v<pPre[i]) max_v = pPre[i];
        if(min_v>pPre[i]) min_v = pPre[i];
    }
    max_v -= min_v;
    if(max_v>0)
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) 255*(double)(pPre[i] - min_v)/max_v;
        }
    }
    else
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) pPre[i];
        }
    }
}
vector<V3DLONG> landMarkList2poss(LandmarkList LandmarkList_input, V3DLONG _offset_Y, V3DLONG _offest_Z)
{
    vector<V3DLONG> poss_result;
    V3DLONG count_landmark=LandmarkList_input.count();
    for (V3DLONG idx_input=0;idx_input<count_landmark;idx_input++)
    {
        poss_result.push_back(landMark2pos(LandmarkList_input.at(idx_input), _offset_Y, _offest_Z));
    }
    return poss_result;
}
QList <LocationSimple> readPosFile_usingMarkerCode(const char * posFile) //last update 090725
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
