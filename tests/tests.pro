TEMPLATE = subdirs

SUBDIRS += \
        mirserver \
        modules \
        testlibs

modules.depends = testlibs
mirserver.depends = testlibs
