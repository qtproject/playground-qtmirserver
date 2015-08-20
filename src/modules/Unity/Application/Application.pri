CONFIG += link_pkgconfig
PKGCONFIG += mirclient mirserver gsettings-qt glib-2.0 ubuntu-app-launch-2 gio-unix-2.0
### egl url-dispatcher-1

QT += qml-private quick-private  gui-private core-private dbus

CONFIG += no_keywords

SOURCES = $$PWD/application_manager.cpp \
    $$PWD/application.cpp \
    $$PWD/desktopfilereader.cpp \
    $$PWD/plugin.cpp \
    $$PWD/applicationscreenshotprovider.cpp \
    $$PWD/dbuswindowstack.cpp \
    $$PWD/taskcontroller.cpp \
    $$PWD/mirsurfacemanager.cpp \
    $$PWD/ubuntukeyboardinfo.cpp \
    $$PWD/mirsurfaceitem.cpp \
    $$PWD/mirbuffersgtexture.cpp \
    $$PWD/proc_info.cpp \
    $$PWD/session.cpp \
    $$PWD/sessionmanager.cpp \
    $$PWD/sharedwakelock.cpp \
    $$PWD/upstart/applicationcontroller.cpp \
    $$PWD/settings.cpp \

HEADERS = $$PWD/application.h \
     $$PWD/application_manager.h \
     $$PWD/applicationcontroller.h \
     $$PWD/applicationscreenshotprovider.h \
     $$PWD/dbuswindowstack.h \
     $$PWD/desktopfilereader.h \
     $$PWD/gscopedpointer.h \
     $$PWD/mirbuffersgtexture.h \
     $$PWD/mirsurfaceitem.h \
     $$PWD/mirsurfaceiteminterface.h \
     $$PWD/mirsurfaceitemmodel.h \
     $$PWD/mirsurfacemanager.h \
     $$PWD/objectlistmodel.h \
     $$PWD/proc_info.h \
     $$PWD/session.h \
     $$PWD/session_interface.h \
     $$PWD/sessionmanager.h \
     $$PWD/sessionmodel.h \
     $$PWD/settings.h \
     $$PWD/settings_interface.h \
     $$PWD/sharedwakelock.h \
     $$PWD/taskcontroller.h \
     $$PWD/ubuntukeyboardinfo.h

include($$PWD/../../../platforms/mirserver/mirserver.pri)
include($$PWD/../../../common/common.pri)

# unity-shell-application does not have a shared library, so we have to run moc
# on their header files

PKG_CONFIG = $$pkgConfigExecutable()
UNITY_INCLUDEDIR = $$system($$PKG_CONFIG --variable=includedir unity-shell-application)

HEADERS += \
    $$UNITY_INCLUDEDIR/unity/shell/application/ApplicationInfoInterface.h \
    $$UNITY_INCLUDEDIR/unity/shell/application/ApplicationManagerInterface.h
