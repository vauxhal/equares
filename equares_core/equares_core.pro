#-------------------------------------------------
#
# Project created by QtCreator 2013-11-17T18:20:38
#
#-------------------------------------------------

!include (../equares.pri):error("Couldn't find the equares.pri file!")

#QT       -= gui
QT       += script

TARGET = equares_core
TEMPLATE = lib
DEFINES += EQUARES_CORE_LIBRARY

contains(DEFINES, EQUARES_CORE_STATIC) {
    CONFIG += staticlib
}

SOURCES += equares_core.cpp \
    ConstantSourceBox.cpp \
    PendulumBox.cpp \
    Rk4Box.cpp \
    DumpBox.cpp \
    Rk4AdjustParamBox.cpp \
    ProjectionBox.cpp \
    CrossSectionBox.cpp \
    ValveBox.cpp \
    CountedFilterBox.cpp \
    CanvasBox.cpp \
    DoublePendulumBox.cpp \
    equares_script.cpp \
    BitmapBox.cpp \
    IntervalFilterBox.cpp \
    MathieuBox.cpp \
    VibratingPendulumBox.cpp \
    OdeJsBox.cpp \
    initBoxFactory.cpp \
    OdeCxxBox.cpp

HEADERS += equares_core.h\
        equares_core_global.h \
    ConstantSourceBox.h \
    PendulumBox.h \
    Rk4Box.h \
    DumpBox.h \
    Rk4AdjustParamBox.h \
    ProjectionBox.h \
    CrossSectionBox.h \
    ValveBox.h \
    CountedFilterBox.h \
    CanvasBox.h \
    DoublePendulumBox.h \
    equares_script.h \
    BitmapBox.h \
    IntervalFilterBox.h \
    MathieuBox.h \
    OdeBox.h \
    VibratingPendulumBox.h \
    OdeJsBox.h \
    script_arrays.h \
    initBoxFactory.h \
    OdeCxxBox.h \
    EquaresException.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE061BE2F
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = equares_core.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

RESOURCES += \
    equares_core.qrc

contains(QMAKESPEC, ^.*msvc.*$) {
    MAKE_COMMAND = nmake
} else {
    MAKE_COMMAND = make
}

equaresbuildspecs.output = $$DESTDIR/buildpath.txt $$DESTDIR/makecmd.txt
equaresbuildspecs.commands = \
    echo $$(PATH) >$$DESTDIR/buildpath.txt $$escape_expand(\\n\\t) \
    echo $$MAKE_COMMAND >$$DESTDIR/makecmd.txt

QMAKE_EXTRA_TARGETS += equaresbuildspecs
POST_TARGETDEPS += equaresbuildspecs
