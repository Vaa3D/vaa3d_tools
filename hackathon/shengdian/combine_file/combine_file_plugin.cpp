/* combine_file_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-2-27 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "combine_file_plugin.h"
#include "basic_surf_objs.h"

#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../sort_neuron_swc/openSWCDialog.h"

using namespace std;
Q_EXPORT_PLUGIN2(combine_file, combine_file);

void generatorcombined4FilesInDir(V3DPluginCallback2 &callback, QWidget *parent, int method_code);
bool generatorcombined4FilesInDir(const V3DPluginArgList & input, V3DPluginArgList & output);

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

QStringList combine_file::menulist() const
{
	return QStringList() 
        << tr("combine all SWC or ESWC files under a directory")
        << tr("combine all marker files under a directory")
        << tr("separate a SWC file by type");
    //    << tr("separate all neuron trees from a single SWC file");
    //	<<tr("about");
}

QStringList combine_file::funclist() const
{
	return QStringList()
        << tr("combine")
		<<tr("help");
}

void combine_file::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("combine all SWC or ESWC files under a directory"))
	{
        generatorcombined4FilesInDir(callback, parent, 1);
	}
    else if (menu_name == tr("combine all marker files under a directory"))
	{
        generatorcombined4FilesInDir(callback, parent, 2);
    }
    else if (menu_name == tr("separate a SWC file by type"))
    {
        QString fileOpenName;
        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;
        fileOpenName = openDlg->file_name;
        NeuronTree nt;
        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
        {
            nt = openDlg->nt;
        }

        map<int, QList<NeuronSWC> > multi_neurons;
        for(V3DLONG i = 0; i<nt.listNeuron.size(); i++)
        {
            multi_neurons[nt.listNeuron[i].type].append(nt.listNeuron[i]);
        }
        for (map<int, QList<NeuronSWC> >::iterator it=multi_neurons.begin(); it!=multi_neurons.end(); ++it)
        {
            NeuronTree nt_trees;
            nt_trees.listNeuron = it->second;
            QString fileSaveName = fileOpenName + QString("_type_%1.swc").arg(it->first);
            writeSWC_file(fileSaveName,nt_trees);
            nt_trees.listNeuron.clear();
        }
        v3d_msg("Done with SWC separation!");

    }

//    else if (menu_name == tr("separate all neuron trees from a single SWC file"))
//    {
//        QString fileOpenName;
//        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
//                "",
//                QObject::tr("Supported file (*.swc *.eswc)"
//                    ";;Neuron structure	(*.swc)"
//                    ";;Extended neuron structure (*.eswc)"
//                    ));
//        if(fileOpenName.isEmpty())
//            return;

//        QString m_folderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory to save all swc files "),
//                                                                      QDir::currentPath(),
//                                                                      QFileDialog::ShowDirsOnly);
//        NeuronTree nt;
//        if (fileOpenName.toUpper().endsWith(".SWC") || fileOpenName.toUpper().endsWith(".ESWC"))
//        {
//             nt = readSWC_file(fileOpenName);
//             int start_node = 0;
//             int end_node = 0;

//             for (int i=1;i<nt.listNeuron.size();i++)
//             {
//                 if(nt.listNeuron[i].pn <0 || i == nt.listNeuron.size()-1)
//                 {
//                    end_node = i-1;
//                    if(i == nt.listNeuron.size()-1) end_node = i;
//                    NeuronSWC each_branch = nt.listNeuron.at(start_node);
//                    QString branch_swc_name =  m_folderName+QString("/x_%1_y_%2_z_%3.swc").arg(each_branch.x).arg(each_branch.y).arg(each_branch.z);
//                    NeuronTree nt_branches;
//                    QList <NeuronSWC> & listNeuron = nt_branches.listNeuron;
//                    for(int dd = start_node; dd <= end_node; dd++)
//                    {
//                        listNeuron << nt.listNeuron.at(dd);
//                    }
//                    writeSWC_file(branch_swc_name,nt_branches);
//                    listNeuron.clear();
//                    start_node = i;
//                 }
//             }
//        }
//    }
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2015-2-27"));
	}
}

bool combine_file::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("combine"))
	{
       return generatorcombined4FilesInDir(input, output);
	}
	else if (func_name == tr("help"))
	{
           cout<<"Usage : v3d -x combine_file -f combine -i <in_folder> -o <combined_file> -p <method_code> "<<endl;
           cout<<endl;
           cout<<"in_folder         choose a directory that contain specified files"<<endl;
           cout<<"combined_file     specify the combined file name to be created" <<endl;
           cout<<"method_code       combine options, default 1"<<endl;
           cout<<"                  1 for for all SWC files under a directory"<< endl;
           cout<<"                  2 for for all marker files under a directory"<< endl;
           cout<<endl;
           cout<<endl;
	}
	else return false;

	return true;
}

vector<MyMarker*> mergeSWCafterCombine(vector<MyMarker*> inputSwc,V3DLONG marker_segid=0)
{
    map<MyMarker*,MyMarker*> seg;//key=tail node,value=head node.
    map<MyMarker*,V3DLONG> seg_length;
    map<MyMarker*,V3DLONG> marker_flag;//0:untouched
    vector<MyMarker*> outswc;//,inputSwc_backup;
    for(V3DLONG i=0;i<inputSwc.size();i++)
    {
        marker_flag[inputSwc.at(i)]=0;
    }
    for(vector<MyMarker*>::iterator it=inputSwc.begin();it!=inputSwc.end();it++)
    {
        V3DLONG _seg_length=0;
        MyMarker* inputMarker= *it;
        if(marker_flag[inputMarker]==marker_segid)
        {
            cout<<"find one"<<endl;
            continue;
        }
        else
        {
            _seg_length=1;
            marker_flag[inputMarker]=marker_segid;
            while(inputMarker->parent!=0)
            {
                inputMarker=inputMarker->parent;
                marker_flag[inputMarker]=marker_segid;
                _seg_length++;
            }
            seg[*it]=inputMarker;
            if(seg_length.find(seg[*it])!=seg_length.end())
            {
                if(seg_length[seg[*it]]<=_seg_length)
                    seg_length[seg[*it]]=_seg_length;
            }
            else
                seg_length[seg[*it]]=_seg_length;
        }

    }

//    while(it!=inputSwc.end())
//    {
//        V3DLONG _seg_length=0;
//        cout<<"now input marker flag seg id "<<marker_flag[*it]<<endl;
//        MyMarker* inputMarker= *it;
//        cout<<"now input marker flag seg id "<<marker_flag[inputMarker]<<endl;
//        if(marker_flag[inputMarker]==marker_segid)
//        {
//            cout<<"find one"<<endl;
//            it++;
//            continue;
//        }
//        _seg_length=1;/*marker_segid++;*/
//        marker_flag[inputMarker]=marker_segid;
//        cout<<"now input marker flag seg id a"<<marker_flag[inputMarker]<<endl;
//        cout<<"now segid is a "<<marker_segid<<endl;
//        while(inputMarker->parent!=0)
//        {
//            cout<<"move to this "<<endl;
//            inputMarker=inputMarker->parent;
//            marker_flag[inputMarker]=marker_segid;
//            _seg_length++;
//        }
//        cout<<"now segid is "<<marker_segid<<endl;

