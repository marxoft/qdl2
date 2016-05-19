TEMPLATE = subdirs

config.files = spankbang.json
icon.files = spankbang.jpg
plugin.files = spankbang.js

unix {
    config.path = /opt/qdl2/plugins/services
    icon.path = /opt/qdl2/plugins/icons
    plugin.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin
}
