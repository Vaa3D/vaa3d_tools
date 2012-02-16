/* eswc_converter_func.cpp
 * This is a plugin to convert between swc and eswc.
 * 2012-02-16 : by Yinan Wan
 */

#include <v3d_interface.h>
#include "eswc_converter_func.h"
#include "read_eswc.h"
const QString title = QObject::tr("eswc Converter");
int swc2eswc_io(V3DPluginCallback2 &callback, QWidget *parent)
{
	QString fileOpenName;
	fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
			"",
			QObject::tr("Supported file (*.swc)"
				";;Neuron structure	(*.swc)"
				));
	if(fileOpenName.isEmpty()) 
	{
		v3d_msg("You don't have any file open.");
		return -1;
	}
	
	NeuronTree neuron;
	vector<V3DLONG> segment_id, segment_layer;
	if (fileOpenName.endsWith(".swc") || fileOpenName.endsWith(".SWC"))
	{
		neuron = readSWC_file(fileOpenName);
		segment_id, segment_layer;
		if (!swc2eswc(neuron,segment_id, segment_layer))
		{
			v3d_msg("Cannot convert swc to eswc.\n");
			return -1;
		}
	}
	else {
		v3d_msg("The file type you specified is not supported. Please check.");
		return -1;
	}
	
	QString fileDefaultName = fileOpenName;
	fileDefaultName.chop(3);
	fileDefaultName += QString("eswc");
	//write new SWC to file
	QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
			fileDefaultName,
			QObject::tr("Supported file (*.eswc)"
				";;Extended Neuron structure	(*.eswc)"
				));
	if (!export_eswc(neuron,segment_id,segment_layer,qPrintable(fileOpenName),qPrintable(fileSaveName)))
	{
		v3d_msg("fail to write the output eswc file.");
		return -1;
	}
	return 1;
}

bool swc2eswc_io(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to swc2eswc_io"<<endl;
	return true;
}

int eswc2swc_io(V3DPluginCallback2 &callback, QWidget *parent)
{
	QString fileOpenName;
	fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
			"",
			QObject::tr("Supported file (*.eswc)"
				";;Extended Neuron structure	(*.eswc)"
				));
	if(fileOpenName.isEmpty()) 
	{
		v3d_msg("You don't have any file open.");
		return -1;
	}
	
	NeuronTree neuron;
	vector<V3DLONG> segment_id, segment_layer;
	vector<double> feature;
	if (fileOpenName.endsWith(".eswc") || fileOpenName.endsWith(".ESWC"))
	{
		neuron = read_eswc(segment_id, segment_layer, feature, fileOpenName);
		segment_id, segment_layer;
		if (!swc2eswc(neuron,segment_id, segment_layer))
		{
			v3d_msg("Cannot convert eswc to swc.\n");
			return -1;
		}
	}
	else {
		v3d_msg("The file type you specified is not supported. Please check.");
		return -1;
	}
	int ret = QMessageBox::Yes;
	if (check_eswc(neuron, segment_id, segment_layer)==-1)
	{
		QMessageBox msgBox;
		msgBox.setText("Something is wrong with the eswc file.");
		msgBox.setInformativeText("Do you still want to convert it to swc?");
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		ret = msgBox.exec();
	}
	if (ret==QMessageBox::Cancel)
		return -1;
	QString fileDefaultName = fileOpenName;
	fileDefaultName.chop(4);
	fileDefaultName += QString("swc");
	//write new SWC to file
	QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
			fileDefaultName,
			QObject::tr("Supported file (*.swc)"
				";;Neuron structure	(*.eswc)"
				));
	if (!export_swc(neuron,qPrintable(fileSaveName)))
	{
		v3d_msg("fail to write the output swc file.");
		return -1;
	}
	return 1;
}

bool eswc2swc_io(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to eswc2swc_io"<<endl;
	return true;
}

