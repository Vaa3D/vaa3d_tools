/* branch_pt_detection_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-12-1 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "branch_pt_detection_plugin.h"
#include "../../../v3d_main/jba/newmat11/newmatap.h"
#include "../../../v3d_main/jba/newmat11/newmatio.h"
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"

using namespace std;
Q_EXPORT_PLUGIN2(branch_pt_detection, branch_pt_detection);
 
QStringList branch_pt_detection::menulist() const
{
	return QStringList() 
        <<tr("detection")
        <<tr("detection_terafly")
		<<tr("about");
}

QStringList branch_pt_detection::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

template <class T> bool compute_Anisotropy_sphere(const T* data1d, V3DLONG N, V3DLONG M, V3DLONG P, V3DLONG c,
                                                  V3DLONG x0, V3DLONG y0, V3DLONG z0,
                                                  V3DLONG rs, double &Score, double & avevalue)
{
    if (!data1d || N<=0 || M<=0 || P<=0 || c<0 || x0<0 || x0>N-1 || y0<0 || y0>M-1 || z0<0 || z0>P-1 || rs<1)
        return false;

        //get the boundary
        V3DLONG offsetc = (c)*(N*M*P);

    V3DLONG xb = x0-rs;if(xb<0) xb = 0;
    V3DLONG xe = x0+rs; if(xe>=N-1) xe = N-1;
    V3DLONG yb = y0-rs;if(yb<0) yb = 0;
    V3DLONG ye = y0+rs; if(ye>=M-1) ye = M-1;
    V3DLONG zb = z0-rs;if(zb<0) zb = 0;
    V3DLONG ze = z0+rs; if(ze>=P-1) ze = P-1;

    V3DLONG i,j,k;
    double w;

    //first get the average value
    double x2, y2, z2;
    double rx2 = double(rs+1)*(rs+1);
    double ry2 = rx2, rz2 = rx2;
    double tmpd;
    double xm=0,ym=0,zm=0, s=0, mv=0, n=0;

    s = 0; n = 0;
    for(k=zb; k<=ze; k++)
    {
        V3DLONG offsetkl = k*M*N;
        z2 = k-z0; z2*=z2;
        for(j=yb; j<=ye; j++)
        {
            V3DLONG offsetjl = j*N;
            y2 = j-y0; y2*=y2;
            tmpd = y2/ry2 + z2/rz2;
            if (tmpd>1.0)
                continue;

            for(i=xb; i<=xe; i++)
            {
                x2 = i-x0; x2*=x2;
                if (x2/rx2 + tmpd > 1.0)
                    continue;

                s += double(data1d[offsetc+offsetkl + offsetjl + i]);
                n = n+1;
            }
        }
    }
    if (n!=0)
        mv = s/n;
    else
        mv = 0;

    //now get the center of mass
    s = 0; n=0;
    for(k=zb; k<=ze; k++)
    {
        V3DLONG offsetkl = k*M*N;
        z2 = k-z0; z2*=z2;
        for(j=yb; j<=ye; j++)
        {
            V3DLONG offsetjl = j*N;
            y2 = j-y0; y2*=y2;
            tmpd = y2/ry2 + z2/rz2;
            if (tmpd>1.0)
                continue;

            for(i=xb; i<=xe; i++)
            {
                x2 = i-x0; x2*=x2;
                if (x2/rx2 + tmpd > 1.0)
                    continue;

                w = double(data1d[offsetc+offsetkl + offsetjl + i]) - mv;
                if (w>0)
                {
                xm += w*i;
                ym += w*j;
                zm += w*k;
                s += w;
                n = n+1;
                }
            }
        }
    }

    if(n>0)
    {
        xm /= s; ym /=s; zm /=s;
    }
    else
    {
        xm = x0; ym=y0; zm=z0;
    }

    double spatial_deviation = sqrt(double(xm-x0)*(xm-x0) + double(ym-y0)*(ym-y0) + double(zm-z0)*(zm-z0)) + 1;
    avevalue = mv;

    double cc11=0, cc12=0, cc13=0, cc22=0, cc23=0, cc33=0;
    double dfx, dfy, dfz;
    for(k=zb; k<=ze; k++)
    {
        z2 = k-z0; z2*=z2;
        dfz = double(k)-zm;
        V3DLONG offsetkl = k*M*N;
        for(j=yb; j<=ye; j++)
        {
            y2 = j-y0; y2*=y2;
            tmpd = y2/ry2 + z2/rz2;
            if (tmpd>1.0)
                continue;

            dfy = double(j)-ym;
            V3DLONG offsetjl = j*N;
            for(i=xb; i<=xe; i++)
            {
                x2 = i-x0; x2*=x2;
                if (x2/rx2 + tmpd > 1.0)
                    continue;

                dfx = double(i)-xm;
                w = double(data1d[offsetc+offsetkl + offsetjl + i]) - mv;
                if (w>0)
                {
                cc11 += w*dfx*dfx;
                cc12 += w*dfx*dfy;
                cc13 += w*dfx*dfz;
                cc22 += w*dfy*dfy;
                cc23 += w*dfy*dfz;
                cc33 += w*dfz*dfz;
                }

            }
        }
    }
    cc11 /= s; 	cc12 /= s; 	cc13 /= s; 	cc22 /= s; 	cc23 /= s; 	cc33 /= s;

    Score = 1; //the Score should at least be 1.
    try
    {
        //then find the eigen vector
        SymmetricMatrix Cov_Matrix(3);
        Cov_Matrix.Row(1) << cc11;
        Cov_Matrix.Row(2) << cc12 << cc22;
        Cov_Matrix.Row(3) << cc13 << cc23 << cc33;

        DiagonalMatrix DD;
        Matrix VV;
        EigenValues(Cov_Matrix,DD,VV);

        //output the result
        double pc1 = DD(3);
        double pc2 = DD(2);
//        double pc3 = DD(1);
        Score = 1/(pc1/pc2/spatial_deviation);
        //Score = sqrt(pc1)/sqrt(pc2)/spatial_deviation;
    }
    catch (...)
    {
        Score = 0;
    }

    return true;

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
    if (menu_name == tr("detection"))
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
        double seg_mean = 0;
        NeuronTree nt = callback.getHandleNeuronTrees_3DGlobalViewer(curwin)->at(0);
        for(V3DLONG i =0; i <nt.listNeuron.size(); i++)
        {
            V3DLONG ix = nt.listNeuron.at(i).x;
            V3DLONG iy = nt.listNeuron.at(i).y;
            V3DLONG iz = nt.listNeuron.at(i).z;

            V3DLONG offsetk = iz*M*N;
            V3DLONG offsetj = iy*N;

            seg_mean += data1d[offsetk + offsetj + ix];
        }
        seg_mean /= nt.listNeuron.size();
        v3d_msg(QString("mean is %1").arg(seg_mean),0);

//        int Wx = 1;
//        int Wy = 1;
//        int Wz = 1;
        int c = sc - 1;
        double rs = 5;

        double score_each = 0, ave_v=0;
        for(V3DLONG i =0; i <nt.listNeuron.size(); i++)
        {
            V3DLONG ix = nt.listNeuron.at(i).x;
            V3DLONG iy = nt.listNeuron.at(i).y;
            V3DLONG iz = nt.listNeuron.at(i).z;

            V3DLONG offsetk = iz*M*N;
            V3DLONG offsetj = iy*N;

            V3DLONG PixelValue = data1d[offsetk + offsetj + ix];
            compute_Anisotropy_sphere(data1d, N, M, P, c, ix, iy, iz, rs, score_each, ave_v);
            if(PixelValue >= seg_mean)
                nt.listNeuron[i].r  = score_each;
            else
                nt.listNeuron[i].r  = 0.001;



//            V3DLONG offsetk = iz*M*N;
//            V3DLONG offsetj = iy*N;


//            V3DLONG xb = ix-Wx; if(xb<0) xb = 0;
//            V3DLONG xe = ix+Wx; if(xe>=N-1) xe = N-1;
//            V3DLONG yb = iy-Wy; if(yb<0) yb = 0;
//            V3DLONG ye = iy+Wy; if(ye>=M-1) ye = M-1;
//            V3DLONG zb = iz-Wz; if(zb<0) zb = 0;
//            V3DLONG ze = iz+Wz; if(ze>=P-1) ze = P-1;

//            double fxx = data1d[offsetk+offsetj+xe]+ data1d[offsetk+offsetj+xb]- 2*data1d[offsetk+offsetj+ix];
//            double fyy = data1d[offsetk+(ye)*N+ix]+data1d[offsetk+(yb)*N+ix]-2*data1d[offsetk+offsetj+ix];
//            double fzz = data1d[(ze)*M*N+offsetj+ix]+data1d[(zb)*M*N+offsetj+ix]- 2*data1d[offsetk+offsetj+ix];

//            double fxy = 0.25*(data1d[offsetk+(ye)*N+xe]+data1d[offsetk+(yb)*N+xb]-data1d[offsetk+(ye)*N+xb]-data1d[offsetk+(yb)*N+xe]);
//            double fxz = 0.25*(data1d[(ze)*M*N+offsetj+xe]+data1d[(zb)*M*N+offsetj+xb]-data1d[(ze)*M*N+offsetj+xb]-data1d[(zb)*M*N+offsetj+xe]);
//            double fyz = 0.25*(data1d[(ze)*M*N+(ye)*N+ix]+data1d[(zb)*M*N+(yb)*N+ix]-data1d[(ze)*M*N+(yb)*N+ix]-data1d[(zb)*M*N+(ye)*N+ix]);

//            SymmetricMatrix Cov_Matrix(3);
//            Cov_Matrix.Row(1) << fxx;
//            Cov_Matrix.Row(2) << fxy << fyy;
//            Cov_Matrix.Row(3) << fxz << fyz <<fzz;

//            DiagonalMatrix DD;

//            EigenValues(Cov_Matrix,DD);
//            double a1 = DD(1), a2 = DD(2), a3 = DD(3);
//            swapthree(a1, a2, a3);
//            nt.listNeuron[i].r = fabs(a3/a1);


        }

        writeSWC_file("/local4/Data/MOST_MOUSE/branch_detection/branch.swc",nt);


	}
    else if (menu_name == tr("detection_terafly"))
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

    if (func_name == tr("detection_terafly"))
	{
        vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
        vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
        vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
        vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
        vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
        vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();

        QString inimg_file = infiles[0];
        int k=0;
        QString swc_name = paras.empty() ? "" : paras[k]; if(swc_name == "NULL") swc_name = ""; k++;
        NeuronTree nt = readSWC_file(swc_name);
        V3DLONG start_x = nt.listNeuron.at(0).x;
        V3DLONG end_x = nt.listNeuron.at(0).x;
        V3DLONG start_y = nt.listNeuron.at(0).y;
        V3DLONG end_y = nt.listNeuron.at(0).y;
        V3DLONG start_z = nt.listNeuron.at(0).z;
        V3DLONG end_z = nt.listNeuron.at(0).z;

        for(V3DLONG i = 1; i < nt.listNeuron.size();i++)
        {
            if(start_x>nt.listNeuron.at(i).x) start_x = nt.listNeuron.at(i).x;
            if(end_x<nt.listNeuron.at(i).x) end_x = nt.listNeuron.at(i).x;

            if(start_y>nt.listNeuron.at(i).y) start_y = nt.listNeuron.at(i).y;
            if(end_y<nt.listNeuron.at(i).y) end_y = nt.listNeuron.at(i).y;

            if(start_z>nt.listNeuron.at(i).z) start_z = nt.listNeuron.at(i).z;
            if(end_z<nt.listNeuron.at(i).z) end_z = nt.listNeuron.at(i).z;

        }

        unsigned char * total1dData = 0;
        total1dData = callback.getSubVolumeTeraFly(inimg_file.toStdString(),start_x,end_x+1,
                                                   start_y,end_y+1,start_z,end_z+1);
        V3DLONG mysz[4];
        mysz[0] = end_x -start_x +1;
        mysz[1] = end_y - start_y +1;
        mysz[2] = end_z - start_z +1;
        mysz[3] = 1;

        double seg_mean = 0;
        for(V3DLONG i =0; i <nt.listNeuron.size(); i++)
        {
            V3DLONG ix = nt.listNeuron.at(i).x - start_x;
            V3DLONG iy = nt.listNeuron.at(i).y - start_y;
            V3DLONG iz = nt.listNeuron.at(i).z - start_z;

            V3DLONG offsetk = iz*mysz[1]*mysz[0];
            V3DLONG offsetj = iy*mysz[0];

            seg_mean += total1dData[offsetk + offsetj + ix];
        }
        seg_mean /= nt.listNeuron.size();

        int c = mysz[3] - 1;
        double rs = 5;

        double score_each = 0, ave_v=0;
        vector<MyMarker> file_inmarkers;
        for(V3DLONG i =0; i <nt.listNeuron.size(); i++)
        {
            V3DLONG ix = nt.listNeuron.at(i).x - start_x;
            V3DLONG iy = nt.listNeuron.at(i).y - start_y;
            V3DLONG iz = nt.listNeuron.at(i).z - start_z;

            V3DLONG offsetk = iz*mysz[1]*mysz[0];
            V3DLONG offsetj = iy*mysz[0];

            V3DLONG PixelValue = total1dData[offsetk + offsetj + ix];
            compute_Anisotropy_sphere(total1dData, mysz[0], mysz[1], mysz[2], c, ix, iy, iz, rs, score_each, ave_v);
            if(PixelValue >= seg_mean && score_each >0.25)
            {
                MyMarker t;
                t.x = nt.listNeuron.at(i).x;
                t.y = nt.listNeuron.at(i).y;
                t.z = nt.listNeuron.at(i).z;
                file_inmarkers.push_back(t);
            }
        }

        QString output_marker = swc_name + "_branches.marker";
        saveMarker_file(output_marker.toStdString(),file_inmarkers);

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

