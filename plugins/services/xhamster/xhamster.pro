TEMPLATE = subdirs

symbian {
    config.sources = xhamster.json
    config.path = !:/qdl2/plugins/services

    icon.sources = xhamster.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = xhamster.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    xhamster_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins XHamster
    DEPLOYMENT += \
        xhamster_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = xhamster.json
    config.path = /opt/qdl2/plugins/services

    icon.files = xhamster.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = xhamster.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
