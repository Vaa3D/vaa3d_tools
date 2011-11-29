/* BilateralFilter.h
 * 2010-06-02: create this program by Lei Qu
 */

#ifndef __BILATERALFILTER_H__
#define __BILATERALFILTER_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class ITKBilateralFilterPlugin: public QObject, public V3DPluginInterface
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface)

public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}

};

class ITKBilateralFilterDialog: public QDialog
{
Q_OBJECT

public:
	ITKBilateralFilterDialog(Image4DSimple *p4DImage, QWidget *parent)
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
		setWindowTitle(QString("Bilateral Filter"));

		connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
	}

	~ITKBilateralFilterDialog()
	{
	}

public slots:

public:
	QGridLayout *gridLayout;

	QPushButton* ok;
	QPushButton* cancel;
};

#endif

