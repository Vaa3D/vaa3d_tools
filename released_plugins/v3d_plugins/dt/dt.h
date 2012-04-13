/* dt.h
 * distance transform of an image stack
 * revised from Fuhui Long's dt versions
 * 2010-04-16: by Hanchuan Peng
 */


#ifndef __DISTTRANFORM_H__
#define __DISTTRANFORM_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>

#include "v3d_interface.h"

class DtPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

//define a simple dialog for choose DT parameters
class DtDialog : public QDialog
{
	Q_OBJECT

public:
	int ch;
	bool b_use_1stmarker; //whether or not use the intensity value at the first marker location as the target value
	bool b_rescale; //if rescale the value of the output

	QGridLayout *gridLayout;
	QLabel* label_imagename;
	QLabel* label_channel;
	QComboBox* combo_channel;
	QCheckBox* check_marker;
	QCheckBox* check_rescale;
	QPushButton* ok;
	QPushButton* cancel;

public:
	DtDialog(V3DPluginCallback2 &cb, QWidget *parent)
	{
		//create a dialog
		gridLayout = new QGridLayout();

		Image4DSimple* image = cb.getImage(cb.currentImageWindow());
		QString imageName = cb.getImageName(cb.currentImageWindow());
		label_imagename = new QLabel(imageName.prepend("Your have selected the image [").append("]"));
		gridLayout->addWidget(label_imagename, 1,0,1,2);

		int c = image->getCDim();
		label_channel = new QLabel(QObject::tr("Choose a channel: "));
		gridLayout->addWidget(label_channel, 2,0,1,2);

		// channel info
		QStringList chList;
		if(c>=3)
			chList << "red" << "green" << "blue";
		else if(c==2)
			chList << "red" << "green";
		else if(c==1)
			chList << "red";
		combo_channel = new QComboBox(); combo_channel->addItems(chList);
		gridLayout->addWidget(combo_channel, 3,0,1,2);

		check_marker = new QCheckBox();
		check_marker->setText(QObject::tr("Use the 1st marker's intensity to define the 'img background' (if checked OFF, then use intensity 0 to indicate background)"));
		check_marker->setChecked(false);
		gridLayout->addWidget(check_marker, 4,0,1,2);

		check_rescale = new QCheckBox();
		check_rescale->setText(QObject::tr("Rescale output image intensity to [0,255]"));
		check_rescale->setChecked(false);
		gridLayout->addWidget(check_rescale, 5,0,1,1);

		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		gridLayout->addWidget(cancel, 6,0,1,1);
		gridLayout->addWidget(ok,     6,1,1,1);

		setLayout(gridLayout);
		setWindowTitle(QString("Distance transform"));

		//slot interface
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

		connect(combo_channel, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(check_marker, SIGNAL(stateChanged(int)), this, SLOT(update()));
		connect(check_rescale, SIGNAL(stateChanged(int)), this, SLOT(update()));
	}

	~DtDialog(){}

	public slots:
	void update()
	{
		ch = combo_channel->currentIndex();
		b_use_1stmarker = (check_marker->isChecked()) ?  true : false;
		b_rescale = (check_rescale->isChecked())? true : false;
	}
};


#endif

