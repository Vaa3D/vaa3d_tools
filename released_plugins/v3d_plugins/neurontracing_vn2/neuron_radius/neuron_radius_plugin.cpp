//last change, 20121214
 
#include "v3d_message.h"
#include <vector>
#include "neuron_radius_plugin.h"
#include "my_surf_objs.h"
#include "stackutil.h"
#include "marker_radius.h"

using namespace std;
Q_EXPORT_PLUGIN2(neuron_radius, SWCRadiusPlugin);
 
QStringList SWCRadiusPlugin::menulist() const
{
	return QStringList() 
		<<tr("neuron_radius")
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

    if (func_name == tr("neuron_radius_2D"))
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
	else //if (func_name == tr("help"))
	{
		cout<<"v3d -x <plugin_dll> -f neuron_radius -i <inimg_file> <inswc_file> -p <threshold> [<is2d>]"
        <<endl
        <<"The output will be appended  .out.swc automatically" 
        <<endl;
	}
    
    return true;
}

