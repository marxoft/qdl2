QT += network
CONFIG += plugin
TARGET = qdl2-googlerecaptcha
TEMPLATE = lib

HEADERS += googlerecaptchaplugin.h
SOURCES += googlerecaptchaplugin.cpp

maemo5 {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/recaptchaplugin.h

    config.files = "$$TARGET".json
    config.path = /opt/qdl2/plugins/recaptcha

    icon.files = "$$TARGET".jpg
    icon.path = /opt/qdl2/plugins/icons

    target.path = /opt/qdl2/plugins/recaptcha

    INSTALLS += \
        config \
        icon \
        target

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/recaptchaplugin.h

    config.files = "$$TARGET".json
    config.path = /usr/share/qdl2/plugins/recaptcha

    icon.files = "$$TARGET".jpg
    icon.path = /usr/share/qdl2/plugins/icons

    target.path = /usr/share/qdl2/plugins/recaptcha

    INSTALLS += \
        config \
        icon \
        target
}
