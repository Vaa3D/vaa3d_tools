/* Mean_Variance_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-5-28 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "Mean_Variance_plugin.h"
#include <v3d_interface.h>
#include <fstream>
#include <math.h>
#include <sstream>

using namespace std;

#define VOID 1000000000
#define PI 3.14159265359
#define dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define anglez(a,b) (acos(((b).z-(a).z)/dist(a,b))*180.0/PI)
#define anglex(a,b) (acos(((b).x-(a).x)/sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)))*180.0/PI)
#define angley(a,b) (acos(((b).y-(a).y)/sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)))*180.0/PI)


Q_EXPORT_PLUGIN2(Mean_Variance, TestPlugin);

void getSwcMeanStd(unsigned char* pdata, V3DLONG* sz, const NeuronTree& nt, double& swcMean, double& swcStd, double &median);

void getSwcMeanStd(QString imagePath, QString swcPath, double& swcMean, double& swcStd, double &median, V3DPluginCallback2 &callback);

QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("Mean_Variance")
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
    if (menu_name == tr("Mean_Variance"))
	{
        ofstream out;
        out.open("E://sucai//Mean_Variance.txt",ios::app);

        QString dir=QFileDialog::getExistingDirectory(parent);
        QDir dir_0(dir);
        QStringList filter;
        filter<<"*.tif";
        QStringList tiffiles=dir_0.entryList(filter);

        for(int i=0;i<tiffiles.size();++i)
        {
            QString tiffile_0=dir+"/"+tiffiles[i];
            QFileInfo tiffile(tiffile_0);

            unsigned char* p=0;
            V3DLONG sz[4]={0,0,0,0};
            int datatype=1;
            simple_loadimage_wrapper(callback,tiffile.absoluteFilePath().toStdString().c_str(),p,sz,datatype);

            V3DLONG num=sz[0]*sz[1]*sz[2];
            vector<double> hdz(num,0);
            for(V3DLONG j=0;j<num;++j)
            {
                hdz[j]=(double)p[j];
            }
            double mean=0,variance=0,all=0,all_0=0;
            for(V3DLONG j=0;j<num;++j)
            {
                all+=hdz[j];
            }
            mean=all/num;
            for(V3DLONG j=0;j<num;++j)
            {
                all_0+=(hdz[j]-mean)*(hdz[j]-mean);
            }
            variance=all_0/num;


            out<<tiffile.baseName().toStdString()<<" "
              <<"mean:"<<mean<<" "<<"variance:"<<variance<<endl;
            delete p;
        }

	}
	else if (menu_name == tr("menu2"))
	{
        QString tif=QFileDialog::getOpenFileName(parent);
        QString tif_l=QFileDialog::getOpenFileName(parent);
        unsigned char* p0=0;
        V3DLONG sz0[4]={0,0,0,0};
        int datatype0=0;
        unsigned char* p1=0;
        V3DLONG sz1[4]={0,0,0,0};
        int datatype1=0;
        simple_loadimage_wrapper(callback,tif.toStdString().c_str(),p0,sz0,datatype0);
        simple_loadimage_wrapper(callback,tif_l.toStdString().c_str(),p1,sz1,datatype1);
        V3DLONG num=sz0[0]*sz0[1]*sz0[2];
        unsigned char* p=new unsigned char[num];
        for(V3DLONG i=0;i<num;++i)
        {
            p[i]=((p0[i]+p1[i])>0)?(p0[i]-p1[i]):0;
        }
        int datatype=1;
        V3DLONG sz[4]={sz0[0],sz0[1],sz0[2],sz0[3]};
        string path="E://sucai//l.tif";
        simple_saveimage_wrapper(callback,path.c_str(),p,sz,datatype);
        delete p0;
        delete p1;
        delete p;
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-5-28"));
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
        if(infiles.size()>=1)
        {
            QString eswcfile(infiles[0]);
            NeuronTree nt=readSWC_file(eswcfile);

            ofstream angle;
            angle.open("E://sucai//angle.txt");

            vector<vector<V3DLONG>> children;
            const V3DLONG num=nt.listNeuron.size();
            children=vector<vector<V3DLONG>>(num,vector<V3DLONG>());
            for(V3DLONG i=0;i<num;++i)
            {
                V3DLONG par=nt.listNeuron[i].parent;
                if(par<0) continue;
                children[nt.hashNeuron.value(par)].push_back(i);
            }
/*
            bool bifurcation[num];
            double distance[num];
            double angle_x[num];
            double angle_z[num];*/

            vector<bool> bifurcation;
            vector<double> distance;
            vector<double> angle_x;
            vector<double> angle_z;
            bifurcation=vector<bool>(num,false);
            distance=vector<double>(num,0.0);
            angle_x=vector<double>(num,0.0);
            angle_z=vector<double>(num,0.0);



            for(V3DLONG i=0;i<num;++i)
            {
                if(children[i].size()>=2){
                    bifurcation[i]=true;
                }
                else{
                    bifurcation[i]=false;
                }
                V3DLONG par=nt.listNeuron[i].parent;
                if(par<0)
                {
                    distance[i]=0.0;
                    angle_x[i]=0.0;
                    angle_z[i]=0.0;
                }else
                {
                    double distance_0=dist(nt.listNeuron[nt.hashNeuron.value(par)],nt.listNeuron[i]);
                    double angle_z_0=anglez(nt.listNeuron[nt.hashNeuron.value(par)],nt.listNeuron[i]);
                    double angle_x_0=anglex(nt.listNeuron[nt.hashNeuron.value(par)],nt.listNeuron[i]);
                    distance[i]=distance_0;
                    angle_x[i]=angle_x_0;
                    angle_z[i]=angle_z_0;
                }

            }

            angle<<"n"<<" "<<"parent"<<" "<<"distance"<<" "<<"angle_x"<<" "<<"angle_z"<<" "
                <<"angle_x_dist"<<" "<<"angle_z_dist"<<" "<<"isbifurcation"<<endl;

            for(V3DLONG i=0;i<num;++i)
            {
                double dist_dx,dist_dz;
                V3DLONG par=nt.listNeuron[i].parent;
                if(par<0)
                {
                    dist_dx=0.0;
                    dist_dz=0.0;
                    angle<<nt.listNeuron[i].n<<" "<<nt.listNeuron[i].parent<<" "
                        <<distance[i]<<" "<<angle_x[i]<<" "<<angle_z[i]
                       <<" "<<dist_dx<<" "<<dist_dz<<" "<<bifurcation[i]<<endl;
                }
                else if(nt.listNeuron[nt.hashNeuron.value(par)].parent<0)
                {
                    dist_dx=0.0;
                    dist_dz=0.0;
                    angle<<nt.listNeuron[i].n<<" "<<nt.listNeuron[i].parent<<" "
                        <<distance[i]<<" "<<angle_x[i]<<" "<<angle_z[i]
                       <<" "<<dist_dx<<" "<<dist_dz<<" "<<bifurcation[i]<<endl;
                }
                else
                {
                    dist_dx=angle_x[i]-angle_x[nt.hashNeuron.value(par)];
                    dist_dz=angle_z[i]-angle_z[nt.hashNeuron.value(par)];
                    angle<<nt.listNeuron[i].n<<" "<<nt.listNeuron[i].parent<<" "
                        <<distance[i]<<" "<<angle_x[i]<<" "<<angle_z[i]
                       <<" "<<dist_dx<<" "<<dist_dz<<" "<<bifurcation[i]<<endl;
                }
            }
        }
	}
	else if (func_name == tr("getSwcMeanStd"))
	{
        QString csvPath = inparas[0];
        QString csvOutPath = inparas[1];
        ifstream csvFileIn;
        csvFileIn.open(csvPath.toStdString().c_str(),ios::in);
        vector<string> arrayList = vector<string>();
        if(csvFileIn){
            string str;
            while(getline(csvFileIn,str)){
                arrayList.push_back(str);
            }
        }else {
            qDebug()<<"open csvFile failed";
            return 0;
        }
        csvFileIn.close();
        int rowSize = arrayList.size();
        vector<QStringList> data = vector<QStringList>();

        for(int i=0; i<rowSize; i++){
            QString s = QString::fromStdString(arrayList[i]);
            QStringList ss = s.split(',');
            data.push_back(ss);
        }
        data[0].append("swcIMean");
        data[0].append("swcIStd");
        data[0].append("swcIMedian");

        int columnSize = data[0].size();

        for(int i=1; i<rowSize; i++){
            QString imagePath = data[i][columnSize-5];
            QString swcPath = data[i][columnSize-4];
            qDebug()<<imagePath;
            qDebug()<<swcPath;
            double swcMean,swcStd,swcMedian;
            getSwcMeanStd(imagePath,swcPath,swcMean,swcStd,swcMedian,callback);
            data[i].append(QString::number(swcMean));
            data[i].append(QString::number(swcStd));
            data[i].append(QString::number(swcMedian));
        }

        ofstream csvFileOut;
        csvFileOut.open(csvOutPath.toStdString().c_str(),ios::out);
        for(int i=0; i<rowSize; i++){
            for(int j=0; j<columnSize; j++){
                csvFileOut<<data[i].at(j).toStdString().c_str();
                if(j != columnSize-1){
                    csvFileOut<<',';
                }else {
                    csvFileOut<<endl;
                }
            }
        }
        csvFileOut.close();


	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void getSwcMeanStd(unsigned char* pdata, V3DLONG* sz, const NeuronTree& nt, double& swcMean, double& swcStd, double& median){
    int x,y,z;
    int size =nt.listNeuron.size();
    swcMean = swcStd = 0;
    vector<unsigned char> intensity = vector<unsigned char>();
    for(int i=0; i<size; i++){
        x = (int)(nt.listNeuron[i].x + 0.5);
        y = (int)(nt.listNeuron[i].y + 0.5);
        z = (int)(nt.listNeuron[i].z + 0.5);
        if(x<0) x=0; if(x>=sz[0]) x=sz[0]-1;
        if(y<0) y=0; if(y>=sz[1]) y=sz[1]-1;
        if(z<0) z=0; if(z>=sz[2]) z=sz[2]-1;
        V3DLONG index = z*sz[0]*sz[1] + y*sz[0] + x;
        intensity.push_back(pdata[index]);
    }
    sort(intensity.begin(),intensity.end());

    median = intensity[size/2];

    for(int i=0; i<size; i++){
        swcMean += intensity[i];
    }
    if(size>0){
        swcMean /= (double)size;
    }
    for(int i=0; i<size; i++){
        swcStd += (intensity[i]-swcMean)*(intensity[i]-swcMean);
    }
    if(size>0){
        swcStd = sqrt(swcStd/size);
    }

}

void getSwcMeanStd(QString imagePath, QString swcPath, double &swcMean, double &swcStd, double &median, V3DPluginCallback2& callback){
    unsigned char* pdata = 0;
    V3DLONG sz[4] = {0,0,0,0};
    int dataType = 1;
    simple_loadimage_wrapper(callback,imagePath.toStdString().c_str(),pdata,sz,dataType);
    NeuronTree nt = readSWC_file(swcPath);
    getSwcMeanStd(pdata,sz,nt,swcMean,swcStd,median);
    if(pdata){
        delete[] pdata;
        pdata = 0;
    }
}

