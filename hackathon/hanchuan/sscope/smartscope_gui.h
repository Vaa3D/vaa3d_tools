/* smartscope_gui.h
 * some GUI components of the SmartScope project. by Hanchuan Peng
 */


#ifndef __SMARTSCOPE_GUI_H__
#define __SMARTSCOPE_GUI_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <v3d_interface.h>

//log file IO
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>

using namespace std;

//
#include <cmath>
#include <ctime>
#include <vector>

#define INF 1E9
#define PI 3.14159265

#define WAITING_PERSLICE 10 // millisecond
#define WAIT_TIME_PERLINE 1 // millisecond

#define R_CUBE 3
#define VOLTAGE_MAX_XY 10
#define VOLTAGE_MAX 10
#define EPS 1E-3

#define INVALID_VALUE 999

#define max(x,y) ((x)>(y))?(x):(y)
#define min(x,y) ((x)<(y))?(x):(y)

//=============================================================================


// log
class LogConfig
{
public:
	LogConfig()
	{
		// load log
		QString m_FileName = "C:\\mi_configuration.log";
		if(!QFile(m_FileName).exists())
		{
			m_FileName = QFileDialog::getOpenFileName(0, QObject::tr("Find Configuration file"),
													  "/Documents",
													  QObject::tr("Image Configuration (*.log)"));
			if(m_FileName.isEmpty())
				return;
		}
		string filename =m_FileName.toStdString();

		//
		ifstream pFile(filename.c_str());
		string str;
		
		if(pFile.is_open())
		{
			while( !pFile.eof() )
			{

				if(getline(pFile, str))
				{
					istringstream iss(str);

					iss >> dev_x; iss >> ao_x;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
					
					iss >> dev_y; iss >> ao_y;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> dev_z; iss >> ao_z;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> aictr_red;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> aictr_green;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> dev_red; iss >> red_n_ai; iss >> red_n_ctr;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> dev_green; iss >> green_n_ai; iss >> green_n_ctr;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> dev_timing; iss >> timing_n;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> dev_clk; iss >> clk_n;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> min_fovx; iss >> max_fovx; iss >> fov_step_x;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> min_fovy; iss >> max_fovy; iss >> fov_step_y;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> min_fovz; iss >> max_fovz; iss >> fov_step_z;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> ratio_x; iss >> ratio_y; iss >> ratio_z; 
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> time_out; 
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> scanning_rate; 
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> sx; iss >> sy; iss >> sz;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> ch;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> marker_sorted;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
					
					iss >> start_x; iss >> start_y; iss >> end_x; iss >> end_y;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> scanMethod;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> b_useSimImg;
				}

			}
		}
		else
		{
			cout << "Unable to open the file";
			return;
		}
		pFile.close();

		sc = (ch<2)?1:2;; //
		duty_cycle = 0.5; //		
	}

	~LogConfig(){}

public:
	int dev_x, dev_y, dev_z, ao_x, ao_y, ao_z, dev_red, red_n_ai, red_n_ctr, dev_green, green_n_ai, green_n_ctr, dev_clk, clk_n, dev_timing, timing_n, time_out, scanning_rate;
	double min_fovx, max_fovx, min_fovy, max_fovy, min_fovz, max_fovz;
	double ratio_x, ratio_y, ratio_z;
	long sx, sy, sz, sc;
	int ch;
	int aictr_red;
	int aictr_green;
	int marker_sorted;
	double fov_step_x, fov_step_y, fov_step_z;
	long start_x, start_y, end_x, end_y;
	double duty_cycle;
	int scanMethod;
	int b_useSimImg;
};

