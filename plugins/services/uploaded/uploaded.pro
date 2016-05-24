QT += core network
CONFIG += plugin
TARGET = uploaded
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += \
    json.h \
    uploadedplugin.h

SOURCES += \
    json.cpp \
    uploadedplugin.cpp

symbian {
    TARGET.UID3 = 0xE74BF1A4
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000

    INCLUDEPATH += ../../src
    HEADERS += ../../src/serviceplugin.h

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    uploaded_deployment.pkg_prerules += vendorinfo

    config.sources = uploaded.json
    config.path = !:/qdl2/plugins/services

    icon.sources = uploaded.jpg
    icon.path = !:/qdl2/plugins/icons

    lib.sources = uploaded.dll
    lib.path = !:/sys/bin

    stub.sources = uploaded.qtplugin
    stub.path = !:/qdl2/plugins/services

    DEPLOYMENT.display_name = QDL Plugins Uploaded.to
    DEPLOYMENT += \
        uploaded_deployment \
        config \
        icon \
        lib \
        stub

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/serviceplugin.h

    config.files = uploaded.json
    config.path = /opt/qdl2/plugins/services

    icon.files = uploaded.jpg
    icon.path = /opt/qdl2/plugins/icons

    target.path = /opt/qdl2/plugins/services

    INSTALLS += \
        target \
        config \
        icon
}
