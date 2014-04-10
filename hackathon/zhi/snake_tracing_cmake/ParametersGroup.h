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
  Program:   Open Snake Tracing System
  Autohr:    Yu Wang
  Email: wangy15@rpi.edu
  Language:  C++
  Date:      $Date:  $
  Version:   $Revision: 0.00 $
=========================================================================*/

#ifndef PARAMETERSGROUP_H
#define PARAMETERSGROUP_H

#include <QtGui>
class General_Parameters1;
class General_Parameters12;
class General_Parameters2;
class General_Parameters21;
class General_Parameters22;
class General_Parameters23;

class General_Parameters : public QGroupBox
{
     Q_OBJECT

 public:
     General_Parameters(const QString &title,
                  QWidget *parent = 0);

	 void setEliteSetting();
	 void setFullSetting();

	 int getRemoveSlice();
	 void setRemoveSlice(int in);
	 int getSf(); //scale factor
	 int getSh(); //shrink factor
	 void setSh(int in);
	 void setSf(int in);

	 bool getContinueAutoProcessing();
	 bool getBatchIProcessing();
	 bool getBatchIIProcessing();

	 bool getSixteenBit();

	 void disableSetting();
	 void enableSetting();

	 QPushButton *stackImageButton;
	 QPushButton *loadButton;
	 QPushButton *loadDisplayButton;
	 QPushButton *reloadButton;
	 QPushButton *autoButton;
	 QPushButton *batchButtonI;
	 QPushButton *batchButtonII;
	 QPushButton *montageButton;

 public slots:
     void batchI_Slot();
	 void batchII_Slot();
	 void auto_Slot();

 signals:
	 void auto_Selected(bool);
	 void batchI_Selected(bool);
	 void batchII_Selected(bool);

 private:

	 QGroupBox *stack_image, *step_process, *automated_process, *montage_process;
	 QLabel *remove_slice_label;
	 QSpinBox *remove_slice_box;
	 QLabel *sh_label;
	 QSpinBox *sh_box;
	 QLabel *sf_label;
	 QSpinBox *sf_box;
	 QRadioButton *no_processing_after;
	 QRadioButton *continue_auto_processing;
	 QRadioButton *continue_batchI_processing;
	 QRadioButton *continue_batchII_processing;
	 QCheckBox *sixteen_bit;

 };

//multi-scale preprocessing parameters
class General_Parameters1 : public QGroupBox
{
     Q_OBJECT

 public:
     General_Parameters1(const QString &title,
                  QWidget *parent = 0);
	 double getMinSigma();
	 void setMinSigma(double in);
	 double getMaxSigma();
	 void setMaxSigma(double in);
	 int getSigmaStep();
	 void setSigmaStep(int in);

	 void disableSetting();
	 void enableSetting();

 private:

	 QLabel *min_sigma_label;
	 QDoubleSpinBox *min_sigma_box;
     QLabel *max_sigma_label;
	 QDoubleSpinBox *max_sigma_box;
	 QLabel *sigma_step_label;
	 QSpinBox *sigma_step_box;

 };

//re-processing buttons
class General_Parameters11 : public QGroupBox
{
     Q_OBJECT

 public:
     General_Parameters11(const QString &title,
                  QWidget *parent = 0);
	 QPushButton *detectSeeds;
	 void disableSetting();
	 void enableSetting();

 private:

};

//preprocessing parameters
class General_Parameters12 : public QGroupBox
{
     Q_OBJECT

 public:
     General_Parameters12(const QString &title,
                  QWidget *parent = 0);

	 void setEliteSetting();
	 void setFullSetting();

	 bool getLaplacianOfGaussian();
	 double getNoiseLevel();
	 void setNoiseLevel(double in);
	 int getNumIteration();
	 void setNumIteration(int in);
	 double getThreshold();
	 void setThreshold(double in);
     int getSeedAdjustment();
	 void setSeedAdjustment(int in);
	 int getSeedRadius();
	 void setSeedRadius(int in);
	 int getSmoothingScale();
	 void setSmoothingScale(int in);

