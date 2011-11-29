/* bioFormat_ex.cpp
 * 2010-03-31: adding series image supporting, updated by Yang Yu
 * 2010-03-22: adding imseqconverter function, updated by Yang Yu
 * 2010-01-05: updated by Yang Yu 
 * 2009-09-20: created by Yang Yu
 */

#include <QtGui>

#include <exception>
#include <iostream>
#include <algorithm>

#include "bioFormat_ex.h"

#if defined (_MSC_VER)
#include "../../basic_c_fun/stackutil.h"
#include <fstream>
using std::ifstream;

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <sstream>
using std::istringstream;

#include <vector>
using std::vector;

// agent image class
class IMAGE 
{
public:
	IMAGE(V3DLONG sizeX, V3DLONG sizeY, V3DLONG sizeZ, V3DLONG sizeC, V3DLONG sizeT, ImagePixelType v3dComponentType, string SeriesName)
	{
		//init
		data1d_8 = 0;
		data1d_16 = 0;
		data1d_32 = 0;
		
		//
		sx = sizeX;
		sy = sizeY;
		sz = sizeZ;
		sc = sizeC;
		st = sizeT;
		v3dDataType = v3dComponentType;
		seriesname = SeriesName;
		
		V3DLONG N = sizeX*sizeY*sizeZ*sizeC*sizeT; //
		
		if(v3dComponentType == V3D_UINT8)
		{
			try
			{
				data1d_8 = new unsigned char[N];
			}
			catch (...) 
			{
				printf("Fail to allocate memory.\n");
				return;
			}
		}
		else if (v3dComponentType == V3D_UINT16)
		{
			try
			{
				data1d_16 = new short int [N];
			}
			catch (...) 
			{
				printf("Fail to allocate memory.\n");
				return;
			}
		}
		else if (v3dComponentType == V3D_FLOAT32)
		{
			try
			{
				data1d_32 = new float [N];
			}
			catch (...) 
			{
				printf("Fail to allocate memory.\n");
				return;
			}
		}
		
		
	}
	~IMAGE(){}
	
public:
	void clear()
	{
		if(data1d_8) {delete []data1d_8; data1d_8=0;}
		if(data1d_16) {delete []data1d_16; data1d_16=0;}
		if(data1d_32) {delete []data1d_32; data1d_32=0;}
	}
	
public:
	string seriesname;
	
	ImagePixelType v3dDataType;
	unsigned char* data1d_8;
	short int* data1d_16;
	float* data1d_32;
	
	V3DLONG sx, sy, sz, sc, st;
};

#else
#include "imageIO_bioformat.h" 
#include <ctime>
#endif


//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(bioFormat_ex, BioFormat_ExPlugin)

int imloader(V3DPluginCallback &callback, QWidget *parent);
int imseqconverter(V3DPluginCallback &callback, QWidget *parent);
int imloader_w(V3DPluginCallback &callback, QWidget *parent); //windows

#if defined (_MSC_VER)
#else
int bioimreader(string filename, vector<IMAGE> &pImgList);
int bioimwriter(string filename_output, Image4DSimple *p4DImage);
#endif

QStringList importSeriesFileList_addnumbersort(const QString & individualFileName); // for imseqconverter func

//for mutex JavaVM
int flag_JavaVM=0;

const QString title = "BioFormats Demo";
QStringList BioFormat_ExPlugin::menulist() const
{
    return QStringList() << tr("Read Image (w/o file buffer)")
						 << tr("Read Image (with file buffer)")
						 << tr("Convert Images2Tifs")
						 << tr("about this plugin");
}

void BioFormat_ExPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("Read Image (w/o file buffer)"))
	{
    	imloader(callback, parent);
    }
	else if (menu_name == tr("Read Image (with file buffer)")) //added by PHC, 2010-06-01
	{
		//use tmp file to get around the conflict of QtCore and JVM for Windows - MSVC
		imloader_w(callback, parent);
	}
	else if (menu_name == tr("Convert Images2Tifs"))
    {
    	imseqconverter(callback, parent);
    }
	else if (menu_name == tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", 
								 "ImageReader Plugin Demo 1.0 (2009-Sep-21) developed by Yang Yu. (Peng Lab, Janelia Research Farm Campus, HHMI) Acknowlegement: Using Bio-Format ImageReader Class");
	}
	
}

