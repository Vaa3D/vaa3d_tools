/* recenterimageplugin.cpp
 * 2009-08-14: created by Yang Yu
 * 2010-11-23: supporting all kinds of datatypes, changed by Yang Yu
 * 2011-09-16: change this plugin interface from 1.0 to 2.1 by Yang Yu
 * 2011-09-22: add dofunc function for headless command line using by Yang Yu
 * 2012-04-13: add help function by Jianlong Zhou
 */

#include <QtGui>

#include <math.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

#include "basic_surf_objs.h"
//#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

//#include "mg_utilities.h"
//#include "mg_image_lib.h"

#include "basic_landmark.h"
//#include "basic_4dimage.h"

#include "recenterimageplugin.h"

using namespace std;

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(recenterimage, ReCenterImagePlugin)

template <class Tidx, class Tdata>
void recentering(Tdata *&p, Tdata *data, Tidx nx, Tidx ny, Tidx nz, Tidx ox, Tidx oy, Tidx oz, Tidx ncolor);

//dofunc
QStringList ReCenterImagePlugin::funclist() const
{
    return QStringList() << "iRecenter"
                         << "help";
}

bool ReCenterImagePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & v3d, QWidget * parent)
{
     if(func_name == tr("help"))
	{
		cout<<"Usage: v3d -x recenterimage -f iRecenter -i <input_image> -o <output_image> -p \"#s <save_blending_result zero(false)/nonzero(true)> #x <dimx> #y <dimy> #z <dimz>\" "<<endl;
		cout<<endl;
          cout<<"e.g. v3d -x recenterimage -f iRecenter -i input.raw -o output.raw -p 0 64 64 64"<<endl;
		cout<<endl;
		return true;
	}

    //
    if(input.size()<1) return false; // no inputs

    vector<char*> * infilelist = (vector<char*> *)(input.at(0).p);
    vector<char*> * paralist;
    vector<char*> * outfilelist;
    if(infilelist->empty())
    {
        //print Help info
        printf("\nUsage: v3d -x recenterimage -f iRecenter -i <input_image> -o <output_image> -p \"#s <save_blending_result zero(false)/nonzero(true)> #x <dimx> #y <dimy> #z <dimz> \"\n");

        return true;
    }

    char * infile = infilelist->at(0); // input images
    char * paras = NULL; // parameters
    char * outfile = NULL; // outputs

    if(output.size()>0) { outfilelist = (vector<char*> *)(output.at(0).p); outfile = outfilelist->at(0);}  // specify output
    if(input.size()>1) { paralist = (vector<char*> *)(input.at(1).p); paras =  paralist->at(0);} // parameters

    bool b_saveimage = true; // save the blended image by default
    V3DLONG ndimx=1, ndimy=1, ndimz=1;

    if(paras)
    {
        int argc = 0;
        int len = strlen(paras);
        int posb[200];
        char * myparas = new char[len];
        strcpy(myparas, paras);
        for(int i = 0; i < len; i++)
        {
            if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
            {
                posb[argc++] = i;
            }
            else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') && (myparas[i] != ' ' && myparas[i] != '\t'))
            {
                posb[argc++] = i;
            }
        }
        char ** argv = new char* [argc];
        for(int i = 0; i < argc; i++)
        {
            argv[i] = myparas + posb[i];
        }
        for(int i = 0; i < len; i++)
        {
            if(myparas[i]==' ' || myparas[i]=='\t')
                myparas[i]='\0';
        }

        char* key;
        for(int i=0; i<argc; i++)
        {
            if(i+1 != argc) // check that we haven't finished parsing yet
            {
                key = argv[i];

                qDebug()<<">>key ..."<<key;

                if (*key == '#')
                {
                    while(*++key)
                    {
                        if (!strcmp(key, "s"))
                        {
                            b_saveimage = (atoi( argv[i+1] ))?true:false;
                            i++;
                        }
                        else if (!strcmp(key, "x"))
                        {
                            ndimx = atoi( argv[i+1] );
                            i++;
                        }
                        else if (!strcmp(key, "y"))
                        {
                            ndimy = atoi( argv[i+1] );
                            i++;
                        }
                        else if (!strcmp(key, "z"))
                        {
                            ndimz = atoi( argv[i+1] );
                            i++;
                        }
                        else
                        {
                            cout<<"parsing ..."<<key<<" "<<i<<" "<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                            return false;
                        }

                    }
                }
                else
                {
                    cout<<"parsing ..."<<key<<" "<<i<<" "<<"Unknown command. Type 'v3d -x plugin_name -f function_name' for usage"<<endl;
                    return false;
                }

            }
        }
    }

    QString outputImageName;

    if(!outfile)
        outputImageName = QString(infile).append("_recentered.v3draw");
    else
        outputImageName = QString(outfile);

    if(QFileInfo(outputImageName).suffix().toUpper() != "V3DRAW")
    {
        outputImageName.append(".v3draw"); // force to save as .v3draw file
    }

    //
    int datatype=0;
    V3DLONG sz_input[4];
    unsigned char* input1d = 0;

    if (simple_loadimage_wrapper(v3d, const_cast<char *>(infile), input1d, sz_input, datatype)!=true)
    {
        printf("Error happens in reading the subject file [%s]. Exit. \n", infile);
        return false;
    }
    V3DLONG sx = sz_input[0], sy = sz_input[1], sz = sz_input[2], sc = sz_input[3];

    V3DLONG sz_output[4];
    sz_output[0] = ndimx;
    sz_output[1] = ndimy;
    sz_output[2] = ndimz;
    sz_output[3] = sc;

    //
    if(datatype == 1)
    {
        unsigned char *pRecenteredImage = NULL;
        recentering<V3DLONG, unsigned char>( pRecenteredImage, (unsigned char*)input1d, ndimx, ndimy, ndimz, sx, sy, sz, sc);
        //output
        if(b_saveimage)
        {
            //save
            if (simple_saveimage_wrapper(v3d, outputImageName.toStdString().c_str(), ( unsigned char *)pRecenteredImage, sz_output, 1)!=true)
            {
                printf("Error happens in file writing. Exit. \n");
                return false;
            }

            //de-alloc
            if(pRecenteredImage) {delete []pRecenteredImage; pRecenteredImage=NULL;}
        }
        else
        {
            V3DPluginArgItem arg;

            arg.type = "data"; arg.p = (void *)(pRecenteredImage); output << arg;

            V3DLONG metaImg[5]; // xyzc datatype
            metaImg[0] = sz_output[0];
            metaImg[1] = sz_output[1];
            metaImg[2] = sz_output[2];
            metaImg[3] = sz_output[3];
            metaImg[4] = datatype;

            arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
        }
    }
    else if(datatype == 2)
    {
        unsigned short *pRecenteredImage = NULL;
        recentering<V3DLONG, unsigned short>( pRecenteredImage, (unsigned short*)input1d, ndimx, ndimy, ndimz, sx, sy, sz, sc);

        // output
        if(b_saveimage)
        {
            //save
            if (simple_saveimage_wrapper(v3d, outputImageName.toStdString().c_str(), ( unsigned char *)pRecenteredImage, sz_output, 2)!=true)
            {
                printf("Error happens in file writing. Exit. \n");
                return false;
            }

            //de-alloc
            if(pRecenteredImage) {delete []pRecenteredImage; pRecenteredImage=NULL;}
        }
        else
        {
            V3DPluginArgItem arg;

            arg.type = "data"; arg.p = (void *)(pRecenteredImage); output << arg;

            V3DLONG metaImg[5]; // xyzc datatype
            metaImg[0] = sz_output[0];
            metaImg[1] = sz_output[1];
            metaImg[2] = sz_output[2];
            metaImg[3] = sz_output[3];
            metaImg[4] = datatype;

            arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
        }
    }
    else if(datatype == 4)
    {
        float *pRecenteredImage = NULL;
        recentering<V3DLONG, float>( pRecenteredImage, (float*)input1d, ndimx, ndimy, ndimz, sx, sy, sz, sc);

        // output
        if(b_saveimage)
        {
            //save
            if (simple_saveimage_wrapper(v3d, outputImageName.toStdString().c_str(), ( unsigned char *)pRecenteredImage, sz_output, 4)!=true)
            {
                printf("Error happens in file writing. Exit. \n");
                return false;
            }

            //de-alloc
            if(pRecenteredImage) {delete []pRecenteredImage; pRecenteredImage=NULL;}
        }
        else
        {
            V3DPluginArgItem arg;

            arg.type = "data"; arg.p = (void *)(pRecenteredImage); output << arg;

            V3DLONG metaImg[5]; // xyzc datatype
            metaImg[0] = sz_output[0];
            metaImg[1] = sz_output[1];
            metaImg[2] = sz_output[2];
            metaImg[3] = sz_output[3];
            metaImg[4] = datatype;

            arg.type = "metaImage"; arg.p = (void *)(metaImg); output << arg;
        }
    }
    else
    {
        return false;
    }

    //
    return true;
}

