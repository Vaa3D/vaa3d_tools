import movie_maker
import os

# User needs to install either PyQt4 or PySide to get GUI
try:
    from PySide import QtGui, QtCore
    from ui_movie_maker_dialog_pyside import Ui_movie_dialog
    # import PySide as PyQt4
except ImportError:
    try:
        from PyQt4 import QtGui, QtCore
        # Hack to get around defect in pyuic4
        _ql = QtGui.QLineEdit
        if not hasattr(_ql, 'setPlaceholderText'):
            def foo(x,y):
                pass
            _ql.setPlaceholderText = foo
        # Use same syntax for pythonic signals/slots
        if not hasattr(QtCore, "Signal"):
            QtCore.Signal = QtCore.pyqtSignal
            QtCore.Slot = QtCore.pyqtSlot
        from ui_movie_maker_dialog_pyqt4 import Ui_movie_dialog
    except ImportError:
        print """
To use V3D Movie GUI, you must install either PySide or PyQt4.
    http://www.pyside.org/
      OR
    http://www.riverbankcomputing.co.uk/software/pyqt/download
            """
        raise


class SingleKeyFrameLabel(QtGui.QLabel):
    """
    Cartoon representation of a single frame, for use in movie GUI.
    """
    go_to_frame = QtCore.Signal(object)
    delete_frame = QtCore.Signal(object)
    replace_frame = QtCore.Signal(object)
    
    def __init__(self, parent=None, frame=None):
        QtGui.QLabel.__init__(self, parent)
        self.frame = frame
        self.setAlignment(QtCore.Qt.AlignHCenter | QtCore.Qt.AlignVCenter)
        iconPath = os.path.join(os.path.dirname(movie_maker.__file__), 'icons')
        self.frameIcon = QtGui.QIcon(os.path.join(iconPath, "film_frame.png"))
        self.setPixmap(self.frameIcon.pixmap(50,50))
        self.setFixedSize(QtCore.QSize(50, 50))
        self.setToolTip("movie key frame")
        # Context menu for deleting frame
        self.setContextMenuPolicy(QtCore.Qt.ActionsContextMenu)
        # Delete
        deleteAction = QtGui.QAction("Delete this frame", self)
        self.addAction(deleteAction)
        self.connect(deleteAction, QtCore.SIGNAL("triggered()"), 
                     self.onDelete)
        # Replace with current
        replaceAction = QtGui.QAction("Replace frame with current view", self)
        self.addAction(replaceAction)
        self.connect(replaceAction, QtCore.SIGNAL("triggered()"),
                     self.onReplaceView)
        
    def mouseMoveEvent(self, e):
        if e.buttons() != QtCore.Qt.RightButton:
            return
        mimeData = QtCore.QMimeData()
        drag = QtGui.QDrag(self)
        drag.setMimeData(mimeData)
        drag.setHotSpot(e.pos() - self.rect().topLeft())
        dropAction = drag.start(QtCore.Qt.MoveAction)

    def mousePressEvent(self, e):
        QtGui.QLabel.mousePressEvent(self, e)
        if e.button() == QtCore.Qt.LeftButton:
            pass
            # print 'press'
 
    def mouseDoubleClickEvent(self, event):
        "After double click, set V3D 3D viewer to this key frame"
        # print "double click sent"
        self.go_to_frame.emit(self.frame)
        
    def onDelete(self):
        self.delete_frame.emit(self)
        
    def onReplaceView(self):
        # print "replacing"
        self.replace_frame.emit(self)
        

class FrameTransitionLabel(QtGui.QLabel):
    """
    Cartoon representation transition between two frames, for use in movie GUI.
    """
    def __init__(self, parent=None, frame=None):
        QtGui.QLabel.__init__(self, parent)
        self.frame = frame
        self.setAlignment(QtCore.Qt.AlignHCenter | QtCore.Qt.AlignVCenter)
        iconPath = os.path.join(os.path.dirname(movie_maker.__file__), 'icons')
        self.arrowIcon = QtGui.QIcon(os.path.join(iconPath, "arrow_right.png"))
        self.setPixmap(self.arrowIcon.pixmap(50,30))
        self.setFixedSize(QtCore.QSize(50, 50))
        self.setToolTip("Transition takes %.2f seconds" % frame.interval)
        
        
