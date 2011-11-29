#ifndef FINETUNINGDIALOG_H
#define FINETUNINGDIALOG_H

#include <QDialog>
#include <QtGui>
#include "../../component_tree.h"
#include "../widgets/glwidget.h"

namespace Ui {
    class FineTuningDialog;
}

class FineTuningDialog : public QDialog {
    Q_OBJECT
public:
    FineTuningDialog(QWidget *parent = 0);
    ~FineTuningDialog();
	void setParameters(ComponentTree* tree, int label);
	int getLabel() const;
public slots:
	///@brief threshold function used to get threshold result under certen threshold value
	///@param v the threshold value
	///@return m_labels
	void onThreshold(int v);
	void onCreateTree();
	void onLoadTree();
	///@brief slot function to response to choose button, the function will try to find the chooed item and its corresponding label
	///@retval choosed label and set m_mainLabel
	void onChoose();
	void onReset();
	void onCheckCell();
	void onClickedNode(float posX, float posY);
protected:
    void changeEvent(QEvent *e);
		
private:
	int numCells();
	void initCellWidget(int cellnum);
	void updateCellWidget(); // first
	void initGLWidget();     // second getTexture will need update CellWidget
	void updateGLWidget();
	void updateInfoWidget();
	unsigned char * getTexture();

private:
    Ui::FineTuningDialog *ui;
	ComponentTree* m_tree;  ///< the component tree that we want to view
	int m_mainLabel;    ///< the main cell, use label to identify the cell
	vector<int> m_labels; ///< all the cells when changed thresh value
	vector<int> m_colors; // color = red + green * 256 + blue * 256 * 256
	vector<int> m_history;
	vector<QCheckBox*> m_checkers;
	vector<QTextEdit*> m_editors;
	GLWidget* m_glWidget;
};

#endif // FINETUNINGDIALOG_H
