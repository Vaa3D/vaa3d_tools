/** 
 * Written by
 *
 * 03-08-2010
 *
 * Ihor Smal
 * Nicolas Chenouard
 * Fabrice de Chaumont
 *
 * 
 * This code is under GPL License
 */


#include <QtGui>
#include <v3d_utils.h>
#include "gaussianspotdetector.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(gaussianspotdetector, GaussianSpotDetector)

QStringList GaussianSpotDetector::menulist() const
{
    return QStringList() <<tr("Gaussian Spot Detector (using FFTW)")<<tr("Wavelet Filtering (using FFTW)");  
}

void GaussianSpotDetector::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("Gaussian Spot Detector (using FFTW)"))
    {
    	GaussianFilter(callback, parent);
    }
	if (menu_name == tr("Wavelet Filtering (using FFTW)"))
    {
    	WaveletsFilter(callback, parent);
    }
}

void thresholdWaveletCoeficients(double* coefs, int N, double sigma) {
	double var = 0;
	for (int i = 0; i < N; i++) {
		var += coefs[i] * coefs[i];
	}
	var /= (N-1);
	var = sqrt(var);
	for (V3DLONG i = 0; i < N; i++) {
		coefs[i] = (abs(coefs[i]) >= var * sigma) ? coefs[i] : 0;
	}
	
}

