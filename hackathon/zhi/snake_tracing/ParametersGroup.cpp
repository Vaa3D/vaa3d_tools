/*=========================================================================
Copyright 2009 Rensselaer Polytechnic Institute
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
=========================================================================*/

/*=========================================================================

  Program:   Farsight Biological Image Segmentation and Visualization Toolkit
  Language:  C++
  Date:      $Date:  $
  Version:   $Revision: 0.00 $

=========================================================================*/
#include "ParametersGroup.h"


General_Parameters::General_Parameters(const QString &title,
                  QWidget *parent)
     : QGroupBox(title, parent)
{
   //stack_image = new QGroupBox(tr("Stack Image"));
   step_process = new QGroupBox(tr("Step-by-Step Process"));
   automated_process = new QGroupBox(tr("Automated Process"));
   //montage_process = new QGroupBox(tr("Montage"));
   QBoxLayout *layout = new QVBoxLayout;
   QBoxLayout *layout0 = new QVBoxLayout;
   QBoxLayout *layout1 = new QVBoxLayout;
   QBoxLayout *layout2 = new QVBoxLayout;
   QBoxLayout *layout3 = new QVBoxLayout;
   
   QBoxLayout *para_layout = new QHBoxLayout;
   QBoxLayout *para_layout1 = new QHBoxLayout;
   QBoxLayout *para_layout2 = new QHBoxLayout;

   remove_slice_label = new QLabel("Remove Slices");
   remove_slice_box = new QSpinBox;

   remove_slice_box->setValue(0);
   remove_slice_box->setSingleStep(1);
   remove_slice_box->setMaximum(20);

   sf_label = new QLabel("Scale Factor");
   sf_box = new QSpinBox;
   sf_box->setValue(1);
   sf_box->setSingleStep(1);
   sf_box->setMaximum(10);

   sf_box->setMinimum(1);
   sh_label = new QLabel("Shrink Factor");
   sh_box = new QSpinBox;
   sh_box->setValue(1);
   sh_box->setSingleStep(1);
   sh_box->setMaximum(10);
   sh_box->setMinimum(1);

   //no_processing_after = new QRadioButton("No Processing", this);
   //no_processing_after->setChecked(true);
   //continue_auto_processing = new QRadioButton("Auto Process", this);
   //continue_batchI_processing = new QRadioButton("Batch Process I", this);
   //continue_batchII_processing = new QRadioButton("Batch Process II", this);

   //connect(continue_auto_processing,SIGNAL(toggled(bool)), this, SLOT(auto_Slot()));
   //connect(continue_batchI_processing,SIGNAL(toggled(bool)), this, SLOT(batchI_Slot()));
   //connect(continue_batchII_processing,SIGNAL(toggled(bool)), this, SLOT(batchII_Slot()));

   //sixteen_bit = new QCheckBox("16-bit Image", this);

   para_layout->addWidget(remove_slice_label);
   para_layout->addWidget(remove_slice_box);

   para_layout1->addWidget(sf_label);
   para_layout1->addWidget(sf_box);

   para_layout2->addWidget(sh_label);
   para_layout2->addWidget(sh_box);

   stackImageButton = new QPushButton("Stack Image");
   loadButton = new QPushButton("Load Image");
   loadDisplayButton = new QPushButton("Load Display");
   reloadButton = new QPushButton("Reload Image");
   autoButton = new QPushButton("Automated Process");
   autoButton->setToolTip("Automated processing without clicking the next button");
   batchButtonI = new QPushButton("Batch Process I");
   batchButtonI->setToolTip(tr("Batch process all images within the specified input folder and store all swcs in specified output folder "));
   batchButtonII = new QPushButton("Batch Process II");
   batchButtonII->setToolTip(tr("Batch process tiles using registration information"));
   montageButton = new QPushButton("Montage");
   montageButton->setToolTip("Automated montage with the name of folder storing the tiles and a txt file containing the coordinates of tiles as input");

   layout->addLayout(para_layout);
   layout->addLayout(para_layout1);
   layout->addLayout(para_layout2);
   //layout0->addWidget(sixteen_bit);
   //layout0->addWidget(stackImageButton);
   //layout0->addWidget(no_processing_after);
   //layout0->addWidget(continue_auto_processing);
   //layout0->addWidget(continue_batchI_processing);
   //layout0->addWidget(continue_batchII_processing);
   layout1->addWidget(loadButton);
   layout1->addWidget(loadDisplayButton);
   layout1->addWidget(reloadButton);
   layout2->addWidget(autoButton);
   layout2->addWidget(batchButtonI);
   layout2->addWidget(batchButtonII);
   //layout3->addWidget(montageButton);

   //stack_image->setLayout(layout0);
   step_process->setLayout(layout1);
   automated_process->setLayout(layout2);
   //montage_process->setLayout(layout3);

   //layout->addWidget(stack_image);
   layout->addWidget(step_process);
   layout->addWidget(automated_process);
   //layout->addWidget(montage_process);
   setLayout(layout);
}

void General_Parameters::setEliteSetting()
{
   remove_slice_label->setVisible(false);
   remove_slice_box->setVisible(false);

   sf_label->setVisible(false);
   sf_box->setVisible(false);
}

void General_Parameters::setFullSetting()
{
   remove_slice_label->setVisible(true);
   remove_slice_box->setVisible(true);

   sf_label->setVisible(true);
   sf_box->setVisible(true);
}

