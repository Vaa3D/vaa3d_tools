/* auto_identify_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2012-01-01 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include <math.h>
#include "auto_identify_plugin.h"
#include "string"
#include "sstream"
#include "../../v3d_main/neuron_editing/v_neuronswc.h"
#include "../../v3d_main/neuron_tracing/neuron_tracing.h"
#include <time.h>
#include <cmath>
#include <iostream>

using namespace std;
Q_EXPORT_PLUGIN2(auto_identify, AutoIdentifyPlugin);

#define V_NeuronSWC_list vector<V_NeuronSWC>


void markers_singleChannel(V3DPluginCallback2 &callback, QWidget *parent);
void count_cells(V3DPluginCallback2 &callback, QWidget *parent);
void identify_neurons(V3DPluginCallback2 &callback, QWidget *parent);

template <class T> bool identify_cells(T* data1d, V3DLONG *dimNum, int c, const LandmarkList & markerlist, LandmarkList & bglist, LandmarkList & outputlist);

LandmarkList neuron_2_mark(const NeuronTree & p, LandmarkList & neuronMarkList);
template <class T> int pixelVal(T* data1d, V3DLONG *dimNum,
                                double xc, double yc, double zc, int c);
template <class T> LocationSimple mass_center(T* data1d,
                                              V3DLONG *dimNum,
                                              double xc, double yc, double zc, double rad, int c);
template <class T> pair<int,int> pixel_range(T* data1d,
                                        V3DLONG *dimNum,
                                        double xc,double yc,double zc,int c,double rad);
template <class T> bool dynamic_pixel(T* data1d,
                                        V3DLONG *dimNum,
                                        double xc, double yc, double zc,
                                        int c, int PixVal, int BGVal, double & dataAve, double & rad);
template <class T> LandmarkList scan_and_count(T* data1d,
                                      V3DLONG *dimNum,
                                      int MarkAve, int MarkStDev,
                                      int PointAve, int PointStDev,
                                      int rad, double radAve, double radStDev, int c, int cat);
template <class T> LandmarkList duplicates(T* data1d, LandmarkList fullList,
                                           V3DLONG *dimNum, int PointAve, int rad, int c);
V_NeuronSWC get_v_neuron_swc(const NeuronTree *p);
V_NeuronSWC_list get_neuron_segments(const NeuronTree *p);
//NeuronTree VSWClist_2_neuron_tree(V_NeuronSWC_list *p);
NeuronTree VSWC_2_neuron_tree(V_NeuronSWC *p, int id);
NeuronSWC make_neuron_swc(V_NeuronSWC_unit *p, int id, bool &start);
template <class T> bool compute_swc_radius(T* data1d, V3DLONG *dimNum, vector<V_NeuronSWC_unit> segment,
                                           int c, double & outputRadAve, double & outputRadStDev);
bool export_list2file(QList<NeuronTree> & N2, QString fileSaveName, QString fileOpenName);
template <class T> bool apply_mask(unsigned char* data1d, V3DLONG *dimNum,
                                   int xc, int yc, int zc, int c, double PixVal, double BGVal, T & maskImg);
bool open_testSWC(NeuronTree & openTree);

 
QStringList AutoIdentifyPlugin::menulist() const
{
	return QStringList() 
        <<tr("Single Channel Cell Counting")
        <<tr("Better Cell Counting")
        <<tr("Neurons")
		<<tr("about");
}

QStringList AutoIdentifyPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void AutoIdentifyPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Single Channel Cell Counting"))
	{
        //markers_singleChannel(callback,parent);
        v3d_msg("Please use the other one, this one is outdated and probably full of bugs. Sorry :<");
	}
    else if (menu_name == tr("Better Cell Counting"))
	{
        count_cells(callback,parent);
	}
    else if (menu_name == tr("Neurons"))
    {
        //v3d_msg("To be implemented"); return;
        identify_neurons(callback,parent);
    }
	else
	{
        v3d_msg(tr("Uses current image's landmarks to find similar objects in image."
            "Work-in-process by Xindi, 2014 Summer"));
	}
}


bool AutoIdentifyPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

/*  ##################################
 * [completed tasks]
 * test neuron SWC needs to be its own structure marked with the comment "test"
 * radius calculation of branches works
 * average intensity of branches works
 * categorizing non-test data sets based on test data works
 * saving newly labeled SWC file works
 * radius StDev of branches works
 * can load test data directly from file
 *
 * [current goals/issues]
 * updating window to reflect type changes does not work yet
 * some coords are returning as indeterminant in the compute_radius function. Right now they are being skipped. Not sure source of bad numbers.
 *
 * [future goals]
 * find way to identify individal segments rather than entire structures as test data. NOT SURE IF EVEN POSSIBLE
 * optimize radius calculation algorithm to be more robust, figure out why it's even capable of returning bad numbers
 *  ##################################
*/
void identify_neurons(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();

    //cancels if no image
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    //if image, pulls the data
    Image4DSimple* p4DImage = callback.getImage(curwin); //the data of the image is in 4D (channel + 3D)

    unsigned char* data1d = p4DImage->getRawData(); //sets data into 1D array

    QString curfilename = callback.getImageName(curwin);

    //defining the dimensions
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    //storing the dimensions
    V3DLONG dimNum[4];
    dimNum[0]=N; dimNum[1]=M; dimNum[2]=P; dimNum[3]=sc;

    //input channel
    unsigned int c=1;
    bool ok;
    if (sc==1)
        c=1; //if only using 1 channel
    else
        c = QInputDialog::getInteger(parent, "Channel", "Enter Channel Number", 1, 1, sc, 1,&ok);

    QList<NeuronTree> * mTreeList;
    mTreeList = callback.getHandleNeuronTrees_3DGlobalViewer(curwin);
    NeuronTree mTree;
    if (mTreeList->isEmpty()) { v3d_msg("There are no neuron traces in the current window."); return; }
    else
    {
        vector<int> segCatArr(10);
        vector<double> segRadArr(10);
        vector<double> segIntensArr(10);
        vector<double> segRadStDevArr(10);

        //open test example SWC
        NeuronTree openTree;
        if (open_testSWC(openTree))
        {
            openTree.comment = "test";
            mTreeList->append(openTree);
        }
        int structNum = mTreeList->count();

        //get examples from test data
        for (int i=0; i<structNum; i++)
        {
            mTree = mTreeList->at(i);

            if (mTree.comment != "test") continue; //defining the testing set by comments
            //v3d_msg("test data");

            V_NeuronSWC_list seg_list = get_neuron_segments(&mTree);
            //syntax: list.at(i) is segment, segment.row is vector of units, vector.at(i) is unit, unit.type is category
            int segNum = seg_list.size();
            //v3d_msg(QString("read in test tree with %1 segments").arg(segNum));
            for (int j=0; j<segNum; j++)
            {
                //segCatArr.push_back(seg_list.at(j).row.at(0).type);
                int segCat = seg_list.at(j).row.at(0).type;
                cout<<segCat<<endl;
                segCatArr[segCat] += 1;
                double radAve,radStDev;
                compute_swc_radius(data1d,dimNum,seg_list.at(j).row,c,radAve,radStDev);
                //v3d_msg("got rad");
                double x,y,z,intensity=0;
                for (int k=0; k<seg_list.at(j).row.size(); k++)
                {
                    x=seg_list.at(j).row.at(k).x;
                    y=seg_list.at(j).row.at(k).y;
                    z=seg_list.at(j).row.at(k).z;
                    intensity += pixelVal(data1d,dimNum,x,y,z,c);
                }
                double intensAve = intensity/seg_list.at(j).row.size();
                //segRadArr.push_back(radAve);
                //segIntensArr.push_back(intensAve);
                //segRadStDevArr.push_back(radStDev);
                segRadArr[segCat] += radAve;
                segIntensArr[segCat] += intensAve;
                segRadStDevArr[segCat] += radStDev;
                //v3d_msg(QString("cat %1 rad %2").arg(seg_list.at(j).row.at(0).type).arg(radAve));
            }
            for (int k=0; k<segCatArr.size(); k++)
            {
                if (segCatArr[k]==0) continue;
                segRadArr[k] /= segCatArr[k];
                segIntensArr[k] /= segCatArr[k];
                segRadStDevArr[k] /= segCatArr[k];
            }
        }
//        v3d_msg(QString("%1 %2 %3 %4").arg(segCatArr.size()).arg(segCatArr.at(0)).arg(segRadArr.size()).arg(segRadArr.at(0)));

        //label remaining neurons using test data
        NeuronTree newTree;
        QList<NeuronTree> newTreeList;
        for (int i=0; i<structNum; i++) //loops through neuron structures as numbered in object manager
        {
            mTree = mTreeList->at(i);

            if (mTree.comment == "test") continue; //defining the testing set by comments
            //v3d_msg("labeling data");

            V_NeuronSWC_list seg_list = get_neuron_segments(&mTree);
            //syntax: list.at(i) is segment, segment.row is vector of units, vector.at(i) is unit, unit.type is category
            int segNum = seg_list.size();
            int id=1;
            //v3d_msg(QString("read in segment of length %1").arg(segNum));
            for (int j=0; j<segNum; j++) //loops through segments within one structure
            {
                double radAve,radStDev;
                compute_swc_radius(data1d,dimNum,seg_list.at(j).row,c,radAve,radStDev);
                double x,y,z,intensity=0;
                for (int k=0; k<seg_list.at(j).row.size(); k++)
                {
                    x=seg_list.at(j).row.at(k).x;
                    y=seg_list.at(j).row.at(k).y;
                    z=seg_list.at(j).row.at(k).z;
                    intensity += pixelVal(data1d,dimNum,x,y,z,c);
                }
                double intensAve = intensity/seg_list.at(j).row.size();
                //v3d_msg(QString("radius %1").arg(radAve));
                double diffRad,diffInt,diffRadStD,diff,diff_min=255;
                int cur_type=3;
                for (int k=0; k<segCatArr.size(); k++)
                {
                    if (segCatArr.at(k)==0) continue;
                    diffRad = abs(radAve-segRadArr.at(k));
                    diffInt = abs(intensAve-segIntensArr.at(k));
                    diffRadStD = abs(radStDev-segRadStDevArr.at(k));
                    diff = (2*diffRad+diffInt+3*diffRadStD)/6; //depending on further testing, may end up weighing this average differently
                    //v3d_msg(QString("diff %3 between calculated %1 and test %2").arg(radAve).arg(segRadArr.at(k)).arg(diff));
                    if (diff<diff_min)
                    {
                        cur_type=k;
                        diff_min=diff;
                    }
                }
                //v3d_msg(QString("identified as type %1").arg(cur_type));
                for (int l=0; l<seg_list.at(j).row.size(); l++)
                {
                    seg_list.at(j).row.at(l).type=cur_type; //sets every unit in segment to be new type
                }
                //attempting to draw in new updated neuron trees, not working...
                newTree = VSWC_2_neuron_tree(&seg_list.at(j),id); //translates segment into a NeuronTree
                id += seg_list.at(j).row.size();
                //callback.setSWC(curwin,newTree);
                //mTreeList->replace(i,newTree);
                newTreeList.append(newTree);
                //v3d_msg(QString("changed segment %1 of rad %3 to type %2").arg(j).arg(cur_type).arg(radAve));
            }

        }
        //need to draw newTreeList into window and remove mTreeList
        //*mTreeList = newTreeList;
        //callback.updateImageWindow(curwin);
        export_list2file(newTreeList,curfilename+"_Labeled_SWC.swc",curfilename);

        /*v3dhandle newwin = callback.newImageWindow(curfilename+" Labeled SWC");
        unsigned char *outputData = 0;
        outputData = new unsigned char [N*M*P*sc];
        Image4DSimple outputImage;
        outputImage.setData((unsigned char*)outputData, N, M, P, sc, V3D_UINT8);
        callback.setImage(newwin,&outputImage);
        callback.open3DWindow(newwin);
        for (int i=0; i<newTreeList.size(); i++)
        {
            NeuronTree drawTree = newTreeList.at(i);
            callback.setSWC(newwin,drawTree);
        }*/
    }

    return;
}


