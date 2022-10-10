/* CCF_nuclei_locator_plugin.cpp
 * This plugin creates a set of markers in specified CCF regions based on experimental density clouds.
 * 2022-7-22 : by Linus Manubens Gil
 */
 
#include "v3d_message.h"
#include <time.h>
#include <vector>
#include <iostream>
#include "CCF_nuclei_locator_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(CCF_nuclei_locator, CCF_nuclei_locator);
 
QStringList CCF_nuclei_locator::menulist() const
{
	return QStringList() 
        <<tr("CCF_nuclei")
		<<tr("about");
}

QStringList CCF_nuclei_locator::funclist() const
{
	return QStringList()
		<<tr("CCF_nuclei")
		<<tr("help");
}

void CCF_nuclei_locator::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("CCF_nuclei"))
	{
        menu_locate(callback,parent);
	}
	else
	{
        v3d_msg(tr("This plugin creates a set of markers in specified CCF regions based on experimental density clouds or at a given density. "
            "Developed by Linus Manubens-Gil, 2022-7-22"));
	}
}

// Function for saving mask
//template<class T> bool save_mask_image(T * &outimg1d, unsigned char * inimg1d, V3DLONG reg_id, int soma_radius, QString output_file,
//                                       long sz0, long sz1, long sz2, int datatype, V3DPluginCallback2 & callback)
bool get_soma_coords_fixed_dens(float * inimg1d, V3DLONG reg_id, float soma_d, float fixed_dens, QString output_marker_file,
                                       long sz0, long sz1, long sz2)

{
    cout << "Setting soma coordinates.\n";
    QList <ImageMarker> coord_list;
    QList <ImageMarker> coord_list_CCF;

    long tol_sz = sz0 * sz1 * sz2;
    long sz01 = sz0 * sz1;

    float sz_fac = float(25)/soma_d;
    //outimg1d = new T[tol_sz];
    //for(long i = 0; i < tol_sz; i++) outimg1d[i] = 0;

    srand( (unsigned)time( NULL ) );
    for(int x=0; x<round(float(sz0)*sz_fac);x++){
        for(int y=0; y<round(float(sz1)*sz_fac);y++){
            for(int z=0; z<round(float(sz2)*sz_fac);z++){
                //int x = allmarkers[i].x-1;
                //int y = allmarkers[i].y-1;
                //int z = allmarkers[i].z-1;
                //if((x<0) || (y<0) || (z<0)){continue;}
                V3DLONG id = round(float(z)/sz_fac)*sz01+round(float(y)/sz_fac)*sz0+round(float(x)/sz_fac);

                //cout << "tol_sz: " << tol_sz << "   id: " << id << "   inimg1d[id]: " << float(inimg1d[id]) << "   reg_id: " << reg_id << endl;

                double r = ((double) rand() / (RAND_MAX));
                if(id<tol_sz & float(inimg1d[id])==float(reg_id) & r < float(fixed_dens*soma_d/25)){
                    ImageMarker icoord, icoord_CCF;
                    icoord.x = round(float(x)*soma_d);
                    icoord.y = round(float(y)*soma_d);
                    icoord.z = round(float(z)*soma_d);
                    icoord.radius = soma_d/2;
                    coord_list.append(icoord);

                    icoord_CCF.x = round(float(x)/sz_fac);
                    icoord_CCF.y = round(float(y)/sz_fac);
                    icoord_CCF.z = round(float(z)/sz_fac);
                    icoord_CCF.radius = soma_d/(2*float(25));
                    coord_list_CCF.append(icoord_CCF);
                    //outimg1d[id] = inimg1d[id];
                }
            }
         }
     }

    QString QString_dot = ".";
    int int_lastIndexOfDot = output_marker_file.lastIndexOf(QString_dot);
    QString output_mk_file_1um = output_marker_file.mid(0, int_lastIndexOfDot);
    output_mk_file_1um.append("_1um_coord.marker");
    writeMarker_file(output_mk_file_1um,coord_list);
    writeMarker_file(output_marker_file,coord_list_CCF);


    //V3DLONG out_sz[4] = {sz0, sz1, sz2, 1};
    //const char * char_file = Qstring_to_char(output_file);
//    cout<<"before saveimage:"<<char_file<<"end"<<endl;
    //simple_saveimage_wrapper(callback, char_file, outimg1d, out_sz, datatype);
    return true;
}

void CCF_nuclei_locator::menu_locate( V3DPluginCallback2 &callback, QWidget *parent)
{
    QString reg = QInputDialog::getText(parent, "Brain region", "Enter a brain region defined in CCF v3 (CCF acronym):");

    QString regions_filename = QDir::currentPath() + "/Mouse.csv";
    QFile qf(regions_filename);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(regions_filename));
