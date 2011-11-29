#include <QtGui>
#include <QtCore>

#include <math.h>
#include <stdlib.h>
#include <list>

#include <v3d_basicdatatype.h>
#include "../basic_c_fun/basic_landmark.h"
#include "../basic_c_fun/v3d_basicdatatype.h"
#include "crashTest3.h"

Q_EXPORT_PLUGIN2(newwindow123WWD, CrashTestPlugin3);

const QString title = "CrashTest 3";

QStringList CrashTestPlugin3::menulist() const
{
    return QStringList()
    << tr("Crash Test 3");
}


void CrashTestPlugin3::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
	myCallback = &callback;
	sourceWindow = callback.currentImageWindow();
	sourceImage = callback.getImage(sourceWindow);

	myDialog = new QDialog(parent);
	QPushButton* processButton     = new QPushButton("process a black image test");
	QPushButton* restoreButton     = new QPushButton("Restore");

	QFormLayout *formLayout = new QFormLayout( myDialog );
	formLayout->addRow( processButton , restoreButton );

	myDialog->connect(processButton, SIGNAL(clicked()), this, SLOT( processBlackImage() ));
	myDialog->connect(restoreButton, SIGNAL(clicked()), this, SLOT( restore() ));

	copyImage();

	myDialog->exec();
}

void CrashTestPlugin3::restore()
{
	printf("\nrestore copy button\n");
	restoreCopy();
}

void CrashTestPlugin3::processBlackImage()
{
	printf("\nprocess black image\n");
	int n = sourceImage->getTotalBytes();
	unsigned char *data = sourceImage->getRawData();
	for ( int i = 0 ; i < n ; i++ )
	{
		data[i] = 0;
	}
	myCallback->updateImageWindow(sourceWindow);
}

void CrashTestPlugin3::copyImage()
{
	printf("copy image\n");
	sourceImageCopy = new Image4DSimple();
	unsigned char *bufferSource = sourceImage->getRawData();
	unsigned char *bufferCopy = new unsigned char[sourceImage->getTotalBytes()];
	memcpy( bufferCopy , bufferSource , sourceImage->getTotalBytes() );

	sourceImageCopy->setData( bufferCopy , sourceImage->getXDim() , sourceImage->getYDim(),
			sourceImage->getZDim(), sourceImage->getCDim() , sourceImage->getDatatype()
			);

}

void CrashTestPlugin3::restoreCopy()
{
	printf("restore copy\n");
	// TODO: regarder si ce n'est pas le pointeur de ce foutu datatype qu'est pas pete des fois

	sourceImage->setXDim( sourceImageCopy->getXDim() );
	sourceImage->setYDim( sourceImageCopy->getYDim() );
	sourceImage->setZDim( sourceImageCopy->getZDim() );
	sourceImage->setCDim( sourceImageCopy->getCDim() );
	sourceImage->setDatatype( sourceImageCopy->getDatatype() );


	// MAC OS Version **************************************************************
/*
//	memcpy( sourceImage->getRawData() , originalImageCopy->getRawData() , originalImageCopy->getTotalBytes() );

	printf("%d source\n" , sourceImage->getTotalBytes() );
	printf("%d original\n" , originalImageCopy->getTotalBytes() );
	printf("\n");

	myCallback->setImage(sourceWindow, originalImageCopy );
	myCallback->updateImageWindow(sourceWindow);
*/
	// WINDOWS Version ************************************************************

	memcpy( sourceImage->getRawData() , sourceImageCopy->getRawData() , sourceImageCopy->getTotalBytes() );

	printf("%dl source\n" , sourceImage->getTotalBytes() );
	printf("%dl original\n" , sourceImageCopy->getTotalBytes() );
	printf("\n");

	myCallback->setImage(sourceWindow, sourceImage );
	myCallback->updateImageWindow(sourceWindow);

	}












