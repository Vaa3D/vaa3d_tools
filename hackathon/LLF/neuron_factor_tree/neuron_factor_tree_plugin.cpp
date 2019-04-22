/* persistence_diagram_plugin.cpp
 * The main file of plugin persistence_diagram
 * 2018-11-27 by Longfei Li.
 */

#include "neuron_factor_tree_plugin.h"
#include <QtGlobal>

Q_EXPORT_PLUGIN2(neuron_factor_tree, NeuronFactorTreePlugin);

QStringList NeuronFactorTreePlugin::menulist() const
{
	return QStringList()
		<< tr("generate_neuron_factor_tree")
		<< tr("serializate_neuron_factor_tree")
		<< tr("collect_nfss_in_folder")
		<< tr("about");
}

QStringList NeuronFactorTreePlugin::funclist() const
{
	return QStringList()
		<< tr("nfss_product_pipeline")
		<< tr("nfss_collect_pipeline")
		<< tr("help");
}

void NeuronFactorTreePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("generate_neuron_factor_tree"))
	{
		generate_neuron_factor_tree();
	}
	else if (menu_name == tr("serializate_neuron_factor_tree"))
	{
		serializate_neuron_factor_tree();
	}
	else if (menu_name == tr("collect_nfss_in_folder"))
	{
		collect_nfss_in_folder();
	}
	else if (menu_name == tr("about"))
	{
		v3d_msg(tr("V3D Plugin NeuronFactor Tree by Longfei Li!"));
	}
}

bool NeuronFactorTreePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	
	if (func_name == tr("nfss_product_pipeline"))
	{
		return nfss_product_pipeline(input, output);
	}
	else if (func_name == tr("nfss_collect_pipeline"))
	{
		return nfss_collect_pipeline(input, output);
	}
	else
	{
		print_help();
		return true;
	}
	return false;
}
/*****************************************************************************/
/*Domenu funcs;                                                              */
/*****************************************************************************/

//from a neuronTree create a level 1 neuronFactorTree.
void generate_neuron_factor_tree()
{
	QStringList filenames = QFileDialog::getOpenFileNames(0, 0, TESTDIR, "Supported file (*.swc)" ";;Neuron structure(*.swc)", 0, 0);

	if (filenames.isEmpty())
	{
		v3d_msg("You don't open any SWC file!");
		return;
	}

	if(filenames.size()!=1)
	{
		v3d_msg("You've select too much swc files!");
		return;
	}

	NeuronTree neuron = readSWC_file((filenames[0]));
	V3DLONG neuron_siz = neuron.listNeuron.size();

	NeuronFactorTree nft = create_neuron_factor_tree(neuron);//create a neuron factor tree;

	QString neuron_path = neuron.file;
	QString nft_output_path = neuron_path.mid(0, neuron_path.lastIndexOf("."))+".nft";
	save_neuron_factor_tree(nft, nft_output_path);//save it;

	//QString nft_swc_output_path = neuron_path+"nft.swc";//for visulation;
	//save_neuron_factor_tree(nft, nft_swc_output_path);

	v3d_msg("Generating NeuronFactor Tree finish.");
}

//from a neuronFactorTree create a neuron factor sequence.
void serializate_neuron_factor_tree()
{
	QStringList filenames = QFileDialog::getOpenFileNames(0, 0, TESTDIR, "Supported file (*.nft)" ";;Neuron Factor Tree structure(*.nft)", 0, 0);

	if (filenames.isEmpty())
	{
		v3d_msg("You don't open any SWC file!");
		return;
	}

	if(filenames.size()!=1)
	{
		v3d_msg("You've select too much swc files!");
		return;
	}

	NeuronFactorTree nft;
	nft = read_neuron_factor_tree(filenames[0]);

	NeuronFactorSequences nfseqs = serialization(nft);

	NeuronFactorSequences nfseqs_n;
	//neuron_factor_sequences_normalization(nfseqs, nfseqs_n);

	QString output_path = filenames[0].mid(0, filenames[0].lastIndexOf(".")) + ".nfss";
	save_neuron_factor_sequences(nfseqs, output_path);

	v3d_msg("Serializating NeuronFactor Tree finish.");
}