// Open a series of inputs
QStringList importSeriesFileList_addnumbersort(const QString & individualFileName)
{
	QStringList myList;
	myList.clear();
	
	// get the image files namelist in the directory
	
	QFileInfo fileInfo(individualFileName);
	QString curFilePath = fileInfo.path();
	QString curSuffix = fileInfo.suffix();
	
	QDir dir(curFilePath);
	if (!dir.exists())
	{
		qWarning("Cannot find the directory");
		return myList;
	}
	
	QStringList imgfilters;
	imgfilters.append("*." + curSuffix);
	foreach (QString file, dir.entryList(imgfilters, QDir::Files, QDir::Name))
	{
		myList += QFileInfo(dir, file).absoluteFilePath();
	}
	
	// print filenames
	foreach (QString qs, myList)  qDebug() << qs;
	
	return myList;
}

#if defined (_MSC_VER)

int imloader(V3DPluginCallback &callback, QWidget *parent)
{
	QMessageBox::information(parent, "Warning", "Not supported on Windows systems");
	return true;
}

int imseqconverter(V3DPluginCallback &callback, QWidget *parent)
{
	QMessageBox::information(parent, "Warning", "Not supported on Windows systems");
	return true;
}

int imloader_w(V3DPluginCallback &callback, QWidget *parent)
{
	//get filename
	QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open An Image Using Bio-Format"),
													"/Documents",
													QObject::tr("Images (*.tif *.lsm *.bmp *.jpg *.png *.gif *.ics *.ids *.lei *.pic *.raw *.mdb *.img *.hdr *.dv *.r3d *.fits *.dcm *.dicom *.jp2 *.mrc *.pgm *.zvi *.dm3 *.seq *.ipl *.mng *.lif *.ome *.2 *.3 *.4 *.ome.tiff);;XML files(*.xml);;All(*)"));
	
	QFileInfo fileInfo(m_FileName);
	//QString curFilePath = fileInfo.path();
	QString curSuffix = fileInfo.suffix();
	
	//readimage		
	vector<IMAGE> pImgList;

	string filename = m_FileName.toStdString(); 
	std::cout << "now reading " << filename << " ..." << endl;
	
	try
	{
		//conventions here
		//system call "imageIO_bioformat" generate temporary nameslist 'names_tmp_images.txt'
		if ( QFile::exists(m_FileName) )
		{
			QString cmd_imread = QString("imageIO_bioformat %1").arg(m_FileName);
			system(qPrintable(cmd_imread));
		}
		else
		{
			return false;
		}

		//load images
		ifstream pFile("names_tmp_images.txt");
		string str;
		char buf[2000];
		string fn_str;

		if(pFile.is_open())
		{
			while( !pFile.eof() )
			{
				while( getline(pFile, str) )
				{
					istringstream iss(str);

					iss >> buf;
					fn_str = buf;
					
					qDebug() << "loading ..." << QString(fn_str.c_str());

					//loading
					unsigned char * img_subject = 0;  
					V3DLONG * sz_subject = 0; 
					int datatype = 0;
					
					if (loadImage(const_cast<char *>(fn_str.c_str()), img_subject, sz_subject, datatype)!=true)
					{
						fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", fn_str.c_str());
						return false;
					}

					ImagePixelType v3dComponentType;
					if(datatype==1)
					{
						v3dComponentType = V3D_UINT8;
					}
					else if(datatype==2)
					{
						v3dComponentType = V3D_UINT16;
					}
					else if(datatype==4)
					{
						v3dComponentType = V3D_FLOAT32;
					}

					IMAGE pImg(sz_subject[0], sz_subject[1], sz_subject[2], sz_subject[3], 1, v3dComponentType, fn_str);

					V3DLONG N = sz_subject[0]*sz_subject[1]*sz_subject[2]*sz_subject[3];
					
					unsigned char* data1d_8 = 0;
					short int* data1d_16 = 0;
					float* data1d_32 = 0;

					if(v3dComponentType == V3D_UINT8)
					{
						data1d_8 = pImg.data1d_8;

						for(V3DLONG i=0; i<N; i++)
						{
							*(data1d_8++) = *(img_subject++);
						}
					}
					else if (v3dComponentType == V3D_UINT16)
					{
						data1d_16 = pImg.data1d_16;

						for(V3DLONG i=0; i<N; i++)
						{
							*(data1d_16++) = *(img_subject++);
						}
					}
					else if (v3dComponentType == V3D_FLOAT32)
					{
						data1d_32 = pImg.data1d_32;

						for(V3DLONG i=0; i<N; i++)
						{
							*(data1d_32++) = *(img_subject++);
						}
					}

					pImgList.push_back(pImg);

					//de-alloc
					//if(img_subject) {delete img_subject; img_subject=0;}

					// del temp files
					size_t found = fn_str.find_first_of("/");
					while(found!=string::npos)
					{
						fn_str[found]='\\';
						found = fn_str.find_first_of("/", found+1);
					}
										
					QString tmp_name(fn_str.c_str());

					QString cmd_rm = QString("del %1").arg(tmp_name);
					system(qPrintable(cmd_rm));

				}
			}
		}
		else
		{
			cout << "Unable to open the file"<<endl;
			return false;
		}
		pFile.close();

		// del temp files
		QString cmd_deltxt = QString("del names_tmp_images.txt");
		system(qPrintable(cmd_deltxt));

	}
	catch(...)
	{
		printf("Fail to implement bioimage reading function!\n");
		return -1;
	}
	
	v3dhandle *newwin = new v3dhandle [pImgList.size()];
	for(int i=0; i<pImgList.size(); i++)
	{
		
		IMAGE pImg = pImgList.at(i);
		
		Image4DSimple p4DImage;
		
		if(pImg.v3dDataType == V3D_UINT8)
			p4DImage.setData((unsigned char*)pImg.data1d_8, pImg.sx, pImg.sy, pImg.sz*pImg.st, pImg.sc, pImg.v3dDataType);
		else if (pImg.v3dDataType == V3D_UINT16)
			p4DImage.setData((unsigned char*)pImg.data1d_16, pImg.sx, pImg.sy, pImg.sz*pImg.st, pImg.sc, pImg.v3dDataType);
		else if (pImg.v3dDataType == V3D_FLOAT32)
			p4DImage.setData((unsigned char*)pImg.data1d_32, pImg.sx, pImg.sy, pImg.sz*pImg.st, pImg.sc, pImg.v3dDataType);
		
		//
		newwin[i] = callback.newImageWindow();
		callback.setImage(newwin[i], &p4DImage);
		callback.setImageName(newwin[i], m_FileName+QString("_series_0%1.tif").arg(i+1)); //
		callback.updateImageWindow(newwin[i]);
		
	}

	return true;

}


