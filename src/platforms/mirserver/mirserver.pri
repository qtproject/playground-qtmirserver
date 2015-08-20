QT += core-private gui-private platformsupport-private dbus sensors

PKGCONFIG += url-dispatcher-1

SOURCES += \
    $$PWD/qteventfeeder.cpp \
    $$PWD/qmirserver.cpp \
    $$PWD/qmirserver_p.cpp \
    $$PWD/sessionauthorizer.cpp \
    $$PWD/sessionlistener.cpp \
    $$PWD/surfaceobserver.cpp \
    $$PWD/promptsessionlistener.cpp \
    $$PWD/mirserver.cpp \
    $$PWD/mirserverstatuslistener.cpp \
    $$PWD/display.cpp \
    $$PWD/screen.cpp \
    $$PWD/displaywindow.cpp \
    $$PWD/mirserverintegration.cpp \
    $$PWD/miropenglcontext.cpp \
    $$PWD/mirwindowmanager.cpp \
    $$PWD/nativeinterface.cpp \
    $$PWD/qtcompositor.cpp \
    $$PWD/services.cpp \
    $$PWD/ubuntutheme.cpp \
    $$PWD/clipboard.cpp

HEADERS += $$PWD/clipboard.h \
    $$PWD/display.h \
    $$PWD/displaywindow.h \
    $$PWD/logging.h \
    $$PWD/mirglconfig.h \
    $$PWD/miropenglcontext.h \
    $$PWD/mirwindowmanager.h \
    $$PWD/mirserver.h \
    $$PWD/mirserverintegration.h \
    $$PWD/mirserverstatuslistener.h \
    $$PWD/nativeinterface.h \
    $$PWD/promptsessionlistener.h \
    $$PWD/qmirserver.h \
    $$PWD/qmirserver_p.h \
    $$PWD/qtcompositor.h \
    $$PWD/qteventfeeder.h \
    $$PWD/screen.h \
    $$PWD/services.h \
    $$PWD/sessionauthorizer.h \
    $$PWD/sessionlistener.h \
    $$PWD/surfaceobserver.h \
    $$PWD/ubuntutheme.h

INCLUDEPATH += $$PWD
