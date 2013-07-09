/* adaptive_median.h
 * 2013-07-08: create this program by Zhi Zhou
 * revised from regiongrow.h
*/


#ifndef __ADAPTIVEMEDIAN_H__
#define __ADAPTIVEMEDIAN_H__


//
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class AdaptiveMedianPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
     float getPluginVersion() const {return 1.2f;}
};


//define a simple dialog for choose Adaptive Median parameters
class AdaptiveMedianDialog : public QDialog
{
	Q_OBJECT

public:
	AdaptiveMedianDialog(V3DPluginCallback2 &cb, QWidget *parent)
	{
		//initialization of variables
		image = 0;
		gridLayout = 0;

		//

		v3dhandleList win_list = cb.getImageWindowList();

		if(win_list.size()<1)
		{
			QMessageBox::information(0, "Adaptive Median ", QObject::tr("No image is open."));
			return;
		}

		//create a dialog
		gridLayout = new QGridLayout();

		image = cb.getImage(cb.currentImageWindow());
		pRoiList=cb.getROI(cb.currentImageWindow());
		QString imageName = cb.getImageName(cb.currentImageWindow());
		label_imagename = new QLabel(imageName.prepend("You have selected the image [").append("]"));
		gridLayout->addWidget(label_imagename, 1,0,1,3);

		int c = image->getCDim();
		label_channel = new QLabel(QObject::tr("Choose a channel: "));
		gridLayout->addWidget(label_channel, 2,0);

		// channel info
		QStringList chList;
		if (c==1)
			chList << "red (ch 1)";
		else if(c==2)
			chList << "red (ch 1)" << "green (ch 2)";
		else if(c>=3)
			chList << "red (ch 1)" << "green (ch 2)" << "blue (ch 3)";
		for (int i=3;i<c;i++)
			chList << QString("ch ").setNum(i+1);
		combo_channel = new QComboBox(); combo_channel->addItems(chList);
		gridLayout->addWidget(combo_channel, 2,1);

		//
		label_th = new QLabel(QObject::tr("Choose a background threshold: "));
		gridLayout->addWidget(label_th, 4,0);

		QStringList thList;
		thList << "mean" << "mean+stdv" <<"usr defined";

		combo_th = new QComboBox(); combo_th->addItems(thList);
		gridLayout->addWidget(combo_th, 4,1);

		//
		spin_th = new QDoubleSpinBox();
		gridLayout->addWidget(spin_th, 4,2);

		spin_th->setEnabled(false);
		spin_th->setMaximum(65535); spin_th->setMinimum(-65535);

		//
		spin_th = new QDoubleSpinBox();
		gridLayout->addWidget(spin_th, 4,2);

		spin_th->setEnabled(false);
		spin_th->setMaximum(65535); spin_th->setMinimum(-65535);

		//
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		gridLayout->addWidget(cancel, 5,1);
		gridLayout->addWidget(ok,     5,2);

		setLayout(gridLayout);
		setWindowTitle(QString("Adaptive Median"));
		update(); //update at least once so to initialize the variables. by Hanchuan Peng, 2011-02-11
	}

	~AdaptiveMedianDialog(){}

	public slots:
	void update()
	{
		ch = combo_channel->currentIndex();

		th_idx = combo_th->currentIndex();

		if(th_idx == 2)
		{
			spin_th->setEnabled(true);
		}
		else
		{
			spin_th->setEnabled(false);
		}

		thresh = spin_th->text().toDouble();

		b_filtersmallobjs = (check_filter->isChecked()) ? true : false;
		spin_vol->setEnabled(b_filtersmallobjs);
		volsz = spin_vol->text().toInt();
	}

public:
	int ch, th_idx;
	int volsz;
	double thresh;
	Image4DSimple* image;
	ROIList pRoiList;
	bool b_filtersmallobjs;

	QGridLayout *gridLayout;

	QLabel* label_imagename;
	QLabel* label_channel;
	QComboBox* combo_channel;

	QLabel* label_th;
	QComboBox* combo_th;

	QDoubleSpinBox* spin_th;

	QCheckBox* check_filter;
	QSpinBox* spin_vol;

	QPushButton* ok;
	QPushButton* cancel;
};


#endif



