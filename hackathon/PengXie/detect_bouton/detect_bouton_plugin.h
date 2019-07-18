/* detect_bouton_plugin.h
 * This plugin detects bouton along a neuron reconstruction
 * 2018-11-20 : by Peng Xie
 */
 
#ifndef __DETECT_BOUTON_PLUGIN_H__
#define __DETECT_BOUTON_PLUGIN_H__
#include "v3d_message.h"
#include <vector>
#include <QtGui>
#include <v3d_interface.h>
#include "detect_bouton_main.h"
#include "get_terminal.h"
#include "get_terminal_signal.h"


class detect_bouton_plugin : public QObject, public V3DPluginInterface2_1
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

