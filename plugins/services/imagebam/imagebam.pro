TEMPLATE = subdirs

symbian {
    config.sources = imagebam.json
    config.path = !:/qdl2/plugins/services

    icon.sources = imagebam.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = imagebam.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    imagebam_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins ImageBam
    DEPLOYMENT += \
        imagebam_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = imagebam.json
    config.path = /opt/qdl2/plugins/services

    icon.files = imagebam.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = imagebam.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
