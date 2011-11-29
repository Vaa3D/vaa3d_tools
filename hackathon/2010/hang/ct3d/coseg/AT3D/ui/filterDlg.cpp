#include <QtGui>
#include <QDoubleValidator>
#include <iostream>
#include "filterDlg.h"
using namespace std;

FilterDlg::FilterDlg(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	immediately = 1;
}

void FilterDlg::setRange(int currentVolumeRange, int volumeRange, int lifeRange, int speedRange, int deformRange)
{
	filters.push_back(make_pair(0.0,(double)currentVolumeRange));
	filters.push_back(make_pair(0.0,(double)volumeRange));	
	filters.push_back(make_pair(0.0,(double)lifeRange));	
	filters.push_back(make_pair(0.0,(double)speedRange));	
	filters.push_back(make_pair(0.0,(double)deformRange));	

	ui.minCurrentVolume->setRange(0,currentVolumeRange);
	ui.maxCurrentVolume->setRange(0,currentVolumeRange);
	ui.maxCurrentVolume->setValue(currentVolumeRange);
	
	ui.minVolume->setRange(0, volumeRange);
	ui.maxVolume->setRange(0, volumeRange);
	ui.maxVolume->setValue(volumeRange);
	
	ui.minLife->setRange(0, lifeRange);
	ui.maxLife->setRange(0, lifeRange);
	ui.maxLife->setValue(lifeRange);
	
	ui.minSpeed->setRange(0, speedRange*100);
	ui.maxSpeed->setRange(0, speedRange*100);
	ui.maxSpeed->setValue(speedRange*100);
	
	ui.minDeform->setRange(0, deformRange*100);
	ui.maxDeform->setRange(0, deformRange*100);
	ui.maxDeform->setValue(deformRange*100);
	
	ui.currentVolumeRange->setText(tr("0 ~ %1").arg(currentVolumeRange));
	ui.volumeRange->setText(tr("0 ~ %1").arg(volumeRange));
	ui.lifeRange->setText(  tr("0 ~ %1").arg(lifeRange));
	ui.speedRange->setText( tr("0 ~ %1*100").arg(speedRange));
	ui.deformRange->setText(tr("0 ~ %1*100").arg(deformRange));
}

const vector< pair<double,double> >& FilterDlg::getData()
{
	return filters;
}

void FilterDlg::accept()
{	
	if(ui.currentVolumeChecker->isChecked())
	{
		filters[0].first = ui.minCurrentVolume->value();
		filters[0].second = ui.maxCurrentVolume->value();
	}
	if(ui.volumeChecker->isChecked())
	{
		filters[1].first = ui.minVolume->value();
		filters[1].second = ui.maxVolume->value();
	}
	if(ui.lifeChecker->isChecked())
	{
		filters[2].first = ui.minLife->value();
		filters[2].second = ui.maxLife->value();
	}
	if(ui.speedChecker->isChecked())
	{
		filters[3].first = ui.minSpeed->value() / 100.0;
		filters[3].second = ui.maxSpeed->value() / 100.0;
	}
	if(ui.deformChecker->isChecked())
	{
		filters[4].first = ui.minDeform->value() / 100.0;
		filters[4].second = ui.maxDeform->value() / 100.0;
	}

	return QDialog::accept();
}

void FilterDlg::reject()
{
	return QDialog::reject();
}

void FilterDlg::onValueChanged()
{
	if(ui.currentVolumeChecker->isChecked())
	{
		filters[0].first = ui.minCurrentVolume->value();
		filters[0].second = ui.maxCurrentVolume->value();
	}
	if(ui.volumeChecker->isChecked())
	{
		filters[1].first = ui.minVolume->value();
		filters[1].second = ui.maxVolume->value();
	}
	if(ui.lifeChecker->isChecked())
	{
		filters[2].first = ui.minLife->value();
		filters[2].second = ui.maxLife->value();
	}
	if(ui.speedChecker->isChecked())
	{
		filters[3].first = ui.minSpeed->value() / 100.0;
		filters[3].second = ui.maxSpeed->value() / 100.0;
	}
	if(ui.deformChecker->isChecked())
	{
		filters[4].first = ui.minDeform->value() / 100.0;
		filters[4].second = ui.maxDeform->value() / 100.0;
	}
	if(immediately) emit valueChanged();
}

void FilterDlg::setImmediate(bool _immediately)
{
	immediately = _immediately;
}

