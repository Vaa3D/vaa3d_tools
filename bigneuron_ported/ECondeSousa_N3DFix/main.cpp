/*
* N3DFix - automatic removal of swelling artifacts in neuronal 2D/3D reconstructions
* last update: Mar 2016
* VERSION 2.0
*
* Authors: Eduardo Conde-Sousa <econdesousa@gmail.com>
*          Paulo de Castro Aguiar <pauloaguiar@ineb.up.pt>
* Date:    Mar 2016
*
* N3DFix v2.0 is described in the following publication (!UNDER REVIEW!)
* Conde-Sousa E, Szucs P, Peng H, Aguiar P - Neuroinformatics, 2016
*
*    Disclaimer
*    ----------
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You can view a copy of the GNU General Public License at
*    <http://www.gnu.org/licenses/>.
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


	
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"====================================================================="<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"                                N3DFix                               "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<" Automatic removal of swelling artifacts in neuronal reconstructions "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"  Authors: Eduardo Conde-Sousa <econdesousa@gmail.com>               "<<std::endl;
    std::cout<<"           Paulo de Castro Aguiar <pauloaguiar@ineb.up.pt>           "<<std::endl;
    std::cout<<"  Date:    Mar 2016                                                  "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<" N3DFix v2.0 is described in the following publication (UNDER REVIEW)"<<std::endl;
    std::cout<<" Conde-Sousa E, Szucs P, Peng H, Aguiar P - Neuroinformatics, 2016   "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"====================================================================="<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<" Disclaimer                                                          "<<std::endl;
    std::cout<<" ----------                                                          "<<std::endl;
    std::cout<<" This program is free software: you can redistribute it and/or modify"<<std::endl;
    std::cout<<" it under the terms of the GNU General Public License as published by"<<std::endl;
    std::cout<<" the Free Software Foundation, either version 3 of the License, or   "<<std::endl;
    std::cout<<" any later version.                                                  "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<" This program is distributed in the hope that it will be useful,     "<<std::endl;
    std::cout<<" but WITHOUT ANY WARRANTY; without even the implied warranty of      "<<std::endl;
    std::cout<<" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        "<<std::endl;
    std::cout<<" GNU General Public License for more details.                        "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
	std::cout<<" You can view a copy of the GNU General Public License at            "<<std::endl;
    std::cout<<" <http://www.gnu.org/licenses/>.                                     "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"====================================================================="<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;



    /////////////////////////////////////////////////////////////////////////////////////////
    //	Set up global parameters and functions
    /////////////////////////////////////////////////////////////////////////////////////////
    double bump_slope = 0.10;   // Bump Slope: given the ith point, we set
                                // slope(i)=(DIAM(i)-DIAM(i-1))/(ARC(i)-ARC(i-1)) and mark the
                                // i^th point as potential bump if abs(slope(i))>BUMP_SLOPE
                                // *not a user-define-parameter*
                                // UNITS:   none
                                // RANGE:   [0 ; 1]
                                // DEFAULT: 0.10

    double bump_rnorm = 0.25;	// Bump Normalized Radius Change: it sets the normalized radius trigger that marks
                                // all points with radius significantly different from baseline.
                                // UNITS:   none
                                // RANGE:   ]0 ; inf[
                                // DEFAULT: 0.25

    double rmin = 0.1;          // Minimum Radius: it refers to the minimum radius that a fiber
                                // could present.
                                // It is used to correct points with "zero" radius.
                                // UNITS:   microns
                                // RANGE:   ]0 ; inf[
                                // DEFAULT: 0.1;


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
    std::vector<double> baseline;
    std::vector<double> weights;


    bool ok = 0;
    bump_rnorm = QInputDialog::getDouble(parent, "Input Parameters","bump normalized radius change - threshold for radius change (normalized to baseline), signaling potential artifacts [0.25]",bump_rnorm,0,100,2,&ok);
    if (!ok){
         return 0;
    }

    rmin = QInputDialog::getDouble(parent, "Input Parameters","minimum fiber radius (in um) - used to correct points with \"zero\" radius. [default: 0.1]",rmin,0,100,2,&ok);
    if (!ok){
         return 0;
    }



    std::cout<<"======================================================"<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                    Parameters:                       "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<" Bump Normalized Radius Change: "<<bump_rnorm          <<std::endl;
    std::cout<<" Minimum Radius:                "<<rmin                <<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"======================================================"<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;



    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return 0;


    QList<NeuronSWC> result;
    QString fileOpenName = openDlg->file_name;



    clock_t begin = clock();

    load_data(x,y,z,tree_id,r,ppid,pid,fileOpenName);
    find_nodes(tree_id, ppid, nodes, endpoints, end_sec, soma);
    create_tree(x, y, z, r, ppid, end_sec, Point, n3d, dend, pid,tree_id, soma);

    //Changed
    std::vector< std::vector<struct RawPoints > > dend_original = dend;
    //Changed

    QString name = "_N3DFix";
    QString fileSaveName = fileOpenName.remove(fileOpenName.size()-4,4)  + name + QLatin1String(".swc");


    QString fileReport = fileSaveName;
    fileReport =  fileReport.remove(fileReport.size()-4,4) + QLatin1String("_report_");
    fileReport.append( QString::number(bump_rnorm * 100));
    fileReport.append( "_");
    fileReport.append( QString::number(rmin * 100));
    fileReport.append( ".txt");

    std::cout<<"======================================================"<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                    output files:                     "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<" swc file:                                            "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<< fileSaveName.toStdString().c_str()                     <<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<" report file:                                         "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<< fileReport.toStdString().c_str()                       <<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"======================================================"<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;


    FILE * fout;
    fout = fopen(fileReport.toStdString().c_str(), "w");//err = fopen_s(&fout,dataFile.toStdString().c_str(), "w");

    if (fout == NULL) {//if (err != 0) {
        v3d_msg("The file was not opened\n",1);
        //return -1;
    }else{
        fprintf (fout, "########################################\n");
        fprintf (fout, "########################################\n");
        fprintf (fout, "#                     N3DFix\n");
        fprintf (fout, "########################################\n");
        fprintf (fout, "########################################\n");
        fprintf (fout, "#\n");
        fprintf (fout, "# This file was generated using N3Dfix plugin\n");
        fprintf (fout, "#\n");
        fprintf (fout, "# Eduardo Conde-Sousa <econdesousa@gmail.com>\n");
        fprintf (fout, "# Paulo Aguiar        <pauloaguiar@ineb.up.pt>\n");
        fprintf (fout, "#\n");
        fprintf (fout, "# N3DFix v2.0 is described in the following publication (!UNDER REVIEW!)");
		fprintf (fout, "# Conde-Sousa E, Szucs P, Peng H, Aguiar P - Neuroinformatics, 2016");
        fprintf (fout, "#\n");
        fprintf (fout, "#\n");
        fprintf (fout, "#\n");
        fprintf (fout, "#Parameters:\n");
        fprintf (fout, "#\n");
        fprintf (fout, "# Bump Normalized Radius Change:\n");
        fprintf (fout, "#               %d\n",bump_rnorm);
        fprintf (fout, "#\n");
        fprintf (fout, "# Minimum Radius:\n");
        fprintf (fout, "#               %f\n",rmin);
        fprintf (fout, "#\n");
        fprintf (fout, "#\n");
        fprintf (fout, "#\n");
        fprintf (fout, "# REPORT:\n");
        fprintf (fout, "#Section\tn3d()[um]\tarc3d()[um]\told_radius[um]\tnew_radius[um]");
        fprintf (fout, "\tslope\trnorm\tbaseline[um]\toutlier_flag\tcorrection_flag\n");
        fclose(fout);
    }


    for(long dend_num = 0;dend_num<dend.size();dend_num++)
    {
        // Don't consider soma.
        // If file's "tree ID" field was erroneously written/created may cause some problems
        if(dend.at(dend_num).at( dend.at(dend_num).size()-1 ).tid != 1){
            N3DFix_v2(dend ,dend_num, ARC, DIAM, baseline ,weights, bump_rnorm, rmin, bump_slope, fileReport );
        }

    }
    write_data(dend_original,dend,fileOpenName,fileSaveName);

//    for(long dend_num = 0;dend_num<dend.size();dend_num++){
//        printf("dend[%d]\n",dend_num);
//        for(long ii = 0;ii<dend[dend_num].size();ii++){
//            if(dend[dend_num][ii].pid>1100 &&dend[dend_num][ii].pid<1300){
//                printf("%d\t%g\t%g\t%g\n",dend_original[dend_num][ii].pid,dend_original[dend_num][ii].arc,dend_original[dend_num][ii].r,dend[dend_num][ii].r);
//            }
//        }
//        printf("\n\n");
//    }


    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    std::cout<<"======================================================"<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    printf("\t*\tElapsed Time = %.1f secs\n",elapsed_secs);
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"======================================================"<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;

    return 0;
}


bool N3DFix_func(const V3DPluginArgList & input, V3DPluginArgList & output){
    clock_t begin = clock();

    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"====================================================================="<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"                                N3DFix                               "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<" Automatic removal of swelling artifacts in neuronal reconstructions "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"  Authors: Eduardo Conde-Sousa <econdesousa@gmail.com>               "<<std::endl;
    std::cout<<"           Paulo de Castro Aguiar <pauloaguiar@ineb.up.pt>           "<<std::endl;
    std::cout<<"  Date:    Mar 2016                                                  "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<" N3DFix v2.0 is described in the following publication (UNDER REVIEW)"<<std::endl;
    std::cout<<" Conde-Sousa E, Szucs P, Peng H, Aguiar P - Neuroinformatics, 2016   "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"====================================================================="<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<" Disclaimer                                                          "<<std::endl;
    std::cout<<" ----------                                                          "<<std::endl;
    std::cout<<" This program is free software: you can redistribute it and/or modify"<<std::endl;
    std::cout<<" it under the terms of the GNU General Public License as published by"<<std::endl;
    std::cout<<" the Free Software Foundation, either version 3 of the License, or   "<<std::endl;
    std::cout<<" any later version.                                                  "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<" This program is distributed in the hope that it will be useful,     "<<std::endl;
    std::cout<<" but WITHOUT ANY WARRANTY; without even the implied warranty of      "<<std::endl;
    std::cout<<" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        "<<std::endl;
    std::cout<<" GNU General Public License for more details.                        "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
	std::cout<<" You can view a copy of the GNU General Public License at            "<<std::endl;
    std::cout<<" <http://www.gnu.org/licenses/>.                                     "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"====================================================================="<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
    std::cout<<"                                                                     "<<std::endl;
	


    std::vector<char*>* inlist = (std::vector<char*>*)(input.at(0).p);
    std::vector<char*>* outlist = NULL;
    std::vector<char*>* paralist = NULL;

    double bump_slope = 0.10;   // Bump Slope: given the ith point, we set
                                // slope(i)=(DIAM(i)-DIAM(i-1))/(ARC(i)-ARC(i-1)) and mark the
                                // i^th point as potential bump if abs(slope(i))>BUMP_SLOPE
                                // *not a user-define-parameter*
                                // UNITS:   none
                                // RANGE:   [0 ; 1]
                                // DEFAULT: 0.10

    double bump_rnorm = 0.25;	// Bump Normalized Radius Change: it sets the normalized radius trigger that marks
                                // all points with radius significantly different from baseline.
                                // UNITS:   none
                                // RANGE:   ]0 ; inf[
                                // DEFAULT: 0.25

    double rmin = 0.1;          // Minimum Radius: it refers to the minimum radius that a fiber
                                // could present.
                                // It is used to correct points with "zero" radius.
                                // UNITS:   microns
                                // RANGE:   ]0 ; inf[
                                // DEFAULT: 0.1;



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
    std::vector<double> baseline;
    std::vector<double> weights;


    QList<NeuronSWC> neuron, result;
    bool hasPara, hasOutput;
    if (input.size()==1)
    {
        std::cout<<"\n\n\n\n\nN3DFix will use default parameters.\n\n\n\n\n";
        std::cout<<" Bump Normalized Radius Change: "<<bump_rnorm        <<std::endl;
        std::cout<<" Minimum Radius:                "<<rmin              <<std::endl;
        std::cout<<"                                                    "<<std::endl;
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
        if (paralist->size()<=1)
        {
            bump_rnorm = 0.25;
            rmin = 0.1;
        }
        else if (paralist->size()==2)
        {
            bump_rnorm = atof(paralist->at(0));
            rmin = atof(paralist->at(1));




            std::cout<<"======================================================"<<std::endl;
            std::cout<<"                                                      "<<std::endl;
            std::cout<<"                    Parameters:                       "<<std::endl;
            std::cout<<"                                                      "<<std::endl;
            std::cout<<" Bump Normalized Radius Change: "<<bump_rnorm          <<std::endl;
            std::cout<<" Minimum Radius:                "<<rmin                <<std::endl;
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
        fileSaveName = QString(outlist->at(0));
    }
    else
    {
        fileSaveName = fileOpenName+QString("_N3DFix.swc");
    }





    QString fileReport = fileSaveName;
    fileReport =  fileReport.remove(fileReport.size()-4,4) + QLatin1String("_report_");    
    fileReport.append( QString::number(bump_rnorm * 100));
    fileReport.append( "_");
    fileReport.append( QString::number(rmin * 100));
    fileReport.append( ".txt");

    std::cout<<"======================================================"<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                    output files:                     "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<" swc file:                                            "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<< fileSaveName.toStdString().c_str()                     <<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<" report file:                                         "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<< fileReport.toStdString().c_str()                       <<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"======================================================"<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;
    std::cout<<"                                                      "<<std::endl;


    FILE * fout;
    fout = fopen(fileReport.toStdString().c_str(), "w");//err = fopen_s(&fout,dataFile.toStdString().c_str(), "w");

    if (fout == NULL) {//if (err != 0) {
        v3d_msg("The file was not opened\n",1);
        //return -1;
    }else{
        fprintf (fout, "########################################\n");
        fprintf (fout, "########################################\n");
        fprintf (fout, "#                     N3DFix\n");
        fprintf (fout, "########################################\n");
        fprintf (fout, "########################################\n");
        fprintf (fout, "#\n");
        fprintf (fout, "#\n");
        fprintf (fout, "# This file was generated using N3Dfix plugin\n");
        fprintf (fout, "#\n");
        fprintf (fout, "# Eduardo Conde-Sousa <econdesousa@gmail.com>\n");
        fprintf (fout, "# Paulo Aguiar        <pauloaguiar@ineb.up.pt>\n");
        fprintf (fout, "#\n");
        fprintf (fout, "# N3DFix v2.0 is described in the following publication (!UNDER REVIEW!)");
		fprintf (fout, "# Conde-Sousa E, Szucs P, Peng H, Aguiar P - Neuroinformatics, 2016");
        fprintf (fout, "#\n");
        fprintf (fout, "#\n");
        fprintf (fout, "#\n");
        fprintf (fout, "#Parameters:\n");
        fprintf (fout, "#\n");
        fprintf (fout, "# Bump Normalized Radius Change:\n");
        fprintf (fout, "#               %d\n",bump_rnorm);
        fprintf (fout, "#\n");
        fprintf (fout, "# Minimum Radius:\n");
        fprintf (fout, "#               %f\n",rmin);
        fprintf (fout, "#\n");
        fprintf (fout, "#\n");
        fprintf (fout, "#\n");
        fprintf (fout, "# REPORT:\n");
        fprintf (fout, "#Section\tn3d()[um]\tarc3d()[um]\told_radius[um]\tnew_radius[um]");
        fprintf (fout, "\tslope\trnorm\tbaseline[um]\toutlier_flag\tcorrection_flag\n");
        fclose(fout);
    }



//    printf("Enter the main function\n");
    load_data(x,y,z,tree_id,r,ppid,pid,fileOpenName);

    //    print_data(x,y,z,tree_id,r,ppid,pid);

    find_nodes(tree_id, ppid, nodes, endpoints, end_sec, soma);


    create_tree(x, y, z, r, ppid, end_sec, Point, n3d, dend, pid,tree_id, soma);

    std::vector< std::vector<struct RawPoints > > dend_original = dend;

    for(long dend_num = 0;dend_num<dend.size();dend_num++)
    {
        // Don't consider soma.
        // If file's "tree ID" field was erroneously written/created may cause some problems
        if(dend.at(dend_num).at( dend.at(dend_num).size()-1 ).tid != 1){
            N3DFix_v2(dend ,dend_num, ARC, DIAM, baseline ,weights, bump_rnorm, rmin, bump_slope, fileReport );
        }

    }
//    for(long dend_num = 0;dend_num<dend.size();dend_num++){
//        for(long ii = 0;ii<dend[dend_num].size();ii++){
//            printf("dend[%d]\n",dend_num);
//            printf("%d\t%g\n",dend[dend_num][ii].pid,dend[dend_num][ii].r);
//        }
//        printf("\n\n");
//    }



    write_data(dend_original,dend,fileOpenName,fileSaveName);

//    printf("pid = \t \t tid= \t ppid =\n");
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
    std::cout<<"\n\n\n\n\n\n\n\n\n\n\n\n                                                            "<<std::endl;
    std::cout<<"===================================================================================="<<std::endl;
    std::cout<<"===================================================================================="<<std::endl;
    std::cout<<"===================================================================================="<<std::endl;
    std::cout<<"                                                                                    "<<std::endl;
    std::cout<<"                                                                                    "<<std::endl;
    std::cout<<"====================================== N3DFix ======================================"<<std::endl;
    std::cout<<"                                                                                    "<<std::endl;
    std::cout<<"                                                                                    "<<std::endl;
    std::cout<<"===================================================================================="<<std::endl;
    std::cout<<"===================================================================================="<<std::endl;
    std::cout<<"===================================================================================="<<std::endl;
    std::cout<<"                                                                                    "<<std::endl;
    std::cout<<"                                                                                    "<<std::endl;
    std::cout<<"                                                                                    "<<std::endl;
    std::cout<<" N3DFix - automatic removal of swelling artifacts in neuronal 2D/3D reconstructions "<<std::endl;
	std::cout<<" last update: Mar 2016                                                              "<<std::endl;
	std::cout<<" VERSION 2.0                                                                        "<<std::endl;
	std::cout<<"                                                                                    "<<std::endl;
	std::cout<<"  Authors: Eduardo Conde-Sousa <econdesousa@gmail.com>                              "<<std::endl;
	std::cout<<"           Paulo de Castro Aguiar <pauloaguiar@ineb.up.pt>                          "<<std::endl;
	std::cout<<"  Date:    Mar 2016                                                                 "<<std::endl;
	std::cout<<"                                                                                    "<<std::endl;
	std::cout<<"  N3DFix v2.0 is described in the following publication (!UNDER REVIEW!)            "<<std::endl;
	std::cout<<"  Conde-Sousa E, Szucs P, Peng H, Aguiar P - Neuroinformatics, 2016                 "<<std::endl;
	std::cout<<"                                                                                    "<<std::endl;
    std::cout<<" HOW TO USE:\n";
    std::cout<<"-f <func name> : N3DFix\n";
    std::cout<<"-i <file name> : input *.swc file\n";
    std::cout<<"-o <file name> : (not required) output *.swc file. DEFAULT: 'inputName_N3DFix.swc'\n";
    std::cout<<"-p <parameters>: (not required) normalized radius change [ratio to baseline] DEFAULT: 0.25\n";
    std::cout<<"                                minimum fiber radius [in um] DEFAULT: 0.1\n\n\n";
    std::cout<<"=================================================================================="<<std::endl;
    std::cout<<"=================================================================================="<<std::endl;
    std::cout<<"==================================================================================\n\n\n"<<std::endl;
}
