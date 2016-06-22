/* flip_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-6-22 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "flip_swc_plugin.h"

#include <iostream>
#include "../IVSCC_sort_swc/openSWCDialog.h"


using namespace std;
Q_EXPORT_PLUGIN2(flip_swc, flip_swc);

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    QFile qf(fileOpenName);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(fileOpenName));
#endif
        return false;
    }
    QString info;
    while (! qf.atEnd())
    {
        char _buf[1000], *buf;
        qf.readLine(_buf, sizeof(_buf));
        for (buf=_buf; (*buf && *buf==' '); buf++); //skip space

        if (buf[0]=='\0')	continue;
        if (buf[0]=='#')
        {
           info = buf;
           myfile<< info.remove('\n') <<endl;
        }

    }

    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
};
 
QStringList flip_swc::menulist() const
{
	return QStringList() 
        <<tr("flip in y direction")
		<<tr("about");
}

QStringList flip_swc::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void flip_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("flip in y direction"))
	{
        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;

        QString fileOpenName = openDlg->file_name;

        double length = 0;
        NeuronTree nt;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
             bool ok;
             nt = openDlg->nt;
             length = QInputDialog::getDouble(parent, "Please specify the maximum Y dimension","Y dimension:",1,0,2147483647,0.1,&ok);
             if (!ok)
                 return;
        }

        for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
        {
            nt.listNeuron[i].y = length - nt.listNeuron[i].y -1;

        }
        QString fileDefaultName = fileOpenName+QString("_Yflipped.swc");
        //write new SWC to file
        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                fileDefaultName,
                QObject::tr("Supported file (*.swc)"
                    ";;Neuron structure	(*.swc)"
                    ));
        if (!export_list2file(nt.listNeuron,fileSaveName,fileOpenName))
        {
            v3d_msg("fail to write the output swc file.");
            return;
        }

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2016-6-22"));
	}
}

bool flip_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
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

