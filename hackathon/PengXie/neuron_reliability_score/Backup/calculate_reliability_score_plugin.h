/* calculate_reliability_score_plugin.h
 * This plugin will generate the reliability score of neuron reconstruction.
 * 2015-4-27 : by Hanbo Chen, Hang Xiao, Hanchuan Peng
 */
 
#ifndef __CALCULATE_RELIABILITY_SCORE_PLUGIN_H__
#define __CALCULATE_RELIABILITY_SCORE_PLUGIN_H__

#include "QtGui"
#include "v3d_interface.h"
//#include "v3d_message.h"
//#include "vector"
//#include "src/topology_analysis.h"
#include "calculate_reliability_score_main.h"
//#include "fstream"
#include "neuron_format_converter.h"

class neuronScore : public QObject, public V3DPluginInterface2_1
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

class neuronScoreDialog : public QDialog
{
    Q_OBJECT

public:
    neuronScoreDialog(V3DPluginCallback2 &callback, QWidget *parent);
    ~neuronScoreDialog();

private:
    V3DPluginCallback2* callback;
    QWidget *parent;

private:
    void creat();
    void checkButtons();
    void initDlg();

public slots:
    void set_img();
    void set_swc();
    void set_output();
    void accept();

public:
    bool isaccept;
    QLineEdit *edit_img, *edit_swc, *edit_output;
    QPushButton *btn_img, *btn_swc, *btn_output;
    QDoubleSpinBox *spin_radiusRate;
    QComboBox *cb_scoreType;
    QPushButton *btn_quit, *btn_ok;
    QTextEdit *edit_info;
};



void printHelp();

#endif

