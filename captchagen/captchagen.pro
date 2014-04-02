#-------------------------------------------------
#
# Project created by QtCreator 2014-02-18T13:56:21
#
#-------------------------------------------------

!include (../equares.pri):error("Couldn't find the equares.pri file!")

QT       += core gui

TARGET = captchagen
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

RESOURCES += \
    captchagen.qrc
