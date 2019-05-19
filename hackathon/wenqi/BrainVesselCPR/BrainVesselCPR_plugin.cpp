/* BrainVesselCPR_plugin.cpp
 * This is a plugin for Brain Vessel CPR in MRA&MRI image
 * 2019-5-14 : by Wenqi Huang
 */

#include "BrainVesselCPR_plugin.h"
#include "BrainVesselCPR_filter.h"
#include "BrainVesselCPR_centerline.h"
#include "BrainVesselCPR_syncview.h"
#include "BrainVesselCPR_sampleplane.h"


void setWLWW(V3DPluginCallback2 &callback, QWidget *parent)
{
    SetContrastWidget * setWLWW_widget = new SetContrastWidget(callback, parent);
    setWLWW_widget->show();
}





Q_EXPORT_PLUGIN2(BrainVesselCPR, BrainVesselCPRPlugin);
 
QStringList BrainVesselCPRPlugin::menulist() const
{
	return QStringList() 
		<<tr("Start CPR")
        <<tr("Set MRI WL/WW")
        <<tr("synchronize 3D viewers")
		<<tr("about");
}

// menu bar settings
// QStringList BrainVesselCPRPlugin::funclist() const
// {
// 	return QStringList()
// 		<<tr("func1")
// 		<<tr("func2")
// 		<<tr("help");
// }

void BrainVesselCPRPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Start CPR"))
	{
		//v3d_msg("To be implemented.");
		startCPR(callback,parent);
    }
    else if(menu_name == tr("Set MRI WL/WW"))
    {
        setWLWW(callback,parent);
    }
    else if (menu_name == tr("synchronize 3D viewers"))
    {
        SynTwoImage(callback, parent);
    }
    else
	{
		v3d_msg(tr("This is a plugin for Brain Vessel CPR in MRA&MRI image. "
			"Developed by Wenqi Huang, 2019-5-14"));
	}
}

void startCPR(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandleList win_list = callback.getImageWindowList();
    v3dhandle curwin = callback.currentImageWindow();
    if(win_list.size()<1)
    {
        v3d_msg("No MRA Image Opened!");
        return;
    }

    //get 2 landmarks as start and end point
    LandmarkList landmark_list = callback.getLandmark(curwin);
    while(landmark_list.size()!=2)
    {
        v3d_msg("Please select TWO landmarks as start point and end point.");
        landmark_list = callback.getLandmark(curwin);
        //TODO: exception
    }
    //marker coord start from 1 instead of 0
    for(long i=0;i<2;i++)
    {
        landmark_list[i].x-=1;
        landmark_list[i].y-=1;
        landmark_list[i].z-=1;
    }
    v3d_msg(QObject::tr("Start point: (%1, %2, %3)\nEnd point: (%4, %5, %6)").\
            arg(landmark_list[0].x).arg(landmark_list[0].y).arg(landmark_list[0].z).\
            arg(landmark_list[1].x).arg(landmark_list[1].y).arg(landmark_list[1].z));


    // get 3d image info & 1d data vector
    Image4DSimple* p4DImage = callback.getImage(curwin);
    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    unsigned short int * data1d = (unsigned short int *) p4DImage->getRawData();
    V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();
    V3DLONG x_length = p4DImage->getXDim();
    V3DLONG y_length = p4DImage->getYDim();
    V3DLONG z_length = p4DImage->getZDim();
    V3DLONG channels = p4DImage->getCDim();
    V3DLONG UnitBytes  = p4DImage->getUnitBytes();

    cout << "total bytes: " << totalpxls << endl << "width: " << x_length << endl \
         << "height: " << y_length << endl << "slice num: " << z_length \
         << endl << "channel: " << channels << endl << "unit bytes: " << UnitBytes << endl;

    //convert landmark to 1d data index
    V3DLONG start;
    V3DLONG goal;
//    test start (113, 123,138), id: , goal (138, 198, 76), id:
//    start = x_length * y_length * 138 + 123 * x_length + 113;
//    goal = x_length * y_length * 76 + 198 * x_length + 138;
    start = V3DLONG(landmark_list[0].x) + V3DLONG(landmark_list[0].y) * x_length + V3DLONG(landmark_list[0].z) * x_length * y_length;
    goal = V3DLONG(landmark_list[1].x) + V3DLONG(landmark_list[1].y) * x_length + V3DLONG(landmark_list[1].z) * x_length * y_length;

    //find path begins!
    cout << "begin find path!" << endl;

    float* data1d_gausssian = 0;
    //data1d_gausssian = new float[x_length*y_length*z_length];
    V3DLONG *in_sz;
    in_sz = new V3DLONG[4];
    in_sz[0] = x_length;
    in_sz[1] = y_length;
    in_sz[2] = z_length;
    in_sz[3] = 1;
    gaussian_filter(data1d, in_sz, 7, 7, 7, 1, 100, data1d_gausssian);
    unsigned short int * data1d_gaussian_uint16;
    data1d_gaussian_uint16 = new unsigned short int[x_length*y_length*z_length];
    for(int i = 0; i < x_length*y_length*z_length; i++)
    {
        data1d_gaussian_uint16[i] = (unsigned short int)data1d_gausssian[i];
    }
    findPath(start, goal, data1d_gaussian_uint16, x_length, y_length, z_length, callback, parent);
    cout << "find path finished!" << endl;



    //sync 3d view of MRA and MRI


}



// menu bar funtions
// bool BrainVesselCPRPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
// {
// 	vector<char*> infiles, inparas, outfiles;
// 	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
// 	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
// 	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

// 	if (func_name == tr("func1"))
// 	{
// 		v3d_msg("To be implemented.");
// 	}
// 	else if (func_name == tr("func2"))
// 	{
// 		v3d_msg("To be implemented.");
// 	}
// 	else if (func_name == tr("help"))
// 	{
// 		v3d_msg("To be implemented.");
// 	}
// 	else return false;

// 	return true;
// }