//        seg[*it]=inputMarker;
//        if(seg_length.find(seg[*it])!=seg_length.end())
//        {
//            if(seg_length[seg[*it]]<=_seg_length)
//                seg_length[seg[*it]]=_seg_length;
//        }
//        else
//            seg_length[seg[*it]]=_seg_length;
//        it++;
//    }
    cout<<"seg size before"<<seg.size()<<endl;

//    for(map<MyMarker*,MyMarker*>::iterator seg_itx=seg.begin();seg_itx!=seg.end();seg_itx++)
//    {
//        for(map<MyMarker*,MyMarker*>::iterator seg_ity=seg.begin();seg_ity!=seg.end();seg_ity++)
//        {
//            if(seg_itx==seg_ity)continue;
//            if(seg_ity->second==seg_itx->second)
//            {
//                if(seg_length[seg_itx->first]>=seg_length[seg_ity->first])
//                {
//                    seg.erase(seg_ity);
//                    continue;
//                }
//            }
//        }
//    }
//    cout<<"seg size after"<<seg.size()<<endl;
//    for(map<MyMarker*,MyMarker*>::iterator seg_itx=seg.begin();seg_itx!=seg.end();seg_itx++)
//    {

//    }
//    for(V3DLONG k=0;k<outswc.size();k++)
//    {
//        MyMarker* temp3=outswc.at(k);
//        if(temp3->parent==0)
//            cout<<"find one";
//    }
    return outswc;
}

void generatorcombined4FilesInDir(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
    if(method_code == 1)
    {
        QString m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all swc and eswc files "),
                                                                      QDir::currentPath(),
                                                                      QFileDialog::ShowDirsOnly);

        QStringList swcList = importFileList_addnumbersort(m_InputfolderName, method_code);

        vector<MyMarker*> outswc;
        for(V3DLONG i = 0; i < swcList.size(); i++)
        {

            QString curPathSWC = swcList.at(i);
            vector<MyMarker*> inputswc;
            inputswc = readSWC_file(curPathSWC.toStdString());
            inputswc=mergeSWCafterCombine(inputswc,i+2);
            for(V3DLONG d = 0; d < inputswc.size(); d++)
            {
                outswc.push_back(inputswc[d]);
            }

        }
        QString swc_combined = m_InputfolderName + "/combined.swc";
        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                swc_combined,
                QObject::tr("Supported file (*.swc)"
                    ";;Neuron structure	(*.swc)"
                    ));
        saveSWC_file(fileSaveName.toStdString().c_str(), outswc);

    }
    else if (method_code == 2)
    {
        QString m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all marker files "),
                                                                      QDir::currentPath(),
                                                                      QFileDialog::ShowDirsOnly);

        QStringList markerList = importFileList_addnumbersort(m_InputfolderName, method_code);

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
        QString marker_combined = m_InputfolderName + "/combined.marker";
        QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
                marker_combined,
                QObject::tr("Supported file (*.marker)"
                    ";;Neuron structure	(*.marker)"
                    ));
        writeMarker_file(fileSaveName,outmarker);
    }
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
