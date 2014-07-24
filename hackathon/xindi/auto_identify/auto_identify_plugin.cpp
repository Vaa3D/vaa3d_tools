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

controlPanel_SWC* controlPanel_SWC::m_pLookPanel_SWC = 0;

void count_cells(V3DPluginCallback2 &callback, QWidget *parent);
void identify_neurons(V3DPluginCallback2 &callback, QWidget *parent);

template <class T> bool identify_cells(T* data1d, V3DLONG *dimNum, int c, const LandmarkList & markerlist, LandmarkList & bglist, LandmarkList & outputlist, Image4DSimple &maskImage);

LandmarkList neuron_2_mark(const NeuronTree & p, LandmarkList & neuronMarkList);
template <class T> int pixelVal(T* data1d, V3DLONG *dimNum,
                                double xc, double yc, double zc, int c);
template <class T> bool mass_center_Lists(T* data1d,
                                    V3DLONG *dimNum,
                                    LandmarkList & originalList, LandmarkList & newList,
                                    double radius, int c, double thresh);
template <class T> bool mass_center_Coords(T* data1d,
                                    V3DLONG *dimNum,
                                    double &x, double &y, double &z,
                                    double radius, int c, double thresh);
template <class T> double compute_ave_cell_val(T* data1d, V3DLONG *dimNum,
                                        double xc,double yc,double zc,int c,double rad);
template <class T> bool compute_cell_values_rad(T* data1d,
                                        V3DLONG *dimNum,
                                        double xc, double yc, double zc,
                                        int c, double threshold, double & dataAve, double & rad);
template <class T> LandmarkList scan_and_count(T* data1d,
                                      V3DLONG *dimNum,
                                      int MarkAve, int MarkStDev,
                                      int PointAve, int PointStDev,
                                      double radAve, double radStDev,
                                      int c, int cat, double thresh, LandmarkList originalList, Image4DSimple &maskImage);
template <class T> LandmarkList remove_duplicates(T* data1d, LandmarkList fullList,
                                           V3DLONG *dimNum, int PointAve, int rad, int c);
V_NeuronSWC get_v_neuron_swc(const NeuronTree *p);
V_NeuronSWC_list get_neuron_segments(const NeuronTree *p);
NeuronTree VSWC_2_neuron_tree(V_NeuronSWC *p, int id);
NeuronSWC make_neuron_swc(V_NeuronSWC_unit *p, int id, bool &start);
template <class T> bool compute_swc_radius(T* data1d, V3DLONG *dimNum, vector<V_NeuronSWC_unit> segment,
                                           int c, double & outputRadAve, double & outputRadStDev);
bool export_list2file(QList<NeuronTree> & N2, QString fileSaveName, QString fileOpenName);
template <class T> bool apply_mask(unsigned char* data1d, V3DLONG *dimNum,
                                   int xc, int yc, int zc, int c, double threshold, T & maskImg);
bool open_testSWC(QString &fileOpenName, NeuronTree & openTree);
 
