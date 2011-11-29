/**
 * scaleinfo.cpp
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

#include <QtGui>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <list>

#include "scaleinfo.h"
#include "wavelets.h"

ScaleInfo::~ScaleInfo(  )
{
	printf("SCALE INFO : destructing scaleInfo.\n");

	QLayoutItem *child;
	while ((child = gridLayout->takeAt(0)) != 0) {
		gridLayout->removeItem(child);
	}

	delete gridLayout ;
	delete enableCheckBox ;
	//delete thresholdSlider; // TODO: restore this delete.
	// delete thresholdValueLabel;
	delete groupBox ;

}

ScaleInfo::ScaleInfo( WaveletPlugin *waveletPlugin , int scaleNumber , QGroupBox *parent ) // TODO: Generalized it (might have som QContainer ?)
{
  this->threshold = 0;	
  this->thresholdValue = 0;
	
  this->waveletPlugin = waveletPlugin;
	printf( "%i \n", scaleNumber );
	groupBox = new QGroupBox( parent );
	gridLayout = new QGridLayout( groupBox );

	this->enable = true;

	char buff[50];
	sprintf(buff, "Scale %i", scaleNumber);

	groupBox->setTitle( buff );

	QLabel* thresholdLabel = new QLabel( "Threshold:" );
	thresholdValueLabel = new QLabel( "0" );
	thresholdValueLabel->setFixedSize(20,20);

	enableCheckBox = new QCheckBox("Enabled" );
	enableCheckBox->setChecked( true );
//	thresholdLineEdit = new QLineEdit("10");

	QSlider *thresholdSlider = new QSlider(Qt::Horizontal);
	thresholdSlider->setFocusPolicy(Qt::StrongFocus);
	thresholdSlider->setTickPosition(QSlider::TicksBothSides);
	thresholdSlider->setTickInterval(1);
	thresholdSlider->setSingleStep(1);
	thresholdSlider->setMaximum(50);
	thresholdSlider->setFixedSize(100,20);

	gridLayout->addWidget( enableCheckBox , 0 , 0 );
	gridLayout->addWidget( thresholdLabel , 0 , 1 );
	gridLayout->addWidget( thresholdValueLabel , 0 , 2 );
	gridLayout->addWidget( thresholdSlider , 0 , 3 );

	//dialog->
	this->connect(enableCheckBox, SIGNAL(clicked()), this, SLOT(enableButtonPressed()));
	this->connect(thresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChange(int)));
}

void ScaleInfo::enableButtonPressed()
{
	printf("enable button pressed\n");
//	thresholdSlider->setEnabled( enableCheckBox->isChecked() );
	enable = enableCheckBox->isChecked();
	waveletPlugin->updateWaveletAskedByGUI();
}

void ScaleInfo::sliderChange(int value )
{
	thresholdValue = value;
	printf( "%d" , thresholdValue );
	char *text = new char[50];
	sprintf( text , "%d" , value );
	thresholdValueLabel->setText( text );
	if (enable)
	{
		waveletPlugin->updateWaveletAskedByGUI();
	}
}











