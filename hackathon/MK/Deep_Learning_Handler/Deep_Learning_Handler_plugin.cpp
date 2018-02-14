/* Deep_Learning_Handler_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2017-11-29 : by YourName
 */

#include "v3d_message.h"
#include <vector>
#include "Deep_Learning_Handler_plugin.h"
#include <iostream>
#include <qfileinfo.h>
#include <qdir.h>
#include <v3d_interface.h>
#include <string>
#include <basic_4dimage.h>

using namespace std;
Q_EXPORT_PLUGIN2(Deep_Learning_Handler, DL_Handler);

QStringList DL_Handler::menulist() const
{
    return QStringList()
        <<tr("menu1")
        <<tr("menu2")
        <<tr("about");
}

QStringList DL_Handler::funclist() const
{
    return QStringList()
        <<tr("func1")
        <<tr("func2")
        <<tr("help");
}

void DL_Handler::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("menu1"))
    {
        v3d_msg("To be implemented.");
    }
    else if (menu_name == tr("menu2"))
    {
        v3d_msg("To be implemented.");
    }
    else
    {
        v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by YourName, 2017-11-29"));
    }
}

bool DL_Handler::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("imageCrop4CaffePredict"))
    {
        string deployFile = inparas.at(0);
        string trainedModel = inparas.at(1);
        string meanFile = inparas.at(2);

        string outputDir = infiles.at(1);
        QString curPath = QDir::currentPath();
        QString patchPath = curPath + "/patches";
        if (!QDir(patchPath).exists()) QDir().mkpath(patchPath);
        QString swcPath = curPath + "/SWCs";
        if (!QDir(swcPath).exists()) QDir().mkpath(swcPath);
        unsigned char* ImgPtr = 0;
        V3DLONG in_sz[4];
        int datatype;
        if (!simple_loadimage_wrapper(callback, infiles.at(0), ImgPtr, in_sz, datatype))
        {
            cerr << "Error reading image file [" << infiles.at(0) << "]. Exit." << endl;
            return true;
        }
        int imgX = in_sz[0];
        int imgY = in_sz[1];
        int imgZ = in_sz[2];
        int channel = in_sz[3];

        int i_incres = imgX / 256;
        int j_incres = imgY / 256;
        qDebug() << i_incres << " " << j_incres;
        int zlb = 1;
        int zhb = imgZ;
        for (int j = 0; j < j_incres; ++j)
        {
            for (int i = 0; i < i_incres; ++i)
            {
                int xlb = 256 * i + 1;
                int xhb = 256 * (i+1);
                int ylb = 256 * j + 1;
                int yhb = 256 * (j+1);
                V3DLONG VOIsz = 256 * 256 * imgZ;
                unsigned char* VOIPtr = new unsigned char[VOIsz];
                V3DLONG VOIxyz[4];
                VOIxyz[0] = 256;
                VOIxyz[1] = 256;
                VOIxyz[2] = imgZ;
                VOIxyz[3] = in_sz[3];
                this->cropStack(ImgPtr, VOIPtr, xlb, xhb, ylb, yhb, 1, zhb, imgX, imgY, imgZ);
                QString outimg_file = patchPath + QString("/x%1_y%2.v3draw").arg(xlb-1).arg(ylb-1);
                char* filenameNew = const_cast<char *>(outimg_file.toStdString().c_str());
                string filename = outimg_file.toStdString();
                qDebug() << VOIPtr[40000] << " " << outimg_file << endl;
                const char* filenameptr = filename.c_str();
                /*Image4DSimple* patch;
                patch->setDatatype(V3D_UINT8);
                patch->setXDim(256);
                patch->setYDim(256);
                patch->setZDim(imgZ);
                patch->setCDim(1);
                patch->setTDim(1);
                patch->setNewRawDataPointer(VOIPtr);
                patch->saveImage(filenameNew);*/
                simple_saveimage_wrapper(callback, filenameptr, VOIPtr, VOIxyz, 1);
                //qDebug() << VOIPtr[40000] << " " << outimg_file << endl;
                delete[] VOIPtr;
            }
        }

        QString m_InputfolderName = patchPath;
        QStringList imgList = importSeriesFileList_addnumbersort(m_InputfolderName);
        cout << "patch number: " << imgList.size() << endl;
        for (QStringList::iterator imgIt=imgList.begin(); imgIt!=imgList.end(); ++imgIt)
        {
            QStringList imgNameparse = (*imgIt).split("/");
            QString name1 = *(imgNameparse.end()-1);
            QStringList name1parse = name1.split(".");
            QString name = name1parse[0];
            QString indiSWCPath = swcPath + "/" + name;
            if (!QDir(indiSWCPath).exists()) QDir().mkpath(indiSWCPath);
            string fileFullName = (*imgIt).toStdString();
            string command = "vaa3d -x prediction_caffe -f 3D_Axon_detection_raw -i ";
            command = command + fileFullName + " -p ";
            command = command + deployFile + " " + trainedModel + " " + meanFile + " 4 128 -o " + indiSWCPath.toStdString() + "/traced.swc";
            cout << command << endl;
            const char* command_cStr = command.c_str();

            system(command_cStr);
        }

        NeuronTree newTree;
        QString SWCsPath = swcPath;
        QDir dir(SWCsPath);
        foreach (QString folderName, dir.entryList(QDir::AllDirs))
        {
            if (folderName == "." || folderName == "..") continue;

            QString tileSWCPath = swcPath + "/" + folderName + "/traced.swc_0.9.swc";
            QStringList tileNameParse = folderName.split("_");
            QStringList xParse = tileNameParse[0].split("x");
            QStringList yParse = tileNameParse[1].split("y");
            float xLoc = xParse[1].toFloat();
            float yLoc = yParse[1].toFloat();
            //cout << xLoc << " " << yLoc << endl;

            const char* tileSWCCString = tileSWCPath.toStdString().c_str();
            NeuronTree nt = readSWC_file(tileSWCCString);
            newTree.listNeuron.push_back(nt.listNeuron.at(0));
            for (QList<NeuronSWC>::iterator it=nt.listNeuron.begin()+1; it!=nt.listNeuron.end(); ++it)
            {
                bool flag = false;
                for (int newi=0; newi<newTree.listNeuron.size(); ++newi)
                {
                    float adX = it->x + xLoc;
                    float adY = it->y + yLoc;

                    if (adX==newTree.listNeuron[newi].x && adY==newTree.listNeuron[newi].y && it->z==newTree.listNeuron[newi].z)
                    {
                        //cout << it->x << " " << it->y << " " << it->z << "      ";
                        //cout << newTree.listNeuron[newi].x << " " << newTree.listNeuron[newi].y << " " << newTree.listNeuron[newi].z << endl;
                        flag = true;
                        break;
                    }
                }
                //cout << "===================" << endl;
                /*for (QList<NeuronSWC>::iterator newIt=newTree.listNeuron.begin(); newIt!=newTree.listNeuron.end(); ++newIt)
                {
                    if (it->x==newIt->x && it->y==newIt->y && it->z==newIt->z)
                    {
                        flag = true;
                        break;
                    }
                }*/

                if (flag == true) continue;
                else
                {
                    NeuronSWC newNode;
                    newNode.x = it->x + xLoc;
                    newNode.y = it->y + yLoc;
                    newNode.z = it->z;
                    newNode.type = it->type;
                    newNode.n = it->n;
                    newNode.parent = it->parent;
                    newTree.listNeuron.push_back(newNode);
                }
            }
        }

        QString saveSWCName = SWCsPath + "/combined.swc";
        const char* saveSWCNameCString = saveSWCName.toStdString().c_str();
        writeSWC_file(saveSWCName, newTree);

        return true;
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

