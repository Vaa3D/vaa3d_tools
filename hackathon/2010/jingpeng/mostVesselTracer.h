/* mostVesselTracer.h
 * by jpwu@CBMP,20100419
 */


#ifndef __MOSTVESSELTRACER_H__
#define __MOSTVESSELTRACER_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>

#include "v3d_interface.h"
#include "../../../v3d/v3d_main/v3d/v3d_core.h"
#include "basic_4dimage.h"
#include "v3d_message.h"

#include "mostimage.h"

class mostVesselTracerPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

private:
        // static const float pluginVersion = 1.1; // not in msvc


public:
//	QStringList menulist() const;
//	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
//
//	QStringList funclist() const {return QStringList();}
//	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
//

	QStringList menulist() const;
	void domenu(const QString & menu_name, V3DPluginCallback2 & v3d,  QWidget * parent);

	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent) {return true;}

        // plugin interface version 2.1 requires plugin version
        float getPluginVersion() const {return 1.1f;}


};

void setSeeds(V3DPluginCallback2 &v3d, QWidget *parent);
void startVesselTracing(V3DPluginCallback2 &v3d, QWidget *parent);

// the visited flag
static QVector < bool >  visited;
#endif

