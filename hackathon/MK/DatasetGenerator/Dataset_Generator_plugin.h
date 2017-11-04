/* Dataset_Generator_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2017-9-26 : by MK
 */
 
#ifndef __DATASET_GENERATOR_PLUGIN_H__
#define __DATASET_GENERATOR_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <qvector.h>
#include "basic_surf_objs.h"
#include "basic_4dimage.h"

/*inline bool simple_loadimage(const char* filename, unsigned short*& pdata, long int sz[], int& datatype)
{
    if (!filename || !sz) return false;

    Image4DSimple* inimg = 0;
    inimg->loadImage((char *)filename);
    if (!inimg || !inimg->valid()) return false;

    if (pdata) {delete []pdata; pdata=0;}

    V3DLONG totalbytes = inimg->getTotalBytes();
    try
    {
        pdata = new unsigned short [totalbytes];
        if (!pdata) goto Label_error_simple_loadimage;

        memcpy(pdata, inimg->getRawData(), totalbytes);
        datatype = inimg->getUnitBytes(); //1,2,or 4
        sz[0] = inimg->getXDim();
        sz[1] = inimg->getYDim();
        sz[2] = inimg->getZDim();
        sz[3] = inimg->getCDim();
        if (inimg) {delete inimg; inimg=0;}

    }
    catch (...)
    {
        goto Label_error_simple_loadimage;
    }
	return true;

Label_error_simple_loadimage:
    if (inimg) {delete inimg; inimg=0;}
	return false;
}*/

class DatasetGenerator : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
	
/*-------------------------------------------------------------------------------------------------------------------------------------------------*/
	DatasetGenerator();
	
	QString inputSWCdir;
	QString inputImagedir;
	QString inputBkgDir;
	QVector<QString> imageFolders;
	QString outputImagedir;
	void getImageFolders();
	
	double valProportion;
	double proportion1;
	double proportion2;
	void createList();
	void createList2();
	
	void shapeFilter_line_dot();

	void pick_save();

};

#endif

