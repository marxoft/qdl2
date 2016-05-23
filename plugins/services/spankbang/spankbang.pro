TEMPLATE = subdirs

symbian {
    config.sources = spankbang.json
    config.path = !:/qdl2/plugins/services

    icon.sources = spankbang.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = spankbang.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    spankbang_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins SpankBang
    DEPLOYMENT += \
        spankbang_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = spankbang.json
    config.path = /opt/qdl2/plugins/services

    icon.files = spankbang.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = spankbang.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