/*  ##################################
 * [completed tasks]
 * all main algorithms are functional, may not be optimized
 * added in use of corner voxels as negative examplers
 * improved threshold algorithm
 * significantly reduced duplicate detection of cells
 * mass center fixed
 *
 * [current goals/issues]
 * still several undetected cells
 * scan_and_count a little slow
 * because of variable cell radius, mass_center does not work equally well across all cells
 *      optimizing to center larger cells would cause clustered cells to be treated as one
 *      keeping small radius to keep clustered cells separate prevents detection of true center of larger cells
 * Cannot detect cells if they are too small in the image
 *
 * [future goals]
 *
 *  ##################################
*/
void count_cells(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();

    //cancels if no image
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    //if image, pulls the data
    Image4DSimple* p4DImage = callback.getImage(curwin); //the data of the image is in 4D (channel + 3D)

    unsigned char* data1d = p4DImage->getRawData(); //sets data into 1D array

    //defining the dimensions
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    //storing the dimensions
    V3DLONG dimNum[4];
    dimNum[0]=N; dimNum[1]=M; dimNum[2]=P; dimNum[3]=sc;

    LandmarkList Marklist = callback.getLandmark(curwin);
    int Marknum = Marklist.count();

    QList<NeuronTree> * mTreeList=0;
    mTreeList = callback.getHandleNeuronTrees_3DGlobalViewer(curwin);

    //check data availability
    if (Marknum<=0 && !mTreeList)
    {
        v3d_msg("you have not specified any marker or swc structure to run this program.");
        return;
    }

    int SWCcount;
    NeuronTree mTree;
    if (mTreeList->isEmpty()) { SWCcount = 0; }
    else
    {
        mTree = mTreeList->first();
        SWCcount = mTree.listNeuron.count();
    }
    //NeuronTree mTree = callback.getSWC(curwin);
    //int SWCcount = mTree.listNeuron.count();

    //input test data type
    int option;
    if (Marknum != 0 && SWCcount ==0 ) { option = 1; }
    else if (Marknum == 0 && SWCcount != 0) { option = 2; }
    else
    {
        QString qtitle = QObject::tr("Choose Test Data Input Type");
        bool ok;
        QStringList items;
        items << "Markers" << "3D Curves" << "Markers and 3D Curves";
        QString item = QInputDialog::getItem(0, qtitle,
                                            QObject::tr("Which type of testing data are you using"), items, 0, false, &ok);
        if (! ok) return;
        int input_type = items.indexOf(item);
        if (input_type==0) { option = 1; }
        else if (input_type==1){ option = 2; }
        else { option = 3; }
    }

    LandmarkList mlist, neuronMarkList;
    if (option == 1)
    {
        mlist = Marklist;
    }
    else if (option == 2 )
    {
        neuronMarkList = neuron_2_mark(mTree,neuronMarkList);
        mlist = neuronMarkList;
    }
    else
    {
        neuronMarkList = neuron_2_mark(mTree,neuronMarkList);
        mlist = Marklist;
        mlist.append(neuronMarkList);
    }

    bool ok;

    //input channel
    unsigned int c=1;
    if (sc==1)
        c=1; //if only using 1 channel
    else
        c = QInputDialog::getInteger(parent, "Channel", "Enter Channel Number", 1, 1, sc, 1,&ok);


    //input sort method
    QString qtitle = QObject::tr("Choose Sorting Method");
    QStringList items;
    items << "Default (binary color threshold)" << "By Type";
    QString item = QInputDialog::getItem(0, qtitle,
                                         QObject::tr("How should the test data be sorted?"), items, 0, false, &ok);
    if (! ok) return;
    int input_type = items.indexOf(item);
    if (input_type==0) //default
    {
        if (mlist.count()<=0)    {v3d_msg("There are no markers in the current image"); return;}

        LandmarkList bglist; //sending in empty bglist to trigger binary sort
        LandmarkList smallList;
        if (identify_cells(data1d,dimNum,c,mlist,bglist, smallList))
        {
            callback.setLandmark(curwin,smallList);
        }
    }
    else if (input_type==1) //type
    {
        //int catNum = QInputDialog::getInt(0,"Number of Categories","Enter number of categories (background category included), if unsure enter 0",0,0,100,1,&ok);
        //Can't think of reason having user inputed cat number would be better than auto counting

        int * catList;
        catList = new int[mlist.count()];
        if (mlist.count()<=0)    {v3d_msg("There are no neuron traces in the current image"); return;}
        LocationSimple tempInd;
        for (int i=0; i<mlist.count(); i++)
        {
            tempInd = mlist.at(i);
            catList[i] = tempInd.category;
//            v3d_msg((QString("hi %1, cat %2").arg(i).arg(catList[i])));
        }

        //counts number of categories
        int catNum=0;
        for (int i=0; i<mlist.count(); i++)
        {
            for (int j=0; j<mlist.count(); j++)
            {
                if (catList[j]==i)
                {
                    catNum++;
                    break;
                }
            }
        }

//v3d_msg(QString("final catNum %1").arg(catNum));


//v3d_msg("start indexing");
        map<int,LandmarkList> catArr;
        LandmarkList temp;
        int row=0;
        for (int catval=0; catval<mlist.count(); catval++) //loop through category values
        {
            int x=0;
            for (int index=0; index<mlist.count(); index++) //loop through markers
            {
                if (catList[index]==catval)
                {
                    x++;
                    temp.append(mlist.at(index));
                }
            }
//            v3d_msg(QString("found %1 values for catVal %2").arg(x).arg(catval));
            if (x==0)
                continue;
            else
            {
                catArr.insert(make_pair(row,temp));
//v3d_msg(QString("row %1 cat %2").arg(row).arg(temp.at(0).category));
                row++;
                temp.clear();
            }
        }
        if (catList) {delete [] catList; catList=0;}

//        v3d_msg("indexing complete");


//v3d_msg("arrays made");
        //run script
        LandmarkList catSortList;
        LandmarkList * marks;
        LandmarkList * bgs = &catArr[0]; //working with assumption that bg has category value 0;
        for (int i=0; i<catNum-1; i++)
        {
//v3d_msg(QString("catSortList start iteration %1").arg(i));
            /*LandmarkList marksL;
            for (int j=0; i<catInd[i+1]; j++)
            {
                marksL.append(mlist.at(catArr[i+1][j]));
            }
            marks = &marksL;*/
            marks = &catArr[i+1];
//v3d_msg(QString("marks %1").arg(marks->count()));
            LandmarkList tempList;
            if (identify_cells(data1d,dimNum,c,*marks,*bgs, tempList))
                catSortList.append(tempList);
//v3d_msg(QString("catSortList append category %1").arg(tempList.at(0).category));
//            marksL.clear();
        }

        callback.setLandmark(curwin,catSortList);

    }
    return;
}

