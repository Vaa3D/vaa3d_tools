/* IVSCC_sort_swc_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2016-5-16 : by Zhi Zhou
 */
 
#ifndef __IVSCC_SORT_SWC_PLUGIN_H__
#define __IVSCC_SORT_SWC_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class TestPlugin : public QObject, public V3DPluginInterface2_1
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

#endif

class swcSortDialog : public QDialog
    {
        Q_OBJECT

    public:
        swcSortDialog(V3DPluginCallback2 &cb, QWidget *parent,QList<NeuronSWC> neuron)
        {

            QGridLayout * layout = new QGridLayout();
            rootid_box = new QSpinBox();
            thres_box = new QSpinBox();

            rootid_box->setMinimum(0);
            rootid_box->setMaximum(neuron.size());
            rootid_box->setValue(1);


            thres_box->setMinimum(-1);
            thres_box->setMaximum(2147483647);


            layout->addWidget(new QLabel("Root number (If you set '0', the first root in file is set as default):"), 1, 0, 1, 1);
            layout->addWidget(rootid_box, 1, 1, 1, 5);
            layout->addWidget(new QLabel("The new threshold for the newly generated link (If you set '-1', all the points will be connected automated; If you set '0', no new link will be generated):"), 2, 0, 1, 1);
            layout->addWidget(thres_box, 2, 1, 1, 5);
            QPushButton * ok = new QPushButton("Ok");
            QPushButton * cancel = new QPushButton("Cancel");
            ok->setDefault(true);
            layout->addWidget(ok, 3, 0, 1, 3);
            layout->addWidget(cancel, 3, 3, 1, 3);
            this->setLayout(layout);
            connect(ok, SIGNAL(clicked()), this, SLOT(_slot_start()));
            connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
        }

        ~swcSortDialog(){}

        public slots:
        void update()
        {
            rootid = atof(rootid_box->text().toStdString().c_str());
            thres = atof(thres_box->text().toStdString().c_str());
        }

        void _slot_start()
        {
            update();
            accept();
        }

    public:
        QSpinBox * rootid_box;
        QSpinBox *  thres_box;

        V3DLONG rootid;
        V3DLONG thres;
    };
