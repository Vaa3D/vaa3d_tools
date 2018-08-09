#include "NFmain.h"
#include "../../../../v3d_external/v3d_main/neuron_editing/global_feature_compute.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include <vector>
#include "../../../../v3d_external/v3d_main/neuron_editing/neuron_format_converter.h"
#include "../../../../v3d_external/v3d_main/neuron_editing/v_neuronswc.h"
#include <math.h>
#include <iostream>
#include "openSWCDialog.h"s

using namespace std;

#define FNUM 22
#define NUM 32
#define getParent(n,nt) ((nt).listNeuron.at(n).pn<0)?(1000000000):((nt).hashNeuron.value((nt).listNeuron.at(n).pn))
#define VOID 1000000000
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

int calSmoothedPN(int pn,NeuronTree & nt_ori,SWCsegment &segm )
{
	if(pn==-1) return -1;
	NeuronSWC s_temp= nt_ori.listNeuron.at(pn-1);
	//qDebug()<<s_temp.n<<"		"<<s_temp.pn<<"type="<<s_temp.type;
	segm.push_back(s_temp.n);
	if(s_temp.type==528)
	{
		if(s_temp.pn==-1) return s_temp.n;
		//528 means  s_temp is a normal node,so search s_temp's parent node
		return calSmoothedPN(s_temp.pn,nt_ori,segm);
	}
	else if(s_temp.type==825)
	{
		//825 means  s_temp is a key node in this segment,so return s_temp's n

		return s_temp.n;
	}
	return -1;
}


