//smartscope_util.cpp
//some utility functions for the smartscope controller
// by Hanchuan Peng
// 2011-May-13

#include "stackutil.h"

#include "smartscope_util.h"
#include "configfileio_lsm.h"
//#include "smartscope_gui.h"

#include <QDateTime> 

void wait_milliseconds( V3DLONG milliseconds )
{
	clock_t t1=clock();
	while (clock()-t1 < milliseconds) {}
}

void wait_pixels( V3DLONG npixels, double scanning_rate )
{
	wait_milliseconds (V3DLONG(npixels*scanning_rate/1000.0));
}

QString get_title()
{
	return QString("SmartScope Imaging");
}

// return a generated file name 
// suffix could be ".log" or ".raw"
// prefix could be "BF" or "FL"
QString generate_img_name(QString prefix, QString suffix) 
{
	//stringstream ss;
	QString working_dir = "C:/smartscope_data/"; // pre-defined

	QDateTime now = QDateTime::currentDateTime();

	int month = now.date().month();
	int day = now.date().day();

	QString qstr_time;
	//qstr_time = QString("%1%2%3").arg(now.date().year()).arg(now.date().month()).arg(now.date().day());

	// e.g. "20110101"
	if(month<10)
	{
		if(day<10)
		{
			qstr_time = QString("%1").arg(now.date().year()).append("0%2").arg(month).append("0%3").arg(day);
		}
		else
		{
			qstr_time = QString("%1").arg(now.date().year()).append("0%2%3").arg(month).arg(day);
		}
	}
	else
	{
		if(day<10)
		{
			qstr_time = QString("%1%2").arg(now.date().year()).arg(month).append("0%3").arg(day);
		}
		else
		{
			qstr_time = QString("%1%2%3").arg(now.date().year()).arg(month).arg(day);
		}
	}

	QString foldername = working_dir + qstr_time + QString("/");
	QString individualFileName = foldername + prefix + QString("_") + qstr_time + QString("_%1").arg(1) + suffix; // for example "20101006\BF_2010106_1.raw"

	//
	QStringList myList;
	myList.clear();
	
	// get the image files namelist in the directory
	QFileInfo fileInfo(individualFileName);
	QString curFilePath = fileInfo.path();
	QString curSuffix = fileInfo.suffix();
	
	QDir dir(curFilePath);
	if (!dir.exists())
	{
		qWarning("Cannot find the directory and now mkdir.");
		bool success_mkdir = dir.mkdir(curFilePath);

		qDebug()<<"mkdir "<<curFilePath<<success_mkdir;
	}

	QStringList imgfilters;
	imgfilters.append("*." + curSuffix);
	foreach (QString file, dir.entryList(imgfilters, QDir::Files, QDir::Name))
	{
		myList += QFileInfo(dir, file).absoluteFilePath();
	}
	
	QString qs_fn = foldername + prefix + QString("_") + qstr_time + QString("_%1").arg(myList.size()+1) + suffix;

	qDebug()<<"filename: "<<qs_fn;
	return qs_fn; 
}


QString get_last_img_name(QString prefix, QString suffix) 
{
	//stringstream ss;
	QString working_dir = "C:/smartscope_data/"; // pre-defined

	QDateTime now = QDateTime::currentDateTime();

	int month = now.date().month();
	int day = now.date().day();

	QString qstr_time;
	//qstr_time = QString("%1%2%3").arg(now.date().year()).arg(now.date().month()).arg(now.date().day());

	// e.g. "20110101"
	if(month<10)
	{
		if(day<10)
		{
			qstr_time = QString("%1").arg(now.date().year()).append("0%2").arg(month).append("0%3").arg(day);
		}
		else
		{
			qstr_time = QString("%1").arg(now.date().year()).append("0%2%3").arg(month).arg(day);
		}
	}
	else
	{
		if(day<10)
		{
			qstr_time = QString("%1%2").arg(now.date().year()).arg(month).append("0%3").arg(day);
		}
		else
		{
			qstr_time = QString("%1%2%3").arg(now.date().year()).arg(month).arg(day);
		}
	}

	QString foldername = working_dir + qstr_time + QString("/");
	QString individualFileName = foldername + prefix + QString("_") + qstr_time + QString("_%1").arg(1) + suffix; // for example "20101006\BF_2010106_1.raw"

	//
	QStringList myList;
	myList.clear();
	
	// get the image files namelist in the directory
	QFileInfo fileInfo(individualFileName);
	QString curFilePath = fileInfo.path();
	QString curSuffix = fileInfo.suffix();
	
	QDir dir(curFilePath);
	//QDir dir(foldername);
	if (!dir.exists())
	{
		v3d_msg("You have not created image today! ", 1);
		return QString("");
	}

	QStringList imgfilters;
	imgfilters.append("*." + curSuffix);
	foreach (QString file, dir.entryList(imgfilters, QDir::Files, QDir::Name))
	{
		myList += QFileInfo(dir, file).absoluteFilePath();
	}
	
	QString qs_fn = foldername + prefix + QString("_") + qstr_time + QString("_%1").arg(myList.size()) + suffix;

	qDebug()<<"filename: "<<qs_fn;
	return qs_fn; 
}


