/*
 *  swc_to_maskimage .cpp
 *  swc_to_maskimage
 *
 *  Created by Yang, Jinzhu, on 11/27/10.
 *  Last change: by Hanchuan peng, 2010-Dec-8
 */

#include <QtGlobal>

#include "swc_to_maskimage.h"
#include "customary_structs/vaa3d_neurontoolbox_para.h"
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "stackutil.h"
#include <vector>
using namespace std;

// lroundf() is gcc-specific --CMB
#ifdef _MSC_VER
inline long lroundf(float num) { return static_cast<long>(num > 0 ? num + 0.5f : ceilf(num - 0.5f)); }
#endif

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(swc_to_maskimage, SWC_TO_MASKIMAGElugin);


//plugin funcs
const QString title = "swc_to_maskimage";
QStringList SWC_TO_MASKIMAGElugin::menulist() const
{
    return QStringList()
	<< tr("swc_to_maskimage")
	<<tr("multiple SWC_to_maskimage")
	<<tr("maskimage filter")
	<<tr("Help");
}
QStringList SWC_TO_MASKIMAGElugin::funclist() const
{
	return QStringList()
	<<tr("swc_to_maskimage")
	<<tr("help");
}
bool SWC_TO_MASKIMAGElugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	if (func_name==tr("swc_to_maskimage"))
	{
        swc_to_maskimage(callback, input, output);
	}
	else if (func_name==tr("help"))
	{
		printHelp();
	}
	else if (func_name==tr("TOOLBOXswc_to_maskimage"))
	{
		swc_to_maskimage_toolbox(input, callback, parent);
		return true;
	}
	return false;
}

void SWC_TO_MASKIMAGElugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("swc_to_maskimage"))
	{
    	swc_to_maskimage(callback, parent,1 );
    }else if (menu_name == tr("multiple SWC_to_maskimage"))
	{
		swc_to_maskimage(callback, parent,2);

	}else if (menu_name == tr("maskimage filter"))
	{
		mrskimage_originalimage(callback, parent,1);
	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("(version 0.11) Convert a SWC file to a mask image, where the area of the swc tubes will have non-zero values, and other area will have 0s; Use mask imge to filter tif image, where the area of the swc non-zero will tif image values not change, and other area will have 0");
		return;
	}
}

QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons)
{
	QHash<V3DLONG, V3DLONG> neuron_id_table;
	for (V3DLONG i=0;i<neurons.listNeuron.size(); i++)
		neuron_id_table.insert(V3DLONG(neurons.listNeuron.at(i).n), i);
	return neuron_id_table;
}

void BoundNeuronCoordinates(NeuronTree & neuron,
							bool b_subtractMinFromAllNonnegatives,
							double & output_xmin,
							double & output_xmax,
							double & output_ymin,
							double & output_ymax,
							double & output_zmin,
							double & output_zmax
)
{
	double xmin, ymin, zmin, xmax, ymax, zmax;
	NeuronSWC *p_cur = 0;
	V3DLONG ii;

	//initial search
	for (ii=0; ii<neuron.listNeuron.size(); ii++)
	{
		p_cur = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));

		if (ii==0)
		{
			xmin = p_cur->x - p_cur->r;
			ymin = p_cur->y - p_cur->r;
			zmin = p_cur->z - p_cur->r;
			xmax = p_cur->x + p_cur->r;
			ymax = p_cur->y + p_cur->r;
			zmax = p_cur->z + p_cur->r;
		}
		else
		{
			xmin = (p_cur->x - p_cur->r < xmin) ? (p_cur->x - p_cur->r) : xmin;
			ymin = (p_cur->y - p_cur->r < ymin) ? (p_cur->y - p_cur->r) : ymin;
			zmin = (p_cur->z - p_cur->r < zmin) ? (p_cur->z - p_cur->r) : zmin;

			xmax = (p_cur->x + p_cur->r > xmax) ? (p_cur->x + p_cur->r) : xmax;
			ymax = (p_cur->y + p_cur->r > ymax) ? (p_cur->y + p_cur->r) : ymax;
			zmax = (p_cur->z + p_cur->r > zmax) ? (p_cur->z + p_cur->r) : zmax;
		}
	}

	//subtraction
