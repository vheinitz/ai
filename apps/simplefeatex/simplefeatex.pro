
CONFIG -=ui
CONFIG +=core
TEMPLATE = vcapp
TARGET = simplefeatex
DESTDIR = ../../../build


DEPENDPATH += .
INCLUDEPATH += .

INCLUDEPATH += .\
  C:/Development/ai/libs/featex/include \
		
LIBS +=	C:/Development/build/featex.lib

INCLUDEPATH += "C:/Development/cl2.git"
LIBS +=	"C:/Development/build/cl2.lib"

# Input
SOURCES += main.cpp
