#include <QtGui>

class ScatterPlotDialog : public QDialog
{
    Q_OBJECT

public:
    ScatterPlotDialog(QWidget *parent = 0);
    int getX();
	int getY();

public slots:
    void setX(int in);
	void setY(int in);
	void accept();
signals:
	void x_changed(int x);
	void y_changed(int y);
private:
    int X;
    int Y;
	QComboBox *ComboBox_X;
	QComboBox *ComboBox_Y;
	QLabel *label_X;
	QLabel *label_Y;

	QPushButton *okButton;
    QPushButton *cancelButton;

};
