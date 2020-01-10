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
		<< tr("cut_neuron_soma_branch")
		<< tr("assemble_neuron_tree")
		<< tr("build_branch_dataset")
		<< tr("display_generated_branch")
		//<< tr("collect_nfss_in_folder")
		<< tr("about");
}

QStringList NeuronFactorTreePlugin::funclist() const
{
	return QStringList()
		<< tr("nfss_product_pipeline")
		//<< tr("nfss_collect_pipeline")
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
	else if (menu_name == tr("cut_neuron_soma_branch"))
	{
		cut_neuron_soma_branch();
	}
	else if (menu_name == tr("assemble_neuron_tree"))
	{
		assemble_neuron_tree();
	}
	else if (menu_name == tr("build_branch_dataset"))
	{
		build_branch_dataset();
	}
	else if (menu_name == tr("display_generated_branch"))
	{
		display_generated_branch();
	}
	/*else if (menu_name == tr("collect_nfss_in_folder"))
	{
		collect_nfss_in_folder();
	}*/
	else if (menu_name == tr("about"))
	{
		v3d_msg(tr("V3D Plugin NeuronFactor Tree by Longfei Li!\n"
				   "Menu items:\n"
		           "    generate_neuron_factor_tree : transform a neuron swc to neuronFactor tree.\n"
			       "        input:neuron.swc, output:neuron.nft.\n"
		           "    serializate_neuron_factor_tree : transform a neuronFactor tree to enuronFactor sequences.\n"
		           "        input:neuron_factor_tree.nft, output:neuron_factor_tree.nfss.\n"));
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
	/*else if (func_name == tr("nfss_collect_pipeline"))
	{
		return nfss_collect_pipeline(input, output);
	}*/
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
	QStringList filenames = QFileDialog::getOpenFileNames(0, 0, TESTDIR, "Supported file (*.swc; *.eswc)" ";;Neuron structure(*.swc; *.eswc)", 0, 0);

	if (filenames.isEmpty())
	{
		v3d_msg("You don't open any SWC file!");
		return;
	}

	for (int i = 0; i < filenames.size(); i++)
	{
		NeuronTree neuron = readSWC_file((filenames[i]));
		V3DLONG neuron_siz = neuron.listNeuron.size();

		NeuronFactorTree nft = create_neuron_factor_tree(neuron, true);//create a neuron factor tree;

		QString neuron_path = neuron.file;
		QString nft_output_path = neuron_path.mid(0, neuron_path.lastIndexOf(".")) + ".nft";
		save_neuron_factor_tree(nft, nft_output_path);//save it;
	}
	

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
		v3d_msg("You don't open any .swc file!");
		return;
	}

	for (int i = 0; i < filenames.size(); i++)
	{
		NeuronFactorTree nft;
		nft = read_neuron_factor_tree(filenames[i]);

		NeuronFactorSequences nfseqs = serialization(nft);

		NeuronFactorSequences nfseqs_n;
		//neuron_factor_sequences_normalization(nfseqs, nfseqs_n);

		QString output_path = filenames[i].mid(0, filenames[i].lastIndexOf(".")) + ".nfss";
		save_neuron_factor_sequences(nfseqs, nft.neuron_path, output_path);
	}

	v3d_msg("Serializating NeuronFactor Tree finish.");
}

//from a .swc, cut it's soma branch part;
void cut_neuron_soma_branch()
{
	QStringList filenames = QFileDialog::getOpenFileNames(0, 0, TESTDIR, "Supported file (*.swc)" ";;Neuron structure(*.swc)", 0, 0);
	if (filenames.isEmpty())
	{
		v3d_msg("You don't open any .swc file!");
		return;
	}

	for each(auto swc_file in filenames)
	{
		NeuronTree neuron = readSWC_file(swc_file);
		SomaBranch sb = get_soma_branch(neuron);
		QString output_path = swc_file.mid(0, swc_file.lastIndexOf(".")) + ".sob";
		save_soma_branch(output_path, sb);
	}

	v3d_msg("Cut soma branch finish.");
}

//from a .gen file assemble a new neuron tree in swc format.
void assemble_neuron_tree()
{
	QStringList filenames = QFileDialog::getOpenFileNames(0, 0, TESTDIR, "Supported file (*.gen)" ";;Generated squences file(*.gen)", 0, 0);
	if (filenames.isEmpty())
	{
		v3d_msg("You don't open any .gen file!");
		return;
	}

	for each(auto gen_file in filenames)
	{
		assemble_neuron_tree_from(gen_file);
	}
	//QString gen_file = filenames[0];
}
void build_branch_dataset()
{
	QStringList filenames = QFileDialog::getOpenFileNames(0, 0, TESTDIR, "Supported file (*.swc; *.eswc)" ";;Neuron structure(*.swc; *.eswc)", 0, 0);

	if (filenames.isEmpty())
	{
		v3d_msg("You don't open any SWC file!");
		return;
	}

	branch_map_t branch_map;
	branch_sequence_vector sequences;

	for (int i = 0; i < filenames.size(); i++)
	{
		cout << qPrintable(filenames[i]) << endl;
		NeuronTree neuron = readSWC_file((filenames[i]));
		collect_branches(neuron, i, branch_map, sequences);
	}
	cout << "Branches collected." << endl;
	 //Normalization.
	int virtual_id = filenames.size();// A virtual id for new cutting-created branch id.
	branch_data_normalization(branch_map, sequences, virtual_id);

	QString dataset_output_path = filenames[0].mid(0, filenames[0].lastIndexOf("/")) + "/branch_dataset.brd";
	if(save_global_branch_dataset(dataset_output_path, branch_map, sequences))
		v3d_msg("Build branch dataset finish.");
	else
		v3d_msg("Building branch dataset failed.");

	QString branch_code_path = filenames[0].mid(0, filenames[0].lastIndexOf("/")) + "/branch_code.bcd";
	if (save_global_branch_code(branch_code_path, branch_map, sequences))
		v3d_msg("Build branch codes finish.");
	else
		v3d_msg("Building branch codes failed.");
}

void display_generated_branch()
{
	QStringList filenames = QFileDialog::getOpenFileNames(0, 0, TESTDIR, "Supported file (*.br)" ";;generated branch structure(*.br)", 0, 0);

	if (filenames.isEmpty())
	{
		v3d_msg("You don't open any branch file!");
		return;
	}

	for (int i = 0; i < filenames.size(); i++)
	{
		Branch branch;
		QString pic_save_path = filenames[i].mid(0, filenames[i].lastIndexOf(".")) + "_display.bmp";
		QString swc_save_path = filenames[i].mid(0, filenames[i].lastIndexOf(".")) + "_display.swc";
		read_branch_from_br(filenames[i], branch);
		//save_single_branch_picture(branch, pic_save_path);
		save_single_branch_swc(branch, swc_save_path);
	}

	v3d_msg("Display branch picutures generated.");
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
	save_neuron_factor_sequences(nfss, nft.neuron_path, nfss_path);

	cout << "==================nfss product pipeline finishes============================" << endl;
}

void print_help() 
{
	cout << "================NeuronFactor Tree plugin help=====================\n" 
			"Usage : \n"
		    "vaa3d.exe /x neuron_factor_tree /f nfss_product_pipeline /i input.swc : from a swc create a nfss file in the same dir with original swc file.\n" << endl;
		    //"vaa3d.exe /x neuron_factor_tree /f nfss_product_pipeline /p input.nfss : collect a nfss file to the file [path(input.nfss)../../collected_features.txt]. \n"<< endl;
}