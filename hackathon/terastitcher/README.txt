#  PREREQUISITES:
#
#   1. OpenCV >= 2.2.x (both shared libraries and headers)
#      download from http://opencv.willowgarage.com/wiki/
#   2. Qt >= 4.7.x (5.x should work too but it has not been tested) 
#
#  INSTRUCTIONS:
#
#   1. Open teramanager.pro with a text editor;
#   2. Check that V3DMAINPATH is set with the right "v3d_main" folder path;
#   3. Search for the line
#      "#set up OpenCV library (please modify include and lib paths if necessary)"
#      set INCLUDEPATH with the path of OpenCV headers
#      set LIBS with the path of OpenCV libraries;
#   4. Save and close teramanager.pro;
#   5. Before running the plugin, check that OpenCV shared libraries are
#      in a path where the system looks for libraries.
#
