/* get_sub_terafly_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-6-12 : by Yongzhang
 */
 
#include "v3d_message.h"
#include <vector>
#include <iostream>
#include "basic_surf_objs.h"

#include "get_sub_terafly_plugin.h"
Q_EXPORT_PLUGIN2(get_sub_terafly, TestPlugin);


bool get_sub_terafly(V3DPluginCallback2 &callback,QWidget *parent,QString outimg_file);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("get_sub_terafly")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("get_sub_terafly")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("get_sub_terafly"))
	{
        QString outimg_file;
        get_sub_terafly(callback,parent,outimg_file);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2018-6-12"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("get_sub_terafly"))
	{
        QString outimg_file;
        get_sub_terafly(callback,parent,outimg_file);
	}
    else if (func_name == tr("help"))
    {
        v3d_msg("To be implemented.");
    }
	else return false;

	return true;
}

bool get_sub_terafly(V3DPluginCallback2 &callback,QWidget *parent,QString outimg_file)
{
    QString inimg_file = callback.getPathTeraFly();
    LandmarkList terafly_landmarks = callback.getLandmarkTeraFly();

    for(V3DLONG i=0;i<terafly_landmarks.size();i++)
    {
        LocationSimple t;
        t.x = terafly_landmarks[i].x;
        t.y = terafly_landmarks[i].y;
        t.z = terafly_landmarks[i].z;

        V3DLONG im_cropped_sz[4];

        unsigned char * im_cropped = 0;
        V3DLONG pagesz;
        double l_x = 256;
        double l_y = 256;
        double l_z = 128;

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

        //cout<<"begin ==================="<<xb<<"  "<<yb<<"  "<<zb<<"  "<<endl;
        //cout<<"end   ==================="<<xe<<"  "<<ye<<"  "<<ze<<"  "<<endl;
        v3d_msg("test!");


        im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,
                                                  yb,ye+1,zb,ze+1);


        QString outimg_file;
        outimg_file = "test.tif";
       // outswc_file = "test.swc";
       // writeSWC_file(outswc_file,outswc);

       // export_list2file(outswc,outswc_file,outswc_file);
        simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
        if(im_cropped) {delete []im_cropped; im_cropped = 0;}
    }

}