void General_Parameters::disableSetting()
{
   remove_slice_box->setEnabled(false);
   sh_box->setEnabled(false);
   sf_box->setEnabled(false);
}
void General_Parameters::enableSetting()
{
   remove_slice_box->setEnabled(true);
   sh_box->setEnabled(true);
   sf_box->setEnabled(true);
}
void General_Parameters::auto_Slot()
{
  /* if( continue_auto_processing->isChecked() )
     emit auto_Selected(true);
   else
	 emit auto_Selected(false); */
}
void General_Parameters::batchI_Slot()
{
  /* if( continue_batchI_processing->isChecked() )
     emit batchI_Selected(true);
   else
	 emit batchI_Selected(false); */
}
void General_Parameters::batchII_Slot()
{
  /* if( continue_batchII_processing->isChecked() )
     emit batchII_Selected(true);
   else
	 emit batchII_Selected(false); */
}
int General_Parameters::getRemoveSlice()
{
	 return remove_slice_box->value();
}
void General_Parameters::setRemoveSlice(int in)
{
    remove_slice_box->setValue(in);
}
int General_Parameters::getSh()
{
	 return sh_box->value();
}
void General_Parameters::setSh(int in)
{
    sh_box->setValue(in);
}
int General_Parameters::getSf()
{
	 return sf_box->value();
}
void General_Parameters::setSf(int in)
{
    sf_box->setValue(in);
}
bool General_Parameters::getContinueAutoProcessing()
{
   return continue_auto_processing->isChecked();
}
bool General_Parameters::getBatchIProcessing()
{
   return continue_batchI_processing->isChecked();
}
bool General_Parameters::getBatchIIProcessing()
{
   return continue_batchII_processing->isChecked();
}
bool General_Parameters::getSixteenBit()
{
   return sixteen_bit->isChecked();
}


General_Parameters1::General_Parameters1(const QString &title,
                  QWidget *parent)
     : QGroupBox(title, parent)
{

   QBoxLayout *layout = new QVBoxLayout;
   
   QBoxLayout *para_layout1 = new QHBoxLayout;
   QBoxLayout *para_layout2 = new QHBoxLayout;
   QBoxLayout *para_layout3 = new QHBoxLayout;

   min_sigma_label = new QLabel("min sigma");
   min_sigma_box = new QDoubleSpinBox;
   max_sigma_label = new QLabel("max sigma");
   max_sigma_box = new QDoubleSpinBox;
   sigma_step_label = new QLabel("sigma steps");
   sigma_step_box = new QSpinBox;

   min_sigma_box->setValue(1);
   min_sigma_box->setSingleStep(0.1);
   min_sigma_box->setMaximum(100);
   min_sigma_box->setMinimum(0.1);

   max_sigma_box->setValue(5);
   max_sigma_box->setSingleStep(0.5);
   max_sigma_box->setMaximum(100);
   max_sigma_box->setMinimum(1);

   sigma_step_box->setValue(4);
   sigma_step_box->setSingleStep(1);
   sigma_step_box->setMaximum(100);
   sigma_step_box->setMinimum(1);


   para_layout1->addWidget(min_sigma_label);
   para_layout1->addWidget(min_sigma_box);
   para_layout2->addWidget(max_sigma_label);
   para_layout2->addWidget(max_sigma_box);
   para_layout3->addWidget(sigma_step_label);
   para_layout3->addWidget(sigma_step_box);

   //computeVesselButton = new QPushButton("Compute Vesselness");
   //enhanceVesselButton = new QPushButton("Enhance Vesselness");

   layout->addLayout(para_layout1);
   layout->addLayout(para_layout2);
   layout->addLayout(para_layout3);
   //layout->addWidget(computeVesselButton);
   //layout->addWidget(enhanceVesselButton);
   setLayout(layout);
}

void General_Parameters1::disableSetting()
{
   min_sigma_box->setEnabled(false);
   max_sigma_box->setEnabled(false);
   sigma_step_box->setEnabled(false);
}
void General_Parameters1::enableSetting()
{
   min_sigma_box->setEnabled(true);
   max_sigma_box->setEnabled(true);
   sigma_step_box->setEnabled(true);
}
double General_Parameters1::getMinSigma()
{
	 return min_sigma_box->value();
}
void General_Parameters1::setMinSigma(double in)
{
    min_sigma_box->setValue(in);
}
double General_Parameters1::getMaxSigma()
{
	 return max_sigma_box->value();
}
void General_Parameters1::setMaxSigma(double in)
{
    max_sigma_box->setValue(in);
}
int General_Parameters1::getSigmaStep()
{
	 return sigma_step_box->value();
}
void General_Parameters1::setSigmaStep(int in)
{
    sigma_step_box->setValue(in);
}

General_Parameters11::General_Parameters11(const QString &title,
                  QWidget *parent)
     : QGroupBox(title, parent)
{
   QBoxLayout *layout = new QVBoxLayout;
   detectSeeds = new QPushButton("Seed Detection");
   layout->addWidget(detectSeeds);
   setLayout(layout);
}
void General_Parameters11::disableSetting()
{
   detectSeeds->setEnabled(false);
}
void General_Parameters11::enableSetting()
{
   detectSeeds->setEnabled(true);
}


