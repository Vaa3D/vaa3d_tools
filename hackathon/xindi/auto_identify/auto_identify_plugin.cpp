/* auto_identify_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2012-01-01 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "auto_identify_plugin.h"
#include "string"
#include "sstream"

using namespace std;
Q_EXPORT_PLUGIN2(auto_identify, AutoIdentifyPlugin);

void markers_singleChannel(V3DPluginCallback2 &callback, QWidget *parent);
void mark_or_curve_singleChannel(V3DPluginCallback2 &callback, QWidget *parent);
template <class T> LandmarkList main_func(T* data1d, V3DLONG *dimNum, int c, LandmarkList & mlist, LandmarkList & bglist);
LandmarkList neuron_2_mark(const NeuronTree & p, LandmarkList & neuronMarkList);
template <class T> int pixelVal(T* data1d, V3DLONG *dimNum,
                                int xc, int yc, int zc, int c);
template <class T> LocationSimple mass_center(T* data1d,
                                              V3DLONG *dimNum,
                                              int xc, int yc, int zc, int rad, int c);
template <class T> pair<int,int> pixel(T* data1d,
                                        V3DLONG *dimNum,
                                        int xc,int yc,int zc,int c,int rad);
template <class T> pair<int,int> dynamic_pixel(T* data1d,
                                        V3DLONG *dimNum,
                                        int xc, int yc, int zc,
                                        int c, int PixVal, int BGVal);
template <class T> LandmarkList count(T* data1d,
                                      V3DLONG *dimNum,
                                      int MarkAve, int MarkStDev,
                                      int PointAve, int PointStDev,
                                      int rad, int radAve, int radStDev, int c, int cat);
template <class T> LandmarkList duplicates(T* data1d, LandmarkList fullList,
                                           V3DLONG *dimNum, int PointAve, int rad, int c);


 
QStringList AutoIdentifyPlugin::menulist() const
{
	return QStringList() 
        <<tr("Single Channel Cell Counting")
        <<tr("Better Cell Counting")
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
        mark_or_curve_singleChannel(callback,parent);
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


//this algorithm is much worse than mark_or_curve
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
            pair<int,int> pixAns = pixel(data1d,dimNum,xc,yc,zc,c,rad);
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

        LandmarkList newList = count(data1d,dimNum,MarkAve,MarkStDev,PointAve,PointStDev,rad,0,0,c,cat);

        //now need to delete duplicate markers on same cell
        LandmarkList smallList = duplicates(data1d,newList,dimNum,PointAve,rad,c);
        LandmarkList& woot = smallList;
        bool draw_le_markers = callback.setLandmark(curwin,woot);

    }
    return;
}


//########
//  current issues with the algorithm
//  -Radius calculation not robust enough to deal with thin branches, can cause program to crash
//  -have not yet implimented ability to translate newly generated markers into paths if desired
//  -mass center tends to skew markers towards (0,0,0)
//########
void mark_or_curve_singleChannel(V3DPluginCallback2 &callback, QWidget *parent)
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
    QList<NeuronTree> * mTreeList;
    mTreeList = callback.getHandleNeuronTrees_3DGlobalViewer(curwin);
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
        LandmarkList bglist; //sending in empty bglist to trigger binary sort
        LandmarkList smallList = main_func(data1d,dimNum,c,mlist,bglist);
        LandmarkList& woot2 = smallList;
        bool draw_le_markers2 = callback.setLandmark(curwin,woot2);
    }
    else if (input_type==1) //type
    {
        //int catNum = QInputDialog::getInt(0,"Number of Categories","Enter number of categories (background category included), if unsure enter 0",0,0,100,1,&ok);
        //Can't think of reason having user inputed cat number would be better than auto counting

        int * catList;
        catList = new int[mlist.count()];
        if (mlist.count()==0) {v3d_msg("There are no neuron traces in the current image"); return;}
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
            LandmarkList tempList = main_func(data1d,dimNum,c,*marks,*bgs);
            catSortList.append(tempList);
//v3d_msg(QString("catSortList append category %1").arg(tempList.at(0).category));
//            marksL.clear();
        }

        LandmarkList& woot3 = catSortList;
        bool draw_le_markers3 = callback.setLandmark(curwin,woot3);

    }
    return;
}

template <class T> LandmarkList main_func(T* data1d, V3DLONG *dimNum, int c, LandmarkList & markerlist, LandmarkList & bglist)
{
    LandmarkList mlist, MarkList, BGList;
    LocationSimple tmpLocation(0,0,0);
    int xc,yc,zc, marks;
    double PixVal,BGVal;
    if (bglist.isEmpty()) //binary sorting
    {
        //sort markers by background/foreground
        mlist = markerlist;
//v3d_msg("presort ckpt");

        int pix,num;
        int marknum = mlist.count();
        int * PixValArr;
        PixValArr = new int[marknum];
//v3d_msg("sort ckpt 1");
        for (int i=0; i<marknum; i++)
        {
            tmpLocation = mlist.at(i);
            tmpLocation.getCoord(xc,yc,zc);
            pix = pixelVal(data1d,dimNum,xc,yc,zc,c);
            //      v3d_msg(QString("pix value %1 %2").arg(pix).arg(pix1));
            PixValArr[i] = pix;
        }
        int max=0,min=255;
        for (int i=0; i<marknum; i++)
        {
            num=PixValArr[i];
            if (num>max) { max=num; }
            if (num<min) { min=num; }
        }
//v3d_msg(QString("sort ckpt 2, min %1 max %2").arg(min).arg(max));
        int thresh = (max+min)/2;
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
    //return MarkList;

    //scan list of cell markers for ValAve, radAve

    int * ValAveArr; int * radAveArr;
    ValAveArr = new int[marks]; radAveArr = new int[marks];
    LocationSimple tempLocation(0,0,0);
    double ValAve=0,radAve=0;
    for (int i=0; i<marks; i++)
    {
        tempLocation = MarkList.at(i);
        tempLocation.getCoord(xc,yc,zc);
        int Pix = pixelVal(data1d,dimNum,xc,yc,zc,c);

        pair<int,int> dynAns = dynamic_pixel(data1d,dimNum,xc,yc,zc,c,Pix,BGVal);
        ValAveArr[i] = dynAns.first;
        radAveArr[i] = dynAns.second;
        ValAve += ValAveArr[i];
        radAve += radAveArr[i];

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


v3d_msg(QString("markers have been scanned. Pixval %1 and stdev %2. radVal %3 and stdev %4."
                      "segVal %5 and stdev %6").arg(PixVal).arg(PixStDev).arg(radAve).arg(radStDev).arg(ValAve).arg(ValStDev));
//v3d_msg(QString("category %1").arg(cat));
    //scans image and generates new set of markers based on testing data
    LandmarkList newList = count(data1d,dimNum,ValAve,2*ValStDev,PixVal,PixStDev,0,radAve,5+radStDev,c,cat);

    //recenters list via mean shift
v3d_msg("recentering");
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
    LandmarkList smallList = duplicates(data1d,newList,dimNum,PixVal,radAve,c);
//v3d_msg("duplicates deleted");

    return smallList;
}




//returns pixel value of marker
template <class T> int pixelVal(T* data1d, V3DLONG *dimNum,
                                int xc, int yc, int zc, int c)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    V3DLONG shiftC = (c-1)*P*M*N;
    int pixelVal = data1d[ shiftC + (V3DLONG)zc*M*N + (V3DLONG)yc*N + (V3DLONG)xc ];
    return pixelVal;
}


//returns new marker that has been recentered
template <class T> LocationSimple mass_center(T* data1d,
                                              V3DLONG *dimNum,
                                              int xc, int yc, int zc, int rad, int c)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];

    //int min=255,newX=0,newY=0,newZ=0;
    //int xweight=0,yweight=0,zweight=0,kernel=0,ktot=0;
    double pVal;
    int kern;
    rad=5;

    //defining limits
    V3DLONG xLow = xc-rad; if(xLow<0) xLow=0;
    V3DLONG xHigh = xc+rad; if(xHigh>N-1) xHigh=N-1;
    V3DLONG yLow = yc-rad; if(yLow<0) yLow=0;
    V3DLONG yHigh = yc+rad; if(yHigh>M-1) yHigh=M-1;
    V3DLONG zLow = zc-rad; if(zLow<0) zLow=0;
    V3DLONG zHigh = zc+rad; if(zHigh>P-1) zHigh=P-1;

    //scanning through the pixels
    double newX=0, newY=0, newZ=0, norm=0;
    V3DLONG k,j,i;
    for (k = zLow; k <= zHigh; k++)
    {
         for (j = yLow; j <= yHigh; j++)
         {
             for (i = xLow; i <= xHigh; i++)
             {
                 double t = (i-xc)*(i-xc)+(j-yc)*(j-yc)+(k-zc)+(k-zc);
                 double dist = sqrt(t);
                 if (dist<rad)
                 {
                     pVal = pixelVal(data1d,dimNum,i,j,k,c);
                     if ((255-pVal)<100) kern=1;
                     else kern=1;
                     newX += pVal*i*kern;
                     newY += pVal*j*kern;
                     newZ += pVal*k*kern;
                     norm += pVal*kern;
                 }
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
template <class T> pair<int,int> dynamic_pixel(T* data1d,
                                        V3DLONG *dimNum,
                                        int xc, int yc, int zc,
                                        int c, int PixVal, int BGVal)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    V3DLONG shiftC = (c-1)*P*M*N;

    int rad=1,dataAve;
    do
    {
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
        int datatotal=0,runs=0;
        for (k = zLow; k < zHigh; k++)
        {
            V3DLONG shiftZ = k*M*N;
            for (j = yLow; j < yHigh; j++)
            {
                V3DLONG shiftY = j*N;
                for (i = xLow; i < xHigh; i++)
                {
                    int dataval = data1d[ shiftC + shiftZ + shiftY + i ];
                    datatotal += dataval;
                    runs++;
                }
            }
        }
        dataAve = datatotal/runs;
        rad++;
    } while ( dataAve > (PixVal+4*BGVal)/5 );

    return make_pair(dataAve,rad);
}


//returns average pixel value in box of radius rad on channel c around a given marker as well as average cell marker intensity
template <class T> pair<int,int> pixel(T* data1d,
                              V3DLONG *dimNum,
                              int xc,int yc,int zc,int c,int rad)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];

    V3DLONG shiftC = (c-1)*P*M*N;

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
    int datatotal=0,runs=0;
    for (k = zLow; k < zHigh; k++)
    {
         V3DLONG shiftZ = k*M*N;
         for (j = yLow; j < yHigh; j++)
         {
             V3DLONG shiftY = j*N;
             for (i = xLow; i < xHigh; i++)
             {
                 int dataval = data1d[ shiftC + shiftZ + shiftY + i ];
                 datatotal += dataval;
                 runs++;
             }
         }
    }
    int dataAve = datatotal/runs;
    //data of point
    int pointval = data1d[ shiftC + zc*M*N + yc*N + xc ];

    return make_pair(dataAve,pointval);
}


//uses test data to scan and mark other cells
template <class T> LandmarkList count(T* data1d,
                              V3DLONG *dimNum,
                              int MarkAve, int MarkStDev,
                              int PointAve, int PointStDev,
                              int rad, int radAve, int radStDev, int c, int cat)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    //1D data array stores in this order: C Z Y X

    LocationSimple tmpLocation(0,0,0);
    LandmarkList newList;
    int seg;

    //this part is for user-entered rad
    if (rad!=0)
    {
        seg=rad/2;
        for (V3DLONG iz=seg; iz<P; iz+=seg)
        {
            for (V3DLONG iy=seg; iy<M; iy+=seg)
            {
                for (V3DLONG ix=seg; ix<N; ix+=seg)
                {
                    //(ix,iy,iz,c) are the coords that we are currently at
                    //we throw these coords into func pixel to get the pixel value to compare to the training values
                    //both sets of averages and st devs have to match up
                    pair<int,int> check = pixel(data1d,dimNum,ix,iy,iz,c,rad);
                    //we will say for now there is a cell if the test data is within 1 std of the training data
                    int TempDataAve = check.first;
                    int TempPointAve = check.second;
                    if ( (TempPointAve>=PointAve-2*PointStDev) && (TempPointAve<=PointAve+2*PointStDev))
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
        seg = radAve/2;
        int init;
        if ((radAve-radStDev) < 0) { init=0;}
        else { init=radAve-radStDev; }
        for (int i=init; i<radAve+radStDev; i++)
        {
            for (V3DLONG iz=seg; iz<P; iz+=seg)
            {
                for (V3DLONG iy=seg; iy<M; iy+=seg)
                {
                    for (V3DLONG ix=seg; ix<N; ix+=seg)
                    {
                        //(ix,iy,iz,c) are the coords that we are currently at
                        //checking radius i
                        //we throw these coords into func pixel to get the pixel value to compare to the training values
                        //both sets of averages and st devs have to match up
                        pair<int,int> check = pixel(data1d,dimNum,ix,iy,iz,c,i);
                        //we will say for now there is a cell if the test data is within 2 std of the training data
                        int TempDataAve = check.first;
                        int TempPointAve = check.second;
                        if ( (TempPointAve>=PointAve-2*PointStDev) && (TempPointAve<=PointAve+2*PointStDev))
                        {
                            if ( (TempDataAve>=MarkAve-2*MarkStDev) && (TempDataAve<=MarkAve+2*MarkStDev) )
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
    LocationSimple& zer = zero; //zer is address of LocationSimple zero
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
            if (dist<rad)
            {
                data1 = abs(pix1-PointAve);
                data2 = abs(pix2-PointAve);

                if (data1>data2)
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
