// smartscope_dev_pgr.h
// smartscope device control program for the PGR camera
// by Hanchuan Peng
// 20110516


#ifndef __SMARTSCOPE_DEV_PGR_H__
#define __SMARTSCOPE_DEV_PGR_H__

#include "FlyCapture2.h" 

namespace FlyCapture2
{
	class PGRImageCap
	{
	public:
		PGRImageCap()
		{
			//data initialization: important
			pData = 0;
			dimx = 0;
			dimy = 0;

			//camera link
			PrintBuildInfo();

			BusManager busMgr;
			if (errorCheck( busMgr.GetNumOfCameras(&numCameras) ) != 0)
				return;
			
			if (errorCheck( busMgr.GetCameraFromIndex(0, &guid) ) != 0)
				return;

			// Connect to camera
			if (errorCheck( cam.Connect(&guid) ) != 0)
				return;

			// Start capturing images
			if (errorCheck( cam.StartCapture() ) != 0)
				return;

			imgcap();

			//closeCam(); //do not close for now
		}

		~PGRImageCap()
		{
			if (pData) {delete []pData; pData=0;}
			dimx = dimy = 0; //also reset size
		}

	public:
		void PrintBuildInfo()
		{
			FC2Version fc2Version;
			Utilities::GetLibraryVersion( &fc2Version );
			char version[128];
			sprintf( 
				version, 
				"FlyCapture2 library version: %d.%d.%d.%d\n", 
				fc2Version.major, fc2Version.minor, fc2Version.type, fc2Version.build );

			printf( "%s", version );

			char timeStamp[512];
			sprintf( timeStamp, "Application build date: %s %s\n\n", __DATE__, __TIME__ );

			printf( "%s", timeStamp );
		}

		void PrintError( Error error )
		{
			error.PrintErrorTrace();
		}

		int imgcap()
		{
			int err;
			V3DLONG start_t = clock();

			Image rawImage;    

			// Retrieve an image
			err = errorCheck( cam.RetrieveBuffer( &rawImage ) );
			if (err!=0)
			{
				v3d_msg("Error in imgcap RetrieveBuffer",0);
				return -1;
			}

			// Get the raw image dimensions
			PixelFormat pixFormat;
			unsigned int rows, cols, stride;
			rawImage.GetDimensions( &rows, &cols, &stride, &pixFormat );

			// Create a converted image
			Image convertedImage;

			// Convert the raw image
			err = errorCheck( rawImage.Convert( PIXEL_FORMAT_MONO8, &convertedImage ) );
			if (err!=0)
			{
				v3d_msg("Error in imgcap rawImgConvert",0);
				return -1;
			}

			//the following part ensures the program always use one single trunk of memory, w/o multple allocation that caused the crash in Yang Yu's code
			if (!pData || dimx!=cols || dimy!=rows) //also reset the image if the dimensions change
			{
				dimx = cols;
				dimy = rows;
				if (pData) {delete []pData; pData=0;} //reset image first if the pointer is not NULL
				try {pData = new unsigned char [dimx*dimy];}
				catch (...) {v3d_msg("Fail to allocate memory in camera imaging", 0); return -1;}
			}

			printf("Dimension of the Image (dimx, dimy) is: (%d, %d)", dimx, dimy);

			V3DLONG pagesz = dimx * dimy;
			for(V3DLONG i=0; i<pagesz; i++)
			{
				pData[i] = convertedImage.GetData()[i]; //this is slow, should use memory copy in the future! Noted by PHC
			}

			V3DLONG end_t = clock();

			//qDebug()<<"time for camera pic-taking : "<<end_t-start_t;
			return 0;
		}

		// This function will create a disk at a random position of the image
		int imgcap_sim()
		{
			long i, j;
			float radius;
			int sx=1280; // image dims
			int sy=960;

			dimx = sx;
			dimy = sy;
			// initialize all to 0
			for(i=0; i<sx*sy; i++)
			{
			  pData[i]=0;
			}

			radius = (sx>=sy)? sy/4.0 : sx/4.0;
			if(radius==0.0) return 1;
		    
			// create random center
			int cenx, ceny;// random center
			int rx=rand()%sx;
			int ry=rand()%sy;
			
			if((sx/2-rx)>=0) cenx=sx/2 - rx/4;
			else cenx=sx/2 + rx/4;
			if((sy/2-ry)>=0) ceny=sy/2 - ry/4;
			else ceny=sy/2 + ry/4;

			for(j=0; j<sy; j++)
			{
			   // for real image part
			   // create a disk (value decreases from center to surroundings)
			   for(i=0; i<sx; i++)
			   {
					float dist;
					dist = sqrt( (float)(i-cenx)*(i-cenx) +
						 (float)(j-ceny)*(j-ceny));
					int idx = j*sx+i;
					if(dist<radius)
					{
						 pData[idx]=(unsigned char)255*(1-dist/radius);                         
					}
			   }
			}

			printf("Finish creating simulation image");
			return 0;
		}

		int closeCam()
		{
			int err = 0;
			if ((err = errorCheck( cam.StopCapture())) == 0 ) // first stop capturing images
				return (err = errorCheck( cam.Disconnect() )); // then disconnect the camera
			else
				return err;
		}

		int errorCheck(Error error)
		{
			if (error != PGRERROR_OK)
			{
				PrintError( error );
				return -1;
			}
			else 
				return 0;
		}

	public:
		unsigned int numCameras;
		PGRGuid guid;
		Camera cam;

		unsigned char *pData;
		V3DLONG dimx, dimy;
	};

}
















#endif