General_Parameters12::General_Parameters12(const QString &title,
                  QWidget *parent)
     : QGroupBox(title, parent)
{

   QBoxLayout *layout = new QVBoxLayout;
   QBoxLayout *layout1 = new QVBoxLayout;
   QBoxLayout *layout2 = new QVBoxLayout;
  
   outside_preprocess = new QGroupBox(tr("External Preprocessor"));
   inside_preprocess = new QGroupBox(tr("Internal Preprocessor"));

   QBoxLayout *para_layout1 = new QHBoxLayout;
   QBoxLayout *para_layout2 = new QHBoxLayout;
   QBoxLayout *para_layout3 = new QHBoxLayout;
   QBoxLayout *para_layout4 = new QHBoxLayout;
   QBoxLayout *para_layout5 = new QHBoxLayout;
   QBoxLayout *para_layout6 = new QHBoxLayout;
   QBoxLayout *para_layout7 = new QHBoxLayout;
   QBoxLayout *para_layout8 = new QHBoxLayout;

   noise_level_label = new QLabel("noise level");
   noise_level_box = new QDoubleSpinBox;
   //noise_level_label->setVisible(false);
   //noise_level_box->setVisible(false);

   num_iteration_label = new QLabel("iteration num");
   num_iteration_box = new QSpinBox;

   threshold_label = new QLabel("threshold");
   threshold_box = new QDoubleSpinBox;

   seed_adjustment_label = new QLabel("seed adjustment");
   seed_adjustment_box = new QSpinBox;

   seed_radius_label = new QLabel("seed radius");
   seed_radius_box = new QSpinBox;
   seed_radius_label->setVisible(false);
   seed_radius_box->setVisible(false);

   smoothing_scale_label = new QLabel("smoothing scale");
   smoothing_scale_box = new QSpinBox;
   smoothing_scale_label->setVisible(false);
   smoothing_scale_box->setVisible(false);

   noise_level_box->setMinimum(0);
   noise_level_box->setMaximum(0.2);
   noise_level_box->setValue(0.1);
   noise_level_box->setSingleStep(0.05);
  
   num_iteration_box->setMaximum(100);
   num_iteration_box->setMinimum(0);
   num_iteration_box->setValue(5);
   num_iteration_box->setSingleStep(1);

   use_multi_scale = new QCheckBox("multi-scale preprocessing", this);
   use_multi_scale->setVisible(true);

   use_laplacian_of_gaussian = new QCheckBox("laplacian of gaussian", this);
   use_laplacian_of_gaussian->setVisible(true);

   threshold_box->setMaximum(255);
   threshold_box->setMinimum(0);
   threshold_box->setDecimals(4);
   threshold_box->setValue(1);
   threshold_box->setSingleStep(0.1);

   seed_adjustment_box->setMaximum(500);
   seed_adjustment_box->setMinimum(0);
   seed_adjustment_box->setValue(10);
   seed_adjustment_box->setSingleStep(5);

   seed_radius_box->setMaximum(50);
   seed_radius_box->setMinimum(0);
   seed_radius_box->setValue(0);
   seed_radius_box->setSingleStep(1);

   smoothing_scale_box->setMaximum(10);
   smoothing_scale_box->setMinimum(0);
   smoothing_scale_box->setValue(0);
   smoothing_scale_box->setSingleStep(1);

   para_layout1->addWidget(noise_level_label);
   para_layout1->addWidget(noise_level_box);
   para_layout2->addWidget(num_iteration_label);
   para_layout2->addWidget(num_iteration_box);
   para_layout3->addWidget(threshold_label);
   para_layout3->addWidget(threshold_box);
   para_layout4->addWidget(seed_adjustment_label);
   para_layout4->addWidget(seed_adjustment_box);
   para_layout5->addWidget(seed_radius_label);
   para_layout5->addWidget(seed_radius_box);
   
   para_layout7->addWidget(smoothing_scale_label);
   para_layout7->addWidget(smoothing_scale_box);

	para_layout8->addWidget(use_laplacian_of_gaussian);

   general_para1 = new General_Parameters1(tr("Multi-Scale Enhancement"));
   general_para1->setVisible(false);
   general_para11 = new General_Parameters11(tr("Reprocessing"));
   general_para11->setVisible(false);

   preprocessButton1 = new QPushButton("Curvelet + Scalar Voting");
   curvelet_before_batch = new QRadioButton("curvelet before batch processing", this);
   curvelet_during_batch = new QRadioButton("curvelet during batch processing", this);  
   curvelet_before_batch->setChecked(true);
   setCurveletBatchEnabled(false);

   preprocessButton = new QPushButton("Preprocessing");
   reprocessingButton = new QPushButton("Reprocessing");
   outputSeeds = new QPushButton("Output Seeds");
   outputSeeds->setVisible(false);

   connect(reprocessingButton,SIGNAL(clicked()), this, SLOT(reprocessing_slot()));
   reprocessing = false;

   connect(general_para11->detectSeeds,SIGNAL(clicked()), this, SLOT(seed_slot()));

   seed_detection_methods = new QComboBox;
   //seed_detection_methods->addItem(tr("Skeleton Seeds"));
   seed_detection_methods->addItem(tr("Vesselness and Ridgeness"));
   seed_detection_methods->setVisible(false);

   gpu_acceleration = new QComboBox;
   gpu_acceleration->addItem(tr("GPU Acceleration"));
   gpu_acceleration->addItem(tr("CPU Implementation"));
   gpu_acceleration->addItem(tr("Compare GPU&CPU"));
   gpu_acceleration->setCurrentIndex(1); 
   gpu_acceleration->setVisible(false);

   layout1->addWidget(gpu_acceleration);
   layout1->addWidget(seed_detection_methods);

   layout1->addLayout(para_layout1);
   layout1->addLayout(para_layout2);
   layout1->addLayout(para_layout3);
   layout1->addLayout(para_layout4);
   layout1->addLayout(para_layout5);
   layout1->addLayout(para_layout7);
   layout1->addLayout(para_layout8);
   layout1->addWidget(use_multi_scale);
   layout1->addWidget(general_para1);
   layout1->addWidget(preprocessButton);
   layout1->addWidget(reprocessingButton);
   layout1->addWidget(general_para11);
   layout1->addWidget(outputSeeds);

   inside_preprocess->setLayout(layout1);

   layout2->addWidget(preprocessButton1);
   layout2->addWidget(curvelet_before_batch);
   layout2->addWidget(curvelet_during_batch);
   outside_preprocess->setLayout(layout2);

   //layout->addWidget(outside_preprocess);
   layout->addWidget(inside_preprocess);

   connect(use_multi_scale,SIGNAL(stateChanged(int)), this, SLOT(use_multi_scale_slot()));

   setLayout(layout);
}

void General_Parameters12::setEliteSetting()
{
   noise_level_label->setVisible(false);
   noise_level_box->setVisible(false);

   seed_adjustment_label->setVisible(false);
   seed_adjustment_box->setVisible(false);

   seed_radius_label->setVisible(false);

   seed_radius_box->setVisible(false);

   smoothing_scale_label->setVisible(false);
   smoothing_scale_box->setVisible(false);

   seed_detection_methods->setVisible(false);

   gpu_acceleration->setVisible(false);
}

void General_Parameters12::setFullSetting()
{
   noise_level_label->setVisible(false);
   noise_level_box->setVisible(false);

   seed_adjustment_label->setVisible(true);
   seed_adjustment_box->setVisible(true);

   seed_radius_label->setVisible(true);
   seed_radius_box->setVisible(true);

   smoothing_scale_label->setVisible(true);
   smoothing_scale_box->setVisible(true);

   seed_detection_methods->setVisible(false);

   gpu_acceleration->setVisible(false);

}

