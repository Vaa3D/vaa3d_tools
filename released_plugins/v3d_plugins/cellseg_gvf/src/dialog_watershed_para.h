/****************************************************************************
**
dialog_watershed_para.h
**
****************************************************************************/

#ifndef __DIALOG_WATERSHED_PARA_H__
#define __DIALOG_WATERSHED_PARA_H__

#include <QDialog>

#include "ui_FL_watershedSegPara.h"

class segParameter;
class Image4DSimple;

class dialog_watershed_para : public QDialog, private Ui_Dialog_WatershedPara
{
    Q_OBJECT

public:
    dialog_watershed_para(segParameter  *p, Image4DSimple *imgdata);
    void updateContent(segParameter  *p, Image4DSimple *imgdata);
	void fetchData(segParameter  *p);
	
private:
};

#endif
