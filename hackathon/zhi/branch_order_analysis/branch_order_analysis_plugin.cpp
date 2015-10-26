/* branch_order_analysis_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-10-23 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "branch_order_analysis_plugin.h"

#include <math.h>
#include "basic_surf_objs.h"
#include <iostream>
#include "../../../released_plugins/v3d_plugins/pruning_swc/my_surf_objs.h"

using namespace std;
#define PI 3.14159265359
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
Q_EXPORT_PLUGIN2(branch_order_analysis, branch_order_analysis);


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
 
QStringList branch_order_analysis::menulist() const
{
	return QStringList() 
        <<tr("analysis")
		<<tr("about");
}

QStringList branch_order_analysis::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void branch_order_analysis::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("analysis"))
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
        double length = 0;
        NeuronTree nt;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
             nt = readSWC_file(fileOpenName);
        }

        QVector<QVector<V3DLONG> > childs;


        V3DLONG neuronNum = nt.listNeuron.size();
        childs = QVector< QVector<V3DLONG> >(neuronNum, QVector<V3DLONG>() );

        for (V3DLONG i=0;i<neuronNum;i++)
        {
            V3DLONG par = nt.listNeuron[i].pn;
            if (par<0) continue;
            childs[nt.hashNeuron.value(par)].push_back(i);
        }

        QList<NeuronSWC> list = nt.listNeuron;
        vector<MyMarker*> swc_file = readSWC_file(fileOpenName.toStdString());

        int branchOrder_apical = 0;
        int branchOrder_basal = 0;

        for (int i=0;i<list.size();i++)
        {
            int parent_node = getParent(i,nt);
            if(parent_node == 1000000000 || 0)
                swc_file[i]->type = 0;
            else if (childs[parent_node].size()<=1)
                swc_file[i]->type =  swc_file[parent_node]->type;
            else if (childs[parent_node].size()>1)
            {
                swc_file[i]->type =  swc_file[parent_node]->type + 1;
            }
        }

        for (int i=0;i<list.size();i++)
        {
            if(list.at(i).type == 3 && swc_file[i]->type > branchOrder_basal)
                branchOrder_basal = swc_file[i]->type;
            else if (list.at(i).type == 4 && swc_file[i]->type > branchOrder_apical)
                branchOrder_apical = swc_file[i]->type;

        }

    //    v3d_msg(QString("apical is %1, basal is %2").arg(branchOrder_apical).arg(branchOrder_basal));
        double *branchapical_mean = new double[branchOrder_apical];
        QString disp_text = "branch orders vs. diameters (apical)\n\n";
        for (int j = 0; j < branchOrder_apical; j++)
        {
            int count = 0;
            double radius_sum = 0;
            for (int i = 0;i < list.size();i++)
            {
                if(list.at(i).type == 4 &&  swc_file[i]->type == j+1)
                {
                    double radius_node = list.at(i).radius;
                    radius_sum += radius_node;
                    count++;
                }
            }
            branchapical_mean[j] = 2*radius_sum/count;
            disp_text += "Branch order = " + QString::number (j+1)  + ", average diameter = "    + QString::number(branchapical_mean[j]) + ";\n";
        }

        double *branchbasal_mean = new double[branchOrder_basal];
        disp_text += "\n\n\nbranch orders vs. diameters (basal)\n\n";
        for (int j = 0; j < branchOrder_basal; j++)
        {
            int count = 0;
            double radius_sum = 0;
            for (int i = 0;i < list.size();i++)
            {
                if(list.at(i).type == 3 &&  swc_file[i]->type == j+1)
                {
                    double radius_node = list.at(i).radius;
                    radius_sum += radius_node;
                    count++;
                }
            }
            branchbasal_mean[j] = 2*radius_sum/count;
            disp_text += "Branch order = " + QString::number (j+1)  + ", average diameter = "    + QString::number(branchbasal_mean[j]) + ";\n";
        }

        v3d_msg(disp_text);

//       QString fileDefaultName = fileOpenName+QString("_ordered.swc");
//       //write new SWC to file
//       QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
//               fileDefaultName,
//               QObject::tr("Supported file (*.swc)"
//                   ";;Neuron structure	(*.swc)"
//                   ));

//       saveSWC_file(fileSaveName.toStdString(), swc_file);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2015-10-23"));
	}
}

bool branch_order_analysis::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

