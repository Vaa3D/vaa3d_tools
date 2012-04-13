/*
 *  montage_image_sections .cpp
 *  montage_image_sections
 *
 *  Created by Yang, Jinzhu and Hanchuan Peng, on 11/22/10.
 *
 */

// Adapted and upgraded to V3DPluginInterface2_1 by Jianlong Zhou, 2012-04-05
// add dofunc() by Jianlong Zhou, 2012-04-12


#include "montage_image_sections.h"
#include "v3d_message.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(Montage, MONTAGEPlugin);

bool do_computation(const V3DPluginArgList & input, V3DPluginArgList & output);

template <class T>
void montage_image_sections (T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer,bool b_draw)
{
	V3DLONG i, j,k,n,count,m,row,column;

	V3DLONG mCount = iImageHeight * iImageWidth;

	column = (V3DLONG)(sqrt((double)iImageLayer)+0.5);
	//v3d_msg("1");
	//printf("column1=%d,",column);

	V3DLONG remainder = iImageLayer - column*column;
	if (remainder == 0)
	{
		row = column;
	}else
	{
		row = column+1;
	}

	for (i=0; i<iImageLayer; i++)
	{
		for (j=0; j<iImageHeight; j++)
		{
			for (k=0; k<iImageWidth; k++)
			{
				if (i < column)
				{
					aspOutput[(j)* column *iImageWidth + (k)  + (i * iImageWidth) ]= apsInput[i * mCount + j*iImageWidth + k];

				}else
				{
					aspOutput[(((i/column)*iImageHeight)+ j )* column* iImageWidth + k + ((i%column) * iImageWidth)]= apsInput[i * mCount + j*iImageWidth + k];
				}
			}
		}
	}
	if(b_draw)
	{
		for (j=0; j<row*iImageHeight; j=j+iImageHeight)
		{
			for( k=0; k<column*iImageWidth; k++)
			{
				aspOutput[j*column*iImageWidth+k] = 255;
			}
		}
		for (j=0; j<row*iImageHeight; j++)
		{
			for(k=0; k<column*iImageWidth; k= k+iImageWidth)
			{
				aspOutput[j*column*iImageWidth+k] = 255;
			}
		}

	}else
		return;

	//v3d_msg("2");
}
template <class T>
void montage_image_stack(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer,V3DLONG column,V3DLONG row, V3DLONG slice,V3DLONG n)
{
	V3DLONG i, j,k;
	V3DLONG mCount = iImageHeight * iImageWidth;
	V3DLONG mCount1 = column * row;
	for (i=0; i<slice; i++)
	{
		for (j=0; j<row; j++)
		{
			for (k=0; k<column; k++)
			{
				aspOutput[i * mCount1 + j*column + k] = 0;

			}
		}
	}
	//V3DLONG w = 7;
	for (j=0; j<iImageHeight; j++)
	{
		for (k=0; k<iImageWidth; k++)
		{
			 aspOutput[((j/row)*(n)+(k/column))*mCount1 + (j%row)*column + (k%column)] = apsInput[j*iImageWidth+k];

		}
	}
}


void do_computation(V3DPluginCallback2 &callback, QWidget *parent, int method_code);

//plugin funcs
const QString title = "montage_image_sections";
QStringList MONTAGEPlugin::menulist() const
{
    return QStringList()
	<< tr("montage_image_sections")
	<< tr ("revert a montage image to a stack")
	<< tr("Help");
}

void MONTAGEPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("montage_image_sections"))
	{
    	do_computation(callback, parent,1 );
    }else if (menu_name == tr("revert a montage image to a stack"))
	{
		do_computation(callback, parent,2);

	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("This plugin produces a montage view of all z slices.");
		return;
	}

}



QStringList MONTAGEPlugin::funclist() const
{
	return QStringList()
		<<tr("mtg")
          <<tr("rmtg")
		<<tr("help");
}


bool MONTAGEPlugin::dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
{
     if (func_name == tr("mtg"))
	{
		return do_computation(input, output);
	}
     else if (func_name == tr("rmtg"))
	{
		return do_computation(input, output);
	}
	else if(func_name == tr("help"))
	{
		cout<<"Usage : v3d -x montage -f mtg -i <inimg_file> -o <outimg_file> -p <wx> <wy> <wz> <ch> <filterflag>"<<endl;
		cout<<endl;
		cout<<"wx          filter window radius size (pixel #) in x direction, window size is 2*wx+1, default 3"<<endl;
		cout<<"wy          filter window radius size (pixel #) in y direction, window size is 2*wy+1, default 3"<<endl;
		cout<<"wz          filter window radius size (pixel #) in z direction, window size is 2*wz+1, default 3"<<endl;
		cout<<"ch           the input channel value, default 1 and start from 1, default 1"<<endl;
		cout<<"filterflag  filter method flag, 1: Max Filter, 2: min Filter, 3: Max-min Filter, 4: min-Max Filter, default 1"<<endl;
		cout<<endl;
		cout<<"e.g. v3d -x montage -f mtg -i input.raw -o output.raw -p 3 3 3 1 1"<<endl;
		cout<<endl;
		return true;
	}
}


