/* affine_transform_swc_plugin.cpp
 * This plugin can be used to apply an input affine transform to a single SWC file.
 * 2015-6-25 : by Xiaoxiao liu
 */
 
#include "v3d_message.h"
#include <vector>
#include "affine_transform_swc_plugin.h"
#include "apply_transform_func.h"
using namespace std;
Q_EXPORT_PLUGIN2(affine_transform, AffineTransformSWC);
 
QStringList AffineTransformSWC::menulist() const
{
	return QStringList() 
		<<tr("apply_transform")
		<<tr("about");
}

QStringList AffineTransformSWC::funclist() const
{
	return QStringList()
		<<tr("apply_transform")
		<<tr("help");
}

void AffineTransformSWC::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("apply_transform"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This plugin can be used to apply an input affine transform to a single SWC file.. "
			"Developed by Xiaoxiao liu, 2015-6-25"));
	}
}

bool AffineTransformSWC::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("apply_transform"))
	{
            return apply_transform_to_swc(input, output);
	}
	else if (func_name == tr("help"))
	{
		printf("\n\nThisplugin can be used to apply an input affine transform to a single SWC file. Developed by Xiaoxiao liu, 2015-6-25\n");
		printf("usage:\n");
		printf("\t-f <function_name>:     apply_transform\n");
		printf("\t-i <input_file_name>:   input .swc\n");
		printf("\t-o <output_file_name>:  output.swc file. DEFAULT: 'inputName_transformed.swc'\n");
		printf("\t-p <transform_file_name>: a 3x4 transform matrix stored in a txt file.\n");
                printf("vaa3d -x affine_transform_swc -f apply_transform  -i input.swc  -o transformed.swc -p transform.txt ");
	}
	else return false;

	return true;
}

