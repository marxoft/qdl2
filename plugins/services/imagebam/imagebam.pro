TEMPLATE = subdirs

config.files = imagebam.json
icon.files = imagebam.jpg
plugin.files = imagebam.js

unix {
    config.path = /opt/qdl2/plugins/services
    icon.path = /opt/qdl2/plugins/icons
    plugin.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin
}
