#ifndef EXTRAFILTERSPLUGIN_H
#define EXTRAFILTERSPLUGIN_H


#include <v3d_interface.h>
#include <QtGui>
#include <QtCore>
#include <list>
#include <v3d_basicdatatype.h>

class CrashTestPlugin3 : public QObject, public V3DPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface);

public:

    QDialog *myDialog;
    v3dhandle sourceWindow;
    Image4DSimple* sourceImage;
    V3DPluginCallback *myCallback;
    Image4DSimple *sourceImageCopy;

    // Plugin stuff

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent);

	QStringList funclist() const {return QStringList();}
	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}

	void copyImage();
	void restoreCopy();

public slots:
	void restore();
	void processBlackImage();

};

#endif
