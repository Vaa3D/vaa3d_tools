/* smartscope_control.cpp
 * 2011-May-13: the controller program of the SmartScope. By Hanchuan Peng
 * 2011-Oct-06: change "acquire()" to "acquire_single_direction()" in "laser_scanning_imaging_entire_fov"
 *				and "laser_scanning_imaging_focuscontrol()" to use single direction scanning. ZJL
 * 2011-Oct-06: average multiple images for the display in "bright_field_focuscontrols()". ZJL
 * 2011-Oct-17: replace do_image_zigzag_acc() with conditional scanMethod. ZJL
 * 2011-Oct-17: replace acquire() with conditional scanMethod. ZJL
 */

#include <QColor>
#include <QDateTime>
#include <QtConcurrentRun>

#include <cmath>
#include <ctime>
#include <vector>
#include <cstdio>

using namespace std;

//v3d connections
#include "v3d_message.h"
#include "stackutil.h"
#include "customary_structs/v3d_imaging_para.h"

//smartscope devices
#include "smartscope_control.h"
//#include "smartscope_lsm.h"
#include "smartscope_dev_pgr.h"
using namespace FlyCapture2;

//file IO, GUI, utilities
#include "configfileio_lsm.h"
#include "smartscope_gui.h"
#include "smartscope_util.h"

const QString ssConfigFolder = "C:\\smartscope_conf\\";

#define USE_PRIOR_CORRECTION 1 // Use prior correction


// set parameters for LSM
void SSDriver::setup_lsm_parameters()
{
	Parameters_LSM para_lsm;

	//load the existing conf file
	QString m_FileName0 = ssConfigFolder + "mi_configuration.txt";
	QString m_FileName = m_FileName0;
	bool b_initnull = false;
	if(!QFile(m_FileName).exists())
	{
		m_FileName = QFileDialog::getOpenFileName(0,
												  QObject::tr("Locate the SmartScope configuration file"),
												  "/Documents",
												  QObject::tr("configuration file (*.txt)"));
		if(m_FileName.isEmpty())
			return;
	}

	if (!para_lsm.load_ParameterFile(m_FileName))
	{
		v3d_msg(QString("Fail to load the the SmartScope Configuration file [%1]").arg(m_FileName), 0);
		b_initnull=true;
	}

	// user can now define parameters
	LSM_Setup_Dialog dialog(&para_lsm, b_initnull);
	if (dialog.exec()!=QDialog::Accepted)
		return;

	dialog.copyDataOut(&para_lsm);

	//save config info
	if (!para_lsm.save_ParameterFile(m_FileName0)) //always save to the same file, do NOT use the manually selected file
	{
		v3d_msg(QString("Fail to save to the SmartScope Configuration file [%1]").arg(m_FileName0), 0);
		return;
	}
}


// scanning the entire image (entire field of view)
void SSDriver::laser_scanning_imaging_entire_fov(V3DPluginCallback &callback, QWidget *parent)
{
	LaserScanImaging myimg(callback);

	myimg.imaging_op = 0; // define imaging operation
	myimg.b_scanner_correction = true;

    if(myimg.scanMethod==0)
	{
		bool succ = myimg.acquire_single_direction(myimg.imaging_op); // single direction
		if(!succ)
		{
			if(myimg.pImFloat) {delete []myimg.pImFloat; myimg.pImFloat=NULL;}
			myimg.cleanData();
			return;
		}
	}
    if(myimg.scanMethod==1)
         myimg.acquire(); // in zigzag direction


	qDebug() << "before the data passing, myimg's pointer is now [" << myimg.pImFloat << "]";

	QString filename_img_saved;

     // save image
     if(USE_PRIOR_CORRECTION == 1)
     {
          saveCorrectedImage(callback, filename_img_saved, &myimg);
     }
     else
     {

          if (!passImagingData2V3D(callback,
                    myimg.pImFloat, //this function will do parameter-check
                    "LSM",          //LSM image
                    false,          //do NOT convert to 8bit directly
                    myimg.sx, myimg.sy, myimg.sz, myimg.sc,
                    myimg.fov_step_x, myimg.fov_step_y, myimg.fov_step_z,
                    myimg.min_fovx, myimg.min_fovy, myimg.min_fovz,
                    myimg.max_fovx, myimg.max_fovy, myimg.max_fovz,
                    myimg.ratio_x, myimg.ratio_y, myimg.ratio_z,
                    myimg.time_out, myimg.scanning_rate, myimg.ch,
                    myimg.scanMethod, myimg.b_useSimImg, myimg.b_scan_z_reverse,
                    filename_img_saved, false))
          {
			v3d_msg("Fail to pass the LSM data back to V3D.",0);
                    return;
          }
     }

	qDebug() << "after the data passing, myimg's pointer is now [" << myimg.pImFloat << "]";
    qDebug() << "after the data passing, V3D current windows's data pointer  [" <<
		callback.getImage(callback.currentImageWindow())->getRawData() << "]";

	myimg.cleanData();

	v3d_msg("Finish laser_scanning_imaging_entire_fov().", 0);
}

void SSDriver::laser_scanning_imaging_focuscontrol(V3DPluginCallback &callback, QWidget *parent)
{
	LaserScanImaging myimg(callback);
	//LaserScanImaging myimg_old(callback);

	if (!myimg.setup_parameters())
		return;

	myimg.sz = 1; // only scan one image
	myimg.imaging_op = 3; // define imaging operation

	// stop dialog
	MicroscopeFocusControls dialog;
	dialog.show();
	dialog.setText( QString("Focus Control...") );

	// data for return
	Image4DSimple p4DImage;
	v3dhandle curwin;
	V3DLONG iter=0;

     if(USE_PRIOR_CORRECTION == 1)
     {
          myimg.b_reset_PHC = true;
     }

	while(1)
	{
		QCoreApplication::processEvents();

		bool flag_stop = dialog.flag_stop;
		if(flag_stop)
		{
            //myimg.make_a_copy_PHC(false);
			break;
        }

		// open laser
		if(myimg.b_useSimImg == 0) myimg.shutter_open();
		if(myimg.scanMethod==0)
			myimg.acquire_single_direction(myimg.imaging_op); // single direction
		if(myimg.scanMethod==1)
			myimg.acquire(); // in zigzag direction

		if(myimg.b_useSimImg == 0) myimg.shutter_close_daq_reset();

          // prepare for USE_PRIOR_CORRECTION
          float *pNewImg = 0;
          V3DLONG n_sx, n_sy, n_sz;
          n_sx = n_sy = n_sz =0;

          if(USE_PRIOR_CORRECTION == 1)
          {
               myimg.make_a_copy_PHC(false);

               // substract image
               n_sx = myimg.sx - (myimg.offset_left + myimg.offset_right);
               n_sy = myimg.sy;
               n_sz = myimg.sz;
               pNewImg = new float [myimg.sc * n_sx * n_sy * n_sz];

               for(V3DLONG c=0; c<myimg.sc; c++)
               {
                    V3DLONG start_c = c * myimg.sz * myimg.sy * myimg.sx;
                    V3DLONG n_start_c = c * n_sz * n_sy * n_sx;

                    for(V3DLONG k=0; k<n_sz; k++)
                    {
                         V3DLONG start_k = k * myimg.sy * myimg.sx;
                         V3DLONG n_start_k = k * n_sy * n_sx;

                         for(V3DLONG j=0; j<n_sy; j++)
                         {
                              V3DLONG start_y = j * myimg.sx;
                              V3DLONG n_start_y = j * n_sx;

                              for(V3DLONG i=0; i<n_sx; i++)
                              {
                                   V3DLONG idx = start_c + start_k + start_y + (i + myimg.offset_left); // real image starts from offset_left
                                   V3DLONG n_idx = n_start_c + n_start_k + n_start_y + i;
                                   pNewImg[n_idx] = myimg.pImFloat[idx];
                              }
                         }
                    }
               }
          }

		//now set image data
		if(!iter) //when iter==0, then create a new window for data returning
		{
			curwin = callback.newImageWindow();

               if(USE_PRIOR_CORRECTION == 1)
               {
                    p4DImage.setData((unsigned char*)pNewImg, n_sx, n_sy, n_sz, myimg.sc, V3D_FLOAT32);
               }
               else
               {
                    p4DImage.setData((unsigned char*)myimg.pImFloat, myimg.sx, myimg.sy, myimg.sz, myimg.sc, V3D_FLOAT32);
               }

			callback.setImage(curwin, &p4DImage);
			callback.setImageName(curwin, QString("Focus_Control_LSM"));
			callback.updateImageWindow(curwin);
		}
		else if (iter>0) //the iteration < 0 should be illegal
		{
			if(!callback.currentImageWindow())
			{
				v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
				break;
			}
			else
			{
				v3dhandleList win_list = callback.getImageWindowList();

				bool flag_win_exist = false;
				for (int i=0; i<win_list.size(); i++)
				{
					QString iname = callback.getImageName(win_list[i]);

					if(iname.compare("Focus_Control_LSM") == 0)
					{
						flag_win_exist = true;
					}
				}
				if(!flag_win_exist)
				{
					v3d_msg("No V3D window with a name Focus_Control_LSM is available for returning data ... Do nothing.", 0);
					break;
				}
				else
				{
					curwin = callback.currentImageWindow();

                         if(USE_PRIOR_CORRECTION == 1)
                         {
                              p4DImage.setData((unsigned char*)pNewImg, n_sx, n_sy, n_sz, myimg.sc, V3D_FLOAT32);
                         }
                         else
                         {
                              p4DImage.setData((unsigned char*)myimg.pImFloat, myimg.sx, myimg.sy, myimg.sz, myimg.sc, V3D_FLOAT32);
					}

                         callback.setImage(curwin, &p4DImage);
					callback.setImageName(curwin, QString("Focus_Control_LSM"));
					callback.updateImageWindow(curwin);
				}
			}
		}

		iter++; //advance the iteration indicator
		if (iter>10000)
		{
			v3d_msg("The iteration indicator now exceeds 10000, so reset it to 1.\n",0);
			iter = 1; //this is to avoid the potential overflow of the V3DLONG type data and really make the loop forever, if not get STOPPED manually
		}
		// clean memory allocation for the current slice
		myimg.cleanData();

		//now process events
		QCoreApplication::processEvents();
	}

	// close laser
	if(myimg.b_useSimImg == 0) myimg.shutter_close_daq_reset();

	myimg.cleanData();
	v3d_msg("Finish laser_scanning_imaging_focuscontrol().", 0);

}


