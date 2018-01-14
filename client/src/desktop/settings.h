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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QStringList>

class Request;

class Settings : public QObject
{
    Q_OBJECT

    // Client settings
    Q_PROPERTY(bool autoReloadEnabled READ autoReloadEnabled WRITE setAutoReloadEnabled
               NOTIFY autoReloadEnabledChanged)
    Q_PROPERTY(bool clipboardMonitorEnabled READ clipboardMonitorEnabled WRITE setClipboardMonitorEnabled
               NOTIFY clipboardMonitorEnabledChanged)
    Q_PROPERTY(bool createSubfolders READ createSubfolders WRITE setCreateSubfolders NOTIFY createSubfoldersChanged)
    Q_PROPERTY(QString defaultCategory READ defaultCategory WRITE setDefaultCategory NOTIFY defaultCategoryChanged)
    Q_PROPERTY(QString defaultServicePlugin READ defaultServicePlugin WRITE setDefaultServicePlugin
               NOTIFY defaultServicePluginChanged)
    Q_PROPERTY(bool usePlugins READ usePlugins WRITE setUsePlugins NOTIFY usePluginsChanged)
    Q_PROPERTY(QString loggerFileName READ loggerFileName WRITE setLoggerFileName NOTIFY loggerFileNameChanged)
    Q_PROPERTY(int loggerVerbosity READ loggerVerbosity WRITE setLoggerVerbosity NOTIFY loggerVerbosityChanged)
    Q_PROPERTY(QString serverHost READ serverHost WRITE setServerHost NOTIFY serverHostChanged)
    Q_PROPERTY(int serverPort READ serverPort WRITE setServerPort NOTIFY serverPortChanged)
    Q_PROPERTY(bool serverAuthenticationEnabled READ serverAuthenticationEnabled WRITE setServerAuthenticationEnabled
               NOTIFY serverAuthenticationEnabledChanged)
    Q_PROPERTY(QString serverUsername READ serverUsername WRITE setServerUsername NOTIFY serverUsernameChanged)
    Q_PROPERTY(QString serverPassword READ serverPassword WRITE setServerPassword NOTIFY serverPasswordChanged)
    Q_PROPERTY(QByteArray transferViewHeaderState READ transferViewHeaderState WRITE setTransferViewHeaderState)
    Q_PROPERTY(QByteArray windowGeometry READ windowGeometry WRITE setWindowGeometry)
    Q_PROPERTY(QByteArray windowState READ windowState WRITE setWindowState)

    // Server settings
    Q_PROPERTY(QString customCommand READ customCommand WRITE setCustomCommand NOTIFY customCommandChanged)
    Q_PROPERTY(bool customCommandEnabled READ customCommandEnabled WRITE setCustomCommandEnabled
               NOTIFY customCommandEnabledChanged)
    Q_PROPERTY(QString decaptchaPlugin READ decaptchaPlugin WRITE setDecaptchaPlugin NOTIFY decaptchaPluginChanged)
    Q_PROPERTY(QString downloadPath READ downloadPath WRITE setDownloadPath NOTIFY downloadPathChanged)
    Q_PROPERTY(bool extractArchives READ extractArchives WRITE setExtractArchives NOTIFY extractArchivesChanged)
    Q_PROPERTY(bool deleteExtractedArchives READ deleteExtractedArchives WRITE setDeleteExtractedArchives
               NOTIFY deleteExtractedArchivesChanged)
    Q_PROPERTY(QStringList archivePasswords READ archivePasswords WRITE setArchivePasswords
               NOTIFY archivePasswordsChanged)
    Q_PROPERTY(int maximumConcurrentTransfers READ maximumConcurrentTransfers WRITE setMaximumConcurrentTransfers
               NOTIFY maximumConcurrentTransfersChanged)
    Q_PROPERTY(bool startTransfersAutomatically READ startTransfersAutomatically WRITE setStartTransfersAutomatically
               NOTIFY startTransfersAutomaticallyChanged)
    Q_PROPERTY(int nextAction READ nextAction WRITE setNextAction NOTIFY nextActionChanged)
    Q_PROPERTY(bool networkProxyEnabled READ networkProxyEnabled WRITE setNetworkProxyEnabled
               NOTIFY networkProxyEnabledChanged)
    Q_PROPERTY(int networkProxyType READ networkProxyType WRITE setNetworkProxyType NOTIFY networkProxyTypeChanged)
    Q_PROPERTY(QString networkProxyHost READ networkProxyHost WRITE setNetworkProxyHost NOTIFY networkProxyHostChanged)
    Q_PROPERTY(int networkProxyPort READ networkProxyPort WRITE setNetworkProxyPort NOTIFY networkProxyPortChanged)
    Q_PROPERTY(bool networkProxyAuthenticationEnabled READ networkProxyAuthenticationEnabled
               WRITE setNetworkProxyAuthenticationEnabled NOTIFY networkProxyAuthenticationEnabledChanged)
    Q_PROPERTY(QString networkProxyUsername READ networkProxyUsername WRITE setNetworkProxyUsername
               NOTIFY networkProxyUsernameChanged)
    Q_PROPERTY(QString networkProxyPassword READ networkProxyPassword WRITE setNetworkProxyPassword
               NOTIFY networkProxyPasswordChanged)

public:
    ~Settings();

    static Settings* instance();

    // Client settings
    static bool autoReloadEnabled();

    static bool clipboardMonitorEnabled();

    static bool createSubfolders();

    static QString defaultCategory();
    
    static QString defaultServicePlugin();
    static bool usePlugins();

