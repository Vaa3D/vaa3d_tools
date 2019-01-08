/* branch_crop_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-12-19 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "branch_crop_plugin.h"
#include <stdio.h>
#include <iostream>
//#include "../get_terminal.h"
using namespace std;
Q_EXPORT_PLUGIN2(branch_crop, TestPlugin)

void get_branches(V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
QList<int> get_branch_points(NeuronTree nt, bool include_root);

QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("get_branch_sample")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("get_branch_point")
        <<tr("branch_point_sample")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("get_branch_sample"))
	{
         v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2018-12-19"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
//	vector<char*> infiles, inparas, outfiles;
//	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
//	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
//	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("get_branch_point"))
	{
        get_branches(input,output,callback);
	}
    else if (func_name == tr("branch_point_sample"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
    {void get_branches(V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
        {
            vector<char*> infiles, inparas, outfiles;
            if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
            //if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
            //if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

        //    QString image_file=infiles.at(0);
        //    QString swc_file = infiles.at(1);
        //    QString output_dir=outfiles.at(0);
        //    XYZ block_size=XYZ(100,100,20);
            QString swc_file = infiles.at(0);

            printf("welcome to use get_branch\n");
            NeuronTree nt = readSWC_file(swc_file);
        //    if(!output_dir.endsWith("/")){
        //        output_dir = output_dir+"/";
        //    }
            QString cell_name = swc_file.right(swc_file.size()-swc_file.lastIndexOf("/")-1);
            cell_name = cell_name.left(cell_name.indexOf("."));

            // Find branch points
            QList<int> branch_list = get_branch_points(nt, false);
        //    cout<<"Number_of_tips\t"<<qPrintable(swc_file)<<"\t"<<tip_list.size()<<endl;

        //    // Crvoid get_branches(V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
            {
                vector<char*> infiles, inparas, outfiles;
                if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
                //if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
                //if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

            //    QString image_file=infiles.at(0);
            //    QString swc_file = infiles.at(1);
            //    QString output_dir=outfiles.at(0);
            //    XYZ block_size=XYZ(100,100,20);
                QString swc_file = infiles.at(0);

                printf("welcome to use get_branch\n");
                NeuronTree nt = readSWC_file(swc_file);
            //    if(!output_dir.endsWith("/")){
            //        output_dir = output_dir+"/";
            //    }
                QString cell_name = swc_file.right(swc_file.size()-swc_file.lastIndexOf("/")-1);
                cell_name = cell_name.left(cell_name.indexOf("."));

                // Find branch points
                QList<int> branch_list = get_branch_points(nt, false);
            //    cout<<"Number_of_tips\t"<<qPrintable(swc_file)<<"\t"<<tip_list.size()<<endl;

            //    // Crop tip-centered regions one by one
            //    block zcenter_block; // This is a block centered at (0,0,0)
            //    zcenter_block.small = 0-block_size/2;
            //    zcenter_block.large = block_size/2;
            //    QList<QString> output_suffix;
            //    output_suffix.append(QString("nrrd"));
            //    output_suffix.append(QString("swc"));

            //    for(int i=0; i<tip_list.size(); i++){
            ////        if(i>0){break;}
            //        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
            //        qDebug()<<node.n;
            //        if(node.type > 5){continue;}
            //        // create a tip-centered block
            //        XYZ shift;
            //        shift.x = (int)node.x;
            //        shift.y = (int)node.y;
            //        shift.z = (int)node.z;

            //        block crop_block = offset_block(zcenter_block, shift);
            //        crop_block.name = cell_name + "_"+QString::number(i);
            //        // crop image
            //        qDebug()<<crop_block.name;
            //        qDebug()<<crop_block.small.x<<crop_block.small.y<<crop_block.small.z;
            //        qDebug()<<crop_block.large.x<<crop_block.large.y<<crop_block.large.z;

            //        crop_img(image_file, crop_block, output_dir, callback, QString(".nrrd"));
            //        // crop swc
            //        QString output_swc = output_dir+crop_block.name+".swc";
            //        crop_swc(swc_file, output_swc, crop_block);
            ////        my_saveANO(output_dir, crop_block.name, output_suffix);
            //    }
                return;
            }

            QList<int> get_branch_points(NeuronTree nt, bool include_root){
                QList<int> branch_list;
                QList<int> plist;
                QList<int> alln;
                int N=nt.listNeuron.size();
                for(int i=0; i<N; i++){
                     cout << nt.listNeuron.at(i).n;
                }
                return branch_list;
            }
op tip-centered regions one by one
        //    block zcenter_block; // This is a block centered at (0,0,0)
        //    zcenter_block.small = 0-block_size/2;
        //    zcenter_block.large = block_size/2;
        //    QList<QString> output_suffix;
        //    output_suffix.append(QString("nrrd"));
        //    output_suffix.append(QString("swc"));

        //    for(int i=0; i<tip_list.size(); i++){
        ////        if(i>0){break;}
        //        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
        //        qDebug()<<node.n;
        //        if(node.type > 5){continue;}
        //        // create a tip-centered block
        //        XYZ shift;
        //        shift.x = (int)node.x;
        //        shift.y = (int)node.y;
        //        shift.z = (int)node.z;

        //        block crop_block = offset_block(zcenter_block, shift);
        //        crop_block.name = cell_name + "_"+QString::number(i);
        //        // crop image
        //        qDebug()<<crop_block.name;
        //        qDebug()<<crop_block.small.x<<crop_block.small.y<<crop_block.small.z;
        //        qDebug()<<crop_block.large.x<<crop_block.large.y<<crop_block.large.z;

        //        crop_img(image_file, crop_block, output_dir, callback, QString(".nrrd"));
        //        // crop swc
        //        QString output_swc = output_dir+crop_block.name+".swc";
        //        crop_swc(swc_file, output_swc, crop_block);
        ////        my_saveANO(output_dir, crop_block.name, output_suffix);
        //    }
            return;
        }

        QList<int> get_branch_points(NeuronTree nt, bool include_root){
            QList<int> branch_list;
            QList<int> plist;
            QList<int> alln;
            int N=nt.listNeuron.size();
            for(int i=0; i<N; i++){
                 cout << nt.listNeuron.at(i).n;
            }
            return branch_list;
        }

		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}
void get_branches(V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    //if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    //if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

//    QString image_file=infiles.at(0);
//    QString swc_file = infiles.at(1);
//    QString output_dir=outfiles.at(0);
//    XYZ block_size=XYZ(100,100,20);
    QString swc_file = infiles.at(0);

    printf("welcome to use get_branch\n");
    NeuronTree nt = readSWC_file(swc_file);
//    if(!output_dir.endsWith("/")){
//        output_dir = output_dir+"/";
//    }
    QString cell_name = swc_file.right(swc_file.size()-swc_file.lastIndexOf("/")-1);
    cell_name = cell_name.left(cell_name.indexOf("."));

    // Find branch points
    QList<int> branch_list = get_branch_points(nt, false);
//    cout<<"Number_of_tips\t"<<qPrintable(swc_file)<<"\t"<<tip_list.size()<<endl;

//    // Crop tip-centered regions one by one
//    block zcenter_block; // This is a block centered at (0,0,0)
//    zcenter_block.small = 0-block_size/2;
//    zcenter_block.large = block_size/2;
//    QList<QString> output_suffix;
//    output_suffix.append(QString("nrrd"));
//    output_suffix.append(QString("swc"));

//    for(int i=0; i<tip_list.size(); i++){
////        if(i>0){break;}
//        NeuronSWC node = nt.listNeuron.at(tip_list.at(i));
//        qDebug()<<node.n;
//        if(node.type > 5){continue;}
//        // create a tip-centered block
//        XYZ shift;
//        shift.x = (int)node.x;
//        shift.y = (int)node.y;
//        shift.z = (int)node.z;

//        block crop_block = offset_block(zcenter_block, shift);
//        crop_block.name = cell_name + "_"+QString::number(i);
//        // crop image
//        qDebug()<<crop_block.name;
//        qDebug()<<crop_block.small.x<<crop_block.small.y<<crop_block.small.z;
//        qDebug()<<crop_block.large.x<<crop_block.large.y<<crop_block.large.z;

//        crop_img(image_file, crop_block, output_dir, callback, QString(".nrrd"));
//        // crop swc
//        QString output_swc = output_dir+crop_block.name+".swc";
//        crop_swc(swc_file, output_swc, crop_block);
////        my_saveANO(output_dir, crop_block.name, output_suffix);
//    }
    return;
}

QList<int> get_branch_points(NeuronTree nt, bool include_root){
    QList<int> branch_list;
    QList<int> plist;
    QList<int> alln;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
         cout << nt.listNeuron.at(i).n;
    }
    return branch_list;
}

