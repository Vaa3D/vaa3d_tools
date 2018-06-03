 #include "v3d_message.h"
#include <vector>
#include <map>
#include "mean_distance_plugin.h"
#include <iostream>
#include <QString>
#include <math.h>

using namespace std;

Q_EXPORT_PLUGIN2(meandistance, mean_distance_marker);
 
QStringList mean_distance_marker::menulist() const
{
	return QStringList() 
		<<tr("mean_distance")
		<<tr("about");
}

QStringList mean_distance_marker::funclist() const
{
    return QStringList()
        <<tr("mean_distance")
		<<tr("about");
}


void mean_distance_marker::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("mean_distance"))
	{
        mean_distance();
	}
	else
	{
		v3d_msg(tr("This plugin will show mean distance"));
	}
}

bool mean_distance_marker::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("mean_distance"))
    {
        mean_distance();

    }
	
	else return false;

	return true;
}

float mean_distance(){
	float a;
	QString fileOpenName;
	fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
		"",
		QObject::tr("Supported file (*.apo *.marker)"
		";;Point Cloud		(*.apo)"
		";;Landmarks		(*.marker)"
		));
	if(fileOpenName.isEmpty()) 
	{
		return a;
	}

	QList <CellAPO> mylist;
	QList <ImageMarker> tmp_list;
	//vector<V3DLONG> segment_id, segment_layer;
	if (fileOpenName.endsWith(".apo") || fileOpenName.endsWith(".APO"))
	{
		mylist = readAPO_file(fileOpenName);


		if(mylist.size()==0)
		{
			cout<<"\nread failed\n"<<endl;
		}
		else
		{
			cout<<"\nread successed\n"<<endl;
		}
		cout<<"\nThis is a demo plugin to show how to read apo data."<<endl;

		float sum_x=0,sum_y=0,sum_z=0;
		float sum_distance=0;
		QList<float> mean_x,mean_y,mean_z;
		/*求虚拟中心*/
		for(int i =0;i<mylist.size();i++){
			sum_x=sum_x+mylist.at(i).x;
			sum_y=sum_y+mylist.at(i).y;
			sum_z=sum_z+mylist.at(i).z;
			if((i+1)%5==0){
				mean_x.append(sum_x/5);
				mean_y.append(sum_y/5);
				mean_z.append(sum_z/5);
				sum_x=0;sum_y=0;sum_z=0;
			}
		}

	
		/*求平均距离*/
		for(int i = 0;i<mylist.size();i++){
			cout<<"\ndistance"<<i+1<<"="<<sqrt(pow(mylist.at(i).x-mean_x.at(i/5),2)+pow(mylist.at(i).y-mean_y.at(i/5),2)+pow(mylist.at(i).z-mean_z.at(i/5),2))<<endl;
			sum_distance = sum_distance + sqrt(pow(mylist.at(i).x-mean_x.at(i/5),2)+pow(mylist.at(i).y-mean_y.at(i/5),2)+pow(mylist.at(i).z-mean_z.at(i/5),2));
			if((i+1)%5==0){
				float mean_distance = sum_distance/5;
				cout<<"\nThe mean distance is:"<<mean_distance<<endl;
				cout<<"*********************************************"<<endl;
				sum_distance=0;
			}
		}
		float mean_distance5 = sum_distance/tmp_list.size();

		//cout<<"\nThe mean distance is:"<<mean_distance<<endl;

		return mean_distance5;

	}else if(fileOpenName.endsWith(".marker") || fileOpenName.endsWith(".MARKER")){
		tmp_list = readMarker_file(fileOpenName);
		if(tmp_list.size()==0)
		{
			cout<<"\nread failed\n"<<endl;
		}
		else
		{
			cout<<"\nread successed\n"<<endl;
		}
		cout<<"\nThis is a demo plugin to show how to read apo data."<<endl;

		float sum_x=0,sum_y=0,sum_z=0;
		float sum_distance=0;
		QList<float> mean_x,mean_y,mean_z;
		/*求虚拟中心*/
		for(int i =0;i<tmp_list.size();i++){
			sum_x=sum_x+tmp_list.at(i).x;
			sum_y=sum_y+tmp_list.at(i).y;
			sum_z=sum_z+tmp_list.at(i).z;
			if((i+1)%5==0){
				mean_x.append(sum_x/5);
				mean_y.append(sum_y/5);
				mean_z.append(sum_z/5);
				sum_x=0;sum_y=0;sum_z=0;
			}
		}
		
	
		/*求平均距离*/
		for(int i = 0;i<tmp_list.size();i++){
			cout<<"\ndistance"<<i+1<<"="<<sqrt(pow(tmp_list.at(i).x-mean_x.at(i/5),2)+pow(tmp_list.at(i).y-mean_y.at(i/5),2)+pow(tmp_list.at(i).z-mean_z.at(i/5),2))<<endl;
			sum_distance = sum_distance + sqrt(pow(tmp_list.at(i).x-mean_x.at(i/5),2)+pow(tmp_list.at(i).y-mean_y.at(i/5),2)+pow(tmp_list.at(i).z-mean_z.at(i/5),2));
			if((i+1)%5==0){
				float mean_distance = sum_distance/5;
				cout<<"\nThe mean distance is:"<<mean_distance<<endl;
				cout<<"*********************************************"<<endl;
				sum_distance=0;
			}
		}
		float mean_distance5 = sum_distance/tmp_list.size();

		//cout<<"\nThe mean distance is:"<<mean_distance<<endl;

		return mean_distance5;
	}
	else {
#ifndef DISABLE_V3D_MSG
		v3d_msg("The file type you specified is not supported. Please check.");
#endif
		return a;
	}
	
}