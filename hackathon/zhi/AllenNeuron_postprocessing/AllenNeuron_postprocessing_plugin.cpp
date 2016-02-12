/* IVSCC_super_plugin_plugin.cpp
 * 
 * 2016-2-3 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "AllenNeuron_postprocessing_plugin.h"
#include <iostream>
//#include <boost/lexical_cast.hpp>
#include "../../../released_plugins/v3d_plugins/resample_swc/resampling.h"
#include "sort_swc_IVSCC.h"
#include "../IVSCC_radius_estimation/my_surf_objs.h"
#include "../IVSCC_radius_estimation/marker_radius.h"
#include "../IVSCC_radius_estimation/smooth_curve.h"
#include "../IVSCC_radius_estimation/hierarchy_prune.h"
#include "../../../released_plugins/v3d_plugins/global_neuron_feature/compute.h"

#define FNUM 22

using namespace std;
Q_EXPORT_PLUGIN2(AllenNeuron_postprocessing, AllenNeuron_postprocessing);
 
QStringList AllenNeuron_postprocessing::menulist() const
{
	return QStringList() 
        <<tr("whole_process")
        <<tr("part1")
		<<tr("about");
}

QStringList AllenNeuron_postprocessing::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void AllenNeuron_postprocessing::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("part1"))
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

        NeuronTree nt = readSWC_file(fileOpenName);
        NeuronTree nt_sort = SortSWC(nt.listNeuron,VOID, 0);
        NeuronTree nt_sort_rs = resample(nt_sort, 10);
        NeuronTree nt_sort_rs_sort = SortSWC(nt_sort_rs.listNeuron,VOID,0);
        NeuronTree nt_sort_rs_sort_prune = pruneswc(nt_sort_rs_sort,5);
        NeuronTree nt_sort_rs_sort_prune_sort = SortSWC(nt_sort_rs_sort_prune.listNeuron,VOID,0);

        QString fileTmpName = fileOpenName+QString("_tmp.swc");
        export_list2file(nt_sort_rs_sort_prune_sort.listNeuron,fileTmpName,fileOpenName);

        vector<MyMarker*> inswc;
        inswc = readSWC_file(fileTmpName.toStdString());
        smoothswc(inswc,10);
        saveSWC_file(fileTmpName.toStdString(), inswc);

        NeuronTree nt_smooth = readSWC_file(fileTmpName);
        NeuronTree nt_smooth_sort = SortSWC(nt_smooth.listNeuron,VOID, 0);
        export_list2file(nt_smooth_sort.listNeuron,fileTmpName,fileOpenName);

        vector<MyMarker*> inswc_smooth_sort = readSWC_file(fileTmpName.toStdString());
        vector<MyMarker*> outswc_interprune = internodeprune(inswc_smooth_sort, nt_smooth_sort);
        saveSWC_file(fileTmpName.toStdString(), outswc_interprune);

        NeuronTree nt_inter= readSWC_file(fileTmpName);
        NeuronTree nt_inter_sort = SortSWC(nt_inter.listNeuron,VOID, 0);

        remove(fileTmpName.toStdString().c_str());


        QString fileDefaultName = fileOpenName+QString("_part1.swc");
        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                fileDefaultName,
                QObject::tr("Supported file (*.swc)"
                    ";;Neuron structure	(*.swc)"
                    ));
        if (fileSaveName.isEmpty())
            return;
        if (!export_list2file(nt_inter_sort.listNeuron,fileSaveName,fileOpenName))
        {
            v3d_msg("fail to write the output swc file.");
            return;
        }

	}
    else if (menu_name == tr("whole_process"))
	{
        radiusEstimationDialog dialog(callback, parent);
        if (!dialog.image)
            return;
        if (dialog.exec()!=QDialog::Accepted)
            return;

        string inswc_file = dialog.inswc_file;
        string outswc_file = dialog.outswc_file;

        QString fileOpenName =  QString::fromStdString(inswc_file);

        NeuronTree nt = readSWC_file(fileOpenName);
        NeuronTree nt_sort = SortSWC(nt.listNeuron,VOID, 0);
        NeuronTree nt_sort_rs = resample(nt_sort, 10);
        NeuronTree nt_sort_rs_sort = SortSWC(nt_sort_rs.listNeuron,VOID,0);
        NeuronTree nt_sort_rs_sort_prune = pruneswc(nt_sort_rs_sort,5);
        NeuronTree nt_sort_rs_sort_prune_sort = SortSWC(nt_sort_rs_sort_prune.listNeuron,VOID,0);

        QString fileTmpName = fileOpenName+QString("_tmp.swc");
        export_list2file(nt_sort_rs_sort_prune_sort.listNeuron,fileTmpName,fileOpenName);

        vector<MyMarker*> inswc_b4smooth;
        inswc_b4smooth = readSWC_file(fileTmpName.toStdString());
        smoothswc(inswc_b4smooth,10);
        saveSWC_file(fileTmpName.toStdString(), inswc_b4smooth);

        NeuronTree nt_smooth = readSWC_file(fileTmpName);
        NeuronTree nt_smooth_sort = SortSWC(nt_smooth.listNeuron,VOID, 0);
        export_list2file(nt_smooth_sort.listNeuron,fileTmpName,fileOpenName);

        vector<MyMarker*> inswc = readSWC_file(fileTmpName.toStdString());

        //radius estimation start
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
            smooth_curve_and_radius_Zonly(seg_markers, 5);
        }
        inswc.clear();
        topo_segs2swc(topo_segs, inswc, 0); // no resampling
        saveSWC_file(fileTmpName.toStdString(), inswc);
        for(int i = 0; i < inswc.size(); i++) delete inswc[i];

        //radius estimation end

        NeuronTree nt_radius= readSWC_file(fileTmpName);
        NeuronTree nt_radius_sort = SortSWC(nt_radius.listNeuron,VOID, 0);
        export_list2file(nt_radius_sort.listNeuron,fileTmpName,fileOpenName);

        vector<MyMarker*> inswc_radius_sort = readSWC_file(fileTmpName.toStdString());
        vector<MyMarker*> outswc_interprune = internodeprune(inswc_radius_sort, nt_radius_sort);
        saveSWC_file(fileTmpName.toStdString(), outswc_interprune);

        NeuronTree nt_inter= readSWC_file(fileTmpName);
        NeuronTree nt_inter_sort = SortSWC(nt_inter.listNeuron,VOID, 0);

        remove(fileTmpName.toStdString().c_str());

      //  export_list2file(nt_inter_sort.listNeuron,fileTmpName,fileOpenName);

        QString fileDefaultName = fileOpenName+QString("_processed.swc");

        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                fileDefaultName,
                QObject::tr("Supported file (*.swc)"
                    ";;Neuron structure	(*.swc)"
                    ));
        if (fileSaveName.isEmpty())
            return;
        if (!export_list2file(nt_inter_sort.listNeuron,fileSaveName,fileOpenName))
        {
            v3d_msg("fail to write the output swc file.");
            return;
        }

        analysis_swc(fileSaveName ,1);

        NeuronTree nt_final = readSWC_file(fileSaveName);
        QList<NeuronSWC> list = nt_final.listNeuron;
        for (V3DLONG i=0;i<list.size();i++)
        {
            if(i>0 && nt_final.listNeuron[i].pn < 0)
            {
                nt_final.listNeuron.erase(nt_final.listNeuron.begin()+i,nt_final.listNeuron.end());
                break;
            }
        }

        double * features = new double[FNUM];
        computeFeature(nt_final,features);
        QMessageBox infoBox;
        infoBox.setText("Global features of the neuron:");
        infoBox.setInformativeText(QString("<pre><font size='4'>"
                    "number of nodes                  : %1<br>"
                    "soma surface                     : %2<br>"
                    "number of stems                  : %3<br>"
                    "number of bifurcations           : %4<br>"
                    "number of branches               : %5<br>"
                    "number of tips                   : %6<br>"
                    "overall width                    : %7<br>"
                    "overall height                   : %8<br>"
                    "overall depth                    : %9<br>"
                    "average diameter                 : %10<br>"
                    "total length                     : %11<br>"
                    "total surface                    : %12<br>"
                    "total volume                     : %13<br>"
                    "max euclidean distance           : %14<br>"
                    "max path distance                : %15<br>"
                    "max branch order                 : %16<br>"
                    "average contraction              : %17<br>"
                    "average fragmentation            : %18<br>"
                    "average parent-daughter ratio    : %19<br>"
                    "average bifurcation angle local  : %20<br>"
                    "average bifurcation angle remote : %21<br>"
                    "Hausdorff dimension              : %22</font></pre>")
                    .arg(features[0])
                    .arg(features[1])
                    .arg(features[2])
                    .arg(features[3])
                    .arg(features[4])
                    .arg(features[5])
                    .arg(features[6])
                    .arg(features[7])
                    .arg(features[8])
                    .arg(features[9])
                    .arg(features[10])
                    .arg(features[11])
                    .arg(features[12])
                    .arg(features[13])
                    .arg(features[14])
                    .arg(features[15])
                    .arg(features[16])
                    .arg(features[17])
                    .arg(features[18])
                    .arg(features[19])
                    .arg(features[20])
                    .arg(features[21]));
        infoBox.exec();

        if (features) {delete []features; features = NULL;}
	}
	else
	{
        v3d_msg(tr("Developed by Zhi Zhou, 2016-2-3"));
	}
}

bool AllenNeuron_postprocessing::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

