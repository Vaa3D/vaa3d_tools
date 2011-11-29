/**
 * wavelets.cpp
 *
 * Written by
 *
 * Ihor Smal
 * Nicolas Chenouard
 * Fabrice de Chaumont
 *
 * Paper reference: ISBI and JC ref
 *
 * This code is under GPL License
 *
 *
 */
#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <list>

#include "wavelets.h"
#include "scaleinfo.h"
#include <v3d_basicdatatype.h>
#include "../basic_c_fun/basic_landmark.h"
#include "waveletConfigException.h"
#include <iostream>


Q_EXPORT_PLUGIN2(newwindow, WaveletPlugin);

const QString title = "Wavelets";

QStringList WaveletPlugin::menulist() const
{
    return QStringList()<< tr("Wavelets")<< tr("Cloning")<<tr("FFT")<<tr("Wavelet Transform");


}

/**
 * From Interface V3DPluginInterface. Called when user click wavelets in plugin menu.
 */
void WaveletPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	if (menu_name == tr("Wavelets"))
	{
		initGUI( callback, parent );
	}
	if (menu_name == tr("Cloning"))
    {
    	Cloning(callback, parent);
    }
    if (menu_name == tr("FFT"))
    {
#if USING_FFT
    	FFT(callback, parent);
#endif
    }
    if (menu_name == tr("Wavelet Transform"))
    {
    	WaveletTransform(callback, parent);
    }
}

WaveletPlugin::WaveletPlugin()
{
	this->scaleInfoList = new std::list<ScaleInfo*>();
  this->scaleComputationReady = false;
  this->thresholdFinal = 0;
}

void WaveletPlugin::addScaleButtonPressed()
{
	printf("WAVELETS : add scale !\n");

	ScaleInfo *scaleInfo = new ScaleInfo( this, scaleInfoList->size() , qBox );
	scaleInfoList->push_back( scaleInfo );
	printf("%p" , scaleInfo );
	refreshScaleInterface();
}

void WaveletPlugin::removeScaleButtonPressed()
{
	printf("WAVELETS : remove scale !\n");
	if ( scaleInfoList->size() > 1 )
	{
		ScaleInfo *scaleInfo = scaleInfoList->back(); //(*lend);
		delete scaleInfo;
		scaleInfoList->pop_back( );
	}

	refreshScaleInterface();
}

void WaveletPlugin::refreshScaleInterface()
{

	printf("WAVELETS : refresh Scale interface\n");

	removeScaleButton->setDisabled( scaleInfoList->empty() );


	// remove everything in the layout.

	QLayoutItem *child;
	while ((child = formLayoutGroupBox->takeAt(0)) != 0) {
		formLayoutGroupBox->removeItem(child);
	}

//	qBox->adjustSize();
	myDialog->adjustSize();

	// write back all the scale interface

	ListType::iterator litr = scaleInfoList->begin();
	ListType::iterator lend = scaleInfoList->end();

	while( litr != lend )
	{
		printf("adding scale interface..");
		formLayoutGroupBox->addRow( (*litr)->groupBox );
		litr++;
	}

	myDialog->adjustSize();

	scaleComputationReady = false;

	updateWaveletAskedByGUI( );
}

/**
 * Cancel Action.
 */
void WaveletPlugin::cancel()
{
	printf("WAVELETS : cancel !\n");
	printf("WAVELETS : Restoring original image...\n");
	restoreOriginalImage();
	printf("WAVELETS : Restore done.\n");
	myDialog->close();
}

void WaveletPlugin::updateWaveletAskedByGUI()
{
	if ( liveUpdateCheckBox->isChecked() )
	{
		computeWavelets( false );
	}
}

void WaveletPlugin::updateWavelet()
{
	computeWavelets( false );
}

void WaveletPlugin::sliderChange(int value )
{
	// deprecated
}

/**
 * Copy original image
 */
void WaveletPlugin::copyOriginalImage()
{
	originalImageCopy = new Image4DSimple();
	unsigned char *bufferSource = sourceImage->getRawData();
	unsigned char *bufferCopy = new unsigned char[sourceImage->getTotalBytes()];
	memcpy( bufferCopy , bufferSource , sourceImage->getTotalBytes() );

	originalImageCopy->setData( bufferCopy , sourceImage );



}

