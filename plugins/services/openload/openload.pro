TEMPLATE = subdirs

maemo5 {
    config.files = qdl2-openload.json
    config.path = /opt/qdl2/plugins
    
    icon.files = qdl2-openload.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = qdl2-openload.js
    plugin.path = /opt/qdl2/plugins

    INSTALLS += \
        config \
        plugin

} else:symbian {
    config.sources = qdl2-openload.json
    config.path = !:/qdl2/plugins
    
    icon.sources = qdl2-openload.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = qdl2-openload.js
    plugin.path = !:/qdl2/plugins

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    openload_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins Openload
    DEPLOYMENT += \
        openload_deployment \
        config \
        plugin

} else:unix {
    config.files = qdl2-openload.json
    config.path = /usr/share/qdl2/plugins
    
    icon.files = qdl2-openload.jpg
    icon.path = /usr/share/qdl2/plugins/icons

    plugin.files = qdl2-openload.js
    plugin.path = /usr/share/qdl2/plugins

    INSTALLS += \
        config \
        plugin
}
