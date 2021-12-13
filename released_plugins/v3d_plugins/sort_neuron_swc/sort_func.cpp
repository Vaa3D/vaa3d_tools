/*
 *  sort_func.cpp
 *  io functions for domenu and do func 
 *
 *  Created by Wan, Yinan, on 02/01/2012.
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "sort_func.h"
#include "openSWCDialog.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include <vector>
#include <iostream>
#include "sort_swc.h"
#include <QInputDialog>
using namespace std;

const QString title = QObject::tr("Sort SWC Plugin");

void sort_menu(V3DPluginCallback2 &callback, QWidget *parent)
{
    //cout<<"callback_sort: "<<(void*)&callback<<endl;
    //callback.getImageWindowList();
    //v3d_msg("image window list sort", 0);

	OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
	if (!openDlg->exec())
		return;

    NeuronTree nt = openDlg->nt;
	QList<NeuronSWC> neuron = nt.listNeuron;

    V3DLONG rootid;
	V3DLONG thres;
	bool ok;
    rootid = QInputDialog::getInt(0, "Would you like to specify new root number?","New root number:(If you select 'cancel', the first root in file is set as default)",1,1,neuron.size(),1,&ok);
	if (!ok)
        rootid = VOID_VALUE;
	thres = QInputDialog::getDouble(0, "Would you like to set a threshold for the newly generated link?","threshold:(If you select 'cancel', all the points will be connected automated; If you set '0', no new link will be generated)",0,0,2147483647,1,&ok);
	if (!ok)
        thres = VOID_VALUE;

	QList<NeuronSWC> result;
	QString fileOpenName = openDlg->file_name;
	if (SortSWC(neuron, result ,rootid, thres))
	{
        QString fileDefaultName;
        QString fileSaveName;
        if(fileOpenName.endsWith(".eswc",Qt::CaseInsensitive))
        {
            fileDefaultName = fileOpenName+QString("_sorted.eswc");
            fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                        fileDefaultName,
                                                        QObject::tr("Supported file (*.eswc)"
                                                                    ";;Neuron structure	(*.eswc)"
                                                                    ));

        }
        else
        {
            fileDefaultName = fileOpenName+QString("_sorted.swc");
            fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                                                        fileDefaultName,
                                                        QObject::tr("Supported file (*.swc)"
                                                                    ";;Neuron structure	(*.swc)"
                                                                    ));
        }
		if (fileSaveName.isEmpty())
			return;
		if (!export_list2file(result,fileSaveName,fileOpenName))
		{
            v3d_msg("fail to write the output swc/eswc file.");
			return;
		}
	}
	return;
}
void sort_batch_menu(V3DPluginCallback2 &callback, QWidget *parent)
{
    //choose a directory that contain swc files
    QString qs_dir_swc = QFileDialog::getExistingDirectory(parent,
                                                           QString(QObject::tr("Choose the directory that contains files to be soma-corrected"))
                                                           );
    if(qs_dir_swc.size()==0){
        v3d_msg("Empty input folder.\n Let the developer know if you see this message.");
        return;
    }
    if(!qs_dir_swc.endsWith("/")){qs_dir_swc = qs_dir_swc + "/";}
    //choose output directory
    QString qs_dir_output = QFileDialog::getExistingDirectory(parent,
                                                              QString(QObject::tr("Choose the output directory."))
                                                              );
    if((qs_dir_output.size()>0) && (!qs_dir_output.endsWith("/"))){qs_dir_output = qs_dir_output + "/";}

    // Specify parameters
    V3DLONG rootid = 0;
    double thres;
    bool ok;

    thres = QInputDialog::getDouble(0, "Would you like to set a threshold for the newly generated link?","threshold:(If you select 'cancel', all the points will be connected automated; If you set '0', no new link will be generated)",0,0,2147483647,1,&ok);
    if (!ok)
        thres = VOID_VALUE;

    sort_batch(qs_dir_swc, qs_dir_output, thres);
    return;
}

void sort_batch(QString input_dir, QString output_dir, double thres_dist){

    bool skip_existing = true;

    qDebug()<<"Welcome to use the \"sort_swc_batch\" plugin";
    // Step 1: Find swc files under a folder;
    QDir dir(input_dir);
    QStringList qsl_filelist, qsl_filters;
    qsl_filters << "*.swc" << "*.eswc";

    foreach(QString file, dir.entryList(qsl_filters,QDir::Files))
    {
        qsl_filelist+=file;
    }

    if(qsl_filelist.size()==0)
    {
        v3d_msg("Cannot find swc files in the given directory!\nTry another diretory");
        return;
    }

    // Step 2: Specify output folder
    if(output_dir.size()==0){output_dir = input_dir + "Soma_Corrected/";}
    qDebug()<<QString("Output folder is %1.").arg(output_dir);
    if(!QDir::current().mkdir(output_dir)){
//        v3d_msg(QString("Cannot create dir \"%1\" or \"%2\" already exists. Please double check.").arg(output_dir).arg(output_dir));
    }

    // Step 3: Perform swc_sorting through a loop
    for(int i=0; i<qsl_filelist.size(); i++){
        // input swc and apo
        int suffix_len = 5;
        if(qsl_filelist.at(i).endsWith(".swc") || qsl_filelist.at(i).endsWith(".SWC")){suffix_len = 4;}
        QString qs_input_swc = input_dir + qsl_filelist.at(i);

        qDebug()<<qs_input_swc;
        NeuronTree nt = readSWC_file(qs_input_swc);
        QList<NeuronSWC> neuron = nt.listNeuron;
        QList<NeuronSWC> result;
        QString qs_output_swc = output_dir + qsl_filelist.at(i);
        if(qs_input_swc.endsWith(".eswc",Qt::CaseInsensitive))
        {
            qs_output_swc = qs_output_swc+QString("_sorted.eswc");
        }
        else
        {
            qs_output_swc = qs_output_swc+QString("_sorted.swc");
        }
        if((fexists(qs_output_swc)) && (skip_existing)){
            continue;
        }
        if (SortSWC(neuron, result ,VOID_VALUE, thres_dist))
        {
            if (!export_list2file(result, qs_output_swc, qs_input_swc))
            {
                v3d_msg("fail to write the output swc/eswc file.");
                return;
            }
        }
    }
    return;
}

void sort_toolbox(const V3DPluginArgList & input)
{
	vaa3d_neurontoolbox_paras * paras = (vaa3d_neurontoolbox_paras *)(input.at(0).p);
	NeuronTree nt(paras->nt);
	QList<NeuronSWC> neuron = nt.listNeuron;

	V3DLONG rootid;
	V3DLONG thres;
	bool ok;
    rootid = QInputDialog::getInt(0, "Would you like to specify new root number?","New root number:(If you select 'cancel', the first root in file is set as default)",1,1,neuron.size(),1,&ok);
	if (!ok)
        rootid = VOID_VALUE;
	thres = QInputDialog::getDouble(0, "Would you like to set a threshold for the newly generated link?","threshold:(If you select 'cancel', all the points will be connected automated; If you set '0', no new link will be generated)",0,0,2147483647,1,&ok);
	if (!ok)
        thres = VOID_VALUE;

	QList<NeuronSWC> result;
	QString fileOpenName = nt.file;
	if (SortSWC(neuron, result ,rootid, thres))
	{
		QString fileDefaultName = fileOpenName+QString("_sorted.swc");
		//write new SWC to file
		QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
				fileDefaultName,
				QObject::tr("Supported file (*.swc)"
					";;Neuron structure	(*.swc)"
					));
		if (fileSaveName.isEmpty())
			return;
		if (!export_list2file(result,fileSaveName,fileOpenName))
		{
			v3d_msg("fail to write the output swc file.");
			return;
		}
	}
	return;
}

bool sort_func(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"==========Welcome to sort_swc function============="<<endl;
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	vector<char*>* outlist = NULL;
	vector<char*>* paralist = NULL;

    double thres = VOID_VALUE;
	QList<NeuronSWC> neuron, result;
	bool hasPara, hasOutput;
	if (input.size()==1) 
	{
		cout<<"No new parameter specified.\n";
		hasPara = false;
	}
	else {
		hasPara = true;
		paralist = (vector<char*>*)(input.at(1).p);
	}

	if (inlist->size()!=1)
	{
		cout<<"You must specify 1 input file!"<<endl;
		return false;
	}

	if (output.size()==0){
		cout<<"No output file specified.\n";
		hasOutput = false;
	}
	else {
		hasOutput = true;
		if (output.size()>1)
		{
			cout<<"You have specified more than 1 output file.\n";
			return false;
		}
		outlist = (vector<char*>*)(output.at(0).p);
	}

    V3DLONG rootid = VOID_VALUE;
	if (hasPara)
	{
		if (paralist->size()==0)
		{
			cout<< "Threshold not set: All points will connected automatically." << endl;
			cout<<"No root ID is specified: by default will use the first root in the file."  <<endl;
            rootid = VOID_VALUE;
            thres = VOID_VALUE;
		}
		else if (paralist->size() >= 1)
		{
			thres = atof(paralist->at(0));
			cout<<"threshold: "<<thres<<endl;
			if (paralist->size() ==2 )
			{
				rootid = atoi(paralist->at(1));
				cout<<"root id: "<<rootid<<endl;
			}
			else if (paralist->size() >2)
			{
				cout<<"Illegal parameter list."<<endl;
				return false;
			}
		}

	}


	QString fileOpenName = QString(inlist->at(0));
	QString fileSaveName;
	if (hasOutput) 
	{
		cout<<"outfile: "<<outlist->at(0)<<endl;
		fileSaveName = QString(outlist->at(0));
	}
	else 
	{
		fileSaveName = fileOpenName+QString("_sorted.swc");
	}

    if (fileOpenName.endsWith(".swc") || fileOpenName.endsWith(".SWC") || fileOpenName.endsWith(".eswc") || fileOpenName.endsWith(".ESWC"))
		neuron = readSWC_file(fileOpenName).listNeuron;
	else if (fileOpenName.endsWith(".ano") || fileOpenName.endsWith(".ANO"))
	{
		P_ObjectFileType linker_object;
		if (!loadAnoFile(fileOpenName,linker_object))
		{
			cout<<"Error in reading the linker file."<<endl;
			return false;
		}
		QStringList nameList = linker_object.swc_file_list;
		V3DLONG neuronNum = nameList.size();
		vector<QList<NeuronSWC> > nt_list;
		for (V3DLONG i=0;i<neuronNum;i++)
		{
			QList<NeuronSWC> tmp = readSWC_file(nameList.at(i)).listNeuron;
			nt_list.push_back(tmp);
		}
		if (!combine_linker(nt_list, neuron))
		{
			cout<<"Error in combining neurons."<<endl;
			return false;
		}
	}
	else
	{
		cout<<"The file type you specified is not supported."<<endl;
		return false;
	}

	if (!SortSWC(neuron, result , rootid, thres))
	{
		cout<<"Error in sorting swc"<<endl;
		return false;
	}
	if (!export_list2file(result, fileSaveName, fileOpenName))
	{
		cout<<"Error in writing swc to file"<<endl;
		return false;
	}
	return true;
}

void printHelp(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3d_msg("(version 2.0) Reallocate id for the points in 1 neuron (.swc) or mutiple neurons (.ano). Set a new root and sort the SWC file into a new order, where the newly set root has the id of 1, and the parent's id is less than its child's. If the link threshold is set, the plugin will only generate nearest link shorter than that threshold. by Yinan Wan 2012-02-01");
}

void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"(version 2.0) Reallocate id for the points in 1 neuron (.swc) or mutiple neurons (.ano). Set a new root and sort the SWC file into a new order, where the newly set root has the id of 1, and the parent's id is less than its child's. If the link threshold is set, the plugin will only generate nearest link shorter than that threshold. by Yinan Wan 2012-02-01"<<endl;
	cout<<"usage:\n";
	cout<<"-f<func name>:\t\t sort_swc\n";
	cout<<"-i<file name>:\t\t input .swc or .ano file\n";
	cout<<"-o<file name>:\t\t (not required) output sorted swc file. DEFAUTL: 'inputName_sorted.swc'\n";
	cout<<"-p<link threshold, root id>:\t (not required) the threshold of link you want the plugin to generate in case the input swc(s) are broken. if you set thres=0, no new link will be generated. DEFAULT: Infinity (all points will be connected automatically); The node id of the root you would like to specify, by default, the first root's id  in the swc will be used.\n";
	cout<<"Demo1:\t ./v3d -x sort_neuron_swc -f sort_swc -i test.swc -o test_sorted.swc -p 0  1 (no new links) \n";
	cout<<"Demo2:\t ./v3d -x sort_neuron_swc -f sort_swc -i test.swc -o test_sorted.swc <-p Inifinity> (Default threshold Infinity:all points will be connected automatically, use the default first root as the root id) \n";

}