    static QString loggerFileName();
    static int loggerVerbosity();

    static QString serverHost();
    static int serverPort();
    static bool serverAuthenticationEnabled();
    static QString serverUsername();
    static QString serverPassword();

    static QByteArray transferViewHeaderState();
    static QByteArray windowGeometry();
    static QByteArray windowState();

    // Server settings
    QString customCommand() const;
    bool customCommandEnabled() const;

    QString decaptchaPlugin() const;

    QString downloadPath() const;

    bool extractArchives() const;
    bool deleteExtractedArchives() const;
    QStringList archivePasswords() const;

    int maximumConcurrentTransfers() const;
    bool startTransfersAutomatically() const;

    int nextAction() const;

    bool networkProxyEnabled() const;
    int networkProxyType() const;
    QString networkProxyHost() const;
    int networkProxyPort() const;
    bool networkProxyAuthenticationEnabled() const;
    QString networkProxyUsername() const;
    QString networkProxyPassword() const;

public Q_SLOTS:
    // Client settings
    static void setAutoReloadEnabled(bool enabled);

    static void setClipboardMonitorEnabled(bool enabled);

    static void setCreateSubfolders(bool enabled);

    static void setDefaultCategory(const QString &category);
    
    static void setDefaultServicePlugin(const QString &pluginId);
    static void setUsePlugins(bool enabled);

    static void setLoggerFileName(const QString &fileName);
    static void setLoggerVerbosity(int verbosity);

    static void setServerHost(const QString &host);
    static void setServerPort(int port);
    static void setServerAuthenticationEnabled(bool enabled);
    static void setServerUsername(const QString &username);
    static void setServerPassword(const QString &password);

    static void setTransferViewHeaderState(const QByteArray &state);
    static void setWindowGeometry(const QByteArray &geometry);
    static void setWindowState(const QByteArray &state);

    // Server settings
    void setCustomCommand(const QString &command, bool saveNow = true);
    void setCustomCommandEnabled(bool enabled, bool saveNow = true);

    void setDecaptchaPlugin(const QString &pluginId, bool saveNow = true);

    void setDownloadPath(const QString &path, bool saveNow = true);

    void setExtractArchives(bool enabled, bool saveNow = true);
    void setDeleteExtractedArchives(bool enabled, bool saveNow = true);
    void setArchivePasswords(const QStringList &passwords, bool saveNow = true);

    void setMaximumConcurrentTransfers(int maximum, bool saveNow = true);

    void setNextAction(int action, bool saveNow = true);

    void setStartTransfersAutomatically(bool enabled, bool saveNow = true);

    void setNetworkProxyEnabled(bool enabled, bool saveNow = true);
    void setNetworkProxyType(int type, bool saveNow = true);
    void setNetworkProxyHost(const QString &host, bool saveNow = true);
    void setNetworkProxyPort(int port, bool saveNow = true);
    void setNetworkProxyAuthenticationEnabled(bool enabled, bool saveNow = true);
    void setNetworkProxyUsername(const QString &username, bool saveNow = true);
    void setNetworkProxyPassword(const QString &password, bool saveNow = true);

    void restore();
    void save();

private Q_SLOTS:
    void onRequestFinished(Request *request);

Q_SIGNALS:
    void autoReloadEnabledChanged(bool enabled);
    void clipboardMonitorEnabledChanged(bool enabled);
    void createSubfoldersChanged(bool enabled);
    void customCommandChanged(const QString &command);
    void customCommandEnabledChanged(bool enabled);
    void decaptchaPluginChanged(const QString &pluginId);
    void defaultCategoryChanged(const QString &category);
    void defaultServicePluginChanged(const QString &pluginId);
    void usePluginsChanged(bool enabled);
    void downloadPathChanged(const QString &path);
    void extractArchivesChanged(bool enabled);
    void deleteExtractedArchivesChanged(bool enabled);
    void archivePasswordsChanged(const QStringList &passwords);
    void loggerFileNameChanged(const QString &fileName);
    void loggerVerbosityChanged(int verbosity);
    void maximumConcurrentTransfersChanged(int maximum);
    void nextActionChanged(int action);
    void startTransfersAutomaticallyChanged(bool enabled);
    void networkProxyEnabledChanged(bool enabled);
    void networkProxyTypeChanged(int type);
    void networkProxyHostChanged(const QString &host);
    void networkProxyPortChanged(int port);
    void networkProxyAuthenticationEnabledChanged(bool enabled);
    void networkProxyUsernameChanged(const QString &username);
    void networkProxyPasswordChanged(const QString &password);
    void serverHostChanged(const QString &host);
    void serverPortChanged(int port);
    void serverAuthenticationEnabledChanged(bool enabled);
    void serverUsernameChanged(const QString &username);
    void serverPasswordChanged(const QString &password);
    void error(const QString &errorString);

private:
    Settings();

    QString m_customCommand;
    bool m_customCommandEnabled;

    QString m_decaptchaPlugin;

    QString m_downloadPath;
    bool m_createSubfolders;

    bool m_extractArchives;
    bool m_deleteExtractedArchives;
    QStringList m_archivePasswords;

    int m_maximumConcurrentTransfers;
    int m_nextAction;
    bool m_startTransfersAutomatically;

    bool m_networkProxyEnabled;
    int m_networkProxyType;
    QString m_networkProxyHost;
    int m_networkProxyPort;
    bool m_networkProxyAuthenticationEnabled;
    QString m_networkProxyUsername;
    QString m_networkProxyPassword;

    static Settings *self;
};

#endif // SETTINGS_H
