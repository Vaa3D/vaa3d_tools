/* image_blend_plugin.cpp
 * 
 * 2014-01-28 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "image_blend_plugin.h"
#include <fstream>
#include <iostream>

using namespace std;
Q_EXPORT_PLUGIN2(image_blend, image_blend);
 
QStringList image_blend::menulist() const
{
	return QStringList() 
		<<tr("about");
}

QStringList image_blend::funclist() const
{
	return QStringList()
		<<tr("image_blend")
		<<tr("help");
}

void image_blend::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("about"))
	{
        v3d_msg(tr("This plugin blends two images. This plugin is designed for command line use ONLY right now. "
			"Developed by Zhi Zhou, 2014-01-28"));
	}
}

bool image_blend::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("image_blend"))
	{
        if (output.size() != 1) return false;

        char * inimg_file1 = ((vector<char*> *)(input.at(0).p))->at(0);
        char * inimg_file2 = ((vector<char*> *)(input.at(1).p))->at(0);

        char * output_file = ((vector<char*> *)(output.at(0).p))->at(0);


        unsigned char * image1 = 0;
        V3DLONG in_sz1[4];
        int datatype1;
        if(!simple_loadimage_wrapper(callback, inimg_file1, image1, in_sz1, datatype1))
        {
            cerr<<"load image1 "<<inimg_file1<<" error!"<<endl;
            if (image1) {delete image1; image1=0;}
            return false;
        }
        V3DLONG pagesz1 = in_sz1[0]*in_sz1[1]*in_sz1[2];

        unsigned char * image2 = 0;
        V3DLONG in_sz2[4];
        int datatype2;
        if(!simple_loadimage_wrapper(callback, inimg_file2, image2, in_sz2, datatype2))
        {
            cerr<<"load image2 "<<inimg_file2<<" error!"<<endl;
             if (image2) {delete image2; image2=0;}
            return false;
        }

        V3DLONG pagesz2 = in_sz2[0]*in_sz2[1]*in_sz2[2];

        if(pagesz1 != pagesz2)
        {
             v3d_msg("Two images have differnt size.");
             if (image1) {delete image1; image1=0;}
             if (image2) {delete image2; image2=0;}
             return false;
        }

        unsigned char *data_blended = 0;
        try {data_blended = new unsigned char [pagesz1*3];}
        catch(...)  {v3d_msg("cannot allocate memory for data_blended."); return false;}
        for(V3DLONG i = 0; i < pagesz1*3; i++)
        {
            if(i < pagesz1)
                data_blended[i] = image1[i];
            else if(i < 2*pagesz1)
                data_blended[i] = image2[i-pagesz1];
            else
                data_blended[i] = 0;
        }
        in_sz1[3] = 3;
        simple_saveimage_wrapper(callback, output_file, (unsigned char *)data_blended, in_sz1, 1);

        if (image1) {delete []image1; image1=0;}
        if (image2) {delete []image2; image2=0;}
        if (data_blended) {delete []data_blended; data_blended=0;}

	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