void General_Parameters12::seed_slot()
{
   emit reprocess(3);
}

void General_Parameters12::use_multi_scale_slot()
{
   if( use_multi_scale->isChecked() )
	   general_para1->setVisible(true);
   else
	   general_para1->setVisible(false);
}


void General_Parameters12::reprocessing_slot()
{
   if( !reprocessing )
	   reprocessing = true;
   else
	   reprocessing = false;

   if( reprocessing )
   {
	   general_para11->setVisible(true);
   }
   else
   {
	   general_para11->setVisible(false);
   }
}

int General_Parameters12::getCurveletBatch()
{
	if( curvelet_before_batch->isChecked() )
		return 0;
	else if( curvelet_during_batch->isChecked() )
		return 1;

	return 0;
}

void General_Parameters12::disableSetting()
{
   curvelet_before_batch->setEnabled(false);
   curvelet_during_batch->setEnabled(false);
   noise_level_box->setEnabled(false);
   num_iteration_box->setEnabled(false);
   threshold_box->setEnabled(false);
   seed_adjustment_box->setEnabled(false);
   seed_radius_box->setEnabled(false);
   smoothing_scale_box->setEnabled(false);
   seed_detection_methods->setEnabled(false);
   gpu_acceleration->setEnabled(false);
   reprocessingButton->setEnabled(false);
   preprocessButton->setEnabled(false);
   outputSeeds->setEnabled(false);
   use_multi_scale->setEnabled(false);
}
void General_Parameters12::enableSetting()
{
   curvelet_before_batch->setEnabled(true);
   curvelet_during_batch->setEnabled(true);
   noise_level_box->setEnabled(true);
   num_iteration_box->setEnabled(true);
   threshold_box->setEnabled(true);
   seed_adjustment_box->setEnabled(true);
   seed_radius_box->setEnabled(true);
   smoothing_scale_box->setEnabled(true);
   seed_detection_methods->setEnabled(true);
   gpu_acceleration->setEnabled(true);
   reprocessingButton->setEnabled(true);
   preprocessButton->setEnabled(true);
   outputSeeds->setEnabled(true);
   use_multi_scale->setEnabled(true);
}
void General_Parameters12::setCurveletBatchEnabled(bool in)
{ 
	if( in )
	{
	 curvelet_before_batch->setVisible(true);
     curvelet_during_batch->setVisible(true);
	}
	else
	{
     curvelet_before_batch->setVisible(false);
     curvelet_during_batch->setVisible(false);
	}

}

bool General_Parameters12::getLaplacianOfGaussian()
{
	return use_laplacian_of_gaussian->isChecked();
}

double General_Parameters12::getNoiseLevel()
{
	 return noise_level_box->value();
}
void General_Parameters12::setNoiseLevel(double in)
{
    noise_level_box->setValue(in);
}
int General_Parameters12::getNumIteration()
{
	 return num_iteration_box->value();
}
void General_Parameters12::setNumIteration(int in)
{
    num_iteration_box->setValue(in);
}
double General_Parameters12::getThreshold()
{
	 return threshold_box->value();
}
void General_Parameters12::setThreshold(double in)
{
    threshold_box->setValue(in);
}
int General_Parameters12::getSeedAdjustment()
{
	 return seed_adjustment_box->value();
}
void General_Parameters12::setSeedAdjustment(int in)
{
    seed_adjustment_box->setValue(in);
}
int General_Parameters12::getSeedRadius()
{
	 return seed_radius_box->value();
}
void General_Parameters12::setSeedRadius(int in)
{
    seed_radius_box->setValue(in);
}
int General_Parameters12::getSmoothingScale()
{
	 return smoothing_scale_box->value();
}
void General_Parameters12::setSmoothingScale(int in)
{
    smoothing_scale_box->setValue(in);
}
int General_Parameters12::getCurrentGPUIndex()
{
	return gpu_acceleration->currentIndex();
}
int General_Parameters12::getCurrentSeedIndex()
{
	return seed_detection_methods->currentIndex();
}
void General_Parameters12::setCurrentSeedIndex(int in)
{
	seed_detection_methods->setCurrentIndex(in);
}
bool General_Parameters12::getMultiScale()
{
    return use_multi_scale->isChecked();
}
void General_Parameters12::setMultiScale(int in)
{	
	if(in == 1)
	 use_multi_scale->setCheckState(Qt::Checked);
    else
	 use_multi_scale->setCheckState(Qt::Unchecked);
}

General_Parameters21::General_Parameters21(const QString &title,
                  QWidget *parent)
     : QGroupBox(title, parent)
{

   QBoxLayout *layout = new QVBoxLayout;
   QBoxLayout *para_layout = new QHBoxLayout;
   
   max_angle_label = new QLabel("max angle");
   max_angle_box = new QSpinBox;

   max_angle_box->setValue(100);
   max_angle_box->setSingleStep(10);
   max_angle_box->setMaximum(180);
   max_angle_box->setMinimum(60);

   para_layout->addWidget(max_angle_label);
   para_layout->addWidget(max_angle_box);

   layout->addLayout(para_layout);

   setLayout(layout);
}


void General_Parameters21::disableSetting()
{
   max_angle_box->setEnabled(false);
}
void General_Parameters21::enableSetting()
{
   max_angle_box->setEnabled(true);
}
int General_Parameters21::getMaxAngle()
{
	 return max_angle_box->value();
}
void General_Parameters21::setMaxAngle(double in)
{
    max_angle_box->setValue(in);
}


General_Parameters22::General_Parameters22(const QString &title,
                  QWidget *parent)
     : QGroupBox(title, parent)
{
   QBoxLayout *layout = new QVBoxLayout;
   QBoxLayout *para_layout1 = new QHBoxLayout;
   QBoxLayout *para_layout2 = new QHBoxLayout;

   dist_label = new QLabel("distance");
   dist_box = new QSpinBox;
   angle_label = new QLabel("angle");
   angle_box = new QSpinBox;

   dist_box->setValue(3);
   dist_box->setSingleStep(1);
   dist_box->setMaximum(10);
   dist_box->setMinimum(1);

   angle_box->setValue(30);
   angle_box->setSingleStep(5);
   angle_box->setMaximum(45);
   angle_box->setMinimum(5);

   para_layout1->addWidget(dist_label);
   para_layout1->addWidget(dist_box);
   para_layout2->addWidget(angle_label);
   para_layout2->addWidget(angle_box);

   layout->addLayout(para_layout1);
   layout->addLayout(para_layout2);

   setLayout(layout);
}

