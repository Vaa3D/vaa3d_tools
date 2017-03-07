/* IVSCC_radius_estimation_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-11-19 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "IVSCC_radius_estimation_plugin.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "my_surf_objs.h"
#include "stackutil.h"
#include "marker_radius.h"
#include "smooth_curve.h"
#include "hierarchy_prune.h"
#include "../IVSCC_sort_swc/openSWCDialog.h"

using namespace std;

struct Point;
struct Point
{
    double x,y,z,r;
    V3DLONG type;
    Point* p;
    V3DLONG childNum;
};
typedef vector<Point*> Segment;
typedef vector<Point*> Tree;

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

template <class T> T pow2(T a)
{
    return a*a;

}

NeuronTree interpolate_radius(NeuronTree input);

void interpolate_path(Segment * seg)
{
    Segment seg_r;
    Point* seg_par = seg->back()->p;
    seg_r.push_back(seg->at(0));
    double start_radius;
    Point* start_node = seg->at(seg->size()-1);
    if(start_node->p->type == 1)
        start_radius = start_node->r;
    else
        start_radius = start_node->p->r;

    for(V3DLONG iter_old = 1;iter_old < seg->size();iter_old++)
    {
        Point* start = seg->at(iter_old);
        Point* pt = new Point;
        pt->x = start->x;
        pt->y = start->y;
        pt->z = start->z;
        pt->r = seg->at(0)->r + iter_old*(start_radius-seg->at(0)->r)/(seg->size()-1);
        pt->p = start->p;
        pt->type = start->type;
        seg_r.back()->p = pt;
        seg_r.push_back(pt);
    }
    seg_r.back()->p = seg_par;
    for (V3DLONG i=0;i<seg->size();i++)
        if (!seg->at(i)) {delete seg->at(i); seg->at(i) = NULL;}
    *seg = seg_r;
};

Q_EXPORT_PLUGIN2(IVSCC_radius_estimation, IVSCC_radius_estimation);
 
QStringList IVSCC_radius_estimation::menulist() const
{
	return QStringList() 
        <<tr("neuron_radius_current_window")
        <<tr("neuron_radius_selected_folder")
        <<tr("neuron_radius_interpolation")
        <<tr("swc_3D_to_2D")
        <<tr("swc_2D_to_3D_radius")
		<<tr("about");
}

QStringList IVSCC_radius_estimation::funclist() const
{
	return QStringList()
        <<tr("neuron_radius")
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
        double stop_thresh = dialog.stop_thresh;
        double winX = dialog.winX_size/2;
        double winY = dialog.winY_size/2;

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
            double root_x = inswc[0]->x;
            double root_y = inswc[0]->y;
            double bkg_thresh_updated;
            if(marker->x > root_x - winX && marker->x < root_x + winX && marker->y > root_y - winY && marker->y < root_y + winY)
                bkg_thresh_updated = bkg_thresh;
            else
                bkg_thresh_updated = stop_thresh;
            if(marker->parent > 0 && marker->type != 2)
            {
                if(is_2d)
                    marker->radius = markerRadius_XY_IVSCC(inimg1d, in_sz, *marker, bkg_thresh_updated,0.001,2.0,6.0);
                else
                    marker->radius = markerRadius(inimg1d, in_sz, *marker, bkg_thresh_updated);
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
        saveSWC_file(outswc_file, inswc);
        QMessageBox::information(0,"", string("neuron radius is calculated successfully. \n\nThe output swc is saved to " +outswc_file).c_str());
        for(int i = 0; i < inswc.size(); i++) delete inswc[i];
    }else if (menu_name == tr("neuron_radius_selected_folder"))
    {
        radiusEstimationFolderDialog dialog(callback, parent);
        if (dialog.exec()!=QDialog::Accepted)
            return;

        string inswc_file = dialog.inswc_file;
        string outswc_file = dialog.outswc_file;
        bool is_2d = dialog.is_2d;
        double bkg_thresh = dialog.bkg_thresh;
        double stop_thresh = dialog.stop_thresh;
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
        V3DLONG in_sz[4];
        int datatype;

        if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(0).fn_image.c_str()), data1d_1st, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(0).fn_image.c_str());
            return;
        }
        if(data1d_1st) {delete []data1d_1st; data1d_1st = 0;}


        V3DLONG sz[4];
        sz[0] = in_sz[0];
        sz[1] = in_sz[1];
        sz[2] = NTILES;
        sz[3] = 1;//in_sz[3];

        unsigned char *im_imported = 0;
        V3DLONG pagesz = sz[0]* sz[1]* sz[2]*sz[3];
        try {im_imported = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for im_imported."); return;}

        V3DLONG pagesz_one = in_sz[0]*in_sz[1]*in_sz[2]*1;//in_sz[3];

        V3DLONG i = 0;
        for(V3DLONG ii = 0; ii < NTILES; ii++)
        {
            unsigned char * data1d = 0;
            V3DLONG in_sz[4];
            int datatype;

            if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(ii).fn_image.c_str()), data1d, in_sz, datatype))
            {
                fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                return;
            }

            if(1)
            {
                V3DLONG hsz1=floor((double)(in_sz[1]-1)/2.0); if (hsz1*2<in_sz[1]-1) hsz1+=1;

                for (int j=0;j<hsz1;j++)
                    for (int i=0;i<in_sz[0];i++)
                    {
                        unsigned char tmpv = data1d[(in_sz[1]-j-1)*in_sz[0] + i];
                        data1d[(in_sz[1]-j-1)*in_sz[0] + i] = data1d[j*in_sz[0] + i];
                        data1d[j*in_sz[0] + i] = tmpv;
                    }
            }

            for(V3DLONG j = 0; j < pagesz_one; j++)
            {
                 im_imported[i] = data1d[j];
                 i++;
            }
            if(data1d) {delete []data1d; data1d = 0;}
        }


        vector<MyMarker*> inswc = readSWC_file(inswc_file);
        if(inswc.empty()) return;
        for(int i = 0; i < inswc.size(); i++)
        {
            MyMarker * marker = inswc[i];
            double root_x = inswc[0]->x;
            double root_y = inswc[0]->y;
            double bkg_thresh_updated;
            if(marker->x > root_x - 500 && marker->x < root_x + 500 && marker->y > root_y - 500 && marker->y < root_y + 500)
                bkg_thresh_updated = bkg_thresh;
            else
                bkg_thresh_updated = stop_thresh;

            if(marker->parent > 0 && marker->type != 2)
            {
                if(is_2d)
                    marker->radius = markerRadiusXY(im_imported, sz, *marker, bkg_thresh_updated,0.001);
                else
                    marker->radius = markerRadius(im_imported, sz, *marker, bkg_thresh_updated);
            }
        }

        vector<HierarchySegment*> topo_segs;
        swc2topo_segs(inswc, topo_segs, 1, im_imported, 0, 0, 0);

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
        saveSWC_file(outswc_file, inswc);
        if(im_imported) {delete []im_imported; im_imported = 0;}
        QMessageBox::information(0,"", string("neuron radius is calculated successfully. \n\nThe output swc is saved to " +outswc_file).c_str());
        for(int i = 0; i < inswc.size(); i++) delete inswc[i];
    }
    else if(menu_name == tr("neuron_radius_interpolation"))
    {
        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;

        QString fileOpenName = openDlg->file_name;

        if(fileOpenName.isEmpty())
            return;
        NeuronTree nt;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
            nt = openDlg->nt;

        }
        NeuronTree nt_result = interpolate_radius(nt);

        QString fileDefaultName = fileOpenName+QString("_interpolated.swc");
        //write new SWC to file
        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                fileDefaultName,
                QObject::tr("Supported file (*.swc)"
                    ";;Neuron structure	(*.swc)"
                    ));
        writeSWC_file(fileSaveName,nt_result);

        QVector<QVector<V3DLONG> > childs;
        V3DLONG neuronNum = nt_result.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = nt_result.listNeuron[i].pn;
            if (par<0) continue;
            childs[nt_result.hashNeuron.value(par)].push_back(i);
        }
        QList<NeuronSWC> list = nt_result.listNeuron;
        QList<ImageMarker> bifur_marker;
        QList<ImageMarker> all_bifur_marker;


        for (int i=0;i<list.size();i++)
        {
            ImageMarker t;
            if ((childs[i].size()>1 || childs[i].size() ==0) && nt_result.listNeuron.at(i).type != 1 && nt_result.listNeuron.at(i).type != 2)
            {
                t.x = nt_result.listNeuron.at(i).x;
                t.y = nt_result.listNeuron.at(i).y;
                t.z = nt_result.listNeuron.at(i).z;
                all_bifur_marker.append(t);
                if(nt_result.listNeuron.at(i).r <= 2)
                {
                    bifur_marker.append(t);
                }
            }
        }

        QString MarkerfileName = fileSaveName+QString(".marker");
        QString AllmarkerfileName = fileSaveName+QString("_all.marker");

        writeMarker_file(MarkerfileName, bifur_marker);
        writeMarker_file(AllmarkerfileName, all_bifur_marker);

    }
    else if(menu_name == tr("swc_3D_to_2D"))
    {
        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;

        QString fileOpenName = openDlg->file_name;

        if(fileOpenName.isEmpty())
            return;
        NeuronTree nt_result;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
            nt_result = openDlg->nt;

        }

        for(V3DLONG i =0; i<nt_result.listNeuron.size();i++)
        {
            nt_result.listNeuron[i].z = 0;
        }

        QString fileDefaultName = fileOpenName+QString("_2D.swc");
        //write new SWC to file
        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                fileDefaultName,
                QObject::tr("Supported file (*.swc)"
                    ";;Neuron structure	(*.swc)"
                    ));
        writeSWC_file(fileSaveName,nt_result);

        QVector<QVector<V3DLONG> > childs;
        V3DLONG neuronNum = nt_result.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = nt_result.listNeuron[i].pn;
            if (par<0) continue;
            childs[nt_result.hashNeuron.value(par)].push_back(i);
        }
        QList<NeuronSWC> list = nt_result.listNeuron;
        QList<ImageMarker> all_bifur_marker;


        for (int i=0;i<list.size();i++)
        {
            ImageMarker t;
            if ((childs[i].size()>1 || childs[i].size() ==0) && nt_result.listNeuron.at(i).type != 1 && nt_result.listNeuron.at(i).type != 2)
            {
                t.x = nt_result.listNeuron.at(i).x;
                t.y = nt_result.listNeuron.at(i).y;
                t.z = 1;
                all_bifur_marker.append(t);
            }
        }

        QString AllmarkerfileName = fileSaveName+QString("_all.marker");
        writeMarker_file(AllmarkerfileName, all_bifur_marker);

    }
    else if(menu_name == tr("swc_2D_to_3D_radius"))
    {
        v3d_msg("Please select the 3D swc file.");
        OpenSWCDialog * openDlg1 = new OpenSWCDialog(0, &callback);
        if (!openDlg1->exec())
            return;

        QString fileOpenName3D = openDlg1->file_name;

        if(fileOpenName3D.isEmpty())
            return;
        NeuronTree nt_3D;
        if (fileOpenName3D.toUpper().endsWith(".SWC") || fileOpenName3D.toUpper().endsWith(".ESWC"))
        {
            nt_3D = openDlg1->nt;
        }

        v3d_msg("Please select the 2D swc file.");
        OpenSWCDialog * openDlg2 = new OpenSWCDialog(0, &callback);
        if (!openDlg2->exec())
            return;

        QString fileOpenName2D = openDlg2->file_name;

        if(fileOpenName2D.isEmpty())
            return;

        NeuronTree nt_2D;
        if (fileOpenName2D.toUpper().endsWith(".SWC") || fileOpenName2D.toUpper().endsWith(".ESWC"))
        {
            nt_2D = openDlg2->nt;
        }

        QVector<QVector<V3DLONG> > childs;
        V3DLONG neuronNum = nt_3D.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = nt_3D.listNeuron[i].pn;
            if (par<0) continue;
            childs[nt_3D.hashNeuron.value(par)].push_back(i);
        }

        QList<NeuronSWC> list = nt_3D.listNeuron;
        QList<NeuronSWC> list2 = nt_2D.listNeuron;
        for (int i=0;i<list.size();i++)
        {
            if ((childs[i].size()>1 || childs[i].size() ==0) && nt_3D.listNeuron.at(i).type != 1 && nt_3D.listNeuron.at(i).type != 2)
            {
                double min_dis = 10000;
                int min_ID;
                for(V3DLONG j=0; j<list2.size();j++)
                {
                    double dis = sqrt(pow2(list.at(i).x - list2.at(j).x) + pow2(list.at(i).y - list2.at(j).y));
                    if(dis < min_dis)
                    {
                        min_dis = dis;
                        min_ID = j;

                    }
                }
                nt_3D.listNeuron[i].r = nt_2D.listNeuron[min_ID].r;
            }
        }

//        for(V3DLONG i =0; i<nt_3D.listNeuron.size();i++)
//        {
//            nt_3D.listNeuron[i].r = nt_2D.listNeuron[i].r;
//        }

        QString fileDefaultName = fileOpenName3D+QString("_assembled.swc");
        //write new SWC to file
        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                fileDefaultName,
                QObject::tr("Supported file (*.swc)"
                    ";;Neuron structure	(*.swc)"
                    ));
        writeSWC_file(fileSaveName,nt_3D);

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

    if (func_name == tr("neuron_radius"))
	{
        if(infiles.size() != 2 && infiles.size() != 3)
        {
            cerr<<"Invalid input"<<endl;
            return false;
        }
        string inimg_file = infiles[0];
        string inswc_file = infiles[1];
        string outswc_file = (infiles.size() == 3) ? infiles[2] : "";
        if(outswc_file == "") outswc_file = inswc_file + ".out.swc";

        double bkg_thresh = (inparas.size() >= 1) ? atof(inparas[0]) : 40;
        bool is_2d = (inparas.size() == 2) ? atoi(inparas[1]) : 1;
        double stop_thresh = (inparas.size() == 3) ? atof(inparas[2]) : 0.001;


        cout<<"inimg_file = "<<inimg_file<<endl;
        cout<<"inswc_file = "<<inswc_file<<endl;
        cout<<"outswc_file = "<<outswc_file<<endl;
        cout<<"bkg_thresh = "<<bkg_thresh<<endl;
        cout<<"is2d = "<< (int)is_2d <<endl;
        cout<<"stop_thresh = "<< stop_thresh <<endl;

        unsigned char * inimg1d = 0;
        V3DLONG in_sz[4];
        int datatype;
        if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_sz, datatype)) return false;
        vector<MyMarker*> inswc = readSWC_file(inswc_file);
        if(inswc.empty()) return false;
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

        saveSWC_file(outswc_file, inswc);
        cout<<"The output swc is saved to "<<outswc_file<<endl;
        for(int i = 0; i < inswc.size(); i++) delete inswc[i];
        if(inimg1d){delete [] inimg1d; inimg1d = 0;}
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

NeuronTree interpolate_radius(NeuronTree input)
{
    NeuronTree result;
    V3DLONG siz = input.listNeuron.size();
    Tree tree;
    for (V3DLONG i=0;i<siz;i++)
    {
        NeuronSWC s = input.listNeuron[i];
        Point* pt = new Point;
        pt->x = s.x;
        pt->y = s.y;
        pt->z = s.z;
        pt->r = s.r;
        pt ->type = s.type;
        pt->p = NULL;
        pt->childNum = 0;
        tree.push_back(pt);
    }
    for (V3DLONG i=0;i<siz;i++)
    {
        if (input.listNeuron[i].pn<0) continue;
        V3DLONG pid = input.hashNeuron.value(input.listNeuron[i].pn);
        tree[i]->p = tree[pid];
        tree[pid]->childNum++;
    }
//	printf("tree constructed.\n");
    vector<Segment*> seg_list;
    for (V3DLONG i=0;i<siz;i++)
    {
        if (tree[i]->childNum!=1)//tip or branch point
        {
            Segment* seg = new Segment;
            Point* cur = tree[i];
            do
            {
                seg->push_back(cur);
                cur = cur->p;
            }
            while(cur && cur->childNum==1);
            seg_list.push_back(seg);
        }
    }
//	printf("segment list constructed.\n");
    for (V3DLONG i=0;i<seg_list.size();i++)
    {
        if(seg_list[i]->at(0)->type ==1 || seg_list[i]->at(0)->type ==2)
            continue;
        interpolate_path(seg_list[i]);
    }

    tree.clear();
    map<Point*, V3DLONG> index_map;
    for (V3DLONG i=0;i<seg_list.size();i++)
        for (V3DLONG j=0;j<seg_list[i]->size();j++)
        {
            tree.push_back(seg_list[i]->at(j));
            index_map.insert(pair<Point*, V3DLONG>(seg_list[i]->at(j), tree.size()-1));
        }
    for (V3DLONG i=0;i<tree.size();i++)
    {
        NeuronSWC S;
        Point* p = tree[i];
        S.n = i+1;
        if (p->p==NULL) S.pn = -1;
        else
            S.pn = index_map[p->p]+1;
        if (p->p==p) printf("There is loop in the tree!\n");
        S.x = p->x;
        S.y = p->y;
        S.z = p->z;
        S.r = p->r;
        S.type = p->type;
        result.listNeuron.push_back(S);
    }
    for (V3DLONG i=0;i<tree.size();i++)
    {
        if (tree[i]) {delete tree[i]; tree[i]=NULL;}
    }
    for (V3DLONG j=0;j<seg_list.size();j++)
        if (seg_list[j]) {delete seg_list[j]; seg_list[j] = NULL;}
    for (V3DLONG i=0;i<result.listNeuron.size();i++)
        result.hashNeuron.insert(result.listNeuron[i].n, i);
    return result;
}
