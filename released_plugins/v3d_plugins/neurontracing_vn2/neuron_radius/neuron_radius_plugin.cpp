//last change, 20121214
 
#include "v3d_message.h"
#include <vector>
#include "neuron_radius_plugin.h"
#include "my_surf_objs.h"
#include "stackutil.h"
#include "marker_radius.h"
#include "smooth_curve.h"
#include "hierarchy_prune.h"


using namespace std;
Q_EXPORT_PLUGIN2(neuron_radius, SWCRadiusPlugin);
 
QStringList SWCRadiusPlugin::menulist() const
{
	return QStringList() 
		<<tr("neuron_radius")
        <<tr("neuron_radius_current_window")
        <<tr("neuron_radius_terafly")
		<<tr("about");
}

QStringList SWCRadiusPlugin::funclist() const
{
	return QStringList()
		<<tr("neuron_radius")
		<<tr("help");
}

void SWCRadiusPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("neuron_radius"))
	{
		QDialog * dialog = new QDialog();
		QLineEdit * inimg_box = new QLineEdit("");
		QLineEdit * inswc_box = new QLineEdit("");
		QLineEdit * outswc_box = new QLineEdit("(optional)");
		QLineEdit * thresh_box = new QLineEdit("40");
		QCheckBox * is2d_checker = new QCheckBox("Is 2D radius");
		is2d_checker->setChecked(true);
		{
			QGridLayout * layout = new QGridLayout;
			layout->addWidget(new QLabel("Image path"), 0, 0, 1, 1);
			layout->addWidget(inimg_box, 0, 1, 1, 5);
			layout->addWidget(new QLabel("In swc path"), 1, 0, 1, 1);
			layout->addWidget(inswc_box, 1, 1, 1, 5);
			layout->addWidget(new QLabel("Threshold"), 2, 0, 1, 1);
			layout->addWidget(thresh_box, 2, 1, 1, 5);
			layout->addWidget(new QLabel("Out swc path"), 3, 0, 1, 1);
			layout->addWidget(outswc_box, 3, 1, 1, 5);
			layout->addWidget(is2d_checker, 4, 0, 1, 6);
			QPushButton * ok = new QPushButton("Ok");
			QPushButton * cancel = new QPushButton("Cancel");
			ok->setDefault(true);
			layout->addWidget(ok, 5, 0, 1, 3); 
			layout->addWidget(cancel, 5, 3, 1, 3); 
			dialog->setLayout(layout);
			connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
			connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
		}
		if(dialog->exec() != QDialog::Accepted) return;

		string inimg_file = inimg_box->text().toStdString();
		string inswc_file = inswc_box->text().toStdString();
		string outswc_file =outswc_box->text().toStdString();
		if(outswc_file == "" || outswc_file == "(optional)") outswc_file = inswc_file + ".out.swc";
		bool is_2d = is2d_checker->isChecked(); 

		double bkg_thresh = atof(thresh_box->text().toStdString().c_str());
		cout<<"inimg_file = "<<inimg_file<<endl;
		cout<<"inswc_file = "<<inswc_file<<endl;
		cout<<"outswc_file = "<<outswc_file<<endl;
		cout<<"bkg_thresh = "<<bkg_thresh<<endl;
        
        if (dialog) {delete dialog; dialog=0;}
		
		unsigned char * inimg1d = 0; 
        V3DLONG  in_sz[4];
		int datatype;
        if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_sz, datatype))
		{
			QMessageBox::information(0,"","Invalid Image!");
			return;
		}
		vector<MyMarker*> inswc = readSWC_file(inswc_file);
		if(inswc.empty()) return;
		for(int i = 0; i < inswc.size(); i++)
		{
			MyMarker * marker = inswc[i];
			if(is_2d)
				marker->radius = markerRadiusXY(inimg1d, in_sz, *marker, bkg_thresh);
			else
				marker->radius = markerRadius(inimg1d, in_sz, *marker, bkg_thresh);
		}
		saveSWC_file(outswc_file, inswc);
		QMessageBox::information(0,"", string("neuron radius is calculated successfully. \n\nThe output swc is saved to " +outswc_file).c_str());
		for(int i = 0; i < inswc.size(); i++) delete inswc[i];
		if(inimg1d){delete [] inimg1d; inimg1d = 0;}
	}
    if (menu_name == tr("neuron_radius_current_window"))
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }

        QDialog * dialog = new QDialog();
        QLineEdit * inswc_box = new QLineEdit("");
        QLineEdit * outswc_box = new QLineEdit("(optional)");
        QLineEdit * thresh_box = new QLineEdit("40");
        QCheckBox * is2d_checker = new QCheckBox("Is 2D radius");
        is2d_checker->setChecked(true);
        {
            QGridLayout * layout = new QGridLayout;
            layout->addWidget(new QLabel("In swc path"), 0, 0, 1, 1);
            layout->addWidget(inswc_box, 0, 1, 1, 5);
            layout->addWidget(new QLabel("Threshold"), 1, 0, 1, 1);
            layout->addWidget(thresh_box, 1, 1, 1, 5);
            layout->addWidget(new QLabel("Out swc path"), 2, 0, 1, 1);
            layout->addWidget(outswc_box, 2, 1, 1, 5);
            layout->addWidget(is2d_checker, 3, 0, 1, 6);
            QPushButton * ok = new QPushButton("Ok");
            QPushButton * cancel = new QPushButton("Cancel");
            ok->setDefault(true);
            layout->addWidget(ok, 5, 0, 1, 3);
            layout->addWidget(cancel, 5, 3, 1, 3);
            dialog->setLayout(layout);
            connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
        }
        if(dialog->exec() != QDialog::Accepted) return;

        string inswc_file = inswc_box->text().toStdString();
        string outswc_file =outswc_box->text().toStdString();
        if(outswc_file == "" || outswc_file == "(optional)") outswc_file = inswc_file + ".out.swc";
        bool is_2d = is2d_checker->isChecked();

        double bkg_thresh = atof(thresh_box->text().toStdString().c_str());

        V3DLONG  in_sz[4];
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
            if(is_2d)
                marker->radius = markerRadiusXY(inimg1d, in_sz, *marker, bkg_thresh);
            else
                marker->radius = markerRadius(inimg1d, in_sz, *marker, bkg_thresh);
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
    else if (menu_name == tr("neuron_radius_terafly"))
    {
        QDialog * dialog = new QDialog();
        QLineEdit * inimg_box = new QLineEdit("");
        QLineEdit * inswc_box = new QLineEdit("");
        QLineEdit * outswc_box = new QLineEdit("(optional)");
        QLineEdit * thresh_box = new QLineEdit("40");
        QCheckBox * is2d_checker = new QCheckBox("Is 2D radius");
        is2d_checker->setChecked(true);
        {
            QGridLayout * layout = new QGridLayout;
            layout->addWidget(new QLabel("Terafly image path"), 0, 0, 1, 1);
            layout->addWidget(inimg_box, 0, 1, 1, 5);
            layout->addWidget(new QLabel("In swc path"), 1, 0, 1, 1);
            layout->addWidget(inswc_box, 1, 1, 1, 5);
            layout->addWidget(new QLabel("Threshold"), 2, 0, 1, 1);
            layout->addWidget(thresh_box, 2, 1, 1, 5);
            layout->addWidget(new QLabel("Out swc path"), 3, 0, 1, 1);
            layout->addWidget(outswc_box, 3, 1, 1, 5);
            layout->addWidget(is2d_checker, 4, 0, 1, 6);
            QPushButton * ok = new QPushButton("Ok");
            QPushButton * cancel = new QPushButton("Cancel");
            ok->setDefault(true);
            layout->addWidget(ok, 5, 0, 1, 3);
            layout->addWidget(cancel, 5, 3, 1, 3);
            dialog->setLayout(layout);
            connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
        }
        if(dialog->exec() != QDialog::Accepted) return;

        string inimg_file = inimg_box->text().toStdString();
        string inswc_file = inswc_box->text().toStdString();
        string outswc_file =outswc_box->text().toStdString();
        if(outswc_file == "" || outswc_file == "(optional)") outswc_file = inswc_file + ".out.swc";
        bool is_2d = is2d_checker->isChecked();

        double bkg_thresh = atof(thresh_box->text().toStdString().c_str());
        cout<<"inimg_file = "<<inimg_file<<endl;
        cout<<"inswc_file = "<<inswc_file<<endl;
        cout<<"outswc_file = "<<outswc_file<<endl;
        cout<<"bkg_thresh = "<<bkg_thresh<<endl;

        if (dialog) {delete dialog; dialog=0;}

        V3DLONG *in_zz = 0;
        if(!callback.getDimTeraFly(inimg_file,in_zz))
        {
            return;
        }

        vector<MyMarker*> inswc = readSWC_file(inswc_file);
        for(int i = 0; i < inswc.size(); i++)
        {
            inswc[i]->radius = -1;
        }
        if(inswc.empty()) return;
        for(int i = 0; i < inswc.size(); i++)
        {
            int start_x,end_x,start_y,end_y,start_z,end_z;
            if(inswc[i]->radius == -1)
            {
                start_x = inswc[i]->x - 512; if(start_x<0) start_x = 0;
                end_x = inswc[i]->x + 512; if(end_x > in_zz[0]) end_x = in_zz[0];
                start_y = inswc[i]->y - 512;if(start_y<0) start_y = 0;
                end_y = inswc[i]->y + 512;if(end_y > in_zz[1]) end_y = in_zz[1];
                start_z = inswc[i]->z - 512;if(start_z<0) start_z = 0;
                end_z = inswc[i]->z + 512;if(end_z > in_zz[2]) end_z = in_zz[2];

                V3DLONG *in_sz = new V3DLONG[4];
                in_sz[0] = end_x - start_x;
                in_sz[1] = end_y - start_y;
                in_sz[2] = end_z - start_z;

                unsigned char * inimg1d = 0;
                inimg1d = callback.getSubVolumeTeraFly(inimg_file,start_x,end_x,
                                                       start_y,end_y,start_z,end_z);
                for(int j = 0; j < inswc.size(); j++)
                {
                    if(inswc[j]->radius == -1 && inswc[j]->x >= start_x+10 && inswc[j]->x <end_x-10
                            && inswc[j]->y >= start_y+10 && inswc[j]->y <end_y-10
                            && inswc[j]->z >= start_z+10 && inswc[j]->z <end_z-10)
                    {
                        MyMarker marker;
                        marker.x = inswc[j]->x-start_x;
                        marker.y = inswc[j]->y-start_y;
                        marker.z = inswc[j]->z-start_z;
                        if(is_2d)
                            inswc[j]->radius = markerRadiusXY(inimg1d, in_sz, marker, bkg_thresh);
                        else
                            inswc[j]->radius = markerRadius(inimg1d, in_sz, marker, bkg_thresh);
                    }
                }
                if(inimg1d) {delete []inimg1d; inimg1d=0;}
            }
        }
        if(in_zz) {delete []in_zz; in_zz=0;}

        vector<HierarchySegment*> topo_segs;
        swc2topo_segs_noImg(inswc, topo_segs);

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
		v3d_msg(tr("Re-estimate the radius of a swc. "
			"Developed by Hang Xiao, 2012-11-07"));
	}
}

