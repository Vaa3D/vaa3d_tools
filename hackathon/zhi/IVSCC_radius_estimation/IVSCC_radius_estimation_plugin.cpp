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

using namespace std;

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

Q_EXPORT_PLUGIN2(IVSCC_radius_estimation, IVSCC_radius_estimation);
 
QStringList IVSCC_radius_estimation::menulist() const
{
	return QStringList() 
        <<tr("neuron_radius_current_window")
        <<tr("neuron_radius_selected_folder")
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
            if(marker->x > root_x - 150 && marker->x < root_x + 150 && marker->y > root_y - 150 && marker->y < root_y + 150)
                bkg_thresh_updated = stop_thresh;
            else
                bkg_thresh_updated = bkg_thresh;
            if(marker->parent > 0)
            {
                if(is_2d)
                    marker->radius = markerRadiusXY(inimg1d, in_sz, *marker, bkg_thresh_updated,0.001);
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
        double stop_thresh = dialog.stop_thresh/100;
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
            if(marker->parent > 0)
            {
                if(is_2d)
                    marker->radius = markerRadiusXY(im_imported, sz, *marker, bkg_thresh,stop_thresh);
                else
                    marker->radius = markerRadius(im_imported, sz, *marker, bkg_thresh);
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

