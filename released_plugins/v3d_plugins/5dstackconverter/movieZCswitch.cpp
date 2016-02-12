/* movieZCswitch.cpp
 * 2009-09-22: create this program by Yang Yu
 * 2009-09-28. Last edit by Hanchuan Peng. only change the texts in the options
 * 2010-08-06. by hanchuan Peng, to adapt to the patch of the image4dsimple data structure
 * 2011-01-27. by Yang Yu, to support multiple datatype V3D_UINT8, V3D_UINT16, V3D_FLOAT32
 */
 
#include <QtGui>

#include <string>
#include <exception>
#include <iostream>
#include <algorithm>
#include <math.h>


#include "movieZCswitch.h"

using namespace std;


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(movieZCswitch, MovieZCswitchPlugin)

int changeMS(V3DPluginCallback2 &callback, QWidget *parent);
bool changeMS(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);



const QString title = "5D Stack Converter";
QStringList MovieZCswitchPlugin::menulist() const
{
    return QStringList() << tr("5D Stack Converter")
						 << tr("about this plugin");
}

QStringList MovieZCswitchPlugin::funclist() const
{
    return QStringList()
    <<tr("convert")
    <<tr("help");
}

void MovieZCswitchPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("5D Stack Converter"))
    {
    	changeMS(callback, parent);
    }
	else if (menu_name == tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", 
            QString("5D Stack Converter Plugin Demo %1 (2009-Sep-22) developed by Yang Yu. (Hanchuan Peng Lab, Janelia Research Farm Campus, HHMI)"
            ).arg(getPluginVersion()));
	}
	
}

bool MovieZCswitchPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{
    if (func_name==tr("convert"))
    {
        return changeMS(callback, input, output);
    }

    else if (func_name==tr("help"))
    {
        cout<<"Usage : v3d -x 5D_Stack_Converter -f convert -i <inimg_file> -o <outimg_file> -p <dr> <tp>"<<endl;
        cout<<endl;
        cout<<"dr          converter direction, 0: 4D {XYZ,Color} --> 5D {XYZ,Color,Time}, 1: 5D {XYZ,Color,Time}--> 4D {XYZ,Color}, default 0"<<endl;
        cout<<"tp          timepoints, default 1"<<endl;
        cout<<endl;
        return true;
    }
    return false;
}


