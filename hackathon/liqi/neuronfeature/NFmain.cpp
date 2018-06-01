#include "NFmain.h"
#include "global_feature_compute.h"
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



void test_func(int aaa, double bbb)
{
	qDebug()<<"this is a test";
}

void nf__main(V3DPluginCallback2 &callback, QWidget *parent)
{
    qDebug()<<"This is a test by binWW";
    OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
    if (!openDlg->exec())
        return;

    NeuronTree nt = openDlg->nt;

	double lengthinVR[5] = {}, lengthin3DView[5] = {};
	int segNuminVR[5] = {}, segNumin3DView[5] = {};
	for(int i=0;i<nt.listNeuron.size();i++)
	{
		NeuronSWC curr = nt.listNeuron.at(i);
		int parent = getParent(i,nt);
		qDebug()<<parent;
		if (parent==VOID)
		{
			switch(curr.type)
			{
				case 2:// type ==2
					if((curr.radius - 0.618f < 0.001f)||(curr.radius - 0.666f<0.001f))
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
					if((curr.radius - 0.618f < 0.001f)||(curr.radius - 0.666f<0.001f))
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
					if((curr.radius - 0.618f < 0.001f)||(curr.radius - 0.666f<0.001f))
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
					if((curr.radius - 0.618f < 0.001f)||(curr.radius - 0.666f<0.001f))
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
		if((curr.radius - 0.618f < 0.001f)||(curr.radius - 0.666f<0.001f))
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
			//qDebug()<<"3D View:    length = "<<lengthVR;
		}
	}

	QMessageBox infoBox;
	infoBox.setText("Global features of VR & 3D View:");
	infoBox.setInformativeText(QString("<pre><font size='4'>"
				"Length in VR				 : %1	<br>"
				"			Length in Area1 : %2	<br>"
				"			Length in Area2 : %3	<br>"
				"			Length in Area3 : %4	<br>"
				"			Length in Area4 : %5	<br><br>"
				"Length in 3D View 			 : %6	<br>"
				"			Length in Area1 : %7	<br>"
				"			Length in Area2 : %8	<br>"
				"			Length in Area3 : %9	<br>"
				"			Length in Area4 : %10	<br><br>"
				"Segment Number in VR			: %11	<br>"
				"			Segments in Area1 : %12	<br>"
				"			Segments in Area2 : %13	<br>"
				"			Segments in Area3 : %14	<br>"
				"			Segments in Area4 : %15	<br><br>"
				"Segment Number in 3D View 		: %16	<br>"
				"			Segments in Area1 : %17	<br>"
				"			Segments in Area2 : %18	<br>"
				"			Segments in Area3 : %19	<br>"
				"			Segments in Area4 : %20	</font></pre>")
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
