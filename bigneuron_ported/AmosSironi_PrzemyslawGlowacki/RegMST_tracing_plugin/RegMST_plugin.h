/* RegMST_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2015-6-23 : by Amos Sironi and Przemyslaw Glowacki
 */
 
#ifndef __REGMST_PLUGIN_H__
#define __REGMST_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

#include "v3d_message.h"
#include <vector>
//#include "SQBTree_plugin.h"
//#include "sqb_trees.h"


#include "../libs/sqb_0.1/src/MatrixSQB/vaa3d_link.h"

#include "../libs/regression/sep_conv.h"
#include "../libs/regression/util.h"
//#include "regression/regression_test2.h"

class RegMST : public QObject, public V3DPluginInterface2_1
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

#endif

