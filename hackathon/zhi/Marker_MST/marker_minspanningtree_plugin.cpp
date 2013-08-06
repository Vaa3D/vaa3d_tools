/* marker_minspanningtree_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-08-03 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "marker_minspanningtree_plugin.h"
#include <iostream>


#include <fstream>
#include <sstream>
//#ifndef test_main_cpp


#define INF 1E9
using namespace std;
Q_EXPORT_PLUGIN2(marker_minspanningtree, markertree);
void processImage(V3DPluginCallback2 &callback, QWidget *parent);

QStringList markertree::menulist() const
{
	return QStringList() 
        <<tr("generate a MST for all the markers")
		<<tr("about");
}

QStringList markertree::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void markertree::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("generate a MST for all the markers"))
	{
		processImage(callback,parent);
	}
	else
	{
        v3d_msg(tr("This information display makes no nose, improve it now! "
			"Developed by Zhi Zhou, 2013-08-03"));
	}
}

void processImage(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }
	
    Image4DSimple* p4DImage = callback.getImage(curwin);
    QString imgname = callback.getImageName(curwin);
	
    if (!p4DImage)
    {
        v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    int tmpx,tmpy,tmpz,x1,y1,z1;
	LandmarkList listLandmarks = callback.getLandmark(curwin);
	LocationSimple tmpLocation(0,0,0);
	int marknum = listLandmarks.count();
	double** markEdge = new double*[marknum];
	int** marktree = new int*[marknum];
	for(int i = 0; i < marknum; i++)
	{
		markEdge[i] = new double[marknum];
		marktree[i] = new int[marknum];
		
	}

	
	for (int i=0;i<marknum;i++)
	{
		tmpLocation = listLandmarks.at(i);
		tmpLocation.getCoord(tmpx,tmpy,tmpz);
		x1 = tmpx;
		y1 = tmpy;
		z1 = tmpz;
		for (int j=0;j<marknum;j++)
		{
			tmpLocation = listLandmarks.at(j);
			tmpLocation.getCoord(tmpx,tmpy,tmpz);
			markEdge[i][j] = sqrt(pow(x1-tmpx,2)+pow(y1-tmpy,2)+pow(z1-tmpz,2));
			marktree[i][j] = 0;
		}
	}
    /*for(int rowCnt = 0; rowCnt < marknum; rowCnt++)
	{
		for(int colCnt = 0; colCnt < marknum; colCnt++)
		{
			printf("%.2f  ",markEdge[rowCnt][colCnt]);
		}
		cout << endl;
    }*/
	
	int* pi = new int[marknum];
	for(int i = 0; i< marknum;i++)
		pi[i] = 0;
	pi[0] = 1;
	int indexi,indexj;
	for(int loop = 0; loop<marknum;loop++)
	{
		double min = INF;
		  for(int i = 0; i<marknum; i++)
		  {
			if (pi[i] == 1)
			{
				for(int j = 0;j<marknum; j++)
				{
					if(pi[j] == 0 && min > markEdge[i][j])
					{
						min = markEdge[i][j];
						indexi = i;
						indexj = j;
					}
				}
			}
				
		  }
		marktree[indexi][indexj] = 1;
		pi[indexj] = 1;		
	}
	    
    QString outfilename = imgname + "_marker.swc";
    if (outfilename.startsWith("http", Qt::CaseInsensitive))
    {
        QFileInfo ii(outfilename);
        outfilename = QDir::home().absolutePath() + "/" + ii.fileName();
    }
    
    v3d_msg(QString("The anticipated output file is [%1]").arg(outfilename));
    
    
    //write the data structure to file. Need further improvement in the futuyre to ensure the integrity and also the convention of the SWC format
    //writeSWC_file(const QString& filename, const NeuronTree& nt, const QStringList *infostring=0);
    
    FILE * fp = fopen(qPrintable(outfilename), "wt");
    if (!fp)
    {
        v3d_msg("Could not open the file to save the neuron.");
        return;
    }

    fprintf(fp, "##n,type,x,y,z,radius,parent\n");

    long i = 0;
    long cons = -1;
	for(int rowCnt = 0; rowCnt < marknum; rowCnt++)
	{
		for(int colCnt = 0; colCnt < marknum; colCnt++)
		{
            if(marktree[rowCnt][colCnt]==1)
            {
                tmpLocation = listLandmarks.at(rowCnt);
                tmpLocation.getCoord(tmpx,tmpy,tmpz);
                fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld\n",
                        2*i+1, 3, (double)tmpx-1.0, (double)tmpy-1.0, (double)tmpz-1.0,1.0,cons);

                tmpLocation = listLandmarks.at(colCnt);
                tmpLocation.getCoord(tmpx,tmpy,tmpz);
                fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld\n",
                        2*(i+1), 3, (double)tmpx-1.0, (double)tmpy-1.0, (double)tmpz-1.0,1.0,2*i+1);
                i++;
            }
		}
	}

    fclose(fp);
    v3d_msg(QString("You have totally [%1] markers for the file [%2] and the computed MST has been saved to the file [%3]").arg(marknum).arg(imgname).arg(outfilename));
    return;
}



bool markertree::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
        v3d_msg("To be implemented");
	}
	else if (func_name == tr("func2"))
	{
        v3d_msg("To be implemented");
	}
	else if (func_name == tr("help"))
	{
        v3d_msg("To be implemented");
	}
	else return false;
    
    return true;
}

