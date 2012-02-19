// multithreaded_imgloading.cpp
// Nov. 18, 2010. YuY
// Last update: 2010-12-10. by Hanchuan Peng. update window focus works!
// Last update: 2010-12-28. by PHC. change menu and add alpha-info, also add a wrapper function for menus. still crash on 32bit Mac 10.5.8

#include "multithreaded_imgloading.h"
#include "v3d_message.h"

#include "y_img_thread.h"
#include "customary_structs/v3d_multithreadimageio_para.h"

#include <time.h>


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(multithreaded_imgloading, MTIMGLOADPlugin);

int mtimgloading(V3DPluginCallback2 &callback, QWidget *parent);
int showinfo();
int mtimgio(V3DPluginCallback2 &callback, QWidget *parent); // engine call
int mtimgio(V3DPluginCallback2 &callback, QWidget *parent, string filename);
int mtimgio2(V3DPluginCallback2 &callback, QWidget *parent);
int mtimgio2(V3DPluginCallback2 &callback, QWidget *parent, QString m_FileName);

//plugin funcs
const QString title = "multithreaded_imgloading";
QStringList MTIMGLOADPlugin::menulist() const
{
    return QStringList() 
		<< tr("Show Info")
		<< tr("MT Image Loading")
		<< tr("HIDDEN-MTIMGIO")
		<< tr("MTIMGIO2 (alpha)")
		<< tr("About");
}

void MTIMGLOADPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Show Info"))
    {
		showinfo();
    }
	else if (menu_name == tr("MT Image Loading"))
    {
		mtimgloading(callback, parent);
    }
	else if (menu_name == tr("HIDDEN-MTIMGIO"))
    {
		mtimgio(callback, parent);
    }
	else if (menu_name == tr("MTIMGIO2 (alpha)"))
    {
		mtimgio2(callback, parent);
    }
	else if (menu_name == tr("About"))
	{
    	v3d_msg(QString("Multithreaded Image Loading Plugin %1, Dec 28, 2010. \
						\nKey release 1.1 support .raw/.tif/.lsm files, Dec. 8, 2010. \
						\nKey release 1.0 created by Yang Yu, Peng Lab, Janelia Farm Research Campus, HHMI, Nov. 18, 2010.").arg(getPluginVersion()));
		
		return;
	}
}

// macro for updating min-max and focus
void updateTriview(TriviewControl *triviewControl, V3DLONG currslice)
{
	// updateMinMax then changeFocus
	triviewControl->updateMinMax(currslice-1);
	
	V3DLONG x, y, z; 
	triviewControl->getFocusLocation( x, y, z);
	triviewControl->setFocusLocation( x, y, currslice); // updateViews here Dec 17, 2010, by YuY
	
}

// macro for setting focus at centers
void reinitTriviewFocus(TriviewControl *triviewControl, V3DLONG sx, V3DLONG sy, V3DLONG sz)
{
	// set focus to centers
	triviewControl->setFocusLocation( sx/2, sy/2, sz/2);	
}

