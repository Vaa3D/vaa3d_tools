#ifndef __NEURON_DIST_GUI_H__
#define __NEURON_DIST_GUI_H__
#include <QtGui>
#include "basic_surf_objs.h"

class SelectNeuronDlg : public QDialog
{
	Q_OBJECT

public:
	SelectNeuronDlg(QWidget * parent);

	QLineEdit * line_edit1;
	QLineEdit * line_edit2;
	NeuronTree nt1;
	NeuronTree nt2;
	QString name_nt1;
	QString name_nt2;

public slots:
	void _slots_openFileDlg1();
	void _slots_openFileDlg2();
	void _slots_runPlugin();

};

#endif