QStringList AutoIdentifyPlugin::menulist() const
{
    return QStringList()
        <<tr("Cell Counting")
        <<tr("Label Neurons")
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
    if (menu_name == tr("Cell Counting"))
	{
        count_cells(callback,parent);
    }
    else if (menu_name == tr("Label Neurons"))
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
 * newly labeled SWC is now automatically added to 3D window
 *
 * [current goals/issues]
 * some coords are returning as indeterminant in the compute_radius function. Right now they are being skipped. Not sure source of bad numbers.
 *
 * [future goals]
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
    int c=0;
    //bool ok;
    //if (sc==1)
    //    c=1; //if only using 1 channel
    //else
    //    c = QInputDialog::getInteger(parent, "Channel", "Enter Channel Number", 1, 1, sc, 1,&ok);

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

        //take user inputs
        NeuronTree openTree;
        QString outfilename;
        controlPanel_SWC* p;
        if (controlPanel_SWC::m_pLookPanel_SWC)
        {
            controlPanel_SWC::m_pLookPanel_SWC->show();
            return;
        }
        else
        {
            p = new controlPanel_SWC();
            if (p)	p->show();
        }
        //controlPanel_SWC::controlPanel_SWC(openTree,c,outfilename);
        QString infileName = p->infileName;
        if (open_testSWC(infileName,openTree))
        {
            openTree.comment = "test";
            mTreeList->append(openTree);
        }
        c = p->channel;
        int structNum = mTreeList->count();

        //get examples from test data
        int check = 0;
        for (int i=0; i<structNum; i++)
        {
            mTree = mTreeList->at(i);

            if (mTree.comment != "test") //defining the testing set by comments
            {
                check++;
                continue;
            }
            //v3d_msg("test data");

            V_NeuronSWC_list seg_list = get_neuron_segments(&mTree);
            //syntax: list.at(i) is segment, segment.row is vector of units, vector.at(i) is unit, unit.type is category
            int segNum = seg_list.size();
            //v3d_msg(QString("read in test tree with %1 segments").arg(segNum));
            for (int j=0; j<segNum; j++)
            {
                //segCatArr.push_back(seg_list.at(j).row.at(0).type);
                int segCat = seg_list.at(j).row.at(0).type;
                //cout<<segCat<<endl;
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
        if (check==structNum)
        {
            v3d_msg("No exampler data found, please specify a set of example data and try again.");
            return;
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
                newTree = VSWC_2_neuron_tree(&seg_list.at(j),id); //translates segment into a NeuronTree
                id += seg_list.at(j).row.size();
                newTreeList.append(newTree);
                //v3d_msg(QString("changed segment %1 of rad %3 to type %2").arg(j).arg(cur_type).arg(radAve));
            }

        }
        //QString outfilename = curfilename+"_Labeled_SWC.swc";
        outfilename = p->outfileName;
        if (!outfilename.toUpper().endsWith(".SWC"))
        {
            outfilename.append(curfilename + "_Labeled_SWC.swc");
        }
        export_list2file(newTreeList,outfilename,curfilename);
        NeuronTree nt = readSWC_file(outfilename);
        callback.setSWC(curwin, nt);
    }
    return;
}


