#include "segment_profiling_main.h"
#include "get_sub_block.h"
#include "segment_mean_shift.h"
#include "openSWCDialog.h"
#include "../../../../v3d_external/v3d_main/neuron_editing/neuron_format_converter.h"
#include "../../../../v3d_external/v3d_main/neuron_editing/v_neuronswc.h"
#include "profile_snr.h"

#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define VOID 1000000000
#define dist(a,b) sqrt(((a).x*0.2-(b).x*0.2)*((a).x*0.2-(b).x*0.2)+((a).y*0.2-(b).y*0.2)*((a).y*0.2-(b).y*0.2)+((a).z-(b).z)*((a).z-(b).z))/1000

bool segment_profiling_main(V3DPluginCallback2 &callback,NeuronTree &nt,QString &filename)
{
    //vector<MyMarker*> nt_marker;
    //vector<NeuronSegment*> segment;
    QString path = QFileInfo(".").absolutePath();
    int pos = path.lastIndexOf(QChar('/'));
    //v3d_msg();
    QString path_curr = path.left(pos+1);
    QList<NeuronTree> sketchedNTList;
	double total_length = 0.0,v_l=0.0;
	
	for(int i = 0;i<nt.listNeuron.size();i++){
		NeuronSWC curr = nt.listNeuron.at(i);
		int parent = getParent(i,nt);
		if (parent==VOID)
		{
			continue;
		}
		double distance = dist(curr,nt.listNeuron.at(parent));
		if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
		{
			v_l += distance;
		}
	}

	cout<<"++++++++++++++++++++++++++++++++++"<<v_l<<endl;
    QList<QList<IMAGE_METRICS> > CSVlist;
    V_NeuronSWC_list testVNL = NeuronTree__2__V_NeuronSWC_list(nt);
	//NeuronTree ntree = V_NeuronSWC_list__2__NeuronTree(testVNL)
	
	
    for(int i= 0;i<testVNL.seg.size();i++)
    {
        NeuronTree SS;
        //convert each segment to NeuronTree with one single path
        V_NeuronSWC seg_temp =  testVNL.seg.at(i);
        seg_temp.reverse();
        SS = V_NeuronSWC__2__NeuronTree(seg_temp);
        //append to editable sketchedNTList
        SS.name = "loaded_" + QString("%1").arg(i);
        if (SS.listNeuron.size()>0)
            sketchedNTList.push_back(SS);
    }

    //nt_marker = nt2mm(nt.listNeuron,filename);
    //swc_to_segments(nt_marker,segment);
	V_NeuronSWC_list vnList;
    for(V3DLONG i=0;i<sketchedNTList.size();i++)
    {
		NeuronTree snrTree = sketchedNTList.at(i);
        PARA PA;
        PA.data1d = 0;
        PA.curr_path = path_curr;
		for(int j=0;j<snrTree.listNeuron.size()-1;j++){
			total_length = total_length + sqrt(pow((snrTree.listNeuron.at(j+1).x*0.2-snrTree.listNeuron.at(j).x*0.2),2)+pow((snrTree.listNeuron.at(j+1).y*0.2-snrTree.listNeuron.at(j).y*0.2),2)+pow((snrTree.listNeuron.at(j+1).z-snrTree.listNeuron.at(j).z),2))/1000;
		}
		/*
		if((fabs(snrTree.listNeuron.at(0).radius - 0.618f)<0.001f)||(fabs(snrTree.listNeuron.at(0).radius - 0.666f)<0.001f)){
			for(int j=0;j<snrTree.listNeuron.size()-1;j++){
				v_l = v_l + sqrt(pow((snrTree.listNeuron.at(j+1).x*0.2-snrTree.listNeuron.at(j).x*0.2),2)+pow((snrTree.listNeuron.at(j+1).y*0.2-snrTree.listNeuron.at(j).y*0.2),2)+pow((snrTree.listNeuron.at(j+1).z-snrTree.listNeuron.at(j).z),2))/1000;
			}
		}*/
		
		if(!get_sub_block(callback,1,sketchedNTList[i],PA,i)){
			vnList.append(NeuronTree__2__V_NeuronSWC_list(snrTree).seg.at(0));
			continue;
		}
        LandmarkList marker_v;
        NeuronTree outtree;
		//NeuronTree snrTree = sketchedNTList.at(i);
        //if(sketchedNTList[i].listNeuron.size()<3)continue;
        for(V3DLONG k=0;k<sketchedNTList[i].listNeuron.size();k++)
        {
            NeuronSWC S;
            S.n = k;
            S.x = sketchedNTList[i].listNeuron[k].x-PA.original_o[0];
            S.y = sketchedNTList[i].listNeuron[k].y-PA.original_o[1];
            S.z = sketchedNTList[i].listNeuron[k].z-PA.original_o[2];
            S.r = sketchedNTList[i].listNeuron[k].r;
            S.parent = k-1;
			S.type = sketchedNTList[i].listNeuron[k].type;
            outtree.listNeuron.push_back(S);
        }
        system("mkdir ori_swc");
        writeSWC_file(QString("ori_swc/outtree"+QString::number(i)+".swc"),outtree);
        for(V3DLONG p=0;p<sketchedNTList[i].listNeuron.size();p++)
        {
            LocationSimple mm;
            mm.x = sketchedNTList[i].listNeuron[p].x-PA.original_o[0];
            mm.y = sketchedNTList[i].listNeuron[p].y-PA.original_o[1];
            mm.z = sketchedNTList[i].listNeuron[p].z-PA.original_o[2];
			mm.radius = sketchedNTList[i].listNeuron[p].r;
            //cout<<"mm = "<<mm.x<<"  "<<mm.y<<"  "<<mm.z<<endl;
            mm.color.a = 0;
            mm.color.b = 0;
            mm.color.g = 0;
            mm.color.r = 0;
            marker_v.push_back(mm);
        }

        //segment_mean_shift(data1d,marker_v,im_cropped_sz,i,segment[i]->markers);
        segment_mean_shift_v2(callback,marker_v,PA,i,sketchedNTList[i].listNeuron);

        profile_swc(callback,i,PA,CSVlist);

		if(CSVlist.at(CSVlist.size()-1).at(0).number == i){
			if(CSVlist.at(CSVlist.size()-1).at(0).snr - 1.0000 < 0.0001f){
				for(int j=0;j<snrTree.listNeuron.size();j++){
					snrTree.listNeuron[j].level = 2;
				}
			}else{
				if(CSVlist.at(CSVlist.size()-1).at(0).snr - 1.2000 < 0.0001f){
					for(int j=0;j<snrTree.listNeuron.size();j++){
						snrTree.listNeuron[j].level = 6;
					}
				}else if(CSVlist.at(CSVlist.size()-1).at(0).snr - 1.4000 < 0.0001f){
					for(int j=0;j<snrTree.listNeuron.size();j++){
						snrTree.listNeuron[j].level = 12;
					}
				}
				else{
					for(int j=0;j<snrTree.listNeuron.size();j++){
						snrTree.listNeuron[j].level = 3;
					}
				}
			}	
		}else{
			for(int j=0;j<snrTree.listNeuron.size();j++){
					snrTree.listNeuron[j].level = CSVlist.at(CSVlist.size()-1).at(0).snr*100;
			}
		}
		vnList.append(NeuronTree__2__V_NeuronSWC_list(snrTree).seg.at(0));
        if(PA.data1d) {delete []PA.data1d; PA.data1d = 0;}
    }
	NeuronTree snrNeuron = V_NeuronSWC_list__2__NeuronTree(vnList);
	for(V3DLONG i = 0; i < snrNeuron.listNeuron.size(); i++)
            {
                if((fabs(snrNeuron.listNeuron[i].r - 0.618f) < 0.001f)||(fabs(snrNeuron.listNeuron[i].r - 0.666f) < 0.001f))
				{	snrNeuron.listNeuron[i].type = 4;
					//nt.listNeuron[i].level = 4;
				}
				else
				{	snrNeuron.listNeuron[i].type = 7;
					//nt.listNeuron[i].level = 7;
				}
            }

    QString output_swc_file = filename + "_snr&vr.eswc";
    writeESWC_file(output_swc_file,snrNeuron);

	QString output_csv_file = filename + "w_l.csv";
    writeCSV(CSVlist,output_csv_file,sketchedNTList.size(),total_length,v_l);
	
    return true;

}
bool writeCSV(QList<QList<IMAGE_METRICS> > &CSVlist, QString output_csv_file,int ts,double tl,double vl)
{
    QFile file(output_csv_file);
    if (!file.open(QFile::WriteOnly|QFile::Truncate))
    {
        cout <<"Error opening the file "<<output_csv_file.toStdString().c_str() << endl;
        return false;
    }
    else
    {
        QTextStream stream (&file);
		int total_stroke = CSVlist.size(),vr_stroke = 0;
		double l = 0.0;
		double w_l = 0.0;
		double vr_length = 0.0;
		double ave_snr = 0.0;
		double sum_vr = 0.0;
		double vr_less_than_ave = 0.0;
		double vl_less_than_ave = 0.0;
        stream<< "segment_id,segment_type,num_of_nodes,dynamic_range,cnr,snr,tubularity_mean,tubularity_std,fg_mean,fg_std,brightest_%%20_bg_mean,bg_std,length,w_snr,vr/3d"<<"\n";
		for (int j  = 0; j < CSVlist.size() ; j++){
			w_l = w_l + CSVlist[j][0].length/(CSVlist[j][0].snr-0.2);
			l = l + CSVlist[j][0].length;
			sum_vr = sum_vr + CSVlist[j][0].vr;
			if(CSVlist[j][0].vr == 1){
				vr_stroke++;
			}
		}

		for (int j = 0; j < CSVlist.size() ; j++){
			if(CSVlist[j][0].vr == 1){
				vr_length = vr_length + CSVlist[j][0].length;
			}
		}

		for (int j = 0; j < CSVlist.size() ; j++){
			ave_snr = ave_snr + CSVlist[j][0].length/l*CSVlist[j][0].snr;
		}

		for (int j = 0; j < CSVlist.size() ; j++){
			if(CSVlist[j][0].snr < ave_snr){
				vr_less_than_ave = vr_less_than_ave + CSVlist[j][0].vr;
			}
		}

		for (int j = 0; j < CSVlist.size() ; j++){
			if((CSVlist[j][0].snr < ave_snr)&&(CSVlist[j][0].vr == 1)){
				vl_less_than_ave = vl_less_than_ave + CSVlist[j][0].length;
			}
		}

        for (int j = 0; j < CSVlist.size() ; j++)
        {
            QList<IMAGE_METRICS> result_metrics = CSVlist[j];
            for (int i  = 0; i < result_metrics.size()-1; i++)
            {
				stream << result_metrics[i].number       <<","
					   << result_metrics[i].type       <<","
                       << result_metrics[i].num_of_nodes <<","
                       << result_metrics[i].dy         <<","
                       << result_metrics[i].cnr        <<","
                       << result_metrics[i].snr        <<","
                       << result_metrics[i].tubularity_mean <<","
                       << result_metrics[i].tubularity_std <<","
                       << result_metrics[i].fg_mean    <<","
                       << result_metrics[i].fg_std     <<","
                       << result_metrics[i].bg_mean    <<","
                       << result_metrics[i].bg_std     <<","
					   << result_metrics[i].length/tl      <<","
					   << result_metrics[i].length/tl*result_metrics[i].snr    <<","
					   << result_metrics[i].vr   <<"\n";
            }
        }
		double ts1 = ts,vs1 = vr_stroke;
		string str = output_csv_file.toStdString();
		stream<< "**************************************************************************************************************************"<<"\n";
		stream<< "neuron_id,total_length,vr_length,VR%TL,total_stroke,vr_stroke,VR%TS,ave_snr,vl_usage_rate_1,vl_usage_rate_2,usage_rate_1,usage_rate_2,weighted_length"<<"\n";
		if(sum_vr>-0.000001&&sum_vr<0.000001){
			stream<<999<<","
			  <<tl<<","
			  <<vl<<","
			  <<vl/tl<<","
			  <<ts<<","
			  <<vr_stroke<<","
			  <<vs1/ts1<<","
			  <<ave_snr <<","
			  <<0 <<","
			  <<0 <<","
			  <<0 <<","
			  <<0 <<","
			  <<w_l <<"\n";  
		}else{
			stream<<999<<","
			  <<tl<<","
			  <<vl<<","
			  <<vl/tl<<","
			  <<ts<<","
			  <<vr_stroke<<","
			  <<vs1/ts1<<","
			  <<ave_snr <<","
			  <<vl_less_than_ave/vl <<","
			  <<1 - vl_less_than_ave/vl <<","
			  <<vr_less_than_ave/sum_vr <<","
			  <<1 - vr_less_than_ave/sum_vr <<","
			  <<w_l<<"\n";
		}
		

    file.close();
    }
    return true;

}

