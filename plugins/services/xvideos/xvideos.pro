TEMPLATE = subdirs

symbian {
    config.sources = xvideos.json
    config.path = !:/qdl2/plugins/services

    icon.sources = xvideos.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = xvideos.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    xvideos_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins XVideos
    DEPLOYMENT += \
        xvideos_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = xvideos.json
    config.path = /opt/qdl2/plugins/services

    icon.files = xvideos.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = xvideos.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