	 int getCurrentSeedIndex();
	 void setCurrentSeedIndex(int in);
	 int getCurrentGPUIndex();

	 bool getMultiScale();
     void setMultiScale(int in);

	 int getCurveletBatch();

	 void disableSetting();
	 void enableSetting();
	 void setCurveletBatchEnabled(bool in);

	 QPushButton *preprocessButton1;
	 QPushButton *preprocessButton;
     QPushButton *outputSeeds;
	 QPushButton *reprocessingButton;

	 General_Parameters1 *general_para1;
	 General_Parameters11 *general_para11;

 public slots:
	 void seed_slot();
	 void reprocessing_slot();
	 void use_multi_scale_slot();
 signals:
	 void reprocess(int in);
 private:

	 QGroupBox *outside_preprocess, *inside_preprocess;

	 QLabel *noise_level_label;
	 QDoubleSpinBox *noise_level_box;
     QLabel *num_iteration_label;
	 QSpinBox *num_iteration_box;
     QLabel *threshold_label;
	 QDoubleSpinBox *threshold_box;

	 QLabel *seed_adjustment_label;
	 QSpinBox *seed_adjustment_box;
	 QLabel *seed_radius_label;
	 QSpinBox *seed_radius_box;

	 QLabel *smoothing_scale_label;
	 QSpinBox *smoothing_scale_box;

	 QCheckBox *use_multi_scale;
	 QCheckBox *use_laplacian_of_gaussian;

	 QComboBox *gpu_acceleration;
	 QComboBox *seed_detection_methods;

	 QRadioButton *curvelet_before_batch;
	 QRadioButton *curvelet_during_batch;

     bool reprocessing;
};


//automatic merging group box
class General_Parameters21 : public QGroupBox
{
     Q_OBJECT

 public:
  General_Parameters21(const QString &title,
                  QWidget *parent = 0);

     int getMaxAngle();
     void setMaxAngle(double in);
	 void disableSetting();
	 void enableSetting();

 private:

	 QLabel *max_angle_label;
	 QSpinBox *max_angle_box;
	      
};

//jumping over gaps box
class General_Parameters22 : public QGroupBox
 {
     Q_OBJECT

 public:
  General_Parameters22(const QString &title,
                  QWidget *parent = 0);

     int getDist();
     void setDist(int in);
	 int getAngle();
	 void setAngle(int in);
	 void disableSetting();
	 void enableSetting();

 private:
	 QLabel *dist_label;
	 QSpinBox *dist_box;

	 QLabel *angle_label;
	 QSpinBox *angle_box;  
};

//jumping crossover box
class General_Parameters23 : public QGroupBox
{
     Q_OBJECT

 public:
  General_Parameters23(const QString &title,
                  QWidget *parent = 0);

     int getDist();
     void setDist(int in);

	 void disableSetting();
	 void enableSetting();

 private:

	 QLabel *dist_label;
	 QSpinBox *dist_box;
};


//tracing parameters
class General_Parameters2 : public QGroupBox
{
     Q_OBJECT

 public:
     General_Parameters2(const QString &title,
                  QWidget *parent = 0);

	 void setEliteSetting();
     void setFullSetting();

	 int getStretchRatio();
	 void setStretchRatio(int in);
	 int getCollisionDist();
	 void setCollisionDist(int in);

	 int getPtDistance();
	 void setPtDistance(int in);
     double getGamma();
	 void setGamma(double in);
	 int getMinimumLength();
	 void setMinimumLength(int in);

	 int getRemoveSeedRange();
	 void setRemoveSeedRange(int in);

	 float getSigmaRatio();
	 void setSigmaRatio(float in);

	 int getBorder();
	 void setBorder(int in);

	 int getDeformationITER();
	 void setDeformationITER(int in);

