/* image_save_channel_plugin.cpp
 * This plugin will save the channel of interest.
 * 2015-2-18 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "image_save_channel_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(image_save_channel, image_channel_io);

// func convert2UINT8
void convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz)
{
    unsigned short* pPre = (unsigned short*)pre1d;
    double max_v=0, min_v = 1e10;
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
            pPost[i] = (unsigned char) 255.0*(double)(pPre[i] - min_v)/max_v;
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

void convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz)
{
    float* pPre = (float*)pre1d;
    float max_v=0, min_v = 1e10;
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
            pPost[i] = (unsigned char) 255.0*(double)(pPre[i] - min_v)/(double)max_v;
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

void shiftTwoBits2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz)
{
    unsigned short* pPre = (unsigned short*)pre1d;
    for(V3DLONG i=0; i<imsz; i++)
    {
        pPost[i] = (unsigned char) MIN(255,pPre[i]/4);
//        qDebug()<<i<<":"<<pPre[i]<<":"<<(int)pPost[i];
    }
}

QStringList image_channel_io::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("about");
}

QStringList image_channel_io::funclist() const
{
	return QStringList()
		<<tr("save_RGB_channel")
		<<tr("help");
}

void image_channel_io::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This plugin will save the channel of interest.. "
			"Developed by Hanbo Chen, 2015-2-18"));
	}
}

bool image_channel_io::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("save_RGB_channel"))
	{
        qDebug()<<"==== save RGB channel ====";
        if(infiles.size()!=1 || outfiles.size()!=1)
        {
            qDebug("ERROR: please set input and output!");
            return false;
        }

        //load image
        QString fname_input = ((vector<char*> *)(input.at(0).p))->at(0);
        QString fname_output = ((vector<char*> *)(output.at(0).p))->at(0);

        unsigned char * image1Dc_in = 0;
        unsigned char * image1Dc_out = 0;
        V3DLONG sz_img[4];
        int intype;
        if(!simple_loadimage_wrapper(callback, fname_input.toStdString().c_str(), image1Dc_in, sz_img, intype))
        {
          v3d_msg("load image "+fname_input+" error!");
          return false;
        }

        if(sz_img[3]>3){
            sz_img[3]=3;
        }else{
            qDebug()<<"warning: there are only "<<sz_img[3]<<" channels. Will save them all.";
        }

        V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

        if (intype == 2) //V3D_UINT16;
        {
            image1Dc_out = new unsigned char[size_tmp];
            shiftTwoBits2UINT8((unsigned short*)image1Dc_in, image1Dc_out, size_tmp);
            if(!simple_saveimage_wrapper(callback, qPrintable(fname_output),image1Dc_out,sz_img,1)){
                v3d_msg("failed to save file to " + fname_output);
                return false;
            }
            delete[] image1Dc_out; image1Dc_out=0;
        }
        else if(intype == 4) //V3D_FLOAT32;
        {
            image1Dc_out = new unsigned char[size_tmp];
            convert2UINT8((float*)image1Dc_in, image1Dc_out, size_tmp);
            if(!simple_saveimage_wrapper(callback, qPrintable(fname_output),image1Dc_out,sz_img,1)){
                v3d_msg("failed to save file to " + fname_output);
                return false;
            }
            delete[] image1Dc_out; image1Dc_out=0;
        }
        else
        {
            if(!simple_saveimage_wrapper(callback, qPrintable(fname_output),image1Dc_in,sz_img,1)){
                v3d_msg("failed to save file to " + fname_output);
                return false;
            }
        }

        delete[] image1Dc_in; image1Dc_in=0;
	}
	else if (func_name == tr("help"))
	{
        qDebug()<<"\n==== save RGB channel ====";
        qDebug()<<"Usage: v3d -x dllname -f save_RGB_channel -i <input_image> -o <output_image> ";
        qDebug()<<"\n";
	}
	else return false;

	return true;
}

