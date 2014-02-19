#ifndef __LABEL_OBJECTS_DIALOG_H__
#define __LABEL_OBJECTS_DIALOG_H__

#include "v3d_interface.h"

class LabelImgObjectParaDialog : public QDialog
{
	Q_OBJECT
	
public:
	LabelImgObjectParaDialog(V3DPluginCallback2 &cb, QWidget *parent)
	{
		image = 0;
		gridLayout = 0;
		
		v3dhandleList win_list = cb.getImageWindowList();
		
		if(win_list.size()<1) 
		{
			QMessageBox::information(0, "Label Objects", QObject::tr("No image is open."));
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
			chList << QString("ch 4").setNum(i+1);
		combo_channel = new QComboBox(); combo_channel->addItems(chList);
		gridLayout->addWidget(combo_channel, 2,1);
		
		//
		label_th = new QLabel(QObject::tr("Choose a threshold: ")); 
		gridLayout->addWidget(label_th, 4,0); 
		
		QStringList thList;
		thList << "mean" << "mean+stdv" << "usr defined";
		
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
		check_filter = new QCheckBox();
		check_filter->setText(QObject::tr("Filtering out small objects (voxels): "));
		check_filter->setChecked(true);
		gridLayout->addWidget(check_filter, 5,0); 
		
		spin_vol = new QSpinBox();
		gridLayout->addWidget(spin_vol, 5,1);
		
		spin_vol->setEnabled(false);
		spin_vol->setMaximum(1.0737e+09); spin_vol->setMinimum(0); spin_vol->setValue(200); 
		
		//
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		gridLayout->addWidget(cancel, 6,1); 
		gridLayout->addWidget(ok,     6,2);
		
		setLayout(gridLayout);
		setWindowTitle(QString("Label Objects"));
		
		//slot interface
		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		
		connect(combo_channel, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		connect(combo_th, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
		
		connect(spin_th, SIGNAL(valueChanged(double)), this, SLOT(update()));
		
		connect(check_filter, SIGNAL(stateChanged(int)), this, SLOT(update()));
		connect(spin_vol, SIGNAL(valueChanged(int)), this, SLOT(update()));

		update(); //update at least once so to initialize the variables
	}
	
	~LabelImgObjectParaDialog(){}
	
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