	 float getRepeatRatio();
	 void setRepeatRatio(float in);

 	 int getRepeatDistance();
	 void setRepeatDistance(int in);

	 bool getFreezeBody();
	 void setFreezeBody(int in);
	 bool getAutomaticMerging();
	 void setAutomaticMerging(int in);
	 bool getMultiThreads();
	 void setMultiThreads(int in);
	 /*bool getJumpingGaps();
	 void setJumpingGaps(int in);
	 bool getJumpingCrossover();
	 void setJumpingCrossover(int in);*/

	 int getCurrentTracing();
	 void setCurrentTracing(int in);
	 int getCurrentIndex();
	 void setCurrentIndex(int in);
	 int getCurrentForce();
	 void setCurrentForce(int in);
	 int getCurrentCoding();
	 void setCurrentCoding(int in);

	 void disableSetting();
	 void enableSetting();
     bool enabled;

	 QPushButton *tracingButton;

	 General_Parameters21 *general_para21;
	 //General_Parameters22 *general_para22;
	 //General_Parameters23 *general_para23;
	
 public slots:
     void use_automatic_merging_slot();
	 //void use_jumping_gaps_slot();
	 //void use_jumping_crossover_slot();

 private:

	 QComboBox *tracing_models;
	 QComboBox *seed_expansion_methods;
	 QComboBox *stretching_force;
	 QComboBox *image_coding;

	 QLabel *stretch_ratio_label;
	 QSpinBox *stretch_ratio_box;
     //QLabel *num_iteration_label;
	 //QSpinBox *num_iteration_box;

     QLabel *collision_dist_label;
	 QSpinBox *collision_dist_box;

	 QLabel *pt_distance_label;
	 QSpinBox *pt_distance_box;
	 QLabel *gamma_label;
	 QDoubleSpinBox *gamma_box;
	 QLabel *minimum_length_label;
	 QSpinBox *minimum_length_box;

	 QLabel *remove_seed_range_label;
	 QSpinBox *remove_seed_range_box;

	 QLabel *k_sigma_label;
	 QDoubleSpinBox *k_sigma_box;

	 QLabel *border_label;
	 QSpinBox *border_box;

	 QLabel *deformation_iter_label;
	 QSpinBox *deformation_iter_box;

	 QLabel *repeat_ratio_label;
	 QDoubleSpinBox *repeat_ratio_box;

	 QLabel *repeat_dist_label;
	 QSpinBox *repeat_dist_box;

	 QCheckBox *freeze_body;
	 QCheckBox *use_automatic_merging;
	 QCheckBox *multi_threads;
	 //QCheckBox *use_jumping_gaps;
	 //QCheckBox *use_jumping_crossover;
	 
};


class General_Parameters3 : public QGroupBox
{
     Q_OBJECT

 public:
     General_Parameters3(const QString &title,
                  QWidget *parent = 0);

	 void setEliteSetting();
     void setFullSetting();

     void setX(float in);
	 void setY(float in);
	 void setZ(float in);
	 void setScale(int in);
	 void setMinLength(int in);
	 void setConnection(int in);
	 float getX();
	 float getY();
	 float getZ();
	 int getScale();
	 int getMinLength();
	 int getConnection();
	

	 QPushButton *load_swc_button;
	 QPushButton *output_swc_raw_button;
	 QPushButton *output_swc_button;
	 
 private:

	 QLabel *label_X;
	 QLineEdit *offset_X;
	 QLabel *label_Y;
	 QLineEdit *offset_Y;
     QLabel *label_Z;
 	 QLineEdit *offset_Z;

	 QLabel *label_Scale;
	 QLineEdit *edit_Scale;
	 
 	 QLabel *label_Min_Length;
	 QLineEdit *edit_Min_Length;

	 QLabel *label_Connection;
	 QLineEdit *edit_Connection;

	 float X,Y,Z;
	 int Scale;
	 int MinLength;
	 int Connection;
};

#endif