//combine files by their classes under a base dir : base_dir/classes/class_n/.nfss -> base_dir/collected_features.txt;
void collect_nfss_in_folder()
{
	/*
	 *This func collect all the nf values into one file;
	 */
	cout << "Collecting nfss files." << endl;

	QString test_base_dir = QFileDialog::getExistingDirectory(0, "Please select a directory that stores nfss belong to different neuron classes.", TESTDIR);
	int sequence_total_count = 0;//count total sequences;

	//preparing combined file path;
	FILE * fp;
	auto combined_file_path = test_base_dir + "/collected_features.txt";
	fp = fopen(combined_file_path.toLatin1(), "w");
	cout << "Create combined file : " << qPrintable(combined_file_path) << endl;
	if (fp == NULL) v3d_msg(QString("Create file [%1] failed!").arg(combined_file_path));

	QDir dir = QDir(test_base_dir);
	dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	auto dir_list = dir.entryList();
	for each (auto dir in dir_list)
	{
		QString class_name = dir;//get the name of this class;
		QString class_base_path = test_base_dir + "/" + class_name;
		QDir class_base_dir(class_base_path);
		QStringList filters; filters << QString("*.nfss");
		class_base_dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
		class_base_dir.setNameFilters(filters);
		auto fileInfoList = class_base_dir.entryInfoList();//nfss file list;

		cout << "Collectting class : " << qPrintable(class_name) << " begins." << endl;

		for each (QFileInfo file_info in fileInfoList)
		{
			auto file_name = file_info.fileName();
			auto file_path = class_base_path + "/" + file_name;
			collect_nfss(file_path, fp, sequence_total_count, class_name);
		}

		cout << fileInfoList.size() << " files collected for class : " << qPrintable(class_name) << endl;
	}

	fclose(fp);

	v3d_msg(QString("Collect all nfss files at : [%1], sequnces count : [%2].").arg(combined_file_path).arg(sequence_total_count));
}

/*****************************************************************************/
/*Dofunc funcs;                                                              */
/*****************************************************************************/

//dofunc nfss_product_pipeline, ;
bool nfss_product_pipeline(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout << "==================nfss product pipeline begins============================" << endl;;
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	if (inlist->size() != 1)
	{
		cout << "PLEASE SELECT A FILE!!!" << endl;
		print_help();
		return false;
	}
	QString swc_path = inlist->at(0);
	QString nfss_path = swc_path.mid(0, swc_path.lastIndexOf(".")) + ".nfss";

	NeuronTree nt = readSWC_file(swc_path);
	NeuronFactorTree nft = create_neuron_factor_tree(nt);
	NeuronFactorSequences nfss = serialization(nft);
	save_neuron_factor_sequences(nfss, nfss_path);

	cout << "==================nfss product pipeline finishes============================" << endl;
}

//pipeline to collect all the *.nfss under a folder;
bool nfss_collect_pipeline(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	/*
	 *A *.swc files will be trans into *.nfss besides original file then collect to collected_features.txt.
	 *Input a file.
	 */
	
	cout << "==================nfss collect pipeline begins======================" << endl;
	vector<char*>* inlist = (vector<char*>*)(input.at(1).p);

	if (inlist->size() != 1)
	{
		cout << "PLEASE SELECT A FILE!!!" << endl;
		print_help();
		return false;
	}
	
	int sequence_total_count = 0;
	QString path = inlist->at(0);//nfss path;

	FILE * fp;
	QDir dir(path);
	auto strlist = dir.absolutePath().split("/");
	auto class_name = strlist[strlist.size()-2];
	//auto class_name = dir.absoluteFilePath(path).split("/")[-2];
	dir.cdUp(); dir.cdUp();// back to /test;
	QString combined_file_path = dir.absolutePath() + "/collected_features.txt";
	fp = fopen(combined_file_path.toLatin1(), "a");
	cout << "Combine file at : " << qPrintable(combined_file_path) << endl;
	if (fp == NULL) { v3d_msg(QString("Create file [%1] failed!").arg(combined_file_path)); return false; }

	collect_nfss(path, fp, sequence_total_count, class_name);
	cout << "nfss from [" << qPrintable(path) << "] collected, get sequences : "<<sequence_total_count << endl;
	cout << "==================nfss collect pipeline finishes======================" << endl;

	fclose(fp);
}

void print_help() 
{
	cout << "================NeuronFactor Tree plugin help=====================\n" 
			"Usage : \n"
		    "vaa3d.exe /x neuron_factor_tree /f nfss_product_pipeline /i input.swc : from a swc create a nfss file in the same dir with original swc file.\n"
		    "vaa3d.exe /x neuron_factor_tree /f nfss_product_pipeline /p input.nfss : collect a nfss file to the file [path(input.nfss)../../collected_features.txt]. \n"<< endl;
}