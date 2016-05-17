TEMPLATE = subdirs

config.files = uploadly.json
icon.files = uploadly.jpg
plugin.files = uploadly.js

unix {
    config.path = /opt/qdl2/plugins/services
    icon.path = /opt/qdl2/plugins/icons
    plugin.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin
}