//	v3d_msg(QString("xmin %1 ymin %2 zmin %3 bool %4").arg(xmin).arg(ymin).arg(zmin).arg(b_subtractMinFromAllNonnegatives));
	for (ii=0; ii<neuron.listNeuron.size(); ii++)
	{
		p_cur = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));

		if (xmin<0 || b_subtractMinFromAllNonnegatives) p_cur->x -= xmin;
		if (ymin<0 || b_subtractMinFromAllNonnegatives) p_cur->y -= ymin;
		if (zmin<0 || b_subtractMinFromAllNonnegatives) p_cur->z -= zmin;
	}


	//re-search the bounding box
	for (ii=0; ii<neuron.listNeuron.size(); ii++)
	{
		p_cur = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));

		if (ii==0)
		{
			output_xmin = p_cur->x - p_cur->r;
			output_ymin = p_cur->y - p_cur->r;
			output_zmin = p_cur->z - p_cur->r;
			output_xmax = p_cur->x + p_cur->r;
			output_ymax = p_cur->y + p_cur->r;
			output_zmax = p_cur->z + p_cur->r;
		}
		else
		{
			output_xmin = (p_cur->x - p_cur->r < output_xmin) ? (p_cur->x - p_cur->r) : output_xmin;
			output_ymin = (p_cur->y - p_cur->r < output_ymin) ? (p_cur->y - p_cur->r) : output_ymin;
			output_zmin = (p_cur->z - p_cur->r < output_zmin) ? (p_cur->z - p_cur->r) : output_zmin;

			output_xmax = (p_cur->x + p_cur->r > output_xmax) ? (p_cur->x + p_cur->r) : output_xmax;
			output_ymax = (p_cur->y + p_cur->r > output_ymax) ? (p_cur->y + p_cur->r) : output_ymax;
			output_zmax = (p_cur->z + p_cur->r > output_zmax) ? (p_cur->z + p_cur->r) : output_zmax;
		}
	}

	return;
}

void ComputemaskImage(NeuronTree neurons,
					  unsigned char* pImMask, //output mask image
					  unsigned char* ImMark,  //an indicator image to show whether or not a pixel has been visited/processed
					  V3DLONG sx,
					  V3DLONG sy,
					  V3DLONG sz,
					  int method_code
)
{
	NeuronSWC *p_cur = 0;
	//double check the data to ensure it is correct!
	for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
	{
		p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
	//	v3d_msg(QString("x %1 y %2 z %3 r %4\n").arg(p_cur->x).arg(p_cur->y).arg(p_cur->z).arg(p_cur->r),0);

		if (p_cur->x<0 || p_cur->y<0 || p_cur->z<0 || p_cur->r<0)
		{
			v3d_msg("You have illeagal x,y,z coordinates or radius values. Check your data.");
			return;
		}
	}
	//create a LUT
	QHash<V3DLONG, V3DLONG> neuron_id_table = NeuronNextPn(neurons);

	//compute mask
	double xs = 0, ys = 0, zs = 0, xe = 0, ye = 0, ze = 0, rs = 0, re = 0;
	V3DLONG pagesz = sx*sy;
	for (V3DLONG ii=0; ii<neurons.listNeuron.size(); ii++)
	{
		V3DLONG i,j,k;

		p_cur = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));

		xs = p_cur->x;
		ys = p_cur->y;
		zs = p_cur->z;
		rs = p_cur->r;
		if (rs<1)
		{
			//rs=2-rs;
		}
		double ballx0, ballx1, bally0, bally1, ballz0, ballz1, tmpf;

		ballx0 = xs - rs; ballx0 = qBound(double(0), ballx0, double(sx-1));
		ballx1 = xs + rs; ballx1 = qBound(double(0), ballx1, double(sx-1));
		if (ballx0>ballx1) {tmpf = ballx0; ballx0 = ballx1; ballx1 = tmpf;}

		bally0 = ys - rs; bally0 = qBound(double(0), bally0, double(sy-1));
		bally1 = ys + rs; bally1 = qBound(double(0), bally1, double(sy-1));
		if (bally0>bally1) {tmpf = bally0; bally0 = bally1; bally1 = tmpf;}

		ballz0 = zs - rs; ballz0 = qBound(double(0), ballz0, double(sz-1));
		ballz1 = zs + rs; ballz1 = qBound(double(0), ballz1, double(sz-1));
		if (ballz0>ballz1) {tmpf = ballz0; ballz0 = ballz1; ballz1 = tmpf;}

        //mark all voxels close to the swc node(s)

	    for (k = ballz0; k <= ballz1; k++)
		{
			for (j = bally0; j <= bally1; j++)
			{
				for (i = ballx0; i <= ballx1; i++)
				{
					double norms10 = (xs-i)*(xs-i) + (ys-j)*(ys-j) + (zs-k)*(zs-k);
					double dt = sqrt(norms10);

					V3DLONG ind = (k)*pagesz + (j)*sx + i;

					if(dt <=rs || dt<=1)
					{
						if (method_code == 1)
						{
							pImMask[ind] = 255;

						}else if (method_code ==2)
						{
							ImMark[ind] = 1;

						}

					}
				}
			}
		}

		//find previous node
		if (p_cur->pn < 0) //then it is root node already
		{
			continue;
		}

		//get the parent info

		const NeuronSWC & pp  = neurons.listNeuron.at(neuron_id_table.value(p_cur->pn));
		xe = pp.x;
		ye = pp.y;
		ze = pp.z;
		re = pp.r;
		if (re<1)
		{
			//re=2-re;
		}
		//judge if two points overlap, if yes, then do nothing as the sphere has already been drawn
		if (xe==xs && ye==ys && ze==zs)
		{
			v3d_msg(QString("Detect overlapping coordinates of node [%1]\n").arg(p_cur->n), 0);
			continue;
		}