void SSDriver::stimulation_laser_alignment(V3DPluginCallback &callback, QWidget *parent)
{
	LaserScanImaging myimg(callback);

	if (!myimg.setup_parameters())
		return;

	myimg.sz = 1; // only scan one image
	myimg.ch = 0;
	myimg.sc = 1;
	myimg.imaging_op = 3; // define imaging operation

	// stop dialog
	MicroscopeFocusControls dialog;
	dialog.show();
	dialog.setText( QString("Stimulation Laser Alignment...") );

	// data for return
	Image4DSimple p4DImage;
	v3dhandle curwin;
	V3DLONG iter=0;

	float *pCombImg=0;
	pCombImg = new float[myimg.sx * myimg.sy * myimg.sz * 2]; //2 channels

	while(1)
	{
		QCoreApplication::processEvents();

		bool flag_stop = dialog.flag_stop;
		if(flag_stop)
			break;

		// open laser
		DAQ_Shutter_Stimulation(false);
		myimg.shutter_open();
		myimg.acquire_single_direction(myimg.imaging_op); // single direction
		myimg.shutter_close_daq_reset();

		// combine to final img
		for(V3DLONG k=0; k<myimg.sz; k++)
		{
			V3DLONG offset_k= k*myimg.sy*myimg.sx;
			for(V3DLONG j=0; j<myimg.sy; j++)
			{
				V3DLONG offset_j= j*myimg.sx;
				for(V3DLONG i=0; i<myimg.sx; i++)
				{
					V3DLONG ind = offset_k + offset_j + i;
					pCombImg[ind] = myimg.pImFloat[ind];
				}
			}
		}

		DAQ_Shutter_Stimulation(true);
		DAQ_Shutter_Detection(true);
		myimg.acquire_single_direction(myimg.imaging_op); // single direction
		DAQ_Shutter_Stimulation(false);
		myimg.shutter_close_daq_reset();

		// combine to final img
		V3DLONG offset=myimg.sx*myimg.sy*myimg.sz;
		for(V3DLONG k=0; k<myimg.sz; k++)
		{
			V3DLONG offset_k= k*myimg.sy*myimg.sx;
			for(V3DLONG j=0; j<myimg.sy; j++)
			{
				V3DLONG offset_j= j*myimg.sx;
				for(V3DLONG i=0; i<myimg.sx; i++)
				{
					V3DLONG ind = offset_k + offset_j + i;
					pCombImg[ind+ offset] = myimg.pImFloat[ind];
				}
			}
		}

		//now set image data
		if(iter==0) //when iter==0, then create a new window for data returning
		{
			curwin = callback.newImageWindow();
			p4DImage.setData((unsigned char*)pCombImg, myimg.sx, myimg.sy, myimg.sz, 2, V3D_FLOAT32);
			callback.setImage(curwin, &p4DImage);
			callback.setImageName(curwin, QString("Stimulation_Laser_Alignment"));
			callback.updateImageWindow(curwin);

		}
		else if (iter>0) //the iteration < 0 should be illegal
		{
			if(!callback.currentImageWindow())
			{
				v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
				break;
			}
			else
			{
				v3dhandleList win_list = callback.getImageWindowList();

				bool flag_win_exist = false;
				for (int i=0; i<win_list.size(); i++)
				{
					QString iname = callback.getImageName(win_list[i]);

					if(iname.compare("Stimulation_Laser_Alignment") == 0)
					{
						flag_win_exist = true;
					}
				}
				if(!flag_win_exist)
				{
					v3d_msg("No V3D window with a name Stimulation_Laser_Alignment is available for returning data ... Do nothing.", 0);
					break;
				}
				else
				{
					curwin = callback.currentImageWindow();
					//p4DImage.setData((unsigned char*)pCombImg, myimg.sx, myimg.sy, myimg.sz, 2, V3D_FLOAT32);
					callback.setImage(curwin, &p4DImage);
					callback.setImageName(curwin, QString("Stimulation_Laser_Alignment"));
					callback.updateImageWindow(curwin);
				}
			}
		}

		iter++; //advance the iteration indicator
		if (iter>10000)
		{
			v3d_msg("The iteration indicator now exceeds 10000, so reset it to 1.\n",0);
			iter = 1; //this is to avoid the potential overflow of the V3DLONG type data and really make the loop forever, if not get STOPPED manually
		}

		// clean memory allocation for the current slice
		myimg.cleanData();

		//now process events
		QCoreApplication::processEvents();
	}

	// close laser
	if(myimg.b_useSimImg == 0) myimg.shutter_close_daq_reset();

	myimg.cleanData();
	v3d_msg("Finish stimulation_laser_alignment().", 0);

}


// bright field imaging for focus control
void SSDriver::bright_field_focuscontrols(V3DPluginCallback &callback, QWidget *parent)
{
	//get setting info
	Parameters_LSM mypara;
	if (!mypara.load_ParameterFile(ssConfigFolder + "mi_configuration.txt"))
	{
		v3d_msg("Fail to load the configuration file for bright-field imaging focus control. Do nothing");
		return;
	}

	V3DLONG start_x, start_y, end_x, end_y;
	int dev_x, dev_y, dev_z, ao_x, ao_y, ao_z;
	double min_fovx, max_fovx, min_fovy, max_fovy;
	double fov_step_x, fov_step_y;
	V3DLONG sx, sy;

	int b_useSimImg;
	double aa, dd, ee, ff; // transform factors for LSM->BF

	aa = mypara.LSM_TO_BF_A; dd = mypara.LSM_TO_BF_D; ee = mypara.LSM_TO_BF_E; ff = mypara.LSM_TO_BF_F;

	min_fovx = mypara.FOVX_MIN;	max_fovx = mypara.FOVX_MAX;	sx = mypara.SX;	fov_step_x = mypara.FOVX_STEP;
	min_fovy = mypara.FOVY_MIN;	max_fovy = mypara.FOVY_MAX;	sy = mypara.SY;	fov_step_y = mypara.FOVY_STEP;

	dev_x = mypara.DEVICE_X; dev_y = mypara.DEVICE_Y; dev_z = mypara.DEVICE_Z;
	ao_x = mypara.AO_X; ao_y = mypara.AO_Y; ao_z = mypara.AO_Z;
	b_useSimImg = mypara.USE_SIMIMG;

	// This part needs more work.
	// get start end x y from LSM configuration and transformation
	double n_start_x, n_start_y, n_end_x, n_end_y;
	n_start_x = min_fovx; n_start_y = min_fovy;
	n_end_x = max_fovx;
	n_end_y = max_fovy;

	start_x = aa * n_start_x + ee;  end_x = aa * n_end_x + ee;
	start_y = dd * n_start_y + ff;  end_y = dd * n_end_y + ff;

	//start_x = mypara.STARTX;  end_x = mypara.ENDX;
	//start_y = mypara.STARTY;  end_y = mypara.ENDY;

	if(!b_useSimImg)
	{
		//reset
		DAQ_Reset(dev_x, dev_y, dev_z, ao_x, ao_y, ao_z);
		// close for FL detection signals
		DAQ_Shutter_Detection(false);
		// open LED
		DAQ_Shutter_BFLED(true);
	}

	// showing BF image
	V3DLONG iter = 0;
	Image4DSimple p4DImage;
	v3dhandle curwin;

	// stop dialog
	MicroscopeFocusControls dialog;
	dialog.show();
	dialog.setText( QString("Focus Control...") );

	// BF
	FlyCapture2::PGRImageCap pPGRImageCap;
	unsigned char *pImage = NULL;
	V3DLONG dimx,dimy,pagesz;

	dimx =  pPGRImageCap.dimx;
	dimy =  pPGRImageCap.dimy;
	pagesz = dimx*dimy;

	try
	{
		pImage = new unsigned char [3*pagesz];
	}
	catch(...)
	{
		v3d_msg("Fail to allocate memory in bright_field_focuscontrols().");
		return;
	}

	while(1)
	{
		QCoreApplication::processEvents();

		bool flag_stop = dialog.flag_stop;
		if(flag_stop)
			break;

		v3d_msg(QString("... ... iter [%1]").arg(iter), 0);

		// -------------------------------------------------------------------------
		// record multiple images here and average these images to display. ZJL
		// ----------beginning of averaging images----------------------------------
		int numImg4Avg = 1; // number of images for averaging
		// used for averaging images
		double* pDImage;
		try
		{
			 pDImage = new double [3*pagesz];
		}
		catch(...)
		{
			v3d_msg("Fail to allocate memory for averaging images in bright_field_focuscontrols().");
			return;
		}

		if(!b_useSimImg)
			pPGRImageCap.imgcap();     // record image from camera
		else
			pPGRImageCap.imgcap_sim(); // use simulation image

		for(V3DLONG i=0; i<pagesz; i++)
		{
			unsigned char val =  pPGRImageCap.pData[i];
			pDImage[i] = pDImage[i + pagesz] = pDImage[i + 2*pagesz] = (double) val;
		}

		for(int j=0;j<numImg4Avg-1;j++)
		{
			pPGRImageCap.imgcap();
			for(V3DLONG i=0; i<pagesz; i++)
			{
				// pImage should be double here for sum
				unsigned char val =  pPGRImageCap.pData[i];
				pDImage[i] += (double) val;
				pDImage[i + pagesz] += (double) val;
				pDImage[i + 2*pagesz] += (double) val;
			}
		}

		for(V3DLONG i=0; i<pagesz; i++)
		{
			pImage[i] = (unsigned char)pDImage[i]/numImg4Avg;
			pImage[i + pagesz] = (unsigned char)pDImage[i + pagesz]/numImg4Avg;
			pImage[i + 2*pagesz] = (unsigned char)pDImage[i + 2*pagesz]/numImg4Avg;
		}
		// release memory for pDImage
		if(pDImage) {delete []pDImage; pDImage=0;}
		// ----------- end of averaging images -------------------------------------

		// flip BF in Y direction if bFlip_BF is true
		flip_BF_Y_Axis(dimx,dimy, pImage);

		// drawing ROI boundary box
		for(V3DLONG j=0; j<dimy; j++)
		{
			V3DLONG offset_j_o = j*dimx;
			for(V3DLONG i=0; i<dimx; i++)
			{
				V3DLONG idx_o = offset_j_o + i;

				if( (( i == start_x || i == end_x) && j>=start_y && j<=end_y ) ||
					(( j == start_y || j == end_y) && i>=start_x && i<=end_x ) )
				{
					pImage[idx_o] = 0;
					pImage[idx_o + pagesz] = 0;
					pImage[idx_o + 2*pagesz] = 255;
				}
			}
		}

		if(!iter) //when iter==0, then create a new window for data returning
		{
			curwin = callback.newImageWindow();
			p4DImage.setData((unsigned char *)pImage, dimx, dimy, 1, 3, V3D_UINT8);

			callback.setImage(curwin, &p4DImage);
			callback.setImageName(curwin, QString("Focus_Control_BF"));
			callback.updateImageWindow(curwin);
		}
		else if (iter>0) //the iteration < 0 should be illegal
		{
			if(!callback.currentImageWindow())
			{
				v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
				if(pImage) {delete []pImage; pImage=0;}
				break;
			}
			else
			{
				v3dhandleList win_list = callback.getImageWindowList();

				bool flag_win_exist = false;
				for (int i=0; i<win_list.size(); i++)
				{
					QString iname = callback.getImageName(win_list[i]);

					if(iname.compare("Focus_Control_BF") == 0)
					{
						flag_win_exist = true;
					}
				}
				if(!flag_win_exist)
				{
					v3d_msg("No V3D window with a name Focus_Control_BF is available for returning data ... Do nothing.", 0);
					if(pImage) {delete []pImage; pImage=0;}
					break;
				}
				else
				{
					curwin = callback.currentImageWindow();
					callback.updateImageWindow(curwin);
				}
			}
		}

		iter++; //advance the iteration indicator
		if (iter>10000)
		{
			v3d_msg("The iteration indicator now exceeds 10000, so reset it to 1.\n",0);
			iter = 1; //this is to avoid the potential overflow of the V3DLONg type data and really make the loop forever, if not get STOPPED manually
		}

		QCoreApplication::processEvents();
	}
	pPGRImageCap.closeCam();

	// close LED
	//wait(1000);
	if(!b_useSimImg)
		DAQ_Shutter_BFLED(false);

	return;
}

