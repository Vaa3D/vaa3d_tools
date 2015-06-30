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
* (last update: June 30, 2015)
*/

#include <stdio.h>
#include <iostream>
#include <vector>
#include "myHeader.h"
#include "N3DFix_plugin.h"
#include "v3d_message.h"
#include "Dend_Section.h"
#include <stdio.h>
#include <cerrno>

void load_data(std::vector<float>& x, std::vector<float>& y, std::vector<float>& z,
               std::vector<long>& tree_id, std::vector<float>& r, std::vector<long>& ppid, std::vector<long> &pid, QString &path){
	
	// declare variables and vectors
	FILE * fin;
	float tmp_x, tmp_y, tmp_z, tmp_r;
    long tmp_id, tmp_tid, tmp_ppid;
    char mystring [100000];

    //open file
    fin = fopen(path.toStdString().c_str(), "r");

    // fopen_s(&fin,"C:\\Users\\Eduardo\\Dropbox\\PauloAguiar\\ArtifactRemoval\\reconstructions\\L395_no_axon.swc", "r");
	if (fin == NULL) {
        v3d_msg("Impossible to read data\n",1);
		//return -1;
	}else{
//		printf("\n########################################################################## \n");
//		printf("############################### Commnents: ############################### \n");
//		printf("########################################################################## \n");
        while ( fgets (mystring , 100000, fin) != NULL )
		{
            //pch=strchr(mystring,'#');
			//printf("pch = %s\n",pch);
			//if (pch != NULL){
            if (mystring[0] == '#' || mystring[1] == '#' || mystring[0] == '\n'|| mystring[1] == '\n'){
                //printf("%s",pch);
				//printf("pch = %s\t\t",pch);printf ("found at %d\n\n\n\n",pch-mystring+1);
				//pch=strchr(pch+1,'#');		
//				printf("%s",mystring);
			}else{
                sscanf(mystring, "%ld %ld %f %f %f %f %ld\n", &tmp_id, &tmp_tid, &tmp_x, &tmp_y, &tmp_z, &tmp_r, &tmp_ppid );
                pid.push_back(tmp_id);
                tree_id.push_back( tmp_tid);
				x.push_back( tmp_x );
				y.push_back( tmp_y );
				z.push_back( tmp_z );
                r.push_back( 2 * tmp_r );
                ppid.push_back( tmp_ppid );
            }
		}
		
    }
    fclose( fin );
}


void print_data(std::vector<float>& x, std::vector<float>& y, std::vector<float>& z,std::vector<long>& tree_id,
                std::vector<float>& r, std::vector<long>& ppid,std::vector<long> &pid){

	printf("########################################################################## \n");
	printf("############################## Data Matrix: ############################## \n");
	printf("########################################################################## \n");
	printf("tree_id\tx\t\ty\t\tz\t\tr\t\tppid\n");
	for (unsigned it=0; it<x.size(); it++){
        printf("%d %d\t%f\t%f\t%f\t%f\t%d\n",pid.at(it), tree_id.at(it), x.at(it), y.at(it), z.at(it), r.at(it), ppid.at(it));
	}

	printf("\n\n\n########################################################################## \n");
	printf("############################## # of Points: ############################## \n");
	printf("########################################################################## \n");
	printf("\t\t\tTotal Number of Points = %d\n",x.size());
	printf("########################################################################## \n");
}
void write_data(std::vector< std::vector<struct RawPoints > > &dend, QString &path, QString &fileSaveName){

    // declare variables and vectors
    FILE * fout;
    //open file

    //QString name = "_N3DFix";
    QString dataFile = fileSaveName;//QString dataFile = path.remove(path.size()-4,4)  + name + QLatin1String(".swc");
    fout = fopen(dataFile.toStdString().c_str(), "w");

    if (fout == NULL) {
        v3d_msg("The file was not opened\n",1);
        //return -1;
    }else{
        fprintf (fout, "# This file was generated using N3Dfix plugin\n");
        fprintf (fout, "# Eduardo Conde-Sousa\n");
        fprintf (fout, "# [econdesousa@gmail.com]\n");
        // go through all compartments in neuron and then to all points of the compartment
        long dend_num = 0;
        long point = 0;
        //first compartment is different
        for (unsigned ii =0; ii<1; ii++){
            for (unsigned i=0;i<dend[ii].size();i++){
                fprintf(fout,"%d %d %f %f %f %f %d\n",dend[ii][i].pid,dend[ii][i].tid,dend[ii][i].x,dend[ii][i].y,
                        dend[ii][i].z,dend[ii][i].r/2,dend[ii][i].ppid);
            }
            printf("\n\n");
        }
        //in all other compartments the first point is a duplicate, so as to be removed
        for (unsigned ii =1; ii<dend.size(); ii++){
            for (unsigned i=1;i<dend[ii].size();i++){
                fprintf(fout,"%d %d %f %f %f %f %d\n",dend[ii][i].pid,dend[ii][i].tid,dend[ii][i].x,dend[ii][i].y,
                        dend[ii][i].z,dend[ii][i].r/2,dend[ii][i].ppid);
            }
        }
    }
    fclose( fout );
}