// log for roi
class LogConfigRoi
{
public:
	LogConfigRoi(string filename)
	{
		// load log
		ifstream pFile(filename.c_str());
		string str;

		QString m_FileName = "C:\\mi_configuration.log";
		string filename_pre =m_FileName.toStdString();
		
		if(pFile.is_open())
		{
			while( !pFile.eof() )
			{

				if(getline(pFile, str))
				{
					istringstream iss(str);

					iss >> dev_x; iss >> ao_x;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
					
					iss >> dev_y; iss >> ao_y;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> dev_z; iss >> ao_z;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> aictr_red;
				}


				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> aictr_green;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> dev_red; iss >> red_n_ai; iss >> red_n_ctr;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> dev_green; iss >> green_n_ai; iss >> green_n_ctr;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> dev_timing; iss >> timing_n;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> dev_clk; iss >> clk_n;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> min_fovx; iss >> max_fovx; iss >> fov_step_x;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> min_fovy; iss >> max_fovy; iss >> fov_step_y;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> min_fovz; iss >> max_fovz; iss >> fov_step_z;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> ratio_x; iss >> ratio_y; iss >> ratio_z; 
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> time_out; 
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> scanning_rate; 
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> sx; iss >> sy; iss >> sz;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> ch;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> marker_sorted;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
					
					iss >> start_x; iss >> start_y; iss >> end_x; iss >> end_y;
				}

				if(getline(pFile, str) && strcmp(filename.c_str(), filename_pre.c_str()))
				{
					istringstream iss(str);
					
					iss >> rez_x; iss >> rez_y; iss >> rez_z; iss >> origin_x; iss >> origin_y; iss >> origin_z;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> scanMethod;
				}

				if(getline(pFile, str))
				{
					istringstream iss(str);
				
					iss >> b_useSimImg;
				}
			}
		}
		else
		{
			cout << "Unable to open the file";
			return;
		}
		pFile.close();

		sc = (ch<2)?1:2; //
		duty_cycle = 0.5; //

	}

	~LogConfigRoi(){}

public:
	void write(string filename)
	{
		FILE *pFileOut=0;
		
		pFileOut = fopen(filename.c_str(),"wt");
		if (pFileOut == NULL) perror ("Error opening writing file!");
		else
		{
			fprintf(pFileOut, "%d %d \n", dev_x, ao_x);
			fprintf(pFileOut, "%d %d \n", dev_y, ao_y);
			fprintf(pFileOut, "%d %d \n", dev_z, ao_z);

			fprintf(pFileOut, "%d \n", aictr_red);
			fprintf(pFileOut, "%d \n", aictr_green);

			fprintf(pFileOut, "%d %d %d \n", dev_red, red_n_ai, red_n_ctr);
			fprintf(pFileOut, "%d %d %d \n", dev_green, green_n_ai, green_n_ctr);

			fprintf(pFileOut, "%d %d \n", dev_timing, timing_n);
			fprintf(pFileOut, "%d %d \n", dev_clk, clk_n);

			fprintf(pFileOut, "%lf %lf %lf \n", min_fovx, max_fovx, fov_step_x);
			fprintf(pFileOut, "%lf %lf %lf \n", min_fovy, max_fovy, fov_step_y);
			fprintf(pFileOut, "%lf %lf %lf \n", min_fovz, max_fovz, fov_step_z);

			fprintf(pFileOut, "%lf %lf %lf \n", ratio_x, ratio_y, ratio_z);

			fprintf(pFileOut, "%lf \n", time_out);
			fprintf(pFileOut, "%lf \n", scanning_rate);

			fprintf(pFileOut, "%ld %ld %ld \n", sx, sy, sz);

			fprintf(pFileOut, "%d \n", ch);
			fprintf(pFileOut, "%d \n", marker_sorted);

			fprintf(pFileOut, "%ld %ld %ld %ld \n", start_x, start_y, end_x, end_y );

			fprintf(pFileOut, "%lf %lf %lf %lf %lf %lf \n", rez_x, rez_y, rez_z, origin_x, origin_y, origin_z);

			fprintf(pFileOut, "%d \n", scanMethod);
			fprintf(pFileOut, "%d \n", b_useSimImg);
		}
		fclose(pFileOut);
	}

