#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include "binarization_func.h"

//#include <omp.h>

using namespace std;

#define PI 3.141592653589793238462

template <class T, class U>
void BinaryProcess(T *h_Src, U *h_Dst, V3DLONG imageW, V3DLONG imageH, V3DLONG imageD, V3DLONG l, V3DLONG d, double C)
{
	long count = imageD*imageH*imageW/10L;
	long current = 0L;

	double a_interval = (PI / 2.0) / (double)l;
	double *drxtable = new double [l*2*(l*2-1)+1];
	double *drytable = new double [l*2*(l*2-1)+1];
	double *drztable = new double [l*2*(l*2-1)+1];
	int knum = l*2*(l*2-1)+1;

	drxtable[0] = 0.0; drytable[0] = 0.0; drztable[0] = 1.0;

	for(int b = 0; b < l*2; b++){
		for(int a = 1; a < l*2; a++){
			drxtable[b*(l*2-1) + (a-1) + 1] = sin(a_interval*a) * cos(a_interval*b);
			drytable[b*(l*2-1) + (a-1) + 1] = sin(a_interval*a) * sin(a_interval*b);
			drztable[b*(l*2-1) + (a-1) + 1] = cos(a_interval*a);
		}
	}

	for(int z = 0; z < imageD; z++){
		for(int y = 0; y < imageH; y++){
			#pragma omp parallel for
			for(int x = 0; x < imageW; x++){
				
				double sum;
				double rx, ry, rz, fx, fy, fz, drx, dry, drz, val;
				int ix, iy, iz;
				double verticalC = C;

				for(int i = 0; i < knum; i++){
					sum = 0;
					drx = drxtable[i];
					dry = drytable[i];
					drz = drztable[i];

					double left = 0.0;
					double right = 0.0;
					double center = 0.0;

					for(int k = -d; k < 0; k++){
						rx = x + k * drx;
						ry = y + k * dry;
						rz = z + k * drz;
						int ix = (int)(rx + 0.5);
						int iy = (int)(ry + 0.5);
						int iz = (int)(rz + 0.5);
						if(ix < 0)ix = 0;
						if(ix >= imageW)ix = imageW - 1;
						if(iy < 0)iy = 0;
						if(iy >= imageH)iy = imageH - 1;
						if(iz < 0)iz = 0;
						if(iz >= imageD)iz = imageD - 1;
						val = h_Src[iz*imageH*imageW + iy*imageW + ix];

						left += val;//(float)(val * h_Kernel[kernelR - k]);
					}

					center = h_Src[z*imageH*imageW + y*imageW + x];;

					for(int k = 1; k <= d; k++){
						rx = x + k * drx;
						ry = y + k * dry;
						rz = z + k * drz;
						int ix = (int)(rx + 0.5);
						int iy = (int)(ry + 0.5);
						int iz = (int)(rz + 0.5);
						if(ix < 0)ix = 0;
						if(ix >= imageW)ix = imageW - 1;
						if(iy < 0)iy = 0;
						if(iy >= imageH)iy = imageH - 1;
						if(iz < 0)iz = 0;
						if(iz >= imageD)iz = imageD - 1;
						val = h_Src[iz*imageH*imageW + iy*imageW + ix];

						right += val;//(float)(val * h_Kernel[kernelR - k]);
					}

					sum = right + center + left;
					sum /= (d*2+1);
					//double baseth = (sum + abs(right - left)/2.0)/(d*2+1);

					//sum += verticalC;//pararelC*(1.0 - abs(a*a_interval*2.0/PI - 1.0)) + verticalC * abs(a*a_interval*2.0/PI - 1.0);
					h_Dst[z*imageW*imageH + y*imageW + x] = h_Src[z*imageW*imageH + y*imageW + x] > 127.0+C*(sum-127.0) ? 255.0 : 0;
				}

				current++;
				if(current == count){
					current = 0L;
					putchar('>');
				}
			}
		}
	}
	putchar('\n');

	delete [] drxtable;
	delete [] drytable;
	delete [] drztable;
}


