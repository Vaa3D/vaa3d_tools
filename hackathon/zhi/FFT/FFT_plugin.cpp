/* FFT_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-07-29 : by Zhi Zhou 
 */
 
#include "v3d_message.h"
#include <vector>
#include "FFT_plugin.h"
#include "fftw3.h"


using namespace std;
Q_EXPORT_PLUGIN2(FFT, highpass3d_fftw);

void processImage(V3DPluginCallback2 &callback, QWidget *parent);
//bool processImage(const V3DPluginArgList & input, V3DPluginArgList & output);


template <class T> void FFT_HP(T* data1d,
                                      V3DLONG *in_sz,
                                      unsigned int c,
                                      T* &outimg);
 
QStringList highpass3d_fftw::menulist() const
{
	return QStringList() 
		<<tr("highpass3d_fftw")
		<<tr("about");
}

QStringList highpass3d_fftw::funclist() const
{
	return QStringList()
		<<tr("highpass3d_fftw")
		<<tr("help");
}

void highpass3d_fftw::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("highpass3d_fftw"))
    {
        processImage(callback,parent);
	}
	else
	{
        v3d_msg(tr("This is a test plugin, you can use it as a demo.."
			"Developed by Zhi Zhou , 2013-07-29"));
	}
}

bool highpass3d_fftw::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("highpass3d_fftw"))
	{
	}
	else if (func_name == tr("help"))
	{
    }
	else return false;
}

void processImage(V3DPluginCallback2 &callback, QWidget *parent)
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

    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

     // filter
     V3DLONG in_sz[4];
     in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;
    unsigned int c = 1;

    ImagePixelType pixeltype = p4DImage->getDatatype();
    void* outimg = 0;
    switch (pixeltype)
    {
          case V3D_UINT8: FFT_HP(data1d, in_sz, c,(unsigned char* &)outimg); break;
          case V3D_UINT16: FFT_HP((unsigned short int *)data1d, in_sz, c, (unsigned short int* &)outimg); break;
          case V3D_FLOAT32: FFT_HP((float *)data1d, in_sz, c,(float* &)outimg);break;
          default: v3d_msg("Invalid data type. Do nothing."); return;
    }

     // display
     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)outimg, V3DLONG(N), V3DLONG(M), V3DLONG(P), 1, pixeltype);
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.setImageName(newwin, "3D enhancement result");
     callback.updateImageWindow(newwin);

}


template <class T> void FFT_HP(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int c,
                     T* &outimg)
{

         V3DLONG N = in_sz[0];
         V3DLONG M = in_sz[1];
         V3DLONG P = in_sz[2];

         V3DLONG pagesz = N*M*P;
         V3DLONG offsetc = (c-1)*pagesz;

         T *pImage = new T [pagesz];
        fftwf_complex* buf1= new fftwf_complex[pagesz];
        fftwf_complex* buf2= new fftwf_complex[pagesz];
        if (!pImage)
        {
            printf("Fail to allocate memory.\n");
            return;
         }
         else
         {
            for(V3DLONG i=0; i<pagesz; i++)
                pImage[i] = 0;
          }

         //outimg = pImage;fftw_plan_dft_3d
         V3DLONG i = 0;
        for(V3DLONG iz = 0; iz < P; iz++)
        {
            V3DLONG offsetk = iz*M*N;
            for(V3DLONG iy = 0; iy < M; iy++)
            {
                V3DLONG offsetj = iy*N;
                for(V3DLONG ix = 0; ix < N; ix++)
                {
                       buf1[i][0]	= (float)data1d[offsetc+offsetk + offsetj + ix];;
                       buf1[i][1]	= 0;
                       i++;
                }

            }

        }

        // Transform to frequency space.
        fftwf_plan pFwd = fftwf_plan_dft_3d(N, M, P, buf1, buf2, FFTW_FORWARD, FFTW_ESTIMATE);
        fftwf_execute(pFwd);
        fftwf_destroy_plan(pFwd);

        // Zap the DC value
        buf2[0][0]	= 0;
        buf2[0][1]	= 0;

        // Transform back to image space.
        fftwf_plan pBack	= fftwf_plan_dft_3d(N, M, P, buf2, buf1, FFTW_BACKWARD, FFTW_ESTIMATE);
        fftwf_execute(pBack);
        fftwf_destroy_plan(pBack);

        // Have to scale the output values to get back to the original.
        for(int i = 0; i < pagesz; i++) {
            buf1[i][0]	= buf1[i][0] / pagesz;
            buf1[i][1]	= buf1[i][1] / pagesz;
        }

        //outimg = pImage;
       i = 0;
       for(V3DLONG iz = 0; iz < P; iz++)
       {
           V3DLONG offsetk = iz*M*N;
           for(V3DLONG iy = 0; iy < M; iy++)
           {
               V3DLONG offsetj = iy*N;
               for(V3DLONG ix = 0; ix < N; ix++)
               {
                   double re	= buf1[i][0];
                   double im	= buf1[i][1];
                   double mag	= sqrt (re*re + im*im);
               //    if(mag>10)
                   pImage[offsetk + offsetj + ix] = (u_int8_t)mag;
                   i++;
               }

           }

       }
        outimg = pImage;
        // Cleanup.
        fftwf_free(buf1);
        fftwf_free(buf2);
        return;

}