//		//only set the current point's value in the mask image
//		pImMask[V3DLONG(zs)*sx*sy + V3DLONG(ys)*sx + V3DLONG(xs)] = random()%250 + 1;
//		continue;

		double l =sqrt((xe-xs)*(xe-xs)+(ye-ys)*(ye-ys)+(ze-zs)*(ze-zs));

		//printf("l=%lf\n",l);

		V3DLONG xn,yn,zn;

//		for(double kk = 0; kk <= l ; kk++)
//		{
//			xn = xs + (xe-xs)*(kk/(l*1.5));
//			yn = ys + (ye-ys)*(kk/(l*1.5));
//			zn = zs + (ze-zs)*(kk/(l*1.5));
//
//		    xn = ( xn > sx )? sx : xn;
//			yn = ( yn > sy )? sy : yn;
//			zn = ( zn > sz )? sz : zn;
//
//			V3DLONG idex=(zn)*sx*sy + (yn)*sx + xn;
//			if (method_code == 1)
//			{
//				pImMask[idex] = 255;
//			}
//			else if (method_code ==2)
//			{
//				ImMark[idex] = 1;
//			}
//		}

		double dx = (xe - xs);
		double dy = (ye - ys);
		double dz = (ze - zs);
//		if (dx==0|| dz==0 || dy==0)
//		{
//			//printf("xs=%lf xe=%lf ys=%lf ye=%lf zs=%lf ze=%lf \n",xs,xe,ys,ye,zs,ze);
//			//printf("dx=%lf dy=%lf dz=%lf\n",dx,dy,dz);
//		}

		double x = xs;
		double y = ys;
		double z = zs;

		int steps = lroundf(l);

		steps = (steps < fabs(dx))? fabs(dx):steps;
		steps = (steps < fabs(dy))? fabs(dy):steps;
		steps = (steps < fabs(dz))? fabs(dz):steps;
		if (steps<1)
		{
			steps =1;
		}
	//	printf("steps=%ld\n",steps);

		double xIncrement = double(dx) / (steps*2);
		double yIncrement = double(dy) / (steps*2);
		double zIncrement = double(dz) / (steps*2);
//		if (xIncrement == 0 || yIncrement==0|| zIncrement==0)
//		{
//			printf("xIncrement=%lf yIncrement=%lf zIncrement=%lf\n",xIncrement,yIncrement,zIncrement);
//			//printf("dx=%lf dy=%lf dz=%lf\n",dx,dy,dz);
//		}
		//printf("xIncrement=%lf yIncrement=%lf zIncrement=%lf\n",xIncrement,yIncrement,zIncrement);

		V3DLONG idex1=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);
//
		if (method_code == 1)
		{
			pImMask[idex1] = 255;
		}
		else if (method_code ==2)
		{
			ImMark[idex1] = 1;
		}
		for (int i = 0; i <= steps; i++)
		{
			x += xIncrement;
			y += yIncrement;
			z += zIncrement;

		    x = ( x > sx )? sx : x;
			y = ( y > sy )? sy : y;
			z = ( z > sz )? sz : z;

			V3DLONG idex=lroundf(z)*sx*sy + lroundf(y)*sx + lroundf(x);

			if (method_code == 1)
			{
				pImMask[idex] = 255;
			}
			else if (method_code ==2)
			{
				ImMark[idex] = 1;
			}

		}

		//finding the envelope of the current line segment

		double rbox = (rs>re) ? rs : re;
		double x_down = (xs < xe) ? xs : xe; x_down -= rbox; x_down = V3DLONG(x_down); if (x_down<0) x_down=0; if (x_down>=sx-1) x_down = sx-1;
		double x_top  = (xs > xe) ? xs : xe; x_top  += rbox; x_top  = V3DLONG(x_top ); if (x_top<0)  x_top=0;  if (x_top>=sx-1)  x_top  = sx-1;
		double y_down = (ys < ye) ? ys : ye; y_down -= rbox; y_down = V3DLONG(y_down); if (y_down<0) y_down=0; if (y_down>=sy-1) y_down = sy-1;
		double y_top  = (ys > ye) ? ys : ye; y_top  += rbox; y_top  = V3DLONG(y_top ); if (y_top<0)  y_top=0;  if (y_top>=sy-1)  y_top = sy-1;
		double z_down = (zs < ze) ? zs : ze; z_down -= rbox; z_down = V3DLONG(z_down); if (z_down<0) z_down=0; if (z_down>=sz-1) z_down = sz-1;
		double z_top  = (zs > ze) ? zs : ze; z_top  += rbox; z_top  = V3DLONG(z_top ); if (z_top<0)  z_top=0;  if (z_top>=sz-1)  z_top = sz-1;

		//compute cylinder and flag mask

		for (k=z_down; k<=z_top; k++)
		{
			for (j=y_down; j<=y_top; j++)
			{
				for (i=x_down; i<=x_top; i++)
				{
					double rr = 0;
					double countxsi = (xs-i);
					double countysj = (ys-j);
					double countzsk = (zs-k);
					double countxes = (xe-xs);
					double countyes = (ye-ys);
					double countzes = (ze-zs);
					double norms10 = countxsi * countxsi + countysj * countysj + countzsk * countzsk;
					double norms21 = countxes * countxes + countyes * countyes + countzes * countzes;
					double dots1021 = countxsi * countxes + countysj * countyes + countzsk * countzes;
					double dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
					double t1 = -dots1021/norms21;
				//	printf("t1=%lf\n",t1);
					if(t1<0)
                        dist = sqrt(norms10);
                    else if(t1>1)
                        dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k));
					//compute rr
					if (rs==re)
					{
						rr =rs;

					}else
					{
						// compute point of intersection
						double v1 = xe - xs;
						double v2 = ye - ys;
						double v3 = ze - zs;
						double vpt = v1*v1 + v2*v2 +v3*v3;
						double t = (double(i-xs)*v1 +double(j-ys)*v2 + double(k-zs)*v3)/vpt;
						double xc = xs + v1*t;
						double yc = ys + v2*t;
						double zc = zs + v3*t;
						double normssc = sqrt((xs-xc)*(xs-xc)+(ys-yc)*(ys-yc)+(zs-zc)*(zs-zc));
						double normsce = sqrt((xe-xc)*(xe-xc)+(ye-yc)*(ye-yc)+(ze-zc)*(ze-zc));
						rr = (rs >= re) ? (rs - ((rs - re)/sqrt(norms21))*normssc) : (re - ((re-rs)/sqrt(norms21))*normsce);
					}
					V3DLONG ind1 = (k)*sx*sy + (j)*sx + i;
					//printf("rr=%lf dist=%lf \n",rr,dist);
					if (dist <= rr || dist<=1)
					{
						if (method_code == 1)
						{
							pImMask[ind1] = 255;
						}
						else if (method_code ==2)
						{
							ImMark[ind1] = 1;
						}
					}
				}
			}
		}
	}
	if (method_code == 2)
	{
		for (V3DLONG k = 0; k < sz; k++)
		{
			for(V3DLONG j = 0; j < sy; j++)
			{
				for(V3DLONG i = 0; i < sx; i ++)
				{
					pImMask[k*sx*sy + j*sx +i] += ImMark[k*sx*sy + j*sx +i];

				}
			}
		}
	}


}

