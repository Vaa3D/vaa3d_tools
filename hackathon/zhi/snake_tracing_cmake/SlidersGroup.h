/****************************************************************************
 **
 ** Copyright (C) 2004-2007 Trolltech ASA. All rights reserved.
 **
 ** This file is part of the example classes of the Qt Toolkit.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License version 2.0 as published by the Free Software Foundation
 ** and appearing in the file LICENSE.GPL included in the packaging of
 ** this file.  Please review the following information to ensure GNU
 ** General Public Licensing requirements will be met:
 ** http://www.trolltech.com/products/qt/opensource.html
 **
 ** If you are unsure which license is appropriate for your use, please
 ** review the following information:
 ** http://www.trolltech.com/products/qt/licensing.html or contact the
 ** sales department at sales@trolltech.com.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

 #ifndef SLIDERSGROUP_H
 #define SLIDERSGROUP_H

#include <QtGui>

 class SlidersGroup : public QWidget
 {
     Q_OBJECT

 public:
     SlidersGroup(bool Vertical,
                  QWidget *parent = 0);
	 int getValue();
	 bool getDisplaySeed();
	 bool getDisplaySnakes();
	 bool getColorDisplay();
	 int getRadiusDisplay();
	 bool getTubeDisplay();
	 bool getDynamicDisplay();
	 int State;

 signals:
     void valueChanged(int value);
     void stateChanged(int value);
     void displaySeedsChanged(bool value);
	 void displaySnakesChanged(bool value);
	 void colorLinesChanged(bool value);
     void radiusSphereChanged(int value);
	 void radiusCircleChanged(int value);
	 void radiusTubeChanged();

 public slots:
     void setValue(int value);
     void setMinimum(int value);
     void setMaximum(int value);
	 void setStep(int value);
	 void setImageState(int in);
	 void setDisplaySeed(bool in);

	 void processChangeM1();
	 void processChange1();
	 void processChange2();
	 void processChange3();

	 void displaySeedChange();
	 void displaySnakeChange();
	 void colorLineChange();
	 void radiusSphereChange();
	 void radiusCircleChange();
	 void radiusTubeChange();
	 void setCheckable(int in);
     void setSliderAvailable(bool in);

 private:
     QSlider *slider;
	 QCheckBox *RawImage;
	 QCheckBox *OriginalImage;
	 QCheckBox *VesselImage;
	 QCheckBox *SkeletonImage;

	 QCheckBox *displaySeeds;
	 QCheckBox *displaySnakes;
	 QCheckBox *colorLine;
	 QCheckBox *radiusSphere;
	 QCheckBox *radiusCircle;
	 QCheckBox *radiusTube;
	 QCheckBox *dynamicDisplay;
	 QSpinBox *frame_box;
	 QLabel *slice;

 };




 #endif