#else


// bioimage read
int bioimreader(string filename,  vector<IMAGE> &pImgList)
{
	//readimage
	try 
	{ 
		// load JaveVM
		if(flag_JavaVM==0)
		{
			createJVM();

			flag_JavaVM++;	
		}
		else
		{
			jace::helper::getJavaVM();
			
			flag_JavaVM++;
		}
		
		std::cout<<"flag ..." <<flag_JavaVM <<endl;	

		ImageReader* imageReader = new ImageReader; // read all image formats
		ChannelFiller* channelFiller = new ChannelFiller(*imageReader); // expand 8-bit indexed color images to full RGB
		ChannelSeparator* reader = new ChannelSeparator(*channelFiller); // split interleaved RGB channels into separate image planes
		//FileStitcher* fileStitcher = new FileStitcher(channelSeparator);	
		
		// attach OME metadata object
		IMetadata omeMeta = MetadataTools::createOMEXMLMetadata();
		reader->setMetadataStore(omeMeta);
		
		bool isType = reader->isThisType(filename);
		qDebug("Is this kind of image recognized by Bio-Format: %s \n",(isType)?"Yes":"No");
		
		reader->setId(filename);
		
		String id = imageReader->getFormat(filename);
		//std::cout<< "Format is "<< id<<endl;
		
		int seriesCount = reader->getSeriesCount();
		std::cout << "How many series: " << seriesCount << endl;
		
		MetadataStore ms = reader->getMetadataStore();
		MetadataRetrieve mr = MetadataTools::asRetrieve(ms);
		
		// for all series
		for (int js=0; js<seriesCount; js++) 
		{
			reader->setSeries(js);
			
			std::cout << "Series #" << js;
			if (js < mr.getImageCount()) {
				std::cout << " -- " << mr.getImageName(js) << endl;
			}
			
			int pixelType = reader->getPixelType();
			std::cout << "pixelType is " << pixelType << " -- ";
			
			ImagePixelType v3dComponentType;
			
			if (pixelType == FormatTools::UINT8())
			{
				v3dComponentType = V3D_UINT8;
				std::cout << "UINT8" << endl;
			}
			else if (pixelType == FormatTools::INT8())
			{
				v3dComponentType = V3D_UINT8;
				std::cout << "INT8" << endl;
			}
			else if (pixelType == FormatTools::UINT16())
			{
				v3dComponentType = V3D_UINT16;
				std::cout << "UINT16" << endl;
			}
			else if (pixelType == FormatTools::INT16())
			{
				v3dComponentType = V3D_UINT16;
				std::cout << "INT16" << endl;
			}
			else if (pixelType == FormatTools::UINT32())
			{
				v3dComponentType = V3D_FLOAT32;
				std::cout << "UINT32" << endl;
			}
			else if (pixelType == FormatTools::INT32())
			{
				v3dComponentType = V3D_FLOAT32;
				std::cout << "INT32" << endl;
			}
			else if (pixelType == FormatTools::FLOAT())
			{
				v3dComponentType = V3D_FLOAT32;
				std::cout << "FLOAT" << endl;
			}
			else if (pixelType == FormatTools::DOUBLE())
			{
				v3dComponentType = V3D_FLOAT32;
				std::cout << "DOUBLE" << endl;
			}
			else
				return -1;
			
			// get pixel resolution and dimensional extents
			//change datatype to V3DLONG, 2010-06-01, by PHC
			V3DLONG sizeX = reader->getSizeX();
			V3DLONG sizeY = reader->getSizeY();
			V3DLONG sizeZ = reader->getSizeZ();
			V3DLONG sizeC = reader->getSizeC();
			V3DLONG sizeT = reader->getSizeT();
			V3DLONG effSizeC = reader->getEffectiveSizeC(); // always equals sizeC (because of ChannelSeparator)
			V3DLONG rgbChannelCount = reader->getRGBChannelCount(); // always 1 (because of ChannelSeparator)
			V3DLONG imageCount = reader->getImageCount(); // imageCount = sizeZ*sizeT*effSizeC;
			V3DLONG bpp = FormatTools::getBytesPerPixel(pixelType);
			
			bool little = reader->isLittleEndian(); // for 16-bit data Endianness (intel little, motorola big)
			qDebug("Endianness = %s", (little ? "intel (little)" : "motorola (big)"));
			
			qDebug("sizeX %d sizeY %d sizeZ %d sizeC %d sizeT %d effSizeC %d rgbChannelCount %d imageCount %d bpp %d \n", sizeX, sizeY, sizeZ, sizeC, sizeT, effSizeC, rgbChannelCount, imageCount, bpp);
			
			// size of image
			V3DLONG N = sizeX*sizeY*sizeZ*sizeC*sizeT*bpp; //
			unsigned char* data1d_8 = 0;
			short int* data1d_16 = 0;
			float* data1d_32 = 0;
			
			IMAGE pImg(sizeX, sizeY, sizeZ, sizeC, sizeT, v3dComponentType, mr.getImageName(js));
			
			if(v3dComponentType == V3D_UINT8)
			{
				data1d_8 = pImg.data1d_8;
				
				//data1d_8 = new unsigned char[N/bpp];
				qDebug("V3D_UINT8");
			}
			else if (v3dComponentType == V3D_UINT16)
			{
				data1d_16 = pImg.data1d_16;
				
				//data1d_16 = new V3DLONG[N/bpp];
				qDebug("V3D_UINT16");
			}
			else if (v3dComponentType == V3D_FLOAT32)
			{
				data1d_32 = pImg.data1d_32;
				
				//data1d_32 = new float[N/bpp];
				qDebug("V3D_FLOAT32");
			}
			
			// check image info
			//change the following data types from int to V3DLONG, by PHC, 100601
			V3DLONG pIndex = 0, pCount = imageCount;
			V3DLONG bytesPerPlane = sizeX * sizeY * bpp; // * rgbChannelCount;
			
			V3DLONG pagesz = sizeX * sizeY * bpp;
			V3DLONG channelsz = sizeX * sizeY * sizeZ*sizeT * bpp;
			
			jbyte* jData;
			
			if(v3dComponentType == V3D_UINT8)
				jData = (jbyte*) data1d_8;
			else if (v3dComponentType == V3D_UINT16)
				jData = (jbyte*) data1d_16;
			else if (v3dComponentType == V3D_FLOAT32)
				jData = (jbyte*) data1d_32;
			
			ByteArray buf(bytesPerPlane); // pre-allocate buffer
			
			jbyte* p = jData;
			for (V3DLONG no=pIndex; no<pCount; no++)
			{
				jData=p;
				for(V3DLONG c=0; c<sizeC; c++)
				{
					reader->openBytes(no++, buf, 0, 0, sizeX, sizeY);
					
					// copy raw byte array
					JNIEnv* env = jace::helper::attach();
					jbyteArray jArray = static_cast<jbyteArray>(buf.getJavaJniArray());
					env->GetByteArrayRegion(jArray, 0, bytesPerPlane, jData);
					jData += channelsz;
				}
				no--;
				p += bytesPerPlane;
			}
			
			pImgList.push_back(pImg);
		
		}
		// de-alloc
		imageReader->close();
		delete imageReader;
		delete channelFiller;
		delete reader;
		
		return 0;
		
	}
	catch (FormatException& fe) 
	{
		fe.printStackTrace();
		return -2;
	}
	catch (IOException& ioe) 
	{
		ioe.printStackTrace();
		return -3;
	}
	catch (JNIException& jniException)
	{
		std::cout << "An unexpected JNI error occurred. " << jniException.what() << endl;
		return -4;
	}
	catch (std::exception& e) 
	{
		std::cout << "An unexpected C++ error occurred. " << e.what() << endl;
		return -5;
	}
	
}

