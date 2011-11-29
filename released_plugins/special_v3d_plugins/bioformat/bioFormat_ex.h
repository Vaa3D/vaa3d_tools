/* bioFormat_ex.h
 * 2009-09-20: create this program by Yang Yu
 */

#ifndef __BIOFORMAT_EX_H__
#define __BIOFORMAT_EX_H__

//EXAMPLES USING BIOFORMAT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <v3d_interface.h>

class BioFormat_ExPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface)

public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
};

#endif