template <class T> bool identify_cells(T* data1d, V3DLONG *dimNum, int c, const LandmarkList & markerlist, LandmarkList & bglist, LandmarkList & outputlist)
{
    if (!data1d || !dimNum)
        return false;

    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];

    LandmarkList mlist, MarkList, BGList;
    LocationSimple tmpLocation(0,0,0);
    int xc,yc,zc, marks;
    double PixVal,BGVal;
    if (bglist.isEmpty()) //binary sorting
    {
        //sort markers by background/foreground

        //add corner pixels to list to use as additional bg markers
        mlist = markerlist;
        for (int i=0; i<=N; i+=N)
        {
            for (int j=0; j<=M; j+=M)
            {
                for (int k=0; k<=P; k+=P)
                {
                    LocationSimple extraBG(i,j,k);
                    mlist.append(extraBG);
                }
            }
        }

//v3d_msg("presort ckpt");

        int pix,num;
        int marknum = mlist.count();
        int * PixValArr=0;
        PixValArr = new int[marknum];
//v3d_msg("sort ckpt 1");
        for (int i=0; i<marknum; i++)
        {
            tmpLocation = mlist.at(i);
            tmpLocation.getCoord(xc,yc,zc);
            pix = pixelVal(data1d,dimNum,xc,yc,zc,c);
            //cout<<"value "<<pix<<" at coords "<<xc<<" "<<yc<<" "<<zc<<endl;
            if (pix<0 || pix>255) {v3d_msg("pix is wrong"); return false;}
            //      v3d_msg(QString("pix value %1 %2").arg(pix).arg(pix1));
            PixValArr[i] = pix;
        }

        /*int max=0,min=255;
        for (int i=0; i<marknum; i++)
        {
            num=PixValArr[i];
            if (num>max) { max=num; }
            if (num<min) { min=num; }
        }
        int thresh = (max+min)/2; //this definitely should be changed!!! commented by PHC*/

//v3d_msg("starting thresh calc");
        vector<int> pValHist(52,0); //precise histogram for getting threshold value
        vector<int> pValHist_smooth(26,0); //smoothed histogram for finding valley
        for (int i=0; i<=marknum; i++)
        {
            int histInd = PixValArr[i]/5;
            int smoothHistInd = PixValArr[i]/10;
            if (histInd<0) histInd=0;
            if (histInd>51) histInd=51;
            if (smoothHistInd<0) smoothHistInd=0;
            if (smoothHistInd>25) smoothHistInd=25;
            num = pValHist[histInd];
            pValHist[histInd] = num+1;
            num = pValHist_smooth[smoothHistInd];
            pValHist_smooth[smoothHistInd] = num+1;
        }
//v3d_msg("both hist made");
        vector<int> localMaxs;
        if (pValHist_smooth[0]>pValHist_smooth[1]) localMaxs.push_back(0);
        int num0,num1,num2;
        for (int i=1; i<=24; i++)
        {
            num0=pValHist_smooth[i-1];
            num1=pValHist_smooth[i];
            num2=pValHist_smooth[i+1];
            //cout<<num0<<" "<<num1<<" "<<num2<<endl;
            if (num1!=0 && num1>=num0 && num1>=num2) {localMaxs.push_back(i);}
        }
        if (pValHist_smooth[25]>pValHist_smooth[24]) localMaxs.push_back(25);
//v3d_msg("localMaxs made");
        int max=2,valleyPos=0,diff=0;
        for (int i=0; i<localMaxs.size()-1; i++)
        {
            diff = localMaxs[i+1]-localMaxs[i];
            if (diff>max) {valleyPos=i; break;}
        }
//v3d_msg("valleyPos found");
        valleyPos *= 2;
        int thresh=0;
        if (valleyPos<=0) valleyPos=1;
        if (valleyPos>=51) valleyPos=50;
        if (pValHist[valleyPos-1]==pValHist[valleyPos]==pValHist[valleyPos+1]) thresh=valleyPos*5;
        int end=valleyPos+diff*2;
        if (end>50) end=50;
        for (int i=valleyPos; i<=end; i++)
        {
            num0=pValHist[i-1];
            num1=pValHist[i];
            num2=pValHist[i+1];
            if (num1<=num0 && num1<=num2) {thresh=i*5;}
        }

        cout<<"threshold value "<<thresh<<endl<<endl;

        PixVal=0, BGVal=0;
        for (int i=0; i<marknum; i++)
        {
            num=PixValArr[i];
            tmpLocation = mlist.at(i);
            if (num<thresh) { BGList.append(tmpLocation); BGVal += num; }    //BGList holds bg markers
            if (num>thresh) { MarkList.append(tmpLocation); PixVal += num; }  //MarkList holds cell markers
        }
    }
    else    //comment sorting
    {
        MarkList = markerlist;
        PixVal=0, BGVal=0;
        for (int i=0; i<MarkList.count(); i++)
        {
            tmpLocation = MarkList.at(i);
            tmpLocation.getCoord(xc,yc,zc);
            int pix = pixelVal(data1d,dimNum,xc,yc,zc,c);
            PixVal += pix;
        }
        BGList = bglist;
        for (int i=0; i<BGList.count(); i++)
        {
            tmpLocation = BGList.at(i);
            tmpLocation.getCoord(xc,yc,zc);
            int pix = pixelVal(data1d,dimNum,xc,yc,zc,c);
            BGVal += pix;
        }
    }

    marks = MarkList.count();

    if (MarkList.count()<=0 || BGList.count()<=0)
    {
        v3d_msg("Either the foreground marker list or the background marker list is empty. Quit.");
        return false;
    }

    PixVal = PixVal/marks;          //PixVal now stores average pixel value of all cell markers
    BGVal = BGVal/BGList.count();   //BGVal now stores average pixel value of all background markers
    int cat = MarkList.at(0).category;
