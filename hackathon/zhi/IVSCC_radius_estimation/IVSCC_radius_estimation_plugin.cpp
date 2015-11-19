/* IVSCC_radius_estimation_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-11-19 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "IVSCC_radius_estimation_plugin.h"
#include "my_surf_objs.h"
#include "stackutil.h"
#include "marker_radius.h"
#include "smooth_curve.h"
#include "hierarchy_prune.h"

using namespace std;
Q_EXPORT_PLUGIN2(IVSCC_radius_estimation, IVSCC_radius_estimation);
 
QStringList IVSCC_radius_estimation::menulist() const
{
	return QStringList() 
        <<tr("neuron_radius_current_window")
		<<tr("about");
}

QStringList IVSCC_radius_estimation::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}

void IVSCC_radius_estimation::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("neuron_radius_current_window"))
	{
        radiusEstimationDialog dialog(callback, parent);
        if (!dialog.image)
            return;
        if (dialog.exec()!=QDialog::Accepted)
            return;

        string inswc_file = dialog.inswc_file;
        string outswc_file = dialog.outswc_file;
        bool is_2d = dialog.is_2d;
        double bkg_thresh = dialog.bkg_thresh;
        double stop_thresh = dialog.stop_thresh/100;

        V3DLONG  in_sz[4];
        Image4DSimple* p4DImage = dialog.image;
        unsigned char* inimg1d = p4DImage->getRawData();

        in_sz[0] = p4DImage->getXDim();
        in_sz[1] = p4DImage->getYDim();
        in_sz[2] = p4DImage->getZDim();
        in_sz[3] = p4DImage->getCDim();

        vector<MyMarker*> inswc = readSWC_file(inswc_file);
        if(inswc.empty()) return;
        for(int i = 0; i < inswc.size(); i++)
        {
            MyMarker * marker = inswc[i];
            if(marker->parent > 0)
            {
                if(is_2d)
                    marker->radius = markerRadiusXY(inimg1d, in_sz, *marker, bkg_thresh,stop_thresh);
                else
                    marker->radius = markerRadius(inimg1d, in_sz, *marker, bkg_thresh);
            }
        }

        vector<HierarchySegment*> topo_segs;
        swc2topo_segs(inswc, topo_segs, 1, inimg1d, 0, 0, 0);

        cout<<"Smooth the final curve"<<endl;
        for(int i = 0; i < topo_segs.size(); i++)
        {
            HierarchySegment * seg = topo_segs[i];
            MyMarker * leaf_marker = seg->leaf_marker;
            MyMarker * root_marker = seg->root_marker;
            vector<MyMarker*> seg_markers;
            MyMarker * p = leaf_marker;
            while(p != root_marker)
            {
                seg_markers.push_back(p);
                p = p->parent;
            }
            seg_markers.push_back(root_marker);
            smooth_curve_and_radius(seg_markers, 5);
        }
        inswc.clear();
        topo_segs2swc(topo_segs, inswc, 0); // no resampling
        saveSWC_file(outswc_file, inswc);
        QMessageBox::information(0,"", string("neuron radius is calculated successfully. \n\nThe output swc is saved to " +outswc_file).c_str());
        for(int i = 0; i < inswc.size(); i++) delete inswc[i];
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2015-11-19"));
	}
}

bool IVSCC_radius_estimation::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
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

