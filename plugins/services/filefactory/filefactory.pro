QT += core network
CONFIG += plugin
TARGET = filefactory
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += \
    filefactoryplugin.h \
    json.h

SOURCES += \
    filefactoryplugin.cpp \
    json.cpp

symbian {
    TARGET.UID3 = 0xE75BC41C
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000

    INCLUDEPATH += ../../src
    HEADERS += ../../src/serviceplugin.h

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    filefactory_deployment.pkg_prerules += vendorinfo

    config.sources = filefactory.json
    config.path = !:/qdl2/plugins/services

    icon.sources = filefactory.jpg
    icon.path = !:/qdl2/plugins/icons

    lib.sources = filefactory.dll
    lib.path = !:/sys/bin

    stub.sources = filefactory.qtplugin
    stub.path = !:/qdl2/plugins/services

    DEPLOYMENT.display_name = QDL Plugins Filefactory
    DEPLOYMENT += \
        filefactory_deployment \
        config \
        icon \
        lib \
        stub

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/serviceplugin.h

    config.files = filefactory.json
    config.path = /opt/qdl2/plugins/services

    icon.files = filefactory.jpg
    icon.path = /opt/qdl2/plugins/icons

    target.path = /opt/qdl2/plugins/services

    INSTALLS += \
        target \
        config \
        icon
}
