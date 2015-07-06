/* Created by:
*
* 				Eduardo Conde-Sousa [econdesousa@gmail.com]
*							and
*				Paulo de Castro Aguiar [pauloaguiar@ineb.up.pt]
*
* on Dez 10, 2014
* to visualize and remove artifacts resulting
* from the 3D reconstruction of dendrites / axons
*
* (last update: July 06, 2015)
*/


#include "v3d_message.h"
#include <v3d_interface.h>
#include <vector>
#include <stdio.h>
#include <iostream>
#include "myHeader.h"
#include "Dend_Section.h"
#include "N3DFix_plugin.h"
#include <ctime>
#include <QtGui>
#include "openSWCDialog.h"


int _main(V3DPluginCallback2 &callback, QWidget *parent){

    /////////////////////////////////////////////////////////////////////////////////////////
    //	Set up global parameters and functions
    /////////////////////////////////////////////////////////////////////////////////////////
    double max_dist = 40;	// distance in um

    double thresh =0.3;		// between 0 and 1

    double step_min = 0.3; 	// adding to the THRESHOLD, this parameter sets the necessity of the artifact to differ by STEP_MIN %
    // from the original diameter (to prevent the existence of two very close spots with different diameters)


    // preprocessing_flag indicates to the system if proc remove_points_with_same_coordinates() has been performed
    bool preprocessing_flag = 0;


    //define vectors to contain data
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
    std::vector<long> tree_id;
    std::vector<long> pid;
    std::vector<float> r;
    std::vector<long> ppid;
    std::vector<long> nodes;
    std::vector<long> endpoints;
    std::vector<long> end_sec;
    struct RawPoints Point;
    std::vector<struct RawPoints > n3d;
    std::vector< std::vector<struct RawPoints > > dend;
    std::vector<long> soma;
    std::vector<double> ARC;
    std::vector<double> DIAM;
    std::vector<double> dydx;
    std::vector<long> ppslope;
    std::vector<long> pnslope;
    std::vector<long> pzslope;


    bool ok = 0;
    max_dist = QInputDialog::getDouble(parent, "Input Parameters","Please specify the maximum size distance (through the fiber)\nbetween the first and the last points on the artifact to fix",max_dist,0,10000,0,&ok);
    if (!ok){
         return 0;
    } printf("max_dist = %f\n\n", max_dist);

    thresh = QInputDialog::getDouble(parent,"Input Parameters", "Slope Threshold (the slope of the first point to be considered in the artifact)",thresh,0,100,2,&ok);
    if (!ok){
         return 0;
    } printf("threshold = %f\n\n", thresh);

    step_min = QInputDialog::getDouble(parent, "Input Parameters","Please specify the minimum increase in height!",step_min,0,100,2,&ok);
    if (!ok){
         return 0;
    } printf("\nstep_min = %f\n\n", step_min);





    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return 0;

//    NeuronTree nt = openDlg->nt;
//    QList<NeuronSWC> neuron = nt.listNeuron;
//    //std::cout << "Path = " << qPrintable(neuron) << std::endl;
    QList<NeuronSWC> result;
    QString fileOpenName = openDlg->file_name;
    std::cout << "\n\n\n\t*\tPath = " << qPrintable(fileOpenName) << std::endl;


    clock_t begin = clock();
    load_data(x,y,z,tree_id,r,ppid,pid,fileOpenName);
    find_nodes(tree_id, ppid, nodes, endpoints, end_sec, soma);
    create_tree(x, y, z, r, ppid, end_sec, Point, n3d, dend, pid,tree_id, soma);


    for(long dend_num = 0;dend_num<dend.size();dend_num++)
    {
        // Don't consider soma.
        // If file's "tree ID" field was erroneously written/created may cause some problems
        if(dend.at(dend_num).at( dend.at(dend_num).size()-1 ).tid != 1){
            locate_and_smooth_bumps(dydx,dend ,preprocessing_flag, dend_num, ppslope,pnslope, pzslope,ARC,DIAM,thresh,max_dist,step_min);
        }

    }

    QString name = "_N3DFix";
    QString fileSaveName = fileOpenName.remove(fileOpenName.size()-4,4)  + name + QLatin1String(".swc");
    write_data(dend,fileOpenName,fileSaveName);


    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    printf("\n\t*\tElapsed Time = %.1f secs\n\n\n",elapsed_secs);



    return 0;
}


