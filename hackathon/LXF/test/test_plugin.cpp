/* test_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-6-21 : by LXF
 */
 
#include "v3d_message.h"
#include <vector>
//#include "basic_surf_objs.h"

#include "test_plugin.h"
#include<map>
#include"math.h"
#include<iostream>

#include "find_feature.h"
#include "match_swc.h"
//#include "sim_measure.h"


Q_EXPORT_PLUGIN2(test, TestPlugin);

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
    NeuronTree nt;
    NeuronTree listLandmarks;
};


void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu);
//void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu,QStringList &name_list);
int KNN(input_PARA &PARA,double DIST);
NeuronTree match(input_PARA &PARA,double DIST,NeuronTree nt_output);
NeuronTree find_most_point(input_PARA &PARA,NeuronTree nt_in);
vector<V3DLONG> BubbleSort(vector<V3DLONG> branches, int size_branches);
//int BubbleSort_for_int(int branches[], int size_branches);
int find_biggest(vector<V3DLONG> branches,int size_branches);
int find_shortest(vector<V3DLONG> branches,int size_branches);
void match_swc(const NeuronTree &nt_raw, const NeuronTree &mk_raw, NeuronTree &s_mk, vector<NeuronTree> s_forest);



 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("tracing_menu")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("tracing_func")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("tracing_menu"))
	{
        bool bmenu = true;
        input_PARA PARA;
        QStringList name_list;
        ml_func(callback,parent,PARA,bmenu);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by LXF, 2017-6-21"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    printf("****************************this is start************************************");
	if (func_name == tr("tracing_func"))
	{
        bool bmenu = false;
        input_PARA PARA;

        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        if(infiles.empty())
        {
            fprintf (stderr, "Need input image. \n");
            return false;
        }
        else
            PARA.inimg_file = infiles[0];
        int k=0;
        PARA.channel = (paras.size() >= k+1) ? atoi(paras[k]) : 1;  k++;
        QString inneuron_file = (paras.size() >= k+1) ? paras[k] : "";k++;
        if(!inneuron_file.isEmpty())
            PARA.nt = readSWC_file(inneuron_file);
     //   QStringList name_list;
    //    name_list.append(inneuron_file);
        printf("****************************readSWC_1_done************************************");
        QString inmarker_file = paras.empty() ? "" : paras[k]; if(inmarker_file == "NULL") inmarker_file = ""; k++;
        NeuronTree file_inmarkers;
        if(!inmarker_file.isEmpty())
            file_inmarkers = readSWC_file(inmarker_file);




       // QString swc_name = PARA.inimg_file + "_temp.swc";


            //writeSWC_file(swc_name.toStdString().c_str(),file_inmarkers);

        printf("****************************readSWC_2_done************************************");

        NeuronSWC t;
        for(int i = 0; i < file_inmarkers.listNeuron.size(); i++)
        {
            t.n = file_inmarkers.listNeuron[i].n;
            t.x = file_inmarkers.listNeuron[i].x;
            t.y = file_inmarkers.listNeuron[i].y;
            t.z = file_inmarkers.listNeuron[i].z;
            t.r = file_inmarkers.listNeuron[i].r;
            t.pn = file_inmarkers.listNeuron[i].pn;
            PARA.listLandmarks.listNeuron.append(t);
        }

        ml_func(callback,parent,PARA,bmenu);
	}
    else if (func_name == tr("help"))
    {

        ////HERE IS WHERE THE DEVELOPERS SHOULD UPDATE THE USAGE OF THE PLUGIN


		printf("**** Usage of test tracing **** \n");
        printf("vaa3d -x test -f tracing_func -i <inimg_file> -p <channel> <swc_file> <swc_file> <other parameters>\n");
        printf("inimg_file       The input image\n");
        printf("channel          Data channel for tracing. Start from 1 (default 1).\n");
        printf("swc_file         SWC file path.\n");
        printf("marker_file      Marker file path.\n");

        printf("outswc_file      Will be named automatically based on the input image file name, so you don't have to specify it.\n");
        printf("outmarker_file   Will be named automatically based on the input image file name, so you don't have to specify it.\n\n");

	}
	else return false;

	return true;
}

