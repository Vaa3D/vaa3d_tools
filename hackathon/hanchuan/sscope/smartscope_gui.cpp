

#include "smartscope_gui.h"


void StimulatingDialog::copyDataOut(StimulatingParas *para)
{
	if (!para)
		return;

	para->NPULSE = nPulse;
	para->WAITING = waiting;
	para->SCANNING_RATE = scanning_rate;
	para->REGION_RADIUS = region_radius;

}




bool StimulatingParas::importKeywordString2ParameterValues(QString ss, QString vv)
{
	QString tss = ss.trimmed().toUpper();
	QString tvv = vv.trimmed();

	if (tss.isEmpty() || tvv.isEmpty())
	{
		v3d_msg("Either the keyword string or the value string is empty. Do nothing.\n");
		return false;
	}

	if (tss=="NPULSE"){this->NPULSE = tvv.toInt(); }
	else if (tss=="WAITING"){this->WAITING = tvv.toInt(); }
	else if (tss=="SCANNING_RATE"){this->SCANNING_RATE = tvv.toDouble();}
	else if (tss=="REGION_RADIUS"){this->REGION_RADIUS = tvv.toInt(); }

	return true;
}


bool StimulatingParas::load_ParaFile(QString openFileNameLabel)
{
	QFile file(openFileNameLabel);
	QString baseName = openFileNameLabel.section('/', -1);
	QString baseDir = openFileNameLabel;
	baseDir.chop(baseName.size());

	file.open(QIODevice::ReadOnly);
	QString tmp = file.readAll();
	file.close();

	if (tmp.isEmpty())
		return false;

	QStringList tmpList = tmp.split(QString("\n"));
	if (tmpList.size()<=0)
		return false; //must not be a valid file

	int cnt=0;
	for (int i=0;i<tmpList.size(); i++)
	{
		//printf("(%s)\n", tmpList.at(i).toAscii().data());
		QStringList itemList;

		if (tmpList.at(i).isEmpty() || tmpList.at(i).at(0)=='#') //a comment line, do nothing
			continue;

		itemList = tmpList.at(i).split("=");
		if (itemList.size()==2)
		{
			if (importKeywordString2ParameterValues(itemList.at(0).trimmed().toUpper(), itemList.at(1).trimmed())==true)
			{
				cnt++;
				continue;
			}
		}
	}

	return true; 
}	


bool StimulatingParas::save_ParaFile(QString openFileNameLabel)
{
	FILE * fp=0;
	fp = fopen((char *)qPrintable(openFileNameLabel), "wt");
	if (!fp)
	{
		v3d_msg(QString("Fail to open file %1 to write.").arg(openFileNameLabel));
		return false;
	}

	//now save
	fprintf(fp, "NPULSE=%d\n", this->NPULSE);
	fprintf(fp, "WAITING=%d\n", this->WAITING);	
	fprintf(fp, "SCANNING_RATE=%.4f\n", this->SCANNING_RATE);
	fprintf(fp, "REGION_RADIUS=%d\n", this->REGION_RADIUS);	

	//finish up
	if (fp) fclose(fp);
	return true;
}	

// RegiongrowDialog
RegiongrowDialog::RegiongrowDialog(V3DPluginCallback* cb, QWidget *parent)
{
	//initialization of variables
	image = 0;
	gridLayout = 0;
	
	v3dhandleList win_list = cb->getImageWindowList();
	
	if(win_list.size()<1) 
	{
		QMessageBox::information(0, "Label Objects", QObject::tr("No image is open."));
		return;
	}
	
	//create a dialog
	gridLayout = new QGridLayout();
	
	image = cb->getImage(cb->currentImageWindow());
	pRoiList=cb->getROI(cb->currentImageWindow());
	QString imageName = cb->getImageName(cb->currentImageWindow());
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
	combo_channel = new QComboBox(); 
	combo_channel->addItems(chList);
	gridLayout->addWidget(combo_channel, 2,1);
	
	//
	label_th = new QLabel(QObject::tr("Choose a threshold: ")); 
	gridLayout->addWidget(label_th, 4,0); 
	
	QStringList thList;
	thList << "mean" << "mean+stdv" << "usr defined";
	
	combo_th = new QComboBox(); 
	combo_th->addItems(thList);
	combo_th->setCurrentIndex(1);
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
	check_filter->setText(QObject::tr("Filtering out small-components (voxels): "));
	check_filter->setChecked(false);
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
	
	update(); //update at least once so to initialize the variables. by Hanchuan Peng, 2011-02-11
}

void RegiongrowDialog::update()
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