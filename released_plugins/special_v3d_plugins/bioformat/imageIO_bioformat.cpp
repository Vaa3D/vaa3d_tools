/* imageIO_bioformat.cpp
 * 2010-06-01: create this program by Yang Yu
 */

#include "imageIO_bioformat.h"

// .raw or .tif IO
#include "../../basic_c_fun/stackutil.h"

//
void printHelp();
void printHelp()
{
	//pirnt help messages
	printf("\nUsage: imageIO_bioformat <input_file>\n");
	printf(" Image IO using Bio-format. \n");
	printf(" [-h]	HELP message.\n");
	return;
}


//read image file and save as .tif/.raw file
int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printHelp ();
		return 0;
	}
	
	char *dfile_input = argv[1];
	//char *dfile_output = argv[2];
	
	try 
	{
		string filename = dfile_input;
		
		createJVM();
		
		vector<IMAGE> pImgList;

		//
		ImageReader* imageReader = new ImageReader; // read all image formats
		ChannelFiller* channelFiller = new ChannelFiller(*imageReader); // expand 8-bit indexed color images to full RGB
		ChannelSeparator* reader = new ChannelSeparator(*channelFiller); // split interleaved RGB channels into separate image planes
		//FileStitcher* fileStitcher = new FileStitcher(channelSeparator);	
		
		// attach OME metadata object
		IMetadata omeMeta = MetadataTools::createOMEXMLMetadata();
		reader->setMetadataStore(omeMeta);
		
		bool isType = reader->isThisType(filename);
		printf("Is this kind of image recognized by Bio-Format: %s \n",(isType)?"Yes":"No");
		
		reader->setId(filename);
		
		String id = imageReader->getFormat(filename);
		//std::cout<< "Format is "<< id<<endl;
		
		int seriesCount = reader->getSeriesCount();
		std::cout << "How many series: " << seriesCount << endl;
		
		MetadataStore ms = reader->getMetadataStore();
		MetadataRetrieve mr = MetadataTools::asRetrieve(ms);
		
		//save temp images' names to text file
		FILE *pTIFile=0;
		string tmpimage = "names_tmp_images.txt";
		pTIFile = fopen(tmpimage.c_str(),"wt");

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
			printf("Endianness = %s \n", (little ? "intel (little)" : "motorola (big)"));
			
			printf("sizeX %d sizeY %d sizeZ %d sizeC %d sizeT %d effSizeC %d rgbChannelCount %d imageCount %d bpp %d \n", sizeX, sizeY, sizeZ, sizeC, sizeT, effSizeC, rgbChannelCount, imageCount, bpp);
			
			// size of image
			V3DLONG N = sizeX*sizeY*sizeZ*sizeC*sizeT*bpp; //
			unsigned char* data1d_8 = 0;
			short int* data1d_16 = 0;
			float* data1d_32 = 0;
			
			IMAGE pImg(sizeX, sizeY, sizeZ, sizeC, sizeT, v3dComponentType, mr.getImageName(js));
			
			if(v3dComponentType == V3D_UINT8)
			{
				data1d_8 = pImg.data1d_8;
				printf("V3D_UINT8 \n");
			}
			else if (v3dComponentType == V3D_UINT16)
			{
				data1d_16 = pImg.data1d_16;
				printf("V3D_UINT16 \n");
			}
			else if (v3dComponentType == V3D_FLOAT32)
			{
				data1d_32 = pImg.data1d_32;
				printf("V3D_FLOAT32 \n");
			}
			
			// check image info
			//change the following data types from int to V3DLONG, by PHC, 100601
			V3DLONG pIndex = 0, pCount = imageCount;
			V3DLONG bytesPerPlane = sizeX * sizeY * bpp * rgbChannelCount;
			
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

			//temprary image
			char tmp[5];
			int ten = js/10;
			int dig = js - ten*10;
			tmp[0] = 's'; tmp[1] = ten + '0'; tmp[2] = dig + '0'; tmp[3] = '\0';

			string tmp_filename = filename + "_" + tmp + ".raw";

			V3DLONG sz_tmp[4];
			
			sz_tmp[0] = sizeX; sz_tmp[1] = sizeY; sz_tmp[2] = sizeZ*sizeT; sz_tmp[3] = sizeC; 

			if(v3dComponentType == V3D_UINT8)
			{
				if (saveImage(tmp_filename.c_str(), (const unsigned char *)data1d_8, sz_tmp, V3D_UINT8)!=true)
				{
					fprintf(stderr, "Error happens in file writing. Exit. \n");
					return -1;
				}
			}
			else if (v3dComponentType == V3D_UINT16)
			{
				if (saveImage(tmp_filename.c_str(), (const unsigned char *)data1d_16, sz_tmp, V3D_UINT16)!=true)
				{
					fprintf(stderr, "Error happens in file writing. Exit. \n");
					return -1;
				}
			}
			else if (v3dComponentType == V3D_FLOAT32)
			{
				if (saveImage(tmp_filename.c_str(), (const unsigned char *)data1d_32, sz_tmp, V3D_FLOAT32)!=true)
				{
					fprintf(stderr, "Error happens in file writing. Exit. \n");
					return -1;
				}
			}
			
			//
			fprintf(pTIFile, "%s \n", tmp_filename.c_str() );
		
		}
		// de-alloc
		imageReader->close();
		delete imageReader;
		delete channelFiller;
		delete reader;
		fclose(pTIFile);

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
