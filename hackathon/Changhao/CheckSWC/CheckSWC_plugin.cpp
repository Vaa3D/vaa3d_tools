/* CheckSWC_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-11-18 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "CheckSWC_plugin.h"
//#include "get_sub_plugin.h"
//#include "loc_input.h"
#include "my_surf_objs.h"
//#include"neuron_tools.h"
#include <set>

using namespace std;
Q_EXPORT_PLUGIN2(CheckSWC, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("CheckSWC_plugin")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("CheckSWC_plugin"))
    {
        int flag = CheckSWC(callback,parent);
        if (flag == 1)
        {
            cout<<endl;
            cout<<"********************"<<endl;
            cout<<"*CheckSWC finished!*"<<endl;
            cout<<"********************"<<endl;
        }
        else
        {   v3d_msg("menu flag != 1"); return;}
    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-11-18"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
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

bool writeMarker_file(const QString & filename, LandmarkList & listMarker)
{
    FILE * fp = fopen(filename.toLatin1(), "wt");
    if (!fp)
    {
        return false;
    }

    fprintf(fp, "##x,y,z,radius,shape,name,comment, color_r,color_g,color_b\n");

    for (int i=0;i<listMarker.size(); i++)
    {
        LocationSimple  p_pt;
        p_pt = listMarker[i];
        fprintf(fp, "%5.3f, %5.3f, %5.3f, %d, %d, %s, %s, %d,%d,%d\n",
                // 090617 RZC: marker file is 1-based
                p_pt.x,
                p_pt.y,
                p_pt.z,
                int(p_pt.radius), p_pt.shape,
                p_pt.name, p_pt.comments,
                p_pt.color.r,p_pt.color.g,p_pt.color.b);
    }

    fclose(fp);
    return true;
}

int CheckSWC(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString inimg_file = callback.getPathTeraFly();
    QString file_name = inimg_file;
    QFileInfo info(file_name);

    //*************the default size of block;
    int BoxX = 90;
    int BoxY = 90;
    int BoxZ = 90;

    //************set update the dialog
    QDialog * dialog = new QDialog();

    QGridLayout * layout = new QGridLayout();

    //*************set spinbox
    QSpinBox * block_X = new QSpinBox();
    block_X->setRange(0,5000000);      //???
    block_X->setValue(BoxX);  //???

    QSpinBox * block_Y = new QSpinBox();
    block_Y->setRange(0,5000000);      //???
    block_Y->setValue(BoxY);  //???

    QSpinBox * block_Z = new QSpinBox();
    block_Z->setRange(0,5000000);      //???
    block_Z->setValue(BoxZ);  //???

    layout->addWidget(new QLabel("X of block"),0,0);
    layout->addWidget(block_X, 0,1,1,5);

    layout->addWidget(new QLabel("Y of block"),1,0);
    layout->addWidget(block_Y, 1,1,1,5);

    layout->addWidget(new QLabel("Z of block"),2,0);
    layout->addWidget(block_Z, 2,1,1,5);


    QHBoxLayout * hbox2 = new QHBoxLayout();
    QPushButton * ok = new QPushButton(" ok ");
    ok->setDefault(true);
    QPushButton * cancel = new QPushButton("cancel");
    hbox2->addWidget(cancel);
    hbox2->addWidget(ok);

    layout->addLayout(hbox2,6,0,1,6);
    dialog->setLayout(layout);
    QObject::connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    QObject::connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));

    //**************run the dialog
    if(dialog->exec() != QDialog::Accepted)
    {
        if (dialog)
        {
                delete dialog;
                dialog=0;
                cout<<"delete dialog"<<endl;
        }
        return -1;
    }

    //***************get the dialog return values
    BoxX = block_X->value(); //   block的大小
    BoxY = block_Y->value();
    BoxZ = block_Z->value();

    if (dialog) {delete dialog; dialog=0;}

    V3DLONG *sz = new V3DLONG[4];
    callback.getDimTeraFly(inimg_file.toStdString(),sz);
    cout<<"sz[0]:"<<sz[0]<<" sz[1]:"<<sz[1]<<" sz[2]:"<<sz[2]<<endl;


    int num_along_x=ceil((float)sz[0]/BoxX);
    int num_along_y=ceil((float)sz[1]/BoxY);
    int num_along_z=ceil((float)sz[2]/BoxZ);
    int NumberOfBlock=num_along_x*num_along_y*num_along_z; // block的总个数
    cout<<"num along x is "<< num_along_x << endl;
    cout<<"num along y is "<< num_along_y << endl;
    cout<<"num along z is "<< num_along_z << endl;
    cout<<"NumberOfBlock is "<< NumberOfBlock << endl;

    std::set <TBlock> blcok_flag;  //set容器可以不重复地添加元素


    vector<MyMarker*> outMarker; outMarker.clear();

    QStringList swc_list;

    NeuronTree nt = callback.getSWCTeraFly();
    int swcInWindows=0;
    if(nt.listNeuron.size()!=0)
    {   swcInWindows=1;}
    else
    {
        swcInWindows=0;
        swc_list=QFileDialog::getOpenFileNames(0,"Choose ESWC or SWC","D:\\",QObject::tr("*.swc"));
        if(swc_list.isEmpty()){return 0;}
        int input_as_swc;//0 for eswc, 1 for swc
        if(swc_list.at(0).endsWith(".eswc"))
        {   input_as_swc=0;}
        else{   input_as_swc=1;}
        if(input_as_swc)
        {
            for(int i=0;i<swc_list.size();i++)
            {
                if(!swc_list.at(i).endsWith(".swc"))
                {
                    swc_list.removeAt(i);
                }
            }
            for(int i=0;i<swc_list.size();i++)
            {
                cout<<swc_list.at(i).toUtf8().data()<<endl;
            }

            for(int i=0;i<swc_list.size();i++)
            {
                vector<MyMarker*> temp_swc;temp_swc.clear();
                readSWC_file(swc_list.at(i).toStdString(),temp_swc);
                cout<<"i :"<<i<<" temp_swc.size:"<<temp_swc.size()<<endl;
                for(int j=0;j<temp_swc.size();j++)
                {

                    TBlock temT; //定义临时存放标记的变量

                    temT.tx = ceil(temp_swc.at(j)->x/BoxX);
                    temT.ty = ceil(temp_swc.at(j)->y/BoxY);
                    temT.tz = ceil(temp_swc.at(j)->z/BoxZ);
                    temT.t = num_along_x*temT.ty*(temT.tz-1)+num_along_x*(temT.ty-1)+temT.tx; //当前点对应的block标号

                    if(temT.t >= NumberOfBlock){  cout<<"flase,exceed NumberOfBlock";}
                    else
                    {
                       blcok_flag.insert(temT); //把swc对应的blcok标号记下
                    }
                }
            }
        }
        else
        {
            //0 for eswc
            for(int i=0;i<swc_list.size();i++)
            {
                if(!swc_list.at(i).endsWith(".swc"))
                {
                    swc_list.removeAt(i);
                }
            }
            for(int i=0;i<swc_list.size();i++)
            {
                cout<<swc_list.at(i).toUtf8().data()<<endl;
            }

            for(int i=0;i<swc_list.size();i++)
            {
                vector<MyMarkerX*> temp_eswc;temp_eswc.clear();
                readESWC_file(swc_list.at(i).toStdString(),temp_eswc);
                cout<<"i :"<<i<<" temp_eswc.size:"<<temp_eswc.size()<<endl;
                for(int j=0;j<temp_eswc.size();j++)
                {
                    TBlock temT; //定义临时存放标记的变量

                    temT.tx = ceil(temp_eswc.at(j)->x/BoxX);
                    temT.ty = ceil(temp_eswc.at(j)->y/BoxY);
                    temT.tz = ceil(temp_eswc.at(j)->z/BoxZ);
                    temT.t = num_along_x*temT.ty*(temT.tz-1)+num_along_x*(temT.ty-1)+temT.tx; //当前点对应的block标号

                    if(temT.t >= NumberOfBlock){  cout<<"flase,exceed NumberOfBlock";}
                    else
                    {
                       blcok_flag.insert(temT); //把swc对应的blcok标号记下
                    }
                }
            }

        }

     }


    // 找block的中心点
    vector<MyMarker> Block_Markerset;
    MyMarker Block_Marker;
    LandmarkList curlist;
    LocationSimple s;
    for(set<TBlock>::iterator it = blcok_flag.begin(); it!= blcok_flag.end(); it++)//遍历有swc的block
    {

        Block_Marker.x=((double)(*it).tx-0.5)*BoxX;
        Block_Marker.y=((double)(*it).ty-0.5)*BoxY;
        Block_Marker.z=((double)(*it).tz-0.5)*BoxZ;
        Block_Markerset.push_back(Block_Marker);
//        cout<< "Block_Marker.t  " << (*it).t  ;
//        cout<< "  Block_Marker.x  " << Block_Marker.x  ;
//        cout<< "  Block_Marker.y  " << Block_Marker.y  ;
//        cout<< "  Block_Marker.z  " << Block_Marker.z << endl ;
        s.x= Block_Marker.x;
        s.y= Block_Marker.y;
        s.z= Block_Marker.z;
        s.radius=1;
        s.color = random_rgba8(255);
        curlist << s;
    }

    v3d_msg(QString("save %1 markers").arg(curlist.size()),0);

    QString outimg_dir = "";
    QString default_name = info.baseName()+"_for_sub.marker";
    outimg_dir = QFileDialog::getExistingDirectory(0,
                                  "Choose a dir to save file " );
    QString outimg_file = outimg_dir + "\\" + default_name;
    writeMarker_file(outimg_file,curlist);


    return 1;




}
