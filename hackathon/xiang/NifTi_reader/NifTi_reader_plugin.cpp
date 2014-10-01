/* NifTi_reader_plugin.cpp
 * Reader for NifTi format file
 * 2014-09-29 : by Xiang Li
 */
 
#include "v3d_message.h"
#include <vector>
#include "NifTi_reader_plugin.h"
#include <iostream>
#include <sstream>
#include <string>
#include <nifti1_io.h>
using namespace std;
Q_EXPORT_PLUGIN2(NifTi_reader, NifTi_reader);
 

QStringList NifTi_reader::menulist() const
{
	return QStringList() 
		<<tr("Import .nii file...")
		<<tr("about");
}

QStringList NifTi_reader::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void NifTi_reader::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Import .nii file..."))
	{
		//Show open file dialog;
        QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Choose .nii file to import"),
                                                          QDir::currentPath(),
                                                          QObject::tr("NIFTI File (*.*)"));
        if(m_FileName.isEmpty())
        {
             return;
        }


		QByteArray QByteA = m_FileName.toLocal8Bit();
		char* string_FileName = QByteA.data();

		/* Open the file and read it in, but do not load the data;  */
		nifti_image *nimage_Input = nifti_image_read(string_FileName, 0);	

		/* Check whether the file is valid; */
		if( ((nimage_Input) == NULL) || ((nimage_Input)->iname == NULL) || ((nimage_Input)->nbyper <= 0) || ((nimage_Input)->nvox <= 0))
		{
			v3d_msg("Cannot locate the desired file or Bad header info!");
			return;
		}
		else
		{
			//v3d_msg("File opened correctly!");
			nifti_image_load(nimage_Input);
		}
		
		stringstream ss; //stringstream used to do the conversion from any type to string;

		//Get total bytes;
		int ntot=(int)(nimage_Input->nbyper)*(int)(nimage_Input->nvox);
		/* For debugging only
		ss <<"Total bytes in the image: "<<ntot<<";";
		v3d_msg(ss.str().c_str());
		ss.str(std::string());
		*/

		//Get dimensions;
		int nX = nimage_Input->nx;
		int nY = nimage_Input->ny;
		int nZ = nimage_Input->nz;
		/* For debugging only
		ss <<"Dimensions of the image, X: "<<nX<<", Y:"<<nY<<", Z:"<<nZ<<";";
		v3d_msg(ss.str().c_str());
		ss.str(std::string());
		*/

		//Get datatype of the imported NIFTI file (important!);
		int dataType = nimage_Input->datatype;
		/* For debugging only */
		ss <<"Datatype: "<<nifti_datatype_string(dataType)<<";";
		v3d_msg(ss.str().c_str());
		ss.str(std::string());
		/* */

		
		if (dataType == DT_INT16)
		{
			int* vct_data = (int *)calloc(1,ntot);
			vct_data = (int *)nimage_Input->data;
			Image4DSimple Image4D_Main;
			Image4D_Main.setData((unsigned char*)vct_data, nX, nY, nZ, 1, V3D_UINT16);
			v3dhandle newwin = callback.newImageWindow();
			callback.setImage(newwin, &Image4D_Main);
			callback.updateImageWindow(newwin);
		}
		else if (dataType == DT_DOUBLE)
		{
			double* vct_data = (double *)calloc(1,ntot);
			vct_data = (double *)nimage_Input->data;
			Image4DSimple Image4D_Main;
			Image4D_Main.setData((unsigned char*)vct_data, nX, nY, nZ, 1, V3D_FLOAT32);
			v3dhandle newwin = callback.newImageWindow();
			callback.setImage(newwin, &Image4D_Main);
			callback.updateImageWindow(newwin);
		}
		else
		{
			float* vct_data = (float *)calloc(1,ntot);
			vct_data = (float *)nimage_Input->data;
			Image4DSimple Image4D_Main;
			Image4D_Main.setData((unsigned char*)vct_data, nX, nY, nZ, 1, V3D_FLOAT32);
			v3dhandle newwin = callback.newImageWindow();
			callback.setImage(newwin, &Image4D_Main);
			callback.updateImageWindow(newwin);
		}
		
		


		//nifti_image_unload(nimage_Input);
		//nifti_image_free(nimage_Input);	
	}
	else
	{
		v3d_msg(tr("Reader for NifTi format file. "
			"Developed by Xiang Li, 2014-09-29"));
	}
}

bool NifTi_reader::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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