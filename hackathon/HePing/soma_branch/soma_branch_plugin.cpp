/* soma_branch_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-11-20 : by heping
 */
 
#include "v3d_message.h"
#include <vector>
#include "soma_branch_plugin.h"
#include "branch_count_soma.h"
#include<v3d_interface.h>
#include<eigen_3_3_4/Eigen/Dense>
#include<fstream>
#include<direct.h>
#include<io.h>
using namespace std;
//extern template<class T> bool compute_marker_pca_hp(vector<T> markers,V3DLONG r,double &pc1,double &pc2,double &pc3);
Q_EXPORT_PLUGIN2(soma_branch, BranchCount);
 
QStringList BranchCount::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList BranchCount::funclist() const
{
	return QStringList()
        <<tr("branch_count")
		<<tr("func2")
		<<tr("help");
}

void BranchCount::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
        //branch_count_menu(callback,parent);
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by heping, 2019-11-20"));
	}
}

bool BranchCount::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("branch_count"))
	{
        //QString swcfile=(infiles.size()>=1)?infiles[0]:"";
        QString apofile=(outfiles.size()>=1)?outfiles[0]:"";

        if(_access(apofile.toStdString().c_str(),0)==-1)
            _mkdir(apofile.toStdString().c_str());
        string path=apofile.toStdString()+"//data.txt";
        ofstream datafile(path,ios::app);
        if(datafile.fail()){
            qDebug()<<"file can't open!";
        }

        for(int i=0;i<infiles.size();i++){
            QString swcfile=infiles[i];
            NeuronTree nt=readSWC_file(swcfile);
            SWCTree t;
            QList<ImageMarker> markers;
            vector<location> points;
            double pc1=0,pc2=0,pc3=0;
            double pc13=0;
            double max_radius;
            QFileInfo eswcfileinfo;
            eswcfileinfo=QFileInfo(swcfile);
            QString eswcfile=eswcfileinfo.fileName();
            eswcfile.mid(0,eswcfile.indexOf("."));
            t.count_branch_location(nt,markers,points,max_radius);
            V3DLONG max_r=(V3DLONG)(max_radius+1);
            compute_marker_pca_hp(points,max_r,pc1,pc2,pc3);
            qDebug()<<pc1<<pc2<<pc3;
            if(pc1==0||pc2==0||pc3==0){
                qDebug()<<"it is pc problem!"<<endl;
                pc13=1;
            }
            else{
                double num1,num2;//min,max
                num1=(pc1<pc2)?pc1:pc2;
                num1=(num1<pc3)?num1:pc3;
                num2=(pc1<pc2)?pc2:pc1;
                num2=(num2<pc3)?pc3:num2;
                pc13=num1/num2;
            }
            qDebug()<<"sigma1/sigma2:"<<pc13;
            //datafile<<markers.size()<<" "<<pc13<<endl;
            datafile<<pc13<<endl;
    //        bool flag=four_point(points);
    //        if(flag==true) qDebug()<<"yes!";
    //        else qDebug()<<"no!";
            qDebug()<<"branch num:"<<markers.size();
            writeMarker_file(apofile+"//"+eswcfile+".marker",markers);
        }
        datafile.close();


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

