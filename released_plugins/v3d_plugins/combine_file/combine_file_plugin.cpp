/* combine_file_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2015-2-27 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "combine_file_plugin.h"
#include "basic_surf_objs.h"

#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h""

#include "../sort_neuron_swc/openSWCDialog.h"
#include <algorithm>

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

            NeuronTree nt_sorted;
            SortSWC(nt_trees.listNeuron,nt_sorted.listNeuron,VOID,0);
            writeSWC_file(fileSaveName,nt_sorted);
            nt_trees.listNeuron.clear();
            nt_sorted.listNeuron.clear();

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
        v3d_msg(tr("this is a plugin used to combine two or more swc files together.Firstly, you need put all the swc or eswc files to one folder.. "
                   "Developed by Zhi Zhou (2015-2-27) and redesigned by Shengdian Jiang (2018-07-31)"));
	}
}

bool combine_file::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("combine"))
	{
       return generatorcombined4FilesInDir(input, output);
    }
    else if (func_name == tr("separate"))
    {
        char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
        QString fileOpenName = QString(inimg_file);
        NeuronTree nt = readSWC_file(fileOpenName);

        NeuronTree nt_trees;
        for(int j=110; j<240; j+=30)
        {
            QString fileSaveName = fileOpenName + QString("_level_%1.swc").arg(j);
            for(int i=0; i<nt.listNeuron.size();i++)
            {
                if(nt.listNeuron.at(i).level<=j)
                    nt_trees.listNeuron.push_back(nt.listNeuron.at(i));
            }
            writeSWC_file(fileSaveName,nt_trees);
            nt_trees.listNeuron.clear();
        }
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
double distanceofMarker(MyMarker* marker1,MyMarker* marker2)
{
    double distance=0;/*int count=0;*/
    distance=(marker1->x-marker2->x)*(marker1->x-marker2->x)+(marker1->y-marker2->y)*(marker1->y-marker2->y)+(marker1->z-marker2->z)*(marker1->z-marker2->z);
    return distance;
}

MyMarker* findNearestMarker(vector<MyMarker*> inputSwc,MyMarker* inputMarker)
{
    MyMarker* outMarker,*findMarker;
    double Mindistance=0;

    if(inputMarker->parent!=0)
    {
        for(vector<MyMarker*>::iterator tar=inputSwc.begin();tar!=inputSwc.end();tar++)
        {
            MyMarker* targetMarker=*tar;
            if(tar==inputSwc.begin())
            {
                Mindistance=distanceofMarker(targetMarker,inputMarker);
                findMarker=targetMarker;
            }
            else
            {
                double tempDistace=distanceofMarker(targetMarker,inputMarker);
                if(Mindistance>tempDistace)
                {
                    Mindistance=tempDistace;
                    findMarker=targetMarker;
                }
            }
        }
    }
    else
    {
        qDebug("this marker is already parent marker");
    }
    if(Mindistance>150)
    {
        qDebug("the distance is worng.");
        return outMarker;
    }
    else
        outMarker=inputMarker->parent;
    if(findMarker&&outMarker)
    {
        qDebug("find one");
        outMarker->x=findMarker->x;
        outMarker->y=findMarker->y;
        outMarker->z=findMarker->z;
        outMarker->type=findMarker->type;
        outMarker->radius=findMarker->radius;
        outMarker->parent=0;
    }
    else
        qDebug("can't find parent node.");
    return outMarker;
}

