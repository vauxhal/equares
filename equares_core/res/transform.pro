QT -= gui
TARGET = transform
TEMPLATE = lib
CONFIG += dll
QMAKE_CXXFLAGS += -O2
SOURCES += transform.cpp

#contains(QMAKESPEC, ^.*msvc.*$) {
windows {
  WINDESTTARGET = $$OUT_PWD/release/transform.dll
  WINDESTTARGET = $$replace(WINDESTTARGET, /, \\)
  WINDESTFINAL = $$OUT_PWD/
  WINDESTFINAL = $$replace(WINDESTFINAL, /, \\)
  QMAKE_POST_LINK += xcopy /Y $$WINDESTTARGET $$WINDESTFINAL
} else {
  DESTDIR = $$OUT_PWD
}
