/* neuroncrawler_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-2-16 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuroncrawler_plugin.h"
#include "tracing_func.h"

#include "../../../hackathon/zhi/APP2_large_scale/readRawfile_func.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"


using namespace std;
Q_EXPORT_PLUGIN2(neuroncrawler, neuroncrawler);

 
QStringList neuroncrawler::menulist() const
{
	return QStringList() 
        <<tr("trace_APP2")
        <<tr("trace_APP1")
        <<tr("trace_MOST")
        <<tr("trace_NEUTUBE")
        <<tr("trace_SNAKE")
		<<tr("about");
}

QStringList neuroncrawler::funclist() const
{
	return QStringList()
		<<tr("trace_raw")
		<<tr("trace_tc")
		<<tr("help");
}

void neuroncrawler::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("trace_APP2"))
	{
        TRACE_LS_PARA P;
        bool bmenu = true;
        neuroncrawler_app2_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.is_gsdt = dialog.is_gsdt;
        P.is_break_accept = dialog.is_break_accept;
        P.bkg_thresh = dialog.bkg_thresh;
        P.length_thresh = dialog.length_thresh;
        P.cnn_type = dialog.cnn_type;
        P.channel = dialog.channel;
        P.SR_ratio = dialog.SR_ratio;
        P.b_256cube = dialog.b_256cube;
        P.b_RadiusFrom2D = dialog.b_RadiusFrom2D;
        P.block_size = dialog.block_size;
        P.visible_thresh = 0;
        P.adap_win = dialog.adap_win;
        P.method = 2;
        crawler_raw_app(callback,parent,P,bmenu);
    }else if (menu_name == tr("trace_APP1"))
	{
        TRACE_LS_PARA P;
        bool bmenu = true;
        neuroncrawler_app1_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.bkg_thresh = dialog.bkg_thresh;
        P.channel = dialog.channel;
        P.b_256cube = dialog.b_256cube;
        P.visible_thresh = dialog.visible_thresh;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.method = 1;
        crawler_raw_app(callback,parent,P,bmenu);
	}
    else if (menu_name == tr("trace_MOST"))
    {
        TRACE_LS_PARA P;
        bool bmenu = true;
        neuroncrawler_most_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.bkg_thresh = dialog.bkg_thresh;
        P.channel = dialog.channel;
        P.seed_win = dialog.seed_win;
        P.slip_win = dialog.slip_win;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.method = 4;

        crawler_raw_all(callback,parent,P,bmenu,3);
    }
    else if (menu_name == tr("trace_NEUTUBE"))
    {
        TRACE_LS_PARA P;
        bool bmenu = true;
        neuroncrawler_neutube_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.method = 3;

        crawler_raw_all(callback,parent,P,bmenu,1);
    }else if (menu_name == tr("trace_SNAKE"))
    {
        TRACE_LS_PARA P;
        bool bmenu = true;
        neuroncrawler_neutube_raw dialog(callback, parent);

        if (dialog.image && dialog.listLandmarks.size()==0)
            return;

        if (dialog.exec()!=QDialog::Accepted)
            return;

        if(dialog.rawfilename.isEmpty())
        {
            v3d_msg("Please select the image file.");
            return;
        }

        if(dialog.markerfilename.isEmpty() && ! dialog.image)
        {
            v3d_msg("Please select the marker file.");
            return;
        }

        if(!dialog.image)
        {
            P.markerfilename = dialog.markerfilename;
            P.image = 0;
        }else
        {
            P.image = dialog.image;
            P.listLandmarks = dialog.listLandmarks;
        }
        P.inimg_file = dialog.rawfilename;
        P.block_size = dialog.block_size;
        P.adap_win = dialog.adap_win;
        P.method = 5;


        crawler_raw_all(callback,parent,P,bmenu,2);
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2016-2-16"));
	}
}

bool neuroncrawler::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("trace_raw"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("trace_tc"))
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