int bioimwriter(string filename_output, Image4DSimple *p4DImage)
{

	try
	{ 
		// load JavaVM
		jace::helper::getJavaVM();
		
		// get image info
		V3DLONG sx = p4DImage->getXDim();
		V3DLONG sy = p4DImage->getYDim();
		V3DLONG sz = p4DImage->getZDim(); 
		V3DLONG sc = p4DImage->getCDim();
		
		ImagePixelType v3dComponentType = p4DImage->getDatatype();
		
		// option
		if (v3dComponentType!=V3D_UINT8)
		{
			QMessageBox::information(0, title, QObject::tr("This demo program only supports 8-bit data. Your current image data type is not supported."));
			return -1;
		}
		
		unsigned char* data1d_8 = 0;
		V3DLONG* data1d_16 = 0;
		float* data1d_32 = 0;
		
		//datatype
		int pixelType; 
		
		if (v3dComponentType == V3D_UINT8) // 1
		{
			pixelType = FormatTools::UINT8();
			data1d_8 = p4DImage->getRawData();
			printf("UINT8");
		}
		else if (v3dComponentType == V3D_UINT16) // 2
		{
			pixelType = FormatTools::UINT16();
			//data1d_16 = p4DImage->getRawData();
			printf("UINT16");
		}
		else if (v3dComponentType == V3D_FLOAT32) // 4
		{
			pixelType = FormatTools::FLOAT();
			//data1d_32 = = p4DImage->getRawData();
			printf("FLOAT");
		}
		else
			return -1;
		
		// set pixel resolution and dimensional extents
		V3DLONG sizeX = sx;
		V3DLONG sizeY = sy;
		V3DLONG sizeZ = sz;
		V3DLONG sizeC = sc; 
		V3DLONG sizeT = 1;
		V3DLONG rgbChannelCount = sc; 
		V3DLONG effSizeC = sizeC / rgbChannelCount;
		V3DLONG imageCount = effSizeC * sizeZ * sizeT;
		V3DLONG bpp = FormatTools::getBytesPerPixel(pixelType);
		
		// check image info
		V3DLONG pIndex = 0, pCount = imageCount;
		V3DLONG bytesPerPlane = sizeX * sizeY * bpp * rgbChannelCount;
		
		// save to movie
		bool last=false;
		
		V3DLONG pagesz = sizeX * sizeY * bpp;
		V3DLONG channelsz = sizeX * sizeY * imageCount * bpp;
		
		// attach OME metadata object
		IMetadata meta = MetadataTools::createOMEXMLMetadata();
		meta.createRoot();
		meta.setPixelsBigEndian(Boolean(1), 0, 0);
		meta.setPixelsDimensionOrder("XYZCT", 0, 0);
		meta.setPixelsPixelType(FormatTools::getPixelTypeString(pixelType), 0, 0);
		meta.setPixelsSizeX(Integer(sizeX), 0, 0);
		meta.setPixelsSizeY(Integer(sizeY), 0, 0);
		meta.setPixelsSizeZ(Integer(sizeZ), 0, 0);
		meta.setPixelsSizeC(Integer(sizeC), 0, 0);
		meta.setPixelsSizeT(Integer(sizeT), 0, 0);
		meta.setLogicalChannelSamplesPerPixel(Integer(rgbChannelCount), 0, 0);
		
		ImageWriter *writer = new ImageWriter;
		//writer->staticGetJavaJniClass();
		writer->setMetadataRetrieve(meta);
		writer->setId(filename_output);	
		
		ByteArray buf(bytesPerPlane);
		
		unsigned char *pBuf = new unsigned char [bytesPerPlane];
		if (!pBuf) 
		{
			printf("Fail to allocate memory.\n");
			return -1;
		}		
		
		jbyte* jData = (jbyte*) pBuf;
		for (V3DLONG no=pIndex; no<pCount; no++)
		{
			if(no==(pCount-1)) last=true;
			
			int mid_t = clock();
			
			for(V3DLONG c=0; c<rgbChannelCount; c++)
			{
				V3DLONG offsets = no*pagesz;
				V3DLONG offsets_buf = c*pagesz;
				
				offsets += c*channelsz;
				
				unsigned char *pDATA = data1d_8 + offsets;
				unsigned char *pPTMP = pBuf + offsets_buf;
				
				memcpy(pPTMP, pDATA, pagesz);
			}
			
			// copy raw byte array
			JNIEnv* env = jace::helper::attach();
			jbyteArray jArray = static_cast<jbyteArray>(buf.getJavaJniArray());
			env->SetByteArrayRegion(jArray, 0, bytesPerPlane, jData);
			
			writer->saveBytes(buf, last);
			
			int end_t = clock();
			//printf("No. %d of total %d planes, saving consumes %lf seconds per plane \n",no, pCount, double(end_t-mid_t)/1000000.0);
		}
		
		//de-alloc
		writer->close();
		delete writer;		
		
		return 0;
	}
	catch (FormatException& fe) {
		fe.printStackTrace();
		return -1;
	}
	catch ( IOException& ioe ) 
	{
		ioe.printStackTrace();
		return -2;
	}
	catch ( JNIException& jniException ) 
	{
		std::cout << "An unexpected JNI error occured. " << jniException.what() << endl;
		return -3;
	}
	catch ( std::exception& e ) 
	{
		std::cout << "An unexpected C++ error occured. " << e.what() << endl;
		return -4;
	}

}