void GaussianSpotDetector::WaveletsFilter(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* p4DImage = callback.getImage(oldwin);
	if (!p4DImage)
	{
		QMessageBox::information(0, "Wavelet Spot Detector", QObject::tr("No image is open."));
		return;
	}
	
	//get dims
    V3DLONG szx = p4DImage->getXDim();
    V3DLONG szy = p4DImage->getYDim();
    V3DLONG szz = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    V3DLONG N = szx * szy * szz;
	
	// input 
	bool ok1, ok2, ok3, okc;
	double sigma;
	int n_scales;
	
	int	c = QInputDialog::getInteger(parent, tr("Wavelet Filter Parameters"),
									 tr("Enter channel NO (starts from 1):"),
									 1, 1, sc, 1, &okc);
	
	if (okc) {
		n_scales = QInputDialog::getInteger(parent, tr("Wavelet Filter Parameters"),
										 tr("Enter number of scales: "),
										 3, 1, 7, 1, &ok1);
	} else 
		return;
	
	if (ok1) {
		sigma = QInputDialog::getDouble(parent, tr("Wavelet Filter Parameters"),
										 tr("times stdev of wavelet coefs: "),
										 2.0, 0.0, 5.0, 3, &ok2);
	} else
		return;
	
	if (ok2) 
	
		{
		
		// get 1d double array
		double* data1dD = v3d_utils::channelToDoubleArray(p4DImage, c);
		
		// FFTW
		//careful, fftw is using row major indexing = z dimension is varying first, while v3d is using column major indexing = x dimension is varying first
		//the last dimension is cut in half (x dimension for v3d convention)
		V3DLONG nxOut = szx / 2 + 1;
		V3DLONG nyOut = szy;
		V3DLONG nzOut = szz;
		V3DLONG nOut = 	nxOut * nyOut * nzOut;
		
		//create output array
		fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nOut);
		
		//prepare execution plan
		fftw_plan p;
		p = fftw_plan_dft_r2c_3d(szz, szy, szx, data1dD, out, FFTW_ESTIMATE);
		
		//execute plan
		fftw_execute(p);

			double* lowpassimage = new double[N]; 
			double* rec_image = new double[N];
			
			double** w_coef = new double* [n_scales];
			fftw_complex** mask_f = new fftw_complex* [n_scales];
			w_coef[0] = new double[n_scales * N]; 
			mask_f[0] = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * n_scales* nOut);  
			for (int i = 1; i < n_scales; i++) {	
				w_coef[i] = w_coef[i-1] + N;
				mask_f[i] = mask_f[i-1] + nOut;
			}
				
			
			// create and transform the B3 spline kernel
			//double b3_1d[] = {0.0625, 0.25, 0.375, 0.25, 0.0625};
			double* b_mask = new double[N];
			for (V3DLONG k = 0; k < szz; k++) {
				for (V3DLONG j = 0; j < szy; j++) {
					for (V3DLONG i = 0; i < szx; i++) {
						b_mask[szx*szy*k + szx*j +i] = 0;
					}
				}
			}			
			
			for (int sk = 0; sk < n_scales; sk++) {	

				int nn = pow(2, sk) - 1;
				double b3_1d_e[5+nn*4];
				V3DLONG center = (5+nn*4)/2;
				b3_1d_e[center] = 0.375;
				int iii = 0;
				for (int ii = 0; ii < nn; ii++) {	
					iii++;
					b3_1d_e[center + iii] = 0;	
					b3_1d_e[center - iii] = 0;	
				}			
				iii++;
				b3_1d_e[center + iii] = 0.25;	
				b3_1d_e[center - iii] = 0.25;	
				for (int ii = 0; ii < nn; ii++) {	
					iii++;
					b3_1d_e[center + iii] = 0;	
					b3_1d_e[center - iii] = 0;	
				}			
				iii++;
				b3_1d_e[center + iii] = 0.0625;	
				b3_1d_e[center - iii] = 0.0625;	
				
				
				for (V3DLONG k = szz/2-2*nn-2; k <= szz/2+2*nn+2; k++) {
					for (V3DLONG j = szy/2-2*nn-2; j < szy/2+2*nn+2; j++) {
						for (V3DLONG i = szx/2-2*nn-2; i < szx/2+2*nn+2; i++) {
							if (k >=0 && k < szz && j >=0 && j < szy && i >=0 && i < szx)
								b_mask[szx*szy*k + szx*j +i] = b3_1d_e[i - szx/2+2*nn+2]*
														   b3_1d_e[j - szy/2+2*nn+2]*
														   b3_1d_e[k - szz/2+2*nn+2];
						}
					}
				}			
				
				//create output array for the kernel
				p = fftw_plan_dft_r2c_3d(szz, szy, szx, b_mask, mask_f[sk], FFTW_ESTIMATE);
				fftw_execute(p);
				

				// filter
				if (sk > 0) {
					for (V3DLONG i = 0; i < nOut; i++) {
						double a0 = mask_f[sk][i][0];
						double a1 = mask_f[sk][i][1];
						double a = sqrt(a0 * a0 + a1 * a1);
						mask_f[sk][i][0] = a * mask_f[sk-1][i][0];
						mask_f[sk][i][1] = a * mask_f[sk-1][i][1];
					}
				} else {
					for (V3DLONG i = 0; i < nOut; i++) {
						double a0 = mask_f[sk][i][0];
						double a1 = mask_f[sk][i][1];
						double a = sqrt(a0 * a0 + a1 * a1);
						mask_f[sk][i][0] = a * out[i][0];
						mask_f[sk][i][1] = a * out[i][1];
					}
					
				}
				
				
				
			}
			for (int sk = 0; sk < n_scales; sk++) {	

				// inverse FFT
				p = fftw_plan_dft_c2r_3d(szz, szy, szx, mask_f[sk], rec_image, FFTW_ESTIMATE);
				fftw_execute(p);
				// FFTW-specific normalization 
				if (sk > 0) {
					for (V3DLONG i = 0; i < N; i++) {
						w_coef[sk][i] = lowpassimage[i] - rec_image[i] / N;
						lowpassimage[i] = rec_image[i] / N;
					}
				} else {
					for (V3DLONG i = 0; i < N; i++) {
						lowpassimage[i] = rec_image[i] / N;
						w_coef[sk][i] = data1dD[i] - lowpassimage[i];
					}
				}
				thresholdWaveletCoeficients(w_coef[sk], N, sigma);
			}
			
			// reconstruct image 
			
			for (V3DLONG i = 0; i < N; i++) {
				rec_image[i] = lowpassimage[i];
				//reconstruction without the firs scale
				for (int sk = 1; sk < n_scales; sk++) {
						rec_image[i] += w_coef[sk][i];
				}	
			}
			
			for (int sk = 0; sk < n_scales; sk++) {	
				//rescale for displaying
				v3d_utils::rescaleForDisplay(w_coef[sk], w_coef[sk], N, p4DImage->datatype);		
			
				// output image 
				Image4DSimple outImage;
				unsigned char* dataOut1d = v3d_utils::doubleArrayToCharArray(w_coef[sk], N, p4DImage->datatype);
				outImage.setData(dataOut1d, p4DImage->sz0, p4DImage->sz1, p4DImage->sz2, 1, p4DImage->datatype);
				v3dhandle newwin = callback.newImageWindow();
				callback.setImage(newwin, &outImage);
				char buffer [50];
				sprintf(buffer, "wavelet scale %d", sk+1);
				callback.setImageName(newwin,buffer);
			}

			// output image low pass image
			v3d_utils::rescaleForDisplay(lowpassimage, lowpassimage, N, p4DImage->datatype);
			Image4DSimple outImage;
			unsigned char* dataOut1d = v3d_utils::doubleArrayToCharArray(lowpassimage, N, p4DImage->datatype);
			outImage.setData(dataOut1d, p4DImage->sz0, p4DImage->sz1, p4DImage->sz2, 1, p4DImage->datatype);
			v3dhandle newwin = callback.newImageWindow();
			callback.setImage(newwin, &outImage);
			callback.setImageName(newwin,"Low pass scale");
			
			
			// output reconstructed image low pass image
			Image4DSimple outImage1;
			unsigned char* dataOut1d1 = v3d_utils::doubleArrayToCharArray(rec_image, N, p4DImage->datatype);
			outImage1.setData(dataOut1d1, p4DImage->sz0, p4DImage->sz1, p4DImage->sz2, 1, p4DImage->datatype);
			v3dhandle newwin1 = callback.newImageWindow();
			callback.setImage(newwin1, &outImage1);
			callback.setImageName(newwin1,"Reconstruction");
			
			
			
			//free memory
			fftw_destroy_plan(p);
			fftw_free(out);
			fftw_free(mask_f[0]);
			fftw_free(mask_f);
			delete b_mask;
			b_mask = NULL;
			delete rec_image;
			delete lowpassimage;
			rec_image = 0;
			lowpassimage = 0;
			
			
			delete [] w_coef[0];
			delete [] w_coef;
			w_coef = 0;
		
			free(data1dD);
			
	} else
		return;
	
}



