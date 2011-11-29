/*
 * neuron_feature.h
 *
 *  Created by Yinan Wan on 07/07/11.
 *  Last change: 
 *
 */

#ifndef __NEURON_FEATURE_H
#define __NEURON_FEATURE_H

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include "v3d_interface.h"
#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"

class NFPlugin: public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);
	
public:
	float getPluginVersion() const {return 1.0f;}
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	QStringList funclist() const ;
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent);
};

void neuron_feature(V3DPluginCallback &callback, QWidget *parent, int method_code);

#endif