vector<MyMarker*> findNearestMarker(vector<MyMarker*> inputSwc,MyMarker* inputMarker,bool isSecondMerge)
{
    MyMarker*findMarker;
    vector<MyMarker*> outMarkerSwc;
    double Mindistance=0;
    if(!isSecondMerge)
        return outMarkerSwc;
    if(inputMarker->parent!=0)
    {
        for(vector<MyMarker*>::iterator tar=inputSwc.begin();tar!=inputSwc.end();tar++)
        {
            MyMarker* targetMarker=*tar;
            if(tar==inputSwc.begin())
            {
                Mindistance=distanceofMarker(targetMarker,inputMarker);
                findMarker=targetMarker;
            }
            else
            {
                double tempDistace=distanceofMarker(targetMarker,inputMarker);
                if(Mindistance>tempDistace)
                {
                    Mindistance=tempDistace;
                    findMarker=targetMarker;
                }
            }
        }
    }
    else
    {
        qDebug("this marker is already parent marker");
    }
    double parent2targetdistance=distanceofMarker(findMarker,inputMarker->parent);
    while(Mindistance>parent2targetdistance&&inputMarker->parent!=0)
    {
        inputMarker=inputMarker->parent;
        Mindistance=parent2targetdistance;
        parent2targetdistance=distanceofMarker(findMarker,inputMarker);
        outMarkerSwc.push_back(inputMarker);
    }
    if(outMarkerSwc.size()==0)
        return outMarkerSwc;
    if(findMarker)
    {        
        MyMarker* outMarker=outMarkerSwc.back();
        outMarker->x=findMarker->x;
        outMarker->y=findMarker->y;
        outMarker->z=findMarker->z;
        outMarker->type=findMarker->type;
        outMarker->radius=findMarker->radius;
        outMarker->parent=0;
    }
    else
        qDebug("can't find parent node.");
    return outMarkerSwc;
}

