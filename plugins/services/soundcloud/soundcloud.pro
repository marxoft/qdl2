QT += core network
CONFIG += plugin
TARGET = qdl2-soundcloud
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += soundcloudplugin.h
SOURCES += soundcloudplugin.cpp

maemo5 {
    LIBS += -L/opt/lib -lqsoundcloud
    CONFIG += link_prl
    PKGCONFIG += libqsoundcloud

    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/serviceplugin.h
    
    config.files = "$$TARGET".json
    config.path = /opt/qdl2/plugins/services
    
    icon.files = "$$TARGET".jpg
    icon.path = /opt/qdl2/plugins/icons
    
    target.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        target \
        config \
        icon

} else:unix {
    LIBS += -L/usr/lib -lqsoundcloud
    CONFIG += link_prl
    PKGCONFIG += libqsoundcloud

    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/serviceplugin.h
    
    config.files = "$$TARGET".json
    config.path = /usr/share/qdl2/plugins/services
    
    icon.files = "$$TARGET".jpg
    icon.path = /usr/share/qdl2/plugins/icons
    
    target.path = /usr/share/qdl2/plugins/services
    
    INSTALLS += \
        target \
        config \
        icon
}