//    v3d_msg(QString("PixVal %1, pixCount %2, BGVal %3, BGCount %4").arg(PixVal).arg(marks).arg(BGVal).arg(BGList.count()));


//v3d_msg(QString("marks = %1, bgcount = %2. Marks all sorted").arg(marks).arg(BGList.count()));


    //recalibrates marker list by mean shift
    LandmarkList tempList;
    LocationSimple temp(0,0,0),newMark(0,0,0);
    //int tempPix;
    for (int i=0; i<marks; i++)
    {
        temp = MarkList.at(i);
        temp.getCoord(xc,yc,zc);
        //tempPix = pixelVal(data1d,dimNum,xc,yc,zc,c);
        for (int j=0; j<10; j++)
        {
            newMark = mass_center(data1d,dimNum,xc,yc,zc,15,c);
            newMark.getCoord(xc,yc,zc);
        }
        tempList.append(newMark);
    }
    MarkList = tempList;
    //outputlist = tempList; return true;

    //scan list of cell markers for ValAve, radAve

    int * ValAveArr; int * radAveArr;
    ValAveArr = new int[marks]; radAveArr = new int[marks];
    LocationSimple tempLocation(0,0,0);
    double ValAve=0,radAve=0,tmpDataAve,tmpRad;

    for (int i=0; i<marks; i++)
    {
        tempLocation = MarkList.at(i);
        tempLocation.getCoord(xc,yc,zc);
        int Pix = pixelVal(data1d,dimNum,xc,yc,zc,c);

        if (dynamic_pixel(data1d,dimNum,xc,yc,zc,c,Pix,BGVal,tmpDataAve,tmpRad))
        {
            ValAveArr[i] = tmpDataAve;
            radAveArr[i] = tmpRad;
            ValAve += ValAveArr[i];
            radAve += radAveArr[i];
        }

//v3d_msg(QString("ValAve %1, radAve %2").arg(ValAve).arg(radAve));
    }


//v3d_msg("scan checkpoint");

    ValAve /= marks;  //average pixel value of each segment
    radAve /= marks;  //average radius of segment

//v3d_msg(QString("FINAL ValAve %1, radAve %2").arg(ValAve).arg(radAve));

    double stV=0, stR=0, stP=0;
    for (int i=0; i<marks; i++)
    {
        double s = (ValAveArr[i]-ValAve)*(ValAveArr[i]-ValAve);
        stV += s;

        double t = (radAveArr[i]-radAve)*(radAveArr[i]-radAve);
        stR += t;

        tempLocation = MarkList.at(i);
        tempLocation.getCoord(xc,yc,zc);
        int Pix = pixelVal(data1d,dimNum,xc,yc,zc,c);
//        v3d_msg(QString("Pix %1 and PixVal %2").arg(Pix).arg(PixVal));
        double u = (Pix-PixVal)*(Pix-PixVal);
        stP += u;
//v3d_msg(QString("pixel value %1, diff %2, stP %3").arg(Pix).arg(Pix-PixVal).arg(stP));
    }
    double ValStDev = sqrt(stV/marks);
    double radStDev = sqrt(stR/marks);
    double PixStDev = sqrt(stP/marks);


//v3d_msg(QString("markers have been scanned. Pixval %1 and stdev %2. radVal %3 and stdev %4."
//                      "segVal %5 and stdev %6").arg(PixVal).arg(PixStDev).arg(radAve).arg(radStDev).arg(ValAve).arg(ValStDev));
//v3d_msg(QString("category %1").arg(cat));

    //scans image and generates new set of markers based on testing data
    LandmarkList newList = scan_and_count(data1d,dimNum,ValAve,2*ValStDev,PixVal,PixStDev,0,radAve,radStDev,c,cat);
    cout<<"Cell count "<<newList.count()<<endl;
    //recenters list via mean shift
//v3d_msg("recentering");
    LandmarkList tempL2;
    LocationSimple temp2;
    for (int i=0; i<newList.count(); i++)
    {
        temp2 = newList.at(i);
        temp2.getCoord(xc,yc,zc);
        for (int j=0; j<10; j++)
        {
            newMark = mass_center(data1d,dimNum,xc,yc,zc,radAve,c);
            newMark.getCoord(xc,yc,zc);
            newMark.category = cat;
            stringstream catStr;
            catStr << cat;
            newMark.comments = catStr.str();
        }
        tempL2.append(newMark);
    }
//v3d_msg("ckpt 2");
    newList = tempL2;

//v3d_msg("newList made");


    //        LandmarkList& woot = newList;
    //        bool draw_le_markers = callback.setLandmark(curwin,woot);
    //        v3d_msg(QString("newList has %1 markers").arg(newList.count()));

    //deletes duplicate markers based on their proximity

    outputlist = duplicates(data1d,newList,dimNum,PixVal,radAve,c);
//    outputlist = newList;
//v3d_msg("duplicates deleted");

    return true;
}




//returns pixel value of marker
template <class T> int pixelVal(T* data1d, V3DLONG *dimNum,
                                double xc, double yc, double zc, int c)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    V3DLONG shiftC = (c-1)*P*M*N;
    if (xc<=0) xc=1; if (xc>N) xc=N;
    if (yc<=0) yc=1; if (yc>M) yc=M;
    if (zc<=0) zc=1; if (zc>P) zc=P;
    double pixelVal = data1d[ shiftC + (V3DLONG)(zc-1)*M*N + (V3DLONG)(yc-1)*N + (V3DLONG)(xc-1) ];
    if (pixelVal<0) pixelVal=0;
    if (pixelVal>255) pixelVal=255;
    return pixelVal;
}


