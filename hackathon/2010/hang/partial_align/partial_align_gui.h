#ifndef PARTIAL_ALIGN_GUI_H_H
#define PARTIAL_ALIGN_GUI_H_H

#include <QtGui>
#include <v3d_interface.h>

class PartialAlignDialog : public QDialog
{
	Q_OBJECT
	
public:
	PartialAlignDialog(V3DPluginCallback &callback, QWidget *parent)
	{
		v3dhandleList win_list = callback.getImageWindowList();
		int curindex = 0;
		for(curindex = 0; curindex < win_list.size(); curindex++)
		{
			if(win_list[curindex] == callback.currentImageWindow()) break;
		}

		QStringList items;
		for(int i=0; i < win_list.size(); i++) items << callback.getImageName(win_list[i]);

		// create a dialog
		combo_subject = new QComboBox(); combo_subject->addItems(items);
		combo_subject->setCurrentIndex(curindex);

		label_subject = new QLabel(QObject::tr("Subject Image : "));

		i1 = combo_subject->currentIndex();

		combo_target =  new QComboBox(); combo_target->addItems(items);
		combo_target->setCurrentIndex(curindex);

		label_target = new QLabel(QObject::tr("Target Image : "));

		i2 = combo_target->currentIndex();

		//
		label_sub_channel = new QLabel(QObject::tr("Which channel of the image 1: "));
		label_tar_channel = new QLabel(QObject::tr("Which channel of the image 2: "));

		channel_sub = new QSpinBox();
		channel_tar = new QSpinBox();

		//Image4DSimple* subject = callback.getImage(win_list[i1]);
		//Image4DSimple* target  = callback.getImage(win_list[i2]);

		channel_sub->setMaximum(4); channel_sub->setMinimum(1); channel_sub->setValue(1);
		channel_tar->setMaximum(4); channel_tar->setMinimum(1); channel_tar->setValue(1);

		//
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");

		// gridlayout
		QGridLayout* gridLayout = new QGridLayout();

		gridLayout->addWidget(label_subject, 0,0,1,1); gridLayout->addWidget(combo_subject,0,1,1,5);
		gridLayout->addWidget(label_sub_channel, 1,0); gridLayout->addWidget(channel_sub, 1, 1);

		gridLayout->addWidget(label_target, 2,0,1,1); gridLayout->addWidget(combo_target,2,1,1,5);
		gridLayout->addWidget(label_tar_channel, 3,0); gridLayout->addWidget(channel_tar,3,1);

		gridLayout->addWidget(cancel,5,4,Qt::AlignRight); gridLayout->addWidget(ok, 5,5,Qt::AlignRight);

		setLayout(gridLayout);

		// signal and slot
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

		connect(combo_subject, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		connect(combo_target, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));

		connect(channel_sub, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(channel_tar, SIGNAL(valueChanged(int)), this, SLOT(update()));

		// stupid method to avoid warning
		parent = parent; 
	}

	~PartialAlignDialog(){}
	public slots:
		void update()
		{
			i1 = combo_subject->currentIndex();
			i2 = combo_target->currentIndex();

			sub_c = channel_sub->text().toInt();
			tar_c = channel_tar->text().toInt();
		}

public:

	int i1, i2;
	int sub_c, tar_c;

	QLabel* label_subject;
	QComboBox* combo_subject;

	QLabel* label_target;
	QComboBox* combo_target;

	QLabel *label_sub_channel;
	QLabel *label_tar_channel;

	QSpinBox *channel_sub;
	QSpinBox *channel_tar;

	QPushButton* ok;
	QPushButton* cancel;
};

#endif
