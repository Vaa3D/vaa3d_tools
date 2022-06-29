/* TraceNeuronBasedLine_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2021-12-20 : by ZX
 */
 
#include "v3d_message.h"
#include <vector>
#include "TraceNeuronBasedLine_plugin.h"

#include "dataset.h"


using namespace std;
Q_EXPORT_PLUGIN2(TraceNeuronBasedLine, TraceNeuronBasedLinePlugin);
 
QStringList TraceNeuronBasedLinePlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList TraceNeuronBasedLinePlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TraceNeuronBasedLinePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by ZX, 2021-12-20"));
	}
}

bool TraceNeuronBasedLinePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("getDataSet"))
	{
        QString image_path = infiles.size() >= 1 ? infiles[0] : "";
        QString swc_brain_dir_path = infiles.size() >= 2 ? infiles[1] : "";

        QString out_dir_path = outfiles.size() >= 1 ? outfiles[0] : "";

        int k = 0;

        float center_x = inparas.size() >= k + 1 ? atof(inparas[k]) : 0; k++;
        float center_y = inparas.size() >= k + 1 ? atof(inparas[k]) : 0; k++;
        float center_z = inparas.size() >= k + 1 ? atof(inparas[k]) : 0; k++;
        float resolution = inparas.size() >= k + 1 ? atof(inparas[k]) : 1; k++;
        float l_thres_max = inparas.size() >= k + 1 ? atof(inparas[k]) : 160; k++;
        float l_thres_min = inparas.size() >= k + 1 ? atof(inparas[k]) : 20; k++;
        float t_length = inparas.size() >= k + 1 ? atof(inparas[k]) : 5; k++;
        float soma_ratio = inparas.size() >= k + 1 ? atof(inparas[k]) : 1.2; k++;
        float inflection_d = inparas.size() >= k + 1 ? atof(inparas[k]) : 10; k++;
        float cos_angle_thres = inparas.size() >= k + 1 ? atof(inparas[k]) : 0; k++;
        bool symmetry = inparas.size() >= k + 1 ? atoi(inparas[k]) : false; k++;
        bool save_mid_result = inparas.size() >= k + 1 ? atoi(inparas[k]) : false;

        unsigned char* pdata = 0;
        int data_type = 1;
        V3DLONG sz[4] = {0, 0, 0, 0};
        simple_loadimage_wrapper(callback, image_path.toStdString().c_str(), pdata, sz, data_type);

        qDebug()<<"get All Tree in block";
        vector<NeuronTree> results =  getAllTreesInBlock(image_path, swc_brain_dir_path, out_dir_path, save_mid_result,
                           XYZ(center_x, center_y, center_z), resolution, callback);

        QDir out_dir(out_dir_path);
        QString lines_features_csv_path = out_dir.absoluteFilePath(QFileInfo(image_path).baseName() + "_lines_features.csv");
        QString lines_matrix_csv_path = out_dir.absoluteFilePath(QFileInfo(image_path).baseName() + "_lines_matrix.csv");
        qDebug()<<"lines_features_csv_path: "<<lines_features_csv_path;
        qDebug()<<"lines_matrix_csv_path: "<<lines_matrix_csv_path;

        qDebug()<<"get Lines in block";

        getLinesInBlock(results, lines_features_csv_path, lines_matrix_csv_path,
                        pdata, sz, inflection_d, cos_angle_thres, l_thres_max, l_thres_min, t_length, soma_ratio, symmetry);

	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

