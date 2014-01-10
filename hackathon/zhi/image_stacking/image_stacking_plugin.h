/* image_stacking_plugin.h
 * Stack two images
 * 2014-01-08 : by Zhi Zhou
 */
 
#ifndef __IMAGE_STACKING_PLUGIN_H__
#define __IMAGE_STACKING_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class image_stacking : public QObject, public V3DPluginInterface2_1
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


class controlPanel: public QDialog
{
    Q_OBJECT

public:
    controlPanel(V3DPluginCallback2 &v3d, QWidget *parent);
    ~controlPanel();

public:
    QComboBox* combo_subject;
    QComboBox* combo_target;
    QLabel* label_subject;
    QLabel* label_target;
    QLineEdit *m_pLineEdit_filepath;


    v3dhandleList win_list;
    V3DPluginCallback2 & m_v3d;
    QGridLayout *gridLayout;
    static controlPanel *panel;


private slots:
    void _slot_stack();

};

#endif