//returns new marker that has been recentered
template <class T> LocationSimple mass_center(T* data1d,
                                              V3DLONG *dimNum,
                                              double xc, double yc, double zc, double rad, int c)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];

    double pVal, newX=0, newY=0, newZ=0, norm=0;

    /*//defining limits
    V3DLONG xLow = xc-rad; if(xLow<0) xLow=0;
    V3DLONG xHigh = xc+rad; if(xHigh>N-1) xHigh=N-1;
    V3DLONG yLow = yc-rad; if(yLow<0) yLow=0;
    V3DLONG yHigh = yc+rad; if(yHigh>M-1) yHigh=M-1;
    V3DLONG zLow = zc-rad; if(zLow<0) zLow=0;
    V3DLONG zHigh = zc+rad; if(zHigh>P-1) zHigh=P-1;

    //scanning through the pixels
    V3DLONG k,j,i;
    for (k = zLow; k <= zHigh; k++)
    {
         for (j = yLow; j <= yHigh; j++)
         {
             for (i = xLow; i <= xHigh; i++)
             {
                 double t = (i-xc)*(i-xc)+(j-yc)*(j-yc)+(k-zc)*(k-zc);
                 double dist = sqrt(t);
                 if (dist<=rad)
                 {
                     pVal = pixelVal(data1d,dimNum,i,j,k,c);
                     newX += pVal*i;
                     newY += pVal*j;
                     newZ += pVal*k;
                     norm += pVal;
                 }
             }
         }
    }*/

    double x,y,z,pi=3.14;
    //rad = 15;

    for (double r=rad/5; r<=rad; r+=rad/5)
    {
        for (double theta=0; theta<2*pi; theta+=(pi/4))
        {
            for (double phi=0; phi<pi; phi+=(pi/4))
            {
                //cout<<"pixel iteration "<<runs<<endl;
                //cout<<r<<" "<<theta<<" "<<phi<<endl;
                x = xc+r*cos(theta)*sin(phi);
                if (x>N-1) x=N-1; if (x<0) x=0;
                y = yc+r*sin(theta)*sin(phi);
                if (y>M-1) y=M-1; if (y<0) y=0;
                z = zc+r*cos(phi);
                if (z>P-1) z=P-1; if (z<0) z=0;
                pVal = pixelVal(data1d,dimNum,x,y,z,c);
                newX += pVal*x;
                newY += pVal*y;
                newZ += pVal*z;
                norm += pVal;
                //cout<<dataval<<" "<<datatotal<<" "<<runs<<endl;
                //cout<<x<<" "<<y<<" "<<z<<" "<<endl<<endl;
            }
        }
    }

    newX /= norm;
    newY /= norm;
    newZ /= norm;

    LocationSimple newMark(newX,newY,newZ);
//    v3d_msg(QString("New coords %1 %2 %3 vs old coords %4 %5 %6").arg(newX).arg(newY).arg(newZ).arg(xc).arg(yc).arg(zc));
    return newMark;
}


//returns average pixel value and radius of cell around a marker
template <class T> bool dynamic_pixel(T* data1d,
                                        V3DLONG *dimNum,
                                        double xc, double yc, double zc,
                                        int c, int PixVal, int BGVal, double & dataAve, double & rad)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    V3DLONG shiftC = (c-1)*P*M*N;

    do
    {
        rad++;
        /*//defining limits
        V3DLONG xLow = xc-rad; if(xLow<0) xLow=0;
        V3DLONG xHigh = xc+rad; if(xHigh>N-1) xHigh=N-1;
        V3DLONG yLow = yc-rad; if(yLow<0) yLow=0;
        V3DLONG yHigh = yc+rad; if(yHigh>M-1) yHigh=M-1;
        V3DLONG zLow = zc-rad; if(zLow<0) zLow=0;
        V3DLONG zHigh = zc+rad; if(zHigh>P-1) zHigh=P-1;

        //scanning through the pixels
        V3DLONG k,j,i;
        //average data of each segment
        int datatotal=0,runs=0;
        for (k = zLow; k <= zHigh; k++)
        {
            V3DLONG shiftZ = k*M*N;
            for (j = yLow; j <= yHigh; j++)
            {
                V3DLONG shiftY = j*N;
                for (i = xLow; i <= xHigh; i++)
                {
                    double t = (i-xc)*(i-xc)+(j-yc)*(j-yc)+(k-zc)*(k-zc);
                    double dist = sqrt(t);
                    if (dist<=rad)
                    {
                        int dataval = data1d[ shiftC + shiftZ + shiftY + i ];
                        datatotal += dataval;
                        runs++;
                    }
                }
            }
        }
        dataAve = datatotal/runs;*/

        double x,y,z,datatotal=0,pi=3.14;;
        int runs=0;
        for (double theta=0; theta<2*pi; theta+=(pi/4))
        {
            for (double phi=0; phi<pi; phi+=(pi/4))
            {
                //cout<<"pixel iteration "<<runs<<endl;
                //cout<<r<<" "<<theta<<" "<<phi<<endl<<endl;
                x = xc+rad*cos(theta)*sin(phi);
                if (x>N) x=N; if (x<0) x=0;
                y = yc+rad*sin(theta)*sin(phi);
                if (y>M) y=M; if (y<0) y=0;
                z = zc+rad*cos(phi);
                if (z>P) z=P; if (z<0) z=0;
                double dataval = pixelVal(data1d,dimNum,x,y,z,c);
                datatotal += dataval;
                //cout<<dataval<<" "<<datatotal<<endl;
                runs++;
                //cout<<x<<" "<<y<<" "<<z<<" "<<endl;
            }
        }

        runs++;
        dataAve = datatotal/runs;
    } while ( dataAve > (PixVal+2*BGVal)/3 );

    return true;
}


//returns average pixel value in box of radius rad on channel c around a given marker as well as average cell marker intensity
template <class T> pair<int,int> pixel_range(T* data1d,
                              V3DLONG *dimNum,
                              double xc, double yc, double zc, int c, double rad)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];

    /*V3DLONG shiftC = (c-1)*P*M*N;

    //defining limits
    V3DLONG xLow = xc-rad; if(xLow<0) xLow=0;
    V3DLONG xHigh = xc+rad; if(xHigh>N-1) xHigh=N-1;
    V3DLONG yLow = yc-rad; if(yLow<0) yLow=0;
    V3DLONG yHigh = yc+rad; if(yHigh>M-1) yHigh=M-1;
    V3DLONG zLow = zc-rad; if(zLow<0) zLow=0;
    V3DLONG zHigh = zc+rad; if(zHigh>P-1) zHigh=P-1;

    //scanning through the pixels
    V3DLONG k,j,i;
    //average data of each segment
    int datatotal1=0,runs1=0;
    double t,dist;
    for (k = zLow; k <= zHigh; k++)
    {
         V3DLONG shiftZ = k*M*N;
         for (j = yLow; j <= yHigh; j++)
         {
             V3DLONG shiftY = j*N;
             for (i = xLow; i <= xHigh; i++)
             {
                 t = (i-xc)*(i-xc)+(j-yc)*(j-yc)+(k-zc)*(k-zc);
                 dist = sqrt(t);
                 if (dist<=rad)
                 {
                     int dataval = data1d[ shiftC + shiftZ + shiftY + i ];
                     datatotal1 += dataval;
                     runs1++;
                 }
                 else continue;
             }
         }
    }

    int dataAve1 = datatotal1/runs1;
    //cout<<endl<<"original coords "<<xc<<" "<<yc<<" "<<zc<<" "<<endl;
    if (dataAve1>=100) cout<<"first algorithm "<<dataAve1<<endl;*/

    double x,y,z,datatotal=0,pi=3.14;
    int runs=0;
    double r = rad;

    for (double theta=0; theta<2*pi; theta+=(pi/4))
    {
        for (double phi=0; phi<pi; phi+=(pi/4))
        {
            //cout<<"pixel iteration "<<runs<<endl;
            //cout<<r<<" "<<theta<<" "<<phi<<endl;
            x = xc+r*cos(theta)*sin(phi);
            if (x>N-1) x=N-1; if (x<0) x=0;
            y = yc+r*sin(theta)*sin(phi);
            if (y>M-1) y=M-1; if (y<0) y=0;
            z = zc+r*cos(phi);
            if (z>P-1) z=P-1; if (z<0) z=0;
            double dataval = pixelVal(data1d,dimNum,x,y,z,c);
            datatotal += dataval;
            runs++;
            //cout<<dataval<<" "<<datatotal<<" "<<runs<<endl;
            //cout<<x<<" "<<y<<" "<<z<<" "<<endl<<endl;
        }
    }
    //cout<<"done with loop"<<endl;
    int pointval = pixelVal(data1d,dimNum,xc,yc,zc,c);
    //v3d_msg(QString("pointval %1").arg(pointval));
    //cout<<pointval<<endl;
    int dataAve;
    if (runs==0) dataAve=0;
    else dataAve = datatotal/runs;

    //cout<<"second algorithm "<<dataAve<<endl;

    return make_pair(dataAve,pointval);
}