void eswc_relevel(V3DPluginCallback2 &callback, QWidget *parent){

	OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return;

    NeuronTree nt = openDlg->nt;

	for(int i=0;i<nt.listNeuron.size();i++){
		if(nt.listNeuron.at(i).type == 7)
			nt.listNeuron[i].level = 7;
		else if(nt.listNeuron.at(i).type == 4)
			nt.listNeuron[i].level = 4;
	}
	QString filename = openDlg->file_name;
	QString output_swc_file = filename + "_vr.eswc";
	writeESWC_file(output_swc_file,nt);
}

void nf__main(V3DPluginCallback2 &callback, QWidget *parent)
{
    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return;

    NeuronTree nt = openDlg->nt;
	//NeuronTree nt2 = openDlg->nt;

	double total_length = 0.0,v_l=0.0;
	int total_stroke = 0,vr_stroke = 0;
	
	for(int i = 0;i<nt.listNeuron.size();i++){
		NeuronSWC curr = nt.listNeuron.at(i);
		int parent = getParent(i,nt);
		if (parent==VOID)
		{
			continue;
		}
		double distance = dist(curr,nt.listNeuron.at(parent));
		if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
		{
			v_l += distance;
		}
	}

	 V_NeuronSWC_list testVNL = NeuronTree__2__V_NeuronSWC_list(nt);
	//NeuronTree ntree = V_NeuronSWC_list__2__NeuronTree(testVNL)
	 QList<NeuronTree> sketchedNTList;
	
    for(int i= 0;i<testVNL.seg.size();i++)
    {
        NeuronTree SS;
        //convert each segment to NeuronTree with one single path
        V_NeuronSWC seg_temp =  testVNL.seg.at(i);
        seg_temp.reverse();
        SS = V_NeuronSWC__2__NeuronTree(seg_temp);
        //append to editable sketchedNTList
        SS.name = "loaded_" + QString("%1").arg(i);
        if (SS.listNeuron.size()>0)
            sketchedNTList.push_back(SS);
    }

	for(V3DLONG i=0;i<sketchedNTList.size();i++)
    {
		NeuronTree snrTree = sketchedNTList.at(i);
		for(int j=0;j<snrTree.listNeuron.size()-1;j++){
			total_length = total_length + sqrt(pow((snrTree.listNeuron.at(j+1).x*0.2-snrTree.listNeuron.at(j).x*0.2),2)+pow((snrTree.listNeuron.at(j+1).y*0.2-snrTree.listNeuron.at(j).y*0.2),2)+pow((snrTree.listNeuron.at(j+1).z-snrTree.listNeuron.at(j).z),2))/1000;
		}
		if((fabs(snrTree.listNeuron.at(0).radius - 0.618f) < 0.001f)||(fabs(snrTree.listNeuron.at(0).radius - 0.666f)<0.001f)){
			vr_stroke = total_stroke + 1;
		}
	}
	cout<<"total length"<<total_length<<endl;
	cout<<"vr length"<<v_l<<endl;
	cout<<"total stroke"<<sketchedNTList.size()<<endl;
	cout<<"vr stroke"<<vr_stroke<<endl;
	
}
void testNTree_to_SWCList(V3DPluginCallback2 &callback, QWidget *parent){

	QList<NeuronTree> sketchedNTList;
	OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return;

    NeuronTree nt = openDlg->nt;
	V_NeuronSWC_list testVNL = NeuronTree__2__V_NeuronSWC_list(nt);

	 for(int i= 0;i<testVNL.seg.size();i++)
    {
        NeuronTree SS;
        //convert each segment to NeuronTree with one single path
        V_NeuronSWC seg_temp =  testVNL.seg.at(i);
        //seg_temp.reverse();
        SS = V_NeuronSWC__2__NeuronTree(seg_temp);
        if (SS.listNeuron.size()>0)
            sketchedNTList.push_back(SS);
    }
	V_NeuronSWC_list vnList;
	for(V3DLONG i=0;i<sketchedNTList.size();i++){
		NeuronTree snrTree = sketchedNTList.at(i);
		vnList.append(NeuronTree__2__V_NeuronSWC_list(snrTree).seg.at(0));
	}

	NeuronTree snrNeuron = V_NeuronSWC_list__2__NeuronTree(vnList);
	for(V3DLONG i = 0; i < snrNeuron.listNeuron.size(); i++)
            {
                if((fabs(snrNeuron.listNeuron[i].r - 0.618f) < 0.001f)||(fabs(snrNeuron.listNeuron[i].r - 0.666f) < 0.001f))
				{	snrNeuron.listNeuron[i].type = 4;
					//nt.listNeuron[i].level = 4;
				}
				else
				{	snrNeuron.listNeuron[i].type = 7;
					//nt.listNeuron[i].level = 7;
				}
            }

    QString output_swc_file = "test_snr&vr.eswc";
    writeESWC_file(output_swc_file,snrNeuron);
}