// read image header and show 
int showinfo()
{
	// anchor the image to handle
	QString m_FileName = QFileDialog::getOpenFileName(0, QObject::tr("Find a file"), "/", QObject::tr("Image (*.*)"));
	
	string filename = m_FileName.toStdString();
	
	// image load and visualize streaming
	YY_ImgRead<V3DLONG, float> imglv(filename);
	
	if(imglv.tag_fileformat==-1)
	{
		v3d_msg("This file format is not supported by default! Please try to open it using V3D bio-format plugin!\n");
		return false;
	}
	
	imglv.init();
	
	//
	if(imglv.tag_fileformat == V3DRAW) // Hanchuan's .raw
	{
		
		QTextEdit *rawinfo = new QTextEdit(QString("image: %1 <br>\
													<br><br> Dimensions: \
													<br> x (%2) \
													<br> y (%3) \
													<br> z (%4) \
													<br> c (%5) \
													<br><br> Resolutions: \
													<br> x (%6) \
													<br> y (%7) \
													<br> z (%8) ").arg(filename.c_str()).arg(imglv.dims_x).arg(imglv.dims_y).arg(imglv.dims_z)
													.arg(imglv.dims_c).arg(imglv.res_x).arg(imglv.res_y).arg(imglv.res_z));
		
		rawinfo->setDocumentTitle("Image Header Info");
		rawinfo->resize(500, 300);
		rawinfo->setReadOnly(true);
		rawinfo->setFontPointSize(12);
		rawinfo->show();
		
	}
	else if (imglv.tag_fileformat == V3DTIFF) // .tif
	{
		QTextEdit *tiffinfo = new QTextEdit(QString("image: %1 <br>\
												   <br><br> Dimensions: \
												   <br> x (%2) \
												   <br> y (%3) \
												   <br> z (%4) \
												   <br> c (%5) \
												   <br><br> Resolutions: \
												   <br> x (%6) dpi \
												   <br> y (%7) dpi ").arg(filename.c_str()).arg(imglv.dims_x).arg(imglv.dims_y).arg(imglv.dims_z)
										   .arg(imglv.dims_c).arg(imglv.res_x).arg(imglv.res_y)); // cannot get z resolution for .tif
		
		tiffinfo->setDocumentTitle("Image Header Info");
		tiffinfo->resize(500, 300);
		tiffinfo->setReadOnly(true);
		tiffinfo->setFontPointSize(12);
		tiffinfo->show();

	}
	else if (imglv.tag_fileformat == V3DLSM) // .lsm
	{
		QTextEdit *lsminfo = new QTextEdit(QString("image: %1 <br>\
												   <br><br> Dimensions: \
												   <br> x (%2) \
												   <br> y (%3) \
												   <br> z (%4) \
												   <br> c (%5) \
												   <br><br> Resolutions: \
												   <br> x (%6) um\
												   <br> y (%7) um\
												   <br> z (%8) um").arg(filename.c_str()).arg(imglv.dims_x).arg(imglv.dims_y).arg(imglv.dims_z)
										   .arg(imglv.dims_c).arg(imglv.res_x*1e6).arg(imglv.res_y*1e6).arg(imglv.res_z*1e6));
		
		lsminfo->setDocumentTitle("Image Header Info");
		lsminfo->resize(500, 300);
		lsminfo->setReadOnly(true);
		lsminfo->setFontPointSize(12);
		lsminfo->show();
	
	}
	else
	{
		v3d_msg("This file format is not supported by default! Please try to open it using V3D bio-format plugin!\n");
		return false;
	}

	//
	return true;
	
}

