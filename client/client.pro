TEMPLATE = app
TARGET = qdl2-client

QT += network

INCLUDEPATH += \
    src/base \
    src/plugins

HEADERS += \
    src/base/actionmodel.h \
    src/base/archivepasswordmodel.h \
    src/base/captchatype.h \
    src/base/categories.h \
    src/base/categorymodel.h \
    src/base/categoryselectionmodel.h \
    src/base/clipboardurlmodel.h \
    src/base/concurrenttransfersmodel.h \
    src/base/downloadrequestmodel.h \
    src/base/imagecache.h \
    src/base/json.h \
    src/base/logger.h \
    src/base/loggerverbositymodel.h \
    src/base/networkproxytypemodel.h \
    src/base/package.h \
    src/base/request.h \
    src/base/qdl.h \
    src/base/selectionmodel.h \
    src/base/serviceselectionmodel.h \
    src/base/stringmodel.h \
    src/base/transfer.h \
    src/base/transferitem.h \
    src/base/transferitemprioritymodel.h \
    src/base/transfermodel.h \
    src/base/urlcheckmodel.h \
    src/base/urlretrievalmodel.h \
    src/base/utils.h \
    src/plugins/decaptchapluginconfig.h \
    src/plugins/decaptchapluginconfigmodel.h \
    src/plugins/decaptchapluginmanager.h \
    src/plugins/pluginsettings.h \
    src/plugins/recaptchapluginconfig.h \
    src/plugins/recaptchapluginconfigmodel.h \
    src/plugins/recaptchapluginmanager.h \
    src/plugins/servicepluginconfig.h \
    src/plugins/servicepluginconfigmodel.h \
    src/plugins/servicepluginmanager.h

SOURCES += \
    src/base/archivepasswordmodel.cpp \
    src/base/categories.cpp \
    src/base/categorymodel.cpp \
    src/base/clipboardurlmodel.cpp \
    src/base/downloadrequestmodel.cpp \
    src/base/imagecache.cpp \
    src/base/json.cpp \
    src/base/logger.cpp \
    src/base/package.cpp \
    src/base/request.cpp \
    src/base/selectionmodel.cpp \
    src/base/stringmodel.cpp \
    src/base/transfer.cpp \
    src/base/transferitem.cpp \
    src/base/transfermodel.cpp \
    src/base/urlcheckmodel.cpp \
    src/base/urlretrievalmodel.cpp \
    src/base/utils.cpp \
    src/plugins/decaptchapluginconfig.cpp \
    src/plugins/decaptchapluginconfigmodel.cpp \
    src/plugins/decaptchapluginmanager.cpp \
    src/plugins/pluginsettings.cpp \
    src/plugins/recaptchapluginconfig.cpp \
    src/plugins/recaptchapluginconfigmodel.cpp \
    src/plugins/recaptchapluginmanager.cpp \
    src/plugins/servicepluginconfig.cpp \
    src/plugins/servicepluginconfigmodel.cpp \
    src/plugins/servicepluginmanager.cpp

