TEMPLATE = lib

CONFIG += plugin
CONFIG += silent
CONFIG += warn_on
CONFIG += hide_symbols
CONFIG += no_private_qt_headers_warning

CONFIG += strict_c++
CONFIG += c++11
CONFIG += pedantic

QT += gui gui-private
QT += devicediscovery_support_private

pedantic {
    linux-g++ | linux-g++-64 {

        QMAKE_CXXFLAGS *= -pedantic-errors
        QMAKE_CXXFLAGS *= -Wpedantic

        QMAKE_CXXFLAGS *= -Wsuggest-override
        QMAKE_CXXFLAGS *= -Wsuggest-final-types
        QMAKE_CXXFLAGS *= -Wsuggest-final-methods

        #QMAKE_CXXFLAGS *= -fanalyzer

           QMAKE_CXXFLAGS += \
                -isystem $$[QT_INSTALL_HEADERS]/QtCore \
                -isystem $$[QT_INSTALL_HEADERS]/QtCore/$$[QT_VERSION]/QtCore \
                -isystem $$[QT_INSTALL_HEADERS]/QtGui \
                -isystem $$[QT_INSTALL_HEADERS]/QtGui/$$[QT_VERSION]/QtGui \
    }
}

greaterThan(QT_MAJOR_VERSION, 5) {

    QT += eglfsdeviceintegration_private

} else {

    QT += egl_support-private
    QT.egl_support_private.uses =

    QT += eventdispatcher_support-private
    QT += fontdatabase_support-private

    CONFIG += egl
}

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

INSTALL_ROOT=/usr/local/qpagbm
# INSTALL_ROOT=$$[QT_INSTALL_PREFIX]

target.path = $${INSTALL_ROOT}/plugins/platforms
INSTALLS += target

