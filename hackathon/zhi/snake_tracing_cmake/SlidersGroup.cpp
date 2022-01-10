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

 #include "SlidersGroup.h"

 SlidersGroup::SlidersGroup(bool Vertical,
                            QWidget *parent)
 {
     QBoxLayout *layout = new QHBoxLayout;
     
	 RawImage = new QCheckBox("Raw", this);
	 OriginalImage = new QCheckBox("Preprocessed",this);
	 VesselImage = new QCheckBox("Vesselness",this);
	 SkeletonImage = new QCheckBox("Label",this);

	 layout->addWidget(RawImage);
	 layout->addWidget(OriginalImage);
	 layout->addWidget(VesselImage);
	 layout->addWidget(SkeletonImage);

	 QFrame *f = new QFrame( this );
     f->setFrameStyle( QFrame::VLine | QFrame::Sunken );
	 layout->addWidget(f);

	 displaySeeds = new QCheckBox("Seeds",this);
	 displaySnakes = new QCheckBox("Snakes",this);
	 colorLine = new QCheckBox("Color Lines",this);
     radiusSphere = new QCheckBox("Radius Sphere",this);
     radiusCircle = new QCheckBox("Radius Circle",this);
	 radiusTube = new QCheckBox("Tube", this);
	 dynamicDisplay = new QCheckBox("Dynamic Display", this);

	 layout->addWidget(displaySeeds);
     layout->addWidget(displaySnakes);
	 layout->addWidget(colorLine);
	 layout->addWidget(radiusSphere);
	 layout->addWidget(radiusCircle);
	 layout->addWidget(radiusTube);
	 layout->addWidget(dynamicDisplay);

	 slice = new QLabel("Slice");
     layout->addWidget(slice);

	 slider = new QSlider(Qt::Horizontal);
     //slider->setFocusPolicy(Qt::StrongFocus);
     //slider->setTickPosition(QSlider::TicksBothSides);
     //slider->setTickInterval(10);
     slider->setSingleStep(1);
	 slider->setValue(1);
     connect(slider, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)));

	 connect(RawImage, SIGNAL(stateChanged(int)), this, SLOT(processChangeM1()));
	 connect(OriginalImage, SIGNAL(stateChanged(int)), this, SLOT(processChange1()));
	 connect(VesselImage, SIGNAL(stateChanged(int)), this, SLOT(processChange2()));
     connect(SkeletonImage, SIGNAL(stateChanged(int)), this, SLOT(processChange3()));

     connect(displaySeeds, SIGNAL(stateChanged(int)), this, SLOT(displaySeedChange()));
	 connect(displaySnakes, SIGNAL(stateChanged(int)), this, SLOT(displaySnakeChange()));
	 connect(colorLine, SIGNAL(stateChanged(int)), this, SLOT(colorLineChange()));
     connect(radiusSphere, SIGNAL(stateChanged(int)), this, SLOT(radiusSphereChange()));
	 connect(radiusCircle, SIGNAL(stateChanged(int)), this, SLOT(radiusCircleChange()));
	 connect(radiusTube, SIGNAL(stateChanged(int)), this, SLOT(radiusTubeChange()));

     layout->addWidget(slider);
   
	 frame_box = new QSpinBox;
	 frame_box->setValue(1);
	 connect(slider, SIGNAL(valueChanged(int)), frame_box, SLOT(setValue(int)));
	 connect(frame_box, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
	 layout->addWidget(frame_box);

     //QBoxLayout *slidersLayout = new QHBoxLayout;
	 RawImage->setEnabled(false);
	 displaySeeds->setChecked(true);
	 displaySnakes->setChecked(true);
	 setSliderAvailable(false);
     setLayout(layout);
 }

 void SlidersGroup::displaySeedChange()
 {
   if(displaySeeds->isChecked())
   {
	   emit displaySeedsChanged(true);
   }
   else
   {
       emit displaySeedsChanged(false);
   }
 }

 void SlidersGroup::displaySnakeChange()
 {
   if(displaySnakes->isChecked())
   {
	   emit displaySnakesChanged(true);
   }
   else
   {
       emit displaySnakesChanged(false);
   }
 }

 void SlidersGroup::colorLineChange()
 {
   if(colorLine->isChecked())
   {
	   emit colorLinesChanged(true);
   }
   else
   {
       emit colorLinesChanged(false);
   }
 }

 void SlidersGroup::radiusSphereChange()
 {
   if(radiusSphere->isChecked())
   {
	   if(radiusCircle->isChecked())
	    emit radiusSphereChanged(3);
	   else
	    emit radiusSphereChanged(1);
   }
   else
   { 
	  if(radiusCircle->isChecked())
       emit radiusSphereChanged(2);
	  else
	   emit radiusSphereChanged(0);
   }
 }

 void SlidersGroup::radiusTubeChange()
 {
     emit radiusTubeChanged();
 }

 void SlidersGroup::radiusCircleChange()
 {
   if(radiusCircle->isChecked())
   {
      if(radiusSphere->isChecked())
	   emit radiusCircleChanged(3);
	  else
	   emit radiusCircleChanged(2);
   }
   else
   {
      if(radiusSphere->isChecked())
       emit radiusCircleChanged(1);
	  else
       emit radiusCircleChanged(0);
   }
 }

 void SlidersGroup::processChangeM1()
 {
   	 if(RawImage->isChecked())
	 {
	   State = -1;
	 }
	 if( !RawImage->isChecked() && !OriginalImage->isChecked() && !VesselImage->isChecked() && !SkeletonImage->isChecked() )
	 {
	   State = 0;
	 }


	 if(State == 0)
	 {
	   State = 1;
	   setImageState(1);
	   slider->setValue(1);
	   emit stateChanged(1);
	 }
	 else if(State == -1)
	 {
	   //VesselImage->setCheckable(false);
	   //BinaryImage->setCheckable(false);
	   OriginalImage->setCheckState(Qt::Unchecked);
	   VesselImage->setCheckState(Qt::Unchecked);
	   SkeletonImage->setCheckState(Qt::Unchecked);
	   //DepthImage->setCheckState(Qt::Unchecked);
	   slider->setValue(-1);
	   emit stateChanged(-1);
	 }
 }

 void SlidersGroup::processChange1()
 {
   	 if(OriginalImage->isChecked())
	 {
	   State = 1;
	 }
	 if( !RawImage->isChecked() && !OriginalImage->isChecked() && !VesselImage->isChecked() && !SkeletonImage->isChecked() )
	 {
	   State = 0;
	 }


	 if(State == 0)
	 {
	   State = 1;
	   setImageState(1);
	   slider->setValue(1);
	   emit stateChanged(1);
	 }
	 else if(State == 1)
	 {
	   //VesselImage->setCheckable(false);
	   //BinaryImage->setCheckable(false);
	   RawImage->setCheckState(Qt::Unchecked);
	   VesselImage->setCheckState(Qt::Unchecked);
	   SkeletonImage->setCheckState(Qt::Unchecked);
	   //DepthImage->setCheckState(Qt::Unchecked);
	   slider->setValue(1);
	   emit stateChanged(1);
	 }
 }

 void SlidersGroup::processChange2()
 {
	 if(VesselImage->isChecked())
	 {
	   State = 2;
	 }
	 if( !RawImage->isChecked() && !OriginalImage->isChecked() && !VesselImage->isChecked() && !SkeletonImage->isChecked() )
	 {
	   State = 0;
	 }

	 if(State == 0)
	 {
	   State = 1;
	   setImageState(1);
	   slider->setValue(1);
	   emit stateChanged(1);
	 }
	 else if(State == 2)
	 {
	   //VesselImage->setCheckable(false);
	   //BinaryImage->setCheckable(false);
	   RawImage->setCheckState(Qt::Unchecked);
	   OriginalImage->setCheckState(Qt::Unchecked);
	   SkeletonImage->setCheckState(Qt::Unchecked);
	   //DepthImage->setCheckState(Qt::Unchecked);
	   slider->setValue(1);
	   emit stateChanged(2);
	 }
 }

 void SlidersGroup::processChange3()
 {
   	 if(SkeletonImage->isChecked())
	 {
	   State = 3;
	 }
     if( !RawImage->isChecked() && !OriginalImage->isChecked() && !VesselImage->isChecked() && !SkeletonImage->isChecked() )
	 {
	   State = 0;
	 }

	 if(State == 0)
	 {
	   State = 1;
	   setImageState(1);
	   slider->setValue(1);
	   emit stateChanged(1);
	 }
	 else if(State == 3)
	 {
	   //VesselImage->setCheckable(false);
	   //BinaryImage->setCheckable(false);
	   RawImage->setCheckState(Qt::Unchecked);
	   OriginalImage->setCheckState(Qt::Unchecked);
	   VesselImage->setCheckState(Qt::Unchecked);
	   //DepthImage->setCheckState(Qt::Unchecked);
	   slider->setValue(1);
	   emit stateChanged(3);
	 }
 }

 void SlidersGroup::setSliderAvailable(bool in)
 {
    if( in )
	 slider->setEnabled(true);
	else
	 slider->setEnabled(false);
 }

 void SlidersGroup::setCheckable(int in)
 {
   if( in == -1 )
   {
	   RawImage->setEnabled(true);
   }
   if( in == 0 )
   {
       OriginalImage->setEnabled(false);
	   VesselImage->setEnabled(false);
	   SkeletonImage->setEnabled(false);
	   //DepthImage->setEnabled(false);
   }
   else if(in == 1)
   {   
       OriginalImage->setEnabled(true);
	   VesselImage->setEnabled(false);
	   SkeletonImage->setEnabled(false);
	   //DepthImage->setEnabled(false);
   }
   else if(in == 2)
   {
       OriginalImage->setEnabled(true);
	   VesselImage->setEnabled(true);
	   SkeletonImage->setEnabled(false);
	   //DepthImage->setEnabled(false);
   }
   else if(in == 3)
   {
       OriginalImage->setEnabled(true);
	   VesselImage->setEnabled(true);
	   SkeletonImage->setEnabled(true);
	   //DepthImage->setEnabled(false);
   }
 }

 void SlidersGroup::setValue(int value)
 {
     slider->setValue(value);
	 frame_box->setValue(value);
 }

 int SlidersGroup::getValue()
 {
     return frame_box->value();
 }

 bool SlidersGroup::getDisplaySeed()
 {
     return displaySeeds->isChecked();
 }
 bool SlidersGroup::getDisplaySnakes()
 {
     return displaySnakes->isChecked();
 }
 bool SlidersGroup::getColorDisplay()
 {
     return colorLine->isChecked();
 }
 int SlidersGroup::getRadiusDisplay()
 {
   int s = 0;
   if( radiusSphere->isChecked() )
   {
      s = 1;
	  if( radiusCircle->isChecked() )
		  s = 3;
   }
   else if( radiusCircle->isChecked() )
   {
      s = 2;
   }
   
   return s;
 }
 bool SlidersGroup::getTubeDisplay()
 {
	 bool state;
    if( radiusTube->isChecked() )
	{
		state = true;
		return state;
	}
	else
	{   state = false;
		return state;
	}
 }
 bool SlidersGroup::getDynamicDisplay()
 {
     bool state;
 if( dynamicDisplay->isChecked() )
	{
		state = true;
		return state;
	}
	else
	{   state = false;
		return state;
	}

 }

 void SlidersGroup::setMinimum(int value)
 {
     slider->setMinimum(value);
	 frame_box->setMinimum(value);
 }

 void SlidersGroup::setMaximum(int value)
 {
     slider->setMaximum(value);
	 frame_box->setMaximum(value);
 }

 void SlidersGroup::setStep(int step)
 {
     slider->setSingleStep(step);
	 frame_box->setSingleStep(step);
 }

 void SlidersGroup::setDisplaySeed(bool in)
 {
   if( in )
	   displaySeeds->setCheckState(Qt::Checked);
   else
	   displaySeeds->setCheckState(Qt::Unchecked);
 }

