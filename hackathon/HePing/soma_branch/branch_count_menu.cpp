#include<v3d_interface.h>
#include<algorithm>
#include<math.h>
#include"filter_dialog.h"
#include "branch_count_gui.h"
bool branch_count_menu(V3DPluginCallback2 &callback, QWidget *parent){
    CountDialog dialog(parent);
    QString swc_path,save_path;
    if(dialog.exec()!=QDialog::Accepted)return false;
    dialog.update();
//    swc_path=dialog.swc_folder.c_str();
//    save_path=dialog.save_folder.c_str();






    return true;
}


