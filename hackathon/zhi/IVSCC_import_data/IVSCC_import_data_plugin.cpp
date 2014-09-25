/* IVSCC_import_data_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-08-13 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "IVSCC_import_data_plugin.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"

using namespace std;

// Open a series of inputs
QStringList importSeriesFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.tif"<<"*.raw"<<"*.v3draw"<<"*.lsm"
            <<"*.TIF"<<"*.RAW"<<"*.V3DRAW"<<"*.LSM";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}


Q_EXPORT_PLUGIN2(IVSCC_import_data, IVSCC_import_data);
 
QStringList IVSCC_import_data::menulist() const
{
	return QStringList() 
		<<tr("about");
}

QStringList IVSCC_import_data::funclist() const
{
	return QStringList()
        <<tr("import")
		<<tr("help");
}

void IVSCC_import_data::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("about"))
	{
        v3d_msg(tr("This is a plugin only worked using command line to import 2D image sequences in IVSCC pipeline. "
			"Developed by Zhi Zhou, 2014-08-13"));
	}
}

bool IVSCC_import_data::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("import"))
	{
        cout<<"Welcome to IVSCC import data plugin"<<endl;
        if (output.size() != 1) return false;
        char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
        char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);

        QString m_InputfolderName(inimg_file);

        QStringList imgList = importSeriesFileList_addnumbersort(m_InputfolderName);

        Y_VIM<REAL, V3DLONG, indexed_t<V3DLONG, REAL>, LUT<V3DLONG> > vim;

        V3DLONG count=0;
        foreach (QString img_str, imgList)
        {
            V3DLONG offset[3];
            offset[0]=0; offset[1]=0; offset[2]=0;

            indexed_t<V3DLONG, REAL> idx_t(offset);

            idx_t.n = count;
            idx_t.ref_n = 0; // init with default values
            idx_t.fn_image = img_str.toStdString();
            idx_t.score = 0;

            vim.tilesList.push_back(idx_t);
            count++;
        }

        int NTILES  = vim.tilesList.size();

        unsigned char * data1d_1st = 0;
        V3DLONG in_sz[4];
        int datatype;

        if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(0).fn_image.c_str()), data1d_1st, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(0).fn_image.c_str());
            return false;
        }
        if(data1d_1st) {delete []data1d_1st; data1d_1st = 0;}


        V3DLONG sz[4];
        sz[0] = in_sz[0];
        sz[1] = in_sz[1];
        sz[2] = NTILES;
        sz[3] = 1;//in_sz[3];

        unsigned char *im_imported = 0;
        V3DLONG pagesz = sz[0]* sz[1]* sz[2]*sz[3];
        try {im_imported = new unsigned char [pagesz];}
        catch(...)  {v3d_msg("cannot allocate memory for im_imported."); return false;}

        V3DLONG pagesz_one = in_sz[0]*in_sz[1]*in_sz[2]*1;//in_sz[3];

        V3DLONG i = 0;
        for(V3DLONG ii = 0; ii < NTILES; ii++)
        {
            unsigned char * data1d = 0;
            V3DLONG in_sz[4];
            int datatype;

            if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(ii).fn_image.c_str()), data1d, in_sz, datatype))
            {
                fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                return false;
            }
            for(V3DLONG j = 0; j < pagesz_one; j++)
            {
                 im_imported[i] = 255 - data1d[j];
                 i++;
            }
            if(data1d) {delete []data1d; data1d = 0;}
        }

        simple_saveimage_wrapper(callback, outimg_file,(unsigned char *)im_imported,sz,datatype);
        if(im_imported) {delete []im_imported; im_imported = 0;}
        return true;
	}
    else if (func_name == tr("help"))
    {
        cout<<"Usage : v3d -x dllname -f import -i <inimg_folder> -o <outimg_file>"<<endl;
        cout<<endl;
    }
    else return false;;

	return true;
}

