TEMPLATE = subdirs

symbian {
    config.sources = imageupper.json
    config.path = !:/qdl2/plugins/services

    icon.sources = imageupper.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = imageupper.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    imageupper_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins Image Upper
    DEPLOYMENT += \
        imageupper_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = imageupper.json
    config.path = /opt/qdl2/plugins/services

    icon.files = imageupper.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = imageupper.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
