/* DemonsRegistration.h
 * 2010-06-03: create this program by Luis Ibanez
 */

#ifndef __DEMONSREGISTRATION_H__
#define __DEMONSREGISTRATION_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class ITKDemonsRegistrationPlugin: public QObject, public V3DPluginInterface
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

