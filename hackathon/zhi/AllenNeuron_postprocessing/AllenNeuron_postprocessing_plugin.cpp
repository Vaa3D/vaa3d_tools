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
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"

#define FNUM 22

using namespace std;
Q_EXPORT_PLUGIN2(AllenNeuron_postprocessing, AllenNeuron_postprocessing);

QStringList importSeriesFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.tif"<<"*.raw"<<"*.v3draw"<<"*.lsm"
            <<"*.TIF"<<"*.RAW"<<"*.V3DRAW"<<"*.LSM";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}
 
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

        int p = 0;
        if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Smooth the swc file?"), QMessageBox::Yes, QMessageBox::No))    p = 1;

        bool ok;
        int length;
        length = QInputDialog::getInteger(parent, "Please specify the length for pruning","length:",5,0,256,1,&ok);
        if (!ok)
            return;


        NeuronTree nt = readSWC_file(fileOpenName);
        NeuronTree nt_sort = SortSWC_pipeline(nt.listNeuron,VOID, 0);
        NeuronTree nt_sort_rs = resample(nt_sort, 10);
        NeuronTree nt_sort_rs_sort = SortSWC_pipeline(nt_sort_rs.listNeuron,VOID,0);
        NeuronTree nt_sort_rs_sort_prune = pruneswc(nt_sort_rs_sort,length);
        NeuronTree nt_sort_rs_sort_prune_sort = SortSWC_pipeline(nt_sort_rs_sort_prune.listNeuron,VOID,0);

        QString fileTmpName = fileOpenName+QString("_tmp.swc");
        export_list2file(nt_sort_rs_sort_prune_sort.listNeuron,fileTmpName,fileOpenName);

        if(p)
        {
            vector<MyMarker*> inswc;
            inswc = readSWC_file(fileTmpName.toStdString());
            smoothswc(inswc,10);
            saveSWC_file(fileTmpName.toStdString(), inswc);

            NeuronTree nt_smooth = readSWC_file(fileTmpName);
            NeuronTree nt_smooth_sort = SortSWC_pipeline(nt_smooth.listNeuron,VOID, 0);
            export_list2file(nt_smooth_sort.listNeuron,fileTmpName,fileOpenName);

            vector<MyMarker*> inswc_smooth_sort = readSWC_file(fileTmpName.toStdString());
            vector<MyMarker*> outswc_interprune = internodeprune(inswc_smooth_sort, nt_smooth_sort);
            saveSWC_file(fileTmpName.toStdString(), outswc_interprune);
        }else
        {
            vector<MyMarker*> inswc_nosmooth_sort = readSWC_file(fileTmpName.toStdString());
            vector<MyMarker*> outswc_interprune = internodeprune(inswc_nosmooth_sort, nt_sort_rs_sort_prune_sort);
            saveSWC_file(fileTmpName.toStdString(), outswc_interprune);
        }


        NeuronTree nt_inter= readSWC_file(fileTmpName);
        NeuronTree nt_inter_sort = SortSWC_pipeline(nt_inter.listNeuron,VOID, 0);

        remove(fileTmpName.toStdString().c_str());


        QString fileDefaultName;
        if(p)
            fileDefaultName = fileOpenName+QString("_part1.swc");
        else
            fileDefaultName = fileOpenName+QString("_part1_nosmooth.swc");

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
        v3dhandle curwin = callback.currentImageWindow();
        string inswc_file;
        bool is_2d;
        double bkg_thresh, stop_thresh;
        V3DLONG  in_sz[4];
        unsigned char* inimg1d = 0;
        int p = 0,length;
        bool ok;

        if(curwin)
        {
            radiusEstimationDialog dialog(callback, parent);
            if (!dialog.image)
                return;
            if (dialog.exec()!=QDialog::Accepted)
                return;

            if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Smooth the swc file?"), QMessageBox::Yes, QMessageBox::No))    p = 1;
            length = QInputDialog::getInteger(parent, "Please specify the length for pruning","length:",5,0,256,1,&ok);
            if (!ok)
                return;

            inswc_file = dialog.inswc_file;
            is_2d = dialog.is_2d;
            bkg_thresh = dialog.bkg_thresh;
            stop_thresh = dialog.stop_thresh/100;
            Image4DSimple* p4DImage = dialog.image;
            inimg1d = p4DImage->getRawData();
            in_sz[0] = p4DImage->getXDim();
            in_sz[1] = p4DImage->getYDim();
            in_sz[2] = p4DImage->getZDim();
            in_sz[3] = p4DImage->getCDim();
        }else
        {
            radiusEstimationFolderDialog dialog(callback, parent);
            if (dialog.exec()!=QDialog::Accepted)
                return;
            if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Smooth the swc file?"), QMessageBox::Yes, QMessageBox::No))    p = 1;
            length = QInputDialog::getInteger(parent, "Please specify the length for pruning","length:",5,0,256,1,&ok);
            if (!ok)
                return;

            inswc_file = dialog.inswc_file;
            inswc_file = dialog.inswc_file;
            is_2d = dialog.is_2d;
            bkg_thresh = dialog.bkg_thresh;
            stop_thresh = dialog.stop_thresh/100;

            QString m_InputfolderName(dialog.image_folder.c_str());
            QStringList imgList = importSeriesFileList_addnumbersort(m_InputfolderName);

            Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;
            V3DLONG count=0;
            foreach (QString img_str, imgList)
            {
                V3DLONG offset[3];
                offset[0]=0; offset[1]=0; offset[2]=0;
                indexed_t<V3DLONG, REAL> idx_t(offset);
                idx_t.n = count;
                idx_t.ref_n = 0; // init with default values
                idx_t.fn_image = img_str.toStdString();
                idx_t.score = 0;
                vim.tilesList.push_back(idx_t);
                count++;
            }
            int NTILES  = vim.tilesList.size();
            unsigned char * data1d_1st = 0;
            V3DLONG sz[4];
            int datatype;
            if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(0).fn_image.c_str()), data1d_1st, sz, datatype))
            {
                fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(0).fn_image.c_str());
                return;
            }
            if(data1d_1st) {delete []data1d_1st; data1d_1st = 0;}

            in_sz[0] = sz[0];
            in_sz[1] = sz[1];
            in_sz[2] = NTILES;
            in_sz[3] = 1;//sz[3];

        //    unsigned char *inimg1d = 0;
            V3DLONG pagesz = in_sz[0]* in_sz[1]* in_sz[2]*in_sz[3];
            try {inimg1d = new unsigned char [pagesz];}
            catch(...)  {v3d_msg("cannot allocate memory for inimg1d."); return;}

            V3DLONG pagesz_one = sz[0]*sz[1]*sz[2]*1;//sz[3];

            V3DLONG i = 0;
            for(V3DLONG ii = 0; ii < NTILES; ii++)
            {
                unsigned char * data1d = 0;
                V3DLONG sz[4];
                int datatype;

                if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(ii).fn_image.c_str()), data1d, sz, datatype))
                {
                    fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                    return;
                }

                if(1)
                {
                    V3DLONG hsz1=floor((double)(sz[1]-1)/2.0); if (hsz1*2<sz[1]-1) hsz1+=1;

                    for (int j=0;j<hsz1;j++)
                        for (int i=0;i<sz[0];i++)
                        {
                            unsigned char tmpv = data1d[(sz[1]-j-1)*sz[0] + i];
                            data1d[(sz[1]-j-1)*sz[0] + i] = data1d[j*sz[0] + i];
                            data1d[j*sz[0] + i] = tmpv;
                        }
                }

                for(V3DLONG j = 0; j < pagesz_one; j++)
                {
                     inimg1d[i] = data1d[j];
                     i++;
                }
                if(data1d) {delete []data1d; data1d = 0;}
            }
        }

       // string outswc_file = dialog.outswc_file;

        QString fileOpenName =  QString::fromStdString(inswc_file);

        NeuronTree nt = readSWC_file(fileOpenName);
        NeuronTree nt_sort = SortSWC_pipeline(nt.listNeuron,VOID, 0);
        NeuronTree nt_sort_rs = resample(nt_sort, 10);
        NeuronTree nt_sort_rs_sort = SortSWC_pipeline(nt_sort_rs.listNeuron,VOID,0);
        NeuronTree nt_sort_rs_sort_prune = pruneswc(nt_sort_rs_sort,length);
        NeuronTree nt_sort_rs_sort_prune_sort = SortSWC_pipeline(nt_sort_rs_sort_prune.listNeuron,VOID,0);

        QString fileTmpName = fileOpenName+QString("_tmp.swc");
        export_list2file(nt_sort_rs_sort_prune_sort.listNeuron,fileTmpName,fileOpenName);

        vector<MyMarker*> inswc_nosmooth_sort = readSWC_file(fileTmpName.toStdString());
        vector<MyMarker*> outswc_interprune = internodeprune(inswc_nosmooth_sort, nt_sort_rs_sort_prune_sort);
        saveSWC_file(fileTmpName.toStdString(), outswc_interprune);

        NeuronTree nt_inter= readSWC_file(fileTmpName);
        NeuronTree nt_inter_sort = SortSWC_pipeline(nt_inter.listNeuron,VOID, 0);
        export_list2file(nt_inter_sort.listNeuron,fileTmpName,fileOpenName);


        vector<MyMarker*> inswc = readSWC_file(fileTmpName.toStdString());

        //radius estimation start
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
        NeuronTree nt_radius_sort = SortSWC_pipeline(nt_radius.listNeuron,VOID, 0);

        if(p)
        {
            export_list2file(nt_radius_sort.listNeuron,fileTmpName,fileOpenName);
            vector<MyMarker*> inswc_b4smooth;
            inswc_b4smooth = readSWC_file(fileTmpName.toStdString());
            smoothswc(inswc_b4smooth,10);
            saveSWC_file(fileTmpName.toStdString(), inswc_b4smooth);

            NeuronTree nt_smooth = readSWC_file(fileTmpName);
            NeuronTree nt_smooth_sort = SortSWC_pipeline(nt_smooth.listNeuron,VOID, 0);
            nt_radius_sort = nt_smooth_sort;
        }

        remove(fileTmpName.toStdString().c_str());

      //  export_list2file(nt_inter_sort.listNeuron,fileTmpName,fileOpenName);

        QString fileDefaultName;
        if(p)
           fileDefaultName = fileOpenName+QString("_processed.swc");
        else
            fileDefaultName = fileOpenName+QString("_processed_nosmooth.swc");

        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                fileDefaultName,
                QObject::tr("Supported file (*.swc)"
                    ";;Neuron structure	(*.swc)"
                    ));
        if (fileSaveName.isEmpty())
            return;
        if (!export_list2file(nt_radius_sort.listNeuron,fileSaveName,fileOpenName))
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

