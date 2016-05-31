/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "settings.h"
#include "definitions.h"
#include <QSettings>

static QVariant value(const QString &property, const QVariant &defaultValue = QVariant()) {
    return QSettings(APP_CONFIG_PATH + "settings", QSettings::IniFormat).value(property, defaultValue);
}

static void setValue(const QString &property, const QVariant &value) {
    QSettings(APP_CONFIG_PATH + "settings", QSettings::IniFormat).setValue(property, value);
}

Settings* Settings::self = 0;

Settings::Settings() :
    QObject()
{
}

Settings::~Settings() {
    self = 0;
}

Settings* Settings::instance() {
    return self ? self : self = new Settings;
}

bool Settings::clipboardMonitorEnabled() {
    return value("clipboardMonitorEnabled", false).toBool();
}

void Settings::setClipboardMonitorEnabled(bool enabled) {
    if (enabled != clipboardMonitorEnabled()) {
        setValue("clipboardMonitorEnabled", enabled);

        if (self) {
            emit self->clipboardMonitorEnabledChanged(enabled);
        }
    }
}

QString Settings::customCommand() {
    return value("customCommand").toString();
}

void Settings::setCustomCommand(const QString &command) {
    if (command != customCommand()) {
        setValue("customCommand", command);

        if (self) {
            emit self->customCommandChanged(command);
        }
    }
}

bool Settings::customCommandEnabled() {
    return value("customCommandEnabled", false).toBool();
}

void Settings::setCustomCommandEnabled(bool enabled) {
    if (enabled != customCommandEnabled()) {
        setValue("customCommandEnabled", enabled);
        
        if (self) {
            emit self->customCommandEnabledChanged(enabled);
        }
    }
}

QString Settings::decaptchaPlugin() {
    return value("Plugins/decaptchaPlugin").toString();
}

void Settings::setDecaptchaPlugin(const QString &pluginId) {
    if (pluginId != decaptchaPlugin()) {
        setValue("Plugins/decaptchaPlugin", pluginId);

        if (self) {
            emit self->decaptchaPluginChanged(pluginId);
        }
    }
}

QString Settings::defaultCategory() {
    return value("defaultCategory").toString();
}

void Settings::setDefaultCategory(const QString &category) {
    if (category != defaultCategory()) {
        setValue("defaultCategory", category);

        if (self) {
            emit self->defaultCategoryChanged(category);
        }
    }
}

QString Settings::defaultServicePlugin() {
    return value("Plugins/defaultServicePlugin").toString();
}

void Settings::setDefaultServicePlugin(const QString &pluginId) {
    if (pluginId != defaultServicePlugin()) {
        setValue("Plugins/defaultServicePlugin", pluginId);

        if (self) {
            emit self->defaultServicePluginChanged(pluginId);
        }
    }
}

bool Settings::usePlugins() {
    return value("Plugins/usePlugins", true).toBool();
}

void Settings::setUsePlugins(bool enabled) {
    if (enabled != usePlugins()) {
        setValue("Plugins/usePlugins", enabled);
        
        if (self) {
            emit self->usePluginsChanged(enabled);
        }
    }
}

QString Settings::downloadPath() {
    QString path = value("downloadPath", DOWNLOAD_PATH).toString();

    if (!path.endsWith("/")) {
        path.append("/");
    }

    return path;
}

void Settings::setDownloadPath(const QString &path) {
    if (path != downloadPath()) {
        setValue("downloadPath", path);

        if (self) {
            emit self->downloadPathChanged(path.endsWith("/") ? path : path + "/");
        }
    }
}

bool Settings::createSubfolders() {
    return value("Archives/createSubfolders", false).toBool();
}

void Settings::setCreateSubfolders(bool enabled) {
    if (enabled != createSubfolders()) {
        setValue("Archives/createSubfolders", enabled);

        if (self) {
            emit self->createSubfoldersChanged(enabled);
        }
    }
}

bool Settings::extractArchives() {
    return value("Archives/extractArchives", false).toBool();
}

void Settings::setExtractArchives(bool enabled) {
    if (enabled != extractArchives()) {
        setValue("Archives/extractArchives", enabled);

        if (self) {
            emit self->extractArchivesChanged(enabled);
        }
    }
}

bool Settings::deleteExtractedArchives() {
    return value("Archives/deleteExtractedArchives", false).toBool();
}

void Settings::setDeleteExtractedArchives(bool enabled) {
    if (enabled != deleteExtractedArchives()) {
        setValue("Archives/deleteExtractedArchives", enabled);

        if (self) {
            emit self->deleteExtractedArchivesChanged(enabled);
        }
    }
}