// bright field imaging
void SSDriver::bright_field_imaging(V3DPluginCallback &callback, QWidget *parent)
{
	//get setting info
	Parameters_LSM mypara;
	if (!mypara.load_ParameterFile(ssConfigFolder + "mi_configuration.txt"))
	{
		v3d_msg("Fail to load the configuration file for bright-field imaging focus control. Do nothing");
		return;
	}

	V3DLONG start_x, start_y, end_x, end_y;
	int dev_x, dev_y, dev_z, ao_x, ao_y, ao_z;
	double min_fovx, max_fovx, min_fovy, max_fovy;
	double fov_step_x, fov_step_y;
	V3DLONG sx, sy;

	int b_useSimImg;
	double aa, dd, ee, ff; // transform factors for LSM->BF

	aa = mypara.LSM_TO_BF_A; dd = mypara.LSM_TO_BF_D; ee = mypara.LSM_TO_BF_E; ff = mypara.LSM_TO_BF_F;

	min_fovx = mypara.FOVX_MIN;	max_fovx = mypara.FOVX_MAX;	sx = mypara.SX;	fov_step_x = mypara.FOVX_STEP;
	min_fovy = mypara.FOVY_MIN;	max_fovy = mypara.FOVY_MAX;	sy = mypara.SY;	fov_step_y = mypara.FOVY_STEP;

	dev_x = mypara.DEVICE_X; dev_y = mypara.DEVICE_Y; dev_z = mypara.DEVICE_Z;
	ao_x = mypara.AO_X; ao_y = mypara.AO_Y; ao_z = mypara.AO_Z;
	b_useSimImg = mypara.USE_SIMIMG;

	// get start end x y from LSM configuration and transformation
	double n_start_x, n_start_y, n_end_x, n_end_y;
	n_start_x = min_fovx; n_start_y = min_fovy;
	n_end_x = max_fovx;   n_end_y = max_fovy;

	start_x = aa * n_start_x + ee;  end_x = aa * n_end_x + ee;
	start_y = dd * n_start_y + ff;  end_y = dd * n_end_y + ff;

	//start_x = mypara.STARTX;  end_x = mypara.ENDX;
	//start_y = mypara.STARTY;  end_y = mypara.ENDY;

	printf("start_x = %ld, start_y = %ld, end_x = %ld, end_y = %ld \n", start_x, start_y, end_x, end_y);

	if(!b_useSimImg)
	{
		//reset
		DAQ_Reset(dev_x, dev_y, dev_z, ao_x, ao_y, ao_z);
		// close for FL detection signals
		DAQ_Shutter_Detection(0);
		// open LED
		DAQ_Shutter_BFLED(true);
		//wait_milliseconds(1000); // commented by ZJL 111028
	}

	// BF
	unsigned char *pImage = NULL;
	V3DLONG dimx,dimy,pagesz;

	FlyCapture2::PGRImageCap pPGRImageCap;
	qDebug()<<"pointer ..."<<pPGRImageCap.pData;
	dimx =  pPGRImageCap.dimx;
	dimy =  pPGRImageCap.dimy;
	pagesz = dimx*dimy;

	if(!b_useSimImg)
		pPGRImageCap.imgcap();     // record image from camera
	else
		pPGRImageCap.imgcap_sim(); // use simulation image

	try
	{
		pImage = new unsigned char [3*pagesz];
	}
	catch(...)
	{
		v3d_msg("Unable to allocate memory in bright_field_imaging(). Do nothing.");
		return;
	}

	for(V3DLONG i=0; i<pagesz; i++)
	{
		unsigned char val =  pPGRImageCap.pData[i];
		pImage[i] = pImage[i + pagesz] = pImage[i + 2*pagesz] = val;
	}

	// if bFlip_BF==tue flip image in Y direction
	flip_BF_Y_Axis(dimx,dimy, pImage);

	// drawing ROI boundary box
	for(V3DLONG j=0; j<dimy; j++)
	{
		V3DLONG offset_j_o = j*dimx;
		for(V3DLONG i=0; i<dimx; i++)
		{
			V3DLONG idx_o = offset_j_o + i;

			if( (( i == start_x || i == end_x) && j>=start_y && j<=end_y ) ||
				(( j == start_y || j == end_y) && i>=start_x && i<=end_x ) )
			{
				pImage[idx_o] = 0;
				pImage[idx_o + pagesz] = 0;
				pImage[idx_o + 2*pagesz] = 255;
			}
		}
	}

	// visualization
	QString filename_saved = generate_img_name("BF", ".raw");

	Image4DSimple p4DImage;
	p4DImage.setData((unsigned char *)pImage, dimx, dimy, 1, 3, V3D_UINT8);

	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &p4DImage);
	callback.setImageName(newwin, filename_saved); //
	callback.updateImageWindow(newwin);

	pPGRImageCap.closeCam();

	// close LED
	//wait(1000);
	if(!b_useSimImg)
		DAQ_Shutter_BFLED(false);

	// saving BF image
	V3DLONG save_sz[4];
	save_sz[0] = dimx; save_sz[1] = dimy; save_sz[2] = 1; save_sz[3] = 3;
	saveImage(qPrintable(filename_saved), (unsigned char *)pImage, save_sz, 1);

	return;
}

void SSDriver::flip_BF_Y_Axis(V3DLONG dimx, V3DLONG dimy, unsigned char *pImage)
{
	// flip image in Y direction
	bFlip_BF = true;
	if(bFlip_BF == true)
	{
		V3DLONG pagesz=dimx*dimy;
		V3DLONG hsz1=floor((double)(dimy-1)/2.0); if(hsz1*2<dimy-1) hsz1+=1;
		for (V3DLONG j=0;j<hsz1;j++)
		{
			for (V3DLONG i=0;i<dimx;i++)
			{
				V3DLONG idx1 = (dimy-j-1)*dimx+i;//[dimy-j-1][i]
				V3DLONG idx2 = j*dimx+i;         //[j][i]
				unsigned char tmpv = pImage[idx1];
				pImage[idx1] = pImage[idx1+pagesz] = pImage[idx1+2*pagesz] = pImage[idx2];
				pImage[idx2] = pImage[idx2+pagesz] = pImage[idx2+2*pagesz] = tmpv;
			}
		}
	}
}