/*  ##################################
 * [completed tasks]
 * all main algorithms are functional, may not be optimized
 * added in use of corner voxels as negative examplers
 * improved threshold algorithm
 * significantly reduced duplicate detection of cells
 * mass center algorithm inaccuracy fixed
 * mass center should be smarter now
 * fixed apply_mask, broke mass_center...again
 *
 * [current goals/issues]
 * still several undetected cells
 * scan_and_count is slow
 * because of variable cell radius, mass_center does not work equally well across all cells
 *      optimizing to center larger cells would cause clustered cells to be treated as one
 *      keeping small radius to keep clustered cells separate prevents detection of true center of larger cells
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
        LandmarkList outputList;
        Image4DSimple maskImage;
        if (identify_cells(data1d,dimNum,c,mlist,bglist,outputList,maskImage))
        {
            callback.setLandmark(curwin,outputList);

            //v3dhandle newwin = callback.newImageWindow("mask");
            //callback.setImage(newwin,&maskImage);
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
            Image4DSimple maskImage;
            if (identify_cells(data1d,dimNum,c,*marks,*bgs, tempList,maskImage))
                catSortList.append(tempList);
//v3d_msg(QString("catSortList append category %1").arg(tempList.at(0).category));
//            marksL.clear();
        }

        callback.setLandmark(curwin,catSortList);

    }
    return;
}

template <class T> bool identify_cells(T* data1d, V3DLONG *dimNum, int c, const LandmarkList & markerlist, LandmarkList & bglist, LandmarkList & outputlist, Image4DSimple &maskImage)
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
    double thresh=0;
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
        //threshold calculation based on histogram of marker+bg voxel intensities
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
    LandmarkList CenteredList;
    double blah,checkrad=0;
    compute_cell_values_rad(data1d,dimNum,xc,yc,zc,c,thresh,blah,checkrad);
    mass_center_Lists(data1d,dimNum,MarkList,CenteredList,checkrad*1.5,c,thresh);
    MarkList = CenteredList;
    //outputlist = CenteredList; return true;

    //scan list of cell markers for ValAve, radAve

    int * ValAveArr; int * radAveArr;
    ValAveArr = new int[marks]; radAveArr = new int[marks];
    LocationSimple tempLocation(0,0,0);
    double ValAve=0,radAve=0,tmpDataAve,tmpRad;

    for (int i=0; i<marks; i++)
    {
        tempLocation = MarkList.at(i);
        tempLocation.getCoord(xc,yc,zc);
        //int Pix = pixelVal(data1d,dimNum,xc,yc,zc,c);

        if (compute_cell_values_rad(data1d,dimNum,xc,yc,zc,c,thresh,tmpDataAve,tmpRad))
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


//v3d_msg(QString("markers have been scanned. Pixval %1 and stdev %2. radVal %3 and stdev %4. segVal %5 and stdev %6").arg(PixVal).arg(PixStDev).arg(radAve).arg(radStDev).arg(ValAve).arg(ValStDev));

//v3d_msg(QString("category %1").arg(cat));

    //scans image and generates new set of markers based on testing data
    LandmarkList scannedList = scan_and_count(data1d,dimNum,ValAve,ValStDev,PixVal,PixStDev,radAve,radStDev,c,cat,thresh,MarkList,maskImage);
    //cout<<"Cell count "<<newList.count()<<endl;
//v3d_msg("newList made");
    //recenters list via mean shift
//v3d_msg("recentering");
    LandmarkList RecenterList;
    mass_center_Lists(data1d,dimNum,scannedList,RecenterList,radAve+radStDev,c,thresh);
    for (int i=0; i<RecenterList.size();i++)
    {
        LocationSimple tmp(0,0,0);
        tmp = RecenterList.at(i);
        tmp.category = cat;
        stringstream catStr;
        catStr << cat;
        tmp.comments = catStr.str();
    }
//v3d_msg("ckpt 2");
    outputlist = remove_duplicates(data1d,RecenterList,dimNum,PixVal,radAve,c);
//    outputlist = RecenterList;
//v3d_msg("duplicates deleted");

    cout<<"Cell count "<<outputlist.count()<<endl<<endl;
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
    //xc+=0.5; yc+=0.5; zc+=0.5; //insures proper rounding
    if (xc<=0) xc=1; if (xc>N) xc=N;
    if (yc<=0) yc=1; if (yc>M) yc=M;
    if (zc<=0) zc=1; if (zc>P) zc=P;
    int pixelVal = data1d[ shiftC + (V3DLONG)(zc-1)*M*N + (V3DLONG)(yc-1)*N + (V3DLONG)(xc-1) ];
    if (pixelVal<0) pixelVal=0;
    if (pixelVal>255) pixelVal=255;
    return pixelVal;
}


//returns recentered LandmarkList
template <class T> bool mass_center_Lists(T* data1d,
                                    V3DLONG *dimNum,
                                    LandmarkList & originalList, LandmarkList & newList,
                                    double radius, int c, double thresh)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];

    double pVal,newX,newY,newZ,norm;
    double x,y,z,pi=3.14;
    int xc,yc,zc;
    LocationSimple tmp(0,0,0);
    for (int i=0; i<originalList.size(); i++)
    {
        double rad = radius;
        tmp = originalList.at(i);
        tmp.getCoord(xc,yc,zc);
        int xo=xc,yo=yc,zo=zc;
        int check=0,runs=0;
        bool converge=false;
        //cout<<endl<<"looping for marker "<<xo<<" "<<yo<<" "<<zo<<endl;
        do
        {
            //cout<<"loop "<<j<<" using coords "<<xc<<" "<<yc<<" "<<zc<<endl;
            pVal = pixelVal(data1d,dimNum,xc,yc,zc,c);
            newX=xc*pVal, newY=yc*pVal, newZ=zc*pVal, norm=pVal;
            for (double r=rad/5; r<=rad; r+=rad/5)
            {
                for (double theta=0; theta<2*pi; theta+=(pi/8))
                {
                    for (double phi=0; phi<pi; phi+=(pi/8))
                    {
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
                        //cout<<x<<" "<<y<<" "<<z<<" "<<endl<<endl;
                    }
                }
            }
            newX /= norm;
            newY /= norm;
            newZ /= norm;

            if (newX==xc && newY==yc && newZ==zc) {converge=true;}

            xc=newX; yc=newY; zc=newZ;
            //cout<<xc<<" "<<yc<<" "<<zc<<endl;

            //if marker has been placed in background, restart loop with some changes
            pVal = pixelVal(data1d,dimNum,newX,newY,newZ,c);
            if (pVal<=thresh)
            {
                //cout<<"hit check number "<<check<<" in marker "<<i<<endl;
                if (check==0) //first check just reset loop with smaller radius
                {
                    xc=xo; yc=yo; zc=zo;
                    rad/=2;
                    converge=false;
                }
                else if (check==1) //second check shift the original coords
                {
                    double shiftX,shiftY,shiftZ;
                    shiftX = (newX-xo)/2;
                    shiftY = (newY-yo)/2;
                    shiftZ = (newZ-zo)/2;
                    xc=xo-shiftX; yc=yo-shiftY; zc=zo-shiftZ;
                    if (xc<0) xc=0; if (xc>N-1) xc=N-1;
                    if (yc<0) yc=0; if (yc>M-1) yc=M-1;
                    if (zc<0) zc=0; if (zc>P-1) zc=P-1;
                    converge=false;
                }
                else if (check>1) //if still hasn't fixed idk
                {
                    newX=xo; newY=yo; newZ=zo;
                    converge=true;
                }
                runs=0;
                check++;
            }
            runs++;
        } while (converge==false && runs<10); //if not converged in 10 runs, likely stuck in rounding loop
        if (newX<0) newX=0;
        if (newY<0) newY=0;
        if (newZ<0) newZ=0;
        if (newX>N-1) newX=N-1;
        if (newY>M-1) newY=M-1;
        if (newZ>P-1) newZ=P-1;

        LocationSimple newMark(newX,newY,newZ);
        newList.append(newMark);
    }
    return true;
}

//returns recentered coords x,y,z
template <class T> bool mass_center_Coords(T* data1d,
                                    V3DLONG *dimNum,
                                    double &x, double &y, double &z,
                                    double radius, int c, double thresh)
{
    LandmarkList tmpList,recenteredList;
    LocationSimple tmp(x,y,z),recentered(0,0,0);
    tmpList.append(tmp);
    mass_center_Lists(data1d,dimNum,tmpList,recenteredList,radius,c,thresh);
    recentered = recenteredList.at(0);
    x = recentered.x;
    y = recentered.y;
    z = recentered.z;
    return true;
}

//returns average pixel value and radius of cell around a marker
template <class T> bool compute_cell_values_rad(T* data1d,
                                        V3DLONG *dimNum,
                                        double xc, double yc, double zc,
                                        int c, double threshold, double & outputdataAve, double & outputrad)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    outputrad=0;

    do
    {
        outputrad++;
        double x,y,z,datatotal=0,pi=3.14;;
        int runs=0;
        for (double theta=0; theta<2*pi; theta+=(pi/8))
        {
            for (double phi=0; phi<pi; phi+=(pi/8))
            {
                //cout<<"pixel iteration "<<runs<<endl;
                //cout<<r<<" "<<theta<<" "<<phi<<endl<<endl;
                x = xc+outputrad*cos(theta)*sin(phi);
                if (x>N) x=N; if (x<0) x=0;
                y = yc+outputrad*sin(theta)*sin(phi);
                if (y>M) y=M; if (y<0) y=0;
                z = zc+outputrad*cos(phi);
                if (z>P) z=P; if (z<0) z=0;
                double dataval = pixelVal(data1d,dimNum,x,y,z,c);
                datatotal += dataval;
                //cout<<dataval<<" "<<datatotal<<endl;
                runs++;
                //cout<<x<<" "<<y<<" "<<z<<" "<<endl;
            }
        }

        runs++;
        outputdataAve = datatotal/runs;
    } while ( outputdataAve > threshold*2/3 );

    return true;
}


//returns average pixel value in sphere of radius rad on channel c around a given marker
template <class T> double compute_ave_cell_val(T* data1d, V3DLONG *dimNum,
                                               double xc, double yc, double zc, int c, double rad)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];

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
    double dataAve;
    if (runs==0) dataAve=0;
    else dataAve = datatotal/runs;

    return dataAve;
}


//uses test data to scan and mark other cells
template <class T> LandmarkList scan_and_count(T* data1d,
                              V3DLONG *dimNum,
                              int cellAve, int cellStDev,
                              int PointAve, int PointStDev,
                              double radAve, double radStDev,
                              int c, int cat, double thresh, LandmarkList originalList, Image4DSimple &maskImage)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];

    //set up mask
    unsigned char *maskData = 0;
    maskData = new unsigned char [N*M*P];
    for (V3DLONG tmpi=0;tmpi<N*M*P;++tmpi) maskData[tmpi] = 0; //preset to be all 0
    //Image4DSimple maskImage;
    maskImage.setData((unsigned char*)maskData, N, M, P, 1, V3D_UINT8);
    Image4DProxy<Image4DSimple> maskImg(&maskImage);

    LandmarkList newList;
    cout<<"starting count"<<endl;

    //set original markers into new list and mask them
    if (!originalList.empty())
    {
        for (int i=0; i<originalList.size(); i++)
        {
            LocationSimple tmp(0,0,0);
            int xc,yc,zc;
            tmp = originalList.at(i);
            tmp.getCoord(xc,yc,zc);
            //cout<<"coords "<<xc<<" "<<yc<<" "<<zc<<endl;
            apply_mask(data1d,dimNum,xc,yc,zc,c,thresh,maskImg);
            //cout<<"mask applied for marker "<<i<<endl<<endl;
        }
        newList = originalList;
    }
    //cout<<"original list appended, listsize currently "<<newList.size()<<endl;

    LocationSimple tmpLocation(0,0,0);
    double seg;
    seg = radAve/4;
    double init=radAve+radStDev;
    double end=radAve-radStDev;
    if (end<1) { end=1;}
    //v3d_msg(QString("init %1").arg(init));
    for (double i=init; i>=end; i-=0.5)
    {
        for (double iz=seg; iz<P; iz+=seg)
        {
            for (double iy=seg; iy<M; iy+=seg)
            {
                for (double ix=seg; ix<N; ix+=seg)
                {
                    //cout<<"coords "<<x1<<" "<<y1<<" "<<z1<<" have mask "<<*maskImg.at(x1,y1,z1,0)<<endl;
                    if (*maskImg.at(ix,iy,iz,0) == 0)
                    {
                        double TempPointVal = pixelVal(data1d,dimNum,ix,iy,iz,c);
                        if ((TempPointVal>=PointAve-PointStDev) && (TempPointVal<=PointAve+PointStDev))
                        {
                            double x,y,z;
                            x=ix; y=iy; z=iz; //mass_center_Coords will rewrite x,y,z so need to keep ix,iy,iz untouched
                            mass_center_Coords(data1d,dimNum,x,y,z,radAve,c,thresh);
                            if (*maskImg.at(x,y,z,0)!=0) continue;
                            double TempDataAve = compute_ave_cell_val(data1d,dimNum,x,y,z,c,i);
                            if ( (TempDataAve>=cellAve-cellStDev) && (TempDataAve<=cellAve+cellStDev))
                            {
                                //cout<<"found a marker "<<TempDataAve<<" with rad "<<i<<" at coords "<<ix<<" "<<iy<<" "<<iz<<endl;

                                tmpLocation.x = x;
                                tmpLocation.y = y;
                                tmpLocation.z = z;
                                tmpLocation.category = cat;
                                stringstream catStr;
                                catStr << cat;
                                tmpLocation.comments = catStr.str();
                                newList.append(tmpLocation);

                                apply_mask(data1d,dimNum,x,y,z,c,thresh,maskImg);
                            }
                        }
                    }
                }
            }
        }
    }
    return newList;
}


//detects markers too close together, deletes marker with pixel value farther from PointAve
template <class T> LandmarkList remove_duplicates(T* data1d, LandmarkList fullList,
                                           V3DLONG *dimNum, int PointAve, int rad, int c)
{
    int marknum = fullList.count();
    LandmarkList smallList = fullList;
    int x1,y1,z1,x2,y2,z2;
    double data1,data2,t,dist;
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
            if (dist<=rad)
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

//compute radius of neuron SWC segment
template <class T> bool compute_swc_radius(T* data1d, V3DLONG *dimNum, vector<V_NeuronSWC_unit> segment,
                                           int c, double & outputRadAve, double & outputRadStDev)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    double rad,radAve,radTot=0;
    vector<double> radArr;
    int init,end;
    if (segment.size()>2)       {init=1;end=segment.size()-1;} //going to omit first and last unit per segment
    else if (segment.size()==2) {init=0;end=1;}
    else {v3d_msg("Neuron segment with only 1 node, shouldn't be there"); return false;}
    for (int unit=init; unit<end; unit++)
    {
        //cout<<endl<<"unit "<<unit<<endl;
        V_NeuronSWC_unit P1,P2,P0;

        if (segment.size()>2)
        {
            P0 = segment.at(unit);
            P1 = segment.at(unit-1);
            P2 = segment.at(unit+1);
        }
        if (segment.size()==2)
        {
            P1 = segment.at(0);
            P2 = segment.at(1);
            P0.x = (P2.x+P1.x)/2;
            P0.y = (P2.y+P1.y)/2;
            P0.z = (P2.z+P1.z)/2;
        }

        double Vnum[] = {P2.x-P1.x,P2.y-P1.y,P2.z-P1.z};
        double Vnorm  = sqrt(Vnum[0]*Vnum[0]+Vnum[1]*Vnum[1]+Vnum[2]*Vnum[2]);
        double V[]    = {Vnum[0]/Vnorm,Vnum[1]/Vnorm,Vnum[2]/Vnorm}; //axis of rotation
        double Anum[] = {-V[1],V[0],(-V[1]*V[0]-V[0]*V[1])/V[2]};
        double Anorm  = sqrt(Anum[0]*Anum[0]+Anum[1]*Anum[1]+Anum[2]*Anum[2]);
        double A[]    = {Anum[0]/Anorm,Anum[1]/Anorm,Anum[2]/Anorm}; //perpendicular to V
        double B[]    = {A[1]*V[2]-A[2]*V[1],A[2]*V[0]-A[0]*V[2],A[0]*V[1]-A[1]*V[0]}; //perpendicular to A and V
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
                    if (x!=x || y!=y || z!=z) continue; //skips segments that return indefinite coords
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

    //cout<<"radAve "<<radAve<<endl;
    outputRadAve=radAve;
    return true;

}

//write and save QList NeuronTree into file
bool export_list2file(QList<NeuronTree> & N2, QString fileSaveName, QString fileOpenName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);
    myfile<<"# generated by Vaa3D Plugin auto_identify - label neurons"<<endl;
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
}

template <class T> bool apply_mask(unsigned char* data1d, V3DLONG *dimNum,
                                   int xc, int yc, int zc, int c, double threshold, T & maskImg)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    int x,y,z;
    double pi=3.14,rad,dataAve;

    compute_cell_values_rad(data1d,dimNum,xc,yc,zc,c,threshold,dataAve,rad);
    //cout<<"rad "<<rad<<endl;

    //cout<<"applying mask"<<endl;
    *maskImg.at(xc,yc,zc,0) = 200;
    //cout<<"ck 1 ";
    double angle = pi/2;

    for (double r=0.5; r<=rad; r+=0.5)
    {
        for (double theta=0; theta<2*pi; theta+=angle)
        {
            for (double phi=0; phi<pi; phi+=angle)
            {
                x = xc+r*cos(theta)*sin(phi);
                if (x>N-1) x=N-1; if (x<0) x=0;

                y = yc+r*sin(theta)*sin(phi);
                if (y>M-1) y=M-1; if (y<0) y=0;
                z = zc+r*cos(phi);
                if (z>P-1) z=P-1; if (z<0) z=0;
                *maskImg.at(x,y,z,0) = 200;
            }
        }
        if (angle>=pi/16 && r>=1) angle/=2;
    }
    //cout<<"2"<<endl;
    return true;
}

bool open_testSWC(QString &fileOpenName, NeuronTree & openTree)
{
//    QString fileOpenName;
//    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Example SWC File"),
//            "",
//            QObject::tr("Supported file (*.swc)"));

    if (!fileOpenName.isEmpty() && fileOpenName.toUpper().endsWith(".SWC"))
    {
        openTree = readSWC_file(fileOpenName);
        return true;
    }
    else
    {
        v3d_msg("You did not choose a valid file type, or the file you chose is empty. Will attempt to find exampler set in window.");
        return false;
    }
}


//below edited from movieConverter plugin
QString getAppPath()
{
    QDir testPluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
        testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    // In a Mac app bundle, plugins directory could be either
    //  a - below the actual executable i.e. v3d.app/Contents/MacOS/plugins/
    //  b - parallel to v3d.app i.e. foo/v3d.app and foo/plugins/
    if (testPluginsDir.dirName() == "MacOS") {
        QDir testUpperPluginsDir = testPluginsDir;
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
        if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
        testPluginsDir.cdUp();
    }
#endif

    return testPluginsDir.absolutePath();
}

controlPanel_SWC::controlPanel_SWC()
{
    QString exepath = getAppPath();

    m_pLookPanel_SWC = this;

    m_pLineEdit_testfilepath = new QLineEdit();
    m_pLineEdit_outputfilepath = new QLineEdit(exepath);
    m_pLineEdit_channelno = new QLineEdit(QObject::tr("1"));
    QPushButton *pPushButton_start = new QPushButton(QObject::tr("start labeling"));
    QPushButton *pPushButton_close = new QPushButton(QObject::tr("close"));
    QPushButton *pPushButton_openFileDlg_input = new QPushButton(QObject::tr("Browse"));
    QPushButton *pPushButton_openFileDlg_output = new QPushButton(QObject::tr("Browse"));

    QGroupBox *input_panel = new QGroupBox("Input:");
    input_panel->setStyle(new QWindowsStyle());
    QGridLayout *inputLayout = new QGridLayout();
    input_panel->setStyle(new QWindowsStyle());
    inputLayout->addWidget(new QLabel(QObject::tr("Labeled Example SWC:")),1,1);
    inputLayout->addWidget(m_pLineEdit_testfilepath,2,1,1,2);
    inputLayout->addWidget(pPushButton_openFileDlg_input,2,3,1,1);
    input_panel->setLayout(inputLayout);

    QGroupBox *output_panel = new QGroupBox("Output:");
    output_panel->setStyle(new QWindowsStyle());
    QGridLayout *outputLayout = new QGridLayout();
    outputLayout->addWidget(new QLabel(QObject::tr("Choose directory to save labeled SWC:")),1,1);
    outputLayout->addWidget(m_pLineEdit_outputfilepath,2,1,1,2);
    outputLayout->addWidget(pPushButton_openFileDlg_output,2,3,1,1);
    output_panel->setLayout(outputLayout);


    QGroupBox *channel_panel = new QGroupBox("Channel Number:");
    channel_panel->setStyle(new QWindowsStyle());
    QGridLayout *channelLayout = new QGridLayout();
    channelLayout->addWidget(new QLabel(QObject::tr("Channel:")),4,1);
    channelLayout->addWidget(m_pLineEdit_channelno,4,2);
    channel_panel->setLayout(channelLayout);

    QWidget* container = new QWidget();
    QGridLayout* bottomBar = new QGridLayout();
    bottomBar->addWidget(pPushButton_start,1,1);
    bottomBar->addWidget(pPushButton_close,1,2);
    container->setLayout(bottomBar);

    QGridLayout *pGridLayout = new QGridLayout();
    pGridLayout->addWidget(input_panel);
    pGridLayout->addWidget(output_panel);
    pGridLayout->addWidget(channel_panel);
    pGridLayout->addWidget(container);

    setLayout(pGridLayout);
    setWindowTitle(QString("Label currently displayed SWC using example file"));

    connect(pPushButton_start, SIGNAL(clicked()), this, SLOT(_slot_start()));
    connect(pPushButton_close, SIGNAL(clicked()), this, SLOT(_slot_close()));
    connect(pPushButton_openFileDlg_input, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_input()));
    connect(pPushButton_openFileDlg_output, SIGNAL(clicked()), this, SLOT(_slots_openFileDlg_output()));

}

void controlPanel_SWC::_slot_close()
{
    if (m_pLookPanel_SWC)
    {
        delete m_pLookPanel_SWC;
        m_pLookPanel_SWC=0;
    }
}
void controlPanel_SWC::_slot_start()
{
          infileName = m_pLineEdit_testfilepath->text();
          if (!infileName.toUpper().endsWith(".SWC"))
          {
              v3d_msg("You did not choose a valid file type, or the example file you chose is empty. Will attempt to find exampler set in window.");
              return;
          }

          outfileName= m_pLineEdit_outputfilepath->text();
          if (!QFile(outfileName).exists())
          {
             v3d_msg("Output file path does not exist, ");
             return;
          }

          QString channelno = m_pLineEdit_channelno->text();
          int c = channelno.toInt();
          channel = c;
}
void controlPanel_SWC::_slots_openFileDlg_input()
{
    QFileDialog d(this);
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open Example SWC File"),
            "",
            QObject::tr("Supported file (*.swc)"));;
    if(!fileOpenName.isEmpty())
    {
        m_pLineEdit_testfilepath->setText(fileOpenName);
    }
}
void controlPanel_SWC::_slots_openFileDlg_output()
{
    QFileDialog d(this);
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Choose output file dir"));;
    if(!fileOpenName.isEmpty())
    {
        m_pLineEdit_outputfilepath->setText(fileOpenName);
    }
}