void ml_func(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA, bool bmenu)
{
    printf("****************************enter into ml_func************************************");
    unsigned char* data1d = 0;
    V3DLONG N,M,P,sc,c;
    V3DLONG in_sz[4];
    printf("****************************this is 1************************************");
    if(bmenu)
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have any image open in the main window.");
            return;
        }
        Image4DSimple* p4DImage = callback.getImage(curwin);
        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }

        data1d = p4DImage->getRawData();
        N = p4DImage->getXDim();
        M = p4DImage->getYDim();
        P = p4DImage->getZDim();
        sc = p4DImage->getCDim();
        bool ok1;

        if(sc==1)
        {
            c=1;
            ok1=true;
        }
        else
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                             "Enter channel NO:",
                                             1, 1, sc, 1, &ok1);
        }

        if(!ok1)
            return;

        PARA.listLandmarks = callback.getSWC(curwin);
        PARA.nt = callback.getSWC(curwin);


        in_sz[0] = N;
        in_sz[1] = M;
        in_sz[2] = P;
        in_sz[3] = sc;


        PARA.inimg_file = p4DImage->getFileName();
    }
    else
    {
        int datatype = 0;
        printf("****************************this is 2************************************");
        if (!simple_loadimage_wrapper(callback,PARA.inimg_file.toStdString().c_str(), data1d, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",PARA.inimg_file.toStdString().c_str());
            return;
        }
        printf("****************************this is 3************************************");
        if(PARA.channel < 1 || PARA.channel > in_sz[3])
        {
            fprintf (stderr, "Invalid channel number. \n");
            return;
        }
        printf("****************************this is 4************************************");

        N = in_sz[0];
        M = in_sz[1];
        P = in_sz[2];
        sc = in_sz[3];
        c = PARA.channel;

    }
    //main neuron machine learning code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON MACHINE LEARNING CODE    


    cout<<"******************This is main function*********************"<<endl;

    vector<NeuronTree> s_forest;
    NeuronTree s_mk;

    cout<<"******************This is match_swc*********************"<<endl;

    //match_swc(PARA.nt,PARA.listLandmarks,s_mk,s_forest);
   // vector<vector<V3DLONG> > retrieved_all;
   // vector<NeuronTree> result_out;
    vector<V3DLONG> num_out;

   // double* m_feature_morph,m_feature_gmi;


    QList<double*> v_morph_list,v_gmi_list,m_morph_list,m_gmi_list;
     cout<<"******************This is match*********************"<<endl;
   // match_little_pattern(s_forest,s_mk,retrieved_all,R);
    // s_forest.push_back(PARA.listLandmarks);
   // match_little_pattern(s_forest,PARA.listLandmarks,num_out,m_feature_morph,m_feature_gmi,v_morph_list,v_gmi_list);
 s_forest.push_back(PARA.nt);
 //match_little_pattern(s_forest,PARA.listLandmarks,num_out,m_morph_list,m_gmi_list,v_morph_list,v_gmi_list);
 get_feature(PARA.nt,PARA.listLandmarks,m_morph_list,m_gmi_list,v_morph_list,v_gmi_list);

//cout<<"size = "<<v_morph_list.size();

// for(V3DLONG i=0;i<m_morph_list.size();i++)
// {
//     for(V3DLONG i = 0;i < 21;i++)
//     {
//         cout<<"v_feature["<<i<<"]= "<<v_feature_morph[i]<<endl;
//     }
// }








cout<<"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&         tvxqtvxq"<<endl;















    //Output
  //  NeuronTree nt_output,result,nt_in;
/*

    QString swc_name = PARA.inimg_file + "_test.swc";
  //  result_out.name = "test";
    cout<<"sizeof out = "<<result_out.size()<<endl;
    for(V3DLONG i = 0;i<result_out.size();i++)
    {
        writeSWC_file(swc_name.toStdString().c_str(),result_out[i]);
    }
    cout<<"*********************SWC has been generated********************"<<endl;*/
    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

  //  if(nt_output.listNeuron.size()>0) v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);
  //  if(marker_output.size()>0) v3d_msg(QString("Now you can drag and drop the generated marker fle [%1] into Vaa3D.").arg(marker_name.toStdString().c_str()),bmenu);


    return;
}


















































