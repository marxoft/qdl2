QT += core network
CONFIG += plugin
TARGET = qdl2-deathbycaptcha
TEMPLATE = lib

HEADERS += \
    deathbycaptchaplugin.h \
    json.h

SOURCES += \
    json.cpp \
    deathbycaptchaplugin.cpp

maemo5 {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/decaptchaplugin.h

    config.files = "$$TARGET".json
    config.path = /opt/qdl2/plugins/decaptcha

    icon.files = "$$TARGET".jpg
    icon.path = /opt/qdl2/plugins/icons

    target.path = /opt/qdl2/plugins/decaptcha
    
    INSTALLS += \
        target \
        config \
        icon

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/decaptchaplugin.h

    config.files = "$$TARGET".json
    config.path = /usr/share/qdl2/plugins/decaptcha

    icon.files = "$$TARGET".jpg
    icon.path = /usr/share/qdl2/plugins/icons

    target.path = /usr/share/qdl2/plugins/decaptcha
    
    INSTALLS += \
        target \
        config \
        icon
}
