/* RegistrationBSpline.h
 * 2010-06-04: create this program by Lei Qu
 */

#ifndef __REGISTRATIONBSPLINE_H__
#define __REGISTRATIONBSPLINE_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class ITKRegistrationBSplinePlugin: public QObject, public V3DPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface)

public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}

};

class ITKRegistrationBSplineDialog: public QDialog
{
Q_OBJECT

public:
	ITKRegistrationBSplineDialog(Image4DSimple *p4DImage, QWidget *parent)
	{
		if (!p4DImage)
			return;

		printf("Passing data to data1d\n");

		ok = new QPushButton("OK");
		cancel = new QPushButton("Cancel");

		gridLayout = new QGridLayout();

		gridLayout->addWidget(cancel, 0, 0);
		gridLayout->addWidget(ok, 0, 1);
		setLayout( gridLayout);
		setWindowTitle(QString("BSpline Registration"));

		connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
	}

	~ITKRegistrationBSplineDialog()
	{
	}

public slots:

public:
	QGridLayout *gridLayout;

	QPushButton* ok;
	QPushButton* cancel;
};

#endif