// load a single image into V3D
int imloader(V3DPluginCallback &callback, QWidget *parent)
{
	//get filename
	QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open An Image Using Bio-Format"),
													"/Documents",
													QObject::tr("Images (*.tif *.lsm *.bmp *.jpg *.png *.gif *.ics *.ids *.lei *.pic *.raw *.mdb *.img *.hdr *.dv *.r3d *.fits *.dcm *.dicom *.jp2 *.mrc *.pgm *.zvi *.dm3 *.seq *.ipl *.mng *.lif *.ome *.2 *.3 *.4 *.ome.tiff);;XML files(*.xml);;All(*)"));
	
	string filename =m_FileName.toStdString(); 
	
	QFileInfo fileInfo(m_FileName);
	//QString curFilePath = fileInfo.path();
	QString curSuffix = fileInfo.suffix();
	
	//readimage		
	vector<IMAGE> pImgList;

	std::cout << "now reading " << filename << " ..." << endl;
	
	try
	{
		bioimreader(filename, pImgList);
	}
	catch(...)
	{
		printf("Fail to implement bioimage reading function!\n");
		return -1;
	}

	
	v3dhandle *newwin = new v3dhandle [pImgList.size()];
	for(int i=0; i<pImgList.size(); i++)
	{
		
		IMAGE pImg = pImgList.at(i);
		
		Image4DSimple p4DImage;
		
		if(pImg.v3dDataType == V3D_UINT8)
			p4DImage.setData((unsigned char*)pImg.data1d_8, pImg.sx, pImg.sy, pImg.sz*pImg.st, pImg.sc, pImg.v3dDataType);
		else if (pImg.v3dDataType == V3D_UINT16)
			p4DImage.setData((unsigned char*)pImg.data1d_16, pImg.sx, pImg.sy, pImg.sz*pImg.st, pImg.sc, pImg.v3dDataType);
		else if (pImg.v3dDataType == V3D_FLOAT32)
			p4DImage.setData((unsigned char*)pImg.data1d_32, pImg.sx, pImg.sy, pImg.sz*pImg.st, pImg.sc, pImg.v3dDataType);
		
		//
		newwin[i] = callback.newImageWindow();
		callback.setImage(newwin[i], &p4DImage);
		callback.setImageName(newwin[i], m_FileName+QString("_series_0%1.tif").arg(i+1)); //
		callback.updateImageWindow(newwin[i]);
		
	}

	return true;

}

