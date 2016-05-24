/* swc_removal_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-5-16 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "swc_removal_plugin.h"
#include <iostream>
#include "../AllenNeuron_postprocessing/sort_swc_IVSCC.h"
#include "../IVSCC_sort_swc/openSWCDialog.h"




using namespace std;
Q_EXPORT_PLUGIN2(swc_removal, swc_removal);

bool export_list2file_removal(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName)
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
 
QStringList swc_removal::menulist() const
{
	return QStringList() 
		<<tr("remove_swc")
		<<tr("about");
}

QStringList swc_removal::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("help");
}

void swc_removal::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("remove_swc"))
	{
        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;

        QString fileOpenName = openDlg->file_name;

        int type;
        NeuronTree nt;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
             bool ok;
             nt = openDlg->nt;
             type = QInputDialog::getInteger(parent, "Please specify the node type","type:",2,0,256,1,&ok);
             if (!ok)
                 return;

             QList<NeuronSWC> list = nt.listNeuron;
             V3DLONG *flag = new V3DLONG[list.size()];


             for (int i=0;i<list.size();i++)
             {

                 if (list[i].type == type)
                     flag[i] = -1;
                 else
                     flag[i] = 1;
             }
             NeuronTree nt_prunned;
             QList <NeuronSWC> listNeuron;
             QHash <int, int>  hashNeuron;
             listNeuron.clear();
             hashNeuron.clear();

             //set node

             NeuronSWC S;
             for (int i=0;i<list.size();i++)
             {
                 if(flag[i] == 1)
                 {
                      NeuronSWC curr = list.at(i);
                      S.n 	= curr.n;
                      S.type 	= curr.type;
                      S.x 	= curr.x;
                      S.y 	= curr.y;
                      S.z 	= curr.z;
                      S.r 	= curr.r;
                      S.pn 	= curr.pn;
                      listNeuron.append(S);
                      hashNeuron.insert(S.n, listNeuron.size()-1);
                 }

            }
             nt_prunned.n = -1;
             nt_prunned.on = true;
             nt_prunned.listNeuron = listNeuron;
             nt_prunned.hashNeuron = hashNeuron;

             if(flag) {delete[] flag; flag = 0;}

             swcSortDialog dialog(callback, parent,nt_prunned.listNeuron);
             if (dialog.exec()!=QDialog::Accepted)
                 return;


             V3DLONG rootid = dialog.rootid;
             V3DLONG thres = dialog.thres;

             if(rootid == 0)  rootid = VOID;
             if(thres == -1)  thres = VOID;

             NeuronTree nt_sort = SortSWC_pipeline(nt_prunned.listNeuron,rootid, thres);     //added one more sorting step by Zhi Zhou


             QString fileDefaultName = fileOpenName+QString("_removed.swc");
             //write new SWC to file
             QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                     fileDefaultName,
                     QObject::tr("Supported file (*.swc)"
                         ";;Neuron structure	(*.swc)"
                         ));
             if (!export_list2file_removal(nt_sort.listNeuron,fileSaveName,fileOpenName))
             {
                 v3d_msg("fail to write the output swc file.");
                 return;
             }

        }
    }
	else
	{
        v3d_msg(tr("This is a plugin to remove user specified branches based on the type information. "
			"Developed by YourName, 2016-5-16"));
	}
}

bool swc_removal::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

