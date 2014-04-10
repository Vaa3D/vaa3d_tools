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
#ifndef OPENSNAKETRACER_H
#define OPENSNAKETRACER_H

#include "itkNumericTraits.h"
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.h>
#include <vnl/algo/vnl_matrix_inverse.h>
#include <QTextEdit>
#include "TracingCore/SnakeOperation.h"
#include "qthread.h"
#include "qmutex.h"
#include "qwaitcondition.h"

class TracingThread;

class OpenSnakeTracer
{
public:

    SnakeListClass SnakeList;
	void Init();
	void SetImage(ImageOperation *I_Input);
	void Open_Curve_Snake_Tracing();
	void Cast_Open_Snake_3D();
   

	//Functions for GUI
	QTextEdit *log_viewer;

    int seed_num;

	ImageOperation *IM;
	//thread for controlling snake stretching and dynamic displaying
	TracingThread *tracing_thread;
};


class TracingThread : public QThread
{
 Q_OBJECT
 public:

	TracingThread();
    
	void setIterNum(int num);
	void setSnake(SnakeClass *s);
    void run();
	void stop();
	void suspend();
	void resume();
    volatile bool stopped;
	bool ToSuspend;

	SnakeClass *snake;
    int iter_num;

	QWaitCondition condition;
	QMutex mutex;
   
 public slots:

 signals:
    void stretched();

 private:

};

#endif
