/* datatypecnvrt.h
 * 2011-01-21: create this program by Yang Yu
 */

// Upgraded to V3DPluginInterface2_1 by Jianlong Zhou, 2012-04-05

#ifndef __DATATYPECNVRT_H__
#define __DATATYPECNVRT_H__

//
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class DTCPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const;
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
          V3DPluginCallback2 & v3d, QWidget * parent);
};

class DTCDialog : public QDialog
{
	Q_OBJECT

public:
	DTCDialog(V3DPluginCallback2 &callback, QWidget *parent)
	{
		v3dhandleList win_list = callback.getImageWindowList();

		v3dhandle wincurr = callback.currentImageWindow(); //
		QString itemcurr = callback.getImageName(wincurr);
		int idxcurr = 0;

		QStringList items;
		for (int i=0; i<win_list.size(); i++)
		{
			QString item = callback.getImageName(win_list[i]);

			items << item;

			if(item.compare(itemcurr) == 0)
				idxcurr = i;
		}

		combo_subject =  new QComboBox(); combo_subject->addItems(items);
		combo_subject->setCurrentIndex(idxcurr);

		label_subject = new QLabel(QObject::tr("Image: "));

		isub = combo_subject->currentIndex();

		//
		label_sub_dt = new QLabel(QObject::tr("Which datatype convert to: "));

		QStringList dtList;
		dtList << "V3D_UINT8" << "V3D_UINT16" << "V3D_FLOAT32";

		combo_dt = new QComboBox(); combo_dt->addItems(dtList);

		//
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");

		// gridlayout
		QGridLayout* gridLayout = new QGridLayout();

		gridLayout->addWidget(label_subject, 0,0,1,1); gridLayout->addWidget(combo_subject, 0,1,1,4);
		gridLayout->addWidget(label_sub_dt, 1,0); gridLayout->addWidget(combo_dt, 1,1);

		gridLayout->addWidget(cancel, 3,3,Qt::AlignRight); gridLayout->addWidget(ok, 3,4,Qt::AlignRight);

		setLayout(gridLayout);
		setWindowTitle(QString("Image Stitching"));

		// signal and slot
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

		connect(combo_subject, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));

	}

	~DTCDialog(){}

public slots:
	void update()
	{
		isub = combo_subject->currentIndex();
		dt = combo_dt->currentIndex();
	}

public:

	int isub;
	int dt;

	QLabel* label_subject;
	QComboBox* combo_subject;

	QLabel* label_sub_dt;
	QComboBox* combo_dt;

	QPushButton* ok;
	QPushButton* cancel;

};

#endif