double distanceofPoints(XYZ point1,XYZ point2)
{
    double distance=0;
    distance=(point1.x-point2.x)*(point1.x-point2.x)+(point1.y-point2.y)*(point1.y-point2.y)+(point1.z-point2.z)*(point1.z-point2.z);
    return distance;
}
double distanceofMarker2point(MyMarker* inputMarker,XYZ inputPosition)
{
    double distanceMP=0;
    XYZ inputMarkerP;
    inputMarkerP.x=inputMarker->x;
    inputMarkerP.y=inputMarker->y;
    inputMarkerP.z=inputMarker->z;
    distanceMP=distanceofPoints(inputMarkerP,inputPosition);
    return distanceMP;
}
///interpolate node between markers,if one node is too far to their parent node, interpolate node between them.
//some code may need to adjust.
vector<XYZ> InterpolationBetweenMarkers(vector<MyMarker*> inputSwc)
{
    vector<XYZ> outPostion;

    if(inputSwc.size()>0)
    {
        for(vector<MyMarker*>::iterator it=inputSwc.begin();it!=inputSwc.end();it++)
        {
            MyMarker* inputMarker=*it;
            XYZ inputPosition,parentPosition;
            if(!inputMarker->parent)
                continue;
            inputPosition.x=(float)inputMarker->x;inputPosition.y=(float)inputMarker->y;inputPosition.z=(float)inputMarker->z;
            parentPosition.x=(float)inputMarker->parent->x;parentPosition.y=(float)inputMarker->parent->y;parentPosition.z=(float)inputMarker->parent->z;
            double distanceInit=distanceofPoints(inputPosition,parentPosition);
            int interpolationNumber=1;
            if(distanceInit<35)
            {
                continue;
            }
            else
            {
                interpolationNumber=(int)(distanceInit/35)+1;
                //cout<<"interpolation number is "<<interpolationNumber<<endl;
            }
            //interpolate points
            for(int i=1;i<interpolationNumber;i++)
            {
                XYZ point;
                point.x=i*(-inputPosition.x+parentPosition.x)/interpolationNumber+inputPosition.x;
                point.y=i*(-inputPosition.y+parentPosition.y)/interpolationNumber+inputPosition.y;
                point.z=i*(-inputPosition.z+parentPosition.z)/interpolationNumber+inputPosition.z;
                outPostion.push_back(point);
            }
        }
    }
    return outPostion;
}
#define MAXSIZE 500000
////merge version2: 1.merge the small size Swc file to large size SWC file.
////                2.interpolate node to these markers whose position are very big to their parent node.
////                3.if input node is too small to the target node, delete it. add the rest (input node) to the output swc file.
////                4.if input node is too small to the interpolate nodes, delete it. add the rest (input node) to the output swc file.
////                5.for all outMarker,find the nearest node in targe nodes and adjust postion of the parent node of those outmarker to it.
vector<MyMarker*> mergeSWCorESWC(vector<MyMarker*> inputSwc1,//inputSwc nodes merge to targetSwc nodes
                                       vector<MyMarker*> inputSwc2)
{
    vector<MyMarker*> targetSwc,inputSwc,outSwc,MergeSwc;
    //merge the small size SWC file to large size SWC file.
    {
    if(inputSwc1.size()==0&&inputSwc2.size()==0)
    {
        qDebug("Input SWC file is Worng.");
        return outSwc;
    }
    else if(inputSwc1.size()!=0&&inputSwc2.size()==0)
    {
        outSwc=inputSwc1;
        return outSwc;
    }
    else if(inputSwc1.size()==0&&inputSwc2.size()!=0)
    {
        outSwc=inputSwc2;
        return outSwc;
    }
    else if(inputSwc1.size()>0&&inputSwc2.size()>=inputSwc1.size())
    {
        targetSwc=inputSwc2;
        inputSwc=inputSwc1;
    }
    else if(inputSwc2.size()>0&&inputSwc1.size()>=inputSwc2.size())
    {
        targetSwc=inputSwc1;
        inputSwc=inputSwc2;
    }
    }
    //resample the targeSwc, add point between two nodes if the distance of these nodes is too big.
    vector<XYZ> interpolationPosition =InterpolationBetweenMarkers(targetSwc);
    qDebug("interpolation size is %d",interpolationPosition.size());

    bool miok;
    double distanceThreshold=QInputDialog::getDouble(0,"distance Threshold (>0)","please input your number",50,1,150,5,&miok);


    if(MAXSIZE<inputSwc.size())
        qDebug("input swc size is very big, need to change the define of MAXSIZE.");return outSwc;
    int _countnumber=0;int _flag[MAXSIZE];
    if(miok)
    {
        cout<<"input number is "<<distanceThreshold<<endl;
        for(vector<MyMarker*>::iterator inp=inputSwc.begin();inp!=inputSwc.end();inp++)
        {
            MyMarker* inputMarker=*inp;
            _flag[_countnumber]=0;
            for(vector<MyMarker*>::iterator tari=targetSwc.begin();tari!=targetSwc.end();tari++)
            {
                MyMarker* targetMarkerT=*tari;
                if(distanceofMarker(targetMarkerT,inputMarker)<distanceThreshold)
                {
                    _flag[_countnumber]=1;
                    break;
                }
            }
            if(_flag[_countnumber]!=1)
            {
                for(vector<XYZ>::iterator interpolationp=interpolationPosition.begin();interpolationp!=interpolationPosition.end();interpolationp++)
                {
                    XYZ interpolationposition=*interpolationp;
                    if(distanceofMarker2point(inputMarker,interpolationposition)<distanceThreshold)
                    {
                         _flag[_countnumber]=1;
                         break;
                    }
                }
            }
            _countnumber++;
        }
    }

    int countnumber=0;
    for(vector<MyMarker*>::iterator inp=inputSwc.begin();inp!=inputSwc.end();inp++)
    {
        MyMarker* interMarker=*inp;
        if(_flag[countnumber]!=1)
        {
            MergeSwc.push_back(interMarker);
            outSwc.push_back(interMarker);
        }
        countnumber++;
    }
    qDebug("out swc size is %d",outSwc.size());

    vector<MyMarker*> mergeParentSwc;
    if(MergeSwc.size()>0)
    {
        for(vector<MyMarker*>::iterator se=MergeSwc.begin();se!=MergeSwc.end();se++)
        {
            MyMarker* MergeMarker=*se;
            if(MergeMarker->parent!=0)
            {
                //if parent node already in firstMergeMarker, continue;else add a parent node
                vector<MyMarker*>::iterator searchResult=find(MergeSwc.begin(),MergeSwc.end(),MergeMarker->parent);
                if(searchResult==MergeSwc.end())
                {
                    mergeParentSwc.push_back(MergeMarker);
                }
                else
                    continue;
            }
            else
                continue;
        }
    }
    qDebug("merge parent swc size is %d",mergeParentSwc.size());

    //find the nearest node in targe node of second Merge node without parent node and add.
    for(vector<MyMarker*>::iterator ses=mergeParentSwc.begin();ses!=mergeParentSwc.end();ses++)
    {
        MyMarker* mergeParentMarker =*ses;
        vector<MyMarker*> resultSWC=findNearestMarker(targetSwc,mergeParentMarker,true);
        if(resultSWC.size()>0)
        {
            for(vector<MyMarker*>::iterator re=resultSWC.begin();re!=resultSWC.end();re++)
            {
                MyMarker* resultMarker=*re;
                outSwc.push_back(resultMarker);
            }
        }
    }

    qDebug("out swc size is %d",outSwc.size());
    return outSwc;
}

