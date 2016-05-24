QT += core network
CONFIG += plugin
TARGET = filespace
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += filespaceplugin.h
SOURCES += filespaceplugin.cpp

symbian {
    TARGET.UID3 = 0xE73FA3BC
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000

    INCLUDEPATH += ../../src
    HEADERS += ../../src/serviceplugin.h

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    filespace_deployment.pkg_prerules += vendorinfo

    config.sources = filespace.json
    config.path = !:/qdl2/plugins/services

    icon.sources = filespace.jpg
    icon.path = !:/qdl2/plugins/icons

    lib.sources = filespace.dll
    lib.path = !:/sys/bin

    stub.sources = filespace.qtplugin
    stub.path = !:/qdl2/plugins/services

    DEPLOYMENT.display_name = QDL Plugins Filespace
    DEPLOYMENT += \
        filespace_deployment \
        config \
        icon \
        lib \
        stub

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/serviceplugin.h

    config.files = filespace.json
    config.path = /opt/qdl2/plugins/services

    icon.files = filespace.jpg
    icon.path = /opt/qdl2/plugins/icons

    target.path = /opt/qdl2/plugins/services

    INSTALLS += \
        target \
        config \
        icon
}
