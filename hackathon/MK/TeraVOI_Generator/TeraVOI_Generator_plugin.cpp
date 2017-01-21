/* TeraVOI_Generator_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-1-20 : by MK
 */

#include "../../zhi/IVSCC_sort_swc/openSWCDialog.h"
#include "v3d_message.h"
#include <vector>
#include "TeraVOI_Generator_plugin.h"
#include "VOI_func.h"
#include "../../../released_plugins/v3d_plugins/gsdt/common_dialog.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"

using namespace std;
Q_EXPORT_PLUGIN2(TeraVOI_Generator, TeraVOI_Generator);

QStringList TeraVOI_Generator::menulist() const
{
    return QStringList()
        <<tr("Generate VOI from image series")
        <<tr("about");
}

QStringList TeraVOI_Generator::funclist() const
{
    return QStringList()
        <<tr("func1")
        <<tr("help");
}

void TeraVOI_Generator::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Generate VOI from image series"))
    {
        ///////////////////////////////////////////// Specify the path to image files and put file names into a QString
        QString m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the folder where images are stored"),
                                                                      QDir::currentPath(),
                                                                      QFileDialog::ShowDirsOnly);
        //cout << m_InputfolderName.toStdString() << endl;
        QString outputfolder = m_InputfolderName + QString("/output");
        QDir().mkdir(outputfolder);

        QStringList imgList = importSeriesFileList_addnumbersort(m_InputfolderName);
        foreach (QString qs, imgList)  qDebug() << qs;
        ///////////////////////////////////// END of [Specify the path to image files and put file names into a QString]

        /////////////////////////////////////////// Load SWC file and create the neuron tree
        QString fileOpenName;
        OpenSWCDialog * openDlg = new OpenSWCDialog(0, &callback);
        if (!openDlg->exec())
            return;
        fileOpenName = openDlg->file_name;
        NeuronTree InputSWC = openDlg -> nt;
        /////////////////////////////////// END of [Load SWC file and create the neuron tree]

        ///////////////////////////////////// Volume of interest size specification
        vector<string> items;
        items.push_back("Specify x length of VOI: ");
        items.push_back("Specify y length of VOI: ");
        items.push_back("Specify z length of VOI: ");
        CommonDialog dialog(items);
        dialog.setWindowTitle("Volume of interest specification");
        if (dialog.exec()!=QDialog::Accepted)
            return;

        int xLength = NULL, yLength = NULL, zLength = NULL;
        dialog.get_num("Specify x length of VOI: ", xLength);
        dialog.get_num("Specify y length of VOI: ", yLength);
        dialog.get_num("Specify z length of VOI: ", zLength);
        if (xLength == NULL || yLength == NULL || zLength == NULL)
        {
            v3d_msg("Parameter(s) not specified. Failed to generate marker file.");
            return;
        }
        int xRadius = (xLength + 1)/2;
        int yRadius = (yLength + 1)/2;
        int zRadius = (zLength + 1)/2;
        //////////////////////////// END of [Volume of interest size specification]

        ////////////////////////// Aqcure node coords, collect and crop images, form 3D cubes, save files to the output folder
        QList<NeuronSWC> list = InputSWC.listNeuron;

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
        int n_slice = vim.tilesList.size();

        unsigned char* ImgPtr = 0;
        V3DLONG in_sz[3];
        int datatype;
        if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(0).fn_image.c_str()), ImgPtr, in_sz, datatype))
        {
            fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(0).fn_image.c_str());
            return;
        }

        V3DLONG img_demen[3];
        img_demen[0] = in_sz[0];
        img_demen[1] = in_sz[1];
        img_demen[2] = n_slice;  // => Get images demention information
        //cout << sz[0] << "\n" << sz[1] << "\n" << sz[2] << "\n" << endl;

        int x_coord, y_coord, z_coord;
        int xlb, xhb, ylb, yhb, zlb, zhb;
        for (int ii=0; ii<list.size(); ii++)
        {
            x_coord = int(InputSWC.listNeuron.at(ii).x);
            y_coord = int(InputSWC.listNeuron.at(ii).y);
            z_coord = int(InputSWC.listNeuron.at(ii).z);
            if (x_coord - xRadius - 1 <= 0) xlb = 0;
            else if (x_coord + xRadius - 1 >= img_demen[0]) xhb = img_demen[0] - 1;
            else
            {
                xlb = x_coord - xRadius - 1;
                xhb = x_coord + xRadius + 1;
            }
            if (y_coord - yRadius - 1 <= 0) ylb = 0;
            else if (y_coord + yRadius - 1 >= img_demen[1]) yhb = img_demen[1] - 1;
            else
            {
                ylb = y_coord - yRadius - 1;
                yhb = y_coord + yRadius + 1;
            }
            if (z_coord - zRadius - 1 <= 0) zlb = 0;
            else if (z_coord + zRadius - 1 >= img_demen[2]) zhb = img_demen[2] - 1;
            else
            {
                zlb = z_coord - zRadius - 1;
                zhb = z_coord + zRadius + 1;
            }
            NeuronTree prunnedTree = cropSWCfile3D(InputSWC, xlb, xhb, ylb, yhb, zlb, zhb, -1);
            QString outimg_fileSWC = outputfolder + QString("/x%1_y%2_z%3.swc").arg(x_coord).arg(y_coord).arg(z_coord);
            writeSWC_file(outimg_fileSWC, prunnedTree); // Save cropped neuron tree in separate SWC files.

            unsigned char* VOIPtr;
            V3DLONG VOIsz = (xhb-xlb+1) * (yhb-ylb+1) * (zhb-zlb+1);
            V3DLONG VOIxyz[4];
            VOIxyz[0] = xhb-xlb+1;
            VOIxyz[1] = yhb-ylb+1;
            VOIxyz[2] = zhb-zlb+1;
            VOIxyz[3] = 1;
            try {VOIPtr = new unsigned char [VOIsz];}
            catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return;}
            Piling2Dimages(ImgPtr, VOIPtr, xlb, xhb, ylb, yhb, zlb, zhb, xLength, yLength, zLength);
            ImagePixelType pixeltype;
            switch (datatype)
            {
            case 1: pixeltype = V3D_UINT8; break;
            case 2: pixeltype = V3D_UINT16; break;
            case 4: pixeltype = V3D_FLOAT32;break;
            default: v3d_msg("Invalid data type. Do nothing."); return;
            }

            //Image4DSimple * new4DImage = new Image4DSimple();
            //new4DImage->setData((unsigned char *)VOIPtr, (xhb-xlb+1), (yhb-ylb+1), (zhb-zlb+1), -1, pixeltype);
            QString outimg_file = outputfolder + QString("/x%1_y%2_z%3.tif").arg(x_coord).arg(y_coord).arg(z_coord);
            string filename = outimg_file.toStdString();
            const char* filenameptr = filename.c_str();
            simple_saveimage_wrapper(callback, filenameptr, VOIPtr, VOIxyz, 1);

            QString outimg_file_linker = outputfolder + QString("/x%1_y%2_z%3.ano").arg(x_coord).arg(y_coord).arg(z_coord);
            QFile qf_anofile(outimg_file_linker);
            if(!qf_anofile.open(QIODevice::WriteOnly))
            {
                v3d_msg("Cannot open file for writing.",0);
                return;
            }
            QTextStream out(&qf_anofile);
            out << "RAWIMG= " << filenameptr << endl;
            out << "SWCFILE= " << outimg_fileSWC.toStdString().c_str()<< endl;
        }
        QString FinishMsg = QString("All fiels are saved in ") + outputfolder;
        v3d_msg(FinishMsg);

        return;
    }
    else
    {
        v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by MK, 2017-1-20"));
    }
}

bool TeraVOI_Generator::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("func1"))
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

