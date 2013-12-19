DESTDIR = $$OUT_PWD
QT -= gui
TARGET = ode
TEMPLATE = lib
CONFIG += dll
QMAKE_CXXFLAGS += -O2
SOURCES += ode.cpp

