/****************************************************************************
 **
 dialog_watershed_para.cpp
 **
 ****************************************************************************/

#include <QDialog>

#include "dialog_watershed_para.h"
#include "seg_parameter.h"

#include "v3d_interface.h"


dialog_watershed_para::dialog_watershed_para(segParameter  *p, Image4DSimple *imgdata)
{
	if (!p || !imgdata) {qDebug("The input data of dialog_watershed_para::dialog_watershed_para() is invalid.\n"); return;}
	updateContent(p, imgdata);
}

void dialog_watershed_para::updateContent(segParameter  *p, Image4DSimple *imgdata)
{
	if (!p || !imgdata) {qDebug("The input data of dialog_watershed_para::updateContent() is invalid.\n"); return;}
	setupUi(this);

	spinBox_Channel->setRange(1, imgdata->getCDim()); //2 to change
	spinBox_Channel->setValue(p->channelNo+1);
	
	spinBox_MedianSz->setRange(0, 20);
	spinBox_MedianSz->setValue(p->medianFiltWid);

	spinBox_GaussianSz->setRange(0, 20);
	spinBox_GaussianSz->setValue(p->gaussinFiltWid);

	doubleSpinBox_GaussianSigma->setRange(0.1, 5);
	doubleSpinBox_GaussianSigma->setValue(p->gaussinFiltSigma);
	
	comboBox_Threshold->setCurrentIndex(p->adpatThreMethod);
	comboBox_Method->setCurrentIndex(p->segMethod);
}

void dialog_watershed_para::fetchData(segParameter  *p)
{
	if (!p) return;

	p->channelNo = spinBox_Channel->value()-1;
	p->channelNo = (p->channelNo < 0)?0:p->channelNo;
	
	p->medianFiltWid = spinBox_MedianSz->value();
	p->gaussinFiltWid = spinBox_GaussianSz->value();
	p->gaussinFiltSigma = doubleSpinBox_GaussianSigma->value();
	
	p->adpatThreMethod = comboBox_Threshold->currentIndex();
	p->segMethod = comboBox_Method->currentIndex();
}