bool N3DFix_func(const V3DPluginArgList & input, V3DPluginArgList & output){
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"======================================================"<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                          N3DFix                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<" Created by:                                          "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"    Eduardo Conde-Sousa        [econdesousa@gmail.com]"<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                           and                        "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"    Paulo de Castro Aguiar    [pauloaguiar@ineb.up.pt]"<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<" to visualize and remove artifacts resulting          "<<std::endl;
    std::cout<<" from the 3D reconstruction of dendrites / axons      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"======================================================"<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;



    std::vector<char*>* inlist = (std::vector<char*>*)(input.at(0).p);
    std::vector<char*>* outlist = NULL;
    std::vector<char*>* paralist = NULL;

    double max_dist = 40;	// distance in um

    double thresh =0.3;		// between 0 and 1

    double step_min = 0.3; 	// adding to the THRESHOLD, this parameter sets the necessity of the artifact to differ by STEP_MIN %
    // from the original diameter (to prevent the existence of two very close spots with different diameters)


    // preprocessing_flag indicates to the system if proc remove_points_with_same_coordinates() has been performed
    bool preprocessing_flag = 0;


    //define vectors to contain data
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
    std::vector<long> tree_id;
    std::vector<long> pid;
    std::vector<float> r;
    std::vector<long> ppid;
    std::vector<long> nodes;
    std::vector<long> endpoints;
    std::vector<long> end_sec;
    struct RawPoints Point;
    std::vector<struct RawPoints > n3d;
    std::vector< std::vector<struct RawPoints > > dend;
    std::vector<long> soma;
    std::vector<double> ARC;
    std::vector<double> DIAM;
    std::vector<double> dydx;
    std::vector<long> ppslope;
    std::vector<long> pnslope;
    std::vector<long> pzslope;

    QList<NeuronSWC> neuron, result;
    bool hasPara, hasOutput;
    if (input.size()==1)
    {
        std::cout<<"\n\n\n\n\nN3DFix will use default parameters.\n\n\n\n\n";
        std::cout<<"max_dist: "<<max_dist<<std::endl;
        std::cout<<"threshold: "<<thresh<<std::endl;
        std::cout<<"ratio: "<<step_min<<std::endl;
        hasPara = false;
    }
    else {
        hasPara = true;
        paralist = (std::vector<char*>*)(input.at(1).p);
    }

    if (inlist->size()!=1)
    {
        std::cout<<"You must specify 1 input file!"<<std::endl;
        return false;
    }

    if (output.size()==0){
        std::cout<<"                                                      "<<std::endl;
        std::cout<<"                                                      "<<std::endl;
        std::cout<<"======================================================"<<std::endl;
        std::cout<<"                                                      "<<std::endl;
        std::cout<<                "No output file specified.             "<<std::endl;
        std::cout<<"                                                      "<<std::endl;
        std::cout<<"======================================================"<<std::endl;
        std::cout<<"                                                      "<<std::endl;
        std::cout<<"                                                      "<<std::endl;
        hasOutput = false;
    }
    else {
        hasOutput = true;
        if (output.size()>1)
        {
            std::cout<<"You have specified more than 1 output file.\n";
            return false;
        }
        outlist = (std::vector<char*>*)(output.at(0).p);
    }


    if (hasPara)
    {
        if (paralist->size()<=2)
        {
            max_dist = 40;
            thresh =0.3;
            step_min = 0.3;
        }
        else if (paralist->size()==3)
        {
            max_dist = atof(paralist->at(0));
            thresh = atof(paralist->at(1));
            step_min = atof(paralist->at(2));

            std::cout<<"======================================================"<<std::endl;
            std::cout<<"                                                      "<<std::endl;
            std::cout<<"                    Parameters:                       "<<std::endl;
            std::cout<<"                                                      "<<std::endl;
            std::cout<<"     max_dist:    "<<max_dist                          <<std::endl;
            std::cout<<"    threshold:    "<<thresh                            <<std::endl;
            std::cout<<"        ratio:    "<<step_min                          <<std::endl;
            std::cout<<"                                                      "<<std::endl;
            std::cout<<"======================================================"<<std::endl;
            std::cout<<"                                                      "<<std::endl;
            std::cout<<"                                                      "<<std::endl;
        }
        else
        {
            std::cout<<"Illegal number of parameters."<<std::endl;
            return false;
        }
    }


    QString fileOpenName = QString(inlist->at(0));
    QString fileSaveName;
    if (hasOutput)
    {
        std::cout<<"======================================================"<<std::endl;
        std::cout<<"                                                      "<<std::endl;
        std::cout<<"                       outfile:                       "<<std::endl;
        std::cout<<outlist->at(0)<<std::endl;
        std::cout<<"                                                      "<<std::endl;
        std::cout<<"======================================================"<<std::endl;
        std::cout<<"                                                      "<<std::endl;
        std::cout<<"                                                      "<<std::endl;


        fileSaveName = QString(outlist->at(0));
    }
    else
    {
        fileSaveName = fileOpenName+QString("_N3DFix.swc");
    }


    clock_t begin = clock();
//    printf("Enter the main function\n");
    load_data(x,y,z,tree_id,r,ppid,pid,fileOpenName);

    //    print_data(x,y,z,tree_id,r,ppid,pid);

    find_nodes(tree_id, ppid, nodes, endpoints, end_sec, soma);


    create_tree(x, y, z, r, ppid, end_sec, Point, n3d, dend, pid,tree_id, soma);


    for(long dend_num = 0;dend_num<dend.size();dend_num++)
    {
        // Don't consider soma.
        // If file's "tree ID" field was erroneously written/created may cause some problems
        if(dend.at(dend_num).at( dend.at(dend_num).size()-1 ).tid != 1){
            locate_and_smooth_bumps(dydx,dend ,preprocessing_flag, dend_num, ppslope,pnslope, pzslope,ARC,DIAM,thresh,max_dist,step_min);
        }

    }
    write_data(dend,fileOpenName,fileSaveName);


//    printf("pid = \t \t tid= \t ppid =");
//    for(int i=0;i<pid.size();i++){
//        printf("%d\t%d\t%d \n",pid.at(i),tree_id.at(i),ppid.at(i));
//    }


//    for (unsigned ii =1; ii<dend.size(); ii++){
//        for (unsigned i=1;i<dend[ii].size();i++){
//            printf("dend[%d][%d] = [%d %d %f, %f, %f, %f, %f, %d ]\n",ii, i,dend[ii][i].pid,dend[ii][i].tid,dend[ii][i].x,dend[ii][i].y,
//                   dend[ii][i].z,dend[ii][i].r,dend[ii][i].arc,dend[ii][i].ppid);
//        }
//        printf("\n\n");
//    }


    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
    printf("\n\t*\tElapsed Time = %.1f secs\n\n\n",elapsed_secs);



    return true;
}

