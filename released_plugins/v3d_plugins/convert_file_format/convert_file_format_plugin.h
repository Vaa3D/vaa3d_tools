/* convert_file_format_plugin.h
 * Convert a few file formats, e.g. v3draw to tif and vice versa.
 * 2012-2013 : by Hanchuan Peng
 */
 
#ifndef __CONVERT_FILE_FORMAT_PLUGIN_H__
#define __CONVERT_FILE_FORMAT_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class ConvertFileFormatPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
    float getPluginVersion() const {return 1.2f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