bool test_batch_file(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback){
	char * paras = NULL;
	
	vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
	if (inlist->size()==0)
	{
		cerr<<"You must specify input linker or swc files"<<endl;
		return false;
	}

	cout<<"group swc list begin!"<<endl;
	cout<<inlist->size()<<endl;
	vector<NeuronTree> nt_list;
	QStringList nameList;
	QString qs_linker;
	char * dfile_result = NULL;
	V3DLONG neuronNum = 0;
	QString s[100];
	QString fileDefaultName;
	//fileDefaultName +="_0.sswc";
	for (int i=0;i<inlist->size();i++)
	{
		cout<<inlist->at(i)<<endl;
		qs_linker = QString(inlist->at(i));
		if (qs_linker.toUpper().endsWith(".ANO"))
		{
			cout<<"(0). reading a linker file."<<endl
			;
			P_ObjectFileType linker_object;
            if (!loadAnoFile(qs_linker,linker_object))
			{
				fprintf(stderr,"Error in reading the linker file.\n");
				return 1;
			}
			nameList = linker_object.swc_file_list;
			neuronNum += nameList.size();
			for (V3DLONG i=0;i<neuronNum;i++)
			{
				NeuronTree tmp = readSWC_file(nameList.at(i));
				nt_list.push_back(tmp);
				//cout<<nameList.at(i).toStdString()<<endl;
				//s[i]=nameList.at(i).toStdString().substr(16,3);
				s[i]=nameList.at(i);
			}
		}
		else if (qs_linker.toUpper().endsWith(".SWC"))
		{
            //cout<<"(0). reading an swc file"<<endl;
			NeuronTree tmp = readSWC_file(qs_linker);
			nt_list.push_back(tmp);
			neuronNum++;
		}
	}
	fileDefaultName = qs_linker;
	fileDefaultName +="_0.txt";
	cout<<nt_list.size()<<endl;

	for(int i=0;i<nt_list.size();i++){
		segment_profiling_main(callback,nt_list.at(i),s[i]);
	}

}

