/* EMEE_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-08-05 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "EMEE_plugin.h"
#define INF 1E9
using namespace std;
Q_EXPORT_PLUGIN2(EMEE, EMEE);
void processImage(V3DPluginCallback2 &callback, QWidget *parent);

QStringList EMEE::menulist() const
{
	return QStringList() 
		<<tr("EMEE")
		<<tr("about");
}

QStringList EMEE::funclist() const
{
	return QStringList()
		<<tr("EMEE")
		<<tr("help");
}

void EMEE::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("EMEE"))
	{
        processImage(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2013-08-05"));
	}
}

bool EMEE::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("EMEE"))
	{
	}
	else if (func_name == tr("help"))
	{
	}
	else return false;
}

void processImage(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    unsigned char* data1d = p4DImage->getRawData();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    //define datatype here

    //input
    bool ok1, ok2, ok3, ok4;
    unsigned int Wx=1, Wy=1, Wz=1, c=1;

    Wx = QInputDialog::getInteger(parent, "Window X ",
                                  "Enter radius (window size is 2*radius+1):",
                                  1, 1, N, 1, &ok1);

    if(ok1)
    {
        Wy = QInputDialog::getInteger(parent, "Window Y",
                                      "Enter radius (window size is 2*radius+1):",
                                      1, 1, M, 1, &ok2);
    }
    else
        return;

    if(ok2)
    {
        Wz = QInputDialog::getInteger(parent, "Window Z",
                                      "Enter radius (window size is 2*radius+1):",
                                      1, 1, P, 1, &ok3);
    }
    else
        return;

    if(sc==1)
    {
        c=1;
        ok4=true;
    }
    else
    {
        if(ok3)
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel NO:",
                                         1, 1, sc, 1, &ok4);
        }
        else
            return;
    }


    V3DLONG offsetc = (c-1)*pagesz;
    double blockvalue = 0;
    V3DLONG blocknum = 0;
    int maxfl , minfl;
    for(V3DLONG iz = 0; iz < P-Wz; iz = iz+Wz)
    {

        for(V3DLONG iy = 0; iy < M-Wy; iy = iy+Wy)
        {

            for(V3DLONG ix = 0; ix < N-Wx; ix = ix+Wx)
            {
                maxfl = 0;
                minfl = INF;
                V3DLONG xb = ix;
                V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
                V3DLONG yb = iy;
                V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
                V3DLONG zb = iz;
                V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;
                for(V3DLONG k=zb; k<ze; k++)
                {
                    V3DLONG offsetkl = k*M*N;
                    for(V3DLONG j=yb; j<ye; j++)
                    {
                        V3DLONG offsetjl = j*N;
                        for(V3DLONG i=xb; i<xe; i++)
                        {
                            int dataval = data1d[ offsetc + offsetkl + offsetjl + i];
                            if(dataval+1>maxfl) maxfl = dataval+1;
                            if(dataval+1<minfl) minfl = dataval+1;

                        }
                    }
                }

                blockvalue = blockvalue +  (maxfl/minfl)*log((double)maxfl/minfl);
                blocknum++;
                // printf("\nEME is %.4f %d %d",blockvalue, maxfl,minfl);
            }
        }
    }
    printf("EMEE is %.5f, %d\n\n",blockvalue/(double)blocknum,blocknum);
    return;
}
