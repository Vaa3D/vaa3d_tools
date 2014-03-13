/* retype_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-03-13 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "retype_swc_plugin.h"

#include "basic_surf_objs.h"
#include <iostream>
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include "../APP2_zhi/app2/my_surf_objs.h"

using namespace std;
Q_EXPORT_PLUGIN2(retype_swc, retype_swc);
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
 
QStringList retype_swc::menulist() const
{
	return QStringList() 
		<<tr("retype")
		<<tr("about");
}

QStringList retype_swc::funclist() const
{
	return QStringList()
		<<tr("help");
}

void retype_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("retype"))
	{
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(fileOpenName.isEmpty())
            return;
        NeuronTree nt;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {

             nt = readSWC_file(fileOpenName);
            for(V3DLONG i = 0; i < nt.listNeuron.size(); i++)
            {
                if(i == 0)
                    nt.listNeuron[i].type = 1;
                else
                    nt.listNeuron[i].type = 3;

            }
            QString fileDefaultName = fileOpenName+QString("_retype.swc");
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
	}
	else
	{
        v3d_msg(tr("This is a plugin to retype the swc file (first node to be root, and the rest to be dendrites). "
			"Developed by Zhi Zhou, 2014-03-13"));
	}
}

bool retype_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

