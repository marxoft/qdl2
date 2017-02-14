QT += core network
CONFIG += plugin
TARGET = qdl2-pornhubsearch
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += pornhubsearchplugin.h
SOURCES += pornhubsearchplugin.cpp

maemo5 {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/searchplugin.h
    
    config.files = "$$TARGET".json
    config.path = /opt/qdl2/plugins/search
    
    icon.files = "$$TARGET".jpg
    icon.path = /opt/qdl2/plugins/icons
    
    target.path = /opt/qdl2/plugins/search
    
    INSTALLS += \
        target \
        config \
        icon

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/searchplugin.h
    
    config.files = "$$TARGET".json
    config.path = /usr/share/qdl2/plugins/search
    
    icon.files = "$$TARGET".jpg
    icon.path = /usr/share/qdl2/plugins/icons
    
    target.path = /usr/share/qdl2/plugins/search
    
    INSTALLS += \
        target \
        config \
        icon
}
