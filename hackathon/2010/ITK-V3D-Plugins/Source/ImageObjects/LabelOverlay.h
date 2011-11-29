/* LabelOverlayPlugin.h
 * 2010-07-30: Plugin created by Aurelien Lucchi
 */

#ifndef __LabelOverlay_H__
#define __LabelOverlay_H__


#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class LabelOverlayPlugin : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface)

public:
	LabelOverlayPlugin() {}
    QStringList menulist() const;
	QStringList funclist() const;

	void domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent);

	virtual void dofunc(const QString & func_name,
			const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent);

 private:
        bool useSingleColor;
};

#endif