// calibration the coordinates of BF and LSM images
void SSDriver::calibrate_BF_LSM(V3DPluginCallback &callback, QWidget *parent)
{
	// input two images BF and FL with at least two markers
	// math: BF (xb, yb) LSM (xf, yf)
	//
	// csx = (xb1-xb2)/(xf1-xf2);
	// ctx = xb1 - xf1*csx;
	//
	// start point (ctx, cty)
	// end point (csx * sx + ctx, csy * sy + cty)

	v3dhandleList win_list = callback.getImageWindowList();
	if(win_list.size()<2)
	{
		v3d_msg("At least TWO images (one LSM and one BF images will be needed for calibration.");
		return;
	}

	QStringList items;
	for(int i=0; i<win_list.size(); i++)
		items << callback.getImageName(win_list[i]);

	// create a dialog for the calibration
	QDialog d(parent);
	QComboBox *combo_bf = new QComboBox();  combo_bf->addItems(items);
	QComboBox *combo_lsm = new QComboBox(); combo_lsm->addItems(items);

	QPushButton *ok = new QPushButton("Ok");
	QPushButton *cancel = new QPushButton("Cancel");

	QFormLayout *formLayout = new QFormLayout();
	formLayout->addRow(QObject::tr("LSM Image with at least 2 markers: "), combo_lsm);
	formLayout->addRow(QObject::tr("BF Image with at least 2 markers: "), combo_bf);
	formLayout->addRow(ok, cancel);
	d.setLayout(formLayout);
	d.setWindowTitle(QObject::tr("FOV (BF/LSM) Calibration"));

	d.connect(ok, SIGNAL(clicked()), &d, SLOT(accept()));
	d.connect(cancel, SIGNAL(clicked()), &d, SLOT(reject()));

	if(d.exec() != QDialog::Accepted)
		return;

	int i_lsm = combo_lsm->currentIndex();
	int i_bf = combo_bf->currentIndex();

	Image4DSimple *pImLSM = callback.getImage(win_list[i_lsm]);
	Image4DSimple *pImBF = callback.getImage(win_list[i_bf]);

	LandmarkList list_landmark_lsm =callback.getLandmark(win_list[i_lsm]);
	LandmarkList list_landmark_bf =callback.getLandmark(win_list[i_bf]);

	if(list_landmark_lsm.size()<2 || list_landmark_bf.size()<2)
	{
		v3d_msg("To calibrate, you need at least 2 markers in the image.");
		return;
	}

	V3DLONG sx = pImLSM->getXDim();
	V3DLONG sy = pImLSM->getYDim();
	V3DLONG sz = pImLSM->getZDim();

	// calibration
	double xb1, xb2, yb1, yb2, xf1, xf2, yf1, yf2;

	xb1 = list_landmark_bf[0].x;
	yb1 = list_landmark_bf[0].y;
	xb2 = list_landmark_bf[1].x;
	yb2 = list_landmark_bf[1].y;

	xf1 = list_landmark_lsm[0].x;
	yf1 = list_landmark_lsm[0].y;
	xf2 = list_landmark_lsm[1].x;
	yf2 = list_landmark_lsm[1].y;

	if(xf1==xf2 || yf1==yf2 || xb1==xb2 || yb1==yb2)
	{
		v3d_msg("You need to reselect 2 markers in the image.");
		return;
	}

	// get absolute physical position of (xf,yf) in LSM
	double o_origin_x, o_origin_y;
	double o_rez_x, o_rez_y;

	// Acquire origin from the log file of the current image window
	QString logname_lsm = callback.getImageName(win_list[i_lsm]);

	// If the image is not created in the current session, it cannot get its origin from
	// pImLSM->getOriginX()... So the image log file is used for origin information.
	// decide whether this is selected from Focus_Control_LSM window
	if(logname_lsm.compare("Focus_Control_LSM")!=0)
	{
		Parameters_LSM pl;

		logname_lsm.chop(4);
		logname_lsm.append(".txt");
		pl.load_ParameterFile(logname_lsm);

		o_origin_x = pl.FOVX_MIN;
		o_origin_y = pl.FOVY_MIN;
		o_rez_x = pl.FOVX_STEP;
		o_rez_y = pl.FOVY_STEP;
	}else // the current image is a live image from focus control
	{
		o_origin_x = pImLSM->getOriginX();
		o_origin_y = pImLSM->getOriginY();
		o_rez_x = pImLSM->getRezX();
		o_rez_y = pImLSM->getRezY();
	}

	printf("(xf1,yf1): (%lf, %lf),(xf2,yf2): (%lf, %lf)\n", xf1,yf1,xf2,yf2);
	qDebug()<<"origin_x, origin_y: "<<o_origin_x<<" "<<o_origin_y;
	qDebug()<<"o_rez_x, o_rez_y: "<<o_rez_x<<" "<<o_rez_y;

	// absolute physical position: pixel->physical pos
	double n_xf1 = o_origin_x + (o_rez_x * (double)xf1);
	double n_yf1 = o_origin_y + (o_rez_y * (double)yf1);
	double n_xf2 = o_origin_x + (o_rez_x * (double)xf2);
	double n_yf2 = o_origin_y + (o_rez_y * (double)yf2);
	printf("(nxf1,nyf1): (%lf, %lf),(nxf2,nyf2): (%lf, %lf)\n", n_xf1,n_yf1,n_xf2,n_yf2);

	// get transformation coefficients from BF->LSM
	// xf= a*xb + e, yf= d*yb + f
	double a0, d0, e0, f0;
	a0 = (n_xf1 - n_xf2)/(xb1-xb2);
	d0 = (n_yf1 - n_yf2)/(yb1-yb2);
	e0 = n_xf1 - a0*xb1;
	f0 = n_yf1 - d0*yb1;

	// get transformation coefficients from LSM->BF
	// LSM pos should be physical pos
	// xb= aa * xf + ee, yb= dd * yf + ff
	double aa, dd, ee, ff;
	aa = (xb1-xb2)/(n_xf1 - n_xf2);
	dd = (yb1-yb2)/(n_yf1 - n_yf2);
	ee = xb1 - aa*n_xf1;
	ff = yb1 - dd*n_yf1;

	// save start point and end point
	QString m_FileName = ssConfigFolder + "mi_configuration.txt";
	if(!QFile(m_FileName).exists())
	{
		m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Find Configuration file"),
												  "/Documents",
												  QObject::tr("Configuration (*.txt)"));
		if(m_FileName.isEmpty())
			return;
	}

	Parameters_LSM mypara;
	if (!mypara.load_ParameterFile(m_FileName))
	{
		v3d_msg("Fail to load the configuration file for bright-field imaging focus control. Do nothing");
		return;
	}

	mypara.BF_TO_LSM_A = a0;
	mypara.BF_TO_LSM_D = d0;
	mypara.BF_TO_LSM_E = e0;
	mypara.BF_TO_LSM_F = f0;

	mypara.LSM_TO_BF_A = aa;
	mypara.LSM_TO_BF_D = dd;
	mypara.LSM_TO_BF_E = ee;
	mypara.LSM_TO_BF_F = ff;

	// yuy's method
	double csx, csy, ctx, cty;
	csx = (xb1-xb2)/(xf1-xf2);
	ctx = xb1 - xf1*csx;

	csy = (yb1-yb2)/(yf1-yf2);
	cty = yb1 - yf1*csy;

	mypara.STARTX = (V3DLONG)ctx;
	mypara.STARTY = (V3DLONG)cty;
	mypara.ENDX = (V3DLONG)(csx * sx + ctx);
	mypara.ENDY = (V3DLONG)(csy * sy + cty);

	// save paras
	mypara.save_ParameterFile(m_FileName);

	// LSM->BF
	double box_sx=aa*o_origin_x+ee;
	double box_sy=dd*o_origin_y+ff;
	double box_ex=aa*(o_origin_x+o_rez_x*sx)+ee;
	double box_ey=dd*(o_origin_y+o_rez_y*sy)+ff;

	qDebug()<<"done for FOV calibrating zjl method ..."<<box_sx <<" "<<box_sy<<" "<<box_ex<<" "<<box_ey;
	qDebug()<<"done for FOV calibrating yuy method ..."<<ctx <<" "<<cty<<" "<<csx * sx + ctx<<" "<<csy * sy + cty;
}


void SSDriver::reset_BF_field(V3DPluginCallback &callback, QWidget *parent)
{
	// reset start point and end point
	QString m_FileName = ssConfigFolder + "mi_configuration.txt";
	if(!QFile(m_FileName).exists())
	{
		m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Find Configuration file"),
												  "/Documents",
												  QObject::tr("Configuration (*.txt)"));
		if(m_FileName.isEmpty())
			return;
	}

	Parameters_LSM mypara;
	if (!mypara.load_ParameterFile(m_FileName))
	{
		v3d_msg("Fail to load the configuration file for bright-field imaging focus control. Do nothing");
		return;
	}

	// these values are from previous settings before calibration
	// maybe need to be changed
	mypara.STARTX = -204;
	mypara.STARTY = 57;
	mypara.ENDX = 418;
	mypara.ENDY = 185;

	mypara.save_ParameterFile(m_FileName);
}

// set parameters for full image scanning
void SSDriver::setpara_markerablation(V3DPluginCallback &callback, QWidget *parent)
{
	// preprocessing
	StimulatingParas para_abl;

	// retrieve log file
	QString m_FileName = ssConfigFolder + "markerablation_configuration.log";
	if(!QFile(m_FileName).exists())
	{
		m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Find Configuration file"),
												  "/Documents",
												  QObject::tr("Image Configuration (*.log)"));
		if(m_FileName.isEmpty())
			return;
	}
	if (!para_abl.load_ParaFile(m_FileName))
	{
		v3d_msg(QString("Fail to load the the Ablation Configuration file [%1]").arg(m_FileName), 0);
	}

	// adjusting by users
	StimulatingDialog dialog(callback, parent, &para_abl);
	if (dialog.exec()!=QDialog::Accepted)
		return;

	dialog.copyDataOut(&para_abl);

	//save config info
	if (!para_abl.save_ParaFile(m_FileName)) //always save to the same file, do NOT use the manually selected file
	{
		v3d_msg(QString("Fail to save to the Ablation Configuration file [%1]").arg(m_FileName), 0);
		return;
	}
}


void SSDriver::calibrate_AO_AI(V3DPluginCallback &callback, bool flag_ao2ai) // AO and AI calibration
{
	// get scanning rate, dims
	LaserScanImaging *flimg=new LaserScanImaging(callback);
	flimg->imaging_op = 5; // define imaging operation
	flimg->b_scanner_correction = true;

	bool succ = flimg->acquire_single_direction(flimg->imaging_op);

	if(succ==false) 	return;


	dimx = flimg->sx;
	dimy = flimg->sy;

	V3DLONG pagesz = flimg->sx * flimg->sy;

	if(flag_ao2ai)
	{
		// imaging ao 2 ai
		try
		{
			pCalibrateAO2AI = new float [4*pagesz];
		}
		catch (...)
		{
			printf("Fail to allocate memory for LUT pointer.\n");
			return;
		}

		// save the first line on the first slice of pAIx and pAOcx for calibration
		// the saved file is used as the standard (pAIx,pAOcx) for calibration
		QString filename_AIx=ssConfigFolder + "calibratingAO2AI_SR_AIx_AOx.txt";
		printImg2Txt(filename_AIx, flimg->sx, 1, flimg->pAIx, flimg->pAOcx);


		//// save pAIx to a text file for analysis
		//QString filename_txt_saved = ssConfigFolder + "calibratingAO2AI_SR" + QString::number(flimg.scanning_rate);
		//QString filename_AIx=filename_txt_saved + "_AIx.txt";
		//// print pAIx to a text file
		//printImg2Txt(filename_AIx, flimg.sx, flimg.sy, flimg.pAIx);

		//V3DLONG idx;
		//for(V3DLONG i=0; i<pagesz; i++)
		//{
		//	pCalibrateAO2AI[i] = flimg.pAOcx[i];   //AO x

		//	idx = i + pagesz;
		//	pCalibrateAO2AI[idx] = flimg.pAOcy[i]; //AO y

		//	idx += pagesz;
		//	pCalibrateAO2AI[idx] = flimg.pAIx[i];  // AI x
		//
		//	idx += pagesz;
		//	pCalibrateAO2AI[idx] = flimg.pAIy[i];  // AI y
		//}

		//QString filename_img_saved = ssConfigFolder + "calibratingAO2AI.raw";
		//V3DLONG save_sz[4];
		//save_sz[0] = flimg.sx; save_sz[1] = flimg.sy; save_sz[2] = 4; save_sz[3] = 1;
		//saveImage(qPrintable(filename_img_saved), (unsigned char *)pCalibrateAO2AI, save_sz, 4);

		//// visualization
		//Image4DSimple p4DImage;
		//v3dhandle curwin;
		//QString win_name = filename_img_saved;
		//
		//v3dhandleList win_list = callback.getImageWindowList();
		//bool flag_win_exist = false;
		//for (int i=0; i<win_list.size(); i++)
		//{
		//	QString iname = callback.getImageName(win_list.at(i));

		//	if(iname.compare(filename_img_saved) == 0)
		//	{
		//		flag_win_exist = true;
		//		curwin=win_list.at(i);
		//	}
		//}

		//if(!flag_win_exist)
		//{
		//	// Create a new window
		//	p4DImage.setData((unsigned char *)pCalibrateAO2AI, save_sz[0], save_sz[1], save_sz[2], save_sz[3], V3D_FLOAT32); //default data type is FLOAT
		//	//also assign the resolution and origin information for the image
		//	p4DImage.setRezX(flimg.fov_step_x); p4DImage.setRezY(flimg.fov_step_y); p4DImage.setRezZ(flimg.fov_step_z);
		//	p4DImage.setOriginX(flimg.min_fovx); p4DImage.setOriginY(flimg.min_fovy); p4DImage.setOriginZ(flimg.min_fovz);

		//	curwin = callback.newImageWindow();
		//	callback.setImage(curwin, &p4DImage);
		//	callback.setImageName(curwin, win_name);
		//	callback.updateImageWindow(curwin);
		//}
		//else
		//{
		//	p4DImage.setData((unsigned char *)pCalibrateAO2AI, save_sz[0], save_sz[1], save_sz[2], save_sz[3], V3D_FLOAT32);
		//	//also assign the resolution and origin information for the image
		//	p4DImage.setRezX(flimg.fov_step_x); p4DImage.setRezY(flimg.fov_step_y); p4DImage.setRezZ(flimg.fov_step_z);
		//	p4DImage.setOriginX(flimg.min_fovx); p4DImage.setOriginY(flimg.min_fovy); p4DImage.setOriginZ(flimg.min_fovz);

		//	callback.setImage(curwin, &p4DImage);
		//	callback.setImageName(curwin, win_name);
		//	callback.updateImageWindow(curwin);
		//}

		//// saving log file
		//QString qstr_logname = filename_img_saved;
		//qstr_logname.chop(4);
		//qstr_logname.append(".txt");

		//QString m_FileName = ssConfigFolder + "mi_configuration.txt";

		//// using Parameters_ROI_LSM for saving log
		//Parameters_ROI_LSM lsm_conf;
		//lsm_conf.load_ParameterFile(m_FileName);

		//lsm_conf.REZ_X = flimg.fov_step_x;
		//lsm_conf.REZ_Y = flimg.fov_step_y;
		//lsm_conf.REZ_Z = flimg.fov_step_z;

		//lsm_conf.ORIGIN_X = flimg.min_fovx;
		//lsm_conf.ORIGIN_Y = flimg.min_fovy;
		//lsm_conf.ORIGIN_Z = flimg.min_fovz;

		//lsm_conf.save_ParameterFile(qstr_logname);

		v3d_msg("Calibration information and log saved ....");
	}
	else
	{
		v3d_msg("This is not realized yet!");
	}

	flimg->cleanData();
	delete flimg;

	v3d_msg("Success! ...", 0);
}