public:
	int dev_x, dev_y, dev_z, ao_x, ao_y, ao_z, dev_red, red_n_ai, red_n_ctr, dev_green, green_n_ai, green_n_ctr, dev_clk, clk_n, dev_timing, timing_n;
	double time_out, scanning_rate;
	double min_fovx, max_fovx, min_fovy, max_fovy, min_fovz, max_fovz;
	double ratio_x, ratio_y, ratio_z;
	long sx, sy, sz, sc;
	int ch;
	int aictr_red;
	int aictr_green;
	int marker_sorted;
	double fov_step_x, fov_step_y, fov_step_z;
	long start_x, start_y, end_x, end_y;
	double duty_cycle;

	double rez_x, rez_y, rez_z, origin_x, origin_y, origin_z;

	int scanMethod; // 0:single direction with acc, 1:zigzag_acc ZJL
	int b_useSimImg; // create simulation data for testing
};


#define NPLUSE_MAX 100000
#define NPLUSE_MIN 0
//#define NPLUSE_DEFAULT 6
#define NPLUSE_STEP 1

#define STIMULATION_TIME_MAX 40000
#define STIMULATION_TIME_MIN 1
//#define STIMULATION_TIME_DEFAULT 1
#define STIMULATION_TIME_STEP 1

#define SCAN_RATE_MAX 10000000
#define SCAN_RATE_MIN 100
//#define SCAN_RATE_DEFAULT 1000
#define SCAN_RATE_STEP 1

#define SCAN_REGION_SIZE_MAX 1000


class StimulatingParas
{
public:
	double SCANNING_RATE;
	int NPULSE;
	int WAITING;
	int REGION_RADIUS;
public:
	StimulatingParas()
	{
		SCANNING_RATE=0.0;
		NPULSE=0;
		WAITING=0;
		REGION_RADIUS=0;
	}
private:
	bool importKeywordString2ParameterValues(QString ss, QString vv);
public:
	bool load_ParaFile(QString openFileNameLabel);
	bool save_ParaFile(QString openFileNameLabel);
};


