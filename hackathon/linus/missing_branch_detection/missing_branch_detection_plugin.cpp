/* missing_branch_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-01-13 : by LMG
 */
 
#include "v3d_message.h"
#include <vector>
#include <algorithm>
#include "missing_branch_detection_plugin.h"
#include "../../../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
using namespace std;
Q_EXPORT_PLUGIN2(missing_branch_detection, missing_branch);
 
QStringList missing_branch::menulist() const
{
	return QStringList() 
        <<tr("missing_branch_detection")
		<<tr("about");
}

QStringList missing_branch::funclist() const
{
	return QStringList()
        <<tr("missing_branch_detection")
        <<tr("help");
}

void missing_branch::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("missing_branch_detection"))
	{
        if(callback.currentImageWindow())
        {
            v3dhandle curwin = callback.currentImageWindow();
            Image4DSimple* p4DImage = callback.getImage(curwin);

            // Get Image Data
            int nChannel = p4DImage->getCDim();

            V3DLONG mysz[4];
            mysz[0] = p4DImage->getXDim();
            mysz[1] = p4DImage->getYDim();
            mysz[2] = p4DImage->getZDim();
            mysz[3] = nChannel;
            cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
            unsigned char *data1d_crop=p4DImage->getRawDataAtChannel(nChannel);

            // Run missing branch detection
            LandmarkList candidates;
            ImagePixelType pixtype = V3D_UINT16;
            candidates = get_missing_branches_menu(callback,parent,p4DImage);

//            QList <ImageMarker> candidates_mk;
//            for(V3DLONG i=0; i<candidates.size(); i++)
//            {
//                ImageMarker candidate;
//                candidate.x = candidates.at(i).x;
//                candidate.y = candidates.at(i).y;
//                candidate.z = candidates.at(i).z;
//                candidates_mk.push_back(candidate);
//            }
            if(callback.setLandmark(curwin,candidates));
            if(load_data(&callback,data1d_crop,candidates,pixtype,mysz,curwin))
            {
                QList <ImageMarker> final_pts;
                vector <long> final_ptsx,final_ptsy,final_ptsz;
//                vector <struct XYZ> final_pts_xyz;
                mean_shift_fun ms;
                qDebug() << "mean shift fun instantiated";
                for(V3DLONG i=0; i<candidates.size(); i++)
                {
                    vector <float> final_pt;
                    ImageMarker final_pt_mk;
                    qDebug() << "mean shift not called";
                    final_pt = ms.mean_shift_center(i,20);
                    qDebug() << "mean shift called";
                    final_pt_mk.x = final_pt[0];
                    final_pt_mk.y = final_pt[1];
                    final_pt_mk.z = final_pt[2];
                    if(find(final_ptsx.begin(),final_ptsx.end(),final_pt_mk.x)!=final_ptsx.end() ||
                       find(final_ptsy.begin(),final_ptsy.end(),final_pt_mk.y)!=final_ptsy.end() ||
                       find(final_ptsz.begin(),final_ptsz.end(),final_pt_mk.z)!=final_ptsz.end())
                    {
                        final_ptsx.push_back(long(final_pt[0]));
                        final_ptsy.push_back(long(final_pt[1]));
                        final_ptsz.push_back(long(final_pt[2]));
                        final_pts.push_back(final_pt_mk);
                    }
//                    struct XYZ final_pt_xyz = XYZ(final_pt[0],final_pt[1],final_pt[2]);
//                    if(find(final_pts_xyz.begin(),final_pts_xyz.end(),final_pt_xyz) != final_pts_xyz.end()) final_pts_xyz.push_back(final_pt_xyz);
                }
                qDebug() << "Final points size:" << final_pts.size();

            }
        }
        else v3d_msg(tr("An open 3D image is needed."));
	}
	else
	{
        v3d_msg(tr("This is a plugin for missing branch detection."
            "Developed by LMG, 2019-01-13"));
	}
}

bool missing_branch::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{

    if (func_name == tr("missing_branch_detection"))
	{
        get_missing_branches_func(input,output,callback);
	}
	else if (func_name == tr("help"))
	{
        printHelp(input,output);
    }
	else return false;

	return true;
}

