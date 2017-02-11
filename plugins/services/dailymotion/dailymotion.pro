QT += core network
CONFIG += plugin
TARGET = qdl2-dailymotion
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += dailymotionplugin.h
SOURCES += dailymotionplugin.cpp

maemo5 {
    LIBS += -L/usr/lib -lqdailymotion
    CONFIG += link_prl
    PKGCONFIG += libqdailymotion

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
    LIBS += -L/usr/lib -lqdailymotion
    CONFIG += link_prl
    PKGCONFIG += libqdailymotion

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