bool test_batch_file_menu(V3DPluginCallback2 &callback){
	vector<NeuronTree> nt_list;
	QStringList nameList;
	V3DLONG neuronNum = 0;
	QString s[105];

	QString fileOpenName;
	fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
		"",
		QObject::tr("Supported file (*.ano *.swc  )"
                    ";;Neuron structure	(*.swc)"
					";;annotatin files	(*.ano)"
		));
	if(fileOpenName.isEmpty()) 
	{
		return NULL;
	}


	if (fileOpenName.endsWith(".ano") || fileOpenName.endsWith(".ANO"))
	{
		cout<<"(0). reading a linker file."<<endl;
		P_ObjectFileType linker_object;
        if (!loadAnoFile(fileOpenName,linker_object))
		{
			fprintf(stderr,"Error in reading the linker file.\n");
			return 1;
		}
		nameList = linker_object.swc_file_list;
		neuronNum += nameList.size();
		for (V3DLONG i=0;i<neuronNum;i++)
		{
			NeuronTree tmp = readSWC_file(nameList.at(i));
			nt_list.push_back(tmp);
			s[i]=nameList.at(i);
		}
	}
	else {
#ifndef DISABLE_V3D_MSG
		v3d_msg("The file type you specified is not supported. Please check.");
#endif
		return NULL;
	}
	if(nt_list.size()==0)
	{
		cout<<"\nread failed\n"<<endl;
	}
	else
	{
		cout<<"\nread successed\n"<<endl;
	}
	cout<<"\nThis is a demo plugin to show how to read swc data."<<endl;
	cout<<nt_list.size()<<endl;

	for(int i=0;i<nt_list.size();i++){
		segment_profiling_main(callback,nt_list.at(i),s[i]);
		//count_low_snr_length(callback,nt_list.at(i),s[i]);
	}

	return true;

}

