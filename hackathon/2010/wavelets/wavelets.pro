# if #if stuff do not work !


# nicolas's version
#TEMPLATE      = lib
#CONFIG       += plugin warning_off 
#INCLUDEPATH  += /Users/nicolas/prog/v3d/my_v3d_plugins/basic_c_fun
#HEADERS     = wavelets.h scaleinfo.h ioV3dUtils.h waveletConfigException.h waveletTransform.h		 
#SOURCES     = wavelets.cpp scaleinfo.cpp ioV3dUtils.cpp waveletConfigException.cpp waveletTransform.cpp
#LIBS          += -lfftw3 			
			
#TARGET        = $$qtLibraryTarget(wavelets)
#DESTDIR       = .

# fab's version

TEMPLATE      = lib
CONFIG       += plugin warning_off 
INCLUDEPATH  += ../../v3d_main/basic_c_fun
HEADERS     = wavelets.h scaleinfo.h ioV3dUtils.h waveletConfigException.h waveletTransform.h v3d_utils.h 	
SOURCES     = wavelets.cpp scaleinfo.cpp ioV3dUtils.cpp waveletConfigException.cpp waveletTransform.cpp v3d_utils.cpp
TARGET        = $$qtLibraryTarget(wavelets)
DESTDIR       = ../../v3d_main/v3d/plugins/Wavelets