int changeMS(V3DPluginCallback2 &callback, QWidget *parent)
{

	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* image = callback.getImage(oldwin);
	if (! image)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}
	
	ImagePixelType imgdatatype = image->getDatatype();
	
	V3DLONG N = image->getTotalBytes();
	
	Image4DSimple p4DImage;
	
	unsigned char* image1d = image->getRawData();
	V3DLONG sx=image->getXDim(), sy=image->getYDim(), sz=image->getZDim(), sc=image->getCDim();
	
	V3DLONG ts_max = (sz>sc)?sz:sc;
	
	//get Z->C or C->Z command
	bool ok1, ok2;
	int timepoints=0;

	QStringList items;
	items << QObject::tr("4D {XYZ,Color} Stack --> 5D {XYZ,Color,Time} Stack") << QObject::tr("5D {XYZ,Color,Time} Stack --> 4D {XYZ,Color} Stack");

	QString item = QInputDialog::getItem(parent, QObject::tr("Change Movie Stack"),
										QObject::tr("Which direction do you want to change:"), items, 0, false, &ok1);
	
	
	if(ok1)
	{
		timepoints = QInputDialog::getInteger(parent, QObject::tr("Set time points"),
									  QObject::tr("Enter the number of time points:"),
									  1, 1, ts_max, 1, &ok2);
	}
	else
	{
		return -1;
	}
	
	void *pResult = NULL;
	
	//Get the old image info
	if(!(QString::compare(item, "4D {XYZ,Color} Stack --> 5D {XYZ,Color,Time} Stack")))
	{
		p4DImage.setTimePackType(TIME_PACK_C);

//		V3DLONG imagecount = image->getTDim();
		
		V3DLONG pagesz=sx*sy;
		V3DLONG channelsz=sx*sy*sz;
		
		p4DImage.setTDim(timepoints);
		V3DLONG imagecount = timepoints;
		
		sz /= imagecount;
		
		if(imgdatatype == V3D_UINT8)
		{
			// init
			unsigned char *data1d = NULL;
			
			try
			{
				data1d = new unsigned char [N];
				
				memset(data1d, 0, sizeof(unsigned char)*N);
			}
			catch (...) 
			{
				printf("Fail to allocate memory.\n");
				return -1;
			}
			
			// assign
			unsigned char *pImg = (unsigned char *)image1d;
			for(V3DLONG no=0; no<imagecount; no++)
			{
				for(V3DLONG c=0; c<sc; c++)
				{
					for(V3DLONG k =0; k<sz; k++)
					{
						V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
						V3DLONG offsets = k*pagesz + c*channelsz + no*pagesz*sz;
						for(V3DLONG i=0; i<pagesz; i++)
						{
							data1d[i+offsetc] = pImg[i + offsets];
						}
					}
				}
			}
			
			sc = sc*imagecount;
			
			pResult = data1d; //
		}
		else if(imgdatatype == V3D_UINT16)
		{
			// init
			unsigned short *data1d = NULL;
			
			try
			{
				data1d = new unsigned short [N];
				
				memset(data1d, 0, sizeof(unsigned short)*N);
			}
			catch (...) 
			{
				printf("Fail to allocate memory.\n");
				return -1;
			}
			
			// assign
			unsigned short *pImg = (unsigned short *)image1d;
			for(V3DLONG no=0; no<imagecount; no++)
			{
				for(V3DLONG c=0; c<sc; c++)
				{
					for(V3DLONG k =0; k<sz; k++)
					{
						V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
						V3DLONG offsets = k*pagesz + c*channelsz + no*pagesz*sz;
						for(V3DLONG i=0; i<pagesz; i++)
						{
							data1d[i+offsetc] = pImg[i + offsets];
						}
					}
				}
			}
			
			sc = sc*imagecount;
			
			pResult = data1d; //
		}
		else if(imgdatatype == V3D_FLOAT32)
		{
			// init
			float *data1d = NULL;
			
			try
			{
				data1d = new float [N];
				
				memset(data1d, 0, sizeof(float)*N);
			}
			catch (...) 
			{
				printf("Fail to allocate memory.\n");
				return -1;
			}
			
			// assign
			float *pImg = (float *)image1d;
			for(V3DLONG no=0; no<imagecount; no++)
			{
				for(V3DLONG c=0; c<sc; c++)
				{
					for(V3DLONG k =0; k<sz; k++)
					{
						V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
						V3DLONG offsets = k*pagesz + c*channelsz + no*pagesz*sz;
						for(V3DLONG i=0; i<pagesz; i++)
						{
							data1d[i+offsetc] = pImg[i + offsets];
						}
					}
				}
			}
			
			sc = sc*imagecount;
			
			pResult = data1d; //
		}
		else
		{
			printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
			return -1;
		}
	}
	else if(!(QString::compare(item, "5D {XYZ,Color,Time} Stack --> 4D {XYZ,Color} Stack")))
	{
		p4DImage.setTimePackType(TIME_PACK_Z);
		
		V3DLONG pagesz=sx*sy;
//		V3DLONG channelsz=sx*sy*sz;
//		V3DLONG imagecount = image->getTDim();
		
		V3DLONG imagecount = timepoints;
		
		if(imagecount>sc)
		{
			QMessageBox::information(0, title, QObject::tr("# time points should not be greater than # color channel."));
			return -1;
		}

		sc /= imagecount;

		if(imgdatatype == V3D_UINT8)
		{
			// init
			unsigned char *data1d = NULL;
			
			try
			{
				data1d = new unsigned char [N];
				
				memset(data1d, 0, sizeof(unsigned char)*N);
			}
			catch (...) 
			{
				printf("Fail to allocate memory.\n");
				return -1;
			}
			
			// assign
			unsigned char *pImg = (unsigned char *)image1d;
			for(V3DLONG no=0; no<imagecount; no++)
			{
				for(V3DLONG c=0; c<sc; c++)
				{
					for(V3DLONG k =0; k<sz; k++)
					{
						V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
						V3DLONG offsets = k*pagesz + c*pagesz*sz*imagecount + no*pagesz*sz;
						for(V3DLONG i=0; i<pagesz; i++)
						{
							data1d[i+offsets] = pImg[i + offsetc];
						}
					}
				}
			}
			
			sz = sz*imagecount;
			
			pResult = data1d; //
		}
		else if(imgdatatype == V3D_UINT16)
		{
			// init
			unsigned short *data1d = NULL;
			
			try
			{
				data1d = new unsigned short [N];
				
				memset(data1d, 0, sizeof(unsigned short)*N);
			}
			catch (...) 
			{
				printf("Fail to allocate memory.\n");
				return -1;
			}
			
			// assign
			unsigned short *pImg = (unsigned short *)image1d;
			for(V3DLONG no=0; no<imagecount; no++)
			{
				for(V3DLONG c=0; c<sc; c++)
				{
					for(V3DLONG k =0; k<sz; k++)
					{
						V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
						V3DLONG offsets = k*pagesz + c*pagesz*sz*imagecount + no*pagesz*sz;
						for(V3DLONG i=0; i<pagesz; i++)
						{
							data1d[i+offsets] = pImg[i + offsetc];
						}
					}
				}
			}
			
			sz = sz*imagecount;
			
			pResult = data1d; //
		}
		else if(imgdatatype == V3D_FLOAT32)
		{
			// init
			float *data1d = NULL;
			
			try
			{
				data1d = new float [N];
				
				memset(data1d, 0, sizeof(float)*N);
			}
			catch (...) 
			{
				printf("Fail to allocate memory.\n");
				return -1;
			}
			
			// assign
			float *pImg = (float *)image1d;
			for(V3DLONG no=0; no<imagecount; no++)
			{
				for(V3DLONG c=0; c<sc; c++)
				{
					for(V3DLONG k =0; k<sz; k++)
					{
						V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
						V3DLONG offsets = k*pagesz + c*pagesz*sz*imagecount + no*pagesz*sz;
						for(V3DLONG i=0; i<pagesz; i++)
						{
							data1d[i+offsets] = pImg[i + offsetc];
						}
					}
				}
			}
			
			sz = sz*imagecount;
			
			pResult = data1d; //
		}
		else
		{
			printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
			return -1;
		}

	}
	else
	{
		QMessageBox::information(0, title, QObject::tr("This program only supports time series data. Your current image data type is not supported."));
		return -3;
	}

	// show in v3d
	p4DImage.setData((unsigned char*)pResult, sx,sy,sz,sc, imgdatatype);
	
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &p4DImage);
	callback.setImageName(newwin,  callback.getImageName(oldwin)+"_changed");
	callback.updateImageWindow(newwin);

	return 0;
}


