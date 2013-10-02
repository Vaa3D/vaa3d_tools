//------------------------------------------------------------------------------------------------
// Copyright (c) 2012  Alessandro Bria and Giulio Iannello (University Campus Bio-Medico of Rome).
// All rights reserved.
//------------------------------------------------------------------------------------------------

/*******************************************************************************************************************************************************************************************
*    LICENSE NOTICE
********************************************************************************************************************************************************************************************
*    By downloading/using/running/editing/changing any portion of codes in this package you agree to this license. If you do not agree to this license, do not download/use/run/edit/change
*    this code.
********************************************************************************************************************************************************************************************
*    1. This material is free for non-profit research, but needs a special license for any commercial purpose. Please contact Alessandro Bria at a.bria@unicas.it or Giulio Iannello at
*       g.iannello@unicampus.it for further details.
*    2. You agree to appropriately cite this work in your related studies and publications.
*
*       Bria, A., et al., (2012) "Stitching Terabyte-sized 3D Images Acquired in Confocal Ultramicroscopy", Proceedings of the 9th IEEE International Symposium on Biomedical Imaging.
*       Bria, A., Iannello, G., "A Tool for Fast 3D Automatic Stitching of Teravoxel-sized Datasets", submitted on July 2012 to IEEE Transactions on Information Technology in Biomedicine.
*
*    3. This material is provided by  the copyright holders (Alessandro Bria  and  Giulio Iannello),  University Campus Bio-Medico and contributors "as is" and any express or implied war-
*       ranties, including, but  not limited to,  any implied warranties  of merchantability,  non-infringement, or fitness for a particular purpose are  disclaimed. In no event shall the
*       copyright owners, University Campus Bio-Medico, or contributors be liable for any direct, indirect, incidental, special, exemplary, or  consequential  damages  (including, but not
*       limited to, procurement of substitute goods or services; loss of use, data, or profits;reasonable royalties; or business interruption) however caused  and on any theory of liabil-
*       ity, whether in contract, strict liability, or tort  (including negligence or otherwise) arising in any way out of the use of this software,  even if advised of the possibility of
*       such damage.
*    4. Neither the name of University  Campus Bio-Medico of Rome, nor Alessandro Bria and Giulio Iannello, may be used to endorse or  promote products  derived from this software without
*       specific prior written permission.
********************************************************************************************************************************************************************************************/

#include <QSettings>
#include <iostream>
#include "CSettings.h"

using namespace teramanager;
using namespace std;

CSettings* CSettings::uniqueInstance = NULL;

void CSettings::uninstance()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CSettings::uninstance()\n");
    #endif

    if(uniqueInstance)
    {
        delete uniqueInstance;
        uniqueInstance = NULL;
    }
}

CSettings::~CSettings()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CSettings::~CSettings()\n");
    printf("--------------------- teramanager plugin [thread ?] >> CSettings destroyed\n");
    #endif

    writeSettings();
}

void CSettings::loadDefaultSettings()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CSettings::loadDefaultSettings()\n");
    #endif

    //TeraFly settings
    volumePathLRU = "";
    annotationPathLRU = "";
    volMapSizeLimit = 30;
    VOIdimV = VOIdimH = 200;
    VOIdimD = 50;
    traslX = traslY = traslZ = 50;  //percentage value

    //TeraConverter settings
    volumeConverterInputPathLRU = "";
    volumeConverterOutputPathLRU = "";
    volumeConverterInputFormatLRU = "Vaa3D raw (nontiled)";
    volumeConverterOutputFormatLRU = "Vaa3D raw (tiled)";
    volumeConverterStacksWidthLRU = 256;
    volumeConverterStacksHeightLRU = 256;
    volumeConverterStacksDepthLRU = 256;
}

