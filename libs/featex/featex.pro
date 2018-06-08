#-------------------------------------------------
#
# Feature extractor
# VH, 2013-10-16
#-------------------------------------------------

QT       += core
QT       += script

TEMPLATE = vclib
#DESTDIR = C:/Development/libs/featex/lib
DESTDIR = C:/Development/build
TARGET = featex
DEFINES += BUILDING_FEATEX_DLL

DEPENDPATH += .
INCLUDEPATH += .\
  ./features \
  "C:/Development/libs/cl2/include" 
  
INCLUDEPATH += "C:/Development/ai/libs/cells"
		
LIBS +=	"C:/Development/libs/cl2/lib/cl2.lib" 
LIBS +=	"C:/Development/build/cells.lib"

SOURCES += \
	./src/featex_p.cpp\
	./src/haralick.cpp\
#	./src/features/feature.cpp\
#	./src/features/testing.cpp\
#	./src/features/eccentricity.cpp\
#	./src/features/circularity.cpp\
#	./src/features/convexity.cpp\
#	./src/features/form.cpp\
#	./src/features/radialdist.cpp\
	
HEADERS  += \
	./include/featex.h\
	./src/featex_p.h\
	./src/haralick.h\
#	./features/feature.h\
#	./features/testing.h\
#	./features/eccentricity.h\
#	./features/circularity.h\
#	./features/convexity.h\
#	./features/form.h\
#	./features/radialdist.h\


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

