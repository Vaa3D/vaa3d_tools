/* linker_terafly_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-3-13 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "linker_terafly_plugin.h"
#include "../IVSCC_sort_swc/openSWCDialog.h"


using namespace std;
Q_EXPORT_PLUGIN2(linker_terafly, linker_terafly);
 
QStringList linker_terafly::menulist() const
{
	return QStringList() 
		<<tr("linker")
		<<tr("about");
}

QStringList linker_terafly::funclist() const
{
	return QStringList()
		<<tr("help");
}

void linker_terafly::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("linker"))
	{
        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;

        QString fileOpenName = openDlg->file_name;
        NeuronTree nt;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
             nt = openDlg->nt;
        }


        QList<CellAPO> file_inmarkers;

        for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
        {
            CellAPO t;
            if(nt.listNeuron[i].parent==-1)
            {
                t.x = nt.listNeuron[i].x;
                t.y = nt.listNeuron[i].y;
                t.z = nt.listNeuron[i].z;
                t.volsize = 50;
                t.color.r = 0;
                t.color.g = 0;
                t.color.b = 255;

                file_inmarkers.push_back(t);
            }
        }

        QString apo_name = fileOpenName + ".apo";
        writeAPO_file(apo_name,file_inmarkers);
        QString linker_name = fileOpenName + ".ano";
        QFile qf_anofile(linker_name);
        if(!qf_anofile.open(QIODevice::WriteOnly))
        {
            v3d_msg("Cannot open file for writing!");
            return;
        }

        QTextStream out(&qf_anofile);
        out << "SWCFILE=" << QFileInfo(fileOpenName).fileName()<<endl;
        out << "APOFILE=" << QFileInfo(apo_name).fileName()<<endl;
        v3d_msg(QString("Save the linker file to: %1 Complete!").arg(linker_name));


	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2018-3-13"));
	}
}

bool linker_terafly::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

