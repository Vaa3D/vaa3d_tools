/*
 *  roi_editor.h
 *
 *  Created by Yang, Jinzhu on 11/24/10.
 *
 */

#ifndef __ROI_EDITOR_H__
#define __ROI_EDITOR_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include "v3d_interface.h"

class ROI_Editor_Plugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);
	
public:
	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
	
	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
        {return false;}
        float getPluginVersion() const {return 1.1f;}
};


//define a simple dialog for choose DT parameters
class ParameterDialog : public QDialog
	{
		Q_OBJECT
		
	public:
		
		QGridLayout *gridLayout;
		QLabel* label_imagename;
		QLabel* label_channel;
		QComboBox* combo_channel;
		
		QLabel *labelx;
		
		QPushButton* ok;
		QPushButton* cancel;
		
	public:
		ParameterDialog(V3DPluginCallback2 &cb, QWidget *parent)
		{
			Image4DSimple* image = cb.getImage(cb.currentImageWindow());
			QString curImageName = cb.getImageName(cb.currentImageWindow());		
			v3dhandleList win_list = cb.getImageWindowList();
			//create a dialog
			ok     = new QPushButton("OK");
			cancel = new QPushButton("Cancel");
			gridLayout = new QGridLayout();
			
			labelx = new QLabel(QObject::tr("Target imange"));
			gridLayout->addWidget(labelx, 0,0);
			
			QStringList chList;
			for (int i = 0; i < win_list.size() ;i++ )
			{
				//if (cb.getImageName(win_list[i]) == curImageName)
                  //  continue;
				chList  << cb.getImageName(win_list[i]);
			}
			
			combo_channel = new QComboBox(); 
			
			combo_channel->addItems(chList);
			
			gridLayout->addWidget(combo_channel, 2,0);
			
			
			gridLayout->addWidget(cancel, 6,1); gridLayout->addWidget(ok, 6,0);
			setLayout(gridLayout);
			setWindowTitle(QString("ROI editor"));
			
			connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
			connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		}
		
		~ParameterDialog(){}
		
		public slots:
	};

#endif