void swc_to_maskimage(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
	NeuronTree neuron;
	double x_min,x_max,y_min,y_max,z_min,z_max;
	x_min=x_max=y_min=y_max=z_min=z_max=0;
	V3DLONG sx,sy,sz;
	unsigned char* pImMask = 0;
	unsigned char* ImMark = 0;
	QString filename;
	QStringList filenames;
	V3DLONG h;
	V3DLONG d;
	V3DLONG nx,ny,nz;
	if (method_code == 1)
	{
		filename = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
												"",
												QObject::tr("Supported file (*.swc)"
															";;Neuron structure	(*.swc)"
															));
		if(filename.isEmpty())
		{
			v3d_msg("You don't have any SWC file open in the main window.");
			return;
		}

		NeuronSWC *p_t=0;
		if (filename.size()>0)
		{
			neuron = readSWC_file(filename);

			bool b_subtractMinFromAllNonnegatives = false;
			BoundNeuronCoordinates(neuron,
									b_subtractMinFromAllNonnegatives,
									x_min,
									x_max,
									y_min,
									y_max,
									z_min,
								    z_max);

//			for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
//			{
//				p_t = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));
//				p_t->x=(p_t->x < 0)?(p_t->x - x_min):p_t->x;
//				p_t->y=(p_t->y < 0)?(p_t->y - y_min):p_t->y;
//				p_t->z=(p_t->z < 0)?(p_t->z - z_min):p_t->z;

//			  //v3d_msg(QString("x %1 y %2 z %3 r %4\n").arg(p_cur->x).arg(p_cur->y).arg(p_cur->z).arg(p_cur->r),0);
//			}
			sx = (b_subtractMinFromAllNonnegatives || x_min<0) ? V3DLONG(ceil(x_max - x_min + 1)) : V3DLONG(ceil(x_max + 1));
			sy = (b_subtractMinFromAllNonnegatives || y_min<0) ? V3DLONG(ceil(y_max - y_min + 1)) : V3DLONG(ceil(y_max + 1));
			sz = (b_subtractMinFromAllNonnegatives || z_min<0) ? V3DLONG(ceil(z_max - z_min + 1)) : V3DLONG(ceil(z_max + 1));
            qDebug()<<"sx:"<<sx<<"sy:"<<sy<<"sz:"<<sz;

			V3DLONG stacksz = sx*sy*sz;
			try
			{
				pImMask = new unsigned char [stacksz];
				ImMark = new unsigned char [stacksz];
			}
			catch (...)
			{
				v3d_msg("Fail to allocate memory.\n");
				return;
			}

			for (V3DLONG i=0; i<stacksz; i++)
					pImMask[i] = ImMark[i] = 0;

			ComputemaskImage(neuron, pImMask, ImMark, sx, sy, sz,1);
		}
		else
		{
			v3d_msg("You don't have any SWC file open in the main window.");
			return;
		}

	}
	///////////////////////////////////////////////////////
	else if (method_code ==2)
	{

		filenames = QFileDialog::getOpenFileNames(0, 0,"","Supported file (*.swc)" ";;Neuron structure(*.swc)",0,0);
		if(filenames.isEmpty())
		{
			v3d_msg("You don't have any SWC file open in the main window.");
			return;
		}
		NeuronSWC *p_cur=0;

		for (V3DLONG i = 0; i < filenames.size();i++)//////re-search the bounding box
		{
			filename = filenames[i];
			if (filename.size()>0)
			{
				neuron = readSWC_file(filename);
				for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
				{
					p_cur = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));
					if (p_cur->r<=0)
					{
						v3d_msg("You have illeagal radius values. Check your data.");
						return;
					}
					x_min = (p_cur->x - p_cur->r < x_min) ? (p_cur->x - p_cur->r) : x_min;
					y_min = (p_cur->y - p_cur->r < y_min) ? (p_cur->y - p_cur->r) : y_min;
					z_min = (p_cur->z - p_cur->r < z_min) ? (p_cur->z - p_cur->r) : z_min;

					x_max = (p_cur->x + p_cur->r > x_max) ? (p_cur->x + p_cur->r) : x_max;
					y_max = (p_cur->y + p_cur->r > y_max) ? (p_cur->y + p_cur->r) : y_max;
					z_max = (p_cur->z + p_cur->r > z_max) ? (p_cur->z + p_cur->r) : z_max;

				}
			}
			else
			{
				v3d_msg("You don't have any SWC file open in the main window.");
				return;
			}

		}
		sx = (x_min<0) ? V3DLONG(ceil(x_max - x_min + 1)) : V3DLONG(ceil(x_max + 1));
		sy = (y_min<0) ? V3DLONG(ceil(y_max - y_min + 1)) : V3DLONG(ceil(y_max + 1));
		sz = (z_min<0) ? V3DLONG(ceil(z_max - z_min + 1)) : V3DLONG(ceil(z_max + 1));

	//	v3d_msg(QString("sx %1 sy %2 sz %3  \n").arg(sx).arg(sy).arg(sz));
		V3DLONG stacksz = sx*sy*sz;
		try
		{
			pImMask = new unsigned char [stacksz];
		}
		catch (...)
		{
			v3d_msg("Fail to allocate memory.\n");
			return;
		}

		for (V3DLONG i=0; i<stacksz; i++)
			pImMask[i] =  0;

		for (V3DLONG i = 0; i < filenames.size();i++)
		{
			filename = filenames[i];

			if (filename.size()>0)
			{
				neuron = readSWC_file(filename);
				for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
				{
					p_cur = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));

					if (x_min<0 ) p_cur->x -= x_min;
					if (y_min<0 ) p_cur->y -= y_min;
					if (z_min<0 ) p_cur->z -= z_min;
				}
				try
				{
					ImMark = new unsigned char [stacksz];
				}
				catch (...)
				{
					v3d_msg("Fail to allocate memory.\n");
					return;
				}

				for (V3DLONG i=0; i<stacksz; i++)
					 ImMark[i] = 0;

				ComputemaskImage(neuron, pImMask, ImMark, sx, sy, sz,2);

				if (ImMark) {delete []ImMark; ImMark=0;}
			}
			else
			{
				v3d_msg("You don't have any SWC file open in the main window.");
				return;
			}
		}
	}