// multi-threaded loading image
int mtimgloading(V3DPluginCallback2 &callback, QWidget *parent)
{
	// anchor the image to handle
	QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Find a file"), "/", QObject::tr("Image (*.*)"));
	
	string filename = m_FileName.toStdString();
	
	if (0)
		return mtimgio(callback, parent, filename);
	else
	{
		
		// image load and visualize streaming
		YY_ImgRead<V3DLONG, float> imglv(filename);
		
		if(imglv.tag_fileformat == V3DSURFACEOBJ)
		{
			v3d_msg("This file is the V3D surface object!\n");
			return false;
		}
		else if(imglv.tag_fileformat == V3DUNKNOWN)
		{
			v3d_msg("This file format is not supported by default! Please try to open it using V3D bio-format plugin!\n");
			return false;
		}
		
		imglv.init();
		
		V3DLONG datatype = imglv.datatype;
		V3DLONG sx = imglv.dims_x, sy = imglv.dims_y, sz = imglv.dims_z, sc = imglv.dims_c;
		
		Image4DSimple p4DImage;
		
		v3dhandle curwin;
		curwin = callback.newImageWindow();
		
		qDebug()<<"dims ..."<<sx<<sy<<sz<<sc<<"datatype ..."<<datatype;
		
		TriviewControl *triviewControl =  callback.getTriviewControl(curwin);
		
		// For different datatype
		if (datatype!=1 && datatype!=2 && datatype!=4)
			return false;
		
		if(datatype == 1)
		{
			unsigned char *pImage = NULL;
			
			YY_ImgRead<V3DLONG, unsigned char> imglv_u8bit(filename);
			imglv_u8bit.init(pImage);
			imglv_u8bit.setTriviewControl(triviewControl);
			
			if(pImage)
			{
				p4DImage.setData((unsigned char*)pImage, sx, sy, sz, sc, V3D_UINT8); // Set data 
				
				callback.setImage(curwin, &p4DImage);
				callback.setImageName(curwin, m_FileName);
				callback.updateImageWindow(curwin);

				qDebug()<<"loading 8 bit data ... ";
				
				int mytype = -1;
				triviewControl->getTriViewColorDispType(mytype);
				triviewControl->setTriViewColorDispType(mytype);
				
				qDebug()<<"ImageDisplayColorType ..."<<mytype;
				
				while (!imglv_u8bit.image_loaded && !imglv_u8bit.stopped) 
				{
					// update tri-view 
					if(!callback.currentImageWindow())//
					{
						imglv_u8bit.stop();
					}
					else
					{
						v3dhandleList win_list = callback.getImageWindowList();
						
						bool flag_win_exist = false;
						for (int i=0; i<win_list.size(); i++) 
						{
							QString iname = callback.getImageName(win_list[i]);
							
							if(iname.compare(m_FileName) == 0)
							{
								flag_win_exist = true;
							}
						}
						if(!flag_win_exist)
						{
							imglv_u8bit.stop();
							continue;
						}
						else
						{
							// load a frame
							imglv_u8bit.start();
							imglv_u8bit.wait();
							
							//callback.updateImageWindow(curwin);
							//updateTriview(triviewControl, imglv_u8bit.nFrame);
							QCoreApplication::processEvents();
						}
					}
				}
			}
			qDebug()<<"8-bit data loading success!";
			
		}
		else if(datatype == 2)
		{
			short int *pImage = NULL;
			
			YY_ImgRead<V3DLONG, short int> imglv_u16bit(filename);
			imglv_u16bit.init(pImage);
			imglv_u16bit.setTriviewControl(triviewControl);
			
			if(pImage)
			{
				p4DImage.setData((unsigned char*)pImage, sx, sy, sz, sc, V3D_UINT16); // Set data 
				
				callback.setImage(curwin, &p4DImage);
				callback.setImageName(curwin, m_FileName);
				callback.updateImageWindow(curwin);
				
				//
				qDebug()<<"loading 16 bit data ... ";
				while (!imglv_u16bit.image_loaded  && !imglv_u16bit.stopped) 
				{
					
					// update tri-view 
					if(!callback.currentImageWindow())//
					{
						imglv_u16bit.stop();
					}
					else
					{
						//
						v3dhandleList win_list = callback.getImageWindowList();
						
						bool flag_win_exist = false;
						for (int i=0; i<win_list.size(); i++) 
						{
							QString iname = callback.getImageName(win_list[i]);
							
							if(iname.compare(m_FileName) == 0)
							{
								flag_win_exist = true;
							}
						}
						if(!flag_win_exist)
						{
							imglv_u16bit.stop();
							continue;
						}
						else
						{
							// load a frame
							imglv_u16bit.start();
							imglv_u16bit.wait();
							
							//callback.updateImageWindow(curwin);
							//updateTriview(triviewControl, imglv_u16bit.nFrame);
							QCoreApplication::processEvents();
						}
					}
				}
			}
			qDebug()<<"16-bit data loading success!";
			
		}
		else if(datatype == 4)
		{
			float *pImage = NULL;
			
			YY_ImgRead<V3DLONG, float> imglv_float32bit(filename);
			imglv_float32bit.init(pImage);
			imglv_float32bit.setTriviewControl(triviewControl);
			
			if(pImage)
			{
				p4DImage.setData((unsigned char*)pImage, sx, sy, sz, sc, V3D_FLOAT32); // Set data 
				
				callback.setImage(curwin, &p4DImage);
				callback.setImageName(curwin, m_FileName);
				callback.updateImageWindow(curwin);
				
				//
				qDebug()<<"loading 32 bit data ... ";
				while (!imglv_float32bit.image_loaded && !imglv_float32bit.stopped) 
				{
					// update tri-view 
					if(!callback.currentImageWindow())//
					{
						imglv_float32bit.stop();
					}
					else
					{
						//
						v3dhandleList win_list = callback.getImageWindowList();
						
						bool flag_win_exist = false;
						for (int i=0; i<win_list.size(); i++) 
						{
							QString iname = callback.getImageName(win_list[i]);
							
							if(iname.compare(m_FileName) == 0)
							{
								flag_win_exist = true;
							}
						}
						if(!flag_win_exist)
						{
							imglv_float32bit.stop();
							continue;
						}
						else
						{
							// load a frame
							imglv_float32bit.start();
							imglv_float32bit.wait();
							
							//callback.updateImageWindow(curwin);
							//updateTriview(triviewControl, imglv_float32bit.nFrame);
							QCoreApplication::processEvents();
						}
					}
				}
			}
			qDebug()<<"32-bit data loading success!";
		}
		
		if (triviewControl)
			reinitTriviewFocus(triviewControl, sx, sy, sz);

		return true;
	}
}