#endif
        return;
    }

    V3DLONG k=0;
    bool reg_found=0;
    V3DLONG pop_reg_id;
    while (! qf.atEnd())
    {
        char curline[2000];
        qf.readLine(curline, sizeof(curline));
        k++;
        {
            QStringList qsl = QString(curline).trimmed().split(",");
            int qsl_count=qsl.size();
            if (qsl_count<11)   continue;

            unsigned long long reg_id;
            QString reg_name;

            reg_id = qsl[1].toLongLong();
            reg_name = qsl[2];//qPrintable(qsl[2].trimmed());

            //cout << reg_id << endl;
            //cout << qPrintable(qsl[2].trimmed()) << endl;

            if(reg_name == reg){
                pop_reg_id = V3DLONG(reg_id);
                reg_found = 1;
                break;
            }
        }
    }
    if(reg_found==0){
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("Region [%1] not found in CCFv3 (Mouse.csv).").arg(reg));
#endif
        return;
    }

    cout << qPrintable(reg) << endl;
    cout << pop_reg_id << endl;


    QString celltype = QInputDialog::getText(parent,"Cell type", "Enter the cell type (e.g. Pyramidal, PV...");

    //Show open file dialog;
    QString QString_fileName = QFileDialog::getOpenFileName(parent, QObject::tr("Choose CCF image file to import..."),
                                                      QDir::currentPath(),
                                                      QObject::tr("CCF image files (*.*)"));


    if(QString_fileName.isEmpty())
    {
         return;
    }

    QByteArray QByteA = QString_fileName.toLocal8Bit();
    char* inimg_file = QByteA.data();


    unsigned char * data1d = 0;
    V3DLONG in_sz[4];

    int datatype;
    if(!simple_loadimage_wrapper(callback, inimg_file, data1d, in_sz, datatype))
         {
              cerr<<"load image "<<inimg_file<<" error!"<<endl;
              return;
         }

    cout << "Data type: " << datatype << endl;
    switch (datatype)
         {
              case 1: cout << "V3D_UINT8" << endl; break; //rotateimage(data1d, in_sz, V3D_UINT8, rotateflag, outimg); break;
              case 2: cout << "V3D_UINT16" << endl; break; //rotateimage(data1d, in_sz, V3D_UINT16, rotateflag, outimg); break;
              case 4: cout << "V3D_FLOAT32" << endl; break; //rotateimage(data1d, in_sz, V3D_FLOAT32, rotateflag, outimg); break;
              default:
                   v3d_msg("Invalid datatype.");
                   if (data1d) {delete []data1d; data1d=0;}
                   return;                  
    }

    float soma_d = QInputDialog::getDouble(parent, "soma_diameter",
                                         "Enter soma diameter (in micrometers).",
                                         15.0, 1.0, 100.0);


    int voxel_size = 25;
    /*int  voxel_size = QInputDialog::getInteger(parent, "CCF voxel size",
                                          "Enter the voxel size (side) in um for the CCF volume:",
                                          25, 1, 100);
                                          */

     float fixed_dens = QInputDialog::getDouble(parent, "max rotation",
                                          "Enter fixed density of neurons (number of neurons in a 25um side cube). Set 0 for using spatial distribution file.",
                                          1.0, 0.001, 50.0,6);

     // Initialize variables for spatial density image file
     char* densimg_file;
     char* densimg_name;

     unsigned char * dens_data1d = 0;
     V3DLONG dens_in_sz[4];

     int dens_datatype;

     // Get spatial density image file if fixed density==0
     if(fixed_dens==0){
         QString QString_fileName = QFileDialog::getOpenFileName(parent, QObject::tr("Choose spatial distribution image file (each voxel has a value equivalent to the number of neurons for the voxel volume -of side 25um to match CCFv3-) to import..."),
                                                           QDir::currentPath(),
                                                           QObject::tr("Spatial distribution image file in CCF space (*.*)"));

         if(QString_fileName.isEmpty())
         {
              return;
         }

         QByteArray QByteA = QString_fileName.toLocal8Bit();
         densimg_file = QByteA.data();

         QString QString_dot = ".";
         QString QString_slash = "/";
         int int_lastIndexOfDot = QString_fileName.lastIndexOf(QString_dot);
         int int_lastIndexOfSlash = QString_fileName.lastIndexOf(QString_slash);
         QString QString_mainName = QString_fileName.mid(int_lastIndexOfSlash, int_lastIndexOfDot);
         QByteA = QString_mainName.toLocal8Bit();
         densimg_name = QByteA.data();


         if(!simple_loadimage_wrapper(callback, densimg_file, dens_data1d, dens_in_sz, dens_datatype))
              {
                   cerr<<"load image "<<densimg_file<<" error!"<<endl;
                   return;
              }
     }


     //------------------------

     QString  output_marker_file;
     if(fixed_dens != 0 ){
         output_marker_file = "CCF_nuclei_"+ reg + "_" + celltype + "_fixed_dens_" + QString::number(fixed_dens) +"_per_25um_cube.marker";
         if(!get_soma_coords_fixed_dens((float *)data1d, pop_reg_id, soma_d, fixed_dens, output_marker_file, in_sz[0], in_sz[1], in_sz[2]))
         {
             cerr << "Error when getting soma coordinates" << endl;
         }
     }
     else{
         output_marker_file = "CCF_nuclei_"+ reg + "_" + celltype + "_" + densimg_name +".marker";
     }
     v3d_msg("output marker file can be found at: " + output_marker_file);

    return;
}

bool CCF_nuclei_locator::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("CCF_nuclei"))
	{
        char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
        cout<<"inimg_file = "<<inimg_file<<endl;

        unsigned char * data1d = 0;
        V3DLONG in_sz[4];

        int datatype;
        if(!simple_loadimage_wrapper(callback, inimg_file, data1d, in_sz, datatype))
             {
                  cerr<<"load image "<<inimg_file<<" error!"<<endl;
                  return false;
             }

        switch (datatype)
             {
                  /*
                  case 1: rotateimage(data1d, in_sz, V3D_UINT8, rotateflag, outimg); break;
                  case 2: rotateimage(data1d, in_sz, V3D_UINT16, rotateflag, outimg); break;
                  case 4: rotateimage(data1d, in_sz, V3D_FLOAT32, rotateflag, outimg); break;
                  default:
                       v3d_msg("Invalid datatype.");
                       if (data1d) {delete []data1d; data1d=0;}
                       return false;
                  */
             }

        v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