void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output){
    std::cout<<"\n\n\n=================================================================================="<<std::endl;
    std::cout<<"=================================================================================="<<std::endl;
    std::cout<<"=================================================================================="<<std::endl;
    std::cout<<"\n\nRemove artifacts resulting from the 3D reconstruction of dendrites/axons."<<std::endl;
    std::cout<<"\n\n\t Eduardo Conde-Sousa (econdesousa@gmail.com) \n\n"<<std::endl;
    std::cout<<"* USAGE:\n";
    std::cout<<"-f <func name> :    N3DFix\n";
    std::cout<<"-i <file name> :    input *.swc file\n";
    std::cout<<"-o <file name> :    (not required) output *.swc file. DEFAUTL: 'inputName_N3DFix.swc'\n";
    std::cout<<"-p <parameters>:    (not required) Artifact Size: maximum size distance (through the fiber) between the first and the last points on the artifact to fix.  DEFAUTL: 40\n";
    std::cout<<"                                   Slope Threshold: the slope of the first point to be considered in the artifact  DEFAUTL: 0.3\n";
    std::cout<<"                                   Step Size: the minimum increase in height.  DEFAUTL: 0.3\n\n\n";
    std::cout<<"=================================================================================="<<std::endl;
    std::cout<<"=================================================================================="<<std::endl;
    std::cout<<"==================================================================================\n\n\n"<<std::endl;
}