// multi-threaded image io for engine call
int mtimgio(V3DPluginCallback2 &callback, QWidget *parent)
{
	// anchor the image to handle
	Image4DSimple p4DImage;
	
	v3dhandle curwin;
	//curwin = callback.newImageWindow();
	curwin = callback.curHiddenSelectedWindow();
	
	TriviewControl *triviewControl =  callback.getTriviewControl(curwin);
	
	// passing parameters from triview
	v3d_multithreadimageio_paras *p = (v3d_multithreadimageio_paras *)triviewControl->getCustomStructPointer();
	
	if(!p)
	{
		v3d_msg("The pointer to the parameter-struct is invalid. Do nothing.\n", 0);
		return false;
	}
	
	QString m_FileName = p->qFilename;
	
	if(m_FileName.isEmpty())
	{
		v3d_msg("This file name is not passed correctly!\n");
		return false;
	}
	
	string filename = m_FileName.toStdString();
	
	return mtimgio(callback, parent, filename);
}

int mtimgio(V3DPluginCallback2 &callback, QWidget *parent, string filename)
{
	// anchor the image to handle
	Image4DSimple p4DImage;
	
	v3dhandle curwin;
	//curwin = callback.newImageWindow();
	curwin = callback.curHiddenSelectedWindow();

	TriviewControl *triviewControl =  callback.getTriviewControl(curwin);
	
	// passing parameters from triview
	v3d_multithreadimageio_paras *p = (v3d_multithreadimageio_paras *)triviewControl->getCustomStructPointer();
	
	if(!p)
	{
		v3d_msg("The pointer to the parameter-struct is invalid. Do nothing.\n", 0);
		return false;
	}
	
	// image load and visualize streaming
	YY_ImgRead<V3DLONG, float> imglv(filename);
	
	if(imglv.tag_fileformat == V3DSURFACEOBJ)
	{
		v3d_msg("This file is the V3D surface object!\n", 0);
		return false;
	}
	else if(imglv.tag_fileformat == V3DUNKNOWN)
	{
		v3d_msg("This file format is not supported by default! Please try to open it using V3D bio-format plugin!\n", 0);
		return false;
	}
	
	imglv.init();
	
	V3DLONG datatype = imglv.datatype;
	V3DLONG sx = imglv.dims_x, sy = imglv.dims_y, sz = imglv.dims_z, sc = imglv.dims_c;
	
	//	
	qDebug()<<"dims ..."<<sx<<sy<<sz<<sc<<"datatype ..."<<datatype;
	
	// For different datatype
	if (datatype!=1 && datatype!=2 && datatype!=4)
		return false;
	
	size_t start_t = clock();
	
	V3DLONG NNFRAME = 10;
	if(datatype == 1)
	{
		unsigned char *pImage = NULL;
		
		YY_ImgRead<V3DLONG, unsigned char> imglv_u8bit(filename);
		imglv_u8bit.init(pImage);
		imglv_u8bit.setTriviewControl(triviewControl);
		
		if(pImage)
		{
			p4DImage.setData((unsigned char*)pImage, sx, sy, sz, sc, V3D_UINT8); // Set data 
			
			callback.setImage(curwin, &p4DImage);
			callback.setImageName(curwin, filename.c_str());
			callback.updateImageWindow(curwin);
			
			qDebug()<<"loading 8 bit data ... ";
			
			int mytype = -1;
			triviewControl->getTriViewColorDispType(mytype);
			triviewControl->setTriViewColorDispType(mytype);
			
			qDebug()<<"ImageDisplayColorType ..."<<mytype;
			
			while (!imglv_u8bit.image_loaded && !imglv_u8bit.stopped) 
			{
				// update tri-view 
				if(!callback.currentImageWindow())//
				{
					imglv_u8bit.stop();
				}
				else
				{
					v3dhandleList win_list = callback.getImageWindowList();
					
					bool flag_win_exist = false;
					for (int i=0; i<win_list.size(); i++) 
					{
						QString iname = callback.getImageName(win_list[i]);
						
						if(iname.compare(filename.c_str()) == 0)
						{
							flag_win_exist = true;
						}
					}
					if(!flag_win_exist)
					{
						imglv_u8bit.stop();
						continue;
					}
					else
					{
						// load a frame
						imglv_u8bit.start();
						imglv_u8bit.wait();
						
						//callback.updateImageWindow(curwin);
						
						if ( (imglv_u8bit.nFrame-1)%NNFRAME==0  && triviewControl)
						{
							updateTriview(triviewControl, imglv_u8bit.nFrame);
							QCoreApplication::processEvents();
						}
					}
				}
			}
		}
	}
	else if(datatype == 2)
	{
		unsigned short *pImage = NULL;
		
		YY_ImgRead<V3DLONG, unsigned short> imglv_u16bit(filename);
		imglv_u16bit.init(pImage);
		imglv_u16bit.setTriviewControl(triviewControl);
		
		if(pImage)
		{
			p4DImage.setData((unsigned char*)pImage, sx, sy, sz, sc, V3D_UINT16); // Set data 
			
			callback.setImage(curwin, &p4DImage);
			callback.setImageName(curwin, filename.c_str());
			callback.updateImageWindow(curwin);
			
			//
			qDebug()<<"loading 16 bit data ... ";
			while (!imglv_u16bit.image_loaded  && !imglv_u16bit.stopped) 
			{
				// update tri-view 
				if(!callback.currentImageWindow())//
				{
					imglv_u16bit.stop();
				}
				else
				{
					//
					v3dhandleList win_list = callback.getImageWindowList();
					
					bool flag_win_exist = false;
					for (int i=0; i<win_list.size(); i++) 
					{
						QString iname = callback.getImageName(win_list[i]);
						
						if(iname.compare(filename.c_str()) == 0)
						{
							flag_win_exist = true;
						}
					}
					if(!flag_win_exist)
					{
						imglv_u16bit.stop();
						continue;
					}
					else
					{
						// load a frame
						imglv_u16bit.start();
						imglv_u16bit.wait();
						
						//callback.updateImageWindow(curwin);
						
						if ( (imglv_u16bit.nFrame-1)%NNFRAME==0 && triviewControl)
						{
							updateTriview(triviewControl, imglv_u16bit.nFrame);
							QCoreApplication::processEvents();
						}
					}
				}
				//
			}
		}
		qDebug()<<"16-bit data loading success!";
	}
	else //if(datatype == 4)
	{
		float *pImage = NULL;
		
		YY_ImgRead<V3DLONG, float> imglv_float32bit(filename);
		imglv_float32bit.init(pImage);
		imglv_float32bit.setTriviewControl(triviewControl);
		
		if(pImage)
		{
			p4DImage.setData((unsigned char*)pImage, sx, sy, sz, sc, V3D_FLOAT32); // Set data 
			
			callback.setImage(curwin, &p4DImage);
			callback.setImageName(curwin, filename.c_str());
			callback.updateImageWindow(curwin);
			
			//
			qDebug()<<"loading 32 bit data ... ";
			while (!imglv_float32bit.image_loaded && !imglv_float32bit.stopped) 
			{
				// update tri-view 
				if(!callback.currentImageWindow())//
				{
					imglv_float32bit.stop();
				}
				else
				{
					//
					v3dhandleList win_list = callback.getImageWindowList();
					
					bool flag_win_exist = false;
					for (int i=0; i<win_list.size(); i++) 
					{
						QString iname = callback.getImageName(win_list[i]);
						
						if(iname.compare(filename.c_str()) == 0)
						{
							flag_win_exist = true;
						}
					}
					if(!flag_win_exist)
					{
						imglv_float32bit.stop();
						continue;
					}
					else
					{
						// load a frame
						imglv_float32bit.start();
						imglv_float32bit.wait();
						
						//callback.updateImageWindow(curwin);
						
						if ( (imglv_float32bit.nFrame-1)%NNFRAME==0 && triviewControl)
						{
							updateTriview(triviewControl, imglv_float32bit.nFrame);
							QCoreApplication::processEvents();
						}
					}
				}
				
			}
		}
		qDebug()<<"32-bit data loading success!";
	}
	
	if (triviewControl)
		reinitTriviewFocus(triviewControl, sx, sy, sz);

	size_t end_t = clock();
	qDebug()<<"The image stack loading spends"<<(end_t-start_t)/1e6<<"seconds.";
	
	return true;
}

