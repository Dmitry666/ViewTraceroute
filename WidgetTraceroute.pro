#-------------------------------------------------
#
# Project created by QtCreator 2014-12-02T23:33:20
#
#-------------------------------------------------

QT       += core gui network webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WidgetTraceroute
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    geocode_data_manager.cpp

HEADERS  += mainwindow.h \
    geocode_data_manager.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    google_maps.html

RESOURCES += \
    resources.qrc

LIBS += -L/usr/local/lib
LIBS += -lmaxminddb