bool changeMS(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"Welcome to 5D Stack Converter plugin"<<endl;
    if (output.size() != 1) return false;
    unsigned int direction = 0,timepoints = 1;
    double th = 0;
    if (input.size()>=2)
    {
        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) direction = atoi(paras.at(0));
        if(paras.size() >= 2) timepoints = atoi(paras.at(1));
    }


    if(direction < 0 || direction > 1)
    {
        cerr<<"Invalid direction, error!"<<endl;
        return false;
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

    cout<<"direction = "<<direction<<endl;
    cout<<"timepoints = "<<timepoints<<endl;
    cout<<"inimg_file = "<<inimg_file<<endl;
    cout<<"outimg_file = "<<outimg_file<<endl;

    unsigned char * image1d = 0;
    V3DLONG in_sz[4];

    int datatype;
    if(!simple_loadimage_wrapper(callback, inimg_file, image1d, in_sz, datatype))
    {
        cerr<<"load image "<<inimg_file<<" error!"<<endl;
        return false;
    }

    V3DLONG sx=in_sz[0], sy = in_sz[1], sz=in_sz[2], sc=in_sz[3];
    V3DLONG ts_max = (sz>sc)?sz:sc;
    V3DLONG N = in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];

    if(timepoints < 1 || timepoints > ts_max)
    {
        cerr<<"Invalid timepoints, error!"<<endl;
        return false;
    }

    QStringList items;
    items << QObject::tr("4D {XYZ,Color} Stack --> 5D {XYZ,Color,Time} Stack") << QObject::tr("5D {XYZ,Color,Time} Stack --> 4D {XYZ,Color} Stack");
    QString item = items.at(direction);


    void *pResult = NULL;

    Image4DSimple p4DImage;
    //Get the old image info
    if(!(QString::compare(item, "4D {XYZ,Color} Stack --> 5D {XYZ,Color,Time} Stack")))
    {
        p4DImage.setTimePackType(TIME_PACK_C);
        V3DLONG pagesz=sx*sy;
        V3DLONG channelsz=sx*sy*sz;

        p4DImage.setTDim(timepoints);
        V3DLONG imagecount = timepoints;

        sz /= imagecount;

        if(datatype == 1)
        {
            // init
            unsigned char *data1d = NULL;

            try
            {
                data1d = new unsigned char [N];

                memset(data1d, 0, sizeof(unsigned char)*N);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }

            // assign
            unsigned char *pImg = (unsigned char *)image1d;
            for(V3DLONG no=0; no<imagecount; no++)
            {
                for(V3DLONG c=0; c<sc; c++)
                {
                    for(V3DLONG k =0; k<sz; k++)
                    {
                        V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
                        V3DLONG offsets = k*pagesz + c*channelsz + no*pagesz*sz;
                        for(V3DLONG i=0; i<pagesz; i++)
                        {
                            data1d[i+offsetc] = pImg[i + offsets];
                        }
                    }
                }
            }

            sc = sc*imagecount;

            pResult = data1d; //
        }
        else if(datatype == 2)
        {
            // init
            unsigned short *data1d = NULL;

            try
            {
                data1d = new unsigned short [N];

                memset(data1d, 0, sizeof(unsigned short)*N);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }

            // assign
            unsigned short *pImg = (unsigned short *)image1d;
            for(V3DLONG no=0; no<imagecount; no++)
            {
                for(V3DLONG c=0; c<sc; c++)
                {
                    for(V3DLONG k =0; k<sz; k++)
                    {
                        V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
                        V3DLONG offsets = k*pagesz + c*channelsz + no*pagesz*sz;
                        for(V3DLONG i=0; i<pagesz; i++)
                        {
                            data1d[i+offsetc] = pImg[i + offsets];
                        }
                    }
                }
            }

            sc = sc*imagecount;

            pResult = data1d; //
        }
        else if(datatype == 4)
        {
            // init
            float *data1d = NULL;

            try
            {
                data1d = new float [N];

                memset(data1d, 0, sizeof(float)*N);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }

            // assign
            float *pImg = (float *)image1d;
            for(V3DLONG no=0; no<imagecount; no++)
            {
                for(V3DLONG c=0; c<sc; c++)
                {
                    for(V3DLONG k =0; k<sz; k++)
                    {
                        V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
                        V3DLONG offsets = k*pagesz + c*channelsz + no*pagesz*sz;
                        for(V3DLONG i=0; i<pagesz; i++)
                        {
                            data1d[i+offsetc] = pImg[i + offsets];
                        }
                    }
                }
            }

            sc = sc*imagecount;

            pResult = data1d; //
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }
    }
    else if(!(QString::compare(item, "5D {XYZ,Color,Time} Stack --> 4D {XYZ,Color} Stack")))
    {
        p4DImage.setTimePackType(TIME_PACK_Z);
        V3DLONG pagesz=sx*sy;
        V3DLONG imagecount = timepoints;

        if(imagecount>sc)
        {
            QMessageBox::information(0, title, QObject::tr("# time points should not be greater than # color channel."),0);
            return -1;
        }

        sc /= imagecount;

        if(datatype == 1)
        {
            // init
            unsigned char *data1d = NULL;

            try
            {
                data1d = new unsigned char [N];

                memset(data1d, 0, sizeof(unsigned char)*N);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }

            // assign
            unsigned char *pImg = (unsigned char *)image1d;
            for(V3DLONG no=0; no<imagecount; no++)
            {
                for(V3DLONG c=0; c<sc; c++)
                {
                    for(V3DLONG k =0; k<sz; k++)
                    {
                        V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
                        V3DLONG offsets = k*pagesz + c*pagesz*sz*imagecount + no*pagesz*sz;
                        for(V3DLONG i=0; i<pagesz; i++)
                        {
                            data1d[i+offsets] = pImg[i + offsetc];
                        }
                    }
                }
            }

            sz = sz*imagecount;

            pResult = data1d; //
        }
        else if(datatype == 2)
        {
            // init
            unsigned short *data1d = NULL;

            try
            {
                data1d = new unsigned short [N];

                memset(data1d, 0, sizeof(unsigned short)*N);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return false;
            }

            // assign
            unsigned short *pImg = (unsigned short *)image1d;
            for(V3DLONG no=0; no<imagecount; no++)
            {
                for(V3DLONG c=0; c<sc; c++)
                {
                    for(V3DLONG k =0; k<sz; k++)
                    {
                        V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
                        V3DLONG offsets = k*pagesz + c*pagesz*sz*imagecount + no*pagesz*sz;
                        for(V3DLONG i=0; i<pagesz; i++)
                        {
                            data1d[i+offsets] = pImg[i + offsetc];
                        }
                    }
                }
            }

            sz = sz*imagecount;

            pResult = data1d; //
        }
        else if(datatype == 4)
        {
            // init
            float *data1d = NULL;

            try
            {
                data1d = new float [N];

                memset(data1d, 0, sizeof(float)*N);
            }
            catch (...)
            {
                printf("Fail to allocate memory.\n");
                return -1;
            }

            // assign
            float *pImg = (float *)image1d;
            for(V3DLONG no=0; no<imagecount; no++)
            {
                for(V3DLONG c=0; c<sc; c++)
                {
                    for(V3DLONG k =0; k<sz; k++)
                    {
                        V3DLONG offsetc = k*pagesz + c*pagesz*sz + no*pagesz*sz*sc;
                        V3DLONG offsets = k*pagesz + c*pagesz*sz*imagecount + no*pagesz*sz;
                        for(V3DLONG i=0; i<pagesz; i++)
                        {
                            data1d[i+offsets] = pImg[i + offsetc];
                        }
                    }
                }
            }

            sz = sz*imagecount;

            pResult = data1d; //
        }
        else
        {
            printf("Currently this program only support UINT8, UINT16, and FLOAT32 datatype.\n");
            return false;
        }

    }
    else
    {
        QMessageBox::information(0, title, QObject::tr("This program only supports time series data. Your current image data type is not supported."),0);
        return -3;
    }

    simple_saveimage_wrapper(callback, outimg_file, (unsigned char *)pResult, in_sz, datatype);
    return true;
}
