TARGET=mirserver

CONFIG += link_pkgconfig
PKGCONFIG += mirclient mirserver egl url-dispatcher-1

PLUGIN_TYPE = platforms
PLUGIN_CLASS_NAME = MirServerIntegrationPlugin
!equals(TARGET, $$QT_DEFAULT_QPA_PLUGIN): PLUGIN_EXTENDS = -
load(qt_plugin)

CONFIG += qpa/genericunixfontdatabase

SOURCES = plugin.cpp

HEADERS = plugin.h

include (mirserver.pri)

include (../../common/common.pri)
include (../../common/tracepoints.pri)

OTHER_FILES += \
            mirserver.json
