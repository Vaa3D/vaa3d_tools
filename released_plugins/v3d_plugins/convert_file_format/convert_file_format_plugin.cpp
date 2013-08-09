/* convert_file_format_plugin.cpp
 * Convert a few file formats, e.g. v3draw to tif and vice versa.
 * 2012-2013 : by Hanchuan Peng
 */
 
#include "v3d_message.h"
#include <vector>
#include "convert_file_format_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(convert_file_format, ConvertFileFormatPlugin);
 
QStringList ConvertFileFormatPlugin::menulist() const
{
	return QStringList() 
		<<tr("about");
}

QStringList ConvertFileFormatPlugin::funclist() const
{
	return QStringList()
		<<tr("convert_format")
		<<tr("help");
}

void ConvertFileFormatPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("about"))
	{
        v3d_msg(tr("Convert a few file formats, e.g. v3draw to tif and vice versa. This plugin is most useful for command line use."
			"Developed by Hanchuan Peng, 2012-2013"));
	}
}

bool ConvertFileFormatPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("convert_format"))
	{
        if (infiles.size()<1 || output.size()<1 )
        {
            v3d_msg("No input image or output image has been specified.");
            return false;
        }

        Image4DSimple inimg, outimg;

        inimg.loadImage(infiles.at(0));
        if (!inimg.valid())
        {
            v3d_msg("Fail to load the specified input image.", 0);
            return false;
        }

        if (!outimg.saveImage(outfiles.at(0)))
            return false;
    }
	else if (func_name == tr("help"))
	{
        v3d_msg("This plugin converts file formats from oen to another, based on the extension of the file names provided.", 0);
        v3d_msg("Usage: v3d -x dll_name -f convert_format -i input_image_file -o output_image_file", 0);
    }
	else return false;

	return true;
}

