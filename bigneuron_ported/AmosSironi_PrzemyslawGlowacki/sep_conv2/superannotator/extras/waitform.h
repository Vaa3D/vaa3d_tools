#ifndef WAITFORM_H
#define WAITFORM_H

#include <QWidget>

namespace Ui {
class WaitForm;
}

class WaitForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit WaitForm(QWidget *parent = 0);
    ~WaitForm();
    
private:
    Ui::WaitForm *ui;
    QMovie *mMovie;
};

// this makes it easy to wait for a thread to finish with a wait dialog
template<typename ThreadType>
void runThreadWithProgress(QWidget *parent, ThreadType *T)
{
    WaitForm *progressDialog = new WaitForm(parent);

    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    progressDialog->show();

    // connect finished signal
    parent->connect( T, SIGNAL(finished()), progressDialog, SLOT(close()) );

    T->start();
}

#endif // WAITFORM_H