bool do_computation(const V3DPluginArgList & input, V3DPluginArgList & output)
{
     return true;
}



void do_computation(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
	v3dhandle curwin = callback.currentImageWindow();
	V3DLONG h;
	V3DLONG d;
	V3DLONG channelsz,channelsz1;
	V3DLONG column,row;
	channelsz =channelsz1=column = row = 0;
	if (!curwin)
	{
		v3d_msg("You don't have any image open in the main window.");
		return;
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
	bool b_draw;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to draw line?"), QMessageBox::Yes, QMessageBox::No))
	{
		b_draw = true;
	}else
	{
		b_draw = false;

	}
	//----------------------------------------------------------------------------------------------------------------------------------

	if(method_code == 1)
	{

		column = (V3DLONG)(sqrt((double)sz2)+ 0.5);
		V3DLONG remainder= sz2 - column*column;
		if (remainder == 0)
		{
			row = column;
			channelsz1 = (sz0)*(sz1)*sz2;
		}else
		{
			row = column+1;
			channelsz1 = (sz0)*(sz1)*column*row;
		}
		channelsz = sz0*sz1*sz2;
		//  printf("column=%d,remainder=%d,row=%d sz3=%d\n",column,remainder,row,sz3);
		void *pData=NULL;
		V3DLONG sz_data[4]; sz_data[0]=sz0; sz_data[1]=sz1; sz_data[2]=sz2; sz_data[3]=1;
		switch (subject->getDatatype())
		{
			case V3D_UINT8:

				try
			{
				pData = (void *)(new unsigned char [sz3*channelsz1]());
			}
				catch (...)
			{
				v3d_msg("Fail to allocate memory in data combination.");
				if (pData) {delete []pData; pData=0;}
				return;
			}

			{
				unsigned char * pSubtmp_uint8 = pSub.begin();

				for (V3DLONG ich=0; ich<sz3; ich++)
					montage_image_sections(pSubtmp_uint8+ich*channelsz, (unsigned char *)pData+ich*channelsz1, sz0, sz1, sz2,b_draw);
			}
				break;

			case V3D_UINT16:
				try
			{
				pData = (void *)(new short int [sz3*channelsz1]());
			}
				catch (...)
			{
				v3d_msg("Fail to allocate memory in data combination.");
				if (pData) {delete []pData; pData=0;}
				return;
			}

			{
				short int * pSubtmp_uint16 = (short int *)pSub.begin();

				for (V3DLONG ich=0; ich<sz3; ich++)
					montage_image_sections(pSubtmp_uint16+ich*channelsz, (short int *)pData+ich*channelsz1, sz0, sz1, sz2,b_draw );
			}
				break;
			case V3D_FLOAT32:

				try
			{
				pData = (void *)(new float [sz3*channelsz1]());
			}
				catch (...)
			{
				v3d_msg("Fail to allocate memory in data combination.");
				if (pData) {delete []pData; pData=0;}
				return;
			}

			{
				float * pSubtmp_float32 = (float *)pSub.begin();

				for (V3DLONG ich=0; ich<sz3; ich++)
					montage_image_sections(pSubtmp_float32+ich*channelsz, (float *)pData+ich*channelsz, sz0, sz1, sz2,b_draw );
			}

				break;
			default:
				break;
		}
		int end_t = clock();
		//	printf("time eclapse %d s for dist computing!\n", (end_t-start_t)/1000000);
		///v3d_msg("2");
		//printf("column=%d,remainder=%d,row=%d\n",column,remainder,row);
		Image4DSimple p4DImage;

		p4DImage.setData((unsigned char*)pData, (V3DLONG)column*sz0, (V3DLONG)row*sz1, 1, sz3, subject->getDatatype());
		//p4DImage.setData((unsigned char*)pData, (V3DLONG)column*(sz0), (V3DLONG)row*(sz1), 1, sz3, subject->getDatatype());
		//	printf("sz0= %d sz1=%d sz2=%d dd=%d vv=%d\n", sz0,sz1,sz2,dd,vv);
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();

		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, QString("montage_image_sections"));
		callback.updateImageWindow(newwin);
	}else if (method_code ==2)
	{
		if (sz2 >1)
		{
			QMessageBox::information(0, title, QObject::tr("The image must be a one slice."));
			return;
		}
		V3DLONG c,r;
		SetsizeDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;
		else
		{
			c = dialog.coord_y->text().toLong();
			r = dialog.coord_x->text().toLong();
		}
		//	  column = floor(sz0/n);
		//	  row = column;
		column = floor((double)(sz0/c));
		row = floor((double)(sz1/r));

		//  V3DLONG scount = n*(n+1);
		V3DLONG scount = r*c;
		channelsz = sz0*sz1*sz2;
		channelsz1 = c*r*column*row;
		// channelsz1 = scount*column*row;
		//  printf("column=%d,remainder=%d,row=%d sz3=%d\n",column,remainder,row,sz3);
		void *pData=NULL;
		V3DLONG sz_data[4]; sz_data[0]=sz0; sz_data[1]=sz1; sz_data[2]=sz2; sz_data[3]=1;
		switch (subject->getDatatype())
		{
			case V3D_UINT8:
				try
			{
				pData = (void *)(new unsigned char [sz3*channelsz1]());
			}
				catch (...)
			{
				v3d_msg("Fail to allocate memory in data combination.");
				if (pData) {delete []pData; pData=0;}
				return;
			}

			{
				unsigned char * pSubtmp_uint8 = pSub.begin();

				for (V3DLONG ich=0; ich<sz3; ich++)
					montage_image_stack(pSubtmp_uint8+ich*channelsz, (unsigned char *)pData+ich*channelsz1, sz0, sz1, sz2,column,row,scount,c);
			}
				break;

			case V3D_UINT16:
				try
			{
				pData = (void *)(new short int [sz3*channelsz1]());
			}
				catch (...)
			{
				v3d_msg("Fail to allocate memory in data combination.");
				if (pData) {delete []pData; pData=0;}
				return;
			}

			{
				short int * pSubtmp_uint16 = (short int *)pSub.begin();

				for (V3DLONG ich=0; ich<sz3; ich++)
					montage_image_stack(pSubtmp_uint16+ich*channelsz, (short int *)pData+ich*channelsz1,  sz0, sz1, sz2,column,row,scount,c);
			}
				break;
			case V3D_FLOAT32:

				try
			{
				pData = (void *)(new float [sz3*channelsz1]());
			}
				catch (...)
			{
				v3d_msg("Fail to allocate memory in data combination.");
				if (pData) {delete []pData; pData=0;}
				return;
			}

			{
				float * pSubtmp_float32 = (float *)pSub.begin();

				for (V3DLONG ich=0; ich<sz3; ich++)
					montage_image_stack(pSubtmp_float32+ich*channelsz, (float *)pData+ich*channelsz, sz0, sz1, sz2,column,row,scount,c);
			}

				break;
			default:
				break;
		}
		int end_t = clock();
		//	printf("time eclapse %d s for dist computing!\n", (end_t-start_t)/1000000);
		///v3d_msg("2");
		//printf("column=%d,remainder=%d,row=%d\n",column,remainder,row);
		Image4DSimple p4DImage;

		p4DImage.setData((unsigned char*)pData, (V3DLONG)column, (V3DLONG)row, scount, sz3, subject->getDatatype());
		//p4DImage.setData((unsigned char*)pData, (V3DLONG)column*(sz0), (V3DLONG)row*(sz1), 1, sz3, subject->getDatatype());
		//	printf("sz0= %d sz1=%d sz2=%d dd=%d vv=%d\n", sz0,sz1,sz2,dd,vv);
		v3dhandle newwin;
		if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
			newwin = callback.currentImageWindow();
		else
			newwin = callback.newImageWindow();

		callback.setImage(newwin, &p4DImage);
		callback.setImageName(newwin, QString("revert a montage image to a stack"));
		callback.updateImageWindow(newwin);

	}
	//V3DLONG width= column*sz0;
	//V3DLONG Height = row*sz1;

	//----------------------------------------------------------------------------------------------------------------------------------

}
void SetsizeDialog::update()
{
	//get current data

	NX = coord_x->text().toLong()-1;
	NY = coord_y->text().toLong()-1;
	//NZ = coord_z->text().toLong()-1;


}