void WaveletPlugin::showOriginalPressed()
{
	printf("show original image pressed");
	restoreOriginalImage();
}

/**
 * Restore original image.
 */
void WaveletPlugin::restoreOriginalImage()
{
	printf("\nWAVELETS : Restore original Image \n");

	sourceImage->setXDim( originalImageCopy->getXDim() );
	sourceImage->setYDim( originalImageCopy->getYDim() );
	sourceImage->setZDim( originalImageCopy->getZDim() );
	sourceImage->setCDim( originalImageCopy->getCDim() );
	sourceImage->setDatatype( originalImageCopy->getDatatype() );

	if ( !originalImageCopy->getRawData() )
	{
		printf("\n no pointer for originalImageCopy !! \n");
	}
	if ( !sourceImage->getRawData() )
	{
		printf("\nno pointer for sourceImage !! \n");
	}

	// MAC OS Version **************************************************************

//	memcpy( sourceImage->getRawData() , originalImageCopy->getRawData() , originalImageCopy->getTotalBytes() );
#if 0

	printf("%d source\n" , sourceImage->getTotalBytes() );
	printf("%d original\n" , originalImageCopy->getTotalBytes() );
	printf("\n");

	myCallback->setImage(sourceWindow, originalImageCopy );
	myCallback->updateImageWindow(sourceWindow);
#endif

	// WINDOWS Version ************************************************************


	printf("%ld source\n" , sourceImage->getTotalBytes() );
	printf("%ld original\n" , originalImageCopy->getTotalBytes() );
	printf("\n");

	memcpy( sourceImage->getRawData() , originalImageCopy->getRawData() , originalImageCopy->getTotalBytes() );

	myCallback->setImage(sourceWindow, sourceImage );

  std::cout << "SLEEPING FOR 1 sec" << std::endl;
  sleep( 1 ); 

	myCallback->updateImageWindow(sourceWindow);

}


/**
 *	Init the GUI of the plugin.
 */
