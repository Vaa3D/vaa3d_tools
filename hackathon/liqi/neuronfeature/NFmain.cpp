#include "NFmain.h"
#include "../../../../v3d_external/v3d_main/neuron_editing/global_feature_compute.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include <vector>
#include <math.h>
#include <iostream>
#include "openSWCDialog.h"

using namespace std;

#define FNUM 22
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
	for(int i=0;i<nt.listNeuron.size();i++)
	{
		NeuronSWC curr = nt.listNeuron.at(i);
		int parent = getParent(i,nt);
		//qDebug()<<parent;
		if (parent==VOID)
		{
			switch(curr.type)
			{
				case 2:// type ==2
					if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
					{
						segNuminVR[0]++;
						segNuminVR[1]++;
					}
					else
					{
						segNumin3DView[0]++;
						segNumin3DView[1]++;
					}
					break;
				case 3://type == 3
					if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
					{
						segNuminVR[0]++;
						segNuminVR[2]++;
					}
					else
					{
						segNumin3DView[0]++;
						segNumin3DView[2]++;
					}
					break;
				case 4:// type ==4
					if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
					{
						segNuminVR[0]++;
						segNuminVR[3]++;
					}
					else
					{
						segNumin3DView[0]++;
						segNumin3DView[3]++;
					}
					break;
				case 5://type == 5
					if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
					{
						segNuminVR[0]++;
						segNuminVR[4]++;
					}
					else
					{
						segNumin3DView[0]++;
						segNumin3DView[4]++;
					}
					break;
				default:
					break;
			}
			continue;
		}
		double distance = dist(curr,nt.listNeuron.at(parent));
		//qDebug()<<"distance = "<<distance;
		if((fabs(curr.radius - 0.618f) < 0.001f)||(fabs(curr.radius - 0.666f)<0.001f))
		{
			lengthinVR[0] += distance;
			switch(curr.type)
			{
				case 2:// type ==2
					lengthinVR[1] += distance;
					break;
				case 3://type == 3
					lengthinVR[2] += distance;
					break;
				case 4:// type ==4
					lengthinVR[3] += distance;
					break;
				case 5://type == 5
					lengthinVR[4] += distance;
					break;
				default:
					break;
			}
			//qDebug()<<"VR:    length = "<<lengthVR;
		}
		else
		{
			lengthin3DView[0] += distance;
			switch(curr.type)
			{
				case 2:// type ==2
					lengthin3DView[1] += distance;
					break;
				case 3://type == 3
					lengthin3DView[2] += distance;
					break;
				case 4:// type ==4
					lengthin3DView[3] += distance;
					break;
				case 5://type == 5
					lengthin3DView[4] += distance;
					break;
				default:
					break;
			}
		}
	}

	NeuronTree keyPoint_list;
	NeuronTree normalpoint_list;
	QList<SWCsegment> seg_list;
	
	

	//output segments
	vector <int> childnum(nt.listNeuron.size());
	for(int i=0;i<nt.listNeuron.size();i++)
	{
		if (nt.listNeuron[i].parent != -1) 
			childnum[nt.listNeuron[i].parent-1]++;
	}
	qDebug("run child num");
	for(int i=0;i<nt.listNeuron.size();i++)
	{
		NeuronSWC ss = nt.listNeuron.at(i);
		//SWCsegment seg;
		if(ss.pn==-1)
		{
			//root node ,must be key node(key point)
			nt.listNeuron[i].type=825;//825 means keypoint, 528 means normalpoint
			keyPoint_list.listNeuron.append(ss);
			keyPoint_list.hashNeuron.insert(ss.n, keyPoint_list.listNeuron.size()-1);
		}
		else
		{
			//QList<int> nkeys= neuron.hashNeuron.keys(ss.n);
			int nkeys = childnum.at(i);
			if(nkeys==1)
			{
				normalpoint_list.listNeuron.append(ss);
				normalpoint_list.hashNeuron.insert(ss.n,normalpoint_list.listNeuron.size()-1);
				nt.listNeuron[i].type=528;//set normal node's type as 528
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
	for(int i=0,nkeypoint=0;i<nt.listNeuron.size();i++)
	{
		NeuronSWC s1 = nt.listNeuron.at(i);
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

	int branchNuminVR[5] = {}, branchNumin3DView[5] = {};

	for(int i=0;i<seg_list.size();i++){
		if((fabs(nt.listNeuron.at(seg_list.at(i)[0]-1).radius - 0.618f) < 0.001f)||(fabs(nt.listNeuron.at(seg_list.at(i)[0]-1).radius - 0.666f) < 0.001f)){
			branchNuminVR[0]++;

			switch(nt2.listNeuron.at(seg_list.at(i)[0]-1).type){
				case 2:// type ==2
						branchNuminVR[1]++;
					break;
				case 3://type == 3
						branchNuminVR[2]++;
					break;
				case 4:// type ==4
						branchNuminVR[3]++;
					break;
				case 5://type == 5
						branchNuminVR[4]++;
					break;
				default:
					break;
			}
			continue;

		} 
		else {
			branchNumin3DView[0]++;
			switch(nt2.listNeuron.at(seg_list.at(i)[0]-1).type){
				case 2:// type ==2
						branchNumin3DView[1]++;
					break;
				case 3://type == 3
						branchNumin3DView[2]++;
					break;
				case 4:// type ==4
						branchNumin3DView[3]++;
					break;
				case 5://type == 5
						branchNumin3DView[4]++;
					break;
				default:
					break;
			}
		}
	}
	//qDebug("feature done");

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
				"			Segments in type5 : %20	<br><br>"
				"Branch Number in VR			: %21	<br>"
				"			Segments in type2 : %22	<br>"
				"			Segments in type3 : %23	<br>"
				"			Segments in type4 : %24	<br>"
				"			Segments in type5 : %25	<br><br>"
				"Branch Number in 3D View 		: %26	<br>"
				"			Segments in type2 : %27	<br>"
				"			Segments in type3 : %28	<br>"
				"			Segments in type4 : %29	<br>"
				"			Segments in type5 : %30	</font></pre>")
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
				.arg(segNumin3DView[4])
				.arg(branchNuminVR[0])
				.arg(branchNuminVR[1])
				.arg(branchNuminVR[2])
				.arg(branchNuminVR[3])
				.arg(branchNuminVR[4])
				.arg(branchNumin3DView[0])
				.arg(branchNumin3DView[1])
				.arg(branchNumin3DView[2])
				.arg(branchNumin3DView[3])
				.arg(branchNumin3DView[4]));

	infoBox.exec();

	delete openDlg; // MK, Oct, 2017, free up dialog pointer to memory violation.
}