//experimental
template <class T, class U>
void BinaryProcess2(T *h_Src, U *h_Dst, V3DLONG imageW, V3DLONG imageH, V3DLONG imageD, V3DLONG l, V3DLONG d, double C)
{
	long count = imageD*imageH*imageW/10L;
	long current = 0L;

	double vmax = -1;
	for(int z = 0; z < imageD; z++){
		for(int y = 0; y < imageH; y++){
			for(int x = 0; x < imageW; x++){
				if(vmax < h_Src[z * imageW * imageH + y * imageW + x])vmax = h_Src[z * imageW * imageH + y * imageW + x];
			}
		}
	}

	double a_interval = (PI / 2.0) / (double)l;
	double *slatitable = new double [l*2*(l*2-1)+1];
	double *clatitable = new double [l*2*(l*2-1)+1];
	double *slongitable = new double [l*2*(l*2-1)+1];
	double *clongitable = new double [l*2*(l*2-1)+1];
	int knum = l*2*(l*2-1)+1;

	slatitable[0] = 0.0; clatitable[0] = 1.0; slongitable[0] = 0.0; clongitable[0] = 0.0;

	for(int b = 0; b < l*2; b++){
		for(int a = 1; a < l*2; a++){
			slatitable[b*(l*2-1) + (a-1) + 1] = sin(a_interval*a);
			clatitable[b*(l*2-1) + (a-1) + 1] = cos(a_interval*a);
			slongitable[b*(l*2-1) + (a-1) + 1] = sin(a_interval*b);
			clongitable[b*(l*2-1) + (a-1) + 1] = cos(a_interval*b);
		}
	}

	for(int z = 0; z < imageD; z++){
		for(int y = 0; y < imageH; y++){
			#pragma omp parallel for
			for(int x = 0; x < imageW; x++){
				double sum;
				double rx, ry, rz, fx, fy, fz, drx, dry, drz, val;
				int ix, iy, iz;
				double sinlongi, sinlati;
				double coslongi, coslati;
				
				double bval = -DBL_MAX;
				
				for(int i = 0; i < knum; i++){
					sum = 0;
					drx = slatitable[i]*clongitable[i];
					dry = slatitable[i]*slongitable[i];
					drz = clatitable[i];

					double left = 0.0;
					double right = 0.0;
					double center;

					for(int k = -d; k < 0; k++){
						/*
						rx = x + k * drx;
						ry = y + k * dry;
						rz = z + k * drz;

						if(rx < 0)rx = 0;
						if(rx >= imageW)rx = imageW - 1;
						if(ry < 0)ry = 0;
						if(ry >= imageH)ry = imageH - 1;
						if(rz < 0)rz = 0;
						if(rz >= imageD)rz = imageD - 1;

						ix = (int)rx;
						iy = (int)ry;
						iz = (int)rz;

						if(ix == imageW - 1)ix--;
						if(iy == imageH - 1)iy--;
						if(iz == imageD - 1)iz--;

						fx = rx - ix;
						fy = ry - iy;
						fz = rz - iz;

						float c000 = h_Src[iz*imageH*imageW + iy*imageW + ix];
						float c100 = h_Src[iz*imageH*imageW + iy*imageW + (ix + 1)];
						float c010 = h_Src[iz*imageH*imageW + (iy + 1)*imageW + ix];
						float c001 = h_Src[(iz + 1)*imageH*imageW + iy*imageW + ix];
						float c101 = h_Src[(iz + 1)*imageH*imageW + iy*imageW + (ix + 1)];
						float c011 = h_Src[(iz + 1)*imageH*imageW + (iy + 1)*imageW + ix];
						float c110 = h_Src[iz*imageH*imageW + (iy + 1)*imageW + (ix + 1)];
						float c111 = h_Src[(iz + 1)*imageH*imageW + (iy + 1)*imageW + (ix + 1)];
						val = c000*(1.0f-fx)*(1.0f-fy)*(1.0f-fz) + 
						c100*fx*(1.0f-fy)*(1.0f-fz) + 
						c010*(1.0f-fx)*fy*(1.0f-fz) +
						c001*(1.0f-fx)*(1.0f-fy)*fz +
						c101*fx*(1.0f-fy)*fz +
						c011*(1.0f-fx)*fy*fz +
						c110*fx*fy*(1.0f-fz) +
						c111*fx*fy*fz;
						*/
						rx = x + k * drx;
						ry = y + k * dry;
						rz = z + k * drz;
						int ix = (int)(rx + 0.5);
						int iy = (int)(ry + 0.5);
						int iz = (int)(rz + 0.5);
						if(ix < 0)ix = 0;
						if(ix >= imageW)ix = imageW - 1;
						if(iy < 0)iy = 0;
						if(iy >= imageH)iy = imageH - 1;
						if(iz < 0)iz = 0;
						if(iz >= imageD)iz = imageD - 1;
						val = h_Src[iz*imageH*imageW + iy*imageW + ix];

						left += val;//(float)(val * h_Kernel[kernelR - k]);

					}

					center = h_Src[z * imageW * imageH + y * imageW + x];

					for(int k = 1; k <= d; k++){
						rx = x + k * drx;
						ry = y + k * dry;
						rz = z + k * drz;
						int ix = (int)(rx + 0.5);
						int iy = (int)(ry + 0.5);
						int iz = (int)(rz + 0.5);
						if(ix < 0)ix = 0;
						if(ix >= imageW)ix = imageW - 1;
						if(iy < 0)iy = 0;
						if(iy >= imageH)iy = imageH - 1;
						if(iz < 0)iz = 0;
						if(iz >= imageD)iz = imageD - 1;
						val = h_Src[iz*imageH*imageW + iy*imageW + ix];

						right += val;//(float)(val * h_Kernel[kernelR - k]);

					}

					sum = left + center + right;
					sum = (sum - abs(left - right)/2)/(d*2+1);

					//pararelC*(1.0 - abs(a*a_interval*2.0/PI - 1.0)) + verticalC * abs(a*a_interval*2.0/PI - 1.0);
					if(bval < sum){
						bval = sum;
						sinlati = slatitable[i]; sinlongi = slongitable[i];
						coslati = clatitable[i]; coslongi = clongitable[i];
					}
				}

				double drx2, dry2, drz2;
				double maxval = -DBL_MAX;
				for(int a = 0; a < l*2; a++){
					double bx, by, bz;
					bx = cos(a_interval*a); by = sin(a_interval*a); bz = 0.0;
					drx2 = bx*coslati*coslongi - by*sinlongi + bz*sinlati*coslongi;
					dry2 = bx*coslati*sinlongi + by*coslongi + bz*sinlati*sinlongi;
					drz2 = -bx*sinlati + bz*coslati;

					sum = 0.0;
					double left = 0.0;
					double right = 0.0;
					double center = 0.0;

					for(int k = -d; k < 0; k++){
						rx = x + k * drx;
						ry = y + k * dry;
						rz = z + k * drz;
						int ix = (int)(rx + 0.5);
						int iy = (int)(ry + 0.5);
						int iz = (int)(rz + 0.5);
						if(ix < 0)ix = 0;
						if(ix >= imageW)ix = imageW - 1;
						if(iy < 0)iy = 0;
						if(iy >= imageH)iy = imageH - 1;
						if(iz < 0)iz = 0;
						if(iz >= imageD)iz = imageD - 1;
						val = h_Src[iz*imageH*imageW + iy*imageW + ix];

						left += val;//(float)(val * h_Kernel[kernelR - k]);

					}

					center = h_Src[z * imageW * imageH + y * imageW + x];

					for(int k = 1; k <= d; k++){
						rx = x + k * drx;
						ry = y + k * dry;
						rz = z + k * drz;
						int ix = (int)(rx + 0.5);
						int iy = (int)(ry + 0.5);
						int iz = (int)(rz + 0.5);
						if(ix < 0)ix = 0;
						if(ix >= imageW)ix = imageW - 1;
						if(iy < 0)iy = 0;
						if(iy >= imageH)iy = imageH - 1;
						if(iz < 0)iz = 0;
						if(iz >= imageD)iz = imageD - 1;
						val = h_Src[iz*imageH*imageW + iy*imageW + ix];

						right += val;//(float)(val * h_Kernel[kernelR - k]);

					}

					sum = right + center + left;
					double baseth = (sum + abs(right - left)/2.0)/(d*2+1);

					if(maxval < baseth) maxval = baseth;
				}

//				h_Dst[z * imageW * imageH + y * imageW + x] = h_Src[z * imageW * imageH + y * imageW + x] > maxval ? 
//					(h_Src[z * imageW * imageH + y * imageW + x] - maxval)*(bval>maxval ? sqrt(1.0 + (bval-maxval)/vmax*255.0) : 0.0) : 0;
				h_Dst[z * imageW * imageH + y * imageW + x] = h_Src[z * imageW * imageH + y * imageW + x] > 127.0+C*(maxval-127.0) ? 255.0 : 0;

				current++;
				if(current == count){
					current = 0L;
					putchar('>');
				}
			}
		}
	}
	putchar('\n');

	delete [] slatitable;
	delete [] clatitable;
	delete [] slongitable;
	delete [] clongitable;
}

