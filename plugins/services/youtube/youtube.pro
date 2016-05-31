QT += core network
CONFIG += plugin
TARGET = qdl2-youtube
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += youtubeplugin.h
SOURCES += youtubeplugin.cpp

config.files = "$$TARGET".json
icon.files = "$$TARGET".jpg

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
