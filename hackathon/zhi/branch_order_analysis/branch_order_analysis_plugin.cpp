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
 
QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    if (method_code ==1)
        imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";
    else if (method_code ==2)
        imgSuffix<<"*.marker";
    else if (method_code ==3)
        imgSuffix<<"*.raw"<<"*.v3draw"<<"*.v3dpbd"<<"*.tif"<<"*.RAW"<<"*.V3DRAW"<<"*.TIF"<<"*.V3DPBD";

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
bool analysis_swc(QString fileOpenName ,bool bmenu);
bool batch_analysis_swc(QString fileOpenName ,bool bmenu,int index,int number,QFile &apical_file);


QStringList branch_order_analysis::menulist() const
{
	return QStringList() 
        <<tr("analysis")
        <<tr("batch_analysis")
		<<tr("about");
}

QStringList branch_order_analysis::funclist() const
{
	return QStringList()
        <<tr("analysis")
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

        bool bmenu = 1;
        analysis_swc(fileOpenName ,bmenu);
        return;
	}
    else if (menu_name == tr("batch_analysis"))
    {
        QString m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all swc files "),
                                                                      QDir::currentPath(),
                                                                      QFileDialog::ShowDirsOnly);
        QStringList swcList = importFileList_addnumbersort(m_InputfolderName, 1);
        bool bmenu = 0;

        QString single_apical_file = m_InputfolderName + "_apical.csv";
        QFile apical_file(single_apical_file);
        if (!apical_file.open(QFile::WriteOnly|QFile::Truncate))
        {
            cout <<"Error opening the file "<<single_apical_file.toStdString().c_str() << endl;
        }

        for(V3DLONG i = 0; i < swcList.size(); i++)
        {
           batch_analysis_swc(swcList.at(i) ,bmenu, i,swcList.size(),apical_file);

        }

        v3d_msg("Done!");
        return;
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

    if (func_name == tr("analysis"))
	{
        bool bmenu = false;
        QString fileOpenName = infiles[0];
        analysis_swc(fileOpenName ,bmenu);


    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

bool analysis_swc(QString fileOpenName ,bool bmenu)
{
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
    double branchapical_totalmean = 0;
    double branchapical_x = 0;
    QString disp_text = fileOpenName + "\n\n";
    disp_text += "branch orders vs. diameters (apical)\n\n";
    int count_apical = 0;

    QString output_csv_apical_file = fileOpenName + ".csv";
    QFile file(output_csv_apical_file);
    if (!file.open(QFile::WriteOnly|QFile::Truncate))
    {
        cout <<"Error opening the file "<<output_csv_apical_file.toStdString().c_str() << endl;
    }

    QTextStream stream (&file);
    stream << "Branch order (apical) vs. diameters"<<"\n";

    for (int j = 0; j < branchOrder_apical; j++)
    {

        stream << j+1 <<",";
        int count = 0;
        double radius_sum = 0;
        for (int i = 0;i < list.size();i++)
        {
            if(list.at(i).type == 4 &&  swc_file[i]->type == j+1)
            {
                double radius_node = list.at(i).radius;
                radius_sum += radius_node;
                branchapical_totalmean += radius_node*2;
                branchapical_x+= j+1;
                count++;
                count_apical++;
                stream << radius_node      <<",";
            }
        }
        stream << "\n";
        branchapical_mean[j] = 2*radius_sum/count;
        disp_text += "Branch order = " + QString::number (j+1)  + ", average diameter = "    + QString::number(branchapical_mean[j]) + ";\n";
    }

    branchapical_totalmean = branchapical_totalmean/count_apical;
    branchapical_x = branchapical_x/count_apical;
    double Sxy_apical = 0;
    double Sxx_apical = 0;

    stream << "Branch order (apical) vs. average diameters"<<"\n";


    for (int j = 0; j < branchOrder_apical; j++)
    {
        stream << j+1   <<",";
        stream << branchapical_mean[j]  <<",\n";
        for (int i = 0;i < list.size();i++)
        {
            if(list.at(i).type == 4 &&  swc_file[i]->type == j+1)
            {
                Sxy_apical+= ((j+1)-branchapical_x)*(list.at(i).radius*2-branchapical_totalmean);
                Sxx_apical+= ((j+1)-branchapical_x)*((j+1)-branchapical_x);
            }
        }
    }

    double  branchapical_slope = Sxy_apical/Sxx_apical;
    disp_text += "Slope = " + QString::number (branchapical_slope) + ";\n";
    stream << "Slope"  <<"," << branchapical_slope <<"\n\n";


    double *branchbasal_mean = new double[branchOrder_basal];
    double branchbasal_totalmean = 0;
    double branchbasal_x = 0;
    disp_text += "\n\n\nbranch orders vs. diameters (basal)\n\n";
    int count_basal = 0;
    stream << "Branch order (basal) vs. diameters"<<"\n";

    for (int j = 0; j < branchOrder_basal; j++)
    {
        stream << j+1 <<",";
        int count = 0;
        double radius_sum = 0;
        for (int i = 0;i < list.size();i++)
        {
            if(list.at(i).type == 3 &&  swc_file[i]->type == j+1)
            {
                double radius_node = list.at(i).radius;
                radius_sum += radius_node;
                branchbasal_totalmean += radius_node*2;
                branchbasal_x+= j+1;
                count++;
                count_basal++;
                stream << radius_node      <<",";

            }
        }
        stream << "\n";

        branchbasal_mean[j] = 2*radius_sum/count;
        disp_text += "Branch order = " + QString::number (j+1)  + ", average diameter = "    + QString::number(branchbasal_mean[j]) + ";\n";
    }

    branchbasal_totalmean = branchbasal_totalmean/count_basal;
    branchbasal_x = branchbasal_x/count_basal;
    double Sxy_basal = 0;
    double Sxx_basal = 0;

    stream << "Branch order (basal) vs. average diameters"<<"\n";

    for (int j = 0; j < branchOrder_basal; j++)
    {
        stream << j+1   <<",";
        stream << branchbasal_mean[j]  <<",\n";
        for (int i = 0;i < list.size();i++)
        {
            if(list.at(i).type == 3 &&  swc_file[i]->type == j+1)
            {
                Sxy_basal+= ((j+1)-branchbasal_x)*(list.at(i).radius*2-branchbasal_totalmean);
                Sxx_basal+= ((j+1)-branchbasal_x)*((j+1)-branchbasal_x);
            }
        }
    }

    double  branchbasal_slope = Sxy_basal/Sxx_basal;
    disp_text += "Slope = " + QString::number (branchbasal_slope) + ";\n";

    stream << "Slope"  <<"," << branchbasal_slope <<"\n\n";

    file.close();

    v3d_msg(disp_text,bmenu);
    return true;
}

bool batch_analysis_swc(QString fileOpenName ,bool bmenu, int index, int total_size, QFile &apical_file)
{
    QTextStream apical_stream (&apical_file);
    if(index==0) apical_stream << "["<<"\n";

    apical_stream << "\t{"<<"\n";
    apical_stream << "\t\t\"#Name\": \""<< fileOpenName <<"\"\n";
    apical_stream << "\t\t\"#Namefeature1\": \"fragment_branch_orders\","<<"\n";
    apical_stream << "\t\t\"#Namefeature2\": \"fragment_diameters\","<<"\n";
    apical_stream << "\t\t\"#Type\": \"Neuron\","<<"\n";

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
    double branchapical_totalmean = 0;
    double branchapical_x = 0;
    QString disp_text = fileOpenName + "\n\n";
    disp_text += "branch orders vs. diameters (apical)\n\n";
    int count_apical = 0;

    QString output_csv_apical_file = fileOpenName + ".csv";
    QFile file(output_csv_apical_file);
    if (!file.open(QFile::WriteOnly|QFile::Truncate))
    {
        cout <<"Error opening the file "<<output_csv_apical_file.toStdString().c_str() << endl;
    }

    QTextStream stream (&file);
    stream << "Branch order (apical) vs. diameters"<<"\n";

    for (int j = 0; j < branchOrder_apical; j++)
    {

        stream << j+1 <<",";
        int count = 0;
        double radius_sum = 0;
        for (int i = 0;i < list.size();i++)
        {
            if(list.at(i).type == 4 &&  swc_file[i]->type == j+1)
            {
                double radius_node = list.at(i).radius;
                radius_sum += radius_node;
                branchapical_totalmean += radius_node*2;
                branchapical_x+= j+1;
                count++;
                count_apical++;
                stream << radius_node      <<",";
            }
        }
        stream << "\n";
        branchapical_mean[j] = 2*radius_sum/count;
        disp_text += "Branch order = " + QString::number (j+1)  + ", average diameter = "    + QString::number(branchapical_mean[j]) + ";\n";
    }

    branchapical_totalmean = branchapical_totalmean/count_apical;
    branchapical_x = branchapical_x/count_apical;
    double Sxy_apical = 0;
    double Sxx_apical = 0;

    stream << "Branch order (apical) vs. average diameters"<<"\n";

    apical_stream << "\t\t\"#raw\": \"";
    for (int j = 0; j < branchOrder_apical; j++)
    {
        stream << j+1   <<",";
        stream << branchapical_mean[j]  <<",\n";
        apical_stream << branchapical_mean[j] <<",";

        for (int i = 0;i < list.size();i++)
        {
            if(list.at(i).type == 4 &&  swc_file[i]->type == j+1)
            {
                Sxy_apical+= ((j+1)-branchapical_x)*(list.at(i).radius*2-branchapical_totalmean);
                Sxx_apical+= ((j+1)-branchapical_x)*((j+1)-branchapical_x);
            }
        }
    }

    double  branchapical_slope = Sxy_apical/Sxx_apical;
    disp_text += "Slope = " + QString::number (branchapical_slope) + ";\n";
    stream << "Slope"  <<"," << branchapical_slope <<"\n\n";

    apical_stream <<"\",\n";
    apical_stream << "\t\t\"#statFunct\": \"mean\","<<"\n";
    apical_stream << "\t\t\"linear\": \""<<branchapical_slope<<",\",\n";

    double *branchbasal_mean = new double[branchOrder_basal];
    double branchbasal_totalmean = 0;
    double branchbasal_x = 0;
    disp_text += "\n\n\nbranch orders vs. diameters (basal)\n\n";
    int count_basal = 0;
    stream << "Branch order (basal) vs. diameters"<<"\n";

    for (int j = 0; j < branchOrder_basal; j++)
    {
        stream << j+1 <<",";
        int count = 0;
        double radius_sum = 0;
        for (int i = 0;i < list.size();i++)
        {
            if(list.at(i).type == 3 &&  swc_file[i]->type == j+1)
            {
                double radius_node = list.at(i).radius;
                radius_sum += radius_node;
                branchbasal_totalmean += radius_node*2;
                branchbasal_x+= j+1;
                count++;
                count_basal++;
                stream << radius_node      <<",";

            }
        }
        stream << "\n";

        branchbasal_mean[j] = 2*radius_sum/count;
        disp_text += "Branch order = " + QString::number (j+1)  + ", average diameter = "    + QString::number(branchbasal_mean[j]) + ";\n";
    }

    branchbasal_totalmean = branchbasal_totalmean/count_basal;
    branchbasal_x = branchbasal_x/count_basal;
    double Sxy_basal = 0;
    double Sxx_basal = 0;

    stream << "Branch order (basal) vs. average diameters"<<"\n";

    for (int j = 0; j < branchOrder_basal; j++)
    {
        stream << j+1   <<",";
        stream << branchbasal_mean[j]  <<",\n";
        for (int i = 0;i < list.size();i++)
        {
            if(list.at(i).type == 3 &&  swc_file[i]->type == j+1)
            {
                Sxy_basal+= ((j+1)-branchbasal_x)*(list.at(i).radius*2-branchbasal_totalmean);
                Sxx_basal+= ((j+1)-branchbasal_x)*((j+1)-branchbasal_x);
            }
        }
    }

    double  branchbasal_slope = Sxy_basal/Sxx_basal;
    disp_text += "Slope = " + QString::number (branchbasal_slope) + ";\n";

    stream << "Slope"  <<"," << branchbasal_slope <<"\n\n";

    file.close();

    v3d_msg(disp_text,bmenu);


    if(index==total_size-1)
    {
        apical_stream << "\t}"<<"\n";
        apical_stream << "]"<<"\n";
    }
    else
        apical_stream << "\t},"<<"\n";

    return true;


}