QStringList Settings::archivePasswords() {
    return value("Archives/passwords").toStringList();
}

void Settings::setArchivePasswords(const QStringList &passwords) {
    setValue("Archives/passwords", passwords);

    if (self) {
        emit self->archivePasswordsChanged(passwords);
    }
}

int Settings::maximumConcurrentTransfers() {
    return qBound(1, value("maximumConcurrentTransfers", 1).toInt(), 5);
}

void Settings::setMaximumConcurrentTransfers(int maximum) {
    if (maximum != maximumConcurrentTransfers()) {
        maximum = qBound(1, maximum, 5);
        setValue("maximumConcurrentTransfers", maximum);

        if (self) {
            emit self->maximumConcurrentTransfersChanged(maximum);
        }
    }
}

bool Settings::startTransfersAutomatically() {
    return value("startTransfersAutomatically", true).toBool();
}

void Settings::setStartTransfersAutomatically(bool enabled) {
    if (enabled != startTransfersAutomatically()) {
        setValue("startTransfersAutomatically", enabled);

        if (self) {
            emit self->startTransfersAutomaticallyChanged(enabled);
        }
    }
}

int Settings::nextAction() {
    return value("nextAction", 0).toInt();
}

void Settings::setNextAction(int action) {
    if (action != nextAction()) {
        setValue("nextAction", action);

        if (self) {
            emit self->nextActionChanged(action);
        }
    }
}

bool Settings::networkProxyEnabled() {
    return value("NetworkProxy/networkProxyEnabled", false).toBool();
}

void Settings::setNetworkProxyEnabled(bool enabled) {
    if (enabled != networkProxyEnabled()) {
        setValue("NetworkProxy/networkProxyEnabled", enabled);

        if (self) {
            emit self->networkProxyEnabledChanged(enabled);
        }
    }
}

int Settings::networkProxyType() {
    return value("NetworkProxy/networkProxyType", 0).toInt();
}

void Settings::setNetworkProxyType(int type) {
    if (type != networkProxyType()) {
        setValue("NetworkProxy/networkProxyType", type);

        if (self) {
            emit self->networkProxyTypeChanged(type);
        }
    }
}

QString Settings::networkProxyHost() {
    return value("NetworkProxy/networkProxyHost").toString();
}

void Settings::setNetworkProxyHost(const QString &host) {
    if (host != networkProxyHost()) {
        setValue("NetworkProxy/networkProxyHost", host);

        if (self) {
            emit self->networkProxyHostChanged(host);
        }
    }
}

int Settings::networkProxyPort() {
    return value("NetworkProxy/networkProxyPort", 80).toInt();
}

void Settings::setNetworkProxyPort(int port) {
    if (port != networkProxyPort()) {
        setValue("NetworkProxy/networkProxyPort", port);

        if (self) {
            emit self->networkProxyPortChanged(port);
        }
    }
}

bool Settings::networkProxyAuthenticationEnabled() {
    return value("NetworkProxy/networkProxyAuthenticationEnabled", false).toBool();
}

void Settings::setNetworkProxyAuthenticationEnabled(bool enabled) {
    if (enabled != networkProxyAuthenticationEnabled()) {
        setValue("NetworkProxy/networkProxyAuthenticationEnabled", enabled);

        if (self) {
            emit self->networkProxyAuthenticationEnabledChanged(enabled);
        }
    }
}

QString Settings::networkProxyUsername() {
    return value("NetworkProxy/networkProxyUsername").toString();
}

void Settings::setNetworkProxyUsername(const QString &username) {
    if (username != networkProxyUsername()) {
        setValue("NetworkProxy/networkProxyUsername", username);

        if (self) {
            emit self->networkProxyUsernameChanged(username);
        }
    }
}

QString Settings::networkProxyPassword() {
    return value("NetworkProxy/networkProxyPassword").toString();
}

void Settings::setNetworkProxyPassword(const QString &password) {
    if (password != networkProxyPassword()) {
        setValue("NetworkProxy/networkProxyPassword", password);

        if (self) {
            emit self->networkProxyPasswordChanged(password);
        }
    }
}

QByteArray Settings::transferViewHeaderState() {
    return value("MainWindow/transferViewHeaderState").toByteArray();
}

void Settings::setTransferViewHeaderState(const QByteArray &state) {
    setValue("MainWindow/transferViewHeaderState", state);
}