////merge version1:1.merge the small size Swc file to large size SWC file.
////               2.find the same region of these two file, only working on these region.
////               3.if input node is too small to the target node, delete it. add the rest (input node) to the output swc file.
////               4.for all outMarker,find the nearest node in targe nodes and adjust postion of the parent node of those outmarker to it.
/*vector<MyMarker*> mergeSWCafterCombine(vector<MyMarker*> inputSwc1,//inputSwc nodes merge to targetSwc nodes
                                       vector<MyMarker*> inputSwc2)
{
    //vector<MyMarker*> outSwc;
    vector<MyMarker*> targetSwc,inputSwc,outSwc,firstMergeSwc,secondMergeSwc;
    //merge the small size SWC file to large size SWC file.
    if(inputSwc1.size()==0&&inputSwc2.size()==0)
    {
        qDebug("Input SWC file is Worng.");
        return outSwc;
    }
    else if(inputSwc1.size()!=0&&inputSwc2.size()==0)
    {
        outSwc=inputSwc1;
        return outSwc;
    }
    else if(inputSwc1.size()==0&&inputSwc2.size()!=0)
    {
        outSwc=inputSwc2;
        return outSwc;
    }
    else if(inputSwc1.size()>0&&inputSwc2.size()>=inputSwc1.size())
    {
        targetSwc=inputSwc2;
        inputSwc=inputSwc1;
    }
    else if(inputSwc2.size()>0&&inputSwc1.size()>=inputSwc2.size())
    {
        targetSwc=inputSwc1;
        inputSwc=inputSwc2;
    }

    double tarXmax=0,tarXmin=0,tarYmax=0,tarYmin=0,tarZmax=0,tarZmin=0;
    qDebug("targetswc size is %d",targetSwc.size());
    qDebug("inputswc size is %d",inputSwc.size());

    //firstly, put all the targertSwc marker to outSwc file.
    for(vector<MyMarker*>::iterator tar=targetSwc.begin();tar!=targetSwc.end();tar++)
    {
        MyMarker* targetMarker=*tar;
        outSwc.push_back(targetMarker);
        if(tar==targetSwc.begin())
        {
            tarXmax=tarXmin=targetMarker->x;
            tarYmax=tarYmin=targetMarker->y;
            tarZmax=tarZmin=targetMarker->z;
        }
        //caculate the margin of the region of targetSwc.
        if(targetMarker->x>=tarXmax)
            tarXmax=targetMarker->x;
        else if(targetMarker->x<=tarXmin)
            tarXmin=targetMarker->x;
        if(targetMarker->y>=tarYmax)
            tarYmax=targetMarker->x;
        else if(targetMarker->y<=tarYmin)
            tarYmin=targetMarker->y;
        if(targetMarker->z>=tarZmax)
            tarZmax=targetMarker->z;
        else if(targetMarker->z<=tarZmin)
            tarZmin=targetMarker->z;
    }
    vector<MyMarker*> intersectionSwc,targetIntersectionSwc;
    for(vector<MyMarker*>::iterator inp=inputSwc.begin();inp!=inputSwc.end();inp++)
    {
        //caculate the margin of the region of inputswc
        MyMarker* inputMarker=*inp;
        if(inputMarker->x<=tarXmax&&inputMarker->x>=tarXmin
                &&inputMarker->y<=tarYmax&&inputMarker->y>=tarYmin
                &&inputMarker->z<=tarZmax&&inputMarker->z>=tarZmin)
        {
            intersectionSwc.push_back(inputMarker);
        }
        else //put all the inputSwc marker that is outside of the target margin to outSwc file.
        {
            firstMergeSwc.push_back(inputMarker);
            outSwc.push_back(inputMarker);
        }
    }
    //Add parnet node to firstMergeSwc.
    qDebug("out swc size is %d,before adding parent node at firstly step.",outSwc.size());
    vector<MyMarker*> firstParentSwc;
    if(firstMergeSwc.size()>0)
    {
        for(vector<MyMarker*>::iterator fi=firstMergeSwc.begin();fi!=firstMergeSwc.end();fi++)
        {
            MyMarker* firstMergeMarker=*fi;
            if(firstMergeMarker->parent!=0)
            {
                //if parent node already in firstMergeMarker, continue;else add a parent node
                vector<MyMarker*>::iterator searchResult=find(firstMergeSwc.begin(),firstMergeSwc.end(),firstMergeMarker->parent);
                if(searchResult==firstMergeSwc.end())
                {
                    firstParentSwc.push_back(firstMergeMarker);
                }
                else
                    continue;
            }
            else
                continue;
        }
    }
    qDebug("first Parent Swc size is %d",firstParentSwc.size());
    //find the nearest node in targe node of firstMerge node without parent node and add.
    for(vector<MyMarker*>::iterator fis=firstParentSwc.begin();fis!=firstParentSwc.end();fis++)
    {
        MyMarker* firstParentMarker =*fis;
        MyMarker* resultMarker=findNearestMarker(targetSwc,firstParentMarker);
        if(resultMarker)
            outSwc.push_back(resultMarker);
    }
    qDebug("out swc size is %d. after adding parent node to firstly merging step",outSwc.size());
    qDebug("intersectionswc size is %d",intersectionSwc.size());

    double intersectionXmax=0,intersectionXmin=0,intersectionYmax=0,intersectionYmin=0,intersectionZmax=0,intersectionZmin=0;
    for(vector<MyMarker*>::iterator itp=intersectionSwc.begin();itp!=intersectionSwc.end();itp++)
    {
        MyMarker* tempIntersectionMarker=*itp;
        if(itp==intersectionSwc.begin())
        {
            intersectionXmax=intersectionXmin=tempIntersectionMarker->x;
            intersectionYmax=intersectionYmin=tempIntersectionMarker->y;
            intersectionZmax=intersectionZmin=tempIntersectionMarker->z;
        }
        else
        {
            if(tempIntersectionMarker->x>=intersectionXmax)
                intersectionXmax=tempIntersectionMarker->x;
            else if(tempIntersectionMarker->x<=intersectionXmin)
                intersectionXmin=tempIntersectionMarker->x;
            if(tempIntersectionMarker->y>=intersectionYmax)
                intersectionYmax=tempIntersectionMarker->y;
            else if(tempIntersectionMarker->y<=intersectionYmin)
                intersectionYmin=tempIntersectionMarker->y;
            if(tempIntersectionMarker->z>=intersectionZmax)
                intersectionZmax=tempIntersectionMarker->z;
            else if(tempIntersectionMarker->z<=intersectionZmin)
                intersectionZmin=tempIntersectionMarker->z;
        }
    }
    for(vector<MyMarker*>::iterator tar=targetSwc.begin();tar!=targetSwc.end();tar++)
    {
        MyMarker* targetMarker=*tar;
        //caculate the margin of the region of targetSwc in interaction region.
        if(targetMarker->x<=intersectionXmax&&targetMarker->x>=intersectionXmin
                &&targetMarker->y<=intersectionYmax&&targetMarker->y>=intersectionYmin
                &&targetMarker->z<=intersectionZmax&&targetMarker->z>=intersectionZmin)
        {
            targetIntersectionSwc.push_back(targetMarker);
        }
    }
    qDebug("target intersection size is %d ",targetIntersectionSwc.size());
    qDebug("out swc size is %d",outSwc.size());

    bool miok;
    double distanceThreshold=QInputDialog::getDouble(0,"distance Threshold (>0)","please input your number",150,1,500,10,&miok);

    if(MAXSIZE<intersectionSwc.size())
        qDebug("input swc size is very big, need to change the define of MAXSIZE.");return outSwc;
    int _countnumber=0;int _flag[MAXSIZE];
    if(miok)
    {
        cout<<"input number is "<<distanceThreshold<<endl;
        for(vector<MyMarker*>::iterator tart=intersectionSwc.begin();tart!=intersectionSwc.end();tart++)
        {
            MyMarker* interMarkerI=*tart;_flag[_countnumber]=0;

            for(vector<MyMarker*>::iterator tari=targetIntersectionSwc.begin();tari!=targetIntersectionSwc.end();tari++)
            {
                MyMarker* targetMarkerT=*tari;
                if(distanceofMarker(targetMarkerT,interMarkerI)<distanceThreshold)
                {
                    _flag[_countnumber]=1;
                    break;
                }

            }
            _countnumber++;
        }
    }

    int countnumber=0;
    for(vector<MyMarker*>::iterator tart=intersectionSwc.begin();tart!=intersectionSwc.end();tart++)
    {
        MyMarker* interMarkerI=*tart;
        if(_flag[countnumber]!=1)
        {
            secondMergeSwc.push_back(interMarkerI);
            outSwc.push_back(interMarkerI);
        }
        countnumber++;
    }

    vector<MyMarker*> secondParentSwc;
    if(secondMergeSwc.size()>0)
    {
        for(vector<MyMarker*>::iterator se=secondMergeSwc.begin();se!=secondMergeSwc.end();se++)
        {
            MyMarker* secondMergeMarker=*se;
            if(secondMergeMarker->parent!=0)
            {
                //if parent node already in firstMergeMarker, continue;else add a parent node
                vector<MyMarker*>::iterator searchResult=find(secondMergeSwc.begin(),secondMergeSwc.end(),secondMergeMarker->parent);
                if(searchResult==secondMergeSwc.end())
                {
                    secondParentSwc.push_back(secondMergeMarker);
                }
                else
                    continue;
            }
            else
                continue;
        }
    }
    qDebug("second Parent Swc size is %d",secondParentSwc.size());
    //find the nearest node in targe node of second Merge node without parent node and add.
    for(vector<MyMarker*>::iterator ses=secondParentSwc.begin();ses!=secondParentSwc.end();ses++)
    {
        MyMarker* secondParentMarker =*ses;
        vector<MyMarker*> resultSWC=findNearestMarker(targetSwc,secondParentMarker,true);
        if(resultSWC.size()>0)
        {
            for(vector<MyMarker*>::iterator re=resultSWC.begin();re!=resultSWC.end();re++)
            {
                MyMarker* resultMarker=*re;
                outSwc.push_back(resultMarker);
            }
        }
    }

    qDebug("out swc size is %d",outSwc.size());
    return outSwc;
}*/
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
            vector<MyMarker*> inputswc,mergeswc;
            inputswc = readSWC_file(curPathSWC.toStdString());
            if(i>0&&outswc.size()>0)
            {
                mergeswc=mergeSWCorESWC(outswc,inputswc);
            }
            else
            {
                mergeswc=inputswc;
            }
            for(V3DLONG d = 0; d < mergeswc.size(); d++)
            {
                outswc.push_back(mergeswc[d]);
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