void General_Parameters22::disableSetting()
{
   dist_box->setEnabled(false);
   angle_box->setEnabled(false);
}
void General_Parameters22::enableSetting()
{
   dist_box->setEnabled(true);
   angle_box->setEnabled(true);
}
int General_Parameters22::getDist()
{
	 return dist_box->value();
}
void General_Parameters22::setDist(int in)
{
    dist_box->setValue(in);
}

int General_Parameters22::getAngle()
{
	 return angle_box->value();
}
void General_Parameters22::setAngle(int in)
{
    angle_box->setValue(in);
}


General_Parameters23::General_Parameters23(const QString &title,
                  QWidget *parent)
     : QGroupBox(title, parent)
{
   QBoxLayout *layout = new QVBoxLayout;
   QBoxLayout *para_layout1 = new QHBoxLayout;

   dist_label = new QLabel("distance");
   dist_box = new QSpinBox;

   dist_box->setValue(3);
   dist_box->setSingleStep(1);
   dist_box->setMaximum(100);
   dist_box->setMinimum(1);

   para_layout1->addWidget(dist_label);
   para_layout1->addWidget(dist_box);

   layout->addLayout(para_layout1);

   setLayout(layout);
}


void General_Parameters23::disableSetting()
{
   dist_box->setEnabled(false);
}
void General_Parameters23::enableSetting()
{
   dist_box->setEnabled(true);
}
int General_Parameters23::getDist()
{
	 return dist_box->value();
}
void General_Parameters23::setDist(int in)
{
    dist_box->setValue(in);
}


