/* cropped3DImageSeries_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2014-06-30 : by Zhi Zhou
 */
 
#ifndef __CROPPED3DIMAGESERIES_PLUGIN_H__
#define __CROPPED3DIMAGESERIES_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class cropped3DImageSeries : public QObject, public V3DPluginInterface2_1
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

class CropRegionNavigateDialog : public QDialog
{
    Q_OBJECT

public:
        CropRegionNavigateDialog(QWidget *parentWidget, V3DLONG *sz)
    {
        // create a dialog
        label_rgn = new QLabel(QObject::tr("Choose the Cropped 3D Region "));

        // start
        xs=1; ys=1; zs=1;

        v_xs = new QSpinBox();
        v_ys = new QSpinBox();
        v_zs = new QSpinBox();

        v_xs->setMaximum(sz[0]); v_xs->setMinimum(1); v_xs->setValue(1); v_xs->setSingleStep(1);
        v_ys->setMaximum(sz[1]); v_ys->setMinimum(1); v_ys->setValue(1); v_ys->setSingleStep(1);
        v_zs->setMaximum(sz[2]); v_zs->setMinimum(1); v_zs->setValue(1); v_zs->setSingleStep(1);

        label_xs = new QLabel(QObject::tr(" Start position x: "));
        label_ys = new QLabel(QObject::tr(" y: "));
        label_zs = new QLabel(QObject::tr(" z: "));
        label_channel = new QLabel(QObject::tr(" Channel (select 0 to crop the image in all channels): "));

        // end
        v_xe = new QSpinBox();
        v_ye = new QSpinBox();
        v_ze = new QSpinBox();
        channel = new QSpinBox();

        v_xe->setMaximum(sz[0]); v_xe->setMinimum(xs); v_xe->setValue(sz[0]); v_xe->setSingleStep(1);
        v_ye->setMaximum(sz[1]); v_ye->setMinimum(ys); v_ye->setValue(sz[1]); v_ye->setSingleStep(1);
        v_ze->setMaximum(sz[2]); v_ze->setMinimum(zs); v_ze->setValue(sz[2]); v_ze->setSingleStep(1);

        channel->setMaximum(sz[3]); v_ze->setMinimum(0);

        label_xe = new QLabel(QObject::tr(" End position x: "));
        label_ye = new QLabel(QObject::tr(" y: "));
        label_ze = new QLabel(QObject::tr(" z: "));

        // button
        ok     = new QPushButton("OK");
        cancel = new QPushButton("Cancel");


        // gridlayout
        QGridLayout* gridLayout = new QGridLayout();

        gridLayout->addWidget(label_rgn, 0,0);

        gridLayout->addWidget(label_xs, 1,0,1,1); gridLayout->addWidget(v_xs, 1,1,1,1);
        gridLayout->addWidget(label_ys, 1,2,1,1); gridLayout->addWidget(v_ys, 1,3,1,1);
        gridLayout->addWidget(label_zs, 1,4,1,1); gridLayout->addWidget(v_zs, 1,5,1,1);

        gridLayout->addWidget(label_xe, 5,0,1,1); gridLayout->addWidget(v_xe, 5,1,1,1);
        gridLayout->addWidget(label_ye, 5,2,1,1); gridLayout->addWidget(v_ye, 5,3,1,1);
        gridLayout->addWidget(label_ze, 5,4,1,1); gridLayout->addWidget(v_ze, 5,5,1,1);

        gridLayout->addWidget(label_channel, 7,0,1,1); gridLayout->addWidget(channel, 7,5,1,1);

        gridLayout->addWidget(cancel, 8,4,1,1); gridLayout->addWidget(ok, 8,5,1,1);

        setLayout(gridLayout);
        setWindowTitle(QString("Point Navigation"));

        // signal and slot
        connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
        connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

        connect(v_xs, SIGNAL(valueChanged(int)), this, SLOT(update()));
        connect(v_ys, SIGNAL(valueChanged(int)), this, SLOT(update()));
        connect(v_zs, SIGNAL(valueChanged(int)), this, SLOT(update()));

        connect(v_xe, SIGNAL(valueChanged(int)), this, SLOT(update()));
        connect(v_ye, SIGNAL(valueChanged(int)), this, SLOT(update()));
        connect(v_ze, SIGNAL(valueChanged(int)), this, SLOT(update()));

        connect(channel, SIGNAL(valueChanged(int)), this, SLOT(update()));

    }

    ~CropRegionNavigateDialog(){}

    public slots:
    void update()
    {
        xs = v_xs->text().toLong() - 1;
        ys = v_ys->text().toLong() - 1;
        zs = v_zs->text().toLong() - 1;

        v_xe->setMinimum(xs+1);
        v_ye->setMinimum(ys+1);
        v_ze->setMinimum(zs+1);

        xe = v_xe->text().toLong() - 1;
        ye = v_ye->text().toLong() - 1;
        ze = v_ze->text().toLong() - 1;

        c = channel->text().toInt()-1;
    }

public:
    long xs, ys, zs, xe, ye, ze;
    int c;

    QLabel *label_rgn;

    QLabel *label_xs;
    QLabel *label_ys;
    QLabel *label_zs;
    QLabel *label_channel;


    QSpinBox* v_xs;
    QSpinBox* v_ys;
    QSpinBox* v_zs;
    QSpinBox* channel;


    QLabel *label_xe;
    QLabel *label_ye;
    QLabel *label_ze;

    QSpinBox* v_xe;
    QSpinBox* v_ye;
    QSpinBox* v_ze;

    QPushButton* ok;
    QPushButton* cancel;

};

#endif

