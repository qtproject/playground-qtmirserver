CONFIG += link_pkgconfig
PKGCONFIG += mirclient mirserver

INCLUDEPATH += $$PWD/../../../src/platforms/mirserver/
INCLUDEPATH += $$PWD/../../../src/common
INCLUDEPATH += $$PWD/../../../src/modules/
INCLUDEPATH += $$PWD/../../../src/modules/Unity/Application

LIBS += $$PWD/libmirtestlib.a