class StimulatingDialog : public QDialog
{
	Q_OBJECT
public:
	StimulatingDialog(V3DPluginCallback &callback, QWidget *parent, StimulatingParas *para)
	{
		if (!para)
		{
			scanning_rate = 1000;
			nPulse = 6;
			waiting = 1000;
			region_radius = 0;
		}
		else //then just copy from *pl
		{
			scanning_rate = para->SCANNING_RATE;
			nPulse = para->NPULSE;
			waiting = para->WAITING;
			region_radius = para->REGION_RADIUS;
		}

		gridLayout = new QGridLayout();

		// n pulse
		label_npulse = new QLabel(QObject::tr(" The number of pulse: "));

		qsb_npulse = new QSpinBox();
		qsb_npulse->setMaximum(NPLUSE_MAX); qsb_npulse->setMinimum(NPLUSE_MIN); qsb_npulse->setValue(nPulse); qsb_npulse->setSingleStep(NPLUSE_STEP);

		gridLayout->addWidget(label_npulse, 2,0,1,1); gridLayout->addWidget(qsb_npulse, 2,1,1,1);

		// stimulating time
		label_wait = new QLabel(QObject::tr(" Stimulating Time (ms): "));

		qsb_wait = new QSpinBox();
		qsb_wait->setMaximum(STIMULATION_TIME_MAX); qsb_wait->setMinimum(STIMULATION_TIME_MIN); qsb_wait->setValue(waiting); qsb_wait->setSingleStep(STIMULATION_TIME_STEP);

		gridLayout->addWidget(label_wait, 4,0,1,1); gridLayout->addWidget(qsb_wait, 4,1,1,1);

		// scan rate
		label_scanrate = new QLabel(QObject::tr(" Scanning Rate (Hz): "));
		
		qsb_scanrate = new QDoubleSpinBox();
		qsb_scanrate->setMaximum(SCAN_RATE_MAX); qsb_scanrate->setMinimum(SCAN_RATE_MIN); qsb_scanrate->setValue(scanning_rate); qsb_scanrate->setSingleStep(SCAN_RATE_STEP);

		gridLayout->addWidget(label_scanrate, 6,0,1,1); gridLayout->addWidget(qsb_scanrate, 6,1,1,1);

		// ablation region size
		label_regionsize = new QLabel(QObject::tr(" Region Radius: "));
		
		qsb_regionsize = new QSpinBox();
		qsb_regionsize->setMaximum(SCAN_REGION_SIZE_MAX); qsb_scanrate->setMinimum(0); qsb_regionsize->setValue(region_radius); qsb_regionsize->setSingleStep(1);

		gridLayout->addWidget(label_regionsize, 8,0,1,1); gridLayout->addWidget(qsb_regionsize, 8,1,1,1);

		//
		ok     = new QPushButton("OK");
		cancel = new QPushButton("Cancel");
		
		gridLayout->addWidget(cancel, 10,2,Qt::AlignRight); gridLayout->addWidget(ok, 10,3,Qt::AlignRight);

		//gridlayout
		setLayout(gridLayout);
		setWindowTitle(QString("Stimulating"));
		
		//slot interface
		connect(qsb_npulse, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(qsb_wait, SIGNAL(valueChanged(int)), this, SLOT(update()));
		connect(qsb_scanrate, SIGNAL(valueChanged(double)), this, SLOT(update()));
		connect(qsb_regionsize, SIGNAL(valueChanged(int)), this, SLOT(update()));

		connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
	}
	
	~StimulatingDialog(){}

	void copyDataOut(StimulatingParas *para);


public slots:
	void update()
	{
		nPulse = qsb_npulse->text().toInt();
		waiting = qsb_wait->text().toInt();
		scanning_rate = qsb_scanrate->text().toDouble();
		region_radius = qsb_regionsize->text().toInt();
	}
	
public:
	int i1;
	double scanning_rate;
	int nPulse, waiting;
	int region_radius;

	QLabel *label_subject;
	QComboBox* combo_subject;

	QLabel *label_npulse;
	QSpinBox *qsb_npulse;

	QLabel *label_wait;
	QSpinBox *qsb_wait;

	QLabel *label_scanrate;
	QDoubleSpinBox *qsb_scanrate;

	QLabel *label_regionsize;
	QSpinBox *qsb_regionsize;

	QGridLayout *gridLayout;
	
	QPushButton* ok;
	QPushButton* cancel;
};

class MicroscopeFocusControls: public QDialog
{
	Q_OBJECT
public:
	MicroscopeFocusControls()
	{
		//create a dialog
		gridLayout = new QGridLayout();
		label = new QLabel(QObject::tr(" Scanning Control ")); 
		gridLayout->addWidget(label, 0,0);
		ok     = new QPushButton("Stop");
		gridLayout->addWidget(ok, 3,2,Qt::AlignRight); 

		// init
		flag_stop = false;

		//gridlayout
		setLayout(gridLayout);
		setWindowTitle(QString("Scanning Control"));
		
		//slot interface
		connect(ok,     SIGNAL(clicked()), this, SLOT(update()));
	};

	~MicroscopeFocusControls(){}

public slots:
	void update(){reject();}
	void reject(){flag_stop = true;}

	void setText(QString a) {if (label) label->setText(a);}
	
public:
	bool flag_stop;
	QGridLayout *gridLayout;
	QLabel *label;
	QPushButton* ok;
};

// This class and implementation is from the plugins of regiongrow
// define a simple dialog for choose Region Growing parameters
class RegiongrowDialog : public QDialog
{
	Q_OBJECT
	
public:
	RegiongrowDialog(V3DPluginCallback* cb, QWidget *parent);
	~RegiongrowDialog(){}
	
	public slots:
	void update();
	
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

