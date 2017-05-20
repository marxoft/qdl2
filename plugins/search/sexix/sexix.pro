TEMPLATE = subdirs

maemo5 {
    config.files = qdl2-sexixsearch.json
    config.path = /opt/qdl2/plugins/search

    icon.files = qdl2-sexixsearch.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = qdl2-sexixsearch.js
    plugin.path = /opt/qdl2/plugins/search
    
    INSTALLS += \
        config \
        icon \
        plugin

} else:symbian {
    config.sources = qdl2-sexixsearch.json
    config.path = !:/qdl2/plugins/search

    icon.sources = qdl2-sexixsearch.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = qdl2-sexixsearch.js
    plugin.path = !:/qdl2/plugins/search

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    sexixsearch_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins Sexix Search
    DEPLOYMENT += \
        sexixsearch_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = qdl2-sexixsearch.json
    config.path = /usr/share/qdl2/plugins/search

    icon.files = qdl2-sexixsearch.jpg
    icon.path = /usr/share/qdl2/plugins/icons

    plugin.files = qdl2-sexixsearch.js
    plugin.path = /usr/share/qdl2/plugins/search
    
    INSTALLS += \
        config \
        icon \
        plugin
}
