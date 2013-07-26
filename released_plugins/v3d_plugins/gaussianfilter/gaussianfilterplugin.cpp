/* gaussfilter.cxx
 * 2009-06-03: create this program by Yang Yu
 * 2009-08-14: change into plugin by Yang Yu
 */

// Adapted and upgraded to V3DPluginInterface2_1 by Jianlong Zhou, 2012-04-05
// add dofunc() by Jianlong Zhou, 2012-04-08

#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <iostream>

#include "v3d_message.h"
#include "stackutil.h"

#include "gaussianfilterplugin.h"

using namespace std;

#define INF 1E9

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(gaussianfilter, GaussianFilterPlugin)


void processImage(V3DPluginCallback2 &callback, QWidget *parent);
bool processImage(const V3DPluginArgList & input, V3DPluginArgList & output);
template <class T> void gaussian_filter(T* data1d,
                                        V3DLONG *in_sz,
                                        unsigned int Wx,
                                        unsigned int Wy,
                                        unsigned int Wz,
                                        unsigned int c,
                                        double sigma,
                                        float* &outimg);

const QString title = QObject::tr("Gaussian Filter Plugin");
QStringList GaussianFilterPlugin::menulist() const
{
    return QStringList() << tr("Gaussian Filter") << tr("About");
}

void GaussianFilterPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Gaussian Filter"))
	{
		processImage(callback,parent);
	}
	else if (menu_name == tr("About"))
	{
		v3d_msg("Gaussian filter.");
	}
}

QStringList GaussianFilterPlugin::funclist() const
{
	return QStringList()
		<<tr("gf")
		<<tr("help");
}


bool GaussianFilterPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
     if (func_name == tr("gf"))
	{
		return processImage(input, output);
	}
	else if(func_name == tr("help"))
	{
		cout<<"Usage : v3d -x gaussian -f gf -i <inimg_file> -o <outimg_file> -p <wx> <wy> <wz> <channel> <sigma>"<<endl;
		cout<<endl;
		cout<<"wx          filter window size (pixel #) in x direction, default 7 and maximum image xsize-1"<<endl;
		cout<<"wy          filter window size (pixel #) in y direction, default 7 and maximum image ysize-1"<<endl;
		cout<<"wz          filter window size (pixel #) in z direction, default 3 and maximum image zsize-1"<<endl;
		cout<<"channel     the input channel value, default 1 and start from 1"<<endl;
		cout<<"sigma       filter sigma, default 1.0"<<endl;
		cout<<endl;
		cout<<"e.g. v3d -x gaussian -f gf -i input.raw -o output.raw -p 3 3 3 1 1.0"<<endl;
		cout<<endl;
		return true;
	}
}

