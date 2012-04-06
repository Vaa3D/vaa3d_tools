/*
 *  ada_threshold.cpp
 *
 *  Created by Yang, Jinzhu and Hanchuan Peng, on 11/22/10.
 *
 */

#include "ada_threshold.h"
#include "v3d_message.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(threshold, ThPlugin);

template <class T>
void BinaryProcess(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d)
{
	V3DLONG i, j,k,n,count;
	double t, temp;

	V3DLONG mCount = iImageHeight * iImageWidth;
	for (i=0; i<iImageLayer; i++)
	{
		for (j=0; j<iImageHeight; j++)
		{
			for (k=0; k<iImageWidth; k++)
			{
				V3DLONG curpos = i * mCount + j*iImageWidth + k;
				V3DLONG curpos1 = i* mCount + j*iImageWidth;
				V3DLONG curpos2 = j* iImageWidth + k;
				temp = 0;
				count = 0;
				for(n =1 ; n <= d  ;n++)
				{
					if (k>h*n) {temp += apsInput[curpos1 + k-(h*n)]; count++;}
					if (k+(h*n)< iImageWidth) { temp += apsInput[curpos1 + k+(h*n)]; count++;}
                    if (j>h*n) {temp += apsInput[i* mCount + (j-(h*n))*iImageWidth + k]; count++;}//
					if (j+(h*n)<iImageHeight) {temp += apsInput[i* mCount + (j+(h*n))*iImageWidth + k]; count++;}//
					if (i>(h*n)) {temp += apsInput[(i-(h*n))* mCount + curpos2]; count++;}//
					if (i+(h*n)< iImageLayer) {temp += apsInput[(i+(h*n))* mCount + j* iImageWidth + k ]; count++;}
				}
				t =  apsInput[curpos]-temp/(count);
				aspOutput[curpos]= (t > 0)? t : 0;
			}
		}
	}
}

void thimg(V3DPluginCallback2 &callback, QWidget *parent, int method_code);

//plugin funcs
const QString title = "adaptive threshold transform";
QStringList ThPlugin::menulist() const
{
     return QStringList()
          << tr("3D (w/o parameters)")
          << tr("3D (set parameters)")
          << tr("Help");
}

void ThPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("3D (w/o parameters)"))
	{
          thimg(callback, parent,1 );
     }
	else if (menu_name == tr("3D (set parameters)"))
	{
		thimg(callback, parent, 2 );
	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("Simple adaptive thresholding: for each voxel, compute a threshold which is the average intensity of its neighboring voxels and then subtract the threshold from the current voxel's intensity value. If the result is <0, then set it as 0. The neighborhood is defined along 6 axial directions in 3D, with N samples of each direction (N -- the 'number of sampling points' in the parameter setting dialog), and M voxels between every nearest pair of samples (M -- the 'sampling interval' in the parameter setting dialog).");
		return;
	}

}

void thimg(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
	v3dhandle curwin = callback.currentImageWindow();
	V3DLONG h;
	V3DLONG d;
	if (!curwin)
	{
		v3d_msg("You don't have any image open in the main window.");
		return;
	}

	if (method_code == 1)
	{
		h = 5;
		d = 3;
	}
	else
	{
		if( method_code == 2)
		{
			AdaTDialog dialog(callback, parent);
			if (dialog.exec()!=QDialog::Accepted)
			return;
			else
			{
				h = dialog.Ddistance->text().toLong()-1;
				d = dialog.Dnumber->text().toLong()-1;
				printf("d% h,d% d \n ",h,d);
			}
		}
	}

	int start_t = clock(); // record time point

	Image4DSimple* subject = callback.getImage(curwin);
	QString m_InputFileName = callback.getImageName(curwin);

	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	Image4DProxy<Image4DSimple> pSub(subject);

	V3DLONG sz0 = subject->getXDim();
    V3DLONG sz1 = subject->getYDim();
    V3DLONG sz2 = subject->getZDim();
	V3DLONG sz3 = subject->getCDim();
	V3DLONG pagesz_sub = sz0*sz1*sz2;

	//----------------------------------------------------------------------------------------------------------------------------------
	V3DLONG channelsz = sz0*sz1*sz2;
	void *pData=NULL;

	V3DLONG sz_data[4]; sz_data[0]=sz0; sz_data[1]=sz1; sz_data[2]=sz2; sz_data[3]=1;
		switch (subject->getDatatype())
		{
			case V3D_UINT8:
				try
				{
					pData = (void *)(new unsigned char [sz3*channelsz]);
				}
					catch (...)
				{
					v3d_msg("Fail to allocate memory in Distance Transform.");
					if (pData) {delete []pData; pData=0;}
					return;
				}

				{
					unsigned char * pSubtmp_uint8 = pSub.begin();

					for (V3DLONG ich=0; ich<sz3; ich++)
						BinaryProcess(pSubtmp_uint8+ich*channelsz, (unsigned char *)pData+ich*channelsz, sz0, sz1, sz2, h, d  );
				}
				break;

			case V3D_UINT16:
				try
				{
					pData = (void *)(new short int [sz3*channelsz]);
				}
					catch (...)
				{
					v3d_msg("Fail to allocate memory in Distance Transform.");
					if (pData) {delete []pData; pData=0;}
					return;
				}

				{
					short int * pSubtmp_uint16 = (short int *)pSub.begin();

					for (V3DLONG ich=0; ich<sz3; ich++)
						BinaryProcess(pSubtmp_uint16+ich*channelsz, (short int *)pData+ich*channelsz, sz0, sz1, sz2, h, d );
				}

				break;

			case V3D_FLOAT32:
				try
				{
					pData = (void *)(new float [sz3*channelsz]);
				}
					catch (...)
				{
					v3d_msg("Fail to allocate memory in Distance Transform.");
					if (pData) {delete []pData; pData=0;}
					return;
				}

				{
					float * pSubtmp_float32 = (float *)pSub.begin();

					for (V3DLONG ich=0; ich<sz3; ich++)
						BinaryProcess(pSubtmp_float32+ich*channelsz, (float *)pData+ich*channelsz, sz0, sz1, sz2, h, d );
				}

				break;

			default:
				break;
		}

	//----------------------------------------------------------------------------------------------------------------------------------

	int end_t = clock();
	printf("time eclapse %d s for dist computing!\n", (end_t-start_t)/1000000);

	Image4DSimple p4DImage;
	p4DImage.setData((unsigned char*)pData, sz0, sz1, sz2, sz3, subject->getDatatype());

	v3dhandle newwin;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
		newwin = callback.currentImageWindow();
	else
		newwin = callback.newImageWindow();

	callback.setImage(newwin, &p4DImage);
	callback.setImageName(newwin, QString("thresholded image"));
	callback.updateImageWindow(newwin);
}

void AdaTDialog::update()
{
	//get current data
	Dn = Dnumber->text().toLong()-1;
	Dh = Ddistance->text().toLong()-1;
		//printf("channel %ld val %d x %ld y %ld z %ld ind %ld \n", c, data1d[ind], nx, ny, nz, ind);
}
