TEMPLATE = subdirs

config.files = xvideos.json
icon.files = xvideos.jpg
plugin.files = xvideos.js

unix {
    config.path = /opt/qdl2/plugins/services
    icon.path = /opt/qdl2/plugins/icons
    plugin.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin
}
