TEMPLATE = lib

CONFIG += plugin
CONFIG += silent
CONFIG += hide_symbols

QT += gui gui-private
QT += network
QT += egl_support-private
QT += eventdispatcher_support-private
QT += fontdatabase_support-private

QT += devicediscovery_support_private
QT.egl_support_private.uses =

CONFIG += hide_symbols
CONFIG += no_private_qt_headers_warning
CONFIG += egl

MOC_DIR=moc
OBJECTS_DIR=obj

TARGET = $$qtLibraryTarget(qpagbm)
DESTDIR = plugins/platforms

LIBS += -lgbm

HEADERS += \
    QGbmPlatform.h \

SOURCES += \
    QGbmPlatform.cpp \
    QGbmIntegrationPlugin.cpp

OTHER_FILES += metadata.json
