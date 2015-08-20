TARGET  = unityapplicationplugin
CXX_MODULE = $$TARGET ##????
TARGETPATH = Unity/Application
IMPORT_VERSION = 1.4

include(Application.pri)
include(../../../common/tracepoints.pri)

load(qml_plugin)