int check_eswc_io(V3DPluginCallback2 &callback, QWidget *parent)
{
	QString fileOpenName;
	fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
			"",
			QObject::tr("Supported file (*.eswc)"
				";;Extended Neuron structure	(*.eswc)"
				));
	if(fileOpenName.isEmpty()) 
	{
		v3d_msg("You don't have any file open.");
		return -1;
	}
	
	NeuronTree neuron;
	vector<V3DLONG> segment_id, segment_layer;
	vector<double> feature;
	if (fileOpenName.endsWith(".eswc") || fileOpenName.endsWith(".ESWC"))
	{
		neuron = read_eswc(segment_id, segment_layer, feature, fileOpenName);
		segment_id, segment_layer;
		if (!swc2eswc(neuron,segment_id, segment_layer))
		{
			v3d_msg("Cannot convert eswc to swc.\n");
			return -1;
		}
	}
	else {
		v3d_msg("The file type you specified is not supported. Please check.");
		return -1;
	}

	int check_result = check_eswc(neuron, segment_id, segment_layer);
	if (check_result==-1 || check_result==0)
	{
		QMessageBox msgBox;
		if (check_result==-1)
			msgBox.setText("Something is wrong with the eswc file.");
		else
			msgBox.setText("Your eswc is written in default values.");
		msgBox.setInformativeText("Do you still want to overwrite it with a standard one?");
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Cancel);
		if (msgBox.exec()==QMessageBox::Yes)
		{
			swc2eswc(neuron, segment_id, segment_layer);
			export_eswc(neuron, segment_id, segment_layer, qPrintable(fileOpenName), qPrintable(fileOpenName));
		v3d_msg(QString("%1 has been updated!").arg(fileOpenName));
		}
		else return -1;
	}
	else
		v3d_msg("Your eswc file is in correct format!");

	return 1;
}

bool check_eswc_io(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to check_eswc_io"<<endl;
	return true;
}





bool export_eswc(NeuronTree & input, vector<V3DLONG> seg_id, vector<V3DLONG> seg_layer, const char* infile_name, const char* outfile_name)
{
	V3DLONG pntNum = input.listNeuron.size();
	if (seg_id.size()!=pntNum || seg_layer.size()!=pntNum)
	{
		fprintf(stderr, "Cannot export eswc!.\n");
		return false;
	}
	FILE * fp;
	fp = fopen(outfile_name,"w");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR: %s: failed to open file to write!\n",outfile_name);
		return false;
	}
	fprintf(fp, "#ESWC(extended swc) adapted from %s\n",infile_name);
	fprintf(fp, "##n,type,x,y,z,radius,parent,segment_id,segment_layer,feature_value\n");
	for (int i=0;i<pntNum;i++)
	{
		NeuronSWC curr = input.listNeuron[i];
		fprintf(fp,"%d %d %.2f %.2f %.2f %.3f %d %d %d 0\n",curr.n,curr.type,curr.x,curr.y,curr.z,curr.r,curr.pn,seg_id[i],seg_layer[i]);
	//	fprintf(fp,"%d %d %.2f %.2f %.2f %.3f %d\n",curr.n,seg_layer[i],curr.x,curr.y,curr.z,curr.r,curr.pn);
	}
	fclose(fp);
	return true;
}

bool export_swc(NeuronTree & neuron, const char* filename)
{
	FILE * fp;
	fp = fopen(filename,"w");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR: %s: failed to open file to write!\n",filename);
		return false;
	}
	fprintf(fp,"##n,type,x,y,z,radius,parent\n");
	for (int i=0;i<neuron.listNeuron.size();i++)
	{
		NeuronSWC curr = neuron.listNeuron.at(i);
		fprintf(fp,"%d %d %.2f %.2f %.2f %.3f %d\n",curr.n,curr.type,curr.x,curr.y,curr.z,curr.r,curr.pn);
	}
	fclose(fp);
	return true;
}