bool SWCRadiusPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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
		bool is_2d = (inparas.size() == 2) ? atoi(inparas[1]) : 0;

		cout<<"inimg_file = "<<inimg_file<<endl;
		cout<<"inswc_file = "<<inswc_file<<endl;
		cout<<"outswc_file = "<<outswc_file<<endl;
		cout<<"bkg_thresh = "<<bkg_thresh<<endl;
        
		cout<<"is2d = "<< (int)is_2d <<endl;

		unsigned char * inimg1d = 0; 
        V3DLONG in_sz[4];
		int datatype;
        cout << "*********************!!!!!!!!!!!!!!!!"<<endl;
        if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_sz, datatype)) return false;
		vector<MyMarker*> inswc = readSWC_file(inswc_file);
		if(inswc.empty()) return false;
		for(int i = 0; i < inswc.size(); i++)
		{
			MyMarker * marker = inswc[i];
			if(is_2d)
				marker->radius = markerRadiusXY(inimg1d, in_sz, *marker, bkg_thresh);
			else
				marker->radius = markerRadius(inimg1d, in_sz, *marker, bkg_thresh);
            
            //printf("2d=%5.3f  \t 3d=%5.3f\n", markerRadiusXY(inimg1d, in_sz, *marker, bkg_thresh),
            //       markerRadius(inimg1d, in_sz, *marker, bkg_thresh));

		}
		saveSWC_file(outswc_file, inswc);
		cout<<"The output swc is saved to "<<outswc_file<<endl;
		for(int i = 0; i < inswc.size(); i++) delete inswc[i];
		if(inimg1d){delete [] inimg1d; inimg1d = 0;}
	}
    else if (func_name == tr("neuron_radius_2D"))
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
        bool is_2d = (inparas.size() == 2) ? atoi(inparas[1]) : 0;

        cout<<"inimg_file = "<<inimg_file<<endl;
        cout<<"inswc_file = "<<inswc_file<<endl;
        cout<<"outswc_file = "<<outswc_file<<endl;
        cout<<"bkg_thresh = "<<bkg_thresh<<endl;

        cout<<"is2d = "<< (int)is_2d <<endl;

        unsigned char * inimg1d = 0;
        V3DLONG in_sz[4];
        int datatype;
        if(!simple_loadimage_wrapper(callback,(char*)inimg_file.c_str(), inimg1d, in_sz, datatype)) return false;
        vector<MyMarker*> inswc = readSWC_file(inswc_file);
        vector<double> z_location;

        vector<MyMarker*> tempswc;
        for(V3DLONG i = 0; i <inswc.size(); i++)
        {
            z_location.push_back(inswc[i]->z);
            inswc[i]->z = 0;
            tempswc.push_back(inswc[i]);
        }

        if(inswc.empty()) return false;
        for(int i = 0; i < tempswc.size(); i++)
        {
            MyMarker * marker = tempswc[i];
            if(is_2d)
                marker->radius = markerRadiusXY(inimg1d, in_sz, *marker, bkg_thresh);
            else
                marker->radius = markerRadius(inimg1d, in_sz, *marker, bkg_thresh);

            //printf("2d=%5.3f  \t 3d=%5.3f\n", markerRadiusXY(inimg1d, in_sz, *marker, bkg_thresh),
            //       markerRadius(inimg1d, in_sz, *marker, bkg_thresh));

            tempswc[i]->z = z_location[i];
        }
        saveSWC_file(outswc_file, tempswc);
        cout<<"The output swc is saved to "<<outswc_file<<endl;
        for(int i = 0; i < inswc.size(); i++) delete inswc[i];
        if(inimg1d){delete [] inimg1d; inimg1d = 0;}
    }
    else if(func_name == tr("neuron_radius_terafly"))
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
        bool is_2d = (inparas.size() == 2) ? atoi(inparas[1]) : 0;

        cout<<"inimg_file = "<<inimg_file<<endl;
        cout<<"inswc_file = "<<inswc_file<<endl;
        cout<<"outswc_file = "<<outswc_file<<endl;
        cout<<"bkg_thresh = "<<bkg_thresh<<endl;
        cout<<"is2d = "<< (int)is_2d <<endl;

        V3DLONG *in_zz = 0;
        if(!callback.getDimTeraFly(inimg_file,in_zz))
        {
            return false;
        }

        vector<MyMarker*> inswc = readSWC_file(inswc_file);
        for(int i = 0; i < inswc.size(); i++)
        {
            inswc[i]->radius = -1;
        }
        if(inswc.empty()) return false;
        for(int i = 0; i < inswc.size(); i++)
        {
            int start_x,end_x,start_y,end_y,start_z,end_z;
            if(inswc[i]->radius == -1)
            {
                start_x = inswc[i]->x - 512; if(start_x<0) start_x = 0;
                end_x = inswc[i]->x + 512; if(end_x > in_zz[0]) end_x = in_zz[0];
                start_y = inswc[i]->y - 512;if(start_y<0) start_y = 0;
                end_y = inswc[i]->y + 512;if(end_y > in_zz[1]) end_y = in_zz[1];
                start_z = inswc[i]->z - 512;if(start_z<0) start_z = 0;
                end_z = inswc[i]->z + 512;if(end_z > in_zz[2]) end_z = in_zz[2];

                V3DLONG *in_sz = new V3DLONG[4];
                in_sz[0] = end_x - start_x;
                in_sz[1] = end_y - start_y;
                in_sz[2] = end_z - start_z;

                unsigned char * inimg1d = 0;
                inimg1d = callback.getSubVolumeTeraFly(inimg_file,start_x,end_x,
                                                       start_y,end_y,start_z,end_z);
                for(int j = 0; j < inswc.size(); j++)
                {
                    if(inswc[j]->radius == -1 && inswc[j]->x >= start_x+10 && inswc[j]->x <end_x-10
                            && inswc[j]->y >= start_y+10 && inswc[j]->y <end_y-10
                            && inswc[j]->z >= start_z+10 && inswc[j]->z <end_z-10)
                    {
                        MyMarker marker;
                        marker.x = inswc[j]->x-start_x;
                        marker.y = inswc[j]->y-start_y;
                        marker.z = inswc[j]->z-start_z;
                        if(is_2d)
                            inswc[j]->radius = markerRadiusXY(inimg1d, in_sz, marker, bkg_thresh);
                        else
                            inswc[j]->radius = markerRadius(inimg1d, in_sz, marker, bkg_thresh);
                    }
                }
                if(inimg1d) {delete []inimg1d; inimg1d=0;}
            }
        }
        if(in_zz) {delete []in_zz; in_zz=0;}
        vector<HierarchySegment*> topo_segs;
        swc2topo_segs_noImg(inswc, topo_segs);

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
        v3d_msg(QString("neuron radius is calculated successfully. \n\nThe output swc is saved to %1").arg(outswc_file.c_str()),0);
        for(int i = 0; i < inswc.size(); i++) delete inswc[i];

    }
	else //if (func_name == tr("help"))
	{
		cout<<"v3d -x <plugin_dll> -f neuron_radius -i <inimg_file> <inswc_file> -p <threshold> [<is2d>]"
        <<endl
        <<"The output will be appended  .out.swc automatically" 
        <<endl;
	}
    
    return true;
}