NeuronTree find_most_point(input_PARA &PARA,NeuronTree nt_in)
{

    cout<<"***********************************************enter into find most point****************************"<<endl;

    NeuronSWC s;
   // V3DLONG max_bub,min_bub;


    V3DLONG siz1 = PARA.listLandmarks.listNeuron.size();
    vector<V3DLONG > cup_1;
    cup_1.clear();
    cout<<"siz1="<<siz1<<endl;
    vector<V3DLONG> branches_1; //number of branches on the pnt: 0-tip, 1-internal, >=2-branch
    branches_1.clear();


    cout<<"***********************************************this is hahaha***************************"<<endl;


    for (V3DLONG i=0;i<siz1;i++)
    {
        cout<<"***********************************************branches_and_cup_1****************************"<<endl;
        if (PARA.listLandmarks.listNeuron[i].pn<0) continue;

       // V3DLONG pid = PARA.listLandmarks.hashNeuron.value(PARA.listLandmarks.listNeuron[i].pn);






        V3DLONG pid1 = PARA.listLandmarks.listNeuron[i].pn;
        cout<<"PARA.listLandmarks.listNeuron["<<i<<"].pn = "<<PARA.listLandmarks.listNeuron[i].pn<<endl;
        cout<<"branches_1["<<pid1<<"] = "<<branches_1[pid1]<<endl;
        cout<<"pid1 = "<<pid1<<endl;
        branches_1[pid1]++;

        cout<<"branches_1["<<pid1<<"] = "<<branches_1[pid1]<<endl;
        cout<<"i = "<<i<<endl;
        cup_1.push_back(pid1);
        cout<<"cup_1 = "<<cup_1[i-1]<<endl;

    }

    cout<<"***********************************************enter into out_for1****************************"<<endl;

/*

    V3DLONG siz2 = PARA.nt.listNeuron.size();
    vector<V3DLONG > cup_2(siz2,0);
    vector<V3DLONG> branches_2(siz2,0); //number of branches on the pnt: 0-tip, 1-internal, >=2-branch
    for (V3DLONG i=0;i<siz2;i++)
    {
       // int j=0;
        if (PARA.nt.listNeuron[i].pn<0) continue;
         V3DLONG pid2 = PARA.nt.listNeuron[i].pn;
        //V3DLONG pid2 = PARA.listLandmarks.hashNeuron.value(PARA.listLandmarks.listNeuron[i].pn);
         cout<<"PARA.listLandmarks.listNeuron["<<i<<"].pn = "<<PARA.nt.listNeuron[i].pn<<endl;
         cout<<"branches_2["<<pid2<<"] = "<<branches_2[pid2]<<endl;
        branches_2[pid2]++;

        cout<<"branches_2["<<pid2<<"] = "<<branches_2[pid2]<<endl;
        cout<<"i = "<<i<<endl;
        cup_2.push_back(pid2);
        cout<<"cup_2 = "<<cup_2[i-1]<<endl;
    }


    cout<<"***********************************************enter into out_for2****************************"<<endl;

    for(int i=find_shortest(cup_2,siz2);i<find_biggest(cup_2,siz2);i++)
    {
        if(find_biggest(branches_1,siz1) == branches_2[i])
        {
            {

               s.n = PARA.nt.listNeuron[i].n;
               s.type = PARA.nt.listNeuron[i].type;
               s.x = PARA.nt.listNeuron[i].x;
               s.y = PARA.nt.listNeuron[i].y;
               s.z = PARA.nt.listNeuron[i].z;
               s.r = PARA.nt.listNeuron[i].r;
               s.pn = PARA.nt.listNeuron[i].pn;
               nt_in.listNeuron.append(s);
            }
        }
    }
    */
/*
    for(V3DLONG i =0;i< size_branches_2;i++)
    {
        if( num == branches_2[i])
        {
           NeuronSWC s;
           s.n = PARA.nt.listNeuron[i].n;
           s.type = PARA.nt.listNeuron[i].type;
           s.x = PARA.nt.listNeuron[i].x;
           s.y = PARA.nt.listNeuron[i].y;
           s.z = PARA.nt.listNeuron[i].z;
           s.r = PARA.nt.listNeuron[i].r;
           s.pn = PARA.nt.listNeuron[i].pn;
           nt_in.listNeuron.append(s);
        }
    }
    return nt_in;

*/
    return nt_in;
}
int KNN(input_PARA &PARA,double DIST)
{
    cout<<"size = "<<PARA.listLandmarks.listNeuron.size()<<endl;
   // LocationSimple re;
        int j=0;

        for(int i = 0 ; i < PARA.listLandmarks.listNeuron.size()-2; i = i + 2)
        {
            cout<<"**********************************this is enter into for_in_knn***********************"<<endl;
            V3DLONG dist1;
        //    DIST = 0;
            cout<<"i="<<i<<endl;
            dist1 = sqrt((PARA.listLandmarks.listNeuron[i].x-PARA.listLandmarks.listNeuron[i+2].x)*(PARA.listLandmarks.listNeuron[i].x-PARA.listLandmarks.listNeuron[i+2].x)
                    +(PARA.listLandmarks.listNeuron[i].y-PARA.listLandmarks.listNeuron[i+2].y)*(PARA.listLandmarks.listNeuron[i].y-PARA.listLandmarks.listNeuron[i+2].y)
                    +(PARA.listLandmarks.listNeuron[i].z-PARA.listLandmarks.listNeuron[i+2].z)*(PARA.listLandmarks.listNeuron[i].z-PARA.listLandmarks.listNeuron[i+2].z));
            cout<<"dist1="<<dist1<<endl;
            DIST = DIST + dist1;
            j++;
          //  printf("DIST=",DIST);
            cout<<"DIST="<<DIST<<endl;
            cout<<"j = "<<j<<endl;
        }
        DIST = DIST/j;
cout<<"DIST after /="<<DIST<<endl;

    return DIST;

}

