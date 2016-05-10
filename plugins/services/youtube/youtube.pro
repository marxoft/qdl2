QT += core network
QT -= gui
CONFIG += plugin
TARGET = youtube
TEMPLATE = lib

HEADERS += youtubeplugin.h
SOURCES += youtubeplugin.cpp

config.files = youtube.json
icon.files = youtube.jpg

unix {
    LIBS += -L/usr/lib -lqyoutube
    CONFIG += link_prl
    PKGCONFIG += libqyoutube

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
