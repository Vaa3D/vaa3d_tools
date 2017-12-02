/* branch_pt_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-12-1 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "branch_pt_detection_plugin.h"
#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"

using namespace std;
Q_EXPORT_PLUGIN2(branch_pt_detection, branch_pt_detection);
 
QStringList branch_pt_detection::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList branch_pt_detection::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

template <class T> bool swapthree(T& dummya, T& dummyb, T& dummyc)
{

    if ( (fabs(dummya) >= fabs(dummyb)) && (fabs(dummyb) >= fabs(dummyc)) )
    {
    }
    else if ( (fabs(dummya) >= fabs(dummyc)) && (fabs(dummyc) >= fabs(dummyb)) )
    {
        T temp = dummyb;
        dummyb = dummyc;
        dummyc = temp;
    }
    else if ( (fabs(dummyb) >= fabs(dummya)) && (fabs(dummya) >= fabs(dummyc)) )
    {
        T temp = dummya;
        dummya = dummyb;
        dummyb = temp;
    }
    else if ( (fabs(dummyb) >= fabs(dummyc)) && (fabs(dummyc) >= fabs(dummya)) )
    {
        T temp = dummya;
        dummya = dummyb;
        dummyb = dummyc;
        dummyc = temp;
    }
    else if ( (fabs(dummyc) >= fabs(dummya)) && (fabs(dummya) >= fabs(dummyb)) )
    {
        T temp = dummya;
        dummya = dummyc;
        dummyc = dummyb;
        dummyb = temp;
    }
    else if ( (fabs(dummyc) >= fabs(dummyb)) && (fabs(dummyb) >= fabs(dummya)) )
    {
        T temp = dummyc;
        dummyc = dummya;
        dummya = temp;
    }
    else
    {
        return false;
    }

    return true;
}

void branch_pt_detection::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("menu1"))
	{
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            v3d_msg("You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
            return;
        }

        unsigned char* data1d = p4DImage->getRawData();
        V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

        V3DLONG N = p4DImage->getXDim();
        V3DLONG M = p4DImage->getYDim();
        V3DLONG P = p4DImage->getZDim();
        V3DLONG sc = p4DImage->getCDim();
        ImagePixelType pixeltype = p4DImage->getDatatype();

        V3DLONG in_sz[4];
        in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = 1;

        int Wx = 5;
        int Wy = 5;
        int Wz = 5;

        NeuronTree nt = callback.getHandleNeuronTrees_3DGlobalViewer(curwin)->at(0);
        for(V3DLONG i =0; i <nt.listNeuron.size(); i++)
        {
            V3DLONG ix = nt.listNeuron.at(i).x;
            V3DLONG iy = nt.listNeuron.at(i).y;
            V3DLONG iz = nt.listNeuron.at(i).z;

            V3DLONG offsetk = iz*M*N;
            V3DLONG offsetj = iy*N;


            V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
            V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
            V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
            V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
            V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
            V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;

            double fxx = data1d[offsetk+offsetj+xe]+ data1d[offsetk+offsetj+xb]- 2*data1d[offsetk+offsetj+ix];
            double fyy = data1d[offsetk+(ye)*N+ix]+data1d[offsetk+(yb)*N+ix]-2*data1d[offsetk+offsetj+ix];
            double fzz = data1d[(ze)*M*N+offsetj+ix]+data1d[(zb)*M*N+offsetj+ix]- 2*data1d[offsetk+offsetj+ix];

            double fxy = 0.25*(data1d[offsetk+(ye)*N+xe]+data1d[offsetk+(yb)*N+xb]-data1d[offsetk+(ye)*N+xb]-data1d[offsetk+(yb)*N+xe]);
            double fxz = 0.25*(data1d[(ze)*M*N+offsetj+xe]+data1d[(zb)*M*N+offsetj+xb]-data1d[(ze)*M*N+offsetj+xb]-data1d[(zb)*M*N+offsetj+xe]);
            double fyz = 0.25*(data1d[(ze)*M*N+(ye)*N+ix]+data1d[(zb)*M*N+(yb)*N+ix]-data1d[(ze)*M*N+(yb)*N+ix]-data1d[(zb)*M*N+(ye)*N+ix]);

            SymmetricMatrix Cov_Matrix(3);
            Cov_Matrix.Row(1) << fxx;
            Cov_Matrix.Row(2) << fxy << fyy;
            Cov_Matrix.Row(3) << fxz << fyz <<fzz;

            DiagonalMatrix DD;

            EigenValues(Cov_Matrix,DD);
            double a1 = DD(1), a2 = DD(2), a3 = DD(3);
            swapthree(a1, a2, a3);
            if(a1<0 && a2<0)
            {
                nt.listNeuron[i].r = 1/fabs((fabs(a3)-fabs(a1))*(fabs(a3)-fabs(a2)));

            }else
                nt.listNeuron[i].r = 0;

        }

        writeSWC_file("/local4/Data/MOST_MOUSE/branch_detection/branch.swc",nt);


	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2017-12-1"));
	}
}

bool branch_pt_detection::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

