/* TreeConsensusBuilder_plugin.cpp
 * Given multiple reconstructions of the same neuron, produces a consensus given a confidence threshold (some proportion of input reconstructions voting for each branch).
 * 2015-11-19 : by Todd Gillette
 */
 
#include "v3d_message.h"
#include <vector>
#include "TreeConsensusBuilder_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(TreeConsensusBuilder, TreeConsensusBuilderPlugin);
 
QStringList TreeConsensusBuilderPlugin::menulist() const
{
	return QStringList() 
		<<tr("Build Composite")
		<<tr("Build Consensus")
        <<tr("Quick Consensus")
        <<tr("Pick a new dataset")
		<<tr("about");
}

QStringList TreeConsensusBuilderPlugin::funclist() const
{
	return QStringList()
		<<tr("build_consensus")
		<<tr("help");
}

void TreeConsensusBuilderPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Build Composite"))
	{
        v3d_msg("building composite.");
        build_composite(callback, parent);
	}
	else if (menu_name == tr("Build Consensus"))
	{
		v3d_msg("building consensus.");
        build_consensus(callback, parent);
	}
    else if (menu_name == tr("Quick Consensus"))
    {
        v3d_msg("building quick consensus.");
    }
	else
	{
		v3d_msg(tr("Given multiple reconstructions of the same neuron, produces a consensus given a confidence threshold (some proportion of input reconstructions voting for each branch).. "
			"Developed by Todd Gillette, 2015-11-19"));
	}
}

bool TreeConsensusBuilderPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("help"))
    {
        printf("\n\n(version 1.0) Resample points in a swc file subject to a fixed step length. Developed by Yinan Wan 2012-03-02\n");
        printf("usage:\n");
        printf("\t-f <function_name>:     build_consensus\n");
        printf("\t-d|--directory <swc_directory> \n");
        printf("\t-o|--output <output_file_name>:  (not required) consensus swc file. DEFAULT: '<swc_directory base>.swc'\n");
        printf("\t-s|--scale <scale>:   1   XY resolution (pixels/micron) \n");
        printf("\t-z|--z-scale <zscale>:   1    Z resolution (pixels/micron)\n");
        printf("\t-e|--euclidean <euclidean threshold>:   3 microns\n");
        printf("\t-a|--angle <angle threshold>:   pi/3 radians\n");
        printf("\t-c|--confidence <branch confidence threshold in #branches or proportion>:   0.2\n");
        printf("\t-r|--rescue <rescue threshold in #branches or proportion>:   0\n");
        printf("Demo: v3d -x consensusbuilder -f build_consensus -d dataset1 -o dataset1consensus\n\n");
    } else if (func_name == tr("build_consensus")){
        build_consensus(input, output);
        return true;
	}
    
	return false;
}

bool new_dataset(V3DPluginCallback2 &callback, QWidget *parent){
    QString dataset_dir = QFileDialog::getExistingDirectory(0, QObject::tr("Select dataset directory"),""));

    // Set chosen directory
    
    builder = CompositeBuilder(dataset_dir.toUtf8().constData());
    
    return true;
}