void WaveletPlugin::initGUI( V3DPluginCallback &callback, QWidget *parent )
{
	scaleComputationReady = false;

	thresholdResidualScaleLabel = new QLabel("0");
	thresholdResidualScaleLabel->setFixedSize(20,20);

	liveUpdateCheckBox = new QCheckBox("Live update");
	liveUpdateCheckBox->setChecked( false );

	useLowPassCheckBox  = new QCheckBox("Enable lowPass");
	useLowPassCheckBox->setChecked( false );

	sourceWindow = callback.currentImageWindow();
	Image4DSimple* p4DImage = callback.getImage(sourceWindow);
	if (!p4DImage)
	{
		QMessageBox::information(0, "Cloning", QObject::tr("No image is open."));
		return;
	}

	showOriginalButton = new QPushButton("Display original");

	sourceImage = p4DImage;

	myCallback = &callback;

	copyOriginalImage(); // for cancel purpose.

	// Building the main interface.



	myDialog = new QDialog(parent);
	//myDialog = new QWidget( );

	QPushButton* ok     = new QPushButton("OK");

	QPushButton* addScaleButton     = new QPushButton("Add Scale");
	removeScaleButton     = new QPushButton("Remove Scale");

	QPushButton* cancel = new QPushButton("Cancel");
	formLayout = new QFormLayout( myDialog );

	formLayout->addRow( removeScaleButton , addScaleButton  );

	QSlider *slider = new QSlider(Qt::Horizontal);
	     slider->setFocusPolicy(Qt::StrongFocus);
	     slider->setTickPosition(QSlider::TicksBothSides);
	     slider->setTickInterval(10);
	     slider->setSingleStep(1);
	     slider->setMaximum(255);

//	 	formLayout->addRow( slider );

		thresholdResidualScaleSlider = new QSlider(Qt::Horizontal);
		thresholdResidualScaleSlider->setFocusPolicy(Qt::StrongFocus);
		thresholdResidualScaleSlider->setTickPosition(QSlider::TicksBothSides);
		thresholdResidualScaleSlider->setTickInterval(1);
		thresholdResidualScaleSlider->setSingleStep(1);
		thresholdResidualScaleSlider->setMaximum(100);

	progressBar = new QProgressBar(  );
	progressBar->setRange(0,100);
	progressBar->setValue( 100 );

// 	formLayout->addRow( processButton );
// 	formLayout->addRow( progressBar );

	//QLabel* label2 = new QLabel("Test");
 	qBox= new QGroupBox( myDialog );
	formLayoutGroupBox = new QFormLayout( qBox );


	//qBox->setLayout( formLayoutGroupBox );
	//formLayoutGroupBox->addRow( label2 );

 	formLayout->addRow(qBox);

 	QGroupBox *groupBox2 = new QGroupBox( qBox );
	QGridLayout *gridLayout2 = new QGridLayout( groupBox2 );
	groupBox2->setTitle( "LowPass Residual" );
	gridLayout2->addWidget( useLowPassCheckBox , 0 , 0 );
	gridLayout2->addWidget( thresholdResidualScaleLabel , 0 , 1 );
	gridLayout2->addWidget( thresholdResidualScaleSlider , 0 , 2 );
	formLayout->addRow( groupBox2 );

	thresholdFinalSlider = new QSlider(Qt::Horizontal);
	thresholdFinalLabel = new QLabel("0");
	thresholdFinalLabel->setFixedSize(20,20);

 	QGroupBox *groupBox3 = new QGroupBox( qBox );
	QGridLayout *gridLayout3 = new QGridLayout( groupBox3 );
	groupBox3->setTitle( "Final threshold" );
	thresholdFinalSlider->setFocusPolicy(Qt::StrongFocus);
	thresholdFinalSlider->setTickPosition(QSlider::TicksBothSides);
	thresholdFinalSlider->setTickInterval(20);
	thresholdFinalSlider->setSingleStep(1);
	thresholdFinalSlider->setMaximum(255);
	thresholdFinalLabel->setFixedSize(100,20);
	gridLayout3->addWidget( thresholdFinalSlider , 0 , 0 );
	gridLayout3->addWidget( thresholdFinalLabel , 0 , 1 );
	formLayout->addRow( groupBox3 );

	myDialog->setLayout(formLayout);
	myDialog->setWindowTitle(QString("Wavelets"));

	myDialog->connect(ok,     SIGNAL(clicked()), myDialog, SLOT(accept()));
	myDialog->connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChange(int)));
	myDialog->connect(cancel, SIGNAL(clicked()), this, SLOT(cancel()));
	myDialog->connect(addScaleButton, SIGNAL(clicked()), this, SLOT(addScaleButtonPressed()));
	myDialog->connect(removeScaleButton, SIGNAL(clicked()), this, SLOT(removeScaleButtonPressed()));

	for (int i = 0 ; i < 4 ; i++ ) // add some scales.
	{
		addScaleButtonPressed();
	}

	QPushButton* dev1Button = new QPushButton("Dev #1");
	QPushButton* dev2Button = new QPushButton("Dev #2");
	QPushButton* dev3Button = new QPushButton("Dev #3");
	QPushButton* dev4Button = new QPushButton("Dev #4");

	QPushButton* denoiseButton = new QPushButton("Denoise");
	QPushButton* detectSpotsButton = new QPushButton("Detect spots");

	formLayout->addRow( liveUpdateCheckBox , showOriginalButton );
	formLayout->addRow( denoiseButton, detectSpotsButton );
	formLayout->addRow(ok, cancel);

//	formLayout->addRow(dev1Button, dev2Button);
//	formLayout->addRow(dev3Button, dev4Button);

	myDialog->connect(dev1Button, SIGNAL(clicked()), this, SLOT(dev1ButtonPressed()));
	myDialog->connect(dev2Button, SIGNAL(clicked()), this, SLOT(dev2ButtonPressed()));
	myDialog->connect(dev3Button, SIGNAL(clicked()), this, SLOT(dev3ButtonPressed()));
	myDialog->connect(dev4Button, SIGNAL(clicked()), this, SLOT(dev4ButtonPressed()));

	myDialog->connect(showOriginalButton, SIGNAL(clicked()), this, SLOT(showOriginalPressed()));
	myDialog->connect(denoiseButton, SIGNAL(clicked()), this, SLOT(denoiseButtonPressed()));
	myDialog->connect(detectSpotsButton, SIGNAL(clicked()), this, SLOT(detectSpotsButtonPressed()));

	myDialog->connect(liveUpdateCheckBox, SIGNAL(clicked()), this, SLOT(livePressed()));

	myDialog->connect(useLowPassCheckBox, SIGNAL(clicked()), this, SLOT(useLowPassPressed()));

	myDialog->connect(thresholdResidualScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderResidualChange(int)));

	myDialog->connect(thresholdFinalSlider, SIGNAL(valueChanged(int)), this, SLOT(thresholdFinalSliderChange(int)));



	thresholdResidualScale = 0;

	myDialog->exec();
	//myDialog->show();



}