//domenu
QStringList ReCenterImagePlugin::menulist() const
{
    return QStringList() << tr("ReCenterImage")
						 << tr("About");
}

void ReCenterImagePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    // interactively call ReCenterImage plugin for current focused image opened by V3D
    if (menu_name == tr("ReCenterImage"))
    {
        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(parent, "RecenterImage", "You don't have any image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (! p4DImage) return;

        Image4DProxy<Image4DSimple> p4DProxy(p4DImage);

        //void* data1d = p4DProxy.begin();
        V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

        V3DLONG N = p4DImage->getXDim();
        V3DLONG M = p4DImage->getYDim();
        V3DLONG P = p4DImage->getZDim();
        V3DLONG sc = p4DImage->getCDim();

        int datatype = p4DImage->getDatatype();

        bool ok;
        V3DLONG ndimx = QInputDialog::getInteger(parent, tr("Dimension x"),
             tr("Enter dimx:"),
             N, 0, 10000, 1, &ok);

        V3DLONG ndimy = QInputDialog::getInteger(parent, tr("Dimension y"),
             tr("Enter dimy:"),
             M, 0, 10000, 1, &ok);

        V3DLONG ndimz = QInputDialog::getInteger(parent, tr("Dimension z"),
             tr("Enter dimz:"),
             P, 0, 10000, 1, &ok);

        qDebug("dimx %ld dimy %ld dimz %ld \n", ndimx, ndimy, ndimz);

        V3DLONG ntotalpxls = sc*ndimx*ndimy*ndimz;

        void *pRecenteredImage = NULL;
        if (ok)
        {
			// For different datatype
			if(datatype == 1)
			{
				unsigned char *pImage = NULL;
				recentering<V3DLONG, unsigned char>( pImage, (unsigned char*)p4DProxy.begin(), ndimx, ndimy, ndimz, N, M, P, sc);
                pRecenteredImage=(void *)pImage;
			}
			else if(datatype == 2)
			{
				unsigned short *pImage = NULL;
				recentering<V3DLONG, unsigned short>( pImage, (unsigned short*)p4DProxy.begin(), ndimx, ndimy, ndimz, N, M, P, sc);
                pRecenteredImage=(void *)pImage;
			}
			else if(datatype == 3)
			{
				float *pImage = NULL;
				recentering<V3DLONG, float>( pImage, (float*)p4DProxy.begin(), ndimx, ndimy, ndimz, N, M, P, sc);
                pRecenteredImage=(void *)pImage;
			}
			else
			{
				return;
			}

            Image4DSimple p4DImageRec;
            p4DImageRec.setData((unsigned char*)pRecenteredImage, ndimx, ndimy, ndimz, sc, p4DImage->getDatatype()); // update data in current window

            v3dhandle newwin;
            if(QMessageBox::Yes == QMessageBox::question (0, "", QString("Do you want to use the existing window?"), QMessageBox::Yes, QMessageBox::No))
                newwin = callback.currentImageWindow();
            else
                newwin = callback.newImageWindow();

            callback.setImage(newwin, &p4DImageRec);
            callback.setImageName(newwin, QString("recentered image"));
            callback.updateImageWindow(newwin);

		}
	}
	else if (menu_name == tr("About"))
	{
		QMessageBox::information(parent, "Version info", "Plugin Recenter version 1.0 developed by Yang Yu (Peng Lab, Janelia Farm Research Campus, HHMI).");
	}
	else
    {
		return;
    }

}

