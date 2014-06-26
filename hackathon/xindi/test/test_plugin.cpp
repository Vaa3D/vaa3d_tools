/* test_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2012-01-01 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "test_plugin.h"
#include "string"
#include "sstream"

using namespace std;
Q_EXPORT_PLUGIN2(test, TestPlugin);

void markers_singleChannel(V3DPluginCallback2 &callback, QWidget *parent);
void better_markers_singleChannel(V3DPluginCallback2 &callback, QWidget *parent);
template <class T> int pixelVal(T* data1d, V3DLONG *dimNum,
                                int xc, int yc, int zc, int c);
template <class T> pair<int,int> pixel(T* data1d,
                                        V3DLONG *dimNum,
                                        int xc,int yc,int zc,int c,int rad);
template <class T> pair<int,int> dynamic_pixel(T* data1d,
                                        V3DLONG *dimNum,
                                        int xc, int yc, int zc,
                                        int c, int marknum, int PixVal, int BGVal);
template <class T> LandmarkList count(T* data1d,
                                      V3DLONG *dimNum,
                                      v3dhandle curwin,
                                      int MarkAve, int MarkStDev,
                                      int PointAve, int PointStDev,
                                      int rad, int radAve, int radStDev, int c);
template <class T> LandmarkList duplicates(T* data1d, LandmarkList fullList, int PointAve, int rad);


 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("Single Channel Cell Counting")
        <<tr("Better Cell Counting?!")
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
    if (menu_name == tr("Single Channel Cell Counting"))
	{
        //v3d_msg("To be implemented?");
        markers_singleChannel(callback,parent);
	}
    else if (menu_name == tr("Better Cell Counting?!"))
	{
        //v3d_msg("YES!!!!! There's just nothing here yet.");
        better_markers_singleChannel(callback,parent);
	}
	else
	{
        v3d_msg(tr("Uses current image's landmarks to find similar objects in image."
            "Work-in-process by Xindi, 2014 Summer"));
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

    V3DLONG pages = p4DImage->getTotalUnitNumberPerChannel();
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

        LandmarkList newList = count(data1d,dimNum,curwin,MarkAve,MarkStDev,PointAve,PointStDev,rad,0,0,c);

        //now need to delete duplicate markers on same cell
        LandmarkList smallList = duplicates(data1d,newList,PointAve,rad);
        LandmarkList& woot = smallList;
        bool draw_le_markers = callback.setLandmark(curwin,woot);

    }
    return;
}


void better_markers_singleChannel(V3DPluginCallback2 &callback, QWidget *parent)
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

    V3DLONG pages = p4DImage->getTotalUnitNumberPerChannel();
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
        //sort markers by background/foreground
//        v3d_msg("presort ckpt");
        int pix,num;
        int * PixValArr;
        PixValArr = new int[marknum];
        LandmarkList MarkList, BGList;
//        v3d_msg("sort ckpt 1");
        for (int i=0; i<marknum; i++)
        {
            tmpLocation = mlist.at(i);
            tmpLocation.getCoord(xc,yc,zc);

            pair<int,int> pixAns = pixel(data1d,dimNum,xc,yc,zc,c,rad); //I have no idea why pixelVal isn't working correctly
            pix = pixAns.second;
//            v3d_msg(QString("pix value %1").arg(pix));
            PixValArr[i] = pix;
        }
        int max=0,min=255;
        for (int i=0; i<marknum; i++)
        {
            num=PixValArr[i];
            if (num>max) { max=num; }
            if (num<min) { min=num; }
        }
//        v3d_msg(QString("sort ckpt 2, min %1 max %2").arg(min).arg(max));
        int thresh = (max+min)/2;
        int PixVal=0, BGVal=0;
        for (int i=0; i<marknum; i++)
        {
            num=PixValArr[i];
            tmpLocation = mlist.at(i);
            if (num<thresh) { BGList.append(tmpLocation); BGVal += num; }    //BGList holds bg markers
            if (num>thresh) { MarkList.append(tmpLocation); PixVal += num; }  //MarkList holds cell markers
        }

//        v3d_msg(QString("sort ckpt 3, thresh %1").arg(thresh));

        int marks = MarkList.count();
        PixVal /= marks;            //PixVal now stores average pixel value of all cell markers
        BGVal /= BGList.count();    //BGVal now stores average pixel value of all background markers


//        v3d_msg(QString("marks = %1, bgcount = %2. Marks all sorted").arg(marks).arg(BGList.count())); //crash after this


        //scan list of cell markers for ValAve, radAve
        int xc,yc,zc;
        int * ValAveArr; int * radAveArr;
        ValAveArr = new int[marks]; radAveArr = new int[marks];
        LocationSimple tempLocation(0,0,0);
        int ValAve=0,radAve=0;
        for (int i=0; i<marks; i++)
        {
            tempLocation = MarkList.at(i);
            tempLocation.getCoord(xc,yc,zc);

            pair<int,int> pixAns = pixel(data1d,dimNum,xc,yc,zc,c,rad); //I have no idea why pixelVal isn't working correctly
            int Pix = pixAns.second;

            pair<int,int> dynAns = dynamic_pixel(data1d,dimNum,xc,yc,zc,c,marks,Pix,BGVal);
            ValAveArr[i] = dynAns.first;
            radAveArr[i] = dynAns.second;
            ValAve += ValAveArr[i];
            radAve += radAveArr[i];

//            v3d_msg(QString("ValAve %1, radAve %2").arg(ValAve).arg(radAve));
            //radAve is coming back giant
        }

//        v3d_msg("scan checkpoint");

        ValAve /= marks;  //average pixel value of each segment
        radAve /= marks;  //average radius of segment

//        v3d_msg(QString("FINAL ValAve %1, radAve %2").arg(ValAve).arg(radAve));

        int stV=0, stR=0, stP=0;
        for (int i=0; i<marks; i++)
        {
            int s = pow(ValAveArr[i]-ValAve,2.0);
            stV += s;

            int t = pow(radAveArr[i]-radAve,2.0);
            stR += t;

            tempLocation = MarkList.at(i);
            tempLocation.getCoord(xc,yc,zc);
            pair<int,int> pixAns = pixel(data1d,dimNum,xc,yc,zc,c,rad); //I have no idea why pixelVal isn't working correctly
            int Pix = pixAns.second;
            int u = pow(Pix-PixVal,2.0);
            stP += u;
//            v3d_msg(QString("pixel value %1, diff %2, stP %3").arg(Pix).arg(Pix-PixVal).arg(stP));
        }
        int ValStDev = sqrt(stV/(marks-1.0));
        int radStDev = sqrt(stR/(marks-1.0));
        int PixStDev = sqrt(stP/(marks-1.0));


//        v3d_msg(QString("markers have been scanned. Pixval %1 and stdev %2. radVal %3 and stdev %4."
//                        "segVal %5 and stdev %6").arg(PixVal).arg(PixStDev).arg(radAve).arg(radStDev).arg(ValAve).arg(ValStDev));


        //and here we need to add the function that will scan the rest of the image for other cells based on MarkAve and MarkStDev
        //int CellCnt = count(data1d,dimNum,curwin,MarkAve,MarkStDev,rad,c);
        //v3d_msg(QString("There are %1 cells in this channel").arg(CellCnt));

        LandmarkList newList = count(data1d,dimNum,curwin,ValAve,2*ValStDev,PixVal,PixStDev,0,radAve,5+radStDev,c);

        LandmarkList& woot = newList;
        bool draw_le_markers = callback.setLandmark(curwin,woot);
//        v3d_msg(QString("newList has %1 markers").arg(newList.count()));


//        v3d_msg("new markers have been found");
        //now need to delete duplicate markers on same cell
        LandmarkList smallList = duplicates(data1d,newList,PixVal,radAve);
        LandmarkList& woot2 = smallList;
        bool draw_le_markers2 = callback.setLandmark(curwin,woot2);
    }
    return;
}

template <class T> int pixelVal(T* data1d, V3DLONG *dimNum,
                                int xc, int yc, int zc, int c)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    V3DLONG sc = dimNum[3];
    //1D data array stores in this order: C Z Y X
    V3DLONG shiftC = (c-1)*P*M*N;
    int pixelVal = data1d[ shiftC + zc*M*N + yc*N + xc ];
    return pixelVal;
}


template <class T> pair<int,int> dynamic_pixel(T* data1d,
                                        V3DLONG *dimNum,
                                        int xc, int yc, int zc,
                                        int c, int marknum, int PixVal, int BGVal)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    V3DLONG sc = dimNum[3];
    //1D data array stores in this order: C Z Y X
    V3DLONG shiftC = (c-1)*P*M*N;

//    int xc,yc,zc;
//    LocationSimple tempLocation(0,0,0);
//    for (int t=0; t<MarkList.count(); t++)
//    {
//        tempLocation = MarkList.at(t);
//        tempLocation.getCoord(xc,yc,zc);
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
//        ValAveArr[t]=dataAve; radAveArr[t]=rad;
//    }
    return make_pair(dataAve,rad);
}


//returns average pixel value in box of radius rad on channel c around a given marker
template <class T> pair<int,int> pixel(T* data1d,
                              V3DLONG *dimNum,
                              int xc,int yc,int zc,int c,int rad)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    V3DLONG sc = dimNum[3];
    //1D data array stores in this order: C Z Y X
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
                              v3dhandle curwin,
                              int MarkAve, int MarkStDev,
                              int PointAve, int PointStDev,
                              int rad, int radAve, int radStDev, int c)
{
    V3DLONG N = dimNum[0];
    V3DLONG M = dimNum[1];
    V3DLONG P = dimNum[2];
    V3DLONG sc = dimNum[3];
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
                            newList.append(tmpLocation);

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
        for (int i=radAve-radStDev; i<radAve+radStDev; i++)
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
template <class T> LandmarkList duplicates(T* data1d, LandmarkList fullList, int PointAve, int rad)
{
    int marknum = fullList.count();
    LandmarkList smallList = fullList;
    int x1,y1,z1,x2,y2,z2,dist,data1,data2;
    LocationSimple point1(0,0,0), point2(0,0,0);
    LocationSimple zero(0,0,0);
    LocationSimple& zer = zero; //zer is address of LocationSimple zero
    for (int i=0; i<marknum; i++)
    {
        for (int j=i+1; j<marknum; j++)
        {
            point1 = smallList.at(i);
            point1.getCoord(x1,y1,z1);
            point2 = smallList.at(j);
            point2.getCoord(x2,y2,z2);
            LocationSimple& adr1 = point1;
            LocationSimple& adr2 = point2;

            dist = sqrt( pow(x1-x2,2.0) + pow(y1-y2,2.0) + pow(z1-z2,2.0) );
            if (dist<rad)
            {
                data1 = abs(point1.getPixVal()-PointAve);
                data2 = abs(point2.getPixVal()-PointAve);

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