void WaveletPlugin::thresholdFinalSliderChange( int value )
{
	thresholdFinal = value;
	char * text = new char[50];
	sprintf(text , "%d" , value  );
	thresholdFinalLabel->setText( text );
	updateWaveletAskedByGUI();
}

void WaveletPlugin::useLowPassPressed()
{
	updateWaveletAskedByGUI();
}

void WaveletPlugin::livePressed()
{
	updateWaveletAskedByGUI();
}

void WaveletPlugin::sliderResidualChange(int value)
{
	thresholdResidualScale = value;

	char *text = new char[50];
	sprintf(text , "%d" , value );
	thresholdResidualScaleLabel->setText( text );

	if ( useLowPassCheckBox->isChecked() ){
		updateWaveletAskedByGUI();
	}

}

void WaveletPlugin::computeWavelets( bool displayDetection )
{
	if ( !scaleComputationReady )
	{
		printf("WAVELET : denoise pressed\n");

		data1dD = channelToDoubleArray(originalImageCopy, 1);
printf("channelToDoubleArray() data1dD %p\n", data1dD);
		//get dims
		szx = originalImageCopy->getXDim();
		szy = originalImageCopy->getYDim();
		szz = originalImageCopy->getZDim();
		sc = originalImageCopy->getCDim();
		N = szx * szy * szz;

		numScales = scaleInfoList->size();
		//compute wavelet scales
		resTab = NULL;
		try {
			time_t seconds0 = time (NULL);
			if (szz>1)
        {
				resTab = WaveletTransform::b3WaveletScales(data1dD, szx, szy, szz, numScales);
printf("b3WaveletScales() data1dD %p\n", data1dD);
        }
			else
        {
				resTab = WaveletTransform::b3WaveletScales2D(data1dD, szx, szy, numScales);
printf("b3WaveletScales2D() data1dD %p\n", data1dD);
        }
			time_t seconds1 = time (NULL);
			printf("Computation time : %d" , (seconds1-seconds0) );
		}
		catch(WaveletConfigException e)
		{
			printf("\nEXCEPTION\n %s \n" , e.what() );
			return;
		}

		//compute waveletCoefficients
		lowPassResidual = new double[N];
		WaveletTransform::b3WaveletCoefficientsInplace(resTab, data1dD, lowPassResidual, numScales, N);
printf("b3WaveletCoefficientsInplace() data1dD %p\n", data1dD);

printf("DELETE () data1dD %p\n", data1dD);
    delete [] data1dD;

		// Create a copy of resTab[]

 		resTabCopy = new double*[numScales];
		for ( int i = 0 ; i < numScales ; i++ )
		{
			resTabCopy[i] = new double[N];
			memcpy( resTabCopy[i] , resTab[i] , N * sizeof(double) );
		}
		lowPassResidualCopy = new double[N];
		memcpy( lowPassResidualCopy , lowPassResidual , N * sizeof(double) );

		// end of copy.

		scaleComputationReady = true;
	}
	filterB3Wavelets( displayDetection );

}

/**
 * Denoise
 */
void WaveletPlugin::denoiseButtonPressed()
{
	scaleComputationReady = false;
	updateWavelet();
}