General_Parameters2::General_Parameters2(const QString &title,
                  QWidget *parent)
     : QGroupBox(title, parent)
{

   QBoxLayout *layout = new QVBoxLayout;
   
   QBoxLayout *para_layout1 = new QHBoxLayout;
   QBoxLayout *para_layout2 = new QHBoxLayout;
   QBoxLayout *para_layout3 = new QHBoxLayout;
   QBoxLayout *para_layout4 = new QHBoxLayout;
   QBoxLayout *para_layout5 = new QHBoxLayout;
   QBoxLayout *para_layout6 = new QHBoxLayout;
   QBoxLayout *para_layout7 = new QHBoxLayout;
   QBoxLayout *para_layout8 = new QHBoxLayout;
   QBoxLayout *para_layout9 = new QHBoxLayout;
   QBoxLayout *para_layout10 = new QHBoxLayout;
   QBoxLayout *para_layout11 = new QHBoxLayout;

   tracing_models = new QComboBox;
   tracing_models->addItem(tr("3-D Open-Curve Snake"));
   tracing_models->addItem(tr("4-D Open Snake(Region)"));
   tracing_models->addItem(tr("4-D Open Snake(Gradient+Region)"));
   tracing_models->addItem(tr("4-D Open Snake(Elliptic)"));

   seed_expansion_methods = new QComboBox;
   //seed_expansion_methods->addItem(tr("Skeleton Expansion"));
   seed_expansion_methods->addItem(tr("Hessian Expansion"));
   seed_expansion_methods->setVisible(false);

   stretching_force = new QComboBox;
   stretching_force->addItem(tr("EigenVector Stretch Force"));
   stretching_force->addItem(tr("Momentum Stretch Force"));
   stretching_force->addItem(tr("Combined Stretch Force"));
   stretching_force->setCurrentIndex(1);
   stretching_force->setVisible(false);

   image_coding = new QComboBox;
   image_coding->addItem(tr("Centerline Coding"));
   image_coding->addItem(tr("Vessel Tube Coding"));
   
   //stretching_force->addItem(tr("Curvelet Stretch Force"));

   stretch_ratio_label = new QLabel("stretch ratio");
   stretch_ratio_box = new QSpinBox;

   collision_dist_label = new QLabel("collision dist");
   collision_dist_box = new QSpinBox;

   pt_distance_label = new QLabel("unit length");
   pt_distance_box = new QSpinBox;

   gamma_label = new QLabel("gamma");
   gamma_box = new QDoubleSpinBox;


   minimum_length_label = new QLabel("min length");
   minimum_length_box = new QSpinBox;

   remove_seed_range_label = new QLabel("remove seed");
   remove_seed_range_box = new QSpinBox;


   k_sigma_label = new QLabel("sigma ratio");
   k_sigma_box = new QDoubleSpinBox;


   border_label = new QLabel("image border");
   border_box = new QSpinBox;

   deformation_iter_label = new QLabel("deform iteration");
   deformation_iter_box = new QSpinBox;
   deformation_iter_label->setVisible(false);
   deformation_iter_box->setVisible(false);

   repeat_ratio_label = new QLabel("repeat ratio");
   repeat_ratio_box = new QDoubleSpinBox;
   repeat_dist_label = new QLabel("repeat distance");
   repeat_dist_box = new QSpinBox;
   repeat_ratio_label->setVisible(false);
   repeat_ratio_box->setVisible(false);
   repeat_dist_label->setVisible(false);
   repeat_dist_box->setVisible(false);

   freeze_body = new QCheckBox("freeze body", this);
   freeze_body->setVisible(false);

   use_automatic_merging = new QCheckBox("automatic merging", this);
   multi_threads = new QCheckBox("multi-threads", this);
   multi_threads->setVisible(false);
   //use_jumping_gaps = new QCheckBox("jumping gaps", this);
   //use_jumping_crossover = new QCheckBox("crossover", this);
  
   general_para21 = new General_Parameters21(tr("Automatic Merging"));
   general_para21->setVisible(false);
   connect(use_automatic_merging,SIGNAL(stateChanged(int)), this, SLOT(use_automatic_merging_slot()));

   //general_para22 = new General_Parameters22(tr("Jumping Gaps"));
   //general_para22->setVisible(false);
   //connect(use_jumping_gaps,SIGNAL(stateChanged(int)), this, SLOT(use_jumping_gaps_slot()));

   //general_para23 = new General_Parameters23(tr("Crossover"));
   //general_para23->setVisible(false);
   //connect(use_jumping_crossover,SIGNAL(stateChanged(int)), this, SLOT(use_jumping_crossover_slot()));

   stretch_ratio_box->setMinimum(1);
   stretch_ratio_box->setMaximum(10);
   stretch_ratio_box->setValue(3);
   stretch_ratio_box->setSingleStep(1);
  
   pt_distance_box->setMaximum(20);
   pt_distance_box->setMinimum(1);
   pt_distance_box->setValue(3);
   pt_distance_box->setSingleStep(1);

   gamma_box->setMaximum(5);
   gamma_box->setMinimum(0.5);
   gamma_box->setValue(2);
   gamma_box->setSingleStep(0.5);

   minimum_length_box->setMaximum(500);
   minimum_length_box->setMinimum(1);
   minimum_length_box->setValue(5);
   minimum_length_box->setSingleStep(1);

   collision_dist_box->setMaximum(10);
   collision_dist_box->setMinimum(0);
   collision_dist_box->setValue(1);
   collision_dist_box->setSingleStep(1);

   remove_seed_range_box->setMaximum(30);
   remove_seed_range_box->setMinimum(1);
   remove_seed_range_box->setValue(5);
   remove_seed_range_box->setSingleStep(1);

   k_sigma_box->setMaximum(10);
   k_sigma_box->setMinimum(0.1);
   k_sigma_box->setValue(1);
   k_sigma_box->setSingleStep(0.1);

   deformation_iter_box->setMaximum(20);
   deformation_iter_box->setMinimum(0);
   deformation_iter_box->setValue(0);
   deformation_iter_box->setSingleStep(1);

   repeat_ratio_box->setMaximum(1);
   repeat_ratio_box->setMinimum(0);
   repeat_ratio_box->setValue(0.5);
   repeat_ratio_box->setSingleStep(0.1);

   repeat_dist_box->setMaximum(20);
   repeat_dist_box->setMinimum(0);
   repeat_dist_box->setValue(1);
   repeat_dist_box->setSingleStep(1);

   para_layout1->addWidget(stretch_ratio_label);
   para_layout1->addWidget(stretch_ratio_box);
   para_layout2->addWidget(gamma_label);
   para_layout2->addWidget(gamma_box);

   para_layout3->addWidget(pt_distance_label);
   para_layout3->addWidget(pt_distance_box);
   para_layout4->addWidget(minimum_length_label);
   para_layout4->addWidget(minimum_length_box);
   para_layout5->addWidget(collision_dist_label);
   para_layout5->addWidget(collision_dist_box);
   para_layout6->addWidget(remove_seed_range_label);
   para_layout6->addWidget(remove_seed_range_box);
   para_layout7->addWidget(deformation_iter_label);
   para_layout7->addWidget(deformation_iter_box);
   para_layout8->addWidget(repeat_ratio_label);
   para_layout8->addWidget(repeat_ratio_box);
   para_layout9->addWidget(repeat_dist_label);
   para_layout9->addWidget(repeat_dist_box);

   para_layout10->addWidget(k_sigma_label);
   para_layout10->addWidget(k_sigma_box);

   para_layout11->addWidget(border_label);
   para_layout11->addWidget(border_box);

   tracingButton = new QPushButton("Tracing");

   layout->addWidget(tracing_models);
   layout->addWidget(seed_expansion_methods);
   layout->addWidget(stretching_force);
   layout->addWidget(image_coding);
   layout->addLayout(para_layout1);
   layout->addLayout(para_layout2);
   layout->addLayout(para_layout3);
   layout->addLayout(para_layout4);
   layout->addLayout(para_layout5);
   layout->addLayout(para_layout6);
   layout->addLayout(para_layout10);
   layout->addLayout(para_layout11);
   layout->addLayout(para_layout7);
   layout->addLayout(para_layout8);
   layout->addLayout(para_layout9);

   layout->addWidget(freeze_body);
   layout->addWidget(use_automatic_merging);
   layout->addWidget(multi_threads);
   layout->addWidget(general_para21);
   //layout->addWidget(use_jumping_gaps);
   //layout->addWidget(general_para22);
   //layout->addWidget(use_jumping_crossover);
   //layout->addWidget(general_para23);
   layout->addWidget(tracingButton);

   enabled = true;

   setLayout(layout);
}
void General_Parameters2::setEliteSetting()
{
   stretching_force->setVisible(false);
   image_coding->setVisible(false);

   stretch_ratio_label->setVisible(false);
   stretch_ratio_box->setVisible(false);

   collision_dist_label->setVisible(false);
   collision_dist_box->setVisible(false);

   gamma_label->setVisible(false);
   gamma_box->setVisible(false);

   remove_seed_range_label->setVisible(false);
   remove_seed_range_box->setVisible(false);

   k_sigma_label->setVisible(false);
   k_sigma_box->setVisible(false);

   border_label->setVisible(false);
   border_box->setVisible(false);

   freeze_body->setVisible(false);
}

void General_Parameters2::setFullSetting()
{
   stretching_force->setVisible(false);

   image_coding->setVisible(true);

   stretch_ratio_label->setVisible(true);
   stretch_ratio_box->setVisible(true);

   collision_dist_label->setVisible(true);
   collision_dist_box->setVisible(true);

   gamma_label->setVisible(false);
   gamma_box->setVisible(false);

   remove_seed_range_label->setVisible(true);
   remove_seed_range_box->setVisible(true);

   k_sigma_label->setVisible(true);
   k_sigma_box->setVisible(true);

   border_label->setVisible(true);
   border_box->setVisible(true);

   freeze_body->setVisible(true);
}

bool General_Parameters2::getFreezeBody()
{
   return freeze_body->isChecked();
}
void General_Parameters2::setFreezeBody(int in)
{	
	if(in == 1)
	 freeze_body->setCheckState(Qt::Checked);
    else
	 freeze_body->setCheckState(Qt::Unchecked);
}