//uses test data to scan and mark other cells
template <class T> LandmarkList scan_and_count(T* data1d,
                              V3DLONG *dimNum,
                              int MarkAve, int MarkStDev,
                              int PointAve, int PointStDev,
                              int rad, double radAve, double radStDev, int c, int cat)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    //1D data array stores in this order: C Z Y X

    LocationSimple tmpLocation(0,0,0);
    LandmarkList newList;
    double seg;

    //this part is for user-entered rad
    if (rad!=0)
    {
        seg=rad/3;
        for (V3DLONG iz=seg; iz<P; iz+=seg)
        {
            for (V3DLONG iy=seg; iy<M; iy+=seg)
            {
                for (V3DLONG ix=seg; ix<N; ix+=seg)
                {
                    //(ix,iy,iz,c) are the coords that we are currently at
                    //we throw these coords into func pixel to get the pixel value to compare to the training values
                    //both sets of averages and st devs have to match up
                    pair<int,int> check = pixel_range(data1d,dimNum,ix,iy,iz,c,rad);
                    //we will say for now there is a cell if the test data is within 1 std of the training data
                    int TempDataAve = check.first;
                    int TempPointAve = check.second;
                    if ( (TempPointAve>=PointAve-PointStDev) && (TempPointAve<=PointAve+PointStDev))
                    {
                        if ( (TempDataAve>=MarkAve-MarkStDev) && (TempDataAve<=MarkAve+MarkStDev) )
                        {
                            tmpLocation.x = ix;
                            tmpLocation.y = iy;
                            tmpLocation.z = iz;
                            tmpLocation.category = cat;
                            stringstream catStr;
                            catStr << cat;
                            tmpLocation.comments = catStr.str();
                            newList.append(tmpLocation);
                            continue;

                        }
                    }
                }
            }
        }
    }

    //this part is for dynamically calculated rad
    else
    {
        unsigned char *outputData = 0;
        outputData = new unsigned char [N*M*P];
        for (V3DLONG tmpi=0;tmpi<N*M*P;++tmpi) outputData[tmpi] = 0; //preset to be all 0
        Image4DSimple outputImage;
        outputImage.setData((unsigned char*)outputData, N, M, P, 1, V3D_UINT8);
        Image4DProxy<Image4DSimple> maskImg(&outputImage);


        cout<<"starting count"<<endl;
        seg = radAve/4;
        double init=radAve+radStDev;
        double end=radAve-radStDev;
        if (end<1) { end=1;}
        //v3d_msg(QString("init %1").arg(init));
        for (double i=init; i>=end; i-=0.5)
        {
            for (double iz=seg; iz<=P; iz+=seg)
            {
                for (double iy=seg; iy<=M; iy+=seg)
                {
                    for (double ix=seg; ix<=N; ix+=seg)
                    {
                        //cout<<*maskImg.at(ix,iy,iz,0)<<endl;
                        if (*maskImg.at(ix, iy, iz, 0) == 255) {continue;}
                        //v3d_msg(QString("%1 %2 %3").arg(ix).arg(iy).arg(iz));
                        //cout<<ix<<" "<<iy<<" "<<iz<<" "<<endl;
                        //(ix,iy,iz,c) are the coords that we are currently at
                        //checking radius i
                        //we throw these coords into func pixel to get the pixel value to compare to the training values
                        //both sets of averages and st devs have to match up
                        pair<int,int> check = pixel_range(data1d,dimNum,ix,iy,iz,c,i);
                        int TempDataAve = check.first;
                        int TempPointAve = check.second;
//v3d_msg(QString("%1 %2 %3").arg(i).arg(TempDataAve).arg(TempPointAve));
                        if ( (TempPointAve>=PointAve-PointStDev) && (TempPointAve<=PointAve+PointStDev) && (TempDataAve>=MarkAve-MarkStDev) && (TempDataAve<=MarkAve+MarkStDev))
                        {
                            //cout<<"found a marker "<<TempDataAve<<" with rad "<<i<<" at coords "<<ix<<" "<<iy<<" "<<iz<<endl;
                            tmpLocation.x = ix;
                            tmpLocation.y = iy;
                            tmpLocation.z = iz;
                            tmpLocation.category = cat;
                            stringstream catStr;
                            catStr << cat;
                            tmpLocation.comments = catStr.str();
                            newList.append(tmpLocation);

                            apply_mask(data1d,dimNum,ix,iy,iz,c,PointAve,0,maskImg);

                            continue;
                        }
                        else continue;
                    }
                }
            }
        }

    }

    //note this function does not remember where the test data actually was, so it should find them again
    return newList;
}


//detects markers too close together, deletes marker with pixel value farther from PointAve
template <class T> LandmarkList duplicates(T* data1d, LandmarkList fullList,
                                           V3DLONG *dimNum, int PointAve, int rad, int c)
{
    int marknum = fullList.count();
    LandmarkList smallList = fullList;
    int x1,y1,z1,x2,y2,z2,data1,data2;
    double t, dist;
    LocationSimple point1(0,0,0), point2(0,0,0);
    LocationSimple zero(0,0,0);
    LocationSimple& zer = zero;
    for (int i=0; i<marknum; i++)
    {
        for (int j=i+1; j<marknum; j++)
        {
            point1 = smallList.at(i);
            point1.getCoord(x1,y1,z1);
            int pix1 = pixelVal(data1d,dimNum,x1,y1,z1,c);
            point2 = smallList.at(j);
            point2.getCoord(x2,y2,z2);
            int pix2 = pixelVal(data1d,dimNum,x2,y2,z2,c);

            t = (x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2);
            dist = sqrt(t);
            if (dist<=rad*1.2)
            {
                data1 = abs(pix1-PointAve);
                data2 = abs(pix2-PointAve);
                /*double pix1,rad1,pix2,rad2;
                dynamic_pixel(data1d,dimNum,x1,y1,z1,c,PointAve,0,pix1,rad1);
                dynamic_pixel(data1d,dimNum,x2,y2,z2,c,PointAve,0,pix2,rad2);*/

                if (data1>=data2)
                    smallList.replace(i,zer) ;//replace point1 with 0 to avoid changing length of list and messing up indexes
                else
                    smallList.replace(j,zer) ;//replace point2
            }
        }
    }
    smallList.removeAll(zer);
    return smallList;
}

LandmarkList neuron_2_mark(const NeuronTree & p, LandmarkList & neuronMarkList)
{
    LocationSimple tmpMark(0,0,0);
    for (int i=0;i<p.listNeuron.size();i++)
    {
        tmpMark.x = p.listNeuron.at(i).x;
        tmpMark.y = p.listNeuron.at(i).y;
        tmpMark.z = p.listNeuron.at(i).z;
        tmpMark.category = p.listNeuron.at(i).type;
        neuronMarkList.append(tmpMark);
    }
    return neuronMarkList;
}

