
CONFIG -=ui
CONFIG +=core
TEMPLATE = vcapp
TARGET = objex
DESTDIR = ../../../build


DEPENDPATH += .
INCLUDEPATH += .

INCLUDEPATH += .\
  C:/Development/ai/libs/cells \
  
#TODO pipl in cells, hide opencv
INCLUDEPATH += . \
		"C:/Development/libs/opencv/include/opencv" \
		"C:/Development/libs/opencv/include/" \
		
LIBS +=	C:/Development/build/cells.lib

INCLUDEPATH += "C:/Development/cl2.git"
LIBS +=	"C:/Development/build/cl2.lib"

# Input
SOURCES += main.cpp

win32 {
	CONFIG(debug, debug|release) {
		LIBS += \
		C:/Development/libs/opencv/lib/opencv_contrib240d.lib \
		C:/Development/libs/opencv/lib/opencv_core240d.lib \
		C:/Development/libs/opencv/lib/opencv_features2d240d.lib \
		C:/Development/libs/opencv/lib/opencv_highgui240d.lib \
		C:/Development/libs/opencv/lib/opencv_imgproc240d.lib \
		C:/Development/libs/opencv/lib/opencv_ml240d.lib \
		C:/Development/libs/opencv/lib/opencv_objdetect240d.lib 
	} else {
		LIBS += \
		C:/Development/libs/opencv/lib/opencv_contrib240.lib \
		C:/Development/libs/opencv/lib/opencv_core240.lib \
		C:/Development/libs/opencv/lib/opencv_features2d240.lib \
		C:/Development/libs/opencv/lib/opencv_highgui240.lib \
		C:/Development/libs/opencv/lib/opencv_imgproc240.lib \
		C:/Development/libs/opencv/lib/opencv_ml240.lib \
		C:/Development/libs/opencv/lib/opencv_objdetect240.lib 
	}
}