bool processImage(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to Gaussian filter"<<endl;
	if (output.size() != 1) return false;

	unsigned int Wx=7, Wy=7, Wz=3, c=1;
     float sigma = 1.0;
     if (input.size()>=2)
     {
          vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
          if(paras.size() >= 1) Wx = atoi(paras.at(0));
          if(paras.size() >= 2) Wy = atoi(paras.at(1));
          if(paras.size() >= 3) Wz = atoi(paras.at(2));
          if(paras.size() >= 4) c = atoi(paras.at(3));
          if(paras.size() >= 5) sigma = atof(paras.at(4));
	}

	char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
	char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
	cout<<"Wx = "<<Wx<<endl;
     cout<<"Wy = "<<Wy<<endl;
	cout<<"Wz = "<<Wz<<endl;
     cout<<"c = "<<c<<endl;
     cout<<"sigma = "<<sigma<<endl;
	cout<<"inimg_file = "<<inimg_file<<endl;
	cout<<"outimg_file = "<<outimg_file<<endl;

     double sigma_s2 = 0.5/(sigma*sigma);

	unsigned char * data1d = 0,  * outimg1d = 0;
	float * phi = 0;
	V3DLONG * in_sz = 0;

	int datatype;
	if(!loadImage(inimg_file, data1d, in_sz, datatype))
     {
          cerr<<"load image "<<inimg_file<<" error!"<<endl;
          return false;
     }

     if(c > in_sz[3])// check the input channel number range
     {
          v3d_msg("The input channel number is out of real channel range.\n", 0 );
          return false;
     }

	//input
     float* outimg = 0;

     switch (datatype)
     {
          case 1: gaussian_filter(data1d, in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
          case 2: gaussian_filter((unsigned short int*)data1d, in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
          case 4: gaussian_filter((float *)data1d, in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
          default:
               v3d_msg("Invalid datatype.");
               if (data1d) {delete []data1d; data1d=0;}
               if (in_sz) {delete []in_sz; in_sz=0;}
               return false;
     }

     // save image
     in_sz[3]=1;
     saveImage(outimg_file, (unsigned char *)outimg, in_sz, 4);

     if(outimg) {delete []outimg; outimg =0;}
     if(data1d) {delete []data1d; data1d=0;}
     if(in_sz)  {delete []in_sz; in_sz=0;}

     return true;
}


void processImage(V3DPluginCallback2 &callback, QWidget *parent)
{
     v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
        v3d_msg("You don't have any image open in the main window.");
		return;
	}

     Image4DSimple* p4DImage = callback.getImage(curwin);

	if (!p4DImage)
	{
		v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
		return;
	}


     unsigned char* data1d = p4DImage->getRawData();
     V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

     V3DLONG N = p4DImage->getXDim();
     V3DLONG M = p4DImage->getYDim();
     V3DLONG P = p4DImage->getZDim();
     V3DLONG sc = p4DImage->getCDim();

	//input
	bool ok1, ok2, ok3, ok4, ok5;
	unsigned int Wx=1, Wy=1, Wz=1, c=1;
     double sigma = 1.0;

	Wx = QInputDialog::getInteger(parent, "Window X ",
								  "Enter window size (# voxels) along x:",
								  7, 1, N, 1, &ok1);

	if(ok1)
	{
		Wy = QInputDialog::getInteger(parent, "Window Y",
									  "Enter window size  (# voxels) along y:",
									  7, 1, M, 1, &ok2);
	}
	else
		return;

	if(ok2)
	{
		Wz = QInputDialog::getInteger(parent, "Window Z",
									  "Enter window size  (# voxels) along z:",
									  3, 1, P, 1, &ok3);
	}
	else
		return;

	if(ok3)
	{
		c = QInputDialog::getInteger(parent, "Channel",
									 "Enter channel NO (starts from 1):",
									 1, 1, sc, 1, &ok4);
	}
	else
		return;

     if(ok4)
	{
		sigma = QInputDialog::getDouble(parent, "Sigma",
									 "Enter Sigma value:",
									 1, 0.1, 100, 1, &ok5);
	}
	else
		return;

     // gaussian_filter
     V3DLONG in_sz[4];
     in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;

    float* outimg = 0;
    switch (p4DImage->getDatatype())
    {
        case V3D_UINT8: gaussian_filter(data1d, in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
        case V3D_UINT16: gaussian_filter((unsigned short int *)data1d, in_sz, Wx, Wy, Wz, c, sigma, outimg); break;
        case V3D_FLOAT32: gaussian_filter((float *)data1d, in_sz, Wx, Wy, Wz, c, sigma, outimg);break;
        default: v3d_msg("Invalid data type. Do nothing."); return;
    }

     // display
     Image4DSimple * new4DImage = new Image4DSimple();
     new4DImage->setData((unsigned char *)outimg, N, M, P, 1, V3D_FLOAT32);
     v3dhandle newwin = callback.newImageWindow();
     callback.setImage(newwin, new4DImage);
     callback.setImageName(newwin, title);
     callback.updateImageWindow(newwin);
}



template <class T> void gaussian_filter(T* data1d,
                     V3DLONG *in_sz,
                     unsigned int Wx,
                     unsigned int Wy,
                     unsigned int Wz,
                     unsigned int c,
                     double sigma,
                     float* &outimg)
{
    if (!data1d || !in_sz || in_sz[0]<=0 || in_sz[1]<=0 || in_sz[2]<=0 || in_sz[3]<=0 || outimg)
    {
        v3d_msg("Invalid parameters to gaussian_filter().", 0);
        return;
    }

     // for filter kernel
     double sigma_s2 = 0.5/(sigma*sigma); // 1/(2*sigma*sigma)
     double pi_sigma = 1.0/(sqrt(2*3.1415926)*sigma); // 1.0/(sqrt(2*pi)*sigma)

     float min_val = INF, max_val = 0;

     V3DLONG N = in_sz[0];
     V3DLONG M = in_sz[1];
     V3DLONG P = in_sz[2];
     V3DLONG sc = in_sz[3];
     V3DLONG pagesz = N*M*P;

     //filtering
     V3DLONG offset_init = (c-1)*pagesz;

     //declare temporary pointer
     float *pImage = new float [pagesz];
     if (!pImage)
     {
          printf("Fail to allocate memory.\n");
          return;
     }
     else
     {
          for(V3DLONG i=0; i<pagesz; i++)
               pImage[i] = data1d[i + offset_init];  //first channel data (red in V3D, green in ImageJ)
     }
       //Filtering
     //
     //   Filtering along x
     if(N<2)
     {
          //do nothing
     }
     else
     {
          //create Gaussian kernel
          float  *WeightsX = 0;
		  if(Wx % 2 == 0) Wx = Wx + 1; // Revised by Zhi Zhou 
          WeightsX = new float [Wx];
          if (!WeightsX)
               return;

          unsigned int Half = Wx >> 1;

          WeightsX[Half] = 1.;

          // Gaussian filter equation:
          // http://en.wikipedia.org/wiki/Gaussian_blur
          for (unsigned int Weight = 1; Weight < Half + 1; ++Weight)
          {
               const float  x = 3.* float (Weight) / float (Half);
               WeightsX[Half - Weight] = WeightsX[Half + Weight] = pi_sigma * exp(-x * x *sigma_s2); /// 2.);	// Corresponding symmetric WeightsX
          }

          double k = 0.;
          for (unsigned int Weight = 0; Weight < Wx; ++Weight)
               k += WeightsX[Weight];

          for (unsigned int Weight = 0; Weight < Wx; ++Weight)
               WeightsX[Weight] /= k;


          //   Allocate 1-D extension array
          float  *extension_bufferX = 0;
          extension_bufferX = new float [N + (Wx<<1)];

          unsigned int offset = Wx>>1;

          //	along x
          const float  *extStop = extension_bufferX + N + offset;
         
          for(V3DLONG iz = 0; iz < P; iz++)
          {
               for(V3DLONG iy = 0; iy < M; iy++)
               {
                    float  *extIter = extension_bufferX + Wx;
                    for(V3DLONG ix = 0; ix < N; ix++)
                    {
                         *(extIter++) = pImage[iz*M*N + iy*N + ix];
                    }

                    //   Extend image
                    const float  *const stop_line = extension_bufferX - 1;
                    float  *extLeft = extension_bufferX + Wx - 1;
                    const float  *arrLeft = extLeft + 2;
                    float  *extRight = extLeft + N + 1;
                    const float  *arrRight = extRight - 2;

                    while (extLeft > stop_line)
                    {
                         *(extLeft--) = *(arrLeft++);
                         *(extRight++) = *(arrRight--);
      
                    }

                    //	Filtering
                    extIter = extension_bufferX + offset;

                    float  *resIter = &(pImage[iz*M*N + iy*N]);

                    while (extIter < extStop)
                    {
                         double sum = 0.;
                         const float  *weightIter = WeightsX;
                         const float  *const End = WeightsX + Wx;
                         const float * arrIter = extIter;
                         while (weightIter < End)
                              sum += *(weightIter++) * float (*(arrIter++));
                         extIter++;
                         *(resIter++) = sum;

                         //for rescale
                         if(max_val<*arrIter) max_val = *arrIter;
                         if(min_val>*arrIter) min_val = *arrIter;
                  

                    }
       
               }
          }
          //de-alloc
           if (WeightsX) {delete []WeightsX; WeightsX=0;}
           if (extension_bufferX) {delete []extension_bufferX; extension_bufferX=0;}

     }

     //   Filtering along y
     if(M<2)
     {
          //do nothing
     }
     else
     {
          //create Gaussian kernel
          float  *WeightsY = 0;		  
		  if(Wy % 2 == 0) Wy = Wy + 1; // Revised by Zhi Zhou 
          WeightsY = new float [Wy];
          if (!WeightsY)
               return;

          unsigned int Half = Wy >> 1;

          WeightsY[Half] = 1.;

          for (unsigned int Weight = 1; Weight < Half + 1; ++Weight)
          {
               const float  y = 3.* float (Weight) / float (Half);
               WeightsY[Half - Weight] = WeightsY[Half + Weight] = pi_sigma * exp(-y * y * sigma_s2);	// Corresponding symmetric WeightsY
          }

          double k = 0.;
          for (unsigned int Weight = 0; Weight < Wy; ++Weight)
               k += WeightsY[Weight];

          for (unsigned int Weight = 0; Weight < Wy; ++Weight)
               WeightsY[Weight] /= k;

          //	along y
          float  *extension_bufferY = 0;
          extension_bufferY = new float [M + (Wy<<1)];

          unsigned int offset = Wy>>1;
          const float *extStop = extension_bufferY + M + offset;

          for(V3DLONG iz = 0; iz < P; iz++)
          {
               for(V3DLONG ix = 0; ix < N; ix++)
               {
                    float  *extIter = extension_bufferY + Wy;
                    for(V3DLONG iy = 0; iy < M; iy++)
                    {
                         *(extIter++) = pImage[iz*M*N + iy*N + ix];
                    }

                    //   Extend image
                    const float  *const stop_line = extension_bufferY - 1;
                    float  *extLeft = extension_bufferY + Wy - 1;
                    const float  *arrLeft = extLeft + 2;
                    float  *extRight = extLeft + M + 1;
                    const float  *arrRight = extRight - 2;

                    while (extLeft > stop_line)
                    {
                         *(extLeft--) = *(arrLeft++);
                         *(extRight++) = *(arrRight--);
                    }

                    //	Filtering
                    extIter = extension_bufferY + offset;

                    float  *resIter = &(pImage[iz*M*N + ix]);

                    while (extIter < extStop)
                    {
                         double sum = 0.;
                         const float  *weightIter = WeightsY;
                         const float  *const End = WeightsY + Wy;
                         const float * arrIter = extIter;
                         while (weightIter < End)
                              sum += *(weightIter++) * float (*(arrIter++));
                         extIter++;
                         *resIter = sum;
                         resIter += N;

                         //for rescale
                         if(max_val<*arrIter) max_val = *arrIter;
                         if(min_val>*arrIter) min_val = *arrIter;

          
                    }
               
               }
          }

          //de-alloc
          if (WeightsY) {delete []WeightsY; WeightsY=0;}
          if (extension_bufferY) {delete []extension_bufferY; extension_bufferY=0;}


     }

     //  Filtering  along z
     if(P<2)
     {
          //do nothing
     }
     else
     {
          //create Gaussian kernel
          float  *WeightsZ = 0;
		 if(Wz % 2 == 0) Wz = Wz + 1; // Revised by Zhi Zhou 

          WeightsZ = new float [Wz];
          if (!WeightsZ)
               return;

          unsigned int Half = Wz >> 1;
          WeightsZ[Half] = 1.;

          for (unsigned int Weight = 1; Weight < Half + 1; ++Weight)
          {
               const float  z = 3.* float (Weight) / float (Half);
               WeightsZ[Half - Weight] = WeightsZ[Half + Weight] = pi_sigma * exp(-z * z * sigma_s2) ; /// 2.);	// Corresponding symmetric WeightsZ
          }

          double k = 0.;
          for (unsigned int Weight = 0; Weight < Wz; ++Weight)
               k += WeightsZ[Weight];

          for (unsigned int Weight = 0; Weight < Wz; ++Weight)
               WeightsZ[Weight] /= k;


          //	along z
          float  *extension_bufferZ = 0;
          extension_bufferZ = new float [P + (Wz<<1)];

          unsigned int offset = Wz>>1;
          const float *extStop = extension_bufferZ + P + offset;

          for(V3DLONG iy = 0; iy < M; iy++)
          {
               for(V3DLONG ix = 0; ix < N; ix++)
               {

                    float  *extIter = extension_bufferZ + Wz;
                    for(V3DLONG iz = 0; iz < P; iz++)
                    {
                         *(extIter++) = pImage[iz*M*N + iy*N + ix];
                    }

                    //   Extend image
                    const float  *const stop_line = extension_bufferZ - 1;
                    float  *extLeft = extension_bufferZ + Wz - 1;
                    const float  *arrLeft = extLeft + 2;
                    float  *extRight = extLeft + P + 1;
                    const float  *arrRight = extRight - 2;

                    while (extLeft > stop_line)
                    {
                         *(extLeft--) = *(arrLeft++);
                         *(extRight++) = *(arrRight--);
                    }

                    //	Filtering
                    extIter = extension_bufferZ + offset;

                    float  *resIter = &(pImage[iy*N + ix]);

                    while (extIter < extStop)
                    {
                         double sum = 0.;
                         const float  *weightIter = WeightsZ;
                         const float  *const End = WeightsZ + Wz;
                         const float * arrIter = extIter;
                         while (weightIter < End)
                              sum += *(weightIter++) * float (*(arrIter++));
                         extIter++;
                         *resIter = sum;
                         resIter += M*N;

                         //for rescale
                         if(max_val<*arrIter) max_val = *arrIter;
                         if(min_val>*arrIter) min_val = *arrIter;

                    }
               
               }
          }

          //de-alloc
          if (WeightsZ) {delete []WeightsZ; WeightsZ=0;}
          if (extension_bufferZ) {delete []extension_bufferZ; extension_bufferZ=0;}


     }

    outimg = pImage;


    return;
}



