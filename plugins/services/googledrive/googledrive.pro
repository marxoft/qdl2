QT += core network
QT -= gui
CONFIG += plugin
TARGET = googledrive
TEMPLATE = lib

HEADERS += googledriveplugin.h
SOURCES += googledriveplugin.cpp

config.files = googledrive.json
icon.files = googledrive.jpg

unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/serviceplugin.h
    
    config.path = /opt/qdl2/plugins/services
    icon.path = /opt/qdl2/plugins/icons
    target.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        target \
        config \
        icon
}
