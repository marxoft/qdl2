QT += core network
CONFIG += plugin
TARGET = fileboom
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += fileboomplugin.h
SOURCES += fileboomplugin.cpp

symbian {
    TARGET.UID3 = 0xE74FE21C
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000

    INCLUDEPATH += ../../src
    HEADERS += ../../src/serviceplugin.h

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    fileboom_deployment.pkg_prerules += vendorinfo

    config.sources = fileboom.json
    config.path = !:/qdl2/plugins/services

    icon.sources = fileboom.jpg
    icon.path = !:/qdl2/plugins/icons

    lib.sources = fileboom.dll
    lib.path = !:/sys/bin

    stub.sources = fileboom.qtplugin
    stub.path = !:/qdl2/plugins/services

    DEPLOYMENT.display_name = QDL Plugins FileBoom
    DEPLOYMENT += \
        fileboom_deployment \
        config \
        icon \
        lib \
        stub

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/serviceplugin.h

    config.files = fileboom.json
    config.path = /opt/qdl2/plugins/services

    icon.files = fileboom.jpg
    icon.path = /opt/qdl2/plugins/icons

    target.path = /opt/qdl2/plugins/services

    INSTALLS += \
        target \
        config \
        icon
}
