/* FaceDetection.cpp
* This is a test plugin, you can use it as a demo.
* 2015-3-19 : by Yuming LI, Yukun YAN
*/

#include "v3d_message.h"
#include <vector>
#include "FaceDetection_plugin.h"

#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <iostream>
static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascade = 0;
void detect_and_draw(IplImage *img);
const char* cascade_name = "haarcascade_frontalface_alt.xml";



Q_EXPORT_PLUGIN2(FaceDetection, FaceDetection);

using namespace std;

struct input_PARA
{
	QString inimg_file;
	V3DLONG channel;
};

int facedetection_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);

QStringList FaceDetection::menulist() const
{
	return QStringList() 
		<<tr("FaceDet_menu")
		<<tr("about");
}

QStringList FaceDetection::funclist() const
{
	return QStringList()
		<<tr("FaceDet_func")
		<<tr("help");
}

void FaceDetection::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("FaceDet_menu"))
	{
		bool bmenu = true;
		input_PARA PARA;
		facedetection_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("This is a face detection plugin, you can use it as a demo.. "
			"Developed by Yuming LI, Yunkun YAN, 2015-3-19"));
	}
}

bool FaceDetection::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name == tr("FaceDet_func"))
	{
		bool bmenu = false;
		input_PARA PARA;

		vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
		vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
		vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
		vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

		if(infiles.empty())
		{
			fprintf (stderr, "Need input image. \n");
			return false;
		}
		else
			PARA.inimg_file = infiles[0];
		int k=0;
		PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
		facedetection_func(callback,parent,PARA,bmenu);
	}
	else if (func_name == tr("help"))
	{

		////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of FaceDetection tracing **** \n");
		printf("vaa3d -x FaceDetection -f FaceDet_func -i <inimg_file> -p <channel> <other parameters>\n");
		printf("inimg_file       The input image\n");
		printf("channel          Data channel for tracing. Start from 1 (default 1).\n");

		printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}


int facedetection_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
	int a, b;
    char buffer[100]; for (int i=1=0; i<100; i++) buffer[i]='\0';
	a = 1000;
	b = 2000;
	sprintf(buffer, "%d", a + b);
	v3d_msg(buffer);

	IplImage *img = cvCreateImage(cvSize(512,512),8,1);
	cvNamedWindow("test");
    cvShowImage("test",img);
	cvWaitKey();
	return 1;
	//main neuron reconstruction code

	//// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON TRACING CODE
//	CvCapture* capture = 0;
//	IplImage *frame, *frame_copy = 0;
//
//	cascade = (CvHaarClassifierCascade*)cvLoad( cascade_name, 0, 0, 0 );
//
//	if( !cascade )
//	{
//		fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
//		return -1;
//	}
//
//	storage = cvCreateMemStorage(0);
//
//	capture = cvCaptureFromCAM(0);
//
//	if( capture )
//	{
//		int times = 0;
//
//		while (1)
//		{
//			if( !cvGrabFrame( capture ))
//				break;
//			frame = cvRetrieveFrame( capture );
//			if( !frame ){break;}
//			if( !frame_copy )
//			{
//				frame_copy = cvCreateImage( cvSize(frame->width,frame->height),
//					IPL_DEPTH_8U, frame->nChannels );
//			}
//			if( frame->origin == IPL_ORIGIN_TL )
//			{
//				cvCopy( frame, frame_copy, 0 );
//			}
//			else
//			{
//				cvFlip( frame, frame_copy, 0 );
//			}
//
//			detect_and_draw(frame_copy);
//
//			if( cvWaitKey( 20 ) >= 0 )
//				break;
//		}
//
//		cvReleaseImage( &frame_copy );
//		cvReleaseCapture( &capture );
//	}
//
//	return 0;
//}
//
//void detect_and_draw(IplImage *img)
//{
//	static CvScalar colors[] = 
//	{
//		{{0,0,255}},
//		{{0,128,255}},
//		{{0,255,255}},
//		{{0,255,0}},
//		{{255,128,0}},
//		{{255,255,0}},
//		{{255,0,0}},
//		{{255,0,255}}
//	};
//
//	double scale = 1.3;
//	IplImage* gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
//	IplImage* small_img = cvCreateImage( cvSize( cvRound (img->width/scale),
//		cvRound (img->height/scale)),
//		8, 1 );
//	int tim = 0;
//
//	cvCvtColor( img, gray, CV_BGR2GRAY );
//	cvResize( gray, small_img, CV_INTER_LINEAR );
//	cvEqualizeHist( small_img, small_img );
//	cvClearMemStorage( storage );
//
//	if( cascade )
//	{
//		double t = (double)cvGetTickCount();
//		CvSeq* faces = cvHaarDetectObjects( small_img, cascade, storage,
//			1.1, 2, 0/*CV_HAAR_DO_CANNY_PRUNING*/,
//			cvSize(30, 30) );
//		t = (double)cvGetTickCount() - t;
//		printf( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
//		double sx = 0;
//		double sy = 0;
//		double wid = 0;
//		double hei = 0;
//		for( int i = 0; i < (faces ? faces->total : 0); i++ )
//		{
//			CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
//			CvPoint center;
//			int radius;
//			center.x = cvRound((r->x + r->width*0.5)*scale);
//			center.y = cvRound((r->y + r->height*0.5)*scale);
//			radius = cvRound((r->width + r->height)*0.25*scale);
//			cvCircle( img, center, radius, colors[i%8], 3, 8, 0 );
//			sx = r->x;
//			sy = r->y;
//			wid = r->width;
//			hei = r->height;
//			//cvRectangle(img,cvPoint((r->x)*scale,(r->y)*scale),cvPoint((r->x+r->width)*scale,(r->y+r->height)*scale),colors[i%8]);
//		}
//
//		if(wid > 190 && hei > 190)
//		{
//			IplImage* faceimg = cvCreateImage( cvSize(256,256), 8, 1 );
//			cvSetImageROI(small_img,cvRect(sx,sy,wid,hei));
//			cvResize( small_img, faceimg, CV_INTER_LINEAR );
//			cvResetImageROI(small_img);
//			cvEqualizeHist( faceimg, faceimg );
//			cvShowImage("face",faceimg);
//		}
//
//		cvShowImage( "result", img );
//		cvReleaseImage( &gray );
//		cvReleaseImage( &small_img );
//	}
}
