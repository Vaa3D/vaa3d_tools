# coding:utf-8
"""
visual graph的入口

"""

import sys
from PySide6.QtGui import QIcon, QPixmap
from PySide6.QtCore import QThread
from PySide6.QtWidgets import QApplication
from vep_editor import VisualGraphWindow
from tools.vep_tools import EmittingStream, PrintHelper, StdReceiver
from tools.QssLoader import QSSLoadTool, resource_path

from qt_material import apply_stylesheet

if __name__ == "__main__":

    app = QApplication([])
    app.setStyle('fusion')
    # QSSLoadTool.setStyleSheetFile(app, './qss/main.qss')

    try:
        editor = VisualGraphWindow()
        # todo: 设置VEP图标，当前设置不生效！


        # Qt-material主题样式
        # apply_stylesheet(app, theme='dark_lightgreen.xml')
        editor.show()
    except ValueError as e:
        PrintHelper.printError(e)

    sys.exit(app.exec())
