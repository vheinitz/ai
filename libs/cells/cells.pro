#V. Heinitz, 2016-10-29
#Project file for Cells - the cell-analysis tool library

QT       += core

DEFINES += BUILDING_CELLS_DLL
 
INCLUDEPATH += .


#output directory
DESTDIR = C:/Development/build
TARGET = cells
TEMPLATE = lib 


SOURCES += \
    cells.cpp \

HEADERS  += \
    common.h \
    cells.h \



DEPENDPATH += .
INCLUDEPATH += .\
  ./features \
  "C:/Development/libs/cl2/include" \
		
LIBS +=	"C:/Development/libs/cl2/lib/cl2.lib"


INCLUDEPATH += . \
		"C:/Development/libs/opencv/include/opencv" \
		"C:/Development/libs/opencv/include/" \

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