bool passImagingData2V3D(V3DPluginCallback &callback, 
						float *pImFloat, QString imgname, bool b_convertTo8Bit, 
						V3DLONG sx, V3DLONG sy, V3DLONG sz, V3DLONG sc, 
						double fov_step_x, double fov_step_y, double fov_step_z, 
						double min_fovx, double min_fovy, double min_fovz, 
						double max_fovx, double max_fovy, double max_fovz, 
						double ratio_x, double ratio_y, double ratio_z,
						int time_out, int scanning_rate, int ch,
						int scanMethod, int b_useSimImg, int b_scan_z_reverse,
						QString &filename_img_saved, bool b_open3dwin)
{
	//first check if the data is valid
	if (!pImFloat || sx<=0 || sy <=0 || sz<=0 || sc <=0)
	{
		v3d_msg("The data in result_handling() is invalid(). Do nothing.");
		return false;
	}

	// always save a copy of the float data (image) directly
	//QString 
	filename_img_saved = generate_img_name(imgname, ".raw");
	V3DLONG save_sz[4];
	save_sz[0] = sx; save_sz[1] = sy; save_sz[2] = sz; save_sz[3] = sc;
	saveImage(qPrintable(filename_img_saved), (unsigned char *)pImFloat, save_sz, 4);

	// also save the imaging parameters to the respective log file in txt format
	QString qstr_logname = filename_img_saved;
	qstr_logname.chop(4);
	qstr_logname.append(".txt");

	QString m_FileName = "C:/smartscope_conf/mi_configuration.txt";
	string filename_pre = m_FileName.toStdString();

	Parameters_LSM mypara;
	mypara.load_ParameterFile(m_FileName);

	//it seems these assignments are unnecessary, as the parameters passed in from this function are loaded 
	//from the same file for imaging
	mypara.FOVX_STEP = fov_step_x;
	mypara.FOVY_STEP = fov_step_y;
	mypara.FOVZ_STEP = fov_step_z;

	mypara.FOVX_MIN = min_fovx;
	mypara.FOVY_MIN = min_fovy;
	mypara.FOVZ_MIN = min_fovz;

	mypara.FOVX_MAX = max_fovx;
	mypara.FOVY_MAX = max_fovy;
	mypara.FOVZ_MAX = max_fovz;

	mypara.SX = sx;
	mypara.SY = sy;
	mypara.SZ = sz;

	mypara.RATIO_X=ratio_x; mypara.RATIO_Y=ratio_y; mypara.RATIO_Z=ratio_z; 
	mypara.TIMEOUT=time_out; mypara.SCANNING_RATE=scanning_rate; mypara.CHANNEL=ch;
	mypara.SCAN_METHOD=scanMethod; mypara.USE_SIMIMG=b_useSimImg; mypara.SCAN_Z_REVERSE=b_scan_z_reverse;

	mypara.save_ParameterFile(qstr_logname);
	v3d_msg(QString("image [%1] and log [%2] have been saved.").arg(filename_img_saved).arg(qstr_logname), 0);

	// visualization
	Image4DSimple p4DImage;
	QString win_name =  filename_img_saved;

	if(!b_convertTo8Bit)
	{
		p4DImage.setData((unsigned char *)pImFloat, sx, sy, sz, sc, V3D_FLOAT32); //default data type is FLOAT
	}
	else //now convert to 8-bit
	{
		try
		{
			V3DLONG channelsz = sx*sy*sz;
			unsigned char *pImUChar = new unsigned char [channelsz*sc];
			for(V3DLONG c=0; c<sc; c++)
			{
				V3DLONG offset = c*channelsz;
				float maxv = pImFloat[offset], minv = pImFloat[offset]; //should be safe to access element [offset], 
				                             //as the dimensions have been checked at the beginning of this function
				for(V3DLONG i=1+offset; i<channelsz+offset; i++)
				{
					float vv = pImFloat[i];
					if(vv > maxv) 
						maxv = vv;
					else if(vv < minv)
						minv = vv;
				}
				maxv -= minv;

				if(fabs(maxv)<0.000001)
				{
					for(V3DLONG j=offset; j<channelsz+offset; j++)
						pImUChar[j] = 0;
				}
				else
				{
					double sf = 255.0/maxv;
					for(V3DLONG j=offset; j<channelsz+offset; j++)
						pImUChar[j] = (unsigned char)(sf*(pImFloat[j]-minv));
				}
			}
			p4DImage.setData((unsigned char *)pImUChar, sx, sy, sz, sc, V3D_UINT8);

			win_name.chop(4);
			win_name.append("_8bit.raw");

			// de-alloc
			//if(pImFloat) {delete []pImFloat; pImFloat=0;}
		}
		catch(...)
		{
			v3d_msg("Error allocating memory for image.\n");
			return false;
		}
	}

	//also assign the resolution and origin information for the image
	p4DImage.setRezX(fov_step_x); p4DImage.setRezY(fov_step_y); p4DImage.setRezZ(fov_step_z); 
	p4DImage.setOriginX(min_fovx); p4DImage.setOriginY(min_fovy); p4DImage.setOriginZ(min_fovz);

	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &p4DImage);
	callback.setImageName(newwin, win_name); //
	callback.updateImageWindow(newwin);

	if(b_open3dwin)
		callback.open3DWindow(newwin); //open 3D Viewer

	v3d_msg(QString("V3D window created [%1].").arg(win_name), 0);
	v3d_msg(QString("image dims = [%1][%2][%3].").arg(sx).arg(sy).arg(sz), 0);
	v3d_msg(QString("      origin = [%1][%2][%3].").arg(min_fovx).arg(min_fovy).arg(min_fovz), 0);
	v3d_msg(QString("      resolution = [%1][%2][%3].").arg(fov_step_x).arg(fov_step_y).arg(fov_step_z), 0);
	v3d_msg("-----------------------------------------------------------------------\n", 0);

	return true;
}