maemo5 {
    QT += \
        maemo5 \
        webkit

    INCLUDEPATH += src/maemo5

    HEADERS += \
        src/maemo5/aboutdialog.h \
        src/maemo5/aboutpluginsdialog.h \
        src/maemo5/addurlsdialog.h \
        src/maemo5/archivepasswordsdialog.h \
        src/maemo5/captchadialog.h \
        src/maemo5/categoriesdialog.h \
        src/maemo5/clipboardurlsdialog.h \
        src/maemo5/connectiondialog.h \
        src/maemo5/decaptchapluginsdialog.h \
        src/maemo5/definitions.h \
        src/maemo5/downloadrequestdialog.h \
        src/maemo5/itemdelegate.h \
        src/maemo5/mainwindow.h \
        src/maemo5/multivalueselector.h \
        src/maemo5/networkproxydialog.h \
        src/maemo5/nocaptchadialog.h \
        src/maemo5/packagepropertiesdialog.h \
        src/maemo5/page.h \
        src/maemo5/plugindialog.h \
        src/maemo5/pluginsettingsdialog.h \
        src/maemo5/qmaemo5multilistpickselector.h \
        src/maemo5/recaptchapluginsdialog.h \
        src/maemo5/retrieveurlsdialog.h \
        src/maemo5/servicepluginsdialog.h \
        src/maemo5/settings.h \
        src/maemo5/settingsdialog.h \
        src/maemo5/texteditdialog.h \
        src/maemo5/textinputdialog.h \
        src/maemo5/transferpropertiesdialog.h \
        src/maemo5/urlcheckdialog.h \
        src/maemo5/valueselector.h \
        src/maemo5/valueselectoraction.h

    SOURCES += \
        src/maemo5/aboutdialog.cpp \
        src/maemo5/aboutpluginsdialog.cpp \
        src/maemo5/addurlsdialog.cpp \
        src/maemo5/archivepasswordsdialog.cpp \
        src/maemo5/captchadialog.cpp \
        src/maemo5/categoriesdialog.cpp \
        src/maemo5/clipboardurlsdialog.cpp \
        src/maemo5/connectiondialog.cpp \
        src/maemo5/decaptchapluginsdialog.cpp \
        src/maemo5/downloadrequestdialog.cpp \
        src/maemo5/itemdelegate.cpp \
        src/maemo5/main.cpp \
        src/maemo5/mainwindow.cpp \
        src/maemo5/multivalueselector.cpp \
        src/maemo5/networkproxydialog.cpp \
        src/maemo5/nocaptchadialog.cpp \
        src/maemo5/packagepropertiesdialog.cpp \
        src/maemo5/page.cpp \
        src/maemo5/plugindialog.cpp \
        src/maemo5/pluginsettingsdialog.cpp \
        src/maemo5/qmaemo5multilistpickselector.cpp \
        src/maemo5/recaptchapluginsdialog.cpp \
        src/maemo5/retrieveurlsdialog.cpp \
        src/maemo5/servicepluginsdialog.cpp \
        src/maemo5/settings.cpp \
        src/maemo5/settingsdialog.cpp \
        src/maemo5/texteditdialog.cpp \
        src/maemo5/textinputdialog.cpp \
        src/maemo5/transferpropertiesdialog.cpp \
        src/maemo5/urlcheckdialog.cpp \
        src/maemo5/valueselector.cpp \
        src/maemo5/valueselectoraction.cpp
        
    desktop.path = /usr/share/applications/hildon
    desktop.files = desktop/maemo5/qdl2-client.desktop

    icon.files = desktop/maemo5/64x64/qdl2-client.png
    icon.path = /opt/qdl2-client/icons

    icon64.files = desktop/desktop/64x64/qdl2-client.png
    icon64.path = /usr/share/icons/hicolor/64x64/apps
    
    target.path = /opt/qdl2-client/bin
    
    INSTALLS += \
        desktop \
        icon \
        icon64 \
        target

} else:unix {
    greaterThan(QT_MAJOR_VERSION, 4) {
        QT += \
            webkitwidgets \
            widgets
    }
    else {
        QT += webkit
    }

    INCLUDEPATH += src/desktop

    HEADERS += \
        src/desktop/aboutdialog.h \
        src/desktop/addurlsdialog.h \
        src/desktop/captchadialog.h \
        src/desktop/categorysettingspage.h \
        src/desktop/clientsettingspage.h \
        src/desktop/clipboardurlsdialog.h \
        src/desktop/definitions.h \
        src/desktop/decaptchasettingspage.h \
        src/desktop/downloadrequestdialog.h \
        src/desktop/generalsettingspage.h \
        src/desktop/networksettingspage.h \
        src/desktop/nocaptchadialog.h \
        src/desktop/mainwindow.h \
        src/desktop/packagepropertiesdialog.h \
        src/desktop/page.h \
        src/desktop/pluginsettingsdialog.h \
        src/desktop/pluginsettingspage.h \
        src/desktop/recaptchasettingspage.h \
        src/desktop/retrieveurlsdialog.h \
        src/desktop/serversettingspage.h \
        src/desktop/servicesettingspage.h \
        src/desktop/settings.h \
        src/desktop/settingsdialog.h \
        src/desktop/settingspage.h \
        src/desktop/texteditdialog.h \
        src/desktop/transferdelegate.h \
        src/desktop/transferpropertiesdialog.h \
        src/desktop/urlcheckdialog.h

    SOURCES += \
        src/desktop/aboutdialog.cpp \
        src/desktop/addurlsdialog.cpp \
        src/desktop/captchadialog.cpp \
        src/desktop/categorysettingspage.cpp \
        src/desktop/clientsettingspage.cpp \
        src/desktop/clipboardurlsdialog.cpp \
        src/desktop/decaptchasettingspage.cpp \
        src/desktop/downloadrequestdialog.cpp \
        src/desktop/generalsettingspage.cpp \
        src/desktop/networksettingspage.cpp \
        src/desktop/nocaptchadialog.cpp \
        src/desktop/main.cpp \
        src/desktop/mainwindow.cpp \
        src/desktop/packagepropertiesdialog.cpp \
        src/desktop/page.cpp \
        src/desktop/pluginsettingsdialog.cpp \
        src/desktop/pluginsettingspage.cpp \
        src/desktop/recaptchasettingspage.cpp \
        src/desktop/retrieveurlsdialog.cpp \
        src/desktop/serversettingspage.cpp \
        src/desktop/servicesettingspage.cpp \
        src/desktop/settings.cpp \
        src/desktop/settingsdialog.cpp \
        src/desktop/settingspage.cpp \
        src/desktop/texteditdialog.cpp \
        src/desktop/transferdelegate.cpp \
        src/desktop/transferpropertiesdialog.cpp \
        src/desktop/urlcheckdialog.cpp
        
    desktop.path = /usr/share/applications
    desktop.files = desktop/desktop/qdl2-client.desktop

    icon.files = desktop/desktop/64x64/qdl2-client.png
    icon.path = /usr/share/qdl2-client/icons

    icon64.files = desktop/desktop/64x64/qdl2-client.png
    icon64.path = /usr/share/icons/hicolor/64x64/apps

    icon48.files = desktop/desktop/48x48/qdl2-client.png
    icon48.path = /usr/share/icons/hicolor/48x48/apps

    icon22.files = desktop/desktop/22x22/qdl2-client.png
    icon22.path = /usr/share/icons/hicolor/22x22/apps

    icon16.files = desktop/desktop/16x16/qdl2-client.png
    icon16.path = /usr/share/icons/hicolor/16x16/apps
    
    target.path = /usr/bin
    
    INSTALLS += \
        desktop \
        icon \
        icon16 \
        icon22 \
        icon48 \
        icon64 \
        target
}
