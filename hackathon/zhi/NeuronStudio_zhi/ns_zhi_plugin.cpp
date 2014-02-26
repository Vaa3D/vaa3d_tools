/* ns_zhi_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-02-25 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "ns_zhi_plugin.h"



#include "image/nsimage.h"

using namespace std;
Q_EXPORT_PLUGIN2(ns_zhi, ns_zhi);

void autotrace_ns(V3DPluginCallback2 &callback, QWidget *parent);

void raster(int x, int y, int z, int value, NsImage *image )
    {
    NsPixel pixel;

    ns_pixel_init_lum_u8( &pixel, value );
    ns_image_set_pixel_ex( image, x, y, z, &pixel );
    }


 
QStringList ns_zhi::menulist() const
{
	return QStringList() 
		<<tr("trace")
		<<tr("about");
}

QStringList ns_zhi::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void ns_zhi::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("trace"))
	{
        autotrace_ns(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2013-02-25"));
	}
}

bool ns_zhi::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

void autotrace_ns(V3DPluginCallback2 &callback, QWidget *parent)
{

    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }


    unsigned char* data1d = p4DImage->getRawData();
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    bool ok1;
    int c,p = 0;

    if(sc==1)
    {
        c=1;
        ok1=true;
    }
    else
    {
        c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel NO:",
                                         1, 1, sc, 1, &ok1);
    }

    if(!ok1)
        return;


    V3DLONG pagesz = N*M*P;

    ImagePixelType pixeltype = p4DImage->getDatatype();

    NsImage image;
    ns_image_construct(&image);
    ns_image_create( &image, NS_PIXEL_LUM_U8, N, M, P, 1 );
   // ns_image_zero(&image);


    NsPixel pixel;
    V3DLONG offsetc = (c-1)*pagesz;
    for(V3DLONG z = 0; z < P; z++)
    {
         V3DLONG offsetk = z*M*N;
        for(V3DLONG y = 0; y < M; y++)
        {
            V3DLONG offsetj = y*N;
            for(V3DLONG x = 0; x < N; x++)
            {
                printf("(%d,%d,%d)\n",x,y,z);

                   int dataval = data1d[offsetc + offsetk + offsetj + x];
                   ns_pixel_init_lum_u8(&pixel,dataval);
                   printf("(%lld,%lld)\n\n",&image.bytes_per_slice,&image.bytes_per_row);
                   raster(x,y,z,dataval,&image);
                 //  ns_image_set_pixel_ex(&image,x,y,z,&pixel);
            }
        }
    }

  /*  switch (pixeltype)
    {
    case V3D_UINT8:  ns_image_create(&image, NS_PIXEL_LUM_U8, N, M, P,1);break;
    default: v3d_msg("Invalid data type. Do nothing."); return;
    }

    ns_image_zero(&image);*/
   // printf("%d,%d,%d\n",ns_image_width(&image),ns_image_height(&image),ns_image_length(&image));
    return;

}
