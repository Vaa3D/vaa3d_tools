/* choose_marker_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-7-19 : by LXF
 */
 
#include "v3d_message.h"
#include <vector>
#include <iostream>
#include "choose_marker_plugin.h"
#include "../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h"
using namespace std;
Q_EXPORT_PLUGIN2(choose_marker, choose_markerPlugin);
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))


uint qHash(const Coordinate key)
{
    return key.x + key.y + key.z;
}

QStringList choose_markerPlugin::menulist() const
{
	return QStringList() 
        <<tr("choose_marker")
        <<tr("get_original_marker")
        <<tr("get_color")
		<<tr("about");
}

QStringList choose_markerPlugin::funclist() const
{
	return QStringList()
        <<tr("choose_marker")
        <<tr("get_original_marker")
        <<tr("get_color")
		<<tr("help");
}

void choose_markerPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("choose_marker"))
	{
        v3dhandle win = callback.currentImageWindow();
        LandmarkList marker = callback.getLandmark(win);
        QList<ImageMarker> marker_choose,marker_exp;
        for(int i=0;i<marker.size();i++)
        {

            if(marker[i].comments != "")
            {
                ImageMarker m;
                m.x = marker[i].x;
                m.y = marker[i].y;
                m.z = marker[i].z;
                m.color.a = 0;
                m.color.b = 0;
                m.color.g = 0;
                m.color.r = 0;
                marker_choose.push_back(m);
            }
            else
            {
                ImageMarker m;
                m.x = marker[i].x;
                m.y = marker[i].y;
                m.z = marker[i].z;
                m.color.a = 0;
                m.color.b = 0;
                m.color.g = 0;
                m.color.r = 0;
                marker_exp.push_back(m);
            }
        }
        writeMarker_file(QString("marker_choose.marker"),marker_choose);
        writeMarker_file(QString("marker_exp.marker"),marker_exp);
	}
    else if (menu_name == tr("get_original_marker"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by LXF, 2018-7-19"));
	}
}

bool choose_markerPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("get_original_marker"))
	{
        if(infiles.size()!=2)
        {
            v3d_msg("Please input two files,first one is offset result,anther one is markerlist for all center");
            return false;
        }
        QString fileOpenName1 = QString(infiles.at(0));
        QString fileOpenName2 = QString(infiles.at(1));
        QList <ImageMarker> markerlist1 = readMarker_file(fileOpenName1);
        QList <ImageMarker> markerlist2 = readMarker_file(fileOpenName2);
        QList <ImageMarker> marker_result;
        for(V3DLONG i=0;i<markerlist1.size();i++)
        {
            double min_dis = 1000000000000;
            int ind;
            for(V3DLONG j=0;j<markerlist2.size();j++)
            {
                double dis = NTDIS(markerlist1[i],markerlist2[j]);
                if(dis<min_dis)
                {
                    min_dis = dis;
                    ind = j;
                }
            }
            marker_result.push_back(markerlist2[ind]);
        }
        writeMarker_file(QString("marker_result.marker"),marker_result);
        //v3d_msg("To be implemented.");
	}
    else if (func_name == tr("choose_marker"))
	{
		v3d_msg("To be implemented.");
	}
    else if (func_name == tr("get_color"))
    {
        if(infiles.size()!=2)
        {
            //v3d_msg("Please input two files,first one is img file,anther one is marker file");
            return false;
        }
        QString imgOpenName = QString(infiles.at(0));
        QString markerOpenName = QString(infiles.at(1));
        get_color(imgOpenName,markerOpenName,callback);



        //v3d_msg("To be implemented.");
    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}
int get_color(QString & img,QString & markerOpenName,V3DPluginCallback2 & callback)
{
    unsigned char* data1d=0;
    V3DLONG in_sz[4];
    int datatype;
    if(!simple_loadimage_wrapper(callback,(char *)img.toStdString().c_str(), data1d, in_sz, datatype))  {cout<<"load img wrong"<<endl;  return false;}
    QList <ImageMarker> markerlist = readMarker_file(markerOpenName);
    vector<vector<vector<V3DLONG> > > coodinate3D;
    vector<vector<V3DLONG> > coodinate2D;
    vector<V3DLONG> coodinate1D;
    int M = in_sz[0];
    int N = in_sz[1];
    int P = in_sz[2];
    for(V3DLONG iz = 0; iz < P; iz++)
    {
        V3DLONG offsetk = iz*M*N;
        for(V3DLONG iy = 0; iy < N; iy++)
        {
            V3DLONG offsetj = iy*N;
            for(V3DLONG ix = 0; ix < M; ix++)
            {
                V3DLONG tmp = data1d[offsetk + offsetj + ix];
                coodinate1D.push_back(tmp);
            }
            coodinate2D.push_back(coodinate1D);
            coodinate1D.clear();
        }
        coodinate3D.push_back(coodinate2D);
        coodinate2D.clear();
    }
    cout<<"markerlist = "<<markerlist.size()<<endl;
   //v3d_msg("check");
    QHash<Coordinate,int> color;
    for(V3DLONG i=0;i<markerlist.size();i++)
    {

        ImageMarker curr_m = markerlist[i];
        int j;
        Coordinate coord;
        coord.x = curr_m.x;
        coord.y = curr_m.y;
        coord.z = curr_m.z;
        cout<<coord.x<<"  "<<coord.y<<"  "<<coord.z<<endl;
        cout<<"i = "<<i<<endl;
        j = coodinate3D[curr_m.z][curr_m.y][curr_m.x];
        //cout<<"=============="<<endl;
        color.insert(coord,j);

    }
    QString fileSaveName = markerOpenName+"_color.txt";
    cout<<"***********************************************************"<<endl;
    export_TXT(color,fileSaveName);




}
bool export_TXT(QHash<Coordinate,int> &color,QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);


    for(QHash<Coordinate,int>::iterator it = color.begin();it!=color.end();it++)
    {
        cout<< it.key().x <<"    "<<it.key().y<<"    "<<it.key().z<<"    "<<it.value()<<endl;

        myfile << it.key().x <<"    "<<it.key().y<<"    "<<it.key().z<<"    "<<it.value()<<endl;

    }


    file.close();
    cout<<"txt file "<<fileSaveName.toStdString()<<" has been generated, size: "<<color.size()<<endl;
    return true;
}