bool testReadAno(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback){
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
	string s[NUM];
	QString fileDefaultName;
	//fileDefaultName +="_0.sswc";
	for (int i=0;i<inlist->size();i++)
	{
		cout<<inlist->at(i)<<endl;
		qs_linker = QString(inlist->at(i));
		if (qs_linker.toUpper().endsWith(".ANO"))
		{
			cout<<"(0). reading a linker file."<<endl;
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
				s[i]=nameList.at(i).toStdString().substr(16,3);
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

	double lengthinVR[NUM][5] = {}, lengthin3DView[NUM][5] = {};
	int segNuminVR[NUM][5] = {}, segNumin3DView[NUM][5] = {};
	int branchNuminVR[NUM][5] = {}, branchNumin3DView[NUM][5] = {};
	for(int i=0;i<nt_list.size();i++){
		for(int j=0;j<nt_list.at(i).listNeuron.size();j++){
			NeuronSWC curr = nt_list.at(i).listNeuron.at(j);
			int parent = getParent(j,nt_list.at(i));
			if (parent==VOID)
			{
				switch(curr.type)
				{
					case 2:// type ==2
						if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
						{
							segNuminVR[i][0]++;
							segNuminVR[i][1]++;
						}
						else
						{
							segNumin3DView[i][0]++;
							segNumin3DView[i][1]++;
						}
						break;
					case 3://type == 3
						if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
						{
							segNuminVR[i][0]++;
							segNuminVR[i][2]++;
						}
						else
						{
							segNumin3DView[i][0]++;
							segNumin3DView[i][2]++;
						}
						break;
					case 6:// type ==4
						if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
						{
							segNuminVR[i][0]++;
							segNuminVR[i][3]++;
						}
						else
						{
							segNumin3DView[i][0]++;
							segNumin3DView[i][3]++;
						}
						break;
					case 7://type == 5
						if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
						{
							segNuminVR[i][0]++;
							segNuminVR[i][4]++;
						}
						else
						{
							segNumin3DView[i][0]++;
							segNumin3DView[i][4]++;
						}
						break;
					default:
						break;
				}
				continue;
			}
			double distance = dist(curr,nt_list.at(i).listNeuron.at(parent));
			//qDebug()<<"distance = "<<distance;
			if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
			{
				lengthinVR[i][0] += distance;
				switch(curr.type)
				{
					case 2:// type ==2
						lengthinVR[i][1] += distance;
						break;
					case 3://type == 3
						lengthinVR[i][2] += distance;
						break;
					case 6:// type ==4
						lengthinVR[i][3] += distance;
						break;
					case 7://type == 5
						lengthinVR[i][4] += distance;
						break;
					default:
						break;
				}
				//qDebug()<<"VR:    length = "<<lengthVR;
			}
			else
			{
				lengthin3DView[i][0] += distance;
				switch(curr.type)
				{
					case 2:// type ==2
						lengthin3DView[i][1] += distance;
						break;
					case 3://type == 3
						lengthin3DView[i][2] += distance;
						break;
					case 6:// type ==4
						lengthin3DView[i][3] += distance;
						break;
					case 7://type == 5
						lengthin3DView[i][4] += distance;
						break;
					default:
						break;
				}
			}
		}
		cout<<"stroke and segment done!"<<endl;
		NeuronTree nt = nt_list.at(i),nt2 = nt_list.at(i);

		NeuronTree keyPoint_list;
		NeuronTree normalpoint_list;
		QList<SWCsegment> seg_list;
	cout<<"start"<<endl;
	

	//output segments
	vector <int> childnum(nt.listNeuron.size());
	cout<<"start"<<endl;
	for(int j=0;j<nt.listNeuron.size();j++)
	{
		cout<<nt.listNeuron[j].parent<<endl;
		if (nt.listNeuron[j].parent != -1) 
			childnum[nt.listNeuron[j].parent-1]++;
	}
	qDebug("run child num");
	for(int j=0;j<nt.listNeuron.size();j++)
	{
		NeuronSWC ss = nt.listNeuron.at(j);
		//SWCsegment seg;
		if(ss.pn==-1)
		{
			//root node ,must be key node(key point)
			nt.listNeuron[j].type=825;//825 means keypoint, 528 means normalpoint
			keyPoint_list.listNeuron.append(ss);
			keyPoint_list.hashNeuron.insert(ss.n, keyPoint_list.listNeuron.size()-1);
		}
		else
		{
			//QList<int> nkeys= neuron.hashNeuron.keys(ss.n);
			int nkeys = childnum.at(j);
			if(nkeys==1)
			{
				normalpoint_list.listNeuron.append(ss);
				normalpoint_list.hashNeuron.insert(ss.n,normalpoint_list.listNeuron.size()-1);
				nt.listNeuron[j].type=528;//set normal node's type as 528
			}
			else if((nkeys>=2)||(nkeys==0))
			{
				nt.listNeuron[i].type=825;
				keyPoint_list.listNeuron.append(ss);
				keyPoint_list.hashNeuron.insert(ss.n, keyPoint_list.listNeuron.size()-1);
			}
		}
	}
	qDebug("run child num");
	for(int j=0,nkeypoint=0;j<nt.listNeuron.size();j++)
	{
		NeuronSWC s1 = nt.listNeuron.at(j);
		if(s1.pn==-1)
		{
			nkeypoint++;
		}
		else if(s1.type==528) 
		{
		}
		else if(s1.type==825)
		{			
			int parent=0;
			SWCsegment segment;
			segment.push_back(s1.n);
			parent = calSmoothedPN(s1.pn,nt,segment);
			keyPoint_list.listNeuron[nkeypoint].pn = parent;
			seg_list.push_back(segment);
			nkeypoint++;
		}
	}
	childnum.clear();

	
	cout<<"branch count begin"<<endl;
	for(int k=0;k<seg_list.size();k++){
		if((fabs(nt.listNeuron.at(seg_list.at(k)[0]-1).radius - 0.618f) < 0.001f)||(fabs(nt.listNeuron.at(seg_list.at(k)[0]-1).radius - 0.666f) < 0.001f)){
			branchNuminVR[i][0]++;

			switch(nt2.listNeuron.at(seg_list.at(k)[0]-1).type){
				case 2:// type ==2
						branchNuminVR[i][1]++;
					break;
				case 3://type == 3
						branchNuminVR[i][2]++;
					break;
				case 6:// type ==4
						branchNuminVR[i][3]++;
					break;
				case 7://type == 5
						branchNuminVR[i][4]++;
					break;
				default:
					break;
			}
			continue;

		} 
		else {
			branchNumin3DView[i][0]++;
			switch(nt2.listNeuron.at(seg_list.at(k)[0]-1).type){
				case 2:// type ==2
						branchNumin3DView[i][1]++;
					break;
				case 3://type == 3
						branchNumin3DView[i][2]++;
					break;
				case 6:// type ==4
						branchNumin3DView[i][3]++;
					break;
				case 7://type == 5
						branchNumin3DView[i][4]++;
					break;
				default:
					break;
			}
		}
	}
	}
	//cout<<"*****************************************************"<<endl;
	for(int i=0;i<nt_list.size();i++){
		cout<<s[i]<<" "<<branchNuminVR[i][0]<<" "<<branchNuminVR[i][1]<<" "<<branchNuminVR[i][2]<<" "<<branchNuminVR[i][3]<<" "<<branchNuminVR[i][4]<<endl;
		cout<<s[i]<<" "<<branchNumin3DView[i][0]<<" "<<branchNumin3DView[i][1]<<" "<<branchNumin3DView[i][2]<<" "<<branchNumin3DView[i][3]<<" "<<branchNumin3DView[i][4]<<endl;
	}
	FILE * fp;
	fp = fopen(qPrintable(fileDefaultName),"w");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR: %s: failed to open file to write!\n",fileDefaultName);
		return false;
	}
	fprintf(fp, "id  total length   VR\%%   dendrite length   VR den\%%   total strokes   VR strokes   VR\%% \n");
	for(int i=0;i<nt_list.size();i++){
		float a = branchNumin3DView[i][0],b=branchNuminVR[i][0],a2 = branchNumin3DView[i][2],b2 = branchNuminVR[i][2],a3 = branchNumin3DView[i][3],b3 = branchNuminVR[i][3],a4 = branchNumin3DView[i][4],b4 = branchNuminVR[i][4];
		float f = b/(a+b)*100,f2 = b2/(a2+b2)*100,f3 = b3/(a3+b3)*100,f4 = b4/(a4+b4)*100;
		fprintf(fp, "%s\t %5.2f\t %5.2f%%\t %5.2f\t %5.2f%%\t %5.2f\t %5.2f%%\t %5.2f\t %5.2f%%\t %d\t %5.2f%%\t %d\t %5.2f%%\t %d\t %5.2f%%\t %d\t %5.2f%%\n",
				s[i],lengthin3DView[i][0]+lengthinVR[i][0],lengthinVR[i][0]/(lengthin3DView[i][0]+lengthinVR[i][0])*100,lengthin3DView[i][2]+lengthinVR[i][2],lengthinVR[i][2]/(lengthin3DView[i][2]+lengthinVR[i][2])*100,lengthin3DView[i][4]+lengthinVR[i][4],lengthinVR[i][4]/(lengthin3DView[i][4]+lengthinVR[i][4])*100,
				lengthin3DView[i][3]+lengthinVR[i][3],lengthinVR[i][3]/(lengthin3DView[i][3]+lengthinVR[i][3])*100,branchNumin3DView[i][0]+branchNuminVR[i][0],f,branchNuminVR[i][2]+branchNumin3DView[i][2],f2,branchNumin3DView[i][4]+branchNuminVR[i][4],f4,branchNumin3DView[i][3]+branchNuminVR[i][3],f3);
	}
	//cout<<"*****************************************************"<<endl;
	return true;
}


void test_func(int aaa, double bbb)
{
	qDebug()<<"this is a test";
}

void nf__main(V3DPluginCallback2 &callback, QWidget *parent)
{
    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return;

    NeuronTree nt = openDlg->nt;
	NeuronTree nt2 = openDlg->nt;

	double lengthinVR[5] = {}, lengthin3DView[5] = {};
	int segNuminVR[5] = {}, segNumin3DView[5] = {};
	double total_length = 0.0;
	V_NeuronSWC_list testVNL = NeuronTree__2__V_NeuronSWC_list(nt);
	QList<NeuronTree> sketchedNTList;
	/*
	for(int i=0;i<nt.listNeuron.size();i++)
	{
		NeuronSWC curr = nt.listNeuron.at(i);
		int parent = getParent(i,nt);
		//qDebug()<<parent;
		if (parent==VOID)
		{
			continue;
		}
		double distance = dist(curr,nt.listNeuron.at(parent));
		//qDebug()<<"distance = "<<distance;
		if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
		{
			lengthinVR[0] += distance;
		}
	}*/


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
	
    for(V3DLONG i=0;i<sketchedNTList.size();i++)
    {
		NeuronTree snrTree = sketchedNTList.at(i);
		for(int j=0;j<snrTree.listNeuron.size()-1;j++){
			total_length = total_length + sqrt(pow((snrTree.listNeuron.at(j+1).x*0.2-snrTree.listNeuron.at(j).x*0.2),2)+pow((snrTree.listNeuron.at(j+1).y*0.2-snrTree.listNeuron.at(j).y*0.2),2)+pow((snrTree.listNeuron.at(j+1).z-snrTree.listNeuron.at(j).z),2))/1000;
		}
		
		if((fabs(snrTree.listNeuron.at(0).radius - 0.618f)<0.001f)||(fabs(snrTree.listNeuron.at(0).radius - 0.666f)<0.001f)){
			segNuminVR[0]++;
			for(int j=0;j<snrTree.listNeuron.size()-1;j++){
				lengthinVR[0] = lengthinVR[0] + sqrt(pow((snrTree.listNeuron.at(j+1).x*0.2-snrTree.listNeuron.at(j).x*0.2),2)+pow((snrTree.listNeuron.at(j+1).y*0.2-snrTree.listNeuron.at(j).y*0.2),2)+pow((snrTree.listNeuron.at(j+1).z-snrTree.listNeuron.at(j).z),2))/1000;
			}
		}

	}
	
	cout<<"*********************************************************"<<endl;
	cout<<"total_length = "<<total_length<<endl;
	cout<<"total_stroke = "<<sketchedNTList.size()<<endl;

	QMessageBox infoBox;
	infoBox.setText("Global features of VR & 3D View:");
	infoBox.setInformativeText(QString("<pre><font size='4'>"
				"Length in VR				 : %1	<br>"
				"			Length in type2 : %2	<br>"
				"			Length in type3 : %3	<br>"
				"			Length in type4 : %4	<br>"
				"			Length in type5 : %5	<br><br>"
				"Length in 3D View 			 : %6	<br>"
				"			Length in type2 : %7	<br>"
				"			Length in type3 : %8	<br>"
				"			Length in type4 : %9	<br>"
				"			Length in type5 : %10	<br><br>"
				"Segment Number in VR			: %11	<br>"
				"			Segments in type2 : %12	<br>"
				"			Segments in type3 : %13	<br>"
				"			Segments in type4 : %14	<br>"
				"			Segments in type5 : %15	<br><br>"
				"Segment Number in 3D View 		: %16	<br>"
				"			Segments in type2 : %17	<br>"
				"			Segments in type3 : %18	<br>"
				"			Segments in type4 : %19	<br>"
				"			Segments in type5 : %20	</font></pre>")
				.arg(lengthinVR[0])
				.arg(lengthinVR[1])
				.arg(lengthinVR[2])
				.arg(lengthinVR[3])
				.arg(lengthinVR[4])
				.arg(lengthin3DView[0])
				.arg(lengthin3DView[1])
				.arg(lengthin3DView[2])
				.arg(lengthin3DView[3])
				.arg(lengthin3DView[4])
				.arg(segNuminVR[0])
				.arg(segNuminVR[1])
				.arg(segNuminVR[2])
				.arg(segNuminVR[3])
				.arg(segNuminVR[4])
				.arg(segNumin3DView[0])
				.arg(segNumin3DView[1])
				.arg(segNumin3DView[2])
				.arg(segNumin3DView[3])
				.arg(segNumin3DView[4]));

	infoBox.exec();

	
	delete openDlg; // MK, Oct, 2017, free up dialog pointer to memory violation.
}