void GaussianSpotDetector::GaussianFilter(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* p4DImage = callback.getImage(oldwin);
	if (!p4DImage)
	{
		QMessageBox::information(0, "Gaussian Spot Detector", QObject::tr("No image is open."));
		return;
	}
	
	//get dims
    V3DLONG szx = p4DImage->getXDim();
    V3DLONG szy = p4DImage->getYDim();
    V3DLONG szz = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    V3DLONG N = szx * szy * szz;

	// input 
	bool ok1, ok2, ok3, okc, okthresh;
	double sigmax;
	double sigmay;
	double sigmaz;
	double threshold;
	
	int	c = QInputDialog::getInteger(parent, tr("Gaussian Filter Parameters"),
									 tr("Enter channel NO (starts from 1):"),
									 1, 1, sc, 1, &okc);
		
	if (okc) {
		sigmax = QInputDialog::getDouble(parent, tr("Gaussian Filter Parameters"),
											tr("Stdev (sigma) in x-direction: "),
											6.0, 0.0, 50.0, 3, &ok1);
	} else 
		return;
	
	if (ok1) {
		sigmay = QInputDialog::getDouble(parent, tr("Gaussian Filter Parameters"),
												tr("Stdev (sigma) in y-direction: "),
												6.0, 0.0, 50.0, 3, &ok2);
	} else
		return;
	if (ok2) {
		sigmaz = QInputDialog::getDouble(parent, tr("Gaussian Filter Parameters"),
												tr("Stdev (sigma) in z-direction: "),
												6.0, 0.0, 50.0, 3, &ok3);
	} else
		return;

	if (ok3) {
		threshold = QInputDialog::getDouble(parent, tr("Gaussian Filter Parameters"),
										 tr("Intensity threshold for object detection: "),
										 60.0, 0.0, 65535, 3, &okthresh);
	} else
		return;
	
	if (okthresh) {
		
		// get 1d double array
		double* data1dD = v3d_utils::channelToDoubleArray(p4DImage, c);
	
		// FFTW
		//careful, fftw is using row major indexing = z dimension is varying first, while v3d is using column major indexing = x dimension is varying first
		//the last dimension is cut in half (x dimension for v3d convention)
		V3DLONG nxOut = szx / 2 + 1;
		V3DLONG nyOut = szy;
		V3DLONG nzOut = szz;
		V3DLONG nOut = 	nxOut * nyOut * nzOut;
	
		//create output array
		fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nOut);
	
		//prepare execution plan
		fftw_plan p;
		p = fftw_plan_dft_r2c_3d(szz, szy, szx, data1dD, out, FFTW_ESTIMATE);
	
		//execute plan
		fftw_execute(p);
	
		// create Gaussian filter in the frequency domain (sigma's units are specified in the spatial domain)
		double* mask = new double[nOut];
		const double pi = 3.141592;
		
		for (V3DLONG k = -szz/2; k <= szz/2; k++) {
			for (V3DLONG j = -szy/2; j <= szy/2; j++) {
				for (V3DLONG i = 0; i <= szx/2; i++) {
					V3DLONG x = i; 
					V3DLONG y = (j + szy) % szy;
					V3DLONG z = (k + szz) % szz;
					V3DLONG index = nxOut * szy * z + nxOut * y + x;
					mask[index] = exp(-(k * k) * sigmaz * sigmaz * 2 * pi * pi / (szz * szz)
								  -(j * j) * sigmay * sigmay * 2 * pi * pi / (szy * szy)
								  -(i * i) * sigmax * sigmax * 2 * pi * pi / (szx * szx)); 
				}
			}
		}
		
		// filter
		for (V3DLONG i = 0; i < nOut; i++) {
			// 		double a0 = out[i][0];
			//		double a1 = out[i][1];
			//		double a = sqrt(a0 * a0 + a1 * a1);
			//		double theta = atan2(a1, a0);
			//		out[i][0] = a * mask[i] * cos(theta);
			// 		out[i][1] = a * mask[i] * sin(theta);
			out[i][0] *= mask[i];
			out[i][1] *= mask[i];
		}
			
		// inverse FFT
		p = fftw_plan_dft_c2r_3d(szz, szy, szx, out, data1dD, FFTW_ESTIMATE);
		fftw_execute(p);
	
		// FFTW-specific normalization 
		for (V3DLONG i = 0; i < N; i++) {
			data1dD[i] /= N;
		}
	
		//free memory
		fftw_destroy_plan(p);
		fftw_free(out);
	
		// connected components ////////////////////////////////////////////////////////// 
		LandmarkList cmList;
		if (szz ==1) {
			cmList = v3d_utils::getConnectedComponents2D(data1dD, szx, szy, szz, threshold);
		} else {
			cmList = v3d_utils::getConnectedComponents(data1dD, szx, szy, szz, threshold);
		}
		// output image 
		Image4DSimple outImage;
		unsigned char* dataOut1d = v3d_utils::doubleArrayToCharArray(data1dD, N, p4DImage->datatype);
		outImage.setData(dataOut1d, p4DImage->sz0, p4DImage->sz1, p4DImage->sz2, 1, p4DImage->datatype);
		v3dhandle newwin = callback.newImageWindow();
		callback.setImage(newwin, &outImage);
		callback.setImageName(newwin,"Gaussian Filtering");
		callback.setLandmark(newwin, cmList); // center of mass
		//callback.setLandmark(oldwin, cmList); // center of mass

		callback.updateImageWindow(newwin);

		free(data1dD);
		} else
			return;

}