//////////////////////////////////////////////////////

	// compute coordinate region
	Image4DSimple tmp;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want set the image size?"), QMessageBox::Yes, QMessageBox::No))
	{
		SetsizeDialog dialog(callback, parent);
		dialog.coord_x->setValue(sx);
		dialog.coord_y->setValue(sy);
		dialog.coord_z->setValue(sz);
        if (dialog.exec()!=QDialog::Accepted){
            nx=sx;ny=sy;nz=sz;
        }
		else
		{
			nx = dialog.coord_x->text().toLong();
			ny = dialog.coord_y->text().toLong();
			nz = dialog.coord_z->text().toLong();
			//printf("nx=%d ny=%d nz=%d\n ",nx,ny,nz);
		}

        //It is possible sx,sy,sz are larger than nx,ny,nz (originial image where swc is generated)
    }
    else
    {
        nx=sx;ny=sy;nz=sz;
    }

    unsigned char * pData = new unsigned char[nx*ny*nz];

    for (V3DLONG ii=0; ii<nx*ny*nz; ii++)
        pData[ii] = 0;

    if(nx>=sx && ny>=sy && nz>=sz)
     {
         for (V3DLONG k1 = 0; k1 < sz; k1++)
         {
             for(V3DLONG j1 = 0; j1 < sy; j1++)
             {
                for(V3DLONG i1 = 0; i1 < sx; i1++)
                {
                    pData[k1 * nx*ny + j1*nx + i1] = pImMask[k1*sx*sy + j1*sx +i1];

                }
            }
        }

     }

     else
     {
        v3d_msg("The input image size is smaller than the default size of swc file. You might only"
                " get part of the swc mask");
        //tmp.setData(pImMask, nx, ny, nz, 1, V3D_UINT8);
        for (V3DLONG k1 = 0; k1 < sz; k1++){
            for(V3DLONG j1 = 0; j1 < sy; j1++){
               for(V3DLONG i1 = 0; i1 < sx; i1++){
                   if ((i1>nx)||(j1>ny)||(k1>nz)) continue;
                   pData[k1 * nx*ny + j1*nx + i1] = pImMask[k1*sx*sy + j1*sx +i1];

               }
           }
       }

     }
    tmp.setData(pData, nx, ny, nz, 1, V3D_UINT8);
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &tmp);
	callback.setImageName(newwin, QString("Neuron_Mask_%1.tif").arg(filename));
	callback.updateImageWindow(newwin);

	//free space
}