template <class Tidx, class Tdata>
void recentering(Tdata *&p, Tdata *data, Tidx nx, Tidx ny, Tidx nz, Tidx ox, Tidx oy, Tidx oz, Tidx ncolor)
{

	if(p) {delete []p; p=NULL;}
	else
	{
		Tidx nplxs = nx*ny*nz*ncolor;
		Tidx pagesz = ox*oy*oz;

		//Initial New image
		try
		{
			p = new Tdata [nplxs];
			for(Tidx i=0; i<nplxs; i++)
			{
				p[i] = 0;
			}
		}
		catch(...)
		{
			printf("Error allocating memory for new image!\n");
			return;
		}

		//recenter
		Tidx centerx = ox/2;
		Tidx centery = oy/2;
		Tidx centerz = oz/2;

		Tidx ncenterx = nx/2;
		Tidx ncentery = ny/2;
		Tidx ncenterz = nz/2;

		//shift
		Tidx leftx = fabs(ncenterx-centerx);
		Tidx rightx = fabs(ox + leftx);
		if(ox>nx)
		{
			rightx = fabs(nx + leftx);
		}

		Tidx lefty = fabs(ncentery - centery);
		Tidx righty = fabs(oy + lefty);
		if(oy>ny)
		{
			righty = fabs(ny + lefty);
		}

		Tidx leftz = fabs(ncenterz - centerz);
		Tidx rightz = fabs(oz + leftz);
		if(oz>nz)
		{
			rightz = fabs(nz + leftz);
		}

		//simple 8 cases
		if(nx<=ox)
		{
			if(ny<=oy)
			{
				if(nz<=oz)
				{
					//case 1
					qDebug()<< "case 1 ...";

					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk =  offsetc + k*ox*oy;
							Tidx offsetnk = offsetnc + (k-leftz)*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + j*ox;
								Tidx offsetnj = offsetnk + (j-lefty)*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + (i-leftx)] = data[offsetj + i];
								}
							}
						}
					}
				}
				else
				{
					//case 2
					qDebug()<< "case 2 ...";

					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + (k-leftz)*ox*oy;
							Tidx offsetnk = offsetnc + k*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + j*ox;
								Tidx offsetnj = offsetnk + (j-lefty)*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + (i-leftx)] = data[offsetj + i];
								}
							}
						}
					}

				}
			}
			else
			{
				if(nz<=oz)
				{
					//case 3
					qDebug()<< "case 3 ...";

					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + k*ox*oy;
							Tidx offsetnk = offsetnc + (k-leftz)*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + (j-lefty)*ox;
								Tidx offsetnj = offsetnk + j*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + (i-leftx)] = data[offsetj + i];
								}
							}
						}
					}

				}
				else
				{
					//case 4
					qDebug()<< "case 4 ...";

					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + (k-leftz)*ox*oy;
							Tidx offsetnk = offsetnc + k*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + (j-lefty)*ox;
								Tidx offsetnj = offsetnk + j*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + (i-leftx)] = data[offsetj + i];
								}
							}
						}
					}

				}
			}
		}
		else
		{
			if(ny<=oy)
			{
				if(nz<=oz)
				{
					//case 5
					qDebug()<< "case 5 ...";

					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + k*ox*oy;
							Tidx offsetnk = offsetnc + (k-leftz)*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + j*ox;
								Tidx offsetnj = offsetnk + (j-lefty)*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + i] = data[offsetj + (i-leftx)];
								}
							}
						}
					}

				}
				else
				{
					//case 6
					qDebug()<< "case 6 ...";

					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + (k-leftz)*ox*oy;
							Tidx offsetnk = offsetnc + k*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + j*ox;
								Tidx offsetnj = offsetnk + (j-lefty)*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + i] = data[offsetj + (i-leftx)];
								}
							}
						}
					}

				}
			}
			else
			{
				if(nz<=oz)
				{
					//case 7
					qDebug()<< "case 7 ...";

					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + k*ox*oy;
							Tidx offsetnk = offsetnc + (k-leftz)*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + (j-lefty)*ox;
								Tidx offsetnj = offsetnk + j*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + i] = data[offsetj + (i-leftx)];
								}
							}
						}
					}

				}
				else
				{
					//case 8
					qDebug()<< "case 8 ...";

					for(Tidx c=0; c<ncolor; c++)
					{
						Tidx offsetc = c*pagesz;
						Tidx offsetnc = c*nx*ny*nz;
						for(Tidx k=leftz; k<rightz; k++)
						{
							Tidx offsetk = offsetc + (k-leftz)*ox*oy;
							Tidx offsetnk = offsetnc + k*nx*ny;
							for(Tidx j=lefty; j<righty; j++)
							{
								Tidx offsetj = offsetk + (j-lefty)*ox;
								Tidx offsetnj = offsetnk + j*nx;
								for(Tidx i=leftx; i<rightx; i++)
								{
									p[offsetnj + i] = data[offsetj + (i-leftx)];
								}
							}
						}
					}

				}
			}
		}

	}

}

