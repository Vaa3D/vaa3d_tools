
#include <QDebug>
#include <QtGui>
#include <QtNetwork>
#include <stdlib.h>
#include "s2Controller.h"



//! [0]
S2Controller::S2Controller(QWidget *parent):   QDialog(parent), networkSession(0)