void WaveletPlugin::filterB3Wavelets( bool displayDetection )
{
	// Copy back of resTab[]

	for ( int i = 0 ; i < numScales ; i++ )
	{
		memcpy( resTab[i] , resTabCopy[i] , N * sizeof(double) );
	}
	// Copy back of memCopy[]

	memcpy( lowPassResidual , lowPassResidualCopy , N * sizeof(double) );

	int numScales = scaleInfoList->size();

	// filtering wavelets images
	ListType::iterator litr = scaleInfoList->begin();
	ListType::iterator lend = scaleInfoList->end();

	for ( int scale = 0 ; scale < numScales ; scale++ )
	{
		ScaleInfo *scaleInfo = *litr;
		printf("current scale threshold : %d \n" , scaleInfo->thresholdValue );

		double currentScaleThresholdSquared = 0;
		double *image = resTab[scale];
		//double *imageTarget = resTab[scale];

		if ( scaleInfo->enable == true )
		{
			currentScaleThresholdSquared = scaleInfo->thresholdValue*scaleInfo->thresholdValue;
			printf("\n current scale %d threshold squared: %f \n" , scale , currentScaleThresholdSquared );

			for ( int n=0 ; n < N ; n++ )
			{
				if ( image[n]*image[n] < currentScaleThresholdSquared )
				{
					image[n] = 0;
				}

			}

		}else
		{
			for ( int n=0 ; n < N ; n++ )
			{
				image[n] = 0;
			}
		}

		litr++; // increment scaleInfo;
	}
	// end filtering wavelets images.

	// filter lowPassResidual

	if ( !useLowPassCheckBox->isChecked()  )
	{
		for ( int n=0 ; n < N ; n++ )
		{
			lowPassResidual[n] = 0;
		}
	}
	else
	{

		double thresholdResidualScaleSquare = thresholdResidualScale * thresholdResidualScale;
		for ( int n=0 ; n < N ; n++ )
		{
			if ( lowPassResidual[n] < thresholdResidualScaleSquare )
			{
				lowPassResidual[n] = 0;
			}
		}

	}

//	delete(data1dD);  // FIXME

	//reconstruct image from coefficients
	double* rec = new double[N];
	WaveletTransform::b3WaveletReconstruction(resTab, lowPassResidual, rec, numScales, N);


	// Final threshold

	for ( int n = 0 ; n<N ; n++ )
	{
		if ( rec[n] < thresholdFinal )
		{
			rec[n]=0;
		}
	}

	// perform detection

	LandmarkList cmList;
	if ( displayDetection )
	{
		printf("\nDETECTION\n");
		if ( szz > 1 )
		{
			// 3D
			cmList = v3d_utils::getConnectedComponents(rec, szx, szy, szz, 0 );
		}else
		{
			// 2D
			cmList = v3d_utils::getConnectedComponents2D(rec, szx, szy, szz, 0 );
		}
		printf("\nDETECTION FINISHED\n");
	}

	//display reconstructed image
	//rescaleForDisplay(rec, rec, N, originalImageCopy->getDatatype());
	rescaleForDisplay(rec, rec, N, sourceImage->getDatatype());
	//unsigned char* dataOut1d = doubleArrayToCharArray(rec, N, originalImageCopy->getDatatype());
	unsigned char* dataOut1d = doubleArrayToCharArray(rec, N, sourceImage->getDatatype());

	Image4DSimple outImage;
	outImage.setData(dataOut1d, originalImageCopy );

	if ( displayDetection ) // BUG Issue: it is not possible to set detection in the current viewer.
	{
		v3dhandle newwin = myCallback->newImageWindow();
		myCallback->setImage(newwin, &outImage);
		myCallback->setImageName(newwin,"Detection");
		myCallback->setLandmark(newwin, cmList);
		myCallback->updateImageWindow( newwin );
	}
	else
	{
		myCallback->setImage(sourceWindow, &outImage);
		myCallback->updateImageWindow(sourceWindow);
	}


	delete [] rec;

}
/**
 * Detect spots
 */
void WaveletPlugin::detectSpotsButtonPressed()
{
	printf("WAVELET : detection spots pressed\n");
	printf("test\n");

	computeWavelets( true );

	printf("WAVELET : detection spots finished\n");
}

// This is a dev button. Use it for test purpose !
// User : nobody
// Should do : nothing
void WaveletPlugin::dev1ButtonPressed()
{
	printf("dev 1 pressed\n");
	// use myCallback if you need the one provide by
	// WaveletPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)	//
	printf("dev 1 finished\n");
}

// This is a dev button. Use it for test purpose !
// User : nobody
// Should do : nothing
void WaveletPlugin::dev2ButtonPressed()
{
	printf("dev 2 pressed\n");
	// use myCallback if you need the one provide by
	// WaveletPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)	//

	printf("dev 2 finished\n");
}

