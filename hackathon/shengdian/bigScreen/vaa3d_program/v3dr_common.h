/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it.

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) ?�V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,??Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) ?�Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,??Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




/*
 * v3dr_common.h
 *
 *  Created on: Sep 24, 2008
 *      Author: Zongcai Ruan and Hanchuan Peng
 * Last change. 20100406. change ZTHICK_RANGE to 20
 */

#pragma warning(disable: 4430) // To silence a type of warning shown as error in VS2015.

#ifndef V3DR_COMMON_H_
#define V3DR_COMMON_H_

// Added by MK, 11/21/2016, for migrating from VS2010/Qt4 to VS2015/Qt5
#include "../v3d/version_control.h"

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
  #include <QtWidgets>
#else
  #include <QtGui>
#endif

#if defined(USE_GLEW)
  #include <GL\glew.h>
//#else
//  #include "GLee_r.h"
#endif

#include "GLee_r.h" //must before any other OpengGL header file// minor modified glee.h for win32 compatible, by RZC 2008-09-12

// #include <QtOpenGL>
//#include <QtTest>

#include <exception>
#include <iostream>

//instead by freeglut_geometry_r.c
//#include "glut32.h" // minor modified 2000 glut.h for mingw

#include "../jba/newmat11/newmatap.h"
#include "../jba/newmat11/newmatio.h"

#include "../basic_c_fun/basic_4dimage.h"
#include "../basic_c_fun/basic_surf_objs.h"
#include "../basic_c_fun/color_xyz.h"

#include "../basic_c_fun/basic_landmark.h"
#include "../neuron_editing/v_neuronswc.h"

//#include "color_xyz.h" // RGB/RGBA/8/16i/32i/32f, XYZ/XYZW, BoundingBox


//=====================================================================================================================
//#ifdef WIN32  //081001: limited by 3G
//	#define LIMIT_VOLX 512
//	#define LIMIT_VOLY 512
//	#define LIMIT_VOLZ 128
//#else
	#define LIMIT_VOLX 512
	#define LIMIT_VOLY 512
	#define LIMIT_VOLZ 256
//#endif
//	#define LIMIT_VOLX 256
//	#define LIMIT_VOLY 256

#define IS_LESS_64BIT ((sizeof(void*)<8)? true:false)
#define IS_FITTED_VOLUME(dim1,dim2,dim3)  (dim1<=LIMIT_VOLX && dim2<=LIMIT_VOLY && dim3<=LIMIT_VOLZ)

#define ANGLE_X0 (15)			//degree
#define ANGLE_Y0 (360-20)		//degree
#define ANGLE_Z0 (360-2)		//degree
#define ANGLE_TICK 1 			//divided
#define MOUSE_SENSITIVE 1.0f
#define SHIFT_RANGE 100 		//percent of bound
#define ZOOM_RANGE  100         //percent of fov
#define ZOOM_RANGE_RATE 5       //zoom rate of fov
#define CLIP_RANGE  200 		//size of (-100,100)
#define ZTHICK_RANGE 20			//times
#define TRANSPARENT_RANGE 100   //nonlinear divided

#define POPMENU_OPACITY 1

// 081025 by RZC
#define WIN_SIZEX 1024 //800
#define WIN_SIZEY 768  //800
#define CTRL_SIZEX 350
#define MINVIEW_SIZEX 400  //800
#define MINVIEW_SIZEY 400  //800

#define QEvent_Ready (QEvent::User +1)
#define QEvent_OpenFiles (QEvent::User +2)
#define QEvent_DropFiles (QEvent::User +3)
#define QEvent_InitControlValue (QEvent::User +4)
#define QEvent_OpenLocal3DView (QEvent::User +5)
#define QEvent_HistoryChanged (QEvent::User +6) //20170801 RZC: a notify after Undo Histoty changed


//========================================================================================================================
#ifdef Q_WS_MAC
#define CTRL2_MODIFIER Qt::MetaModifier
#else
#define CTRL2_MODIFIER Qt::ControlModifier
#endif

#define ALT2_MODIFIER Qt::AltModifier
#ifdef Q_WS_X11
#define ATL2_MODIFIER Qt::GroupSwitchModifier
#endif


