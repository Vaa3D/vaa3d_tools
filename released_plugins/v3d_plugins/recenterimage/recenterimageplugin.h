/* recenterimageplugin.h
 * 2009-08-14: created by Yang Yu
 * 2010-05-20: by PHC for compatability to VC compiler
 * 2011-09-16: change interface from "V3DSingleImageInterface2_1" to "V3DPluginInterface2_1" by Yang Yu
 */


#ifndef __RECENTERIMAGEPLUGIN_H__
#define __RECENTERIMAGEPLUGIN_H__

//  recenter subject image into the center of target image
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

#if defined (_MSC_VER)  //2010-05-20, by PHC for compatability to VC compiler
#include "vcdiff.h"
#else
#endif

class ReCenterImagePlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1)

public:
    float getPluginVersion() const {return 1.1f;}
    
    QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const;
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & v3d, QWidget * parent);
};

#endif



