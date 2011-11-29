#ifndef __V3DITKPluginDefaultHeader_H__
#define __V3DITKPluginDefaultHeader_H__


#include <QtGui>
#include <QObject>
#include <v3d_interface.h>


#define  V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(_plugin_name_) \
public: \
	_plugin_name_##Plugin() {} \
  QStringList menulist() const; \
	QStringList funclist() const; \
\
	void domenu(const QString & menu_name, V3DPluginCallback & callback, QWidget * parent);\
\
	virtual void dofunc(const QString & func_name,\
			const V3DPluginArgList & input, V3DPluginArgList & output, QWidget * parent);



#define  V3DITKPLUGIN_DEFAULT_HEADER(_plugin_name_) \
 \
class _plugin_name_##Plugin : public QObject, public V3DPluginInterface \
{ \
    Q_OBJECT \
    Q_INTERFACES(V3DPluginInterface) \
 \
   V3DITKPLUGIN_DEFAULT_CLASS_DECLARATION_BODY(_plugin_name_); \
};\

#endif
