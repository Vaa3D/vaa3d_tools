#include <QtGui>

class setRootDialog : public QDialog
{
    Q_OBJECT

public:
	setRootDialog(QWidget *parent = 0);
    float getX();
	float getY();
    float getZ();
public slots:
    void setX(float in);
	void setY(float in);
	void setZ(float in);
	void accept();
signals:
	void root_set();
private:
    float X;
    float Y;
	float Z;

	QLabel *label_X;
	QLineEdit *Edit_X;
	QLabel *label_Y;
	QLineEdit *Edit_Y;
    QLabel *label_Z;
	QLineEdit *Edit_Z;

	QPushButton *okButton;
    QPushButton *cancelButton;

};