void mrskimage_originalimage(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
	v3dhandleList win_list = callback.getImageWindowList();
	if(win_list.size()!=2)
	{
        QMessageBox::information(0, title, QObject::tr("need to open one mask image and one tif file"));
		return;
	}

//    for (int i=0;i<win_list.size();i++)
//    {
//        if(callback->getImageName(win_list[i]).contains("Neuron_Mask"))
//        {
//            Image4DSimple* image1 = callback.getImage(win_list[i]);
//        }
//       break;
//    }
    Image4DSimple* image1 = callback.getImage(win_list[0]);
	Image4DSimple* image2 = callback.getImage(win_list[1]);
	V3DLONG min,min1,max,max1,filter;
	min=max=min1=max1=filter= 0;
	if (!image1 && !image2)
	{
		QMessageBox::information(0, title, QObject::tr("No file is open."));
		return;
	}

	Image4DProxy<Image4DSimple> pSub(image1);
	unsigned char * pData1  = pSub.begin();
	V3DLONG sx1 = image1->getXDim();
    V3DLONG sy1 = image1->getYDim();
    V3DLONG sz1 = image1->getZDim();
	V3DLONG sc1= image1->getCDim();
	V3DLONG pagesz_sub1 = sx1*sy1*sz1;
	printf("sx=%d sy=%d sz=%d sc1=%d\n",sx1,sy1,sz1,sc1);

	Image4DProxy<Image4DSimple> pSub2(image2);
	unsigned char * pData2  = pSub2.begin();
	V3DLONG sx2 = image2->getXDim();
    V3DLONG sy2 = image2->getYDim();
    V3DLONG sz2 = image2->getZDim();
	V3DLONG sc2 = image2->getCDim();
	V3DLONG pagesz_sub2 = sx2*sy2*sz2;

	unsigned char * pData = new unsigned char[sx1*sy1*sz1];
	for (V3DLONG ii=0; ii<sx1*sy1*sz1; ii++)
		pData[ii] = 0;
	//	printf("sx2=%d sy2=%d sz2=%d sc2=%d\n",sx2,sy2,sz2,sc2);

	if (sx1!=sx2 || sy1!=sy2 || sz1!=sz2 )
	{
		v3d_msg(" the image size differ ");
		return;
	}
	for (V3DLONG k = 0; k < sz1; k++)
	{
		for(V3DLONG j = 0; j < sy1; j++)
		{
			for(V3DLONG i = 0; i < sx1; i++)
			{
				min =(min > pData1[k*sx1*sy1+j*sx1+i])? pData1[k*sx1*sy1 + j*sx1 +i] : min;
				max =(pData1[k*sx1*sy1+j*sx1+i] > max)? pData1[k*sx1*sy1 + j*sx1 +i] : max;
				min1 =(min > pData2[k*sx1*sy1+j*sx1+i])? pData2[k*sx1*sy1 + j*sx1 +i] : min;
				max1 =(pData2[k*sx1*sy1+j*sx1+i] > max)? pData2[k*sx1*sy1 + j*sx1 +i] : max;
			}
		}
	}

	filter=(max1 > max)? max : max1;

	for (V3DLONG k = 0; k < sz1; k++)
	{
		for(V3DLONG j = 0; j < sy1; j++)
		{
			for(V3DLONG i = 0; i < sx1; i++)
			{
				if (filter == max)
				{
					if (pData1[k*sx1*sy1 + j*sx1 +i] !=max)
					{
						pData2[k*sx1*sy1 + j*sx1 +i] = 0;
					}
					pData[k*sx1*sy1 + j*sx1 +i]	= pData2[k*sx1*sy1 + j*sx1 +i];
				}else
				{
					if (pData2[k*sx1*sy1 + j*sx1 +i] !=max1)
					{
						pData1[k*sx1*sy1 + j*sx1 +i] = 0;
					}
					pData[k*sx1*sy1 + j*sx1 +i]	= pData1[k*sx1*sy1 + j*sx1 +i];
				}
			}
		}
	}

	Image4DSimple tmp;
	tmp.setData(pData, sx1, sy1, sz1, 1, V3D_UINT8);

	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &tmp);
	callback.setImageName(newwin, "Filter Image");
	callback.updateImageWindow(newwin);
}
void SetsizeDialog::update()
{
	//get current data

	NX = coord_x->text().toLong()-1;
	NY = coord_y->text().toLong()-1;
	NZ = coord_z->text().toLong()-1;

}