// This is a dev button. Use it for test purpose !
// User : nobody
// Should do : nothing
void WaveletPlugin::dev3ButtonPressed()
{
	printf("dev 3 pressed\n");
	// use myCallback if you need the one provide by
	// WaveletPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)	//
	printf("dev 3 finished\n");
}

// This is a dev button. Use it for test purpose !
// User : nobody
// Should do : nothing
void WaveletPlugin::dev4ButtonPressed()
{
	printf("dev 4 pressed\n");
	
	
	v3dhandle sourceWindow = myCallback->currentImageWindow();
	Image4DSimple* p4DImage = myCallback->getImage(sourceWindow);
	
	printf("\n POINTERS %p   |    %p\n\n", p4DImage, &sourceWindow);
	
	
	// 
// 	//use myCallback if you need the one provide by
// 	// WaveletPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)	//
// 	
// 	
// 	unsigned char* imageRaw = sourceImage->getRawData();
// 
// 	if ( !imageRaw )
// 	{
// 		printf("WAVELETS : no image.. \n");
// 		return;
// 	}
// 
// 	printf("WAVELETS : enter1");
// 
// 	for ( int i =0 ; i< 10000 ; i++ )
// 		{
// 		imageRaw[i] = 255;
// 		}
// 
// 	printf("WAVELETS : enter2");
// 
// 	myCallback->updateImageWindow(sourceWindow);
	
	printf("dev 4 finished\n");
}


#if USING_FFT
void WaveletPlugin::FFT(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* p4DImage = callback.getImage(oldwin);
	if (!p4DImage)
	{
		QMessageBox::information(0, "Cloning", QObject::tr("No image is open."));
		return;
	}
	double* data1dD = channelToDoubleArray(p4DImage, 1);

	//get dims
    V3DLONG szx = p4DImage->getXDim();
    V3DLONG szy = p4DImage->getYDim();
    V3DLONG szz = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    V3DLONG N = szx * szy * szz;
  
	// FFTW
	//careful, fftw is using row major indexing = z dimension is varying first, while v3d is using column major indexing = x dimension is varying first
	//the last dimension is cut in half (x dimension for v3d convention)
	V3DLONG nxOut = ((szx / 2 + 1));
	V3DLONG nyOut = szy;
	V3DLONG nzOut = szz;
	V3DLONG nOut = 	nxOut * nyOut * nzOut;
	
	//create output array
	fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nOut);
	
	//prepare execution plan
	fftw_plan p;
	time_t seconds0 = time (NULL);
	p = fftw_plan_dft_r2c_3d(szz, szy, szx, data1dD, out, FFTW_MEASURE);
	//p = fftw_plan_dft_r2c_2d(szy, szx, data1dD, out, FFTW_ESTIMATE);

 	time_t seconds1 = time (NULL);
	printf ("prepare %ld\n", seconds1-seconds0);

	//execute plan
	fftw_execute(p);
 	time_t seconds2 = time (NULL);
	printf ("execute %ld\n", seconds2-seconds1);

	
	double max = 0;
	double min = 0;	
	for (int i = 0; i < nOut; i++)
	{
 		double a0 = ((double*)out[i])[0];
 		double a1 = ((double*)out[i])[1];
 		data1dD[i] = log(sqrt(a0 * a0 + a1 * a1));
 		max = (max < data1dD[i]) ? data1dD[i] : max;	
 		min = (min > data1dD[i]) ? data1dD[i] : min;	
	}
	
	rescaleForDisplay(data1dD, data1dD, nOut, p4DImage->getDatatype());
	
	//free memory
	fftw_destroy_plan(p);
	fftw_free(out);
	
	// output image 
	Image4DSimple outImage;
	unsigned char* dataOut1d = doubleArrayToCharArray(data1dD, nOut, p4DImage->getDatatype());
    outImage.setData(dataOut1d, nxOut, p4DImage->sz1, p4DImage->sz2, 1, p4DImage->getDatatype());
    v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &outImage);
	callback.setImageName(newwin,"fft test");
    callback.updateImageWindow(newwin);
	free(data1dD);
}
#endif

