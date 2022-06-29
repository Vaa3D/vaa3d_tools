#include "myswitch.h"

#include <v3d_interface.h>
#include <QCloseEvent>


void myswitch::beforeClickedSlot()
{
    //获取曾经的指针
    v3dhandleList list = callback->getImageWindowList();
    void* beforeImage = list[list.size() - 2];
    callback->pushImageToTeraWin(beforeImage);

}

void myswitch::afterClickedSlot()
{
    v3dhandleList list = callback->getImageWindowList();
    void* afterImage = list[list.size() - 1];
    callback->pushImageToTeraWin(afterImage);
}

void myswitch::closeEvent(QCloseEvent *event){
    v3dhandleList list = callback->getImageWindowList();
    void* beforeImage = list[list.size() - 2];
    callback->pushImageToTeraWin(beforeImage);

    event->accept();

}
