/* angle_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-6-5 : by LXF
 */
 
#include "v3d_message.h"
#include <iostream>
#include <vector>
#include "angle_detection_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(angle_detection, angle_detection);
bool angle_detection_main(NeuronTree &nt,int para);
void getChildNum(const NeuronTree &nt, vector<vector<V3DLONG> > &childs);
bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName);
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
QStringList angle_detection::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList angle_detection::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void angle_detection::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by LXF, 2018-6-5"));
	}
}

bool angle_detection::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("angle_detection"))
	{
        if(infiles.size()==0)
        {
            v3d_msg("there is no input files!");
            return false;
        }
        int para = atof(inparas.at(0));
        if(para!=0&&para!=1)
        {
            v3d_msg("paramter is wrong,0 is for obtuse angle and 1 is for acute angle,please try again!");
            return false;
        }
        QString fileOpenName = QString(infiles.at(0));
        NeuronTree nt = readSWC_file(fileOpenName);
        angle_detection_main(nt,para);


        //v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

bool angle_detection_main(NeuronTree &nt,int para)
{
    vector<vector<V3DLONG> > childs;
    childs = vector< vector<V3DLONG> >(nt.listNeuron.size(), vector<V3DLONG>() );
    getChildNum(nt,childs);

    QList<NeuronSWC> obtuse,acute,branch_obtuse,branch_acute,wrong_point;
    //getChildNum(nt,childs);
    int child_num;
    NeuronSWC son1;
    NeuronSWC son2;
    double angle;
    double dis_a,dis_b,dis_c;
    for(V3DLONG i=0;i<nt.listNeuron.size();i++)
    {
        cout<<"i = "<<i<<endl;
        NeuronSWC curr = nt.listNeuron[i];
        child_num = childs[nt.listNeuron[i].n].size();
        if(child_num==2)
        {
            son1 = nt.listNeuron[childs[nt.listNeuron[i].n][0]];
            son2 = nt.listNeuron[childs[nt.listNeuron[i].n][1]];

            dis_a = NTDIS(curr,son1);
            dis_b = NTDIS(curr,son2);
            dis_c = NTDIS(son1,son2);
            angle = ( (dis_a*dis_a) + (dis_b*dis_b) - (dis_c*dis_c ) )/(2*dis_a*dis_b);
            //cout<<"angle = "<<angle<<endl;
            if(angle<0)
            {
                branch_obtuse.push_back(nt.listNeuron[i+1]);
                obtuse.push_back(curr);
                obtuse.push_back(son1);
                obtuse.push_back(son2);
            }
            else
            {
                branch_acute.push_back(nt.listNeuron[i+1]);
                acute.push_back(curr);
                acute.push_back(son1);
                acute.push_back(son2);
            }




        }
        else if(child_num>2)
        {
            wrong_point.push_back(nt.listNeuron[i]);
            v3d_msg("Something wrong with auto result.");

        }


    }
    if(para == 0)
    {
        QList<ImageMarker> markerlist;
        QString name = "marker_test_obtuse.marker";
        QString name1 = "swc_test_obtuse.swc";
        for(V3DLONG i=0;i<branch_obtuse.size();i++)
        {
            ImageMarker m;
            m.x = branch_obtuse[i].x;
            m.y = branch_obtuse[i].y;
            m.z = branch_obtuse[i].z;
            markerlist.push_back(m);
        }
        writeMarker_file(name,markerlist);
        export_list2file(obtuse,name1,name1);
    }
    else
    {
        QList<ImageMarker> markerlist2;
        QString name2 = "marker_test_acute.marker";
        QString name3 = "swc_test_acute.swc";
        for(V3DLONG i=0;i<branch_acute.size();i++)
        {
            ImageMarker m;
            m.x = branch_acute[i].x;
            m.y = branch_acute[i].y;
            m.z = branch_acute[i].z;
            markerlist2.push_back(m);
        }
        writeMarker_file(name2,markerlist2);
       export_list2file(acute,name3,name3);
    }



}
void getChildNum(const NeuronTree &nt, vector<vector<V3DLONG> > &childs)
{
    V3DLONG nt_size=nt.listNeuron.size();
    for (V3DLONG i=0; i<nt_size;i++)
    {
        V3DLONG par = nt.listNeuron[i].pn;
        if (par<0) continue;
        childs[nt.hashNeuron.value(par)].push_back(i);

        //cout<<"i="<<i<<"   par="<<nt.hashNeuron.value(par)<<endl;
    }
}
bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
//	myfile<<"# generated by Vaa3D Plugin sort_neuron_swc"<<endl;
//	myfile<<"# source file(s): "<<fileOpenName<<endl;
    myfile<<"# id,type,x,y,z,r,pid"<<endl;
    for (V3DLONG i=0;i<lN.size();i++)
        myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

    file.close();
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<lN.size()<<endl;
    return true;
}
