#include "S2_tracing_connector_func.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "basic_surf_objs.h"
#include <iostream>

using namespace std;

QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    if (method_code ==1)
        imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";
    else if (method_code ==2)
        imgSuffix<<"*.marker";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}

bool generatorcombined4FilesInDir(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"Welcome to combine file generator"<<endl;
    if (input.size() < 1) return false;

	int method_code = 1;
    if (input.size()>=2)
    {
        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        if(paras.size() >= 1) method_code = atoi(paras.at(0));
    }

    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    cout<<"inimg_file = "<<inimg_file<<endl;

    vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
    vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
    QString combined_file;


    if(method_code == 1)
    {
        if(!outfiles.empty())
        {
            combined_file = outfiles[0];
            cout<<"combined_file = "<<combined_file.toStdString().c_str()<<endl;
        }
        else
            combined_file = QString(inimg_file) + "combined.swc";


        QStringList swcList = importFileList_addnumbersort(QString(inimg_file), method_code);

        vector<MyMarker*> outswc;
        for(V3DLONG i = 0; i < swcList.size(); i++)
        {

            QString curPathSWC = swcList.at(i);

            vector<MyMarker*> inputswc = readSWC_file(curPathSWC.toStdString());;

            for(V3DLONG d = 0; d < inputswc.size(); d++)
            {
                outswc.push_back(inputswc[d]);
            }

        }
        saveSWC_file(combined_file.toStdString().c_str(), outswc);

    }
    else if (method_code == 2)
    {
        if(!outfiles.empty())
        {
            combined_file = outfiles[0];
            cout<<"combined_file = "<<combined_file.toStdString().c_str()<<endl;
        }
        else
            combined_file = QString(inimg_file) + "combined.marker";

        QStringList markerList = importFileList_addnumbersort(QString(inimg_file), method_code);

        QList <ImageMarker> outmarker;
        for(V3DLONG i = 0; i < markerList.size(); i++)
        {

            QString curPathMarker = markerList.at(i);
            QList <ImageMarker> inputmarker = readMarker_file(curPathMarker);;

            for(V3DLONG d = 0; d < inputmarker.size(); d++)
            {
                outmarker.push_back(inputmarker.at(d));
            }

        }
        writeMarker_file(combined_file,outmarker);
    }
    return true;
}

