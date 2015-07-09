/* file_combine_plugin.cpp
 * This plugin is for link neuron segments across stacks.
 * 2014-10-07 : by Hanbo Chen
 */

#include "v3d_message.h"
#include <vector>
#include "file_combine_plugin.h"
#include "neuron_stitch_func.h"
#include <iostream>
#include <fstream>

using namespace std;

Q_EXPORT_PLUGIN2(file_combine, file_combine)

QStringList file_combine::menulist() const
{
    return QStringList()
        <<tr("combine_marker_files")
		<<tr("about");
}

QStringList file_combine::funclist() const
{
    return QStringList()
		<<tr("help");
}

void file_combine::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("combine_marker_files"))
    {
        docombine_marker(callback, parent);
    }
	else
	{
        v3d_msg(tr("This plugin is for combine files from different sections.. "
			"Developed by Hanbo Chen, 2014-10-07"));
	}
}

bool file_combine::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("help"))
	{
        qDebug()<<"No do func for this plugin";
    }
	else return false;

	return true;
}

void file_combine::docombine_marker(V3DPluginCallback2 &callback, QWidget *parent)
{
    //input file 1
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open marker File"),
            "",
            QObject::tr("Supported file (*.marker)"
                ";;Marker file	(*.marker)"
                ));
    if(fileOpenName.isEmpty())
        return;
    QList <ImageMarker> inmarker;
    if (fileOpenName.toUpper().endsWith(".MARKER"))
    {
         inmarker = readMarker_file(fileOpenName);
    }

    //input file 2
    QString fileOpenName_2;
    fileOpenName_2 = QFileDialog::getOpenFileName(0, QObject::tr("Open marker File"),
            fileOpenName,
            QObject::tr("Supported file (*.marker)"
                ";;Marker file	(*.marker)"
                ));
    if(fileOpenName_2.isEmpty())
        return;
    QList <ImageMarker> inmarker_2;
    if (fileOpenName.toUpper().endsWith(".MARKER"))
    {
         inmarker_2 = readMarker_file(fileOpenName_2);
    }

    //output file
    for(V3DLONG i=0; i<inmarker_2.size() ; i++){
        inmarker.append(inmarker_2[i]);
    }

    QString fileDefaultName = QFileInfo(fileOpenName).dir().filePath(QFileInfo(fileOpenName).baseName()+
                                                                     "_"+QFileInfo(fileOpenName_2).baseName()+QString("_combine.marker"));
    //write new marker to file
    QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
            fileDefaultName,
            QObject::tr("Supported file (*.marker)"
                ";;Marker	(*.marker)"
                ));
    if(fileSaveName.isEmpty())
        return;
    if (!writeMarker_file(fileSaveName, inmarker))
    {
        v3d_msg("fail to write the output marker file.");
        return;
    }
}
