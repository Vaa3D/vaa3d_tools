/* IVSCC_scaling_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2014-10-30 : by Zhi Zhou
 */
 
#ifndef __IVSCC_SCALING_PLUGIN_H__
#define __IVSCC_SCALING_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

namespace Ui 
{
	class swcScalingUI;
}

void scaleSWC(QStringList params, NeuronTree* nt, QString inputSWCName);

class IVSCC_scaling : public QObject, public V3DPluginInterface2_1
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

class swcScalingUI : public QDialog
{
    Q_OBJECT

public:
	V3DPluginCallback2* callback;
    swcScalingUI(QWidget* parent, V3DPluginCallback2* callback);
	QStringList inputs;
    ~swcScalingUI();

public slots:
	bool okClicked();
	void filePath();
	void checkboxToggled(bool);

private:
    Ui::swcScalingUI* ui;
	NeuronTree inputNt;
	QString SWCfileName;
	QString inputSaveName;
};

#endif