bool swc_to_maskimage(V3DPluginCallback2 & callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
	vector<char*> * pinfiles = (input.size() >= 1) ? (vector<char*> *) input[0].p : 0;
	vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
	vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0; 

	vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
	vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
	vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
	if(infiles.size() != 1) return false;
	if(paras.size() != 0 && paras.size() != 3) return false;
	
	QString qs_input(infiles[0]);
	NeuronTree neuron = readSWC_file(qs_input);
	QString qs_output = outfiles.empty() ? qs_input + "_out.raw" : QString(outfiles[0]);
	
	double x_min,x_max,y_min,y_max,z_min,z_max;
	x_min=x_max=y_min=y_max=z_min=z_max=0;
	V3DLONG sx,sy,sz;
	unsigned char* pImMask = 0;
	unsigned char* ImMark = 0;
	V3DLONG h;
	V3DLONG d;
	V3DLONG nx,ny,nz;
	
	bool b_subtractMinFromAllNonnegatives = false;
    BoundNeuronCoordinates(neuron,
            b_subtractMinFromAllNonnegatives,
            x_min,
            x_max,
            y_min,
            y_max,
            z_min,
            z_max);

//	NeuronSWC * p_t = 0;
//	for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
//	{
//		p_t = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));
//		p_t->x=(p_t->x < 0)?(p_t->x - x_min):p_t->x;
//		p_t->y=(p_t->y < 0)?(p_t->y - y_min):p_t->y;
//		p_t->z=(p_t->z < 0)?(p_t->z - z_min):p_t->z;

//		//v3d_msg(QString("x %1 y %2 z %3 r %4\n").arg(p_cur->x).arg(p_cur->y).arg(p_cur->z).arg(p_cur->r),0);
//	}
	sx = (b_subtractMinFromAllNonnegatives || x_min<0) ? V3DLONG(ceil(x_max - x_min + 1)) : V3DLONG(ceil(x_max + 1));
	sy = (b_subtractMinFromAllNonnegatives || y_min<0) ? V3DLONG(ceil(y_max - y_min + 1)) : V3DLONG(ceil(y_max + 1));
	sz = (b_subtractMinFromAllNonnegatives || z_min<0) ? V3DLONG(ceil(z_max - z_min + 1)) : V3DLONG(ceil(z_max + 1));

	V3DLONG stacksz = sx*sy*sz;
	try
	{
		pImMask = new unsigned char [stacksz];
		ImMark = new unsigned char [stacksz];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory.\n");
		return false;
	}

	for (V3DLONG i=0; i<stacksz; i++)
		pImMask[i] = ImMark[i] = 0;

	ComputemaskImage(neuron, pImMask, ImMark, sx, sy, sz,1);
	
	// compute coordinate region
	if (paras.empty()) {nx = sx; ny = sy; nz = sz;}
	else {
		nx = atoi(paras[0]); 
		ny = atoi(paras[1]); 
		nz = atoi(paras[2]);
	}

	unsigned char * pData = new unsigned char[nx*ny*nz];

	for (V3DLONG ii=0; ii<nx*ny*nz; ii++)
		pData[ii] = 0;

    if(nx>=sx && ny>=sy && nz>=sz)
     {
         for (V3DLONG k1 = 0; k1 < sz; k1++)
         {
             for(V3DLONG j1 = 0; j1 < sy; j1++)
             {
                for(V3DLONG i1 = 0; i1 < sx; i1++)
                {
                    pData[k1 * nx*ny + j1*nx + i1] = pImMask[k1*sx*sy + j1*sx +i1];

                }
            }
        }

     }

     else
     {
        v3d_msg("The input image size is smaller than the default size of swc file. You might only"
                " get part of the swc mask");
        //tmp.setData(pImMask, nx, ny, nz, 1, V3D_UINT8);
        for (V3DLONG k1 = 0; k1 < sz; k1++){
            for(V3DLONG j1 = 0; j1 < sy; j1++){
               for(V3DLONG i1 = 0; i1 < sx; i1++){
                   if ((i1>nx)||(j1>ny)||(k1>nz)) continue;
                   pData[k1 * nx*ny + j1*nx + i1] = pImMask[k1*sx*sy + j1*sx +i1];

               }
           }
       }

     }
	V3DLONG siz[4];
	siz[0] = nx; siz[1] = ny; siz[2] = nz; siz[3] = 1;
    simple_saveimage_wrapper(callback, qPrintable(qs_output), pData, siz, V3D_UINT8);

    if (pData) {delete []pData; pData=0;}

	return true;
}