void CSettings::writeSettings()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CSettings::writeSettings() called\n");
    #endif

    QSettings settings("ICON", "TeraManager");
    QString volumePathLRU_qstring(volumePathLRU.c_str());
    QString annotationPathLRU_qstring(annotationPathLRU.c_str());
    settings.setValue("annotationPathLRU", annotationPathLRU_qstring);
    settings.setValue("volumePathLRU", volumePathLRU_qstring);

    settings.beginWriteArray("volumePathHistory");
    std::list<string>::iterator it = volumePathHistory.begin();
    for (size_t i = 0; i < volumePathHistory.size(); ++i, it++) {
        settings.setArrayIndex(i);
        QString path(it->c_str());
        settings.setValue("path", path);
    }
    settings.endArray();

    settings.setValue("volMapSizeLimit", volMapSizeLimit);
    settings.setValue("VOIdimV", VOIdimV);
    settings.setValue("VOIdimH", VOIdimH);
    settings.setValue("VOIdimD", VOIdimD);
    settings.setValue("traslX", traslX);
    settings.setValue("traslY", traslY);
    settings.setValue("traslZ", traslZ);

    settings.setValue("volumeConverterInputPathLRU", QString(volumeConverterInputPathLRU.c_str()));
    settings.setValue("volumeConverterOutputPathLRU", QString(volumeConverterOutputPathLRU.c_str()));
    settings.setValue("volumeConverterInputFormatLRU", QString(volumeConverterInputFormatLRU.c_str()));
    settings.setValue("volumeConverterOutputFormatLRU", QString(volumeConverterOutputFormatLRU.c_str()));
    settings.setValue("volumeConverterStacksWidthLRU", volumeConverterStacksWidthLRU);
    settings.setValue("volumeConverterStacksHeightLRU", volumeConverterStacksHeightLRU);
    settings.setValue("volumeConverterStacksDepthLRU", volumeConverterStacksDepthLRU);


    settings.setValue("verbosity", itm::DEBUG);
}



void CSettings::readSettings()
{
    #ifdef TMP_DEBUG
    printf("--------------------- teramanager plugin [thread ?] >> CSettings::readSettings()\n");
    #endif

    QSettings settings("ICON", "TeraManager");

    //TeraFly settings
    if(settings.contains("annotationPathLRU"))
        annotationPathLRU = settings.value("annotationPathLRU").toString().toStdString();
    if(settings.contains("volumePathLRU"))
        volumePathLRU = settings.value("volumePathLRU").toString().toStdString();
    if(settings.contains("volMapSizeLimit"))
        volMapSizeLimit = settings.value("volMapSizeLimit").toInt();
    if(settings.contains("VOIdimV"))
        VOIdimV = settings.value("VOIdimV").toInt();
    if(settings.contains("VOIdimH"))
        VOIdimH = settings.value("VOIdimH").toInt();
    if(settings.contains("VOIdimD"))
        VOIdimD = settings.value("VOIdimD").toInt();    
    if(settings.contains("traslX"))
        traslX = settings.value("traslX").toInt();
    if(settings.contains("traslY"))
        traslY = settings.value("traslY").toInt();
    if(settings.contains("traslZ"))
        traslZ = settings.value("traslZ").toInt();
    int size = settings.beginReadArray("volumePathHistory");
    volumePathHistory.clear();
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        volumePathHistory.push_back(settings.value("path").toString().toStdString());
    }
    settings.endArray();

    //TeraManager settings
    if(settings.contains("volumeConverterInputPathLRU"))
        volumeConverterInputPathLRU = settings.value("volumeConverterInputPathLRU").toString().toStdString();
    if(settings.contains("volumeConverterOutputPathLRU"))
        volumeConverterOutputPathLRU = settings.value("volumeConverterOutputPathLRU").toString().toStdString();
    if(settings.contains("volumeConverterInputFormatLRU"))
        volumeConverterInputFormatLRU = settings.value("volumeConverterInputFormatLRU").toString().toStdString();
    if(settings.contains("volumeConverterOutputFormatLRU"))
        volumeConverterOutputFormatLRU = settings.value("volumeConverterOutputFormatLRU").toString().toStdString();
    if(settings.contains("volumeConverterStacksWidthLRU"))
        volumeConverterStacksWidthLRU = settings.value("volumeConverterStacksWidthLRU").toInt();
    if(settings.contains("volumeConverterStacksHeightLRU"))
        volumeConverterStacksHeightLRU = settings.value("volumeConverterStacksHeightLRU").toInt();
    if(settings.contains("volumeConverterStacksDepthLRU"))
        volumeConverterStacksDepthLRU = settings.value("volumeConverterStacksDepthLRU").toInt();

     if(settings.contains("verbosity"))
         itm::DEBUG = settings.value("verbosity").toInt();
     else
         itm::NO_DEBUG;
}
