
#ifndef __GIT_NEURON_PLUGIN_H__
#define __GIT_NEURON_PLUGIN_H__

#include <QtGui>
#include <QDialog>
#include <v3d_interface.h>

class git_neuron_dialog : public QDialog
{
    Q_OBJECT

public:
    git_neuron_dialog();
	git_neuron_dialog(int a);
    ~git_neuron_dialog();
private:
    void creat();
    void initDlg();
    bool checkbtn();
	void creat1();
    void initDlg1();
    bool checkbtn1();
	void addinfo(QString info, bool bold=false);

public slots:
    bool load0();
    bool load1();
	bool load2();
	bool load3();
	bool load4();
    void run();
	void merge();

public:
    QGridLayout *gridLayout;
    QPushButton *btn_load0, *btn_load1, *btn_load2 , *btn_load3 , *btn_load4;
    QLabel *label_load0, *label_load1 , *label_load2 , *label_load3 , *label_load4;
    QLineEdit *edit_load0, *edit_load1, *edit_load2 , *edit_load3 , *edit_load4;
    QPushButton *btn_quit , *btn_quit2 ,*btn_merge, *btn_run;
	 QTextEdit* text_info;
};



class git_neuron_swc : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

    void do_compare(V3DPluginCallback2 &callback, QWidget *parent);
	void do_merge(V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