void SSDriver::printImg2Txt(QString filename, V3DLONG sx, V3DLONG sy, float *pAIx, float *pAOx)
{
	FILE *fp=fopen(filename.toStdString().c_str(), "wt");
	// first is pAIx. only use one slice
	for(V3DLONG j=0; j<sy; j++)
	{
		for(V3DLONG i=0; i<sx; i++)
		{
			V3DLONG idx=j*sx + i;
			fprintf(fp, "%.5f  ", pAIx[idx]);
		}
		fprintf(fp, "\n");
	}

	// second is pAOcx. only use one slice
	for(V3DLONG j=0; j<sy; j++)
	{
		for(V3DLONG i=0; i<sx; i++)
		{
			V3DLONG idx=j*sx + i;
			fprintf(fp, "%.5f  ", pAOx[idx]);
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
}



// imaging with interactive selecting ROI
void SSDriver::mi_imaging_withROI_i(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle curwin;
	curwin=callback.currentImageWindow();
	if(!curwin)
	{
		v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
		return;
	}

	//mare sure there are ROI bounding boxes
	ROIList pRoiList=callback.getROI(curwin);
	if((pRoiList.at(0).size()<1)&&(pRoiList.at(1).size()<1)&&(pRoiList.at(2).size()<1))
	{
		v3d_msg("No ROI specified ... Do nothing.");
		return;
	}

	LaserScanImaging *myimg = new LaserScanImaging(callback);

	myimg->imaging_op = 1; // define imaging operation
	bool succ = myimg->acquire_single_direction(myimg->imaging_op);

	if(!succ)
	{
		if(myimg->pImFloat) {delete []myimg->pImFloat; myimg->pImFloat=NULL;}
		myimg->cleanData();
		return;
	}

	// results
	QString filename_img_saved;// 

     // save image
     if(USE_PRIOR_CORRECTION == 1)
     {
          saveCorrectedImage(callback, filename_img_saved, myimg);
     }
     else
     {
          passImagingData2V3D(callback, myimg->pImFloat, "LSM", false, myimg->sx, myimg->sy, myimg->sz, myimg->sc, myimg->fov_step_x,
						myimg->fov_step_y, myimg->fov_step_z, myimg->min_fovx, myimg->min_fovy, myimg->min_fovz,
						myimg->max_fovx, myimg->max_fovy, myimg->max_fovz,
						myimg->ratio_x, myimg->ratio_y, myimg->ratio_z,
						myimg->time_out, myimg->scanning_rate, myimg->ch,
						myimg->scanMethod, myimg->b_useSimImg, myimg->b_scan_z_reverse,
						filename_img_saved,false);
     }

	 // print LUT to file
	filename_img_saved.chop(4);
	QString filename_lut = filename_img_saved + "_" + QString::number(myimg->scanning_rate)+"Hz_sx_"+
		QString::number(myimg->sx)+"_lut.txt";
	FILE *fp=fopen((char*)qPrintable(filename_lut), "wt");
	for(V3DLONG i=0; i<myimg->sx; i++)
	{
		fprintf(fp, "%d to %d\n", myimg->real_to_exp_pos_LUT[i], i);
	}
	fclose(fp);

	myimg->cleanData();

	if(myimg) delete myimg;

	v3d_msg("Success! ...", 0);
}


// imaging with interactive selecting ROI with full X scanning
void SSDriver::mi_imaging_withROI_i_x(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle curwin;
	curwin=callback.currentImageWindow();
	if(!curwin)
	{
		v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
		return;
	}

	//mare sure there are ROI bounding boxes
	ROIList pRoiList=callback.getROI(curwin);
	if((pRoiList.at(0).size()<1)&&(pRoiList.at(1).size()<1)&&(pRoiList.at(2).size()<1))
	{
		v3d_msg("No ROI specified ... Do nothing.");
		return;
	}

	LaserScanImaging *myimg = new LaserScanImaging(callback);

	myimg->imaging_op = 7; // define imaging operation
	bool succ = myimg->acquire_single_direction(myimg->imaging_op);

	if(!succ)
	{
		if(myimg->pImFloat) {delete []myimg->pImFloat; myimg->pImFloat=NULL;}
		myimg->cleanData();
		return;
	}

	// save image
	QString filename_img_saved; // return back this name for other uses

	if(USE_PRIOR_CORRECTION == 1)
	{
		saveCorrectedImage_roi(callback, filename_img_saved, myimg);
	}
	else
	{
		passImagingData2V3D(callback, myimg->pImFloat, "LSM", false, myimg->sx, myimg->sy, myimg->sz, myimg->sc, myimg->fov_step_x,
					myimg->fov_step_y, myimg->fov_step_z, myimg->min_fovx, myimg->min_fovy, myimg->min_fovz,
					myimg->max_fovx, myimg->max_fovy, myimg->max_fovz,
					myimg->ratio_x, myimg->ratio_y, myimg->ratio_z,
					myimg->time_out, myimg->scanning_rate, myimg->ch,
					myimg->scanMethod, myimg->b_useSimImg, myimg->b_scan_z_reverse,
					filename_img_saved,false);
	}

	// print LUT to file
	filename_img_saved.chop(4);
	QString filename_lut = filename_img_saved + "_" + QString::number(myimg->scanning_rate)+"Hz_sx_"+
		QString::number(myimg->sx)+"_lut.txt";
	FILE *fp=fopen((char*)qPrintable(filename_lut), "wt");
	for(V3DLONG i=0; i<myimg->sx; i++)
	{
		fprintf(fp, "%d to %d\n", myimg->real_to_exp_pos_LUT[i], i);
	}
	fclose(fp);

	myimg->cleanData();
	if(myimg) delete myimg;

	v3d_msg("Success! ...", 0);
}


// imaging with adaptive ROI
void SSDriver::mi_imaging_withROI_adaptive(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle curwin;
	curwin=callback.currentImageWindow();
	if(!curwin)
	{
		v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
		return;
	}

	LaserScanImaging *myimg = new LaserScanImaging(callback);

	myimg->imaging_op = 6; // define imaging operation
	bool succ = myimg->acquire_single_direction(myimg->imaging_op);
	if(!succ)
	{
		if(myimg->pImFloat) {delete []myimg->pImFloat; myimg->pImFloat=NULL;}
		myimg->cleanData();
		return;
	}

	// results
	QString filename_img_saved;// for future use

     // save image
     if(USE_PRIOR_CORRECTION == 1)
     {
          saveCorrectedImage(callback, filename_img_saved, myimg);
     }
     else
     {
          passImagingData2V3D(callback, myimg->pImFloat, "LSM", false, myimg->sx, myimg->sy, myimg->sz, myimg->sc, myimg->fov_step_x,
						myimg->fov_step_y, myimg->fov_step_z, myimg->min_fovx, myimg->min_fovy, myimg->min_fovz,
						myimg->max_fovx, myimg->max_fovy, myimg->max_fovz,
						myimg->ratio_x, myimg->ratio_y, myimg->ratio_z,
						myimg->time_out, myimg->scanning_rate, myimg->ch,
						myimg->scanMethod, myimg->b_useSimImg, myimg->b_scan_z_reverse,
						filename_img_saved,false);
     }

	//// save pAIx to a text file for analysis
	//filename_img_saved.chop(4);
	//QString filename_AIx = filename_img_saved + "_" + QString::number(myimg->scanning_rate)+"Hz_sx_"+
	//	QString::number(myimg->sx)+"_x_xrange_"+QString::number(myimg->min_fovx)+"-"+QString::number(myimg->max_fovx)+"_um";

	//QString filename_AIx_txt=filename_AIx + "_AIx_AOx.txt";
	//// print one line of pAIx and pAOx to a text file
	//printImg2Txt(filename_AIx_txt, myimg->sx, 1, myimg->pAIx, myimg->pAOcx);

	// save pAIx to a image file
	//V3DLONG save_sz[4];
	//QString filename_AIx_img=filename_AIx + "_AIx.raw";
	//save_sz[0] = myimg->sx; save_sz[1] = myimg->sy; save_sz[2] = myimg->sz; save_sz[3] = 1;
	//saveImage(qPrintable(filename_AIx_img), (unsigned char *)myimg->pAIx, save_sz, 4); //float

	myimg->cleanData();

	if(myimg) delete myimg;

	v3d_msg("Success! ...", 0);
}

// imaging roi which chosen from 3d view
void SSDriver::tdviewroi_imaging(V3DPluginCallback &callback, QWidget *parent)
{
	LaserScanImaging *myimg = new LaserScanImaging(callback);

	myimg->imaging_op = 4; // define imaging operation
	bool succ = myimg->acquire_single_direction(myimg->imaging_op);
	if(!succ) return;

	//----------------------------------------------------
	// save image
	QString filename_img_saved; // return back this name for other uses

	if(USE_PRIOR_CORRECTION == 1)
	{
		saveCorrectedImage_roi(callback, filename_img_saved, myimg);
	}
	else
	{
		passImagingData2V3D(callback, myimg->pImFloat, "LSM", false, myimg->sx, myimg->sy, myimg->sz, myimg->sc, myimg->fov_step_x,
					myimg->fov_step_y, myimg->fov_step_z, myimg->min_fovx, myimg->min_fovy, myimg->min_fovz,
					myimg->max_fovx, myimg->max_fovy, myimg->max_fovz,
					myimg->ratio_x, myimg->ratio_y, myimg->ratio_z,
					myimg->time_out, myimg->scanning_rate, myimg->ch,
					myimg->scanMethod, myimg->b_useSimImg, myimg->b_scan_z_reverse,
					filename_img_saved,false);
	}
	//----------------------------------------------------

	// print LUT to file
	filename_img_saved.chop(4);
	QString filename_lut = filename_img_saved + "_" + QString::number(myimg->scanning_rate)+"Hz_sx_"+
		QString::number(myimg->sx)+"_lut.txt";
	FILE *fp=fopen((char*)qPrintable(filename_lut), "wt");
	for(V3DLONG i=0; i<myimg->sx; i++)
	{
		fprintf(fp, "%d to %d\n", myimg->real_to_exp_pos_LUT[i], i);
	}
	fclose(fp);

	//// save pAIx to a text file for analysis
	//filename_img_saved.chop(4);
	//QString filename_AIx = filename_img_saved + "_" + QString::number(myimg->scanning_rate)+"Hz_sx_"+
	//	QString::number(myimg->sx)+"_x_xrange_"+QString::number(myimg->min_fovx)+"-"+QString::number(myimg->max_fovx)+"_um";

	//QString filename_AIx_txt=filename_AIx + "_AIx_AOx.txt";
	//// print one line of pAIx and pAOx to a text file
	//printImg2Txt(filename_AIx_txt, myimg->sx, 1, myimg->pAIx, myimg->pAOcx);


	myimg->cleanData();
	if(myimg) delete myimg;

	v3d_msg("Success! ...", 0);

}

// imaging landmarkers which chosen from 3d view
void SSDriver::tdview_landmarker_imaging(V3DPluginCallback &callback, QWidget *parent)
{
	qDebug()<<".................... Now running landmarker imaging program ...................";

	LaserScanImaging *myimg = new LaserScanImaging(callback);

	myimg->imaging_op = 8; // define imaging operation
	bool succ = myimg->acquire_single_direction(myimg->imaging_op);
	if(!succ) return;

	//----------------------------------------------------
	// save image
	QString filename_img_saved; // return back this name for other uses

	if(USE_PRIOR_CORRECTION == 1)
	{
		saveCorrectedImage_roi(callback, filename_img_saved, myimg);
	}
	else
	{
		passImagingData2V3D(callback, myimg->pImFloat, "LSM", false, myimg->sx, myimg->sy, myimg->sz, myimg->sc, myimg->fov_step_x,
					myimg->fov_step_y, myimg->fov_step_z, myimg->min_fovx, myimg->min_fovy, myimg->min_fovz,
					myimg->max_fovx, myimg->max_fovy, myimg->max_fovz,
					myimg->ratio_x, myimg->ratio_y, myimg->ratio_z,
					myimg->time_out, myimg->scanning_rate, myimg->ch,
					myimg->scanMethod, myimg->b_useSimImg, myimg->b_scan_z_reverse,
					filename_img_saved,false);
	}
	//----------------------------------------------------

	myimg->cleanData();
	if(myimg) {delete myimg; myimg=0;}
	v3d_msg("Finish landmarker imaging from 3D viewer.", 0);
}


// imaging on landermarkers, 
void SSDriver::landmarker_imaging(V3DPluginCallback &callback, QWidget *parent)
{
	// get paras for ablation
	Image4DSimple *pImgIn;
	v3dhandle curwin;
	curwin=callback.currentImageWindow();
	if(!curwin)
	{
		v3d_msg("No V3D window is available for returning data ... Do nothing.");
		return;
	}

	pImgIn=callback.getImage(curwin);
	if(!pImgIn) return;

	LandmarkList list_landmarks=callback.getLandmark(curwin);
	if(list_landmarks.size()<1)
	{
		v3d_msg("Need at least 1 markers in subject image.");
		return;
	}

	// imaging
	LaserScanImaging *myimg = new LaserScanImaging(callback);
	myimg->do_image_landmarkers(curwin, pImgIn, list_landmarks);
	myimg->cleanData();

	if(myimg) {delete myimg; myimg=0;}

	v3d_msg("Finish imaging_landmarkers().", 0);
}

//  markers ablation from 3D view window
void SSDriver::tdview_landmarker_ablating(V3DPluginCallback &callback, QWidget *parent)
{
	qDebug()<<".................... Now running point ablating program ...................";

	LaserScanImaging *myimg = new LaserScanImaging(callback);
	myimg->imaging_op = 9; // ablation
	bool succ = myimg->acquire_single_direction(myimg->imaging_op, true); // true means from 3dviewer

	//myimg->cleanData(); // already cleaned in lsm

	if(myimg) {delete myimg; myimg=0;}
	v3d_msg("Finish landmarker ablation from 3D viewer.", 0);
}


// ablation: stimulating with strong laser
void SSDriver::landmarker_ablating(V3DPluginCallback &callback, QWidget *parent)
{
	qDebug()<<".................... Now running point ablating program ...................";
	LaserScanImaging *myimg = new LaserScanImaging(callback);
	myimg->imaging_op = 9; // ablation
	bool succ = myimg->acquire_single_direction(myimg->imaging_op);

	//myimg->cleanData(); // already cleaned in lsm
	
	if(myimg) {delete myimg; myimg=0;}
	v3d_msg("Finish landmarker ablation.", 0);
}



//  markers ablation from 3D view window
void SSDriver::tdview_landmarker_ablating_nocalib(V3DPluginCallback &callback, QWidget *parent)
{
	qDebug()<<".................... Now running point ablating program ..................."; 

	// current window
	v3dhandle wincurr = callback.curHiddenSelectedWindow(); 
	Image4DSimple* pImgIn = callback.getImage(wincurr); //not using the image class pointer in the parameter-struct, - is there correct in the V3DLONG run? noted by PHC
    if (! pImgIn)
	{
		v3d_msg("The pointer to the current image window is invalid. Do nothing.");
		return;
	}

	// parameters passed from 3d view
	v3d_imaging_paras *p = (v3d_imaging_paras *)pImgIn->getCustomStructPointer();
	//v3d_ablating_paras *p = (v3d_ablating_paras *)pImgIn->getCustomStructPointer();
	if(!p)
	{
		v3d_msg("The pointer to the parameter-struct is invalid. Do nothing.");
		return;
	}
	if (!(p->imgp))
	{
		v3d_msg("The image pointer inside the parameter-struct is invalid. Do nothing.");
		return;
	}

	// Error checking
	if(p->list_landmarks.size()<1)
	{
		v3d_msg("Need at least 1 markers in subject image.");
		return;
	}

	// ablation
	LaserScanImaging *myimg = new LaserScanImaging(callback);

	myimg->do_image_ablation(wincurr, p->imgp, p->list_landmarks);
	myimg->cleanData();

	if(myimg) delete myimg;

	v3d_msg("Finish landmarker ablation from 3D viewer.", 0);
}


// older version of ablation: stimulating with strong laser
void SSDriver::landmarker_ablating_nocalib(V3DPluginCallback &callback, QWidget *parent)
{
	// get paras for ablation
	Image4DSimple *pImgIn;
	v3dhandle curwin;
	curwin=callback.currentImageWindow();
	if(!curwin)
	{
		v3d_msg("No V3D window is available for returning data ... Do nothing.");
		return;
	}

	pImgIn=callback.getImage(curwin);
	if(!pImgIn) return;

	LandmarkList list_landmarks=callback.getLandmark(curwin);
	if(list_landmarks.size()<1)
	{
		v3d_msg("Need at least 1 markers in subject image.");
		return;
	}

	// ablation
	LaserScanImaging *myimg = new LaserScanImaging(callback);
	myimg->do_image_ablation(curwin, pImgIn, list_landmarks);
	myimg->cleanData();

	if(myimg) {delete myimg; myimg=0;}

	v3d_msg("Finish landmarker ablation.", 0);
}


// latest version of ablation: stimulating with strong laser
void SSDriver::landmarker_ablating_with_feedback(V3DPluginCallback &callback, QWidget *parent)
{
	// get paras for ablation
	Image4DSimple *pImgIn;
	v3dhandle curwin;
	curwin=callback.currentImageWindow();
	if(!curwin)
	{
		v3d_msg("No V3D window is available for returning data ... Do nothing.");
		return;
	}

	pImgIn=callback.getImage(curwin);
	if(!pImgIn) return;

	LandmarkList list_landmarks=callback.getLandmark(curwin);
	if(list_landmarks.size()<1)
	{
		v3d_msg("Need at least 1 markers in subject image.");
		return;
	}

	// ablation
	LaserScanImaging *myimg = new LaserScanImaging(callback);
	myimg->do_image_ablation_with_feedback(curwin, pImgIn, list_landmarks);
	myimg->cleanData();

	if(myimg) {delete myimg; myimg=0;}

	v3d_msg("Finish landmarker ablation.", 0);
}

//  markers ablation from 3D view window
void SSDriver::tdview_landmarker_ablating_with_feedback(V3DPluginCallback &callback, QWidget *parent)
{
	qDebug()<<".................... Now running point ablating program ..................."; 

	// current window
	v3dhandle wincurr = callback.curHiddenSelectedWindow(); 
	Image4DSimple* pImgIn = callback.getImage(wincurr); //not using the image class pointer in the parameter-struct, - is there correct in the V3DLONG run? noted by PHC
    if (! pImgIn)
	{
		v3d_msg("The pointer to the current image window is invalid. Do nothing.");
		return;
	}

	// parameters passed from 3d view
	v3d_imaging_paras *p = (v3d_imaging_paras *)pImgIn->getCustomStructPointer();
	//v3d_ablating_paras *p = (v3d_ablating_paras *)pImgIn->getCustomStructPointer();
	if(!p)
	{
		v3d_msg("The pointer to the parameter-struct is invalid. Do nothing.");
		return;
	}
	if (!(p->imgp))
	{
		v3d_msg("The image pointer inside the parameter-struct is invalid. Do nothing.");
		return;
	}

	// Error checking
	if(p->list_landmarks.size()<1)
	{
		v3d_msg("Need at least 1 markers in subject image.");
		return;
	}

	// ablation
	LaserScanImaging *myimg = new LaserScanImaging(callback);

	myimg->do_image_ablation_with_feedback(wincurr, p->imgp, p->list_landmarks);
	myimg->cleanData();

	if(myimg) delete myimg;

	v3d_msg("Finish landmarker ablation from 3D viewer.", 0);
}

//
void SSDriver::loading_imconfig(V3DPluginCallback &callback, QWidget *parent)
{
	// load paras from log file
	v3dhandleList win_list = callback.getImageWindowList();
	if(win_list.size()<1)
	{
		v3d_msg("No image is open.");
		return;
	}

	v3dhandle i1 = callback.currentImageWindow();
	Image4DSimple* pCurrImg = callback.getImage(i1);
    if (! pCurrImg) return;

	// get configure filename and load configuration info
	QString filename = callback.getImageName(i1);
	filename.chop(4); // chop ".raw"
	filename.append(".txt");

	if(!QFile(filename).exists())
	{
		filename = QFileDialog::getOpenFileName(0, QObject::tr("Find Configuration file"),
											  "/Documents", QObject::tr("Image Configuration (*.log)"));
		if(filename.isEmpty())
			return;
	}

	Parameters_LSM pl;
	pl.load_ParameterFile(filename);
	cout << "logfile name:"<<filename.toStdString()<<endl;

	// save back these paras to $ssConfigFolder/mi_configuration.txt
	QString lsm_FileName = ssConfigFolder + "mi_configuration_clipboard.txt";
	pl.save_ParameterFile(lsm_FileName);
}


void SSDriver::shutterAllOnOff(char onoff)
{
	DAQ_Shutter_RedLaser(onoff);
	DAQ_Shutter_GreenLaser(onoff);
	DAQ_Shutter_Detection(onoff);
	DAQ_Shutter_Stimulation(onoff);
}

// confocal imaging with one slice at z = 30 um
void SSDriver::mi_imaging_oneslice_z30(V3DPluginCallback &callback, QWidget *parent)
{
	LaserScanImaging *flimg=new LaserScanImaging(callback);
	flimg->imaging_op = 2; // define imaging operation

	// one slice
	bool succ = flimg->acquire_single_direction(flimg->imaging_op);
	if(!succ) return;

	// results
	QString filename_img_saved;
     // save image
     if(USE_PRIOR_CORRECTION == 1)
     {
          saveCorrectedImage(callback, filename_img_saved, flimg);
     }
     else
     {
          passImagingData2V3D(callback, flimg->pImFloat, "LSM", false, flimg->sx, flimg->sy, flimg->sz, flimg->sc, flimg->fov_step_x,
						flimg->fov_step_y, flimg->fov_step_z, flimg->min_fovx, flimg->min_fovy, flimg->min_fovz,
						flimg->max_fovx, flimg->max_fovy, flimg->max_fovz,
						flimg->ratio_x, flimg->ratio_y, flimg->ratio_z,
						flimg->time_out, flimg->scanning_rate, flimg->ch,
						flimg->scanMethod, flimg->b_useSimImg, flimg->b_scan_z_reverse,
						filename_img_saved, false);
     }

	 // print LUT to file
	filename_img_saved.chop(4);
	QString filename_lut = filename_img_saved + "_" + QString::number(flimg->scanning_rate)+"Hz_sx_"+
		QString::number(flimg->sx)+"_lut.txt";
	FILE *fp=fopen((char*)qPrintable(filename_lut), "wt");
	for(V3DLONG i=0; i<flimg->sx; i++)
	{
		fprintf(fp, "%d to %d\n", flimg->real_to_exp_pos_LUT[i], i);
	}
	fclose(fp);

	if(flimg) delete flimg;

	v3d_msg("Success! ...", 0);
}

// display configuration of current image
void SSDriver::display_imconfig(V3DPluginCallback &callback, QWidget *parent)
{
	// load paras from log file
	v3dhandleList win_list = callback.getImageWindowList();
	if(win_list.size()<1)
	{
		v3d_msg("No image is open.");
		return;
	}

	v3dhandle i1 = callback.currentImageWindow();
	Image4DSimple* pCurrImg = callback.getImage(i1);
    if (! pCurrImg) return;

	// get configure filename and load configuration info
	QString filename = callback.getImageName(i1);
	filename.chop(4); // chop ".raw"
	filename.append(".txt");

	if(!QFile(filename).exists())
	{
		filename = QFileDialog::getOpenFileName(0, QObject::tr("Find Configuration file"),
											  "/Documents", QObject::tr("Image Configuration (*.log)"));
		if(filename.isEmpty())
			return;
	}
	QString urlname("file:///");
	urlname.append(filename);
	QDesktopServices::openUrl(QUrl(urlname, QUrl::TolerantMode));

	v3d_msg("Success! ...", 0);

}

// map ROI in BF to LSM
void SSDriver::mapROI_BF_TO_LSM(V3DPluginCallback &callback, QWidget *parent)
{
	// first get two images from two windows
	v3dhandleList win_list = callback.getImageWindowList();
	if(win_list.size()<2)
	{
		v3d_msg("At least TWO images (one LSM and one BF images will be needed for calibration.");
		return;
	}

	QStringList items;
	for(int i=0; i<win_list.size(); i++)
		items << callback.getImageName(win_list[i]);

	// create a dialog for the calibration
	QDialog d(parent);
	QComboBox *combo_bf = new QComboBox();  combo_bf->addItems(items);
	QComboBox *combo_lsm = new QComboBox(); combo_lsm->addItems(items);

	QPushButton *ok = new QPushButton("Ok");
	QPushButton *cancel = new QPushButton("Cancel");

	QFormLayout *formLayout = new QFormLayout();
	formLayout->addRow(QObject::tr("BF Image with ROI: "), combo_bf);
	formLayout->addRow(QObject::tr("LSM Image for ROI Display: "), combo_lsm);
	formLayout->addRow(ok, cancel);
	d.setLayout(formLayout);
	d.setWindowTitle(QObject::tr("Map ROI from BF to LSM"));

	d.connect(ok, SIGNAL(clicked()), &d, SLOT(accept()));
	d.connect(cancel, SIGNAL(clicked()), &d, SLOT(reject()));

	if(d.exec() != QDialog::Accepted)
		return;

	int i_lsm = combo_lsm->currentIndex();
	int i_bf = combo_bf->currentIndex();

	Image4DSimple *pImLSM = callback.getImage(win_list[i_lsm]);
	Image4DSimple *pImBF = callback.getImage(win_list[i_bf]);

	// get ROI box in BF
	ROIList pRoiList=callback.getROI(win_list[i_bf]);
	if((pRoiList.at(0).size()<1)&&(pRoiList.at(1).size()<1)&&(pRoiList.at(2).size()<1))
	{
		v3d_msg("No ROI specified ... Do nothing.");
		return;
	}

	// ROI list only in the xy plane
	V3DLONG bb_start_x, bb_end_x, bb_start_y, bb_end_y, bb_start_z, bb_end_z;

	QRect b_xy = pRoiList.at(0).boundingRect();

	bb_start_x = qBound(V3DLONG(0), (V3DLONG)b_xy.left(), pImBF->getXDim()-1);
	bb_start_y = qBound(V3DLONG(0), (V3DLONG)b_xy.top(), pImBF->getYDim()-1);
	bb_start_z = 0;

	bb_end_x = qBound(V3DLONG(0), (V3DLONG)b_xy.right(), pImBF->getXDim()-1); //use qMax but not qMin
	bb_end_y = qBound(V3DLONG(0), (V3DLONG)b_xy.bottom(), pImBF->getYDim()-1);
	bb_end_z = 0;

	qDebug()<<"BF bb_start... (x, y, z)"<< bb_start_x << bb_start_y << bb_start_z;
	qDebug()<<"BF bb_end... (x, y, z)"<< bb_end_x << bb_end_y << bb_end_z;

	if (bb_start_x>bb_end_x || bb_start_y>bb_end_y || bb_start_z>bb_end_z)
	{
		v3d_msg("The roi polygons in three views are not intersecting! No crop is done!\n");
		return;
	}

	// get absolute physical position in LSM
	double o_origin_x, o_origin_y;
	double o_rez_x, o_rez_y;
	double a_bflsm, d_bflsm, e_bflsm, f_bflsm;
	Parameters_LSM pl;

	// always get BF_TO_LSM_A... from "mi_configuration.txt"
	// because this is the latest infor.
	Parameters_LSM mypara;
	if (!mypara.load_ParameterFile(ssConfigFolder + "mi_configuration.txt"))
	{
		v3d_msg("Fail to load the configuration file for bright-field imaging focus control. Do nothing");
		return;
	}

	a_bflsm = mypara.BF_TO_LSM_A; d_bflsm = mypara.BF_TO_LSM_D;
	e_bflsm = mypara.BF_TO_LSM_E; f_bflsm = mypara.BF_TO_LSM_F;

	// Acquire origin from the log file of the current image window
	QString logname_lsm = callback.getImageName(win_list[i_lsm]);

	// If the image is not created in the current session, it cannot get its origin from
	// pImLSM->getOriginX()... So the image log file is used for origin information.
	// decide whether this is selected from Focus_Control_LSM window
	if(logname_lsm.compare("Focus_Control_LSM")!=0)
	{
		logname_lsm.chop(4);
		logname_lsm.append(".txt");
		pl.load_ParameterFile(logname_lsm);

		o_origin_x = pl.FOVX_MIN;
		o_origin_y = pl.FOVY_MIN;
		o_rez_x = pl.FOVX_STEP;
		o_rez_y = pl.FOVY_STEP;
	}else // the current image is a live image from focus control
	{
		v3d_msg("Please do not select Focus_Control_LSM window! Do nothing.");
		return;
	}

	ROIList roil_bf=callback.getROI(win_list[i_bf]);
	QPolygon roi_poly = roil_bf.at(0);

	int pt_num=roi_poly.size();
	for(int i=0;i<pt_num; i++)
	{
		QPoint pt=roi_poly.at(i);
		// physical pos
		pt.setX(a_bflsm * pt.x() + e_bflsm);
		pt.setY(d_bflsm * pt.y() + f_bflsm);
		// transform to pixel pos
		pt.setX( (pt.x() - o_origin_x)/o_rez_x);
		pt.setY( (pt.y() - o_origin_y)/o_rez_y);

		// push back
		roi_poly.replace(i, pt);
	}

	roil_bf.replace(0,roi_poly);
	callback.setROI(win_list[i_lsm], roil_bf);

	callback.updateImageWindow(win_list[i_lsm]);

	v3d_msg("Success! ...", 0);

}

// microscopeimaging with 2 steps
void SSDriver::microscopeimaging2step(V3DPluginCallback &callback, QWidget *parent)
{
}




void SSDriver::saveCorrectedImage(V3DPluginCallback &callback, QString &filename_img_saved, LaserScanImaging * myimg)
{
	// correct image
	// substract scanning_offset_x from x and y
	float *pNewImg = 0;
	V3DLONG n_sx = myimg->sx - (V3DLONG)((myimg->offset_left + myimg->offset_right) * myimg->old_fov_step_x/myimg->fov_step_x +0.5);
	V3DLONG n_sy = myimg->sy;
	V3DLONG n_sz = myimg->sz;
	pNewImg = new float [myimg->sc * n_sx * n_sy * n_sz];

	printf("n_sx= %d, n_sy= %d, n_sz= %d\n", n_sx, n_sy, n_sz);
	printf("sx= %d, sy= %d, sz= %d\n", myimg->sx, myimg->sy, myimg->sz);

	for(V3DLONG c=0; c<myimg->sc; c++)
	{
		V3DLONG start_c = c * myimg->sz * myimg->sy * myimg->sx;
		V3DLONG n_start_c = c * n_sz * n_sy * n_sx;

		for(V3DLONG k=0; k<n_sz; k++)
		{
			V3DLONG start_k = k * myimg->sy * myimg->sx;
			V3DLONG n_start_k = k * n_sy * n_sx;

			for(V3DLONG j=0; j<n_sy; j++)
			{
				V3DLONG start_y = j * myimg->sx;
				V3DLONG n_start_y = j * n_sx;

				for(V3DLONG i=0; i<n_sx; i++)
				{
					V3DLONG idx = start_c + start_k + start_y + (i + (V3DLONG)(myimg->offset_left* myimg->old_fov_step_x / myimg->fov_step_x + 0.5 )); // real image starts from offset_left
					V3DLONG n_idx = n_start_c + n_start_k + n_start_y + i;
					pNewImg[n_idx] = myimg->pImFloat[idx];
				}
			}
		}
	}

	// myimg->pImFloat is no use now
	if(myimg->pImFloat) {delete []myimg->pImFloat; myimg->pImFloat = NULL;}

	// save correct image
	passImagingData2V3D(callback, pNewImg, "LSM", false, n_sx, n_sy, n_sz, myimg->sc, myimg->fov_step_x,
						myimg->fov_step_y, myimg->fov_step_z, myimg->min_fovx, myimg->min_fovy, myimg->min_fovz,
						myimg->max_fovx, myimg->max_fovy, myimg->max_fovz,
						myimg->ratio_x, myimg->ratio_y, myimg->ratio_z,
						myimg->time_out, myimg->scanning_rate, myimg->ch,
						myimg->scanMethod, myimg->b_useSimImg, myimg->b_scan_z_reverse,
						filename_img_saved,false);


	// save corrected AI_x
	//pAIx is a 2d img
	float *pNewAI_x = 0;
	pNewAI_x = new float [n_sx * n_sy];

	for(V3DLONG k=0; k<1; k++)
	{
		V3DLONG start_k = k * myimg->sy * myimg->sx;
		V3DLONG n_start_k = k * n_sy * n_sx;

		for(V3DLONG j=0; j<n_sy; j++)
		{
			V3DLONG start_y = j * myimg->sx;
			V3DLONG n_start_y = j * n_sx;

			for(V3DLONG i=0; i<n_sx; i++)
			{
				V3DLONG idx = start_k + start_y + (i + (V3DLONG)(myimg->offset_left* myimg->old_fov_step_x / myimg->fov_step_x + 0.5 )); // real image starts from offset_left
				V3DLONG n_idx = n_start_k + n_start_y + i;
				pNewAI_x[n_idx] = myimg->pAIx[idx];
			}
		}
	}

	//// save corrected AI_y of the whole image
	////pAIx is a 2d img
	//float *pNewAI_y = 0;
	//pNewAI_y = new float [n_sx * n_sy];

	//for(V3DLONG k=0; k<1; k++)
	//{
	//	V3DLONG start_k = k * myimg->sy * myimg->sx;
	//	V3DLONG n_start_k = k * n_sy * n_sx;

	//	for(V3DLONG j=0; j<n_sy; j++)
	//	{
	//		V3DLONG start_y = j * myimg->sx;
	//		V3DLONG n_start_y = j * n_sx;

	//		for(V3DLONG i=0; i<n_sx; i++)
	//		{
	//			V3DLONG idx = start_k + start_y + (i + (V3DLONG)(myimg->offset_left* myimg->old_fov_step_x / myimg->fov_step_x + 0.5 )); // real image starts from offset_left
	//			V3DLONG n_idx = n_start_k + n_start_y + i;
	//			pNewAI_y[n_idx] = myimg->pAIy[idx];
	//		}
	//	}
	//}

	// save AI_x to file name
	QString filename_AI = filename_img_saved;
	filename_AI.chop(4);
	QString filename_AIx_raw=filename_AI + "_AIx.raw";
	//QString filename_AIy_raw=filename_AI + "_AIy.raw";
	QString filename_AIx_txt=filename_AI + "_AIx.txt";

	// save AI_x to a raw img
	V3DLONG save_sz[4];
	save_sz[0] = n_sx; save_sz[1] = n_sy; save_sz[2] = 1; save_sz[3] = 1;
	saveImage(qPrintable(filename_AIx_raw), (unsigned char *)pNewAI_x, save_sz, 4);
	//saveImage(qPrintable(filename_AIy_raw), (unsigned char *)pNewAI_y, save_sz, 4);
	// save AI_x to a txt file, only one line
	saveAIx2Txt(filename_AIx_txt, n_sx, 1, 1, pNewAI_x);

	if(pNewAI_x) {delete []pNewAI_x; pNewAI_x=0;}
}


void SSDriver::saveCorrectedImage_roi(V3DPluginCallback &callback, QString &filename_img_saved, LaserScanImaging * myimg)
{
	// substract scanning_offset_x from x and y
	float *pNewImg = 0;
	V3DLONG n_sx = myimg->sx - (V3DLONG)((myimg->offset_left + myimg->offset_right) * myimg->old_fov_step_x/myimg->fov_step_x +0.5)
							 - ((V3DLONG)((myimg->real_min_fovx - myimg->min_fovx)/myimg->fov_step_x) 
							    + (V3DLONG)((myimg->max_fovx - myimg->real_max_fovx)/myimg->fov_step_x));

	V3DLONG n_sy = myimg->sy;
	V3DLONG n_sz = myimg->sz;
	pNewImg = new float [myimg->sc * n_sx * n_sy * n_sz];

	printf("n_sx= %d, n_sy= %d, n_sz= %d\n", n_sx, n_sy, n_sz);
	printf("sx= %d, sy= %d, sz= %d\n", myimg->sx, myimg->sy, myimg->sz);

	for(V3DLONG c=0; c<myimg->sc; c++)
	{
		V3DLONG start_c = c * myimg->sz * myimg->sy * myimg->sx;
		V3DLONG n_start_c = c * n_sz * n_sy * n_sx;

		for(V3DLONG k=0; k<n_sz; k++)
		{
			V3DLONG start_k = k * myimg->sy * myimg->sx;
			V3DLONG n_start_k = k * n_sy * n_sx;

			for(V3DLONG j=0; j<n_sy; j++)
			{
				V3DLONG start_y = j * myimg->sx;
				V3DLONG n_start_y = j * n_sx;

				for(V3DLONG i=0; i<n_sx; i++)
				{
					V3DLONG idx = start_c + start_k + start_y + (i + (V3DLONG)(myimg->offset_left* myimg->old_fov_step_x / myimg->fov_step_x + 0.5 )
																   + (V3DLONG)((myimg->real_min_fovx-myimg->min_fovx)/myimg->fov_step_x) ); // real image starts from offset_left
					V3DLONG n_idx = n_start_c + n_start_k + n_start_y + i;
					pNewImg[n_idx] = myimg->pImFloat[idx];
				}
			}
		}
	}

	// myimg->pImFloat is no use now
	if(myimg->pImFloat) {delete []myimg->pImFloat; myimg->pImFloat=NULL;}

	// save correct image
	passImagingData2V3D(callback, pNewImg, "LSM", false, n_sx, n_sy, n_sz, myimg->sc, myimg->fov_step_x,
						myimg->fov_step_y, myimg->fov_step_z, myimg->real_min_fovx, myimg->min_fovy, myimg->min_fovz,
						myimg->real_max_fovx, myimg->max_fovy, myimg->max_fovz,
						myimg->ratio_x, myimg->ratio_y, myimg->ratio_z,
						myimg->time_out, myimg->scanning_rate, myimg->ch,
						myimg->scanMethod, myimg->b_useSimImg, myimg->b_scan_z_reverse,
						filename_img_saved,false);


	// save corrected AI_x of the whole image
	//pAIx is a 2d img
	float *pNewAI_x = 0;
	pNewAI_x = new float [n_sx * n_sy];

	for(V3DLONG k=0; k<1; k++)
	{
		V3DLONG start_k = k * myimg->sy * myimg->sx;
		V3DLONG n_start_k = k * n_sy * n_sx;

		for(V3DLONG j=0; j<n_sy; j++)
		{
			V3DLONG start_y = j * myimg->sx;
			V3DLONG n_start_y = j * n_sx;

			for(V3DLONG i=0; i<n_sx; i++)
			{
				V3DLONG idx = start_k + start_y + (i + (V3DLONG)(myimg->offset_left* myimg->old_fov_step_x / myimg->fov_step_x + 0.5 )
																   + (V3DLONG)((myimg->real_min_fovx-myimg->min_fovx)/myimg->fov_step_x) ); // real image starts from offset_left
				V3DLONG n_idx = n_start_k + n_start_y + i;
				pNewAI_x[n_idx] = myimg->pAIx[idx];
			}
		}
	}

	//// save corrected AI_y of the whole image
	////pAIy is a 2d img
	//float *pNewAI_y = 0;
	//pNewAI_y = new float [n_sx * n_sy];

	//for(V3DLONG k=0; k<1; k++)
	//{
	//	V3DLONG start_k = k * myimg->sy * myimg->sx;
	//	V3DLONG n_start_k = k * n_sy * n_sx;

	//	for(V3DLONG j=0; j<n_sy; j++)
	//	{
	//		V3DLONG start_y = j * myimg->sx;
	//		V3DLONG n_start_y = j * n_sx;

	//		for(V3DLONG i=0; i<n_sx; i++)
	//		{
	//			V3DLONG idx = start_k + start_y + (i + (V3DLONG)(myimg->offset_left* myimg->old_fov_step_x / myimg->fov_step_x + 0.5 )
	//															   + (V3DLONG)((myimg->real_min_fovx-myimg->min_fovx)/myimg->fov_step_x) ); // real image starts from offset_left
	//			V3DLONG n_idx = n_start_k + n_start_y + i;
	//			pNewAI_y[n_idx] = myimg->pAIy[idx];
	//		}
	//	}
	//}

	// save AI_x to file name
	QString filename_AI = filename_img_saved;
	filename_AI.chop(4);
	QString filename_AIx_raw=filename_AI + "_AIx.raw";
	QString filename_AIx_txt=filename_AI + "_AIx.txt";
	//QString filename_AIy_raw=filename_AI + "_AIy.raw";


	// save AI_x to a raw img
	V3DLONG save_sz[4];
	save_sz[0] = n_sx; save_sz[1] = n_sy; save_sz[2] = 1; save_sz[3] = 1;
	saveImage(qPrintable(filename_AIx_raw), (unsigned char *)pNewAI_x, save_sz, 4);
	// save AI_x to a txt file, only one line
	saveAIx2Txt(filename_AIx_txt, n_sx, 1, 1, pNewAI_x);
	//saveImage(qPrintable(filename_AIy_raw), (unsigned char *)pNewAI_y, save_sz, 4);

	if(pNewAI_x) {delete []pNewAI_x; pNewAI_x=0;}

}


void SSDriver::saveAIx2Txt(QString filename, V3DLONG sx, V3DLONG sy, V3DLONG sz, float *pAIx)
{
	FILE *fp=fopen(filename.toStdString().c_str(), "wt");
	// first is pAIx. only use one slice
	for(V3DLONG k=0; k<sz; k++)
	{
		V3DLONG n_start_k = k * sy * sx;
		for(V3DLONG j=0; j<sy; j++)
		{
			V3DLONG n_start_y = j * sx;
			for(V3DLONG i=0; i<sx; i++)
			{
				V3DLONG n_idx = n_start_k + n_start_y + i;
				fprintf(fp, "%.6f  ", pAIx[n_idx]);
			}
			fprintf(fp, "\n");
		}
	}
	fclose(fp);
}