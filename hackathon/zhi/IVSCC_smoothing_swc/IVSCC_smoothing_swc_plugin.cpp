/* IVSCC_smoothing_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-11-20 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "IVSCC_smoothing_swc_plugin.h"
#include "../eliminate_neuron_swc/my_surf_objs.h"
#include "../neurontracing_mip/smooth_curve.h"
#include "../../../released_plugins/v3d_plugins/neuron_radius/hierarchy_prune.h"


using namespace std;
Q_EXPORT_PLUGIN2(IVSCC_smoothing_swc, IVSCC_smoothing_swc);
 
QStringList IVSCC_smoothing_swc::menulist() const
{
	return QStringList() 
        <<tr("smooth_swc")
		<<tr("about");
}

QStringList IVSCC_smoothing_swc::funclist() const
{
	return QStringList()
        <<tr("smooth_swc")
		<<tr("help");
}

void IVSCC_smoothing_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("smooth_swc"))
	{
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(fileOpenName.isEmpty())
            return;
        double length = 0;
        vector<MyMarker*> inswc;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
            bool ok;
            inswc = readSWC_file(fileOpenName.toStdString());

            length = QInputDialog::getDouble(parent, "Please specify the smooth step size","step size:",1,0,2147483647,0.1,&ok);
            if (!ok)
                return;
        }

        unsigned char* inimg1d = 0;
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
            smooth_curve_Z(seg_markers, length);
        }
        inswc.clear();

        QString outswc_file = fileOpenName + QString("_Z_T%1.swc").arg(length);
        topo_segs2swc(topo_segs, inswc, 0);

        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                outswc_file,
                QObject::tr("Supported file (*.swc)"
                    ";;Neuron structure	(*.swc)"
                    ));
        saveSWC_file(fileSaveName.toStdString(), inswc);
        for(int i = 0; i < inswc.size(); i++) delete inswc[i];
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2015-11-20"));
	}
}

bool IVSCC_smoothing_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("smooth_swc"))
    {
        cout<<"Welcome to resample_swc"<<endl;
        vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
        vector<char*>* outlist = NULL;
        vector<char*>* paralist = NULL;

        if(input.size() != 2)
        {
            printf("Please specify both input file and step length parameter.\n");
            return false;
        }
        paralist = (vector<char*>*)(input.at(1).p);
        if (paralist->size()!=1)
        {
            printf("Please specify only two parameters.\n");
            return false;
        }
        double length = atof(paralist->at(0));

        QString fileOpenName = QString(inlist->at(0));
        QString fileSaveName;
        if (output.size()==0)
        {
            printf("No outputfile specified.\n");
            fileSaveName = fileOpenName + QString("_Z_T%1.swc").arg(length);
        }
        else if (output.size()==1)
        {
            outlist = (vector<char*>*)(output.at(0).p);
            fileSaveName = QString(outlist->at(0));
        }
        else
        {
            printf("You have specified more than 1 output file.\n");
            return false;
        }

        vector<MyMarker*> inswc;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
            inswc = readSWC_file(fileOpenName.toStdString());

        unsigned char* inimg1d = 0;
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
            smooth_curve_Z(seg_markers, length);
           // smooth_curve_XY(seg_markers, length);
        }
        inswc.clear();

        topo_segs2swc(topo_segs, inswc, 0);
        saveSWC_file(fileSaveName.toStdString(), inswc);
        for(int i = 0; i < inswc.size(); i++) delete inswc[i];

        return true;
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

