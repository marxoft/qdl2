TEMPLATE = subdirs

maemo5 {
    config.files = qdl2-imagefapsearch.json
    config.path = /opt/qdl2/plugins/search
    
    icon.files = qdl2-imagefapsearch.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = qdl2-imagefapsearch.js
    plugin.path = /opt/qdl2/plugins/search

    INSTALLS += \
        config \
        icon \
        plugin

} else:unix {
    config.files = qdl2-imagefapsearch.json
    config.path = /usr/share/qdl2/plugins/search
    
    icon.files = qdl2-imagefapsearch.jpg
    icon.path = /usr/share/qdl2/plugins/icons

    plugin.files = qdl2-imagefapsearch.js
    plugin.path = /usr/share/qdl2/plugins/search

    INSTALLS += \
        config \
        icon \
        plugin
}