void swc_to_maskimage_toolbox(const V3DPluginArgList & input, V3DPluginCallback2 & callback, QWidget * parent)
{
	vaa3d_neurontoolbox_paras * paras = (vaa3d_neurontoolbox_paras *) (input.at(0).p);
	NeuronTree neuron = paras->nt;

	NeuronSWC * p_t = 0;
	double x_min,x_max,y_min,y_max,z_min,z_max;
	x_min=x_max=y_min=y_max=z_min=z_max=0;
	V3DLONG sx,sy,sz;
	unsigned char* pImMask = 0;
	unsigned char* ImMark = 0;
	V3DLONG h;
	V3DLONG d;
	V3DLONG nx,ny,nz;

	bool b_subtractMinFromAllNonnegatives = false;
	BoundNeuronCoordinates(neuron,
			b_subtractMinFromAllNonnegatives,
			x_min,
			x_max,
			y_min,
			y_max,
			z_min,
			z_max);

//	for (V3DLONG ii=0; ii<neuron.listNeuron.size(); ii++)
//	{
//		p_t = (NeuronSWC *)(&(neuron.listNeuron.at(ii)));
//		p_t->x=(p_t->x < 0)?(p_t->x - x_min):p_t->x;
//		p_t->y=(p_t->y < 0)?(p_t->y - y_min):p_t->y;
//		p_t->z=(p_t->z < 0)?(p_t->z - z_min):p_t->z;

//		//v3d_msg(QString("x %1 y %2 z %3 r %4\n").arg(p_cur->x).arg(p_cur->y).arg(p_cur->z).arg(p_cur->r),0);
//	}
	sx = (b_subtractMinFromAllNonnegatives || x_min<0) ? V3DLONG(ceil(x_max - x_min + 1)) : V3DLONG(ceil(x_max + 1));
	sy = (b_subtractMinFromAllNonnegatives || y_min<0) ? V3DLONG(ceil(y_max - y_min + 1)) : V3DLONG(ceil(y_max + 1));
	sz = (b_subtractMinFromAllNonnegatives || z_min<0) ? V3DLONG(ceil(z_max - z_min + 1)) : V3DLONG(ceil(z_max + 1));

	V3DLONG stacksz = sx*sy*sz;
	try
	{
		pImMask = new unsigned char [stacksz];
		ImMark = new unsigned char [stacksz];
	}
	catch (...)
	{
		v3d_msg("Fail to allocate memory.\n");
		return;
	}

	for (V3DLONG i=0; i<stacksz; i++)
		pImMask[i] = ImMark[i] = 0;

	ComputemaskImage(neuron, pImMask, ImMark, sx, sy, sz,1);


	// compute coordinate region
	Image4DSimple tmp;
	if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want set the image size?"), QMessageBox::Yes, QMessageBox::No))
    {
		SetsizeDialog dialog(callback, parent);
		dialog.coord_x->setValue(sx);
		dialog.coord_y->setValue(sy);
		dialog.coord_z->setValue(sz);
        if (dialog.exec()!=QDialog::Accepted){
            nx=sx;ny=sy;nz=sz;
            return;
        }
		else
		{
			nx = dialog.coord_x->text().toLong();
			ny = dialog.coord_y->text().toLong();
			nz = dialog.coord_z->text().toLong();
			//printf("nx=%d ny=%d nz=%d\n ",nx,ny,nz);
		}
     }
     else{
        nx=sx;ny=sy;nz=sz;
     }

    unsigned char * pData = new unsigned char[nx*ny*nz];
    for (V3DLONG ii=0; ii<nx*ny*nz; ii++)
        pData[ii] = 0;

    if(nx>=sx && ny>=sy && nz>=sz)
     {
         for (V3DLONG k1 = 0; k1 < sz; k1++)
         {
             for(V3DLONG j1 = 0; j1 < sy; j1++)
             {
                for(V3DLONG i1 = 0; i1 < sx; i1++)
                {
                    pData[k1 * nx*ny + j1*nx + i1] = pImMask[k1*sx*sy + j1*sx +i1];

                }
            }
        }

     }

     else
     {
        v3d_msg("The input image size is smaller than the default size of swc file. You might only"
                " get part of the swc mask");
        //tmp.setData(pImMask, nx, ny, nz, 1, V3D_UINT8);
        for (V3DLONG k1 = 0; k1 < sz; k1++){
            for(V3DLONG j1 = 0; j1 < sy; j1++){
               for(V3DLONG i1 = 0; i1 < sx; i1++){
                   if ((i1>nx)||(j1>ny)||(k1>nz)) continue;
                   pData[k1 * nx*ny + j1*nx + i1] = pImMask[k1*sx*sy + j1*sx +i1];

               }
           }
       }

     }
    tmp.setData(pData, nx, ny, nz, 1, V3D_UINT8);
	v3dhandle newwin = callback.newImageWindow();
	callback.setImage(newwin, &tmp);
	callback.setImageName(newwin, QString("Neuron_Mask_%1.tif").arg(neuron.file));
	callback.updateImageWindow(newwin);
}

void printHelp()
{
	printf("\nswc to mask image using sphere unit\n");
	printf("Usage v3d -x swc_to_maskimage_sphere_unit -f swc_to_maskimage -i <intput.swc> [-p <sz0> <sz1> <sz2>] [-o <output_image.raw>]\n");
}
