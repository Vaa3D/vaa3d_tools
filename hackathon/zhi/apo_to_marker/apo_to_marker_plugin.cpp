/* apo_to_marker_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-5-16 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "apo_to_marker_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(apo_to_marker, apo_to_marker);
 
QStringList apo_to_marker::menulist() const
{
	return QStringList() 
        <<tr("save apo to marker format")
        <<tr("save swc file to apo format")
		<<tr("about");
}

QStringList apo_to_marker::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}

void apo_to_marker::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("save apo to marker format"))
	{
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Point Cloud File"),
                                                    "",
                                                    QObject::tr("Supported file (*.apo *.APO)"
                                                        ));
        if (fileOpenName.isEmpty())
            return;

        QList<CellAPO> file_inmarkers;
        file_inmarkers = readAPO_file(fileOpenName);
        QList <CellAPO> listLandmarks;

        for(int i = 0; i < file_inmarkers.size(); i++)
        {
//            file_inmarkers[i].x = file_inmarkers[i].x/2;
//            file_inmarkers[i].y = file_inmarkers[i].y/2;
//            file_inmarkers[i].z = file_inmarkers[i].z/2;

            CellAPO t;
            t.x = file_inmarkers[i].x;
            t.y = file_inmarkers[i].y;
            t.z = file_inmarkers[i].z;
            t.color = file_inmarkers[i].color;
//            t.color.r = 255; t.color.g = 0; t.color.b = 0;
            t.volsize = 314.159;
            listLandmarks.push_back(t);
            QString fileDefaultName;
            if(i<9)
                fileDefaultName=QString("/local1/work/SEUAllenJointDataCenter/finished_annotations/18457_UndoNeurons/00%1/00%1_v1.apo").arg(i+1).arg(i+1);
            else if(i<99)
                fileDefaultName=QString("/local1/work/SEUAllenJointDataCenter/finished_annotations/18457_UndoNeurons/0%1/0%1_v1.apo").arg(i+1).arg(i+1);
            else
                fileDefaultName=QString("/local1/work/SEUAllenJointDataCenter/finished_annotations/18457_UndoNeurons/%1/%1_v1.apo").arg(i+1).arg(i+1);

            writeAPO_file(fileDefaultName,listLandmarks);
            listLandmarks.clear();
        }

//        QString fileDefaultName = fileOpenName + ".apo";
//        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
//                                                            fileDefaultName,
//                                                            QObject::tr("Supported file (*.apo)"));
//        if (fileSaveName.isEmpty())
//            return;
//        else
//        {
//            writeAPO_file(fileSaveName,file_inmarkers);
//            v3d_msg(QString("Point Cloud file is save as %1").arg(fileSaveName.toStdString().c_str()));
//        }


//        QString fileDefaultName = fileOpenName + ".marker";
//        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
//                                                            fileDefaultName,
//                                                            QObject::tr("Supported file (*.marker)"));
//        if (fileSaveName.isEmpty())
//            return;
//        else
//        {
//            writeMarker_file(fileSaveName,listLandmarks);
//            v3d_msg(QString("Marker file is save as %1").arg(fileSaveName.toStdString().c_str()));
//        }

    }
    else if (menu_name == tr("save swc file to apo format"))
    {
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
                                                    "",
                                                    QObject::tr("Supported file (*.swc *.SWC)"
                                                        ));
        if (fileOpenName.isEmpty())
            return;

        unsigned int Vsize;
        bool ok1;

        Vsize = QInputDialog::getInteger(parent, "Volume size ",
                                      "Enter volume size:",
                                      50, 1, 1000, 1, &ok1);
        if(!ok1)
            return;

        NeuronTree nt = readSWC_file(fileOpenName);
        QList<CellAPO> file_inmarkers;
        for(V3DLONG i = 0; i <nt.listNeuron.size();i++)
        {
            CellAPO t;
            t.x = nt.listNeuron.at(i).x+1;
            t.y = nt.listNeuron.at(i).y+1;
            t.z = nt.listNeuron.at(i).z+1;
            t.color.r=255;
            t.color.g=0;
            t.color.b=0;

            t.volsize = Vsize;
            file_inmarkers.push_back(t);
        }

        QString fileDefaultName = fileOpenName + ".apo";
        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                            fileDefaultName,
                                                            QObject::tr("Supported file (*.apo)"));
        if (fileSaveName.isEmpty())
            return;
        else
        {
            writeAPO_file(fileSaveName,file_inmarkers);
            v3d_msg(QString("Point Cloud file is save as %1").arg(fileSaveName.toStdString().c_str()));
        }

    }
	else
	{
        v3d_msg(tr("This is a plugin to convert apo file to marker file,"
            "Developed by Zhi Zhou, 2016-5-16"));
	}
}

bool apo_to_marker::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
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

