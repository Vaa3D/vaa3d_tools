/* mostVesselTracer.h
 * by jpwu@CBMP,20100419
 */


#ifndef __MOSTVESSELTRACER_H__
#define __MOSTVESSELTRACER_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <vector>


#include "v3d_interface.h"
#include "v3d_core.h"
#include "basic_4dimage.h"
#include "v3d_message.h"

#include "mostimage.h"
#include <QApplication>
#include<QDialog>


class mostVesselTracerPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

private:
        // static const float pluginVersion = 1.1; // not in msvc


public:
//	QStringList menulist() const;
//	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
//
//	QStringList funclist() const {return QStringList();}
//	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
//

	QStringList menulist() const;
	void domenu(const QString & menu_name, V3DPluginCallback2 & v3d,  QWidget * parent);

	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
                 V3DPluginCallback2 & v3d,  QWidget * parent);

        // plugin interface version 2.1 requires plugin version
        float getPluginVersion() const {return 1.3f;}


};


class AdaTDialog : public QDialog
{
        Q_OBJECT

public:
        QGridLayout *gridLayout;

        QLabel *labelx;
        QLabel *labely;
        QLabel *labelz;
        QLabel *threshould;
        QLabel *resolution;
        QLabel *seed_size;
        QLabel *slip_size;
        QLabel *begin;
        QLabel *end;
        QLabel *distance;
        QLabel *labelx_res;
        QLabel *labely_res;
        QLabel *labelz_res;
        QLabel *detect_seed_seting;
        QLabel *kongge;
        QLabel *save_as;
        QLabel *label_channel;

        QSpinBox *threshould_value;
        QSpinBox *size;
        QSpinBox *slipsize;
        QSpinBox *channel;

        QDoubleSpinBox* resx;
        QDoubleSpinBox* resy;
        QDoubleSpinBox* resz;


        QSpinBox* beginx;
        QSpinBox* beginy;
        QSpinBox* beginz;

        QSpinBox* endx;
        QSpinBox* endy;
        QSpinBox* endz;

        QSpinBox* distancex;
        QSpinBox* distancey;
        QSpinBox* distancez;


        QPushButton* ok;
        QPushButton* cancel;
        QCheckBox* x_select;
        QCheckBox* y_select;
        QCheckBox* z_select;
        QCheckBox* pruning;

        QLineEdit* ds;

