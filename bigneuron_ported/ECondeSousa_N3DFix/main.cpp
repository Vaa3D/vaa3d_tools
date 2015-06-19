// mytest.cpp : Defines the entry point for the console application.
//

#include "v3d_message.h"
#include <v3d_interface.h>
#include <vector>
#include <stdio.h>
#include <iostream>
#include "myHeader.h"
#include "Dend_Section.h"
#include "N3Dfix_plugin.h"
#include <ctime>
#include <QtGui>
#include "openSWCDialog.h"


int _main(V3DPluginCallback2 &callback, QWidget *parent){

    /////////////////////////////////////////////////////////////////////////////////////////
    //	Set up global parameters and functions
    /////////////////////////////////////////////////////////////////////////////////////////
    double max_dist = 40;	// distance in um

    double thresh =0.2;		// between 0 and 1

    double step_min = 0.25; 	// adding to the THRESHOLD, this parameter sets the necessity of the artifact to differ by STEP_MIN %
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
    std::vector<double> ARC;
    std::vector<double> DIAM;
    std::vector<double> dydx;
    std::vector<long> ppslope;
    std::vector<long> pnslope;
    std::vector<long> pzslope;

    bool ok = 0;
    step_min = QInputDialog::getDouble(parent, "Input Parameters","Please specify the minimum increase in height!",step_min,0,100,2,&ok);
    if (!ok){
         return 0;
    } printf("\nstep_min = %f\n\n", step_min);
    max_dist = QInputDialog::getDouble(parent, "Input Parameters","Please specify the maximum size distance (through the fiber)\nbetween the first and the last points on the artifact to fix",max_dist,0,10000,0,&ok);
    if (!ok){
         return 0;
    } printf("max_dist = %f\n\n", max_dist);
    thresh = QInputDialog::getDouble(parent,"Input Parameters", "Slope Threshold (the slope of the first point to be considered in the artifact)",thresh,0,100,2,&ok);
    if (!ok){
         return 0;
    } printf("threshold = %f\n\n", thresh);






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
//    printf("Enter the main function\n");
    load_data(x,y,z,tree_id,r,ppid,pid,fileOpenName);
//    printf("Loading Data\n");
//    print_data(x,y,z,tree_id,r,ppid,pid);
//    printf("Printing Data\n");
    find_nodes(ppid, nodes, endpoints, end_sec);
//    printf("Nodes found\n\n\n");
//    printf("\t*\tStart creating tree\n\n\n");
    create_tree(x, y, z, r, ppid, end_sec, Point, n3d, dend, pid,tree_id);
//    printf("Tree successfully created\n");
//    //getchar()
    for(long dend_num = 0;dend_num<dend.size();dend_num++){
//         printf("\n\t*\tdend[%d]\n\n",dend_num);
//        for (unsigned i=0;i<dend[dend_num].size();i++){
//                printf("dend[%d][%d] = [ %f, %f, %f, %f, %f ]\n",dend_num, i,dend[dend_num][i].x,dend[dend_num][i].y,dend[dend_num][i].z,dend[dend_num][i].r,dend[dend_num][i].arc);
//        }
//        printf("\n\n");
        //getchar()
//        printf("working on dend[%d]\n",dend_num);
        locate_and_smooth_bumps(dydx,dend ,preprocessing_flag, dend_num, ppslope,pnslope, pzslope,ARC,DIAM,thresh,max_dist,step_min);
//        printf("dend[%d] is finished\n",dend_num);

    }
    write_data(dend,fileOpenName);



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



    return 0;
}

