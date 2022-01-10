#include <QtGui>

class setTransformDialog : public QDialog
{
    Q_OBJECT

public:
	setTransformDialog(QWidget *parent = 0);
    int getRoot();
	int getWidth();
    int getHeight();
public slots:
    void setRoot(int in);
	void setWidth(int in);
	void setHeight(int in);
	void accept();
signals:
	void root_set();
private:
    float Root;
    float Width;
	float Height;

	QLabel *label_Root;
	QLineEdit *Edit_Root;
	QLabel *label_Width;
	QLineEdit *Edit_Width;
    QLabel *label_Height;
	QLineEdit *Edit_Height;

	QPushButton *okButton;
    QPushButton *cancelButton;

};