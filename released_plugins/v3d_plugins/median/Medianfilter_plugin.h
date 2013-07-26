/* Medianfilter_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2013-06-10 : by Zhi Zhou
 */
 
#ifndef __MEDIANFILTER_PLUGIN_H__
#define __MEDIANFILTER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fstream>
#include <iostream>

class MedianFilterPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
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
				QMessageBox::information(0, "Adaptive Median", QObject::tr("No image is open."));
				return;
			}
			
			//create a dialog
			gridLayout = new QGridLayout();
			
			image = cb.getImage(cb.currentImageWindow());

            if (!image || !image->valid())
            {
                v3d_msg("The image is not valid yet. Check your data.");
                return;
            }

			pRoiList=cb.getROI(cb.currentImageWindow());
			QString imageName = cb.getImageName(cb.currentImageWindow());
			label_imagename1 = new QLabel(imageName.prepend("You have selected the image [").append("]"));
			gridLayout->addWidget(label_imagename1, 1,0,1,3);

			label_imagename2 = new QLabel(QObject::tr("This feature will use the Grayscale Distance Tranform (GSDT) plugin to determine the local window size adaptively."));
			gridLayout->addWidget(label_imagename2, 2,0,1,3);
			
			label_imagename3 = new QLabel(QObject::tr("Thus please specify the following information to run GSDT."));
			gridLayout->addWidget(label_imagename3, 3,0,1,3);

			int c = image->getCDim();
           		 label_channel = new QLabel(QObject::tr("Choose a channel: "));
			gridLayout->addWidget(label_channel, 4,0);
			
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
			gridLayout->addWidget(combo_channel, 4,1);
			
			//
			label_th = new QLabel(QObject::tr("Choose a background threshold (0~255): "));
			gridLayout->addWidget(label_th, 6,0);
			
			QStringList thList;
			thList << "mean" << "usr defined";
			
			combo_th = new QComboBox(); combo_th->addItems(thList);
			gridLayout->addWidget(combo_th, 6,1);
			
			//
			spin_th = new QDoubleSpinBox();
			gridLayout->addWidget(spin_th, 6,2);
			
			spin_th->setEnabled(false);
			spin_th->setMaximum(255); spin_th->setMinimum(0);
			//
			ok     = new QPushButton("OK");
			cancel = new QPushButton("Cancel");
			gridLayout->addWidget(cancel, 7,1);
			gridLayout->addWidget(ok,     7,2);
			
			setLayout(gridLayout);
			setWindowTitle(QString("Adaptive Median"));
			
			//slot interface
			connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
			connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
			
			connect(combo_channel, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
			connect(combo_th, SIGNAL(currentIndexChanged(int)), this, SLOT(update()));
			
			connect(spin_th, SIGNAL(valueChanged(double)), this, SLOT(update()));
			update(); //update at least once so to initialize the variables. by Hanchuan Peng, 2011-02-11
		}
		
		~AdaptiveMedianDialog(){}
		
		public slots:
		void update()
		{
			ch = combo_channel->currentIndex();
			
			th_idx = combo_th->currentIndex();
			
			if(th_idx == 1)
			{
				spin_th->setEnabled(true);
			}
			else
			{
				spin_th->setEnabled(false);
			}
			
			thresh = spin_th->text().toDouble();

		}
		
	public:
		int ch, th_idx;
		int volsz;
		double thresh;
		Image4DSimple* image;
		ROIList pRoiList;
		bool b_filtersmallobjs;
		
		QGridLayout *gridLayout;
		
		QLabel* label_imagename1;
		QLabel* label_imagename2;
         	QLabel* label_imagename3;
		QLabel* label_channel;
		QComboBox* combo_channel;
		
		QLabel* label_th;
		QComboBox* combo_th;
		
		QDoubleSpinBox* spin_th;
		
		QPushButton* ok;
		QPushButton* cancel;
	};




#endif

