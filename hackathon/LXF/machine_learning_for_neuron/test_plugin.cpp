/* test_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-6-21 : by LXF
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"

#include "test_plugin.h"
#include<map>
#include"math.h"
#include<iostream>

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

//template <class T> Point();
//template <class T> Image3();

int KNN(input_PARA &PARA,double DIST);
NeuronTree match(input_PARA &PARA,double DIST,NeuronTree nt_output);
NeuronTree find_most_point(input_PARA &PARA,NeuronTree nt_in);
vector<V3DLONG> BubbleSort(vector<V3DLONG> branches, int size_branches);
//int BubbleSort_for_int(int branches[], int size_branches);
int find_biggest(vector<V3DLONG> branches,int size_branches);
int find_shortest(vector<V3DLONG> branches,int size_branches);

/*
template <typename T>
class Point {
 public:
  T x = 0;
  T y = 0;
  T z = 0;

  Point(T x, T y, T z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  Point(long idx1d, long *dims) {  // Plays as ind to sub
    long chunk = dims[0] * dims[1];
    z = idx1d / chunk;
    idx1d -= z * chunk;

    y = idx1d / dims[0];
    idx1d -= y * dims[0];

    x = idx1d / 1;
  }

  Point() {
    this->x = -1;
    this->y = -1;
    this->z = -1;
  }

  long make_linear_idx(long *dims) {
    return (long) (this->z * dims[0] * dims[1] + this->y * dims[0] + this->x);
  }

  float dist(const Point<T> p) {
    Point d = *this - p;
    return pow(d.x * d.x + d.y * d.y + d.z * d.z, 0.5);
  }

  Point<long> tolong() {
    return Point<long>((long)this->x, (long)this->y, (long)this->z);
  }

  Point<int> toint() {
    return Point<int>((int) this->x, (int) this->y, (int) this->z);
  }

  Point<double> todouble() {
    return Point<double>((double)this->x, (double)this->y, (double)this->z);
  }

  vector<Point<T> > neighbours_3d(int radius) {
    vector<Point> neighbours;
    // Return the coordinates of neighbours within a radius
    for (float xgv = this->x - radius; xgv <= this->x + radius; xgv++)
      for (float ygv = this->y - radius; ygv <= this->y + radius; ygv++)
        for (float zgv = this->z - radius; zgv <= this->z + radius; zgv++) {
          Point<T> p(xgv, ygv, zgv);
          neighbours.push_back(p);
        }
    return neighbours;
  }

  Point<T> operator-(const Point &other) {
    Point pt;
    pt.x = this->x - other.x;
    pt.y = this->y - other.y;
    pt.z = this->z - other.z;
    return pt;
  }

  T *make_array() {
    T *a = new T[3];
    a[0] = this->x;
    a[1] = this->y;
    a[2] = this->z;
    return a;
  }
};

*/

 
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

//        std::cout<<"N="<<N<<endl;
//        std::cout<<"M="<<M<<endl;
//        std::cout<<"P="<<P<<endl;
//        std::cout<<"sc="<<sc<<endl;

//        V3DLONG in[3],out[3];
//        Point(N,in);


//        long *in_sz3 = new V3DLONG[3];
//        in_sz3[0] = in_sz[0];
//        in_sz3[1] = in_sz[1];
//        in_sz3[2] = in_sz[2];
//    //    in_sz3[3] = in_sz[3];
//        Image3<unsigned char> *img =
//            new Image3<unsigned char>(data1d, in_sz3); // Make the Image
//        img->set_destroy(false);
    }
    //main neuron machine learning code

    //// THIS IS WHERE THE DEVELOPERS SHOULD ADD THEIR OWN NEURON MACHINE LEARNING CODE    






 //   NeuronTree nt_in,nt_out;
   // nt_out = find_most_point(PARA,nt_in);

    double DIST_1,DIST;
    printf("****************************this is do KNN************************************");
    DIST_1 = KNN(PARA,DIST);
   // writeSWC_file(swc_name.toStdString().c_str(),PARA.);
  //  cout<<"DIST="<<DIST<<endl;



    //Output
    NeuronTree nt_output,result,nt_in;
  //  QList<ImageMarker> marker_output;
    printf("****************************this is do match************************************");
    //nt_output = match(PARA,DIST_1,result);
     nt_output = find_most_point(PARA,nt_in);

	QString swc_name = PARA.inimg_file + "_test.swc";
	nt_output.name = "test";
    //QString marker_name = PARA.inimg_file + "_test.marker";
    writeSWC_file(swc_name.toStdString().c_str(),nt_output);
  //  writeMarker_file(marker_name,marker_output);


    if(!bmenu)
    {
        if(data1d) {delete []data1d; data1d = 0;}
    }

    if(nt_output.listNeuron.size()>0) v3d_msg(QString("Now you can drag and drop the generated swc fle [%1] into Vaa3D.").arg(swc_name.toStdString().c_str()),bmenu);
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



        map<int,int> mymap;
        int i;
        mymap.insert(pair<int,int>(i,PARA.nt.listNeuron.pn));




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
