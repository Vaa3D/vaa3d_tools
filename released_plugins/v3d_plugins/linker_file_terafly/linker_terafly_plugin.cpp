/* linker_terafly_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-3-13 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "linker_terafly_plugin.h"
#include "../../../hackathon/zhi/IVSCC_sort_swc/openSWCDialog.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"

QStringList importSeriesFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.swc";

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

using namespace std;
Q_EXPORT_PLUGIN2(linker_terafly, linker_terafly);
 
QStringList linker_terafly::menulist() const
{
	return QStringList() 
		<<tr("linker")
     //   <<tr("linker_folder")
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
    }else if  (menu_name == tr("linker_folder"))
    {

        QStringList imgList = importSeriesFileList_addnumbersort("/Volumes/Samsung_T5/whole_mouse_brains/reconstructions/124_full_resolution");

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
        NeuronTree nt;
        QList<CellAPO> file_inmarkers;
        int NTILES  = vim.tilesList.size();
        CellAPO t;

        for(V3DLONG ii = 0; ii < NTILES; ii++)
        {
            nt = readSWC_file(QString(vim.tilesList.at(ii).fn_image.c_str()));

            t.x = nt.listNeuron[0].x;
            t.y = nt.listNeuron[0].y;
            t.z = nt.listNeuron[0].z;
            t.name = QString("%1").arg(ii+1);
            t.volsize = 50;
            t.color.r = 0;
            t.color.g = 0;
            t.color.b = 255;
            file_inmarkers.push_back(t);
        }

        QString apo_name = "/Volumes/Samsung_T5/whole_mouse_brains/reconstructions/124_full_resolution/somas.apo";
        writeAPO_file(apo_name,file_inmarkers);
        QFile qf_anofile("/Volumes/Samsung_T5/whole_mouse_brains/reconstructions/124_full_resolution/somas.ano");
        QTextStream out(&qf_anofile);
        out << "APOFILE=" << QFileInfo(apo_name).fileName()<<endl;
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