bool General_Parameters2::getAutomaticMerging()
{
   return use_automatic_merging->isChecked();
}
void General_Parameters2::setAutomaticMerging(int in)
{	
	if(in == 1)
	 use_automatic_merging->setCheckState(Qt::Checked);
    else
	 use_automatic_merging->setCheckState(Qt::Unchecked);
}
bool General_Parameters2::getMultiThreads()
{
   return multi_threads->isChecked();
}
void General_Parameters2::setMultiThreads(int in)
{	
	if(in == 1)
	 multi_threads->setCheckState(Qt::Checked);
    else
	 multi_threads->setCheckState(Qt::Unchecked);
}
/*
bool General_Parameters2::getJumpingGaps()
{
   return use_jumping_gaps->isChecked();
}
void General_Parameters2::setJumpingGaps(int in)
{	
	if(in == 1)
	 use_jumping_gaps->setCheckState(Qt::Checked);
    else
	 use_jumping_gaps->setCheckState(Qt::Unchecked);
}
bool General_Parameters2::getJumpingCrossover()
{
   return use_jumping_crossover->isChecked();
}
void General_Parameters2::setJumpingCrossover(int in)
{	
	if(in == 1)
	 use_jumping_crossover->setCheckState(Qt::Checked);
    else
	 use_jumping_crossover->setCheckState(Qt::Unchecked);
}
*/
int General_Parameters2::getCurrentTracing()
{
	return tracing_models->currentIndex();
}
void General_Parameters2::setCurrentTracing(int in)
{
     tracing_models->setCurrentIndex(in);
}

int General_Parameters2::getCurrentIndex()
{
   return seed_expansion_methods->currentIndex();
}
void General_Parameters2::setCurrentIndex(int in)
{
	seed_expansion_methods->setCurrentIndex(in);
}
int General_Parameters2::getCurrentForce()
{
   return stretching_force->currentIndex();
}
void General_Parameters2::setCurrentForce(int in)
{
	stretching_force->setCurrentIndex(in);
}
int General_Parameters2::getCurrentCoding()
{
   return image_coding->currentIndex();
}
void General_Parameters2::setCurrentCoding(int in)
{
	image_coding->setCurrentIndex(in);
}
void General_Parameters2::use_automatic_merging_slot()
{
   if( use_automatic_merging->isChecked() )
	   general_para21->setVisible(true);
   else
	   general_para21->setVisible(false);
}
/*
void General_Parameters2::use_jumping_gaps_slot()
{
   if( use_jumping_gaps->isChecked() )
	   general_para22->setVisible(true);
   else
	   general_para22->setVisible(false);
}

void General_Parameters2::use_jumping_crossover_slot()
{
   if( use_jumping_crossover->isChecked() )
	   general_para23->setVisible(true);
   else
	   general_para23->setVisible(false);
}*/


void General_Parameters2::disableSetting()
{
   enabled = false;
   stretch_ratio_box->setEnabled(false);
   pt_distance_box->setEnabled(false);
   gamma_box->setEnabled(false);
   minimum_length_box->setEnabled(false);
   collision_dist_box->setEnabled(false);
   
   remove_seed_range_box->setEnabled(false);
   k_sigma_box->setEnabled(false);
   border_box->setEnabled(false);
   deformation_iter_box->setEnabled(false);
   repeat_ratio_box->setEnabled(false);
   repeat_dist_box->setEnabled(false);
   freeze_body->setEnabled(false);
   use_automatic_merging->setEnabled(false);
   //use_jumping_gaps->setEnabled(false);
   //use_jumping_crossover->setEnabled(false);
   general_para21->disableSetting();
   //general_para22->disableSetting();
   //general_para23->disableSetting();

   seed_expansion_methods->setEnabled(false);
   tracing_models->setEnabled(false);
   stretching_force->setEnabled(false);
   image_coding->setEnabled(false);
}
void General_Parameters2::enableSetting()
{
   enabled = true;
   stretch_ratio_box->setEnabled(true);
   pt_distance_box->setEnabled(true);
   gamma_box->setEnabled(true);
   minimum_length_box->setEnabled(true);
   collision_dist_box->setEnabled(true);

   remove_seed_range_box->setEnabled(true);
   k_sigma_box->setEnabled(true);
   border_box->setEnabled(true);
   deformation_iter_box->setEnabled(true);
   repeat_ratio_box->setEnabled(true);
   repeat_dist_box->setEnabled(true);
   freeze_body->setEnabled(true);
   use_automatic_merging->setEnabled(true);
   //use_jumping_gaps->setEnabled(true);
   //use_jumping_crossover->setEnabled(true);
   general_para21->enableSetting();
   //general_para22->enableSetting();
   //general_para23->enableSetting();

   tracing_models->setEnabled(true);
   seed_expansion_methods->setEnabled(true);
   stretching_force->setEnabled(true);
   image_coding->setEnabled(true);
}
int General_Parameters2::getStretchRatio()
{
	 return stretch_ratio_box->value();
}
void General_Parameters2::setStretchRatio(int in)
{
    stretch_ratio_box->setValue(in);
}
int General_Parameters2::getCollisionDist()
{
	 return collision_dist_box->value();
}
void General_Parameters2::setCollisionDist(int in)
{
    collision_dist_box->setValue(in);
}

int General_Parameters2::getPtDistance()
{
	 return pt_distance_box->value();
}
void General_Parameters2::setPtDistance(int in)
{
    pt_distance_box->setValue(in);
}

double General_Parameters2::getGamma()
{
	 return gamma_box->value();
}
void General_Parameters2::setGamma(double in)
{
    gamma_box->setValue(in);
}

int General_Parameters2::getMinimumLength()
{
	 return minimum_length_box->value();
}
void General_Parameters2::setMinimumLength(int in)
{
    minimum_length_box->setValue(in);
}
int General_Parameters2::getRemoveSeedRange()
{
	 return remove_seed_range_box->value();
}
void General_Parameters2::setRemoveSeedRange(int in)
{
    remove_seed_range_box->setValue(in);
}
float General_Parameters2::getSigmaRatio()
{
	 return k_sigma_box->value();
}
void General_Parameters2::setSigmaRatio(float in)
{
    k_sigma_box->setValue(in);
}
int General_Parameters2::getBorder()
{
	 return border_box->value();
}
void General_Parameters2::setBorder(int in)
{
    border_box->setValue(in);
}