bool build_consensus(const V3DPluginArgList & input, V3DPluginArgList & output){
    double confidence_threshold = -1, rescue_threshold = 0,
    scale = 1,
    zscale = 1,
    euclidean_threshold = -1,
    angle_threshold = -1;
    bool single_tree = false;

    cout<<"Welcome to build_consensus"<<endl;
    if (output.size() != 1) return false;
    unsigned int Wx=1, Wy=1, Wz=1, ch=1;
    
    if (input.size()>=2)
    {
        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) scale = atod(paras.at(0));
        if(paras.size() >= 2) zscale = atod(paras.at(1));
        if(paras.size() >= 3) loglevel = atoi(paras.at(2));
        if(paras.size() >= 4) euclidean_threshold = atod(paras.at(3));
        if(paras.size() >= 5) angle_threshold = atod(paras.at(4));
        if(paras.size() >= 6) confidence_threshold = atod(paras.at(5));
        if(paras.size() >= 7) rescue_threshold = atod(paras.at(6));
    }
    
    string directory = ((vector<char*> *)(input.at(0).p))->at(0);
    string outfile_orig = ((vector<char*> *)(output.at(0).p))->at(0);
    
    bool arg_error = false;
    if (directory.empty()){
        arg_error = true;
        printf("Argument -i <swc_directory> required\n");
    }
    if (outfile_orig.empty()){
        arg_error = true;
        printf("Argument -o <outfile> required\n");
    }
    if (arg_error)
        exit(0);
    
    bool eswc = outfile_orig.substr(outfile_orig.length() - 4) == "eswc";

    try{
        ConsensusBuilder builder(directory, loglevel);
        printf("Created ConsensusBuilder\n");
        builder.set_match_score_threshold(1);
        builder.set_scale(scale);
        builder.set_z_scale(zscale);
        if (euclidean_threshold > 0){
            builder.set_euclidean_dist_threshold(euclidean_threshold);
        }
        if (angle_threshold > 0){
            builder.set_angle_threshold(angle_threshold);
        }
        
        // Start thread to handle keyboard input
        bool finished = false;
        
        printf("Building consensus\n");
        
        string outfile;
        
        string conf_thresh_str = confidence_threshold > 1 ? to_string((int)confidence_threshold) : to_string(confidence_threshold);
        outfile = outfile_orig;
        if (eswc){
            outfile.replace(outfile.end()-4,outfile.end(),"BranchConfThresh"+conf_thresh_str+".eswc");
            builder.write_consensus_to_eswc(outfile, confidence_threshold, rescue_threshold, !single_tree);
        }else{
            outfile.replace(outfile.end()-3,outfile.end(),"BranchConfThresh"+conf_thresh_str+"_TypeByBranch_Proportion.swc");
            builder.write_consensus_to_swc(outfile, confidence_threshold, BRANCH_CONFIDENCE, PROPORTION, rescue_threshold, !single_tree);
        }

        // Print out the names of the reconstructions that were included
        printf("Reconstructions included in the consensus\n");
        for (string name : builder.get_reconstruction_names()){
            printf("%s\n",name.c_str());
            printf("%i trees\n",builder.get_reconstruction(name)->get_trees().size());
        }
        
    } catch (std::string e){
        printf(e.c_str());
    }

}

bool build_composite(V3DPluginCallback2 &callback, QWidget *parent){
    if (builder.get_builder_state() == INITIALIZED)
        new_dataset(callback, parent);

    if (builder.get_builder_state() > INITIALIZED){
        double scale = QInputDialog::getDouble(parent, "SWC Scale", "Specify the swc scale in proportion to microns:",1,0)
        builder.set_scale(scale);
        builder.build_composite();
        
        v3d_msg("Composite created - consensus and statistics can be generated");
    }

    return true;
}

bool build_consensus(V3DPluginCallback2 &callback, QWidget *parent){
    if (builder.get_builder_state() == INITIALIZED)
        new_dataset(callback, parent);

    BuilderState state = builder.get_builder_state();
    if (state > INITIALIZED){
        if (state < BUILT_COMPOSITE){
            double scale = QInputDialog::getDouble(parent, "SWC Scale", "Specify the swc scale in proportion to microns:",1,0)
            builder.set_scale(scale);
        }
        
        double threshold = QInputDialog::getDouble(parent, "Consensus Confidence Threshold", "Specify the per-branch consensus threshold as a proportion or minimum number of reconstructions:",1,0,builder.get_reconstructions().size());
        builder.build_consensus(threshold));

        v3d_msg("Composite created - consensus and statistics can be generated");
        
        // TODO: Open swc (or eswc) in new window
        
    }
    
    return true;
}
