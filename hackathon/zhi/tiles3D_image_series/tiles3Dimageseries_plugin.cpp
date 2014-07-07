/* tiles3Dimageseries_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2014-07-03 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "tiles3Dimageseries_plugin.h"
#include "../../../released_plugins/v3d_plugins/istitch/y_imglib.h"
#include <boost/lexical_cast.hpp>


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

Q_EXPORT_PLUGIN2(tiles3Dimageseries, tiles3Dimageseries);

void zsectionsTotiles(V3DPluginCallback2 &callback, QWidget *parent);
void titlesZectionsTo3Dtiles(V3DPluginCallback2 &callback, QWidget *parent);
bool zsectionsTotiles(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

QStringList tiles3Dimageseries::menulist() const
{
	return QStringList() 
        <<tr("generate 3D tiles and tc file from image series")
        <<tr("generate 3D tiles from image tile series")
        <<tr("about");
}

QStringList tiles3Dimageseries::funclist() const
{
	return QStringList()
        <<tr("tile3DImageSeries")
		<<tr("help");
}

void tiles3Dimageseries::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("generate 3D tiles and tc file from image series"))
	{
        zsectionsTotiles(callback,parent);
    }
    else if (menu_name == tr("generate 3D tiles from image tile series"))
    {
        titlesZectionsTo3Dtiles(callback,parent);
    }
	else
	{
        v3d_msg(tr("This is a plugin to generate 3d tiles and tc file from image series,"
			"Developed by Zhi Zhou, 2014-07-03"));
	}
}

bool tiles3Dimageseries::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("imageseries_to_3dtiles"))
    {
        return zsectionsTotiles(callback,input, output);
	}
	else if (func_name == tr("help"))
	{
        cout<<"Usage : v3d -x dllname -f imageseries_to_3dtiles -i <inimg_folder> -o <outimg_folder> -p <ws> <ch>"<<endl;
        cout<<endl;
        cout<<"ws      tile size in X and Y dimesions,start from 1 "<<endl;
        cout<<"ch      the channel value,start from 1              "<<endl;
        cout<<endl;
    }
	else return false;

	return true;
}

void zsectionsTotiles(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString m_InputfolderName = 0;
    m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all images "),
                                          QDir::currentPath(),
                                          QFileDialog::ShowDirsOnly);
    if(m_InputfolderName == 0)
        return;
    QString m_OutputfolderName = 0;
    m_OutputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory to save all tiles "),
                                          QDir::currentPath(),
                                          QFileDialog::ShowDirsOnly);
    if(m_OutputfolderName == 0)
        return;
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

    unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    int datatype;

    if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(0).fn_image.c_str()), data1d, in_sz, datatype))
    {
        fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(0).fn_image.c_str());
        return;
    }

    int Ws,c;
    bool ok1,ok2;

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = NTILES;
    V3DLONG SC = in_sz[3];
    V3DLONG pagesz = N*M*1;

    V3DLONG Ws_max;
    if(N>M) Ws_max = M; else Ws_max = N;
    Ws = QInputDialog::getInteger(parent, "tile size",
                                  "Enter tile size:",
                                  1, 1, Ws_max, 1, &ok1);
    if(SC==1)
    {
        c=1;
        ok2=true;
    }
    else
    {
        if(ok1)
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel NO:",
                                         1, 1, SC, 1, &ok2);
        }
        else
            return;
    }

    if(!ok2)
        return;

    if(data1d) {delete []data1d; data1d=0;}

    V3DLONG offsetc = (c-1)*N*M;

    for(V3DLONG iz = 0; iz < NTILES; iz = iz + 100)
    {
        V3DLONG zb = iz;
        V3DLONG ze = iz + 100 - 1; if(ze>=NTILES-1) ze = NTILES-1;

        unsigned char *sub_image=0;
        V3DLONG sub_image_sz = N*M*(ze-zb+1);
        sub_image = new unsigned char [sub_image_sz];
        for(V3DLONG i = 0; i < sub_image_sz; i++)
            sub_image[i] = 0;

        V3DLONG j = 0;
        for(int ii = zb; ii < ze + 1; ii++)
        {
            unsigned char * data1d = 0;
            V3DLONG in_sz[4];
            int datatype;

            if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(ii).fn_image.c_str()), data1d, in_sz, datatype))
            {
                fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                return;
            }

            for(V3DLONG i = 0; i < pagesz; i++)
            {
                sub_image[j] = data1d[offsetc+i];
                j++;
            }
            if(data1d) {delete []data1d; data1d=0;}

        }

        for(V3DLONG iy = 0; iy < M; iy = iy+Ws-Ws/10)
        {
            V3DLONG yb = iy;
            V3DLONG ye = iy+Ws-1; if(ye>=M-1) ye = M-1;

            for(V3DLONG ix = 0; ix < N; ix = ix+Ws-Ws/10)
            {
                V3DLONG xb = ix;
                V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;

                unsigned char *blockarea=0;
                V3DLONG blockpagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
                blockarea = new unsigned char [blockpagesz];
                for(V3DLONG i = 0; i < blockpagesz; i++)
                    blockarea[i] = 0;

                V3DLONG i = 0;
                for(V3DLONG iz = 0; iz < ze-zb+1; iz++)
                {
                    V3DLONG offsetk = iz*M*N;
                    for(V3DLONG iy = yb; iy < ye+1; iy++)
                    {
                        V3DLONG offsetj = iy*N;
                        for(V3DLONG ix = xb; ix < xe+1; ix++)
                        {

                            blockarea[i] = sub_image[offsetk + offsetj + ix];
                            i++;
                        }
                    }
                }

                V3DLONG block_sz[4];
                block_sz[0] = xe-xb+1; block_sz[1] = ye-yb+1; block_sz[2] = (ze-zb+1); block_sz[3] = 1;



                QString outputTile(m_OutputfolderName);
                outputTile.append(QString("/x_%1_%2_y_%3_%4_z_%5_%6.raw").arg(xb).arg(xe).arg(yb).arg(ye).arg(zb).arg(ze));
                simple_saveimage_wrapper(callback, outputTile.toStdString().c_str(), (unsigned char *)blockarea, block_sz, 1);
                if(blockarea) {delete []blockarea; blockarea=0;}

            }
        }

        if(sub_image) {delete []sub_image; sub_image=0;}


    }

    V3DLONG tilenum = (floor(N/(0.9*Ws))+1.0)*(floor(M/(0.9*Ws))+1.0);
    QString tc_name(m_OutputfolderName);
    tc_name.append("/stitched_image.tc");

    ofstream myfile;
    myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
    myfile << "# thumbnail file \n";
    myfile << "NULL \n\n";
    myfile << "# tiles \n";
    myfile << tilenum << " \n\n";
    myfile << "# dimensions (XYZC) \n";
    myfile << N << " " << M << " " << P << " " << 1 << " ";
    myfile << "\n\n";
    myfile << "# origin (XYZ) \n";
    myfile << "0.000000 0.000000 0.000000 \n\n";
    myfile << "# resolution (XYZ) \n";
    myfile << "1.000000 1.000000 1.000000 \n\n";
    myfile << "# image coordinates look up table \n";
    myfile.close();

    for(V3DLONG iy = 0; iy < M; iy = iy+Ws-Ws/10)
    {
        V3DLONG yb = iy;
        V3DLONG ye = iy+Ws-1; if(ye>=M-1) ye = M-1;

        for(V3DLONG ix = 0; ix < N; ix = ix+Ws-Ws/10)
        {
            V3DLONG xb = ix;
            V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;

            unsigned char *tilearea=0;
            V3DLONG tilepagesz = (xe-xb+1)*(ye-yb+1)*P;
            tilearea = new unsigned char [tilepagesz];
            for(V3DLONG i = 0; i < tilepagesz; i++)
                tilearea[i] = 0;

            V3DLONG tilearea_sz[4];
            tilearea_sz[0] = xe-xb+1; tilearea_sz[1] = ye-yb+1; tilearea_sz[2] = P; tilearea_sz[3] = 1;
            V3DLONG i = 0;

            for(V3DLONG iz = 0; iz < NTILES; iz = iz + 100)
            {
                V3DLONG zb = iz;
                V3DLONG ze = iz + 100 - 1; if(ze>=NTILES-1) ze = NTILES-1;

                QString inputTile(m_OutputfolderName);
                inputTile.append(QString("/x_%1_%2_y_%3_%4_z_%5_%6.raw").arg(xb).arg(xe).arg(yb).arg(ye).arg(zb).arg(ze));

                unsigned char * sub_data1d = 0;
                V3DLONG in_sz_sub[4];
                int datatype;

                if (!simple_loadimage_wrapper(callback,inputTile.toStdString().c_str(), sub_data1d, in_sz_sub, datatype))
                {
                    return;
                }
                V3DLONG inputTilepagesz = in_sz_sub[0]*in_sz_sub[1]*in_sz_sub[2];
                for(V3DLONG j = 0; j < inputTilepagesz; j++)
                {
                    tilearea[i] = sub_data1d[j];
                    i++;
                }
                if(sub_data1d) {delete []sub_data1d; sub_data1d = 0;}
                remove(inputTile.toStdString().c_str());
            }


            QString outputTile(m_OutputfolderName);
            outputTile.append(QString("/x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye));
            simple_saveimage_wrapper(callback, outputTile.toStdString().c_str(), (unsigned char *)tilearea, tilearea_sz, 1);

            myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
            QString outputilefull;
            outputilefull.append(QString("x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye));
            outputilefull.append(QString("   ( %1, %2, 0) ( %3, %4, %5)").arg(xb).arg(yb).arg(xe).arg(ye).arg(P-1));
            myfile << outputilefull.toStdString();
            myfile << "\n";
            myfile.close();

            if(tilearea) {delete []tilearea; tilearea =0;}

        }
    }

    myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
    myfile << "\n# MST LUT\n";
    myfile.close();

    v3d_msg(QString("3D tiles and the tc file are saved in %1").arg(m_OutputfolderName.toStdString().c_str()));
    return;
}

void titlesZectionsTo3Dtiles(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString m_InputfolderName = 0;
    m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all images "),
                                          QDir::currentPath(),
                                          QFileDialog::ShowDirsOnly);
    if(m_InputfolderName == 0)
        return;
    QString m_OutputfolderName = 0;
    m_OutputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory to save all tiles "),
                                          QDir::currentPath(),
                                          QFileDialog::ShowDirsOnly);
    if(m_OutputfolderName == 0)
        return;
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
    unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    int datatype;

    if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(0).fn_image.c_str()), data1d, in_sz, datatype))
    {
        fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(0).fn_image.c_str());
        return;
    }

    if(data1d) {delete[]data1d; data1d = 0;}
    V3DLONG im_length = strlen(vim.tilesList.at(0).fn_image.c_str());

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG SC = in_sz[3];
    V3DLONG pagesz = N*M*1;

    int Znum, c;
    bool ok1,ok2;
    Znum = QInputDialog::getInteger(parent, "z sections",
                                  "Enter the number of z sections:",
                                  1, 1, 2000, 1, &ok1);
    if(SC==1)
    {
        c=1;
        ok2=true;
    }
    else
    {
        if(ok1)
        {
            c = QInputDialog::getInteger(parent, "Channel",
                                         "Enter channel NO:",
                                         1, 1, SC, 1, &ok2);
        }
        else
            return;
    }

    if(!ok2)
        return;

    V3DLONG offsetc = (c-1)*pagesz;
    QString order_name(m_OutputfolderName);
    order_name.append("/order.txt");

    ofstream myfile;
    for(V3DLONG iz = 0; iz < NTILES; iz = iz + Znum)
    {
        V3DLONG zb = iz;
        V3DLONG ze = iz + Znum - 1;

        unsigned char *sub_image=0;
        V3DLONG sub_image_sz = N*M*Znum;
        sub_image = new unsigned char [sub_image_sz];
        for(V3DLONG i = 0; i < sub_image_sz; i++)
            sub_image[i] = 0;

        V3DLONG j = 0;
        const char * image_name;
        for(int ii = zb; ii < ze + 1; ii++)
        {
            unsigned char * data1d = 0;
            V3DLONG in_sz[4];
            int datatype;

            if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(ii).fn_image.c_str()), data1d, in_sz, datatype))
            {
                fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                return;
            }
            image_name = vim.tilesList.at(ii).fn_image.c_str();
            std::string location =  std::string(image_name, im_length -13, 1);
            std::string z_index;
            if(location == "-")
                  z_index = std::string(image_name, im_length-15, 2);
            else
                  z_index = std::string(image_name, im_length-15, 3);
            int   z_index_num = boost::lexical_cast<int>(z_index);
            V3DLONG offsetz = z_index_num * pagesz;
            for(V3DLONG i = 0; i < pagesz; i++)
            {
                sub_image[offsetz + i] = data1d[offsetc + i];
            }
            if(data1d) {delete[]data1d; data1d = 0;}
         }
        std::string x_location =  std::string(image_name, im_length -30, 2);
        std::string y_location =  std::string(image_name, im_length -25, 2);
        V3DLONG block_sz[4];
        block_sz[0] = N; block_sz[1] = M; block_sz[2] = Znum; block_sz[3] = 1;

        QString outputTile(m_OutputfolderName);
        outputTile.append(QString("/X%1_Y%2.raw").arg(x_location.c_str()).arg(y_location.c_str()));
        simple_saveimage_wrapper(callback, outputTile.toStdString().c_str(), (unsigned char *)sub_image, block_sz, 1);
        if(sub_image) {delete []sub_image; sub_image=0;}

        myfile.open (order_name.toStdString().c_str(),ios::out | ios::app );
        QString outputilefull;
        outputilefull.append(QString("%1 %2 %3").arg(outputTile.toStdString().c_str()).arg(x_location.c_str()).arg(y_location.c_str()));
        myfile << outputilefull.toStdString();
        myfile << "\n";
        myfile.close();
    }

    return;
}
bool zsectionsTotiles(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"Welcome to tile3DImageSeries plugin"<<endl;
    if (output.size() != 1) return false;
    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
    int Ws = 1, c = 1;
    if (input.size() >= 2)
    {
        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        cout<<paras.size()<<endl;
        if(paras.size() >= 1) Ws = atoi(paras.at(0));
        if(paras.size() >= 2) c = atoi(paras.at(1));
     }
    cout<<"ws = "<<Ws<<endl;
    cout<<"ch = "<<c<<endl;
    cout<<"inimg_folder = "<<inimg_file<<endl;
    cout<<"outimg_folder = "<<outimg_file<<endl;

    QString m_InputfolderName(inimg_file);
    QString m_OutputfolderName(outimg_file);
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

    unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    int datatype;

    if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(0).fn_image.c_str()), data1d, in_sz, datatype))
    {
        fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(0).fn_image.c_str());
        return false;
    }

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = NTILES;
    V3DLONG SC = in_sz[3];
    V3DLONG pagesz = N*M*1;

    V3DLONG Ws_max;
    if(N>M) Ws_max = M; else Ws_max = N;
    if(data1d) {delete []data1d; data1d=0;}

    if(c > SC || c < 1 || Ws > Ws_max)
    {
       printf("invalid channel number or tile size! \n");
       return false;
    }

    V3DLONG offsetc = (c-1)*N*M;

    for(V3DLONG iz = 0; iz < NTILES; iz = iz + 100)
    {
        V3DLONG zb = iz;
        V3DLONG ze = iz + 100 - 1; if(ze>=NTILES-1) ze = NTILES-1;

        unsigned char *sub_image=0;
        V3DLONG sub_image_sz = N*M*(ze-zb+1);
        sub_image = new unsigned char [sub_image_sz];
        for(V3DLONG i = 0; i < sub_image_sz; i++)
            sub_image[i] = 0;

        V3DLONG j = 0;
        for(int ii = zb; ii < ze + 1; ii++)
        {
            unsigned char * data1d = 0;
            V3DLONG in_sz[4];
            int datatype;

            if (!simple_loadimage_wrapper(callback,const_cast<char *>(vim.tilesList.at(ii).fn_image.c_str()), data1d, in_sz, datatype))
            {
                fprintf (stderr, "Error happens in reading the subject file [%0]. Exit. \n",vim.tilesList.at(ii).fn_image.c_str());
                return false;
            }

            for(V3DLONG i = 0; i < pagesz; i++)
            {
                sub_image[j] = data1d[offsetc+i];
                j++;
            }
            if(data1d) {delete []data1d; data1d=0;}

        }

        for(V3DLONG iy = 0; iy < M; iy = iy+Ws-Ws/10)
        {
            V3DLONG yb = iy;
            V3DLONG ye = iy+Ws-1; if(ye>=M-1) ye = M-1;

            for(V3DLONG ix = 0; ix < N; ix = ix+Ws-Ws/10)
            {
                V3DLONG xb = ix;
                V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;

                unsigned char *blockarea=0;
                V3DLONG blockpagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
                blockarea = new unsigned char [blockpagesz];
                for(V3DLONG i = 0; i < blockpagesz; i++)
                    blockarea[i] = 0;

                V3DLONG i = 0;
                for(V3DLONG iz = 0; iz < ze-zb+1; iz++)
                {
                    V3DLONG offsetk = iz*M*N;
                    for(V3DLONG iy = yb; iy < ye+1; iy++)
                    {
                        V3DLONG offsetj = iy*N;
                        for(V3DLONG ix = xb; ix < xe+1; ix++)
                        {

                            blockarea[i] = sub_image[offsetk + offsetj + ix];
                            i++;
                        }
                    }
                }

                V3DLONG block_sz[4];
                block_sz[0] = xe-xb+1; block_sz[1] = ye-yb+1; block_sz[2] = (ze-zb+1); block_sz[3] = 1;

                QString outputTile(m_OutputfolderName);
                outputTile.append(QString("/x_%1_%2_y_%3_%4_z_%5_%6.raw").arg(xb).arg(xe).arg(yb).arg(ye).arg(zb).arg(ze));
                simple_saveimage_wrapper(callback, outputTile.toStdString().c_str(), (unsigned char *)blockarea, block_sz, 1);
                if(blockarea) {delete []blockarea; blockarea=0;}

            }
        }

        if(sub_image) {delete []sub_image; sub_image=0;}


    }

    V3DLONG tilenum = (floor(N/(0.9*Ws))+1.0)*(floor(M/(0.9*Ws))+1.0);
    QString tc_name(m_OutputfolderName);
    tc_name.append("/stitched_image.tc");

    ofstream myfile;
    myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
    myfile << "# thumbnail file \n";
    myfile << "NULL \n\n";
    myfile << "# tiles \n";
    myfile << tilenum << " \n\n";
    myfile << "# dimensions (XYZC) \n";
    myfile << N << " " << M << " " << P << " " << 1 << " ";
    myfile << "\n\n";
    myfile << "# origin (XYZ) \n";
    myfile << "0.000000 0.000000 0.000000 \n\n";
    myfile << "# resolution (XYZ) \n";
    myfile << "1.000000 1.000000 1.000000 \n\n";
    myfile << "# image coordinates look up table \n";
    myfile.close();

    for(V3DLONG iy = 0; iy < M; iy = iy+Ws-Ws/10)
    {
        V3DLONG yb = iy;
        V3DLONG ye = iy+Ws-1; if(ye>=M-1) ye = M-1;

        for(V3DLONG ix = 0; ix < N; ix = ix+Ws-Ws/10)
        {
            V3DLONG xb = ix;
            V3DLONG xe = ix+Ws-1; if(xe>=N-1) xe = N-1;

            unsigned char *tilearea=0;
            V3DLONG tilepagesz = (xe-xb+1)*(ye-yb+1)*P;
            tilearea = new unsigned char [tilepagesz];
            for(V3DLONG i = 0; i < tilepagesz; i++)
                tilearea[i] = 0;

            V3DLONG tilearea_sz[4];
            tilearea_sz[0] = xe-xb+1; tilearea_sz[1] = ye-yb+1; tilearea_sz[2] = P; tilearea_sz[3] = 1;
            V3DLONG i = 0;

            for(V3DLONG iz = 0; iz < NTILES; iz = iz + 100)
            {
                V3DLONG zb = iz;
                V3DLONG ze = iz + 100 - 1; if(ze>=NTILES-1) ze = NTILES-1;

                QString inputTile(m_OutputfolderName);
                inputTile.append(QString("/x_%1_%2_y_%3_%4_z_%5_%6.raw").arg(xb).arg(xe).arg(yb).arg(ye).arg(zb).arg(ze));

                unsigned char * sub_data1d = 0;
                V3DLONG in_sz_sub[4];
                int datatype;

                if (!simple_loadimage_wrapper(callback,inputTile.toStdString().c_str(), sub_data1d, in_sz_sub, datatype))
                {
                    return false;
                }
                V3DLONG inputTilepagesz = in_sz_sub[0]*in_sz_sub[1]*in_sz_sub[2];
                for(V3DLONG j = 0; j < inputTilepagesz; j++)
                {
                    tilearea[i] = sub_data1d[j];
                    i++;
                }
                if(sub_data1d) {delete []sub_data1d; sub_data1d = 0;}
                remove(inputTile.toStdString().c_str());
            }


            QString outputTile(m_OutputfolderName);
            outputTile.append(QString("/x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye));
            simple_saveimage_wrapper(callback, outputTile.toStdString().c_str(), (unsigned char *)tilearea, tilearea_sz, 1);

            myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
            QString outputilefull;
            outputilefull.append(QString("x_%1_%2_y_%3_%4.raw").arg(xb).arg(xe).arg(yb).arg(ye));
            outputilefull.append(QString("   ( %1, %2, 0) ( %3, %4, %5)").arg(xb).arg(yb).arg(xe).arg(ye).arg(P-1));
            myfile << outputilefull.toStdString();
            myfile << "\n";
            myfile.close();

            if(tilearea) {delete []tilearea; tilearea =0;}

        }
    }

    myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
    myfile << "\n# MST LUT\n";
    myfile.close();

    v3d_msg(QString("3D tiles and the tc file are saved in %1").arg(m_OutputfolderName.toStdString().c_str()),0);
    return true;

}
