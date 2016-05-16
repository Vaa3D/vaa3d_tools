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
        QList <ImageMarker> listLandmarks;

        for(int i = 0; i < file_inmarkers.size(); i++)
        {
            ImageMarker t;
            t.x = file_inmarkers[i].x;
            t.y = file_inmarkers[i].y;
            t.z = file_inmarkers[i].z;
            listLandmarks.push_back(t);
        }

        QString fileDefaultName = fileOpenName + ".marker";
        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                            fileDefaultName,
                                                            QObject::tr("Supported file (*.marker)"));
        if (fileSaveName.isEmpty())
            return;
        else
        {
            writeMarker_file(fileSaveName,listLandmarks);
            v3d_msg(QString("Marker file is save as %1").arg(fileSaveName.toStdString().c_str()));
        }

	}
	else
	{
        v3d_msg(tr("This is a plugin to convert apo file to marker file,"
			"Developed by YourName, 2016-5-16"));
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