        V3DLONG Dn;
        V3DLONG Dh;

public:
        AdaTDialog()
        {
            labelx = new QLabel(QObject::tr("x"));
            labely = new QLabel(QObject::tr("y"));
            labelz = new QLabel(QObject::tr("z"));
            labelx_res = new QLabel(QObject::tr("x"));
            labely_res = new QLabel(QObject::tr("y"));
            labelz_res = new QLabel(QObject::tr("z"));
            threshould = new QLabel(QObject::tr("Threshould"));
            resolution = new QLabel(QObject::tr("Resolution"));
            seed_size = new QLabel(QObject::tr("Seed_size"));
            slip_size = new QLabel(QObject::tr("Slip_size"));
            begin = new QLabel(QObject::tr("Begin"));
            end = new QLabel(QObject::tr("End"));
            distance = new QLabel(QObject::tr("Distance"));
            detect_seed_seting = new QLabel(QObject::tr("------------Detect_seed_seting------------"));
            save_as = new QLabel(QObject::tr("save the swc result as:"));
            kongge = new QLabel(QObject::tr("      "));
            label_channel = new QLabel(QObject::tr("Channel"));

            threshould_value = new QSpinBox();
            size = new QSpinBox();
            slipsize = new QSpinBox();
            beginx = new QSpinBox();
            beginy = new QSpinBox();
            beginz = new QSpinBox();
            endx = new QSpinBox();
            endy = new QSpinBox();
            endz = new QSpinBox();
            distancex = new QSpinBox();
            distancey = new QSpinBox();
            distancez = new QSpinBox();

            channel = new QSpinBox();

            resx = new QDoubleSpinBox();
            resy = new QDoubleSpinBox();
            resz = new QDoubleSpinBox();
            x_select = new QCheckBox();
            y_select = new QCheckBox();
            z_select = new QCheckBox();
            pruning = new QCheckBox();

            ds = new QLineEdit();

            threshould_value->setMaximum(255); threshould_value->setMinimum(1);threshould_value->setValue(20);

            size->setMaximum(5000);size->setMinimum(1);size->setValue(6);
            slipsize->setMaximum(5000);slipsize->setMinimum(1);slipsize->setValue(20);

            resx->setMaximum(100);resx->setMinimum(0.001);resx->setValue(1);
            resy->setMaximum(100);resy->setMinimum(0.001);resy->setValue(1);
            resz->setMaximum(100);resz->setMinimum(0.001);resz->setValue(1);
            beginx->setMaximum(10000);beginx->setMinimum(0);beginx->setValue(1);
            beginy->setMaximum(10000);beginy->setMinimum(0);beginy->setValue(1);
            beginz->setMaximum(10000);beginz->setMinimum(0);beginz->setValue(1);
            endx->setMaximum(10000);endx->setMinimum(0);endx->setValue(1143);
            endy->setMaximum(10000);endy->setMinimum(0);endy->setValue(1143);
            endz->setMaximum(10000);endz->setMinimum(0);endz->setValue(1143);
            distancex->setMaximum(10000);distancex->setMinimum(0);distancex->setValue(20);
            distancey->setMaximum(10000);distancey->setMinimum(0);distancey->setValue(20);
            distancez->setMaximum(10000);distancez->setMinimum(0);distancez->setValue(20);
            x_select->setText("x");x_select->setChecked(1);
            y_select->setText("y");y_select->setChecked(1);
            z_select->setText("z");z_select->setChecked(1);
            pruning->setText("segmentPruning or not");pruning->setChecked(0);
            ds->setText("/tmp/result.swc");
            channel->setMaximum(3); channel->setMinimum(1);channel->setValue(1);


            ok     = new QPushButton("OK");
            cancel = new QPushButton("Cancel");

            QVBoxLayout *thred_layout = new QVBoxLayout;
            thred_layout->addWidget(threshould);
            thred_layout->addWidget(threshould_value);
            QVBoxLayout *size_layout = new QVBoxLayout;
            size_layout->addWidget(seed_size);
            size_layout->addWidget(size);
            QVBoxLayout *slipsize_layout = new QVBoxLayout;
            slipsize_layout->addWidget(slip_size);
            slipsize_layout->addWidget(slipsize);
            QVBoxLayout *channel_layout = new QVBoxLayout;
            channel_layout->addWidget(label_channel);
            channel_layout->addWidget(channel);

            QVBoxLayout *res_lab_layout = new QVBoxLayout;
            res_lab_layout->addStretch();
            res_lab_layout->addWidget(resolution);

            QVBoxLayout *resx_layout = new QVBoxLayout;
            resx_layout->addWidget(labelx_res);
            resx_layout->addWidget(resx);
            QVBoxLayout *resy_layout = new QVBoxLayout;
            resy_layout->addWidget(labely_res);
            resy_layout->addWidget(resy);
            QVBoxLayout *resz_layout = new QVBoxLayout;
            resz_layout->addWidget(labelz_res);
            resz_layout->addWidget(resz);

            QHBoxLayout *save = new QHBoxLayout;
            save->addWidget(save_as);
            save->addWidget(ds);

            QHBoxLayout *top_layout = new QHBoxLayout;
            top_layout ->addLayout(thred_layout);
            top_layout ->addLayout(size_layout);
            top_layout ->addLayout(slipsize_layout);
            top_layout ->addLayout(channel_layout);
            top_layout ->addLayout(res_lab_layout);
            top_layout ->addLayout(resx_layout);
            top_layout ->addLayout(resy_layout);
            top_layout ->addLayout(resz_layout);

            QVBoxLayout *detect_lab_layout = new QVBoxLayout;
           // detect_lab_layout->addStretch();
            detect_lab_layout->addWidget(kongge);
            detect_lab_layout->addWidget(x_select);
            detect_lab_layout->addWidget(y_select);
            detect_lab_layout->addWidget(z_select);
            QVBoxLayout *begin_layout = new QVBoxLayout;
            begin_layout->addWidget(begin);
            begin_layout->addWidget(beginx);
            begin_layout->addWidget(beginy);
            begin_layout->addWidget(beginz);
            QVBoxLayout *end_layout = new QVBoxLayout;
            end_layout->addWidget(end);
            end_layout->addWidget(endx);
            end_layout->addWidget(endy);
            end_layout->addWidget(endz);
            QVBoxLayout *distance_layout = new QVBoxLayout;
            distance_layout->addWidget(distance);
            distance_layout->addWidget(distancex);
            distance_layout->addWidget(distancey);
            distance_layout->addWidget(distancez);
            QHBoxLayout *down_layout = new QHBoxLayout;
          //  down_layout->addStretch();
            down_layout->addLayout(detect_lab_layout);
            down_layout->addLayout(begin_layout);
            down_layout->addLayout(end_layout);
            down_layout->addLayout(distance_layout);

            QHBoxLayout *button_layout = new QHBoxLayout;
            button_layout->addStretch();
            button_layout->addWidget(cancel);
            button_layout->addWidget(ok);

            QVBoxLayout *main_layout = new QVBoxLayout;
            main_layout->addLayout(top_layout);
            main_layout->addWidget(pruning);
            main_layout->addLayout(save);

            main_layout->addWidget(detect_seed_seting);
            main_layout->addLayout(down_layout);
            main_layout->addLayout(button_layout);

            setLayout(main_layout);

            connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
            connect(x_select,SIGNAL(clicked(bool)),beginx,SLOT(setEnabled(bool)));
            connect(x_select,SIGNAL(clicked(bool)),endx,SLOT(setEnabled(bool)));
            connect(x_select,SIGNAL(clicked(bool)),distancex,SLOT(setEnabled(bool)));
            connect(y_select,SIGNAL(clicked(bool)),beginy,SLOT(setEnabled(bool)));
            connect(y_select,SIGNAL(clicked(bool)),endy,SLOT(setEnabled(bool)));
            connect(y_select,SIGNAL(clicked(bool)),distancey,SLOT(setEnabled(bool)));
            connect(z_select,SIGNAL(clicked(bool)),beginz,SLOT(setEnabled(bool)));
            connect(z_select,SIGNAL(clicked(bool)),endz,SLOT(setEnabled(bool)));
            connect(z_select,SIGNAL(clicked(bool)),distancez,SLOT(setEnabled(bool)));
           // connect(ok,SIGNAL(clicked()), this, SLOT(accept()));)

                //create a dialog
                setWindowTitle(QString("Change parameters"));
        }

        ~AdaTDialog(){}

        public slots:
        //void update();
};


void setSeeds(V3DPluginCallback2 &v3d, QWidget *parent);
void startVesselTracing(V3DPluginCallback2 &v3d,int xflag,int yflag,int zflag,int xbegin, int xend,int xdis,int ybegin,int yend,int ydis,int zbegin,int zend,int zdis,QString swcfile,int slipsize,int pruning_flag, int c);
void set_dialog(V3DPluginCallback2 &v3d, QWidget *parent);


static   int InitThreshold;
static   int seed_size_all;
static   double res_x_all;
static   double res_y_all;
static   double res_z_all;
static   int x_flag;
static   int y_flag;
static   int z_flag;
/*

*/

// the visited flag
static std::vector < bool >  visited;

#endif

