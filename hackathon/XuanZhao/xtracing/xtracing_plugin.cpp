/* xtracing_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-31 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include <iostream>
#include "xtracing_plugin.h"
#include "t_class.h"
#include "t_function.h"

#include <basic_surf_objs.h>

#define PI 3.14159265359
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define anglez(a,b) (acos(((b).z-(a).z)/dist(a,b))*180.0/PI)
#define anglex(a,b) (acos(((b).x-(a).x)/sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)))*180.0/PI)

using namespace std;
Q_EXPORT_PLUGIN2(xtracing, TestPlugin);


template <class T>
void BinaryProcess(T *apsInput, T * aspOutput, V3DLONG iImageWidth, V3DLONG iImageHeight, V3DLONG iImageLayer, V3DLONG h, V3DLONG d)
{
    V3DLONG i, j,k,n,count;
    double t, temp;

    V3DLONG mCount = iImageHeight * iImageWidth;
    for (i=0; i<iImageLayer; i++)
    {
        for (j=0; j<iImageHeight; j++)
        {
            for (k=0; k<iImageWidth; k++)
            {
                V3DLONG curpos = i * mCount + j*iImageWidth + k;
                V3DLONG curpos1 = i* mCount + j*iImageWidth;
                V3DLONG curpos2 = j* iImageWidth + k;
                temp = 0;
                count = 0;
                for(n =1 ; n <= d  ;n++)
                {
                    if (k>h*n) {temp += apsInput[curpos1 + k-(h*n)]; count++;}
                    if (k+(h*n)< iImageWidth) { temp += apsInput[curpos1 + k+(h*n)]; count++;}
                    if (j>h*n) {temp += apsInput[i* mCount + (j-(h*n))*iImageWidth + k]; count++;}//
                    if (j+(h*n)<iImageHeight) {temp += apsInput[i* mCount + (j+(h*n))*iImageWidth + k]; count++;}//
                    if (i>(h*n)) {temp += apsInput[(i-(h*n))* mCount + curpos2]; count++;}//
                    if (i+(h*n)< iImageLayer) {temp += apsInput[(i+(h*n))* mCount + j* iImageWidth + k ]; count++;}
                }
                t =  apsInput[curpos]-temp/(count);
                aspOutput[curpos]= (t > 0)? apsInput[curpos] : 0;
            }
        }
    }

    double tol=0.0,mean=0.0,num=0;
    for(i=0;i<mCount*iImageLayer;++i)
    {
        if(aspOutput[i]>0)
        {
            tol+=aspOutput[i];
            num++;
        }
    }
    mean=tol/num;
    cout<<mean<<endl;
    for(i=0;i<mCount*iImageLayer;++i)
    {
        if(aspOutput[i]==0)
        {
            continue;
        }
        double tmp=(double)aspOutput[i];
        if(tmp<mean*1.3)
        {
            aspOutput[i]=0;
            //cout<<"yeye"<<endl;
        }
    }
}

 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
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
			"Developed by YourName, 2019-5-31"));
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
        unsigned char * pdata=0;
        V3DLONG sz[4]={0,0,0,0};
        int datatype=0;
        simple_loadimage_wrapper(callback,infiles[0],pdata,sz,datatype);
        V3DLONG tol_sz=sz[0]*sz[1]*sz[2];
        V3DLONG sz01=sz[0]*sz[1];

        unsigned char * tmpdata=new unsigned char[tol_sz];
        V3DLONG h=3,d=5;
        BinaryProcess(pdata,tmpdata,sz[0],sz[1],sz[2],h,d);
        vector<T_Point> points=vector<T_Point>(tol_sz,T_Point());
        V3DLONG count=0;
        for(V3DLONG i=0;i<tol_sz;++i)
        {
            if(tmpdata[i]==0)
            {
                points[i].state=T_Point::FAR;
            }else {
                points[i].state=T_Point::ALIVE;
                count++;
            }
        }

        cout<<"count: "<<count<<endl<<"tol: "<<tol_sz<<endl;

        delete pdata;
        delete tmpdata;


        for(V3DLONG i=0;i<tol_sz;++i)
        {
            points[i].is_turn=false;
            points[i].is_used=false;
            points[i].bifurcation=false;
            points[i].n=i;
            points[i].mode=-1;
            points[i].x=(i%sz01)%sz[0];
            points[i].y=(i%sz01)/sz[0];
            points[i].z=i/sz01;
            //cout<<points[i].x<<"  "<<points[i].y<<"  "<<points[i].z<<endl;
        }

        QList<ImageMarker> markers=readMarker_file(inparas[0]);
        cout<<"??"<<endl;
        double angle0_min=(inparas.size()>=2)?atoi(inparas[1]):30;
        double angle1_min=(inparas.size()>=3)?atoi(inparas[2]):50;
        double angle0_max=(inparas.size()>=4)?atoi(inparas[3]):60;
        double angle1_max=(inparas.size()>=5)?atoi(inparas[4]):80;
        V3DLONG d_min=(inparas.size()>=6)?atoi(inparas[5]):4;
        V3DLONG d_max=(inparas.size()>=7)?atoi(inparas[6]):15;

        T_Point root,p;
        root.x=floor(markers[0].x);
        root.y=floor(markers[0].y);
        root.z=floor(markers[0].z);
        p.x=floor(markers[1].x);
        p.y=floor(markers[1].y);
        p.z=floor(markers[1].z);

        root.z_angle=anglez(root,p);
        root.x_angle=anglex(root,p);

        root.n=root.z*sz01+root.y*sz[0]+root.x;

        cout<<"000"<<endl;



        vector<T_Point> outtree;
        construct_tree(root,points,outtree,angle0_min,angle1_min,angle0_max,angle1_max,d_min,d_max,sz[0],sz[1],sz[2]);

        cout<<"999"<<endl;

        NeuronTree nt;
        for(int i=0;i<outtree.size();++i)
        {
            NeuronSWC tmp;
            tmp.x=outtree[i].x+0.5;
            tmp.y=outtree[i].y+0.5;
            tmp.z=outtree[i].z+0.5;
            tmp.n=outtree[i].n;
            tmp.parent=outtree[i].parent;
            tmp.type=2;
            tmp.radius=1;
            nt.listNeuron.push_back(tmp);
        }

        writeESWC_file("E:\\sucai\\test.eswc",nt);







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