NeuronTree match(input_PARA &PARA,double DIST,NeuronTree result)
{

        NeuronSWC s;
        for(int i = 0; i < PARA.nt.listNeuron.size(); i=i+DIST)
        {
           // cout<<"i="<<i<<endl;
            s.n = PARA.nt.listNeuron[i].n;
            s.type = PARA.nt.listNeuron[i].type;
            s.x = PARA.nt.listNeuron[i].x;
            s.y = PARA.nt.listNeuron[i].y;
            s.z = PARA.nt.listNeuron[i].z;
            s.r = PARA.nt.listNeuron[i].r;
            s.pn = PARA.nt.listNeuron[i].pn;
            result.listNeuron.append(s);
        }



//        map<int,int> mymap;
//        int i;
//        mymap.insert(pair<int,int>(i,PARA.nt.listNeuron.pn));




        return result;

}



vector<V3DLONG> BubbleSort(vector<V3DLONG> branches, int size_branches)
{
    V3DLONG i,j;
     //V3DLONG tmp;
    for(i=0; i<size_branches-1; i++)
    {
       // tmp = branches[i];

        for(j=0; j>size_branches-i-1; j--)
        {
            //找到数组中最小的数，并交换
            if(branches[j] > branches[j+1])
            {
                int temp;
                temp = branches[j];
                branches[j] = branches[j+1];
                branches[j+1] = temp;


               // branches[i] = branches[j];
               // branches[j] = tmp;
               // tmp = branches[i];
            }
        }
        cout<<"branches="<<branches[i]<<endl;
    }



    for(V3DLONG i=0;i< size_branches;i++)
    {

            cout<<"i = "<<i<<endl;
            cout<<"***********************************this is output vector_branches****************************************"<<endl;
            cout<<branches[i]<<endl;

    }

    cout<<"branches[0]="<<branches[0]<<endl;
    cout<<"branches[]="<<branches[size_branches-1]<<endl;

//    vector<V3DLONG> a;
//    V3DLONG b=2;
//    a.push_back(b);

    return branches;
}
/*
int BubbleSort_for_int(int branches[], int size_branches)
{
    V3DLONG i,j;
     V3DLONG tmp;
    for(i=0; i<size_branches-1; i++)
    {
        tmp = branches[i];

        for(j=size_branches-1; j>i; j--)
        {
            //找到数组中最小的数，并交换
            if(tmp > branches[j])
            {
                branches[i] = branches[j];
                branches[j] = tmp;
                tmp = branches[i];
            }
        }
    }

    return branches;
}
*/

//struct Link{
//    int count[];
//    int pn[];
//};

int find_biggest(vector<V3DLONG> branches,int size_branches)
{
    int max_b= 0;
    for(int i=0;i<size_branches;i++)
    {
        if(branches[i]>max_b)
        {
            max_b = branches[i];
        }
    }
    return max_b;
}

int find_shortest(vector<V3DLONG> branches,int size_branches)
{
    int min_s= 0;
    for(int i=0;i<size_branches;i++)
    {
        if(branches[i]<min_s)
        {
            min_s = branches[i];
        }
    }
    return min_s;
}