bool count_low_snr_length(V3DPluginCallback2 &callback,NeuronTree &nt,QString &filename){
	V_NeuronSWC_list testVNL = NeuronTree__2__V_NeuronSWC_list(nt);
	QList<NeuronTree> sketchedNTList;
	double total_length = 0.0;
	double red_length = 0.0;
	double yellow_length = 0.0;


	for(int i= 0;i<testVNL.seg.size();i++)
    {
        NeuronTree SS;
        //convert each segment to NeuronTree with one single path
        V_NeuronSWC seg_temp =  testVNL.seg.at(i);
        //seg_temp.reverse();
        SS = V_NeuronSWC__2__NeuronTree(seg_temp);
        //append to editable sketchedNTList
        //SS.name = "loaded_" + QString("%1").arg(i);
        if (SS.listNeuron.size()>0)
            sketchedNTList.push_back(SS);
    }

    for(V3DLONG i=0;i<sketchedNTList.size();i++)
    {
		NeuronTree snrTree = sketchedNTList.at(i);
		for(int j=0;j<snrTree.listNeuron.size()-1;j++){
			total_length = total_length + sqrt(pow((snrTree.listNeuron.at(j+1).x*0.2-snrTree.listNeuron.at(j).x*0.2),2)+pow((snrTree.listNeuron.at(j+1).y*0.2-snrTree.listNeuron.at(j).y*0.2),2)+pow((snrTree.listNeuron.at(j+1).z-snrTree.listNeuron.at(j).z),2))/1000;
		}
		
		if(snrTree.listNeuron.at(0).level == 2){
			//segNuminVR[0]++;
			for(int j=0;j<snrTree.listNeuron.size()-1;j++){
				red_length = red_length + sqrt(pow((snrTree.listNeuron.at(j+1).x*0.2-snrTree.listNeuron.at(j).x*0.2),2)+pow((snrTree.listNeuron.at(j+1).y*0.2-snrTree.listNeuron.at(j).y*0.2),2)+pow((snrTree.listNeuron.at(j+1).z-snrTree.listNeuron.at(j).z),2))/1000;
			}
		}

		if(snrTree.listNeuron.at(0).level == 6){
			//segNuminVR[0]++;
			for(int j=0;j<snrTree.listNeuron.size()-1;j++){
				yellow_length = yellow_length + sqrt(pow((snrTree.listNeuron.at(j+1).x*0.2-snrTree.listNeuron.at(j).x*0.2),2)+pow((snrTree.listNeuron.at(j+1).y*0.2-snrTree.listNeuron.at(j).y*0.2),2)+pow((snrTree.listNeuron.at(j+1).z-snrTree.listNeuron.at(j).z),2))/1000;
			}
		}

	}
	filename = filename + ("low_snr.csv");
	QFile file(filename);
    if (!file.open(QFile::WriteOnly|QFile::Truncate))
    {
        cout <<"Error opening the file "<<filename.toStdString().c_str() << endl;
        return false;
    }
	else{
		QTextStream stream (&file);
		stream<< "total_length,red_length,yellow_length,rl_tl,yl_tl"<<"\n";
		stream<<total_length<<","
			  <<red_length<<","
			  <<yellow_length<<","
			  <<red_length/total_length<<","
			  <<yellow_length/total_length<<","
			  <<0 <<"\n";  
		file.close();
		
	}
	return true;
}