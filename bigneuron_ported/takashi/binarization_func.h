#ifndef __TEST_FUNC_H__
#define __TEST_FUNC_H__

#include <QtGui>
#include <v3d_interface.h>

struct input_PARA
{
	QString inimg_file;
	V3DLONG channel, l, d;
};

void binarization_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu, int mode);


//define a simple dialog for choose DT parameters
class DSLTDialog : public QDialog
{
	Q_OBJECT

public:
	QGridLayout *gridLayout;

	QLabel *labell;
	QLabel *labeld;
	QSpinBox* Dlevel;
	QSpinBox* Dnumber;

	QPushButton* ok;
	QPushButton* cancel;

	V3DLONG Dl;
	V3DLONG Dn;
	
public:
	DSLTDialog(V3DPluginCallback2 &cb, QWidget *parent)
	{
		Image4DSimple* image = cb.getImage(cb.currentImageWindow());
		QString imageName = cb.getImageName(cb.currentImageWindow());
		//create a dialog
		Dlevel = new QSpinBox();
		Dnumber = new QSpinBox();

		Dlevel->setMaximum(10); Dlevel->setMinimum(1); Dlevel->setValue(2);
		Dnumber->setMaximum(255); Dnumber->setMinimum(1); Dnumber->setValue(4);

		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		gridLayout = new QGridLayout();

		labell = new QLabel(QObject::tr("quality"));
		labeld = new QLabel(QObject::tr("number of sampling points"));

		gridLayout->addWidget(labell, 0,0); gridLayout->addWidget(Dlevel, 0,1);
		gridLayout->addWidget(labeld, 1,0); gridLayout->addWidget(Dnumber, 1,1);

		gridLayout->addWidget(cancel, 6,1); gridLayout->addWidget(ok, 6,0);
		setLayout(gridLayout);
		setWindowTitle(QString("Change parameters"));

		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

		//slot interface
		connect(Dlevel, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(Dnumber,SIGNAL(valueChanged(int)), this, SLOT(update()));
	}

	~DSLTDialog(){}

	public slots:
		void update();
};


#endif