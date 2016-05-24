QT += core network
CONFIG += plugin
TARGET = rapidgator
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += \
    json.h \
    rapidgatorplugin.h

SOURCES += \
    json.cpp \
    rapidgatorplugin.cpp

symbian {
    TARGET.UID3 = 0xE73CA31F
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000

    INCLUDEPATH += ../../src
    HEADERS += ../../src/serviceplugin.h

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    rapidgator_deployment.pkg_prerules += vendorinfo

    config.sources = rapidgator.json
    config.path = !:/qdl2/plugins/services

    icon.sources = rapidgator.jpg
    icon.path = !:/qdl2/plugins/icons

    lib.sources = rapidgator.dll
    lib.path = !:/sys/bin

    stub.sources = rapidgator.qtplugin
    stub.path = !:/qdl2/plugins/services

    DEPLOYMENT.display_name = QDL Plugins RapidGator
    DEPLOYMENT += \
        rapidgator_deployment \
        config \
        icon \
        lib \
        stub

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/serviceplugin.h

    config.files = rapidgator.json
    config.path = /opt/qdl2/plugins/services

    icon.files = rapidgator.jpg
    icon.path = /opt/qdl2/plugins/icons

    target.path = /opt/qdl2/plugins/services

    INSTALLS += \
        target \
        config \
        icon
}