// image sequence converter
int imseqconverter(V3DPluginCallback &callback, QWidget *parent)
{
	//get filename
	QString m_InputFileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open An Image of Sequences"),
													  "/Documents",
													 QObject::tr("Images (*.tif *.lsm *.bmp *.jpg *.png *.gif *.ics *.ids *.lei *.pic *.raw *.mdb *.img *.hdr *.dv *.r3d *.fits *.dcm *.dicom *.jp2 *.mrc *.pgm *.zvi *.dm3 *.seq *.ipl *.mng *.lif *.ome *.2 *.3 *.4 *.ome.tiff);;XML files(*.xml);;All(*)"));
	
	QStringList imgList = importSeriesFileList_addnumbersort(m_InputFileName);

	//V3DLONG count=0;
	foreach (QString img_str, imgList)
	{
		//qDebug("count %ld", count);
		
		//readimage		
		vector<IMAGE> pImgList;
		
		string filename_input =img_str.toStdString(); 
		
		try
		{
			bioimreader(filename_input, pImgList);
		}
		catch(...)
		{
			printf("Fail to implement bioimage reading function!\n");
			return -1;
		}
		
		// save output
		for(int i=0; i<pImgList.size(); i++)
		{
			IMAGE pImg = pImgList.at(i);
			
			Image4DSimple p4DImage;
			
			if(pImg.v3dDataType == V3D_UINT8)
				p4DImage.setData((unsigned char*)pImg.data1d_8, pImg.sx, pImg.sy, pImg.sz*pImg.st, pImg.sc, pImg.v3dDataType);
			else if (pImg.v3dDataType == V3D_UINT16)
				p4DImage.setData((unsigned char*)pImg.data1d_16, pImg.sx, pImg.sy, pImg.sz*pImg.st, pImg.sc, pImg.v3dDataType);
			else if (pImg.v3dDataType == V3D_FLOAT32)
				p4DImage.setData((unsigned char*)pImg.data1d_32, pImg.sx, pImg.sy, pImg.sz*pImg.st, pImg.sc, pImg.v3dDataType);
			
			QString img_tmp = img_str;
			img_tmp.chop(4);
			string filename_output = (img_tmp+QString("_series_0%1.tif").arg(i+1)).toStdString();
			
			try
			{
				bioimwriter(filename_output, &p4DImage);
			}
			catch(...)
			{
				printf("Fail to implement bioimage reading function!\n");
				return -1;
			}
			
			p4DImage.cleanExistData();
			
		}
		
		//count++;
	}
	
}

int imloader_w(V3DPluginCallback &callback, QWidget *parent)
{
	QMessageBox::information(parent, "Warning", "Only supported on Windows systems");
	return true;
}

#endif