class MovieGui(QtGui.QDialog):
    def __init__(self, parent=None):
        QtGui.QDialog.__init__(self, parent)
        self.ui = Ui_movie_dialog()
        self.ui.setupUi(self)
        self.setAcceptDrops(True)
        self.movie = movie_maker.V3dMovie()
        iconPath = os.path.join(os.path.dirname(movie_maker.__file__), 'icons')
        self.playIcon = QtGui.QIcon(os.path.join(iconPath, "play.png"))
        self.recordIcon = QtGui.QIcon(os.path.join(iconPath, "record.png"))
        self.pauseIcon = QtGui.QIcon(os.path.join(iconPath, "pause.png"))
        self.reelIcon = QtGui.QIcon(os.path.join(iconPath, "film_reel.png"))
        self.skipBackIcon = QtGui.QIcon(os.path.join(iconPath, "skip_backward.png"))
        self.skipAheadIcon = QtGui.QIcon(os.path.join(iconPath, "skip_ahead.png"))
        self.setWindowIcon(self.reelIcon)
        self.ui.frameCartoonLabel.hide() # its' just a placeholder
        self.configure_buttons()
        self.previous_save_dir = ""
        self._updateFrameCount()
        # Frame interval
        interval_validator = QtGui.QDoubleValidator(0.00, 10000.0, 2, 
                self.ui.frameIntervalSelector)
        self.ui.frameIntervalSelector.setValidator(interval_validator)
        self.connect(self.ui.frameIntervalSelector, QtCore.SIGNAL('textChanged(QString)'),
                     self.update_frame_interval)
        self.frame_interval = 2.5
        ix = self.ui.frameIntervalSelector.findText("2.50")
        self.ui.frameIntervalSelector.setCurrentIndex(ix)
        self._enter_state('ready')
        
    def configure_buttons(self):
        self.connect(self.ui.addCurrentViewButton, QtCore.SIGNAL('clicked()'), 
               self.append_view)
        self.beginningButton = self.ui.playButtonBox.button(QtGui.QDialogButtonBox.Reset)
        self.beginningButton.setText('First')
        self.beginningButton.setToolTip("Go to the first key frame of your movie")
        # self.beginningButton = self.ui.buttonBox.addButton('Beginning', QtGui.QDialogButtonBox.ActionRole)
        self.beginningButton.setIcon(self.skipBackIcon)
        self.beginningButton.setEnabled(False)
        self.connect(self.beginningButton, QtCore.SIGNAL('clicked()'),
                     self.on_beginning_pressed)
        # self.playButton = self.ui.buttonBox.button(QtGui.QDialogButtonBox.Apply)
        # self.ui.buttonBox.button(QtGui.QDialogButtonBox.Apply).hide()
        self.playButton = self.ui.playButtonBox.addButton('Play', QtGui.QDialogButtonBox.ActionRole)
        self.playButton.setText("Play")
        self.playButton.setToolTip("Preview your movie")
        self.playButton.setIcon(self.playIcon)
        self.playButton.setMinimumWidth(100)
        self.connect(self.playButton, QtCore.SIGNAL('clicked()'),
               self.on_play_pause_pressed)
        self.endButton = self.ui.playButtonBox.addButton('Last', QtGui.QDialogButtonBox.ActionRole)
        self.endButton.setToolTip("Go to the final frame of your movie")
        self.endButton.setIcon(self.skipAheadIcon)
        self.endButton.setEnabled(False)
        self.connect(self.endButton, QtCore.SIGNAL('clicked()'),
                     self.on_end_pressed)
        self.saveImagesButton = self.ui.buttonBox.button(QtGui.QDialogButtonBox.Save)
        self.saveImagesButton.setText("Save images...")
        self.connect(self.saveImagesButton, QtCore.SIGNAL('clicked()'),
               self.save_images)
        # self.saveParametersButton = self.ui.buttonBox.addButton('Save', QtGui.QDialogButtonBox.ApplyRole)
        self.saveParametersButton = self.ui.buttonBox.button(QtGui.QDialogButtonBox.SaveAll)
        self.saveParametersButton.setText("Save...")
        self.saveParametersButton.setToolTip("Save your movie key frame parameters to a text file (.vmv format)")
        self.connect(self.saveParametersButton, QtCore.SIGNAL('clicked()'),
               self.save_parameters)
        # deleteAllButton used to be a generic button
        self.ui.deleteAllButton = self.ui.buttonBox.button(QtGui.QDialogButtonBox.Reset)
        self.ui.deleteAllButton.setText('Clear')
        self.ui.deleteAllButton.setToolTip("Delete all frames to start an empty movie")
        self.connect(self.ui.deleteAllButton, QtCore.SIGNAL('clicked()'),
               self.delete_all)
        self.loadButton = self.ui.buttonBox.button(QtGui.QDialogButtonBox.Open)
        self.loadButton.setToolTip("Load movie key frame parameters from a .vmv file")
        self.connect(self.loadButton, QtCore.SIGNAL('clicked()'),
                     self.load_parameters)
        self.helpButton = self.ui.playButtonBox.button(QtGui.QDialogButtonBox.Help)
        self.connect(self.helpButton, QtCore.SIGNAL('clicked()'),
                     self.help)
        self.ui.buttonBox.button(QtGui.QDialogButtonBox.Close).setToolTip(
                    "Close the movie maker window")
        self.ui.buttonBox.button(QtGui.QDialogButtonBox.Save).setToolTip(
                    "Write movie frame images to disk, so you can make a video file from them")

    def help(self):
        QtGui.QMessageBox.information(self, 
                "About V3D movie maker", 
                """
First adjust the V3D 3D window to the view you want to begin your movie with.  Next press "Add current view".  
Continue alternately changing the view and pressing "Add current view".

Press "Play" to preview your movie.

Press "Save images..." to save the movie frames to disk.
                """)
        
    def update_frame_interval(self, value_text):
        self.frame_interval = float(value_text)
        
    def show_frame(self, frame):
        self.movie.set_current_v3d_camera(frame.camera_position)
        self.movie.image_window.update() # The obviates need to print to get window update

    def delete_frame_cartoon(self, frame_cartoon):
        self.movie.key_frames.remove(frame_cartoon.frame)
        # TODO - remove label
        layout = self.ui.keyLabelFrame.layout()
        # Always remove any arrow label to the left of the frame label
        if frame_cartoon.arrow_label:
            layout.removeWidget(frame_cartoon.arrow_label)
        # TODO - if it's the first label, we should remove next one's arrow
        if layout.indexOf(frame_cartoon) == 1:
            if layout.count() >= 4:
                next_frame = layout.itemAt(3)
                next_frame.arrow_label = None
                print "deleting second frame arrow"
                layout.removeItem(layout.itemAt(2))
        layout.removeWidget(frame_cartoon)
        self._updateFrameCount()
        self._enter_state('ready')
        
    def replace_frame_cartoon(self, frame_cartoon):
        # print "replaced"
        frame_cartoon.frame.camera_position = self.movie.get_current_v3d_camera()
        
    def append_view(self):
        "Add a new key frame based on the current view in the V3D 3D viewer"
        # Perhaps there was no 3D viewer when the MovieGui was launched
        try:
            self.movie.append_current_view(interval=self.frame_interval)
        except ValueError:
            self.movie = movie_maker.V3dMovie()
            try:
                self.movie.append_current_view(interval=self.frame_interval)
            except:
                QtGui.QMessageBox.information(self,
                        "Movie maker cannot find a V3D 3D window.",
                        "Movie maker cannot find a V3D 3D window.  Perhaps you need to open one")
                return
        frame = self.movie.key_frames[-1]
        self.append_cartoon_frame(frame)
        self._updateFrameCount()
        self._enter_state('ready') # stop current animation

    def append_cartoon_frame(self, frame):
        "Add one frame cartoon to the cartoon panel"
        layout = self.ui.keyLabelFrame.layout()
        frame_cartoon = SingleKeyFrameLabel(self, frame)
        frame_cartoon.arrow_label = None
        if 1 < layout.count():
            # arrow
            arrow = FrameTransitionLabel(self, frame)
            layout.addWidget(arrow)
            frame_cartoon.arrow_label = arrow # so we can delete it at the same time
        frame_cartoon.go_to_frame.connect(self.show_frame)
        frame_cartoon.delete_frame.connect(self.delete_frame_cartoon)
        frame_cartoon.replace_frame.connect(self.replace_frame_cartoon)
        layout.addWidget(frame_cartoon)
        
    # There are three states - ready, playing, and paused
    def _enter_state(self, state):
        "Transition movie maker GUI to a new state, such as 'ready', 'playing', etc."
        # Ready to play from start
        self.play_state = state
        if 'ready' == state:
            self.play_generator = None
            self.playButton.setText('Play')
            self.playButton.setIcon(self.playIcon)
            self._updateFrameCount() # enable play button?
            self.ui.playbackLabel.setText('  (idle)  ')
        elif 'playing' == state:
            if None == self.play_generator:
                self.play_generator = self.movie.generate_play_frames()
            self.playButton.setText('Pause')
            self.playButton.setIcon(self.pauseIcon)
            self.endButton.setEnabled(True)
            self.beginningButton.setEnabled(True)
        elif 'paused' == state:
            assert None != self.play_generator
            self.playButton.setText('Play')
            self.playButton.setIcon(self.playIcon)
            self.endButton.setEnabled(True)
            self.beginningButton.setEnabled(True)
        else:
            assert(False)
        
    def on_play_pause_pressed(self):
        if ('ready' == self.play_state) or ('paused' == self.play_state): # play
            self._enter_state('playing')
            for elapsed_time in self.play_generator:
                self.update_play_time(elapsed_time)
                QtGui.QApplication.processEvents()
                if 'playing' != self.play_state:
                    return
            self._enter_state('ready')
        elif 'playing' == self.play_state:
            self._enter_state('paused')
        else: # stop
            assert(False)

    def update_play_time(self, elapsed_time):
        minutes = int(elapsed_time / 60.0)
        seconds = int(elapsed_time - minutes * 60)
        hundredths = int((elapsed_time - seconds - minutes * 60) * 100)
        time_string = "%02dm:%02d.%02ds" % (minutes, seconds, hundredths)
        self.ui.playbackLabel.setText(time_string)
        
    def on_beginning_pressed(self):
        self.movie.generate_play_frames().next()
        self._enter_state('ready')
        
    def on_end_pressed(self):
        self.movie.generate_final_frame_view()
        self._enter_state('ready')

    def save_images(self):
        dir = QtGui.QFileDialog.getExistingDirectory(
                    self,
                    "Choose directory to save frame files in",
                    self.previous_save_dir)
        if os.path.exists(dir):
            self.previous_save_dir = dir
            for n in self.movie.generate_write_frames(directory=str(dir)):
                QtGui.QApplication.processEvents()
            answer = QtGui.QMessageBox.information(self, "Frames saved", 
                 "Finished writing movie frames")
        self.show() # why does window get hidden in this method?

    def save_parameters(self):
        fname = QtGui.QFileDialog.getSaveFileName(
                    self,
                    "Choose file to save frame parameters in",
                    self.previous_save_dir,
                    "V3D movie files (*.vmv)")[0]
        if None == fname: return
        if len(fname) < 1: return
        fname = str(fname)
        # Note: Qt dialog already asks for confirmation.
        dir = os.path.dirname(fname)
        self.previous_save_dir = dir
        file_object = open(fname, 'w')
        self.movie.save_parameter_file(file_object)
        file_object.close()
        answer = QtGui.QMessageBox.information(self, "Parameters saved", 
             "Finished saving movie parameters file")
        self.show() # why does window get hidden in this method?

    def load_parameters(self):
        result = QtGui.QFileDialog.getOpenFileName(
                        self,
                        "Open V3D movie parameters file",
                        self.previous_save_dir,
                        "V3D movie files (*.vmv)")
        self.show() # why does window get hidden in this method?
        if None == result: return
        if 2 > len(result): return
        fname = result[0]
        if None == fname: return
        if len(fname) < 1: return
        if not os.path.exists(fname): 
            QtGui.QMessageBox.information(this, "No such file", "No such file")
            return
        file_object = open(fname, 'r')
        self.movie.load_parameter_file(file_object)
        self.clear_cartoon_panel()
        for frame in self.movie.key_frames:
            self.append_cartoon_frame(frame)
        self._updateFrameCount()
        self.show() # why does window get hidden in this method?
        
    def clear_cartoon_panel(self):
        # Remove the cartoons
        cartoon = self.ui.keyLabelFrame.layout().takeAt(0)
        while None != cartoon:
            cartoon = self.ui.keyLabelFrame.layout().takeAt(0)
        
    def delete_all(self):
        answer = QtGui.QMessageBox.question(self, "Confirm clear movie", 
                 "Really erase all key-frames?", 
                 QtGui.QMessageBox.Yes | QtGui.QMessageBox.No, 
                 QtGui.QMessageBox.No)
        if answer == QtGui.QMessageBox.Yes:
            self.movie.key_frames = []
            self.clear_cartoon_panel()
            self._updateFrameCount()
            self._enter_state('ready')

    def _updateFrameCount(self):
        nframes = len(self.movie.key_frames)
        if nframes == 0:
            self.ui.keyFrameLabel.setText("No key frames added")
            self.playButton.setEnabled(False)
            self.beginningButton.setEnabled(False)
            self.endButton.setEnabled(False)
            self.saveImagesButton.setEnabled(False)
            self.saveParametersButton.setEnabled(False)
            self.ui.deleteAllButton.setEnabled(False)
            self.ui.frameIntervalSelector.setEnabled(False)
        elif nframes == 1:
            self.ui.keyFrameLabel.setText("One key frame added")
            self.playButton.setEnabled(False)
            self.beginningButton.setEnabled(True)
            self.endButton.setEnabled(True)
            self.saveImagesButton.setEnabled(False)
            self.saveParametersButton.setEnabled(True)
            self.ui.deleteAllButton.setEnabled(True)
            self.ui.frameIntervalSelector.setEnabled(True)
        else:
            self.ui.keyFrameLabel.setText("%d key frames added" % nframes)
            self.playButton.setEnabled(True)
            self.beginningButton.setEnabled(True)
            self.endButton.setEnabled(True)
            self.saveImagesButton.setEnabled(True)
            self.saveParametersButton.setEnabled(True)
            self.ui.deleteAllButton.setEnabled(True)
            self.ui.frameIntervalSelector.setEnabled(True)

if __name__ == '__main__':
    movie_gui = MovieGui()
    movie_gui.show()