V_NeuronSWC get_v_neuron_swc(const NeuronTree *p)
{
    V_NeuronSWC cur_seg;	cur_seg.clear();
    const QList<NeuronSWC> & qlist = p->listNeuron;

    for (V3DLONG i=0;i<qlist.size();i++)
    {
        V_NeuronSWC_unit v;
        v.n		= qlist[i].n;
        v.type	= qlist[i].type;
        v.x 	= qlist[i].x;
        v.y 	= qlist[i].y;
        v.z 	= qlist[i].z;
        v.r 	= qlist[i].r;
        v.parent = qlist[i].pn;

        cur_seg.append(v);
        //qDebug("%d ", cur_seg.nnodes());
    }
    cur_seg.name = qPrintable(QString("%1").arg(1));
    cur_seg.b_linegraph=true; //donot forget to do this
    return cur_seg;
}
V_NeuronSWC_list get_neuron_segments(const NeuronTree *p)
{
    V_NeuronSWC cur_seg = get_v_neuron_swc(p);
    V_NeuronSWC_list seg_list;
    seg_list = cur_seg.decompose();
    return seg_list;
}
NeuronSWC make_neuron_swc(V_NeuronSWC_unit *p, int id, bool &start)
{
    NeuronSWC N;

    N.n     = id;
    N.type  = p->type;
    N.x     = p->x;
    N.y     = p->y;
    N.z     = p->z;
    N.r     = p->r;
    if (start==false)    N.parent = id-1;
    else                 N.parent = -1;

    return N;
}

/*NeuronTree VSWClist_2_neuron_tree(V_NeuronSWC_list *p)
{
    QList<NeuronSWC> nTree;
    for (int i=0; i<p->size(); i++)
    {
        V_NeuronSWC v = p->at(i);
        for (int j=0; j<v.row.size(); j++)
        {
            V_NeuronSWC_unit v2 = v.row.at(j);
            NeuronSWC n = make_neuron_swc(&v2);
            nTree.append(n);
        }
    }
    NeuronTree newTree;
    newTree.listNeuron = nTree;
    return newTree;
}*/
NeuronTree VSWC_2_neuron_tree(V_NeuronSWC *p, int id)
{
    QList<NeuronSWC> nTree;
    bool start;
    for (int j=0; j<p->row.size(); j++)
    {
        if (j==0)   start=true;
        else        start=false;
        V_NeuronSWC_unit v = p->row.at(j);
        NeuronSWC n = make_neuron_swc(&v,id,start);
        nTree.append(n);
        id++;
    }

    NeuronTree newTree;
    newTree.listNeuron = nTree;
    return newTree;
}

template <class T> bool compute_swc_radius(T* data1d, V3DLONG *dimNum, vector<V_NeuronSWC_unit> segment,
                                           int c, double & outputRadAve, double & outputRadStDev)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    double radAve,radStDev;

    /*if (segment.size()>2)
    {
        double radTot=0;
        double k,j,i,t,dist,plane;
        for (int unit=1; unit<segment.size()-1; unit++) //going to omit first and last unit per segment
        {
            V_NeuronSWC_unit P1,P2,P0;
            P0 = segment.at(unit);
            P1 = segment.at(unit-1);
            P2 = segment.at(unit+1);
            double norm[] = {P2.x-P1.x,P2.y-P1.y,P2.z-P1.z};
            double rad=0;
            int pVal = pixelVal(data1d,dimNum,P0.x,P0.y,P0.z,c);
            if (pVal<50) { v3d_msg("pVal low, bad neuron, skipping"); continue; }
            int pValCircTot=0, runs=0;
            double check=0;
            do
            {
                rad += 0.2;
                //defining limits
                V3DLONG xLow = P0.x-rad; if(xLow<0) xLow=0;
                V3DLONG xHigh = P0.x+rad; if(xHigh>N-1) xHigh=N-1;
                V3DLONG yLow = P0.y-rad; if(yLow<0) yLow=0;
                V3DLONG yHigh = P0.y+rad; if(yHigh>M-1) yHigh=M-1;
                V3DLONG zLow = P0.z-rad; if(zLow<0) zLow=0;
                V3DLONG zHigh = P0.z+rad; if(zHigh>P-1) zHigh=P-1;

                //scanning through the pixels
                for (k = zLow; k <= zHigh; k+=0.2)
                {
                    for (j = yLow; j <= yHigh; j+=0.2)
                    {
                        for (i = xLow; i <= xHigh; i+=0.2)
                        {
                            t = (i-P0.x)*(i-P0.x)+(j-P0.y)*(j-P0.y)+(k-P0.z)*(k-P0.z);
                            dist = sqrt(t);
                            plane = (norm[0]*i+norm[1]*j+norm[2]*k-(norm[0]*P0.x+norm[1]*P0.y+norm[2]*P0.z));
                            //v3d_msg(QString("Dist %1, plane eq %2").arg(dist).arg(plane));
                            if (dist<=rad+0.2 && dist>=rad-0.2 && plane<=0.2 && plane>=-0.2)
                            {
                                int pValCirc = pixelVal(data1d,dimNum,i,j,k,c);
                                pValCircTot += pValCirc;
                                runs++;
                            }
                        }
                    }
                }
                //v3d_msg(QString("total pVal %1 in %2 runs, rad %3").arg(pValCircTot).arg(runs).arg(rad));
                if (runs==0) {check=255;}
                else {check=pValCircTot/runs;}
            } while (check > pVal*2/3);
            radTot += rad;
        }
        radAve = radTot/(segment.size()-2);
    }*/


    if (segment.size()>2)
    {
        double rad,radTot=0;
        vector<double> radArr;
        for (int unit=1; unit<segment.size()-1; unit++) //going to omit first and last unit per segment
        {
            //cout<<endl<<"unit "<<unit<<endl;
            V_NeuronSWC_unit P1,P2,P0;
            P0 = segment.at(unit);
            P1 = segment.at(unit-1);
            P2 = segment.at(unit+1);
            double Vnum[] = {P2.x-P1.x,P2.y-P1.y,P2.z-P1.z};
            double Vnorm = sqrt(Vnum[0]*Vnum[0]+Vnum[1]*Vnum[1]+Vnum[2]*Vnum[2]);
            double V[] = {Vnum[0]/Vnorm,Vnum[1]/Vnorm,Vnum[2]/Vnorm}; //axis of rotation
            double Anum[] = {-V[1],V[0],(-V[1]*V[0]-V[0]*V[1])/V[2]};
            double Anorm = sqrt(Anum[0]*Anum[0]+Anum[1]*Anum[1]+Anum[2]*Anum[2]);
            double A[] = {Anum[0]/Anorm,Anum[1]/Anorm,Anum[2]/Anorm}; //perpendicular to V
            double B[] = {A[1]*V[2]-A[2]*V[1],A[2]*V[0]-A[0]*V[2],A[0]*V[1]-A[1]*V[0]}; //perpendicular to A and V
            rad=0;
            int pValCircTot=0, runs=0;
            int pVal = pixelVal(data1d,dimNum,P0.x,P0.y,P0.z,c);
            if (pVal<50)
            {
                rad+=0.1;
                continue;
            }
            else
            {
                double check=255, pi=3.14;
                float x,y,z;
                do
                {
                    rad += 0.2;
                    for (double theta=0; theta<2*pi; theta+=pi/8)
                    {
                        x = P0.x+rad*cos(theta)*A[0]+rad*sin(theta)*B[0];
                        if (x>N-1) x=N-1; if (x<0) x=0;
                        y = P0.y+rad*cos(theta)*A[1]+rad*sin(theta)*B[1];
                        if (y>M-1) y=M-1; if (y<0) y=0;
                        z = P0.z+rad*cos(theta)*A[2]+rad*sin(theta)*B[2];
                        if (z>P-1) z=P-1; if (z<0) z=0;
                        if (x!=x || y!=y || z!=z) continue; //checking for NaN, skips segments that return indefinite coords
                        //cout<<x<<" "<<y<<" "<<z<<endl;
                        double pValCirc = pixelVal(data1d,dimNum,x,y,z,c);
                        //cout<<pValCirc<<endl;
                        pValCircTot += pValCirc;
                        runs++;
                    }
                    if (runs==0) check=0; //should only happen if entire segment is returning indefinite coords
                    else check=pValCircTot/runs;
                    //cout<<"rad: "<<rad<<". check vs threshold: "<<check<<" vs "<<pVal*2/3<<endl;
                } while (check > pVal*2/3);
                //cout<<"check passed"<<endl;
            }
            radTot += rad;
            radArr.push_back(rad);
            //cout<<"RadTot "<<radTot<<endl;
        }
        radAve = radTot/(segment.size()-2);
        double stR,t;
        for (int i=0; i<radArr.size(); i++)
        {
            t = (radArr[i]-radAve)*(radArr[i]-radAve);
            stR += t;
        }
        outputRadStDev = sqrt(stR/radArr.size());
    }

    else //2-unit segment
    {
        //v3d_msg("2-unit neuron SWC radius calculation to be implemented, please remove from test data");
    }

    //cout<<"radAve "<<radAve<<endl;
    outputRadAve=radAve;
    return true;

}

