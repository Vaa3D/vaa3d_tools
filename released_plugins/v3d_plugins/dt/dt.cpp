/* dt.cpp
 * distance transform of an image stack
 * revised from Fuhui Long's dt versions
 * 2010-04-16: by Hanchuan Peng
 */

#include "dt.h"
#include "FL_bwdist.h"
#include "v3d_message.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(dt, DtPlugin);

void dtimg(V3DPluginCallback2 &callback, QWidget *parent, int method_code);

//plugin funcs
const QString title = "Distance transform";
QStringList DtPlugin::menulist() const
{
    return QStringList() 
		<< tr("3D")
		<< tr("2D (for all individual Z-sections)");
}

void DtPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("3D"))
    {
    	dtimg(callback, parent, 1); //1 - is the 3D DT
    }
	else if (menu_name == tr("2D (for all individual Z-sections)"))
	{
    	dtimg(callback, parent, 2); //2 - is the within-XY plane DT
	}
}

void dtimg(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
		v3d_msg("You don't have any image open in the main window.");
		return;
	}
	
	if (method_code!=1 && method_code!=2)
	{
		v3d_msg("Invalid DT method code. You should never see this message. Report this bug to the developer");
		return;
	}
		
	DtDialog dialog(callback, parent);
	if (dialog.exec()!=QDialog::Accepted)
		return;

	int ch = dialog.ch; 
	
	int start_t = clock(); // record time point
	
	bool b_use_1stmarker = dialog.b_use_1stmarker;
	bool b_rescale = dialog.b_rescale;
	
	Image4DSimple* subject = callback.getImage(curwin);
	QString m_InputFileName = callback.getImageName(curwin);
	
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	if (subject->getDatatype()!=V3D_UINT8)
	{
		QMessageBox::information(0, title, QObject::tr("This demo program only supports 8-bit data. Your current image data type is not supported."));
		return;
	}
	
	Image4DProxy<Image4DSimple> pSub(subject);
	
	V3DLONG sz0 = subject->getXDim();
    V3DLONG sz1 = subject->getYDim();
    V3DLONG sz2 = subject->getZDim();
	V3DLONG sz3 = subject->getCDim();
	V3DLONG pagesz_sub = sz0*sz1*sz2;
	
	//also get the landmark from the subject
	LandmarkList list_landmark_sub=callback.getLandmark(curwin);
	V3DLONG markerx, markery, markerz; 
	if (b_use_1stmarker)
	{
		if(list_landmark_sub.size()<1)
		{
			b_use_1stmarker = false;
		}
		else
		{
			markerx = V3DLONG(list_landmark_sub.at(0).x)-1; if (markerx<0 || markerx>=sz0) b_use_1stmarker = false;
			markery = V3DLONG(list_landmark_sub.at(0).y)-1; if (markery<0 || markery>=sz1) b_use_1stmarker = false;
			markerz = V3DLONG(list_landmark_sub.at(0).z)-1; if (markerz<0 || markerz>=sz2) b_use_1stmarker = false;
		}
	}
	unsigned char tt = (b_use_1stmarker) ? *(pSub.at(markerx, markery, markerz, ch)) : 0;

	//----------------------------------------------------------------------------------------------------------------------------------
	V3DLONG channelsz = sz0*sz1*sz2;

	V3DLONG *pDist = 0;
	V3DLONG *pLabel = 0;
	unsigned char *pData = 0;
	
	try
	{
		pDist = new V3DLONG [channelsz];
		pLabel = new V3DLONG [channelsz];
		pData = new unsigned char [channelsz]; 
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory in Distance Transform.");
		if (pDist) {delete []pDist; pDist=0;}
		if (pLabel) {delete []pLabel; pLabel=0;}
		if (pData) {delete []pData; pData=0;}
		return;
	}
	
	unsigned char * pSubtmp = pSub.begin();
	for(V3DLONG i = 0; i < channelsz;  i++) 
	{
		pData[i] = (pSubtmp[i]==tt) ? 0 : 1;
	}
	
	// dist transform
	V3DLONG sz_data[4]; sz_data[0]=sz0; sz_data[1]=sz1; sz_data[2]=sz2; sz_data[3]=1;
	if (method_code==1)
		dt3d_binary(pData, pDist, pLabel, sz_data, 1);
	else if (method_code==2)
	{
		V3DLONG pagesz = sz0*sz1;
		for (V3DLONG kk=0;kk<sz2;kk++)
			dt2d_binary(pData+kk*pagesz, pDist+kk*pagesz, pLabel+kk*pagesz, sz_data, 1);
	}
	else
	{
		v3d_msg("Invalid DT method code. You should never see this message. Report this bug to the developer");
		return;
	}
	
	V3DLONG maxtest=0, mintest=INF;
	V3DLONG *p=pDist, *pend=pDist+channelsz;
	for(p=pDist; p<pend; ++p)
	{
		if(*p > maxtest) maxtest = *p;
		else if (*p < mintest) mintest = *p;
	}
	maxtest -= mintest;
	
	if (b_rescale)
	{
		if(maxtest)
		{
			for(V3DLONG i=0; i<channelsz; i++)
				pData[i] = (unsigned char)(255.0*(pDist[i]-mintest)/maxtest);
		}
		else
		{
			for(V3DLONG i=0; i<channelsz; i++)
				pData[i] = 0;
		}
	}
	else
	{
		for(V3DLONG i=0; i<channelsz; i++)
			pData[i] = (unsigned char)(pDist[i]);
	}
		
	if (pDist) {delete []pDist; pDist=0;}
	if (pLabel) {delete []pLabel; pLabel=0;}
	
	//----------------------------------------------------------------------------------------------------------------------------------

	int end_t = clock();
	printf("time eclapse %d s for dist computing!\n", (end_t-start_t)/1000000);
	
	Image4DSimple p4DImage;
	p4DImage.setData((unsigned char*)pData, sz0, sz1, sz2, 1, subject->getDatatype());
	
	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
		newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();
	
	callback.setImage(newwin, &p4DImage);
	callback.setImageName(newwin, QString("distance transformed image"));
	callback.updateImageWindow(newwin);
	
//	setPluginOutputAndDisplayUsingGlobalSetting(pData, sz0, sz1, sz2, sz3, callback);
}
