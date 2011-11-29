/* minMaxfilterplugin.cpp
 * 2009-08-19: create this program by Yang Yu
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>

#include "minMaxfilterplugin.h"

#define INF 1E9

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(minMaxfilter, minMaxFilterPlugin)


QStringList minMaxFilterPlugin::menulist() const
{
    return QStringList() << tr("Max Filter")
						 << tr("min Filter")
						 << tr("Max-min Filter")
						 << tr("min-Max Filter")
						 << tr("about this plugin");
}

void minMaxFilterPlugin::processImage(const QString &arg, Image4DSimple *p4DImage, QWidget *parent)
{
	
	if(arg == tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", 
                QString("min/Max Filter Plugin Demo %1 (2009-Aug-09) developed by Yang Yu. (Peng Lab, Janelia Research Farm Campus, HHMI)")
                .arg(getPluginVersion()));
		return;
	}
	
	
    if (! p4DImage) return;

    unsigned char* data1d = p4DImage->getRawData();
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

	float maxfl = 0, minfl = INF;

    //define datatype here
    //
	
	
	//input
	bool ok1, ok2, ok3, ok4;
	unsigned int Wx=1, Wy=1, Wz=1, c=1;
	
	Wx = QInputDialog::getInteger(parent, tr("Window X "),
											   tr("Enter radius (window size is 2*radius+1):"),
											   3, 1, N, 1, &ok1);
	
	if(ok1)
	{
		Wy = QInputDialog::getInteger(parent, tr("Window Y"),
											   tr("Enter radius (window size is 2*radius+1):"),
											   3, 1, M, 1, &ok2);
	}
	else
		return;
	
	if(ok2)
	{
		Wz = QInputDialog::getInteger(parent, tr("Window Z"),
											   tr("Enter radius (window size is 2*radius+1):"),
											   3, 1, P, 1, &ok3);
	}
	else
		return;
	
	if(sc==1)
	{
		c=1;
		ok4=true;
	}
	else
	{
		if(ok3)
		{
			c = QInputDialog::getInteger(parent, tr("Channel"),
												  tr("Enter channel NO:"),
												  1, 1, sc, 1, &ok4);
		}
		else
			return;
	}

	//choosing filter
	unsigned int filterflag = 0; //Gaussian 0 Max 1 min 2 Max-min 3 min-Max 4
	
	if(arg == tr("Max Filter"))
		filterflag = 1;
	else if(arg == tr("min Filter"))
		filterflag = 2;
	else if(arg == tr("Max-min Filter"))
		filterflag = 3;
	else if(arg == tr("min-Max Filter"))
		filterflag = 4;
	
    //filtering   
	V3DLONG offsetc = (c-1)*pagesz;

	if (ok4 && (filterflag == 1 || filterflag == 2 || filterflag == 3 ||filterflag == 4) )
	{
		//declare temporary pointer
		unsigned char *pImage = new unsigned char [pagesz];
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

		//Filtering
		//
		//min Max filtering
		for(V3DLONG iz = 0; iz < P; iz++)
		{
			V3DLONG offsetk = iz*M*N;
			for(V3DLONG iy = 0; iy < M; iy++)
			{
				V3DLONG offsetj = iy*N;
				for(V3DLONG ix = 0; ix < N; ix++)
				{
					maxfl = 0; minfl = INF;
					
					V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
					V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
					V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
					V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
					V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
					V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;
					
					for(V3DLONG k=zb; k<=ze; k++)
					{
						V3DLONG offsetkl = k*M*N;
						for(V3DLONG j=yb; j<=ye; j++)
						{
							V3DLONG offsetjl = j*N;
							for(V3DLONG i=xb; i<=xe; i++)
							{
								V3DLONG dataval = data1d[ offsetc + offsetkl + offsetjl + i];
								
								if(maxfl<dataval) maxfl = dataval;
								if(minfl>dataval) minfl = dataval;
							}
						}
					}
					
					//set value
					V3DLONG index_pim = offsetk + offsetj + ix;
					
					if(filterflag == 1 || filterflag == 3)
					{
						pImage[index_pim] = maxfl;
					}
					else if(filterflag == 2 || filterflag == 4)
					{
						pImage[index_pim] = minfl;
					}
					
				}
			}
		}
		

		//Max-min || min-Max
		if(ok4 && (filterflag == 3 || filterflag ==4))
		{
			
			//   Filtering
			for(V3DLONG iz = 0; iz < P; iz++)
			{
				V3DLONG offsetk = iz*M*N;
				for(V3DLONG iy = 0; iy < M; iy++)
				{
					V3DLONG offsetj = iy*N;
					for(V3DLONG ix = 0; ix < N; ix++)
					{
						maxfl = 0; minfl = INF;
						
						V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
						V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
						V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
						V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
						V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
						V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;
						
						for(V3DLONG k=zb; k<=ze; k++)
						{
							V3DLONG offsetkl = k*M*N;
							for(V3DLONG j=yb; j<=ye; j++)
							{
								V3DLONG offsetjl = j*N;
								for(V3DLONG i=xb; i<=xe; i++)
								{
									V3DLONG dataval = data1d[ offsetc + offsetkl + offsetjl + i];
									
									if(maxfl<dataval) maxfl = dataval;
									if(minfl>dataval) minfl = dataval;
								}
							}
						}
						
						//set value
						V3DLONG index_pim = offsetk + offsetj + ix;
						
						if(filterflag == 4)
						{
							pImage[index_pim] = maxfl;
						}
						else if(filterflag == 3)
						{
							pImage[index_pim] = minfl;
						}
						
					}
				}
			}
		}

		//rescaling for display
		for(V3DLONG k=0; k<P; k++)
		{
			V3DLONG offsetk = k*M*N;
			for(V3DLONG j=0; j<M; j++)
			{
				V3DLONG offsetj = j*N;
				for(V3DLONG i=0; i<N; i++)
				{
					V3DLONG indLoop = offsetk + offsetj + i;

					data1d[offsetc + indLoop] = pImage[indLoop];
				}
			}
		}

		//de-alloc
		if (pImage) {delete []pImage; pImage=0;}
	}
}




