/* AverageFilter_plugin.h
 * This is a average filter plugin, you can use it as a demo.
 * 2018-3-22 : by shengdian jiang
 */
 
#ifndef __AVERAGEFILTER_PLUGIN_H__
#define __AVERAGEFILTER_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

#define MinWindowFiltersize 1
#define MaxWindowFilterSize 4

class AverageFilterPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};
class AverageFilterDialog:public QDialog
{
    Q_OBJECT
public:
    int Wd;
    Image4DSimple *image;
    QPushButton *ok;
    QPushButton *cancel;
    QSpinBox *windowWidth;
    QGridLayout *gridlayout;
public:
    AverageFilterDialog(V3DPluginCallback2 &callback,QWidget *parent)
    {
        image=0;
        gridlayout=0;
        v3dhandleList win_list=callback.getImageWindowList();
        if(win_list.size()<1)
        {
            QMessageBox::information(0,"Average Filter","No image is open");
            //v3d_msg("No image is open");
            return;
        }
        //create a dialog
        gridlayout =new QGridLayout();

        image=callback.getImage(callback.currentImageWindow());

        if(!image||!image->valid())
        {
            v3d_msg("The image is not vaild yet. Check your data");
            return;
        }
        windowWidth=new QSpinBox();
        windowWidth->setRange(MinWindowFiltersize,MaxWindowFilterSize);
        windowWidth->setValue(MinWindowFiltersize);

        gridlayout->addWidget(new QLabel("Average filter window size"),0,0);
        gridlayout->addWidget(windowWidth,0,1,1,5);

        ok     = new QPushButton("OK");
        cancel = new QPushButton("Cancel");
        gridlayout->addWidget(cancel, 1,0);
        gridlayout->addWidget(ok,     1,1,1,5);

        setLayout(gridlayout);
        setWindowTitle(QString("Average Filter"));

        //slot interface
        connect(ok,SIGNAL(clicked()),this,SLOT(accept()));
        connect(cancel,SIGNAL(clicked()),this,SLOT(reject()));
        update();

    }
    ~AverageFilterDialog(){}
public slots:
    void update()
    {
        Wd=windowWidth->text().toInt();
    }

};

#endif

