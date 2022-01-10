/* istitch.h
 * 2010-02-08: create this program by Yang Yu
 * 2011-07-11: add dofunc function call group stitching by Yang Yu
 */


#ifndef __ISTITCH_H__
#define __ISTITCH_H__

//
#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

//----------------------------------------------------------------------
#ifdef COMPILE_TO_COMMANDLINE
#else

class IStitchPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);
	
public:
    float getPluginVersion() const {return 1.1f;} // version info 
    
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const;
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & v3d, QWidget * parent);
	
};

#endif
//----------------------------------------------------------------------

#endif