void SlidersGroup::setImageState(int in)
{
   State = in;
   if(in == -1)
   {
	   RawImage->setCheckState(Qt::Checked);
	   OriginalImage->setCheckState(Qt::Unchecked);
	   VesselImage->setCheckState(Qt::Unchecked);
	   SkeletonImage->setCheckState(Qt::Unchecked);
	   //DepthImage->setCheckState(Qt::Unchecked);
   }
   else if(in == 1)
   {
	   RawImage->setCheckState(Qt::Unchecked);
	   OriginalImage->setCheckState(Qt::Checked);
	   VesselImage->setCheckState(Qt::Unchecked);
	   SkeletonImage->setCheckState(Qt::Unchecked);
	   //DepthImage->setCheckState(Qt::Unchecked);
   }
   else if(in == 2)
   {
	   RawImage->setCheckState(Qt::Unchecked);
	   OriginalImage->setCheckState(Qt::Unchecked);
	   VesselImage->setCheckState(Qt::Checked);
	   SkeletonImage->setCheckState(Qt::Unchecked);
	   //DepthImage->setCheckState(Qt::Unchecked);
   }
   else if(in == 3)
   {
	   RawImage->setCheckState(Qt::Unchecked);
	   OriginalImage->setCheckState(Qt::Unchecked);
	   VesselImage->setCheckState(Qt::Unchecked);
	   SkeletonImage->setCheckState(Qt::Checked);
	   //DepthImage->setCheckState(Qt::Unchecked);
   }

}


