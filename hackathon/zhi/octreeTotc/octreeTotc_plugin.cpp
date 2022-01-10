/* octreeTotc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2016-10-3 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "octreeTotc_plugin.h"
#include <fstream>
#include <iostream>

using namespace std;
Q_EXPORT_PLUGIN2(octreeTotc, octreeTotc);
 
QStringList octreeTotc::menulist() const
{
	return QStringList() 
        <<tr("convert")
		<<tr("about");
}

QStringList octreeTotc::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void octreeTotc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("convert"))
	{
        QString fileOpenName;
        fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the highest image file:"),
                                                    "",
                                                    QObject::tr("Supported file (*.v3draw *.raw *.tif)"
                                                        ));
        if(fileOpenName.isEmpty())
        {
            return;
        }

        bool ok;
        int layer = QInputDialog::getInt(parent, "Please specify the layer number","layer number:",1,0,10,0.1,&ok);
        if (!ok)
            return;

        unsigned char * inimg1d = 0;
        V3DLONG  in_sz[4];
        int datatype;
        if(!simple_loadimage_wrapper(callback,(char*)fileOpenName.toStdString().c_str(), inimg1d, in_sz, datatype))
        {
            QMessageBox::information(0,"","Invalid Image!");
            return;
        }

        V3DLONG N = in_sz[0]*pow(2.0,layer-1);
        V3DLONG M = in_sz[1]*pow(2.0,layer-1);
        V3DLONG P = in_sz[2]*pow(2.0,layer-1);

        V3DLONG tilenum = 0;
        QDirIterator it(QFileInfo(fileOpenName).path(), QStringList() << "*.0.tif", QDir::Files, QDirIterator::Subdirectories);
        QStringList validFileList;
        while (it.hasNext()) {
         //   qDebug() << it.next();
            QString fileFullName = it.next();
            QString fileRelativeName = fileFullName.remove(0,QFileInfo(fileOpenName).path().size()+1);
            if(fileRelativeName.split("/").size() == layer)
            {
                validFileList.push_back(fileRelativeName);
                tilenum++;
            }
        }

        QString tc_name(QFileInfo(fileOpenName).path());
        tc_name.append("/stitched_image.tc");

        ofstream myfile;
        myfile.open(tc_name.toStdString().c_str(), ios::in);

        if (myfile.is_open()==true)
        {
            myfile.close();
            remove(tc_name.toStdString().c_str());
        }

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

        V3DLONG start_x,start_y,start_z;
        for(V3DLONG i = 0; i < validFileList.size();i++)
        {
            QStringList blocknum = validFileList.at(i).split("/");
            V3DLONG xb=0,xe,yb=0,ye,zb=0,ze;
            for(int j = 0; j < layer-1;j++)
            {
                switch(blocknum.at(j).toInt())
                {
                case 1:start_x=0; start_y=0; start_z=0;break;
                case 2:start_x=N/pow(2,j+1); start_y=0; start_z=0;break;
                case 3:start_x=0; start_y=M/pow(2,j+1); start_z=0;break;
                case 4:start_x=N/pow(2,j+1); start_y=M/pow(2,j+1); start_z=0;break;
                case 5:start_x=0; start_y=0; start_z=P/pow(2,j+1);break;
                case 6:start_x=N/pow(2,j+1); start_y=0; start_z=P/pow(2,j+1);break;
                case 7:start_x=0; start_y=M/pow(2,j+1); start_z=P/pow(2,j+1);break;
                case 8:start_x=N/pow(2,j+1); start_y=M/pow(2,j+1); start_z=P/pow(2,j+1);break;
                }
                xb += start_x;
                yb += start_y;
                zb += start_z;
            }

            xe = xb + in_sz[0] - 1;
            ye = yb + in_sz[1] - 1;
            ze = zb + in_sz[2] - 1;

            myfile.open (tc_name.toStdString().c_str(),ios::out | ios::app );
            QString outputilefull = validFileList.at(i);
            outputilefull.append(QString("   ( %1, %2, %3) ( %4, %5, %6)").arg(xb).arg(yb).arg(zb).arg(xe).arg(ye).arg(ze));
            myfile << outputilefull.toStdString();
            myfile << "\n";
            myfile.close();
        }

        v3d_msg("done!");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2016-10-3"));
	}
}

bool octreeTotc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

