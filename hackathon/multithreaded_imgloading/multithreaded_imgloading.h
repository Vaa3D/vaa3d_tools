// multithreaded_imgloading.h
// Nov. 18, 2010. YuY


#ifndef __MULTITHREADED_IMGLOADING_H__
#define __MULTITHREADED_IMGLOADING_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>

#include "v3d_interface.h"

class MTIMGLOADPlugin : public QObject, public V3DPluginInterface2
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				V3DPluginCallback2 & v3d, QWidget * parent) {return true;}
	float getPluginVersion() const {return 1.23f;} // version info 
};



#endif

