/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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
#include "logger.h"
#include "request.h"
#include <QSettings>

static QVariant value(const QString &property, const QVariant &defaultValue = QVariant()) {
    return QSettings(APP_CONFIG_PATH + "settings", QSettings::IniFormat).value(property, defaultValue);
}

static void setValue(const QString &property, const QVariant &value) {
    QSettings(APP_CONFIG_PATH + "settings", QSettings::IniFormat).setValue(property, value);
}

Settings* Settings::self = 0;

Settings::Settings() :
    QObject(),
    m_customCommandEnabled(false),
    m_extractArchives(false),
    m_deleteExtractedArchives(false),
    m_maximumConcurrentTransfers(1),
    m_nextAction(0),
    m_networkProxyEnabled(false),
    m_networkProxyType(0),
    m_networkProxyPort(80),
    m_networkProxyAuthenticationEnabled(false)
{
}

Settings::~Settings() {
    self = 0;
}

Settings* Settings::instance() {
    return self ? self : self = new Settings;
}

// Client settings
bool Settings::autoReloadEnabled() {
    return value("autoReloadEnabled", false).toBool();
}

void Settings::setAutoReloadEnabled(bool enabled) {
    if (enabled != autoReloadEnabled()) {
        setValue("autoReloadEnabled", enabled);

        if (self) {
            emit self->autoReloadEnabledChanged(enabled);
        }
    }
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

bool Settings::createSubfolders() {
    return value("createSubfolders", false).toBool();
}

void Settings::setCreateSubfolders(bool enabled) {
    if (enabled != createSubfolders()) {
        setValue("createSubfolders", enabled);

        if (self) {
            emit self->createSubfoldersChanged(enabled);
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

QString Settings::loggerFileName() {
    return value("Logger/fileName", APP_CONFIG_PATH + "log").toString();
}

void Settings::setLoggerFileName(const QString &fileName) {
    if (fileName != loggerFileName()) {
        setValue("Logger/fileName", fileName);
        
        if (self) {
            emit self->loggerFileNameChanged(fileName);
        }
    }
}

int Settings::loggerVerbosity() {
    return value("Logger/verbosity", 0).toInt();
}

void Settings::setLoggerVerbosity(int verbosity) {
    if (verbosity != loggerVerbosity()) {
        setValue("Logger/verbosity", verbosity);
        
        if (self) {
            emit self->loggerVerbosityChanged(verbosity);
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

QString Settings::serverHost() {
    return value("Server/host").toString();
}

void Settings::setServerHost(const QString &host) {
    if (host != serverHost()) {
        setValue("Server/host", host);

        if (self) {
            emit self->serverHostChanged(host);
        }
    }
}

int Settings::serverPort() {
    return value("Server/port", 80).toInt();
}

void Settings::setServerPort(int port) {
    if (port != serverPort()) {
        setValue("Server/port", port);

        if (self) {
            emit self->serverPortChanged(port);
        }
    }
}

bool Settings::serverAuthenticationEnabled() {
    return value("Server/authenticationEnabled", false).toBool();
}

void Settings::setServerAuthenticationEnabled(bool enabled) {
    if (enabled != serverAuthenticationEnabled()) {
        setValue("Server/authenticationEnabled", enabled);

        if (self) {
            emit self->serverAuthenticationEnabledChanged(enabled);
        }
    }
}

QString Settings::serverUsername() {
    return value("Server/username").toString();
}

void Settings::setServerUsername(const QString &username) {
    if (username != serverUsername()) {
        setValue("Server/username", username);

        if (self) {
            emit self->serverUsernameChanged(username);
        }
    }
}

QString Settings::serverPassword() {
    return value("Server/password").toString();
}

void Settings::setServerPassword(const QString &password) {
    if (password != serverPassword()) {
        setValue("Server/password", password);

        if (self) {
            emit self->serverPasswordChanged(password);
        }
    }
}

QByteArray Settings::transferViewHeaderState() {
    return value("MainWindow/transferViewHeaderState").toByteArray();
}

void Settings::setTransferViewHeaderState(const QByteArray &state) {
    setValue("MainWindow/transferViewHeaderState", state);
}

QByteArray Settings::windowGeometry() {
    return value("MainWindow/windowGeometry").toByteArray();
}

void Settings::setWindowGeometry(const QByteArray &geometry) {
    setValue("MainWindow/windowGeometry", geometry);
}

QByteArray Settings::windowState() {
    return value("MainWindow/windowState").toByteArray();
}

void Settings::setWindowState(const QByteArray &state) {
    setValue("MainWindow/windowState", state);
}

// Server settings
QString Settings::customCommand() const {
    return m_customCommand;
}

void Settings::setCustomCommand(const QString &command, bool saveNow) {
    if (command != customCommand()) {
        m_customCommand = command;
        emit customCommandChanged(command);

        if (saveNow) {
            QVariantMap data;
            data["customCommand"] = command;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

bool Settings::customCommandEnabled() const {
    return m_customCommandEnabled;
}

void Settings::setCustomCommandEnabled(bool enabled, bool saveNow) {
    if (enabled != customCommandEnabled()) {
        m_customCommandEnabled = enabled;
        emit customCommandEnabledChanged(enabled);

        if (saveNow) {
            QVariantMap data;
            data["customCommandEnabled"] = enabled;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

QString Settings::decaptchaPlugin() const {
    return m_decaptchaPlugin;
}

void Settings::setDecaptchaPlugin(const QString &pluginId, bool saveNow) {
    if (pluginId != decaptchaPlugin()) {
        m_decaptchaPlugin = pluginId;
        emit decaptchaPluginChanged(pluginId);

        if (saveNow) {
            QVariantMap data;
            data["decaptchaPlugin"] = pluginId;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

QString Settings::downloadPath() const {
    return m_downloadPath;
}

void Settings::setDownloadPath(const QString &path, bool saveNow) {
    if (path != downloadPath()) {
        m_downloadPath = path;
        emit downloadPathChanged(path);

        if (saveNow) {
            QVariantMap data;
            data["downloadPath"] = path;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

bool Settings::extractArchives() const {
    return m_extractArchives;
}

void Settings::setExtractArchives(bool enabled, bool saveNow) {
    if (enabled != extractArchives()) {
        m_extractArchives = enabled;
        emit extractArchivesChanged(enabled);

        if (saveNow) {
            QVariantMap data;
            data["extractArchives"] = enabled;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

bool Settings::deleteExtractedArchives() const {
    return m_deleteExtractedArchives;
}

void Settings::setDeleteExtractedArchives(bool enabled, bool saveNow) {
    if (enabled != deleteExtractedArchives()) {
        m_deleteExtractedArchives = enabled;
        emit deleteExtractedArchivesChanged(enabled);

        if (saveNow) {
            QVariantMap data;
            data["deleteExtractedArchives"] = enabled;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

QStringList Settings::archivePasswords() const {
    return m_archivePasswords;
}

void Settings::setArchivePasswords(const QStringList &passwords, bool saveNow) {
    m_archivePasswords = passwords;
    emit archivePasswordsChanged(passwords);

        if (saveNow) {
            QVariantMap data;
            data["archivePasswords"] = passwords;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
}

int Settings::maximumConcurrentTransfers() const {
    return m_maximumConcurrentTransfers;
}

void Settings::setMaximumConcurrentTransfers(int maximum, bool saveNow) {
    if (maximum != maximumConcurrentTransfers()) {
        m_maximumConcurrentTransfers = maximum;
        emit maximumConcurrentTransfersChanged(maximum);

        if (saveNow) {
            QVariantMap data;
            data["maximumConcurrentTransfers"] = maximum;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

int Settings::nextAction() const {
    return m_nextAction;
}

void Settings::setNextAction(int action, bool saveNow) {
    if (action != nextAction()) {
        m_nextAction = action;
        emit nextActionChanged(action);

        if (saveNow) {
            QVariantMap data;
            data["nextAction"] = action;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

bool Settings::networkProxyEnabled() const {
    return m_networkProxyEnabled;
}

void Settings::setNetworkProxyEnabled(bool enabled, bool saveNow) {
    if (enabled != networkProxyEnabled()) {
        m_networkProxyEnabled = enabled;
        emit networkProxyEnabledChanged(enabled);

        if (saveNow) {
            QVariantMap data;
            data["networkProxyEnabled"] = enabled;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

int Settings::networkProxyType() const {
    return m_networkProxyType;
}

void Settings::setNetworkProxyType(int type, bool saveNow) {
    if (type != networkProxyType()) {
        m_networkProxyType = type;
        emit networkProxyTypeChanged(type);

        if (saveNow) {
            QVariantMap data;
            data["networkProxyType"] = type;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

QString Settings::networkProxyHost() const {
    return m_networkProxyHost;
}

void Settings::setNetworkProxyHost(const QString &host, bool saveNow) {
    if (host != networkProxyHost()) {
        m_networkProxyHost = host;
        emit networkProxyHostChanged(host);

        if (saveNow) {
            QVariantMap data;
            data["networkProxyHost"] = host;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

int Settings::networkProxyPort() const {
    return m_networkProxyPort;
}

void Settings::setNetworkProxyPort(int port, bool saveNow) {
    if (port != networkProxyPort()) {
        m_networkProxyPort = port;
        emit networkProxyPortChanged(port);

        if (saveNow) {
            QVariantMap data;
            data["networkProxyPort"] = port;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

bool Settings::networkProxyAuthenticationEnabled() const {
    return m_networkProxyAuthenticationEnabled;
}

void Settings::setNetworkProxyAuthenticationEnabled(bool enabled, bool saveNow) {
    if (enabled != networkProxyAuthenticationEnabled()) {
        m_networkProxyAuthenticationEnabled = enabled;
        emit networkProxyAuthenticationEnabledChanged(enabled);

        if (saveNow) {
            QVariantMap data;
            data["networkProxyAuthenticationEnabled"] = enabled;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

QString Settings::networkProxyUsername() const {
    return m_networkProxyUsername;
}

void Settings::setNetworkProxyUsername(const QString &username, bool saveNow) {
    if (username != networkProxyUsername()) {
        m_networkProxyUsername = username;
        emit networkProxyUsernameChanged(username);

        if (saveNow) {
            QVariantMap data;
            data["networkProxyUsername"] = username;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

QString Settings::networkProxyPassword() const {
    return m_networkProxyPassword;
}

void Settings::setNetworkProxyPassword(const QString &password, bool saveNow) {
    if (password != networkProxyPassword()) {
        m_networkProxyPassword = password;
        emit networkProxyPasswordChanged(password);

        if (saveNow) {
            QVariantMap data;
            data["networkProxyPassword"] = password;
            Request *request = new Request(this);
            request->put("/settings/setSettings", data);
            connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
        }
    }
}

void Settings::restore() {
    Request *request = new Request(this);
    request->get("/settings/getSettings");
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
}

void Settings::save() {
    QVariantMap data;
    data["customCommand"] = customCommand();
    data["customCommandEnabled"] = customCommandEnabled();
    data["decaptchaPlugin"] = decaptchaPlugin();
    data["downloadPath"] = downloadPath();
    data["extractArchives"] = extractArchives();
    data["deleteExtractedArchives"] = deleteExtractedArchives();
    data["archivePasswords"] = archivePasswords();
    data["maximumConcurrentTransfers"] = maximumConcurrentTransfers();
    data["nextAction"] = nextAction();
    data["networkProxyEnabled"] = networkProxyEnabled();
    data["networkProxyType"] = networkProxyType();
    data["networkProxyHost"] = networkProxyHost();
    data["networkProxyPort"] = networkProxyPort();
    data["networkProxyAuthenticationEnabled"] = networkProxyAuthenticationEnabled();
    data["networkProxyUsername"] = networkProxyUsername();
    data["networkProxyPassword"] = networkProxyPassword();
    Request *request = new Request(this);
    request->put("/settings/setSettings", data);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
}

void Settings::onRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        if (request->operation() == Request::GetOperation) {
            const QVariantMap result = request->result().toMap();
            setCustomCommand(result.value("customCommand").toString(), false);
            setCustomCommandEnabled(result.value("customCommandEnabled", false).toBool());
            setDecaptchaPlugin(result.value("decaptchaPlugin").toString(), false);
            setDownloadPath(result.value("downloadPath").toString(), false);
            setExtractArchives(result.value("extractArchives", false).toBool(), false);
            setDeleteExtractedArchives(result.value("deleteExtractedArchives", false).toBool(), false);
            setArchivePasswords(result.value("archivePasswords").toStringList(), false);
            setMaximumConcurrentTransfers(result.value("maximumConcurrentTransfers", 0).toInt(), false);
            setNextAction(result.value("nextAction", 0).toInt(), false);
            setNetworkProxyEnabled(result.value("networkProxyEnabled", false).toBool(), false);
            setNetworkProxyType(result.value("networkProxyType", 0).toInt(), false);
            setNetworkProxyHost(result.value("networkProxyHost").toString(), false);
            setNetworkProxyPort(result.value("networkProxyPort", 80).toInt(), false);
            setNetworkProxyAuthenticationEnabled(result.value("networkProxyAuthenticationEnabled", false).toBool(), false);
            setNetworkProxyUsername(result.value("networkProxyUsername").toString(), false);
            setNetworkProxyPassword(result.value("networkProxyPassword").toString(), false);
        }
    }
    else if (request->status() == Request::Error) {
        Logger::log("Settings::onRequestFinished(). Error: " + request->errorString());
        emit error(request->errorString());
    }

    request->deleteLater();
}