// make a copy of the original image
//void GaussianSpotDetector::Cloning(V3DPluginCallback &callback, QWidget *parent)
//{
//	v3dhandle oldwin = callback.currentImageWindow();
//	Image4DSimple* p4DImage = callback.getImage(oldwin);
//	if (!p4DImage)
//	{
//		QMessageBox::information(0, "Cloning", QObject::tr("No image is open."));
//		return;
//	}
//	double* data1dD = v3d_utils::channelToDoubleArray(p4DImage, 1);
//
//	//get dims
//    V3DLONG szx = p4DImage->getXDim();
//    V3DLONG szy = p4DImage->getYDim();
//    V3DLONG szz = p4DImage->getZDim();
//    V3DLONG sc = p4DImage->getCDim();
//    V3DLONG N = szx * szy * szz;
//  
//	Image4DSimple outImage;
//	unsigned char* dataOut1d = v3d_utils::doubleArrayToCharArray(data1dD, N, p4DImage->datatype);	
//    outImage.setData(dataOut1d, p4DImage->sz0, p4DImage->sz1, p4DImage->sz2, 1, p4DImage->datatype);
//    v3dhandle newwin = callback.newImageWindow();
//	callback.setImage(newwin, &outImage);
//	callback.setImageName(newwin,"cloning test");
//    callback.updateImageWindow(newwin);
//
//}