int General_Parameters2::getDeformationITER()
{
	 return deformation_iter_box->value();
}
void General_Parameters2::setDeformationITER(int in)
{
    deformation_iter_box->setValue(in);
}
float General_Parameters2::getRepeatRatio()
{
	 return repeat_ratio_box->value();
}
void General_Parameters2::setRepeatRatio(float in)
{
    repeat_ratio_box->setValue(in);
}
int General_Parameters2::getRepeatDistance()
{
	 return repeat_dist_box->value();
}
void General_Parameters2::setRepeatDistance(int in)
{
    repeat_dist_box->setValue(in);
}

General_Parameters3::General_Parameters3(const QString &title,
                  QWidget *parent)
     : QGroupBox(title, parent)
{

   QBoxLayout *layout = new QVBoxLayout;

   QBoxLayout *layout2 = new QHBoxLayout;
   QBoxLayout *layout3 = new QHBoxLayout;
   QBoxLayout *layout4 = new QHBoxLayout;
   QBoxLayout *layout5 = new QHBoxLayout;
   QBoxLayout *layout6 = new QHBoxLayout;
   QBoxLayout *layout7 = new QHBoxLayout;

   label_X = new QLabel("offset x:");
   label_Y = new QLabel("offset y:");
   label_Z = new QLabel("offset z:");
   offset_X = new QLineEdit;
   offset_Y = new QLineEdit;
   offset_Z = new QLineEdit;

   label_Scale = new QLabel("scale factor");
   edit_Scale = new QLineEdit;
   label_Min_Length = new QLabel("min length");
   edit_Min_Length = new QLineEdit;

   label_Connection = new QLabel("connection");
   edit_Connection = new QLineEdit;

   offset_X->setValidator(new QDoubleValidator(-10000.0,
             10000.0, 3, offset_X));
   offset_Y->setValidator(new QDoubleValidator(-10000.0,
             10000.0, 3, offset_Y));
   offset_Z->setValidator(new QDoubleValidator(-10000.0,
             10000.0, 3, offset_Z));
   edit_Scale->setValidator(new QIntValidator(1,
             10, edit_Scale));
   edit_Min_Length->setValidator(new QIntValidator(1,
             500, edit_Min_Length));
   edit_Connection->setValidator(new QIntValidator(1,
             100, edit_Connection));
   offset_X->setText("0");
   offset_Y->setText("0");
   offset_Z->setText("0");
   edit_Scale->setText("1");
   edit_Min_Length->setText("5");
   edit_Connection->setText("2");

   layout2->addWidget(label_X);
   layout2->addWidget(offset_X);
   layout3->addWidget(label_Y);
   layout3->addWidget(offset_Y);
   layout4->addWidget(label_Z);
   layout4->addWidget(offset_Z);

   layout5->addWidget(label_Scale);
   layout5->addWidget(edit_Scale);
   label_Scale->setVisible(false);
   edit_Scale->setVisible(false);

   layout6->addWidget(label_Connection);
   layout6->addWidget(edit_Connection);
   layout7->addWidget(label_Min_Length);
   layout7->addWidget(edit_Min_Length);

   load_swc_button = new QPushButton("Load SWC");
   output_swc_raw_button = new QPushButton("Output Raw SWC");
   output_swc_button = new QPushButton("Output SWC");
   
   layout->addLayout(layout2);
   layout->addLayout(layout3);
   layout->addLayout(layout4);
   layout->addLayout(layout5);
   layout->addLayout(layout6);
   layout->addLayout(layout7);

   layout->addWidget(load_swc_button);
   layout->addWidget(output_swc_raw_button);
   layout->addWidget(output_swc_button);
   setLayout(layout);
}


void General_Parameters3::setEliteSetting()
{
   label_X->setVisible(false);
   offset_X->setVisible(false);
   label_Y->setVisible(false);
   offset_Y->setVisible(false);
   label_Z->setVisible(false);
   offset_Z->setVisible(false);

   label_Min_Length->setVisible(false);
   edit_Min_Length->setVisible(false);

   label_Connection->setVisible(false);
   edit_Connection->setVisible(false);
}

void General_Parameters3::setFullSetting()
{
   label_X->setVisible(true);
   offset_X->setVisible(true);
   label_Y->setVisible(true);
   offset_Y->setVisible(true);
   label_Z->setVisible(true);
   offset_Z->setVisible(true);

   label_Min_Length->setVisible(true);
   edit_Min_Length->setVisible(true);

   label_Connection->setVisible(true);
   edit_Connection->setVisible(true);
}



void General_Parameters3::setX(float in)
{
    X = in;
}
void General_Parameters3::setY(float in)
{
    Y = in;
}
void General_Parameters3::setZ(float in)
{
    Z = in;
}
void General_Parameters3::setScale(int in)
{
     Scale = in;
	 QString temp;
	 temp.setNum(Scale);
	 edit_Scale->setText(temp);
}
void General_Parameters3::setMinLength(int in)
{
     MinLength = in;
	 QString temp;
	 temp.setNum(MinLength);
	 edit_Min_Length->setText(temp);
}
void General_Parameters3::setConnection(int in)
{
     Connection = in;
	 QString temp;
	 temp.setNum(Connection);
	 edit_Connection->setText(temp);
}
float General_Parameters3::getX()
{
	X = offset_X->text().toFloat();
    return X;
}
float General_Parameters3::getY()
{
	Y = offset_Y->text().toFloat();
    return Y;
}
float General_Parameters3::getZ()
{
	Z = offset_Z->text().toFloat();
    return Z;
}
int General_Parameters3::getScale()
{
	Scale = edit_Scale->text().toInt();
    return Scale;
}
int General_Parameters3::getMinLength()
{
	MinLength = edit_Min_Length->text().toInt();
	return MinLength;
}
int General_Parameters3::getConnection()
{
	Connection = edit_Connection->text().toInt();
    return Connection;
}