//obsolete function
void markers_singleChannel(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();

    //cancels if no image
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    //if image, pulls the data
    Image4DSimple* p4DImage = callback.getImage(curwin); //the data of the image is in 4D (channel + 3D)

    unsigned char* data1d = p4DImage->getRawData(); //sets data into 1D array

    //defining the dimensions
    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();
    //input channel
    unsigned int c=1, rad=10;
    bool ok;
    if (sc==1)
        c=1; //if only using 1 channel
    else
        c = QInputDialog::getInteger(parent, "Channel", "Enter Channel Number", 1, 1, sc, 1,&ok);

    //storing the dimensions
    V3DLONG dimNum[4];
    dimNum[0]=N; dimNum[1]=M; dimNum[2]=P; dimNum[3]=sc;

    //pulling marker info
    int xc,yc,zc;
    LocationSimple tmpLocation(0,0,0);
    LandmarkList mlist = callback.getLandmark(curwin);
    QString imgname = callback.getImageName(curwin);
    int marknum = mlist.count();
    if (mlist.isEmpty())
    {
        v3d_msg(QString("The marker list of the current image [%1] is empty. Do nothing.").arg(imgname));
        return;
    }
    else
    {
        //radius input
        rad = QInputDialog::getInteger(parent, "Radius", "Enter radius", 1,1,P,1,&ok);
        int cat = mlist.at(0).category;

        //dynamic array to store average pixel values of all markers
        int * markAve; int * pointAve;
        markAve = new int[marknum]; pointAve = new int[marknum];
        int markSum = 0, pointSum = 0, dataAve, pointval;

        //getting pixel values from each marker
        for (int i=0; i<marknum; i++)
        {
            tmpLocation = mlist.at(i);
            tmpLocation.getCoord(xc,yc,zc);
            pair<int,int> pixAns = pixel_range(data1d,dimNum,xc,yc,zc,c,rad);
            dataAve = pixAns.first;
            pointval = pixAns.second;
            markAve[i] = dataAve;
            pointAve[i] = pointval;
            markSum += dataAve;
            pointSum += pointval;
        }

        int MarkAve = markSum/marknum; //average pixel value of all markers in this channel
        int PointAve = pointSum/marknum;
        //st dev of markAve
        int stM=0, stP=0;
        for (int i=0; i<marknum; i++)
        {
            int s = pow(markAve[i]-MarkAve,2.0);
            stM += s;

            int t = pow(pointAve[i]-PointAve,2.0);
            stP += t;
        }
        int MarkStDev = sqrt(stM/(marknum-1.0));
        int PointStDev = sqrt(stP/(marknum-1.0));

        //v3d_msg(QString("Mean value region: %1. St Dev: %2 MarkSum %3 and markNum %4. Mean value point: %5. St Dev: %6").arg(MarkAve).arg(MarkStDev).arg(markSum).arg(marknum).arg(PointAve).arg(PointStDev));

        //and here we need to add the function that will scan the rest of the image for other cells based on MarkAve and MarkStDev
        //int CellCnt = count(data1d,dimNum,curwin,MarkAve,MarkStDev,rad,c);
        //v3d_msg(QString("There are %1 cells in this channel").arg(CellCnt));

        LandmarkList newList = scan_and_count(data1d,dimNum,MarkAve,MarkStDev,PointAve,PointStDev,rad,0,0,c,cat);

        //now need to delete duplicate markers on same cell
        LandmarkList smallList = duplicates(data1d,newList,dimNum,PointAve,rad,c);
        LandmarkList& woot = smallList;
        bool draw_le_markers = callback.setLandmark(curwin,woot);

    }
    return;
}

bool export_list2file(QList<NeuronTree> & N2, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin auto_identify - neurons"<<endl;
    myfile<<"# source file(s): "<<fileOpenName<<endl;
    myfile<<"# id,type,x,y,z,r,pid"<<endl;
    QList<NeuronSWC> N1;
    for (int j=0; j<N2.count(); j++)
    {
        N1 = N2.at(j).listNeuron;
        for (V3DLONG i=0;i<N1.size();i++)
        {
            myfile << N1.at(i).n <<" " << N1.at(i).type << " "<< N1.at(i).x <<" "<<N1.at(i).y << " "<< N1.at(i).z << " "<< N1.at(i).r << " " <<N1.at(i).pn << "\n";
        }
    }
    file.close();
    v3d_msg(QString("SWC file %1 has been generated, size %2").arg(fileSaveName).arg(N1.size()));
    cout<<"swc file "<<fileSaveName.toStdString()<<" has been generated, size: "<<N1.size()<<endl;
    return true;
};

template <class T> bool apply_mask(unsigned char* data1d, V3DLONG *dimNum,
                                   int xc, int yc, int zc, int c, double PixVal, double BGVal, T & maskImg)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    int x,y,z;
    double pi=3.14,rad,dataAve;

    dynamic_pixel(data1d,dimNum,xc,yc,zc,c,PixVal,BGVal,dataAve,rad);

    //cout<<"applying mask"<<endl;
    *maskImg.at(xc,yc,zc,0) = 255;

    for (double r=0.2; r<=rad; r+=0.2)
    {
        for (double theta=0; theta<2*pi; theta+=(pi/8))
        {
            for (double phi=0; phi<pi; phi+=(pi/8))
            {
                x = xc+r*cos(theta)*sin(phi);
                if (x>N-1) x=N-1; if (x<0) x=0;
                y = yc+r*sin(theta)*sin(phi);
                if (y>M-1) y=M-1; if (y<0) y=0;
                z = zc+r*cos(phi);
                if (z>P-1) z=P-1; if (z<0) z=0;
                *maskImg.at(x,y,z,0) = 255;
            }
        }
    }
    return true;
}

bool open_testSWC(NeuronTree & openTree)
{
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Example SWC File"),
            "",
            QObject::tr("Supported file (*.swc)"));

    if (!fileOpenName.isEmpty() && fileOpenName.toUpper().endsWith(".SWC"))
    {
        openTree = readSWC_file(fileOpenName);
        return true;
    }
    else return false;
}
