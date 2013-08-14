/****************************************************************************
**
 pointcloud_atlas_io.h
 by Hanchuan Peng
 2009_May-18
**
****************************************************************************/

#ifndef __POINTCLOUND_ATLAS_IO_H__
#define __POINTCLOUND_ATLAS_IO_H__

#include <QtGui>
#include <QList>
#include <vector>

//using namespace std;

struct apoAtlasLinkerInfo
{
	QString className;
	QString registeredFile;
	QString sigFolder, refFolder;
	QStringList sigFolderFileList, refFolderFileList;
//	QString threratio; //FL, 20100909
};

//FL add 20090519
struct apoAtlasLinkerInfoAll
{
//	QString regTargetFileName; 
	QString referenceMarkerName; // FL 20091028
	QList<apoAtlasLinkerInfo> items;
};

//FL add 20110119
struct apoAtlasBuilderInfo
{
    QString linkerFileName;    
	QString regTargetFileName; 
    float ratio;
    QString forceAddCellFileName;
    
    QString refMarkerCptCellNameFile; // name of the reference gene control points
    bool saveRegDataTag; // tag indiating whether to save data before and after registration
    
};


//vector <string> apoAtlasCoexpressionFilelist;

struct apoAtlasCoexpressionDetect
{    
    QStringList apoAtlasFilelist;
    float distanceVal;
    bool symmetryRule;
    bool bilateralRule;    
    QString coExpressionOutputFilePrefix; // in coexpression detection, it is the output the predicted coexpression;
    
};

struct apoAtlasCoexpressionMerge
{
     
    QStringList apoAtlasFilelist;
    QString coExpressionFile; 
//    QString apoFinalAtlasFile; // it is in fact a prefix
    QString apoMergedAtlasFilePrefix; // it is in fact a prefix
    
};


struct atlasConfig
{
    QString inputFolder;
    QString outputFolder;
    
    QString cellNameFilePrefix; // prefix of cell name file for each marker and final atlas
    QString cellNameFileSurfix; // surfix of cell name file for each marker and final atlas
    QStringList cellTypeFileName; // name of the cell type (e.g., interneurons, motoneurons, etc) files 
    QString cellStatFileSurfix; //surfix of the cell statistics files
//    QString coexpressFileSurfix; // surfix of the coexpression file 
    QString markerMapFileName; // gene expression map for each marker

//    QString refMarkerCptCellNameFile; // name of the reference gene control points
//    bool saveRegDataTag; // tag indiating whether to save data before and after registration
    
};


bool loadPointCloudAtlasInfoListFromFile(const char *filename, apoAtlasLinkerInfoAll & apoinfo);
bool savePointCloudAtlasInfoListToFile(const char *filename, const apoAtlasLinkerInfoAll & apoinfo);

void printApo(apoAtlasLinkerInfoAll & apoinfo);

bool isValidCellName(const QString & s); //judge if an annotated cell name is valid
QStringList uniqueCellNameList(const QStringList & longlist); //get a unique cell name list  
bool writeCellNameList2File(const QString & outfile, const QStringList & cellnamelist);

bool alignPointCloudAtlas(const apoAtlasLinkerInfoAll & apo_atlas_info); //this function is used to scan and align apo files for atlas building, phc wrote, not called by atlas builder


bool loadPointCloudAtlasConfigFile(const char *filename, atlasConfig & cfginfo);
bool savePointCloudAtlasConfigFile(const char *filename, const atlasConfig & cfginfo);

#endif
