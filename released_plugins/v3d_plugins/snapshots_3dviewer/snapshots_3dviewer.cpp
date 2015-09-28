#include <iostream>

#include "basic_surf_objs.h"
#include "snapshots_3dviewer.h"

using namespace std;
//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(snapshots_3dviewer, SnapShots_3Dviewer)


void snapShots3Dviewer(V3DPluginCallback2 & v3d, QWidget * parent);

//plugin funcs
const QString title = "Saving Snapshots From 3D Viewer";
QStringList SnapShots_3Dviewer::menulist() const
{
	return QStringList()
			<< tr("take a snapshot of 3D viewer");
}

void SnapShots_3Dviewer::domenu(const QString & menu_name,	V3DPluginCallback2 & v3d, QWidget * parent)
{
    if (menu_name == tr("take a snapshot of 3D viewer"))
	{
        snapShots3Dviewer(v3d, parent);
	}
}

void SnapShots_3Dviewer(V3DPluginCallback2 & v3d, QWidget * parent)
{
}

void snapShots3Dviewer(V3DPluginCallback2 & v3d, QWidget * parent)
{
    QFileDialog inANO_d(parent);
    inANO_d.setWindowTitle(QObject::tr("Choose input ano filename"));
    inANO_d.setAcceptMode(QFileDialog::AcceptOpen);
    if (!inANO_d.exec()) return;
    QString inANO_fn = (inANO_d.selectedFiles())[0];

    P_ObjectFileType cc;
    if(! loadAnoFile(inANO_fn,cc)){
        cout <<"Fail to load ano file" <<endl;
        return;
    }


    QString output_d=QFileDialog::getExistingDirectory(parent,
    QString(QObject::tr("Choose the output directory, where the snapshots would be saved.")));


    float x_rot = QInputDialog::getDouble(parent, "Roation about x axis",
                                 "Rotation about x :",
                                 90, -180, 180);


    float  y_rot = QInputDialog::getDouble(parent, "Roation about y axis",
                                 "Rotation about y :",
                                0, -180, 180);



    float z_rot = QInputDialog::getDouble(parent, "Roation about z axis",
                                 "Rotation about z :",
                                 90, -180, 180);

    for(int i = 0; i <cc.swc_file_list.size(); i++)
    {
        QString swc_file = cc.swc_file_list[i];

        v3d.open3DViewerForSingleSurfaceFile(swc_file);

        QList<V3dR_MainWindow * > list_3dviewer = v3d.getListAll3DViewers();
        V3dR_MainWindow * surface_win = list_3dviewer[i];
        if (!surface_win)
        {
            cout << "surface_win is empty"<<endl;
        }
        View3DControl  *view = v3d.getView3DControl_Any3DViewer(surface_win);
        view->doAbsoluteRot(x_rot,y_rot,z_rot);
        swc_file = QFileInfo(swc_file).fileName();
        QString BMPfilename = QDir(output_d).absolutePath()+ '/'+swc_file;
       // v3d_msg(BMPfilename);
        v3d.update_3DViewer(surface_win);
        v3d.screenShot_Any3DViewer(surface_win, BMPfilename);

        v3d.close3DWindow(surface_win);
    }

}
