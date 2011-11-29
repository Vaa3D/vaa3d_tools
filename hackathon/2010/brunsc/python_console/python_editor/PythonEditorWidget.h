#ifndef V3D_PYTHON_EDITOR_H_
#define V3D_PYTHON_EDITOR_H_

#include "ui_python_editor.h"
#include <QMainWindow>

class PythonEditorWidget : public QMainWindow, Ui::python_editor
{
        Q_OBJECT
public:
        PythonEditorWidget(QWidget *parent = NULL);
};

#endif /* V3D_PYTHON_EDITOR_H_ */

