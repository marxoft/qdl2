TEMPLATE = subdirs

symbian {
    config.sources = qdl2-imageupper.json
    config.path = !:/qdl2/plugins/services

    icon.sources = qdl2-imageupper.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = qdl2-imageupper.js
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
    config.files = qdl2-imageupper.json
    config.path = /opt/qdl2/plugins/services

    icon.files = qdl2-imageupper.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = qdl2-imageupper.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