void binarization_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu, int mode)
{
	unsigned char* data1d = 0;
	V3DLONG N,M,P,sc,c,l,d;
	V3DLONG in_sz[4];
	ImagePixelType datatype;
	if(bmenu)
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


		data1d = p4DImage->getRawData();
		N = p4DImage->getXDim();
		M = p4DImage->getYDim();
		P = p4DImage->getZDim();
		sc = p4DImage->getCDim();
		datatype = p4DImage->getDatatype();

		bool ok1;

		if(sc==1)
		{
			c=0;
			ok1=true;
		}
		else
		{
			c = QInputDialog::getInteger(parent, "Channel",
											 "Enter channel NO:",
											 1, 1, sc, 1, &ok1);
			c--;
		}

		if(!ok1)
			return;

		in_sz[0] = N;
		in_sz[1] = M;
		in_sz[2] = P;
		in_sz[3] = sc;


		PARA.inimg_file = p4DImage->getFileName();

		DSLTDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;
		else
		{
			l = dialog.Dlevel->text().toLong();
			d = dialog.Dnumber->text().toLong();
			printf("%d l,%d d \n ", l, d);
		}
	}
	else
	{
		if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, (int &)datatype))
		{
			fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
			return;
		}
		if(PARA.channel < 1 || PARA.channel > in_sz[3])
		{
			fprintf (stderr, "Invalid channel number. \n");
			return;
		}
		N = in_sz[0];
		M = in_sz[1];
		P = in_sz[2];
		sc = in_sz[3];
		c = PARA.channel-1;
		l = PARA.l;
		d = PARA.d;
	}

	double C = 0.95;

	//main neuron reconstruction code

	//// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE
	V3DLONG channelsz = N*M*P;
	void *pData=NULL;

	switch (datatype)
	{
	case V3D_UINT8:
		try
		{
			pData = (void *)(new unsigned char [channelsz]);
		}
		catch (...)
		{
			v3d_msg("Fail to allocate memory in Distance Transform.",0);
			if (pData) {delete []pData; pData=0;}
			return;
		}

		{
			unsigned char * pSubtmp_uint8 = (unsigned char *)data1d;
			if(mode == 0)BinaryProcess(pSubtmp_uint8+c*channelsz, (unsigned char *)pData, N, M, P, l, d, C);
			if(mode == 1)BinaryProcess2(pSubtmp_uint8+c*channelsz, (unsigned char *)pData, N, M, P, l, d, C);
		}
		break;

	case V3D_UINT16:
		try
		{
			pData = (void *)(new short int [channelsz]);
		}
		catch (...)
		{
			v3d_msg("Fail to allocate memory in Distance Transform.",0);
			if (pData) {delete []pData; pData=0;}
			return;
		}

		{
			short int * pSubtmp_uint16 = (short int *)data1d;
			if(mode == 0)BinaryProcess(pSubtmp_uint16+c*channelsz, (short int *)pData, N, M, P, l, d, C);
			if(mode == 1)BinaryProcess2(pSubtmp_uint16+c*channelsz, (short int *)pData, N, M, P, l, d, C);
		}

		break;

	case V3D_FLOAT32:
		try
		{
			pData = (void *)(new float [channelsz]);
		}
		catch (...)
		{
			v3d_msg("Fail to allocate memory in Distance Transform.",0);
			if (pData) {delete []pData; pData=0;}
			return;
		}

		{
			float * pSubtmp_float32 = (float *)data1d;
			if(mode == 0)BinaryProcess(pSubtmp_float32+c*channelsz, (float *)pData, N, M, P, l, d, C);
			if(mode == 1)BinaryProcess(pSubtmp_float32+c*channelsz, (float *)pData, N, M, P, l, d, C);
//			for (V3DLONG ich=0; ich<sc; ich++)
//				BinaryProcess(pSubtmp_float32+ich*channelsz, (float *)pData+ich*channelsz, N, M, P, d );
		}

		break;

	default:
		break;
	}

	if(bmenu)
	{
		Image4DSimple * new4DImage = new Image4DSimple();
		new4DImage->setData((unsigned char *)pData, N, M, P, 1, datatype);
		
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();
				
		callback.setImage(newwin, new4DImage);
		callback.setImageName(newwin, QObject::tr("thresholded image"));
		callback.updateImageWindow(newwin);
	}
		
/*
	QString swc_name;


	//Output
	NeuronTree nt;
	writeSWC_file(swc_name.toStdString().c_str(),nt);
*/
	if(!bmenu)
	{
		if(data1d) {delete []data1d; data1d = 0;}
	}

	//v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);

	return;
}

void DSLTDialog::update()
{
	//get current data
	Dl = Dlevel->text().toLong()-1;
	Dn = Dnumber->text().toLong()-1;
}
