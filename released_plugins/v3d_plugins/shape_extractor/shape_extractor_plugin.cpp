/* shape_extractor_plugin.cpp
 * A tool to extract cell shapes
 * 2015-2-17 : by Yujie Li
 */
 
#include "v3d_message.h"
#include <vector>
#include "shape_extractor_plugin.h"
#include "shape_dialog.h"


using namespace std;
Q_EXPORT_PLUGIN2(shape_extractor, shape_extr_plugin);
static shape_dialog *dialog=0;

QStringList shape_extr_plugin::menulist() const
{
	return QStringList() 
        <<tr("shape_extractor")
		<<tr("about");
}

QStringList shape_extr_plugin::funclist() const
{
	return QStringList()
        <<tr("shape_extractor")
        //<<tr("shape_extractor_with_mean_shifted_markers")
		<<tr("help");
}

void shape_extr_plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("shape_extractor"))
	{
        dialog=new shape_dialog(&callback);
        //dialog->setWindowTitle("shape extractor");
        dialog->core();
	}

	else
	{
		v3d_msg(tr("A tool to extract cell shapes. "
			"Developed by Yujie Li, 2015-2-17"));
	}
}

bool shape_extr_plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("shape_extractor"))
	{
		v3d_msg("To be implemented.");
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

//void shape_extr_plugin::shape_extr_fun(V3DPluginCallback2 & callback, const V3DPluginArgList & input,
//                                      V3DPluginArgList & output)
//{
//   vector<char*> infiles, inparas, outfiles;
//   if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
//   if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
//   if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

//    if ((infiles.size()!=2))
//    {
//        qDebug()<<"ERROR: please set input and output! "<<infiles.size()<<":"<<outfiles.size();
//        return;
//    }
//    if (inparas.size() != 0 && inparas.size() != 1)
//    {
//        qDebug()<<"ERROR: please give the parameter of background threshold (0-255) or use the default value of 70! "
//               <<inparas.size();
//        return;
//    }

//    QString qs_input_image(infiles[0]);
//    QString qs_input_mask(infiles[1]);
//    image_data=0;
//    int intype=0;

//    if (!qs_input_image.isEmpty())
//    {
//        if (!simple_loadimage_wrapper(callback, qs_input_image.toStdString().c_str(), image_data, sz_img, intype))
//        {
//            qDebug()<<"Loading error";
//            return;
//        }
//        if (sz_img[3]>3)
//        {
//            sz_img[3]=3;
//            v3d_msg("More than 3 channels were loaded."
//                                     " The first 3 channel will be applied for analysis.",0);
//            return;
//        }

//        V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
//        if(intype!=1)
//        {
//            if (intype == 2) //V3D_UINT16;
//            {
//                convert2UINT8((unsigned short*)image_data, image_data, size_tmp);
//            }
//            else if(intype == 4) //V3D_FLOAT32;
//            {
//                convert2UINT8((float*)image_data, image_data, size_tmp);
//            }
//            else
//            {
//                v3d_msg("Currently this program only supports UINT8, UINT16, and FLOAT32 data type.",0);
//                return;
//            }
//        }
//    }


//    if (qs_input_mask.isEmpty())
//        return;
//    FILE * fp = fopen(qs_input_mask.toAscii(), "r");
//    if (!fp)
//    {
//        qDebug()<<"Can not open the file to load the landmark points.\n";
//        return;
//    }
//    else
//    {
//        fclose(fp); //since I will open the file and close it in the function below, thus close it now
//    }

//    QList <LocationSimple> tmpList = readPosFile_usingMarkerCode(qs_input_mask.toAscii()); //revised on 090725 to use the unique interface

//    if (tmpList.count()<=0)
//    {
//        v3d_msg("Did not find any valid row/record of the markers. Thus do not overwrite the current landmarks if they exist.\n",0);
//        return;
//    }
//    qDebug()<<"tmpList size:"<<tmpList.count();
//}
