/* SQBTree_plugin.h
 */
 
#ifndef __SQBTREE_PLUGIN_H__
#define __SQBTREE_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

#include "v3d_message.h"
#include <vector>

#include "../libs/sqb_0.1/src/MatrixSQB/vaa3d_link.h"
#include "../libs/regression/sep_conv.h"
#include "../libs/regression/util.h"


class RegressionTubularityACPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};


bool testTubularityImage(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);


#endif

