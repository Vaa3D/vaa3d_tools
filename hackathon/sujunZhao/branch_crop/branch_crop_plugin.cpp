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
//op tip-centered regions one by one
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

vector<int> get_branch_points(NeuronTree nt, bool include_root){
    vector<int> branch_list;
    QList<int> plist;
    map<int, int> t;
    vector< vector<int> > childlist;
    int N=nt.listNeuron.size();
    for(int i=0; i<N; i++){
        //qDebug() << nt.listNeuron.at(i).n << nt.listNeuron.at(i).pn;
        plist.append(nt.listNeuron.at(i).pn);
        t.insert(pair<int,int>(plist.at(i),0));
    }
    for(int i=0; i<N; i++){

        t.at(plist.at(i)) = t.at(plist.at(i))+1;
        if((plist.count(plist.at(i))>1)&(t.at(plist.at(i)) == 1)){
            branch_list.push_back(i);
        }
    }
    // branch points filtering
    // 1. based on distances
    //int n_branch = branch_list.size();
    //for(int i=0;i<n_branch;i++){
    //    p = branch_list.at(i);
    //    for(int j=0; j<3;j++){
    //    }
    //}

    return branch_list;
}

// find other branch points in the same cropped block
vector< vector<int> > get_close_points(NeuronTree nt,vector<int> a){
    vector< vector<int> > neighbours;
    int n=a.size();
    for(int i=0; i<n; i++){
        vector<int> cp;
        NeuronSWC node1 = nt.listNeuron.at(a.at(i));
        int min_x = 50;
        int min_y = 50;
        int min_z = 10;
        for(int j=0; j<n; j++){
            NeuronSWC node2 = nt.listNeuron.at(a.at(j));
            if(i != j){
                if(abs(node1.x-node2.x) < min_x){
                    min_x = ceil(abs(node1.x-node2.x));
                }
                if(abs(node1.y-node2.y) < min_y){
                    min_y = ceil(abs(node1.y-node2.y));
                }
                if(abs(node1.z-node2.z) < min_z){
                    min_z = ceil(abs(node1.z-node2.z));
                }
           }
        }
        if(min_x<25){
            min_x = 25;
        }
        if(min_y<25){
            min_y = 25;
        }
        if(min_z<5){
            min_z = 5;
        }
        cp.push_back(min_x);
        cp.push_back(min_y);
        cp.push_back(min_z);
        neighbours.push_back(cp);
    }
    return neighbours;
}

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

XYZ offset_XYZ(XYZ input, XYZ offset){
    input.x += offset.x;
    input.y += offset.y;
    input.z += offset.z;
    return input;
}getXDim

void crop_swc(QString input_swc, QString output_swc, block crop_block)
{

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

void get2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    QString input_swc=infiles.at(0);
    QString input_image=inparas.at(0);
    QString output_2d_dir=outfiles.at(0);
    if(!output_2d_dir.endsWith("/")){
        output_2d_dir = output_2d_dir+"/";
    }
    QStringList list=input_swc.split("/");
    QString flag=list.last(); QStringList list1=flag.split(".");// you don't need to add 1 to find the string you want in input_dir
    QString flag1=list1.first();//dont need to match list index
    //printf("______________:%s\n",output_2d_dir.data());
    qDebug()<<input_swc;
    qDebug("number:%s",qPrintable(flag1));
    NeuronTree nt_crop_sorted=readSWC_file(input_swc);
    Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(input_image) ));
    int nChannel = p4dImage->getCDim();

    V3DLONG mysz[4];
    mysz[0] = p4dImage->getXDim();
    mysz[1] = p4dImage->getYDim();
    mysz[2] = p4dImage->getZDim();
    mysz[3] = nChannel;
    cout<<mysz[0]<<endl<<mysz[1]<<endl<<mysz[2]<<endl<<mysz[3]<<endl;
    unsigned char *data1d_crop=p4dImage->getRawDataAtChannel(nChannel);
    //printf("+++++++++++:%p\n",p4dImage);

   V3DLONG pagesz = mysz[0]*mysz[1]*mysz[2];
   unsigned char* data1d_mask = 0;
   data1d_mask = new unsigned char [pagesz];
   memset(data1d_mask,0,pagesz*sizeof(unsigned char));
   double margin=0;//by PHC 20170531
   ComputemaskImage(nt_crop_sorted, data1d_mask, mysz[0], mysz[1], mysz[2], margin);
   //QString labelSaveString = pathname + ".v3draw_label.tif";
   //simple_saveimage_wrapper(callback, labelSaveString.toLatin1().data(),(unsigned char *)data1d_mask, mysz, 1);

   V3DLONG stacksz =mysz[0]*mysz[1];
   unsigned char *image_mip=0;
   image_mip = new unsigned char [stacksz];//2D orignal image
   unsigned char *label_mip=0;
   label_mip = new unsigned char [stacksz];//2D annotation
   for(V3DLONG iy = 0; iy < mysz[1]; iy++)
   {
       V3DLONG offsetj = iy*mysz[0];
       for(V3DLONG ix = 0; ix < mysz[0]; ix++)
       {
           int max_mip = 0;
           int max_label = 0;
           for(V3DLONG iz = 0; iz < mysz[2]; iz++)
           {
               V3DLONG offsetk = iz*mysz[1]*mysz[0];
               if(data1d_crop[offsetk + offsetj + ix] >= max_mip)
               {
                   image_mip[iy*mysz[0] + ix] = data1d_crop[offsetk + offsetj + ix];
                   max_mip = data1d_crop[offsetk + offsetj + ix];
               }
               if(data1d_mask[offsetk + offsetj + ix] >= max_label)
               {
                   label_mip[iy*mysz[0] + ix] = data1d_mask[offsetk + offsetj + ix];
                   max_label = data1d_mask[offsetk + offsetj + ix];
               }
           }
       }
   }
   unsigned char* data1d_2D = 0;
   data1d_2D = new unsigned char [3*stacksz];//3 channels image
   for(V3DLONG i=0; i<stacksz; i++)
       data1d_2D[i] = image_mip[i];

   for(V3DLONG i=0; i<stacksz; i++)
   {
       data1d_2D[i+stacksz] = (label_mip[i] ==255) ? 255: image_mip[i];
   }
   for(V3DLONG i=0; i<stacksz; i++)
       data1d_2D[i+2*stacksz] = image_mip[i];

   mysz[2] = 1;
   mysz[3] = 3;
   QString mipoutpuut = output_2d_dir +flag1+"_"+"mip.tif";
   simple_saveimage_wrapper(callback,mipoutpuut.toStdString().c_str(),(unsigned char *)data1d_2D,mysz,1);
   if(data1d_crop) {delete [] data1d_crop; data1d_crop=0;}
   if(data1d_mask) {delete [] data1d_mask; data1d_mask=0;}
   if(data1d_2D) {delete [] data1d_2D; data1d_2D=0;}
   if(image_mip) {delete [] image_mip; image_mip=0;}
   if(label_mip) {delete [] label_mip; label_mip=0;}
   //listNeuron.clear();
}

// check missing branches
vector<int> MissingBranch(QString input_swc, QString image){
    // choose the search range
    Image4DSimple * p4dImage = callback.loadImage((char *)(qPrintable(image) ));
    int nChannel = p4dImage->getCDim();

    V3DLONG mysz[4];
    mysz[0] = p4dImage->getXDim();
    mysz[1] = p4dImage->getYDim();
    mysz[2] = p4dImage->getZDim();
    mysz[3] = nChannel;

    NeuronTree nt = readSWC_file(swc_file);




}
