#-------------------------------------------------
#
# Project created by QtCreator 2014-02-28T13:52:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ../../bin/PCBConverter
TEMPLATE = app
win32 {
QMAKE_LFLAGS *= -static -static-libgcc
}

SOURCES += main.cpp\
    libgeometry/gline.cpp \
    libgeometry/gcircle.cpp \
    libgeometry/grect.cpp \
    libgeometry/gpoint.cpp \
    libgeometry/gshape.cpp \
    libgeometry/gintersects.cpp \
    gerberreader.cpp \
    gerberview.cpp \
    pcbconverter.cpp \
    gcodeexport.cpp \
    drillreader.cpp \
    readersstruct.cpp \
    pcbpathcontur.cpp \
    printingpathcontur.cpp \
    pathcontur.cpp \
    zoomablegerberview.cpp \
    settings.cpp

HEADERS  += \
    libgeometry/libgeometry.h \
    libgeometry/gline.h \
    libgeometry/gpoint.h \
    libgeometry/gshape.h \
    libgeometry/gcircle.h \
    libgeometry/grect.h \
    libgeometry/gintersects.h \
    gerberreader.h \
    gerberview.h \
    pathcontur.h \
    pcbconverter.h \
    gcodeexport.h \
    drillreader.h \
    readersstruct.h \
    pcbpathcontur.h \
    printingpathcontur.h \
    zoomablegerberview.h \
    settings.h

FORMS += \
    pcbconverter.ui