// multi-threaded image io for engine call
// signal-slot communicating with main thread

int mtimgio2(V3DPluginCallback2 &callback, QWidget *parent)
{
	// anchor the image to handle
	QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Find a file"), "/", QObject::tr("Image (*.*)"));
	return mtimgio2(callback, parent, m_FileName);
}

int mtimgio2(V3DPluginCallback2 &callback, QWidget *parent, QString m_FileName)
{
	// anchor the image to handle
	Image4DSimple p4DImage;
	
	v3dhandle curwin;
	//curwin = callback.newImageWindow();
	curwin = callback.curHiddenSelectedWindow();
	
	TriviewControl *triviewControl =  callback.getTriviewControl(curwin);
	
	// passing parameters from triview
	v3d_multithreadimageio_paras *p = (v3d_multithreadimageio_paras *)triviewControl->getCustomStructPointer();
	
	if(!p)
	{
		v3d_msg("The pointer to the parameter-struct is invalid. Do nothing.\n");
		return false;
	}
	
	//QString m_FileName = p->qFilename;
	
	if(m_FileName.isEmpty())
	{
		v3d_msg("This file name is not passed correctly!\n");
		return false;
	}
	
	string filename = m_FileName.toStdString();
	
	// image load and visualize streaming
	YY_ImgRead<V3DLONG, float> imglv(filename);
	
	if(imglv.tag_fileformat == V3DSURFACEOBJ)
	{
		v3d_msg("This file is the V3D surface object!\n");
		return false;
	}
	else if(imglv.tag_fileformat == V3DUNKNOWN)
	{
		v3d_msg("This file format is not supported by default! Please try to open it using V3D bio-format plugin!\n");
		return false;
	}
	
	imglv.init();
	
	V3DLONG datatype = imglv.datatype;
	V3DLONG sx = imglv.dims_x, sy = imglv.dims_y, sz = imglv.dims_z, sc = imglv.dims_c;
	
	//	
	qDebug()<<"dims ..."<<sx<<sy<<sz<<sc<<"datatype ..."<<datatype;
	
	// For different datatype
	if (datatype!=1 && datatype!=2 && datatype!=4)
		return false;
	
	size_t start_t = clock();
	
	bool flag_win_exist;
	V3DLONG NNFRAME = 10;
	if(datatype == 1)
	{
		unsigned char *pImage = NULL;
		
		YY_ImgRead<V3DLONG, unsigned char> imglv_u8bit(filename);
		imglv_u8bit.init(pImage);
		imglv_u8bit.setTriviewControl(triviewControl);
		
		if(pImage)
		{
			p4DImage.setData((unsigned char*)pImage, sx, sy, sz, sc, V3D_UINT8); // Set data 
			
			callback.setImage(curwin, &p4DImage);
			callback.setImageName(curwin, m_FileName);
			callback.updateImageWindow(curwin);
			
			triviewControl->setPreValidZslice(0L); // init
			
			qDebug()<<"loading 8 bit data ... ";
			
//			int mytype = -1;
//			triviewControl->getTriViewColorDispType(mytype);
//			triviewControl->setTriViewColorDispType(mytype);			
//			qDebug()<<"ImageDisplayColorType ..."<<mytype;
			
			while (!imglv_u8bit.image_loaded && !imglv_u8bit.stopped) 
			{
				// update tri-view 
				if(!callback.currentImageWindow())//
				{
					imglv_u8bit.stop();
				}
				else
				{
					v3dhandleList win_list = callback.getImageWindowList();
					
					flag_win_exist = false;
					for (int i=0; i<win_list.size(); i++) 
					{
						QString iname = callback.getImageName(win_list[i]);
						
						if(iname.compare(m_FileName) == 0)
						{
							flag_win_exist = true;
						}
					}
					if(!flag_win_exist)
					{
						imglv_u8bit.stop();
						continue;
					}
					else
					{
						// load a frame
						imglv_u8bit.start();
						imglv_u8bit.wait();
						
						//callback.updateImageWindow(curwin);
						QCoreApplication::processEvents();
					}
				}
			}
		}
		qDebug()<<"8-bit data loading success!";
	}
	else if(datatype == 2)
	{
		unsigned short *pImage = NULL;
		
		YY_ImgRead<V3DLONG, unsigned short> imglv_u16bit(filename);
		imglv_u16bit.init(pImage);
		imglv_u16bit.setTriviewControl(triviewControl);
		
		if(pImage)
		{
			p4DImage.setData((unsigned char*)pImage, sx, sy, sz, sc, V3D_UINT16); // Set data 
			
			callback.setImage(curwin, &p4DImage);
			callback.setImageName(curwin, m_FileName);
			callback.updateImageWindow(curwin);
			
			//
			qDebug()<<"loading 16 bit data ... ";
			while (!imglv_u16bit.image_loaded  && !imglv_u16bit.stopped) 
			{
				
				// update tri-view 
				if(!callback.currentImageWindow())//
				{
					imglv_u16bit.stop();
				}
				else
				{
					//
					v3dhandleList win_list = callback.getImageWindowList();
					
					flag_win_exist = false;
					for (int i=0; i<win_list.size(); i++) 
					{
						QString iname = callback.getImageName(win_list[i]);
						
						if(iname.compare(m_FileName) == 0)
						{
							flag_win_exist = true;
						}
					}
					if(!flag_win_exist)
					{
						imglv_u16bit.stop();
						continue;
					}
					else
					{
						// load a frame
						imglv_u16bit.start();
						imglv_u16bit.wait();
						
						//callback.updateImageWindow(curwin);
						QCoreApplication::processEvents();
					}
				}
			}
		}
		qDebug()<<"16-bit data loading success!";
	}
	else //if(datatype == 4)
	{
		float *pImage = NULL;
		
		YY_ImgRead<V3DLONG, float> imglv_float32bit(filename);
		imglv_float32bit.init(pImage);
		imglv_float32bit.setTriviewControl(triviewControl);
		
		if(pImage)
		{
			p4DImage.setData((unsigned char*)pImage, sx, sy, sz, sc, V3D_FLOAT32); // Set data 
			
			callback.setImage(curwin, &p4DImage);
			callback.setImageName(curwin, m_FileName);
			callback.updateImageWindow(curwin);
			
			//
			qDebug()<<"loading 32 bit data ... ";
			while (!imglv_float32bit.image_loaded && !imglv_float32bit.stopped) 
			{
				// update tri-view 
				if(!callback.currentImageWindow())//
				{
					imglv_float32bit.stop();
				}
				else
				{
					//
					v3dhandleList win_list = callback.getImageWindowList();
					
					flag_win_exist = false;
					for (int i=0; i<win_list.size(); i++) 
					{
						QString iname = callback.getImageName(win_list[i]);
						
						if(iname.compare(m_FileName) == 0)
						{
							flag_win_exist = true;
						}
					}
					if(!flag_win_exist)
					{
						imglv_float32bit.stop();
						continue;
					}
					else
					{
						// load a frame
						imglv_float32bit.start();
						imglv_float32bit.wait();
						
						//callback.updateImageWindow(curwin);
						QCoreApplication::processEvents();
					}
				}
			}
		}
		qDebug()<<"32-bit data loading success!";
	}
	
	if (triviewControl)
		reinitTriviewFocus(triviewControl, sx, sy, sz);
	
	size_t end_t = clock();
	qDebug()<<"The image stack loading spends"<<(end_t-start_t)/1e6<<"seconds.";
	
	
	// exit
	v3dhandleList win_list = callback.getImageWindowList();
	
	flag_win_exist = false;
	for (int i=0; i<win_list.size(); i++) 
	{
		QString iname = callback.getImageName(win_list[i]);
		
		if(iname.compare(m_FileName) == 0)
		{
			flag_win_exist = true;
		}
	}
	
	qDebug()<<"return "<<flag_win_exist;
	return flag_win_exist;
}