#define POST_EVENT(pQ, eventType) {	if (pQ!=NULL) QCoreApplication::postEvent(pQ, new QEvent( QEvent::Type(eventType) )); }
#define SEND_EVENT(pQ, eventType) {	if (pQ!=NULL) QCoreApplication::sendEvent(pQ, new QEvent( QEvent::Type(eventType) )); }
#define POST_CLOSE(pQ)	POST_EVENT(pQ, QEvent::Close)
#define ACTIVATE(w)	  { if(w) {QWidget* pQ=(QWidget*)w; pQ->raise(); POST_EVENT(pQ, QEvent::MouseButtonPress);} }

#define SLEEP(t)  { QTime qtime;  qtime.start();  while( qtime.elapsed() < t); }

#define DELETE_AND_ZERO(p)	{ if ((p)!=NULL) delete (p); (p) = NULL; }
#define Q_CSTR(qs)  ( (qs).toStdString().c_str() )
#define QCOLOR_BGRA8(bgra)  ( QColor::fromRgba((unsigned int)(bgra)) )  //QRgb = #AArrGGbb = BGRA8

#define MESSAGE(s) \
{\
	QMessageBox::information(0, "MESSAGE", QObject::tr("%1 \n\n in file(%2) at line(%3)").arg(s).arg(__FILE__).arg(__LINE__)); \
}
#define MESSAGE_ASSERT(s) \
{\
	if (!(s)) \
		QMessageBox::critical(0, "ASSERT", QObject::tr("ASSERT(%1) in file(%2) at line(%3)").arg(#s).arg(__FILE__).arg(__LINE__)); \
	Q_ASSERT(s); \
}

#define CATCH_TO_QString( type, msg ) \
	catch (std::exception& e) { \
		type = "[std]"; \
		msg = e.what(); \
	} \
	catch (int id) { \
		type = "[int]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (V3DLONG id) { \
		type = "[V3DLONG]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (unsigned int id) { \
		type = "[uint]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (unsigned V3DLONG id) { \
		type = "[ulong]"; \
		msg = QString("exception id = %1").arg(id); \
	} \
	catch (char* str) { \
		type = "[char]"; \
		msg = (const char*)str; \
	} \
	catch (const char* str) { \
		type = "[cchar]"; \
		msg = str; \
	} \
	catch (...) { \
		type = "[unknown]"; \
		msg = QString("unknown exception in file(%1) at line(%2)").arg(__FILE__).arg(__LINE__)); \
	}


#define PROGRESS_DIALOG(text, widget)  	QProgressDialog progress( QString(text), 0, 0, 100, (QWidget*)widget, Qt::Tool | Qt::WindowStaysOnTopHint \
										| Qt::CustomizeWindowHint | Qt::WindowTitleHint ); //only title bar, disable buttons on title bar
extern QProgressDialog progress;
#define PROGRESS_PARENT(widget)   progress.setParent( (QWidget*)widget ); //Qt::WShowModal
#define PROGRESS_TEXT(text)   { QApplication::setActiveWindow(&progress);  progress.setLabelText( QString(text) );  progress.repaint();}
#define PROGRESS_PERCENT(i)	  { QApplication::setActiveWindow(&progress);  progress.setValue(i);  progress.repaint(); \
								QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);} //exclude user input is more safe
//It is impossible to create progress bar in new thread because widget classes can not be used in thread.
//The only solution is to put work in new thread and feed back progress bar in main thread.


/////////////////////Qt Tips collected by Zongcai Ruan///////////////////////////////////////////////////////////////////////////////
//
//____Event, Signal, Timer and Widget in QThread
//Each QThread can have its own event loop. You can start the event loop by calling exec(); you can stop it by calling exit() or quit().
//Having an event loop in a thread makes it possible to connect signals from other threads to slots in this thread, using a mechanism called queued connections.
//It also makes it possible to use classes that require the event loop, such as QTimer and QTcpSocket, in the thread.
//Note: However, that it is not possible to use any widget classes in the thread.
//
//____Child-Parent relationship with Layout
//When you use a layout, you do not need to pass a parent when constructing the child widgets.
//The layout will automatically re-parent the widgets (using QWidget::setParent()) so that they are children of the widget on which the layout is installed.
//You can nest layouts using addLayout() on a layout; the inner layout then becomes a child of the layout it is inserted into.
//Note: Widgets in a layout are children of the widget on which the layout is installed, not of the layout itself.
//      Widgets can only have other widgets as parent, not layouts.
//
//____Object Trees
//QObjects organize themselves in object trees. When you create a QObject with another object as parent, it's added to the parent's children() list,
//and is deleted when the parent is. It turns out that this approach fits the needs of GUI objects very well.
//You can also delete child objects yourself, and they will remove themselves from their parents.
//The debugging functions QObject::dumpObjectTree() and QObject::dumpObjectInfo() are often useful when an application looks or acts strangely.
//Note: When QObjects are created on the stack, the C++ language standard (ISO/IEC 14882:2003) specifies that
//     destructors of local objects are called in the reverse order of their constructors.
//
//____�delete?vs deleteLater()
//QObject supports being deleted while signaling. In order to take advantage of it
//you just have to be sure your object does not try to access any of its own members after being deleted.
//However, most Qt objects are not written this way, and there is no requirement for them to be either.
//For this reason, it is recommended that you always call deleteLater() if you need to delete an object during one of its signals,
//because odds are that �delete?will just crash the application.
//That is, it is not always obvious that a code path has a signal source. Often, you might have a block of code that uses �delete?on some objects that is safe today,
//but at some point in the future this same block of code ends up getting invoked from a signal source and now suddenly your application is crashing.
//The only general solution to this problem is to use deleteLater() all the time, even if at a glance it seems unnecessary.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// 090424RZC: because QColorDialog::getColor cannot handle correctly when user clicks Cancel
inline bool v3dr_getColorDialog( QColor *color, QWidget *parent=0)
{
	QRgb input = 0xff000000;
	if (color)	input = color->rgba();
	bool ok;
	QRgb ouput = QColorDialog::getRgba(input, &ok, parent);  //also include alpha channel
	if (ok && color)  *color = QColor::fromRgba( ouput );
	return ok;
}

inline QColor QColorFromRGBA8(RGBA8 c)
{
	return QColor(c.c[0], c.c[1], c.c[2], c.c[3]);
}

inline RGBA8 RGBA8FromQColor(QColor qc)
{
	RGBA8 c;
	c.r=qc.red(); c.g=qc.green(); c.b=qc.blue(); c.a=qc.alpha();
	return c;
}

#define QCOLOR(rgba8)   QColorFromRGBA8( rgba8 )
#define VCOLOR(rgba8)   qVariantFromValue(QColorFromRGBA8( rgba8 ))

#if defined(USE_Qt5_VS2015_Win7_81) || defined(USE_Qt5_VS2015_Win10_10_14393)
#define QCOLORV(var)    (var.value<QColor>( ))
#define RGBA8V(var)     RGBA8FromQColor(var.value<QColor>( ))
#else
#define QCOLORV(var)    (qVariantValue<QColor>( var ))
#define RGBA8V(var)     RGBA8FromQColor(qVariantValue<QColor>( var ))
#endif

//it's global factory, so use table->setEditTriggers(QAbstractItemView::NoEditTriggers) for local table
#define TURNOFF_ITEM_EDITOR()  QItemEditorFactory::setDefaultFactory(new QItemEditorFactory())


//===================================================================================================================
#ifndef test_main_cpp

#include "../v3d/v3d_core.h"
#include "../v3d/mainwindow.h"

inline
My4DImage* v3dr_getImage4d(void* idep)
{
	My4DImage* image4d = 0L;
	if (idep)
	{
		image4d = ( ((iDrawExternalParameter*)idep)->image4d );
	}
	return (image4d); // && image4d->valid())? image4d : 0L;
}
inline
XFormWidget* v3dr_getXWidget(void* idep)
{
	My4DImage* image4d = 0L;
	XFormWidget* xwidget = 0L;
	if (idep)
	{
		image4d = ( ((iDrawExternalParameter*)idep)->image4d );
		xwidget = ( ((iDrawExternalParameter*)idep)->xwidget );
	}
	return (xwidget && (xwidget->getImageData()==image4d))? xwidget : 0L;
}
inline
MainWindow* v3dr_getV3Dmainwindow(void* idep)
{
	MainWindow* window = 0L;
	if (idep)
	{
		window = ( ((iDrawExternalParameter*)idep)->V3Dmainwindow );
	}
	return window;
}
inline
QList<V3dR_MainWindow*>* v3dr_getV3Dview_plist(void* idep)
{
	QList<V3dR_MainWindow*>* plist = 0L;
	if (idep)
	{
		plist = ( ((iDrawExternalParameter*)idep)->p_list_3Dview_win );
	}
	return plist;
}

#else

#define iDrawExternalParameter char
#define My4DImage char
#define MainWindow char
#define v3d_msg(a,b) qDebug()<<a

#endif


#endif /* V3DR_COMMON_H_ */
