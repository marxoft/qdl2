TEMPLATE = subdirs

config.files = pornhub.json
icon.files = pornhub.jpg
plugin.files = pornhub.js

unix {
    config.path = /opt/qdl2/plugins/services
    icon.path = /opt/qdl2/plugins/icons
    plugin.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin
}
