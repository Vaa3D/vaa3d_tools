# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'movie_maker_dialog.ui'
#
# Created: Mon Feb 14 18:10:29 2011
#      by: PyQt4 UI code generator 4.8.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_movie_dialog(object):
    def setupUi(self, movie_dialog):
        movie_dialog.setObjectName(_fromUtf8("movie_dialog"))
        movie_dialog.resize(513, 240)
        self.verticalLayout_2 = QtGui.QVBoxLayout(movie_dialog)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.label_2 = QtGui.QLabel(movie_dialog)
        self.label_2.setWordWrap(True)
        self.label_2.setTextInteractionFlags(QtCore.Qt.LinksAccessibleByMouse|QtCore.Qt.TextSelectableByKeyboard|QtCore.Qt.TextSelectableByMouse)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.horizontalLayout.addWidget(self.label_2)
        self.groupBox = QtGui.QGroupBox(movie_dialog)
        self.groupBox.setObjectName(_fromUtf8("groupBox"))
        self.verticalLayout = QtGui.QVBoxLayout(self.groupBox)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.keyFrameLabel = QtGui.QLabel(self.groupBox)
        self.keyFrameLabel.setObjectName(_fromUtf8("keyFrameLabel"))
        self.verticalLayout.addWidget(self.keyFrameLabel)
        self.addCurrentViewButton = QtGui.QPushButton(self.groupBox)
        self.addCurrentViewButton.setObjectName(_fromUtf8("addCurrentViewButton"))
        self.verticalLayout.addWidget(self.addCurrentViewButton)
        self.deleteAllButton = QtGui.QPushButton(self.groupBox)
        self.deleteAllButton.setObjectName(_fromUtf8("deleteAllButton"))
        self.verticalLayout.addWidget(self.deleteAllButton)
        self.horizontalLayout.addWidget(self.groupBox)
        self.verticalLayout_2.addLayout(self.horizontalLayout)
        self.buttonBox = QtGui.QDialogButtonBox(movie_dialog)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Apply|QtGui.QDialogButtonBox.Save)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.verticalLayout_2.addWidget(self.buttonBox)

        self.retranslateUi(movie_dialog)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), movie_dialog.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), movie_dialog.reject)
        QtCore.QMetaObject.connectSlotsByName(movie_dialog)

    def retranslateUi(self, movie_dialog):
        movie_dialog.setWindowTitle(QtGui.QApplication.translate("movie_dialog", "V3D Key-frame Movie Maker", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("movie_dialog", "Adjust the V3D \'3D View\' to the orientation you want to see in your movie.  Then press \'Add current view\'.  Keep changing the view and pressing \'Add current view\' until you have included all of the key frames of your movie.  \n"
"Press \'Play\' to preview your movie.  \n"
"After you are satisfied with your movie, press \'Save\' to write your movie frames to disk. \n"
"Converting frames to a movie file is an exercise left to the user...", None, QtGui.QApplication.UnicodeUTF8))
        self.groupBox.setTitle(QtGui.QApplication.translate("movie_dialog", "Key Frames", None, QtGui.QApplication.UnicodeUTF8))
        self.keyFrameLabel.setText(QtGui.QApplication.translate("movie_dialog", "No key frames added", None, QtGui.QApplication.UnicodeUTF8))
        self.addCurrentViewButton.setText(QtGui.QApplication.translate("movie_dialog", "Add Current View", None, QtGui.QApplication.UnicodeUTF8))
        self.deleteAllButton.setText(QtGui.QApplication.translate("movie_dialog", "Delete All Frames", None, QtGui.QApplication.UnicodeUTF8))

