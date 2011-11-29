/**
 * wavelets.h
 *
 * Written by
 *
 * Ihor Smal
 * Nicolas Chenouard
 * Fabrice de Chaumont
 *
 * Paper reference: ISBI and JC Ref
 *
 * This code is under GPL License
 *
 */
#ifndef EXTRAFILTERSPLUGIN_H
#define EXTRAFILTERSPLUGIN_H


#define USING_FFT 0 // Do not compile for FFT
//#define USING_FFT 1 // compile for FFT

#include <v3d_interface.h>
#include <QtGui>
#include <QtCore>
#include <list>
#include <v3d_basicdatatype.h>
#include <v3d_utils.h>

#if USING_FFT
#include <fftw3.h>
#endif

#include "scaleinfo.h"
#include <ioV3dUtils.h>
#include "waveletConfigException.h"
#include "waveletTransform.h"

class WaveletPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);

public:


    WaveletPlugin();
    void refreshScaleInterface();

    // interace.
    QDialog *myDialog;
    //QWidget *myDialog;
    QFormLayout *formLayout;
    QFormLayout *formLayoutGroupBox;
    QGroupBox *qBox; // box for scales stuff
    QProgressBar *progressBar;
    QSlider *thresholdResidualScaleSlider;
    QLabel* thresholdResidualScaleLabel ;

    QSlider *thresholdFinalSlider;
    QLabel* thresholdFinalLabel ;

    // wavelet input interface

    typedef std::list<LocationSimple*> LocationSimpleListType ;
    typedef std::list<ScaleInfo*> ListType ;
    ListType *scaleInfoList ;
    QPushButton* removeScaleButton;
    QPushButton* showOriginalButton;
    QCheckBox* liveUpdateCheckBox ;
    QCheckBox* useLowPassCheckBox ;


    // source image
    v3dhandle sourceWindow;
    Image4DSimple* sourceImage;
    V3DPluginCallback *myCallback;

    // denoising Image
    double* data1dD;
    double** resTab;
    void filterB3Wavelets(  bool displayDetection  );
    int numScales;
    V3DLONG szx;
    V3DLONG szy;
    V3DLONG szz;
    V3DLONG sc;
    V3DLONG N;
    double* lowPassResidual;
    bool scaleComputationReady ;

    double* lowPassResidualCopy;
    double** resTabCopy;

    // Copy of original Image;

    Image4DSimple *originalImageCopy;
    void copyOriginalImage();
    void restoreOriginalImage();

    // Plugin stuff

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}

	void initGUI( V3DPluginCallback &callback, QWidget *parent);

	void Cloning(V3DPluginCallback &callback, QWidget *parent);
	#if USING_FFT
	void FFT(V3DPluginCallback &callback, QWidget *parent);
	#endif
	void WaveletTransform(V3DPluginCallback &callback, QWidget *parent);

	int thresholdResidualScale ;
	int thresholdFinal ;

	// refresh CallBack

//	void CALLBACK callBack();

	void updateWavelet( );
	void updateWaveletAskedByGUI( );
	void computeWavelets( bool displayDetection );

public slots:
	void cancel();
	void sliderChange( int value );
	void addScaleButtonPressed();
	void removeScaleButtonPressed();
	void dev1ButtonPressed();
	void dev2ButtonPressed();
	void dev3ButtonPressed();
	void dev4ButtonPressed();
	void detectSpotsButtonPressed();
	void denoiseButtonPressed();
	void sliderResidualChange(int value);
	void showOriginalPressed( );
	void livePressed();
	void useLowPassPressed();
	void thresholdFinalSliderChange( int value );

};

#endif
