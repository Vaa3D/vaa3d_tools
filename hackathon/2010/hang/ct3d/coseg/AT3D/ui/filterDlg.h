#ifndef FILTERDLG_H_H
#define FILTERDLG_H_H

#include <QtGui>
#include "ui_filters.h"
#include <map>
using namespace std;

class FilterDlg : public QDialog
{
	Q_OBJECT

signals:
	void valueChanged();
	
public:
	FilterDlg(QWidget *parent = 0);
	void setRange(int currentVolumeRange, int volumeRange, int lifeRange, int speedRange, int deformRange);
	const vector< pair<double, double> >& getData();

private slots:
	void accept();
	void reject();
	void onValueChanged();
	void setImmediate(bool);
	
private:
	Ui::FilterDlg ui;
	vector< pair<double, double> > filters;
	bool immediately;

};

#endif