void WaveletPlugin::Cloning(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* p4DImage = callback.getImage(oldwin);
	if (!p4DImage)
	{
		QMessageBox::information(0, "Cloning", QObject::tr("No image is open."));
		return;
	}
	double* data1dD = channelToDoubleArray(p4DImage, 1);

	//get dims
    V3DLONG szx = p4DImage->getXDim();
    V3DLONG szy = p4DImage->getYDim();
    V3DLONG szz = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    V3DLONG N = szx * szy * szz;


	Image4DSimple outImage;
	unsigned char* dataOut1d = doubleArrayToCharArray(data1dD, N, p4DImage->getDatatype());	
    outImage.setData(dataOut1d, p4DImage );
    v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &outImage);
	callback.setImageName(newwin,"cloning test");
    callback.updateImageWindow(newwin);
}

void WaveletPlugin::WaveletTransform(V3DPluginCallback &callback, QWidget *parent)
{
	v3dhandle oldwin = callback.currentImageWindow();
	Image4DSimple* p4DImage = callback.getImage(oldwin);
	if (!p4DImage)
	{
		QMessageBox::information(0, "WaveletTransform", QObject::tr("No image is open."));
		return;
	}
	double* data1dD = channelToDoubleArray(p4DImage, 1);

	//get dims
    V3DLONG szx = p4DImage->getXDim();
    V3DLONG szy = p4DImage->getYDim();
    V3DLONG szz = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    V3DLONG N = szx * szy * szz;

	int numScales = 2;
 	//compute wavelet scales
 	double** resTab = NULL;
	try { 	
			time_t seconds0 = time (NULL);
 			if (szz>1)
 				resTab = WaveletTransform::b3WaveletScales(data1dD, szx, szy, szz, numScales);
 			else
 				resTab = WaveletTransform::b3WaveletScales2D(data1dD, szx, szy, numScales);
 			time_t seconds1 = time (NULL);
 			printf("\n Computation time : %d \n" , (seconds1-seconds0) );
 			}
 	catch(WaveletConfigException e)
 	{
 		printf("\nEXCEPTION\n %s \n" , e.what() );
 		return;
 	}

 	
 	//compute waveletCoefficients
 	double* lowPassResidual = new double[N];
 	WaveletTransform::b3WaveletCoefficientsInplace(resTab, data1dD, lowPassResidual, numScales, N);
	
	delete(data1dD);
		
	//reconstruct image from coefficients
	double* rec = new double[N];
	WaveletTransform::b3WaveletReconstruction(resTab, lowPassResidual, rec, numScales, N);
	

	
	//display reconstructed image
	rescaleForDisplay(rec, rec, N, p4DImage->getDatatype());
	unsigned char* dataOut1d = doubleArrayToCharArray(rec, N, p4DImage->getDatatype());
	Image4DSimple outImage;
	outImage.setData(dataOut1d, p4DImage );
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &outImage);
	callback.setImageName(newwin, "reconstruction");
	callback.updateImageWindow(newwin);
	delete(rec);

	
	//output wavelet coefficients
	for (int j = 0; j<numScales; j++)
	{
		//rescale
		double* out = resTab[j];
		rescaleForDisplay(out, out, N, p4DImage->getDatatype());
		unsigned char* dataOut1d = doubleArrayToCharArray(out, N, p4DImage->getDatatype());
    	Image4DSimple outImage;
    	outImage.setData(dataOut1d, p4DImage );
    	v3dhandle newwin = callback.newImageWindow();
		callback.setImage(newwin, &outImage);
		char buffer [50];
		sprintf(buffer, "wavelet scale %d", j+1);
		callback.setImageName(newwin, buffer);
    	callback.updateImageWindow(newwin);
	}
	for (int j = 0; j<numScales; j++)
 	{
 		free(resTab[j]);
 	}
	delete(resTab);

	//output low pass image
	rescaleForDisplay(lowPassResidual, lowPassResidual, N, p4DImage->getDatatype());
	unsigned char* dataOut1dL = doubleArrayToCharArray(lowPassResidual, N, p4DImage->getDatatype());
	Image4DSimple outImageL;
	outImageL.setData(dataOut1dL, p4DImage );
	v3dhandle newwinL = callback.newImageWindow();
	callback.setImage(newwinL, &outImageL);
	callback.setImageName(newwinL, "low pass residual");
	callback.updateImageWindow(newwinL);
	
	delete(lowPassResidual);	
}












