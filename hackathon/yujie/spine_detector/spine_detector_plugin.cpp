/* spine_detector_plugin.cpp
 * This tool detects spine
 * 2015-3-11 : by Yujie Li
 */
 
#include "spine_detector_plugin.h"
#include "app2.h"
#include "manual_correct_dialog.h"


using namespace std;
Q_EXPORT_PLUGIN2(spine_detector, spine_detector);
static spine_detector *dialog=0;

QStringList spine_detector::menulist() const
{
	return QStringList() 
//        <<tr("spine_detector_1")
//        <<tr("skeleton analysis")
        <<tr("spine_detector_1.0 (view by spine)")
        <<tr("spine_detector_1.1 (view by segment)")
        <<tr("handele_big_image")
        <<tr("about");
}

QStringList spine_detector::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void spine_detector::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("spine_detector"))
	{

        spine_detector_dialog *dialog=new spine_detector_dialog(&callback);
        dialog->exec();
	}
    else if (menu_name==tr("spine_detector_1"))
    {
       //QString image_name="C:\\Users\\Jade\\Documents\\V3d\\spine_dec_test_data\\Gaussian_5_5_4.v3draw";
       //QString image_name="C:\\Users\\Jade\\Documents\\V3d\\spine_dec_test_data\\shape_learn\\Test\\Dan_data\\dendrite_segment5_CN3_gausian_4_4_3.v3draw";
       QString image_name="C:\\Users\\Jade\\Documents\\V3d\\spine_dec_test_data\\cd1_coh13-96_100x_cell_d1-1_s5_r1_c4_lh.v3draw";
       QString skel_name="C:\\Users\\Jade\\Documents\\V3d\\spine_dec_test_data\\skeleton.swc_resampled.swc";
       //QString skel_name="C:\\Users\\Jade\\Documents\\V3d\\spine_dec_test_data\\shape_learn\\Test\\Dan_data\\segment5_resampled.swc";
       QString name_out="predict";
       spine_fun * spine_obj=new spine_fun(image_name.toAscii(),skel_name.toAscii()
                              ,name_out.toAscii(),&callback);
       spine_obj->loadData();
       spine_obj->init();
       spine_obj->reverse_dst_grow();
//       spine_obj->run_dstGroup();
       spine_obj->run_intensityGroup();
       spine_obj->conn_comp_nb6();
//       spine_obj->construct_group_profile();
       spine_obj->write_spine_center_profile();
       spine_obj->saveResult();

    }
    else if(menu_name==tr("spine_detector_1.0 (view by spine)"))
    {
        manual_correct_dialog *manual_dialog=new manual_correct_dialog(&callback,0);
        manual_dialog->show();
    }
//    else if (menu_name == tr("skeleton analysis"))
//	{
//        PARA_APP2 p;
//        if(!p.initialize(callback)) //here will initialize the image pointer, bounding box, etc.
//            return;

//        // fetch parameters from dialog
//        if (!p.app2_dialog())
//            return;

//        if (!proc_app2(callback, p, versionStr))
//            return;
//	}
    else if (menu_name==tr("spine_detector_1.1 (view by segment)"))
    {
        manual_correct_dialog *manual_dialog=new manual_correct_dialog(&callback,1);
        manual_dialog->show();
    }
    else if (menu_name==tr("handele_big_image"))
    {
        manual_correct_dialog *manual_dialog=new manual_correct_dialog(&callback);
        manual_dialog->show();
    }
    else
        v3d_msg(tr("This tool detects spines. "
            "Developed by Yujie Li, 2015-3-11"));
}

bool spine_detector::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
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

