TEMPLATE = subdirs

symbian {
    config.sources = pornhub.json
    config.path = !:/qdl2/plugins/services

    icon.sources = pornhub.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = pornhub.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    pornhub_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins Pornhub
    DEPLOYMENT += \
        pornhub_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = pornhub.json
    config.path = /opt/qdl2/plugins/services

    icon.files = pornhub.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = pornhub.js
    plugin.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin
}
