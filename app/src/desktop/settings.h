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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>

class Settings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool clipboardMonitorEnabled READ clipboardMonitorEnabled WRITE setClipboardMonitorEnabled
               NOTIFY clipboardMonitorEnabledChanged)
    Q_PROPERTY(QString customCommand READ customCommand WRITE setCustomCommand NOTIFY customCommandChanged)
    Q_PROPERTY(bool customCommandEnabled READ customCommandEnabled WRITE setCustomCommandEnabled
               NOTIFY customCommandEnabledChanged)
    Q_PROPERTY(QString decaptchaPlugin READ decaptchaPlugin WRITE setDecaptchaPlugin NOTIFY decaptchaPluginChanged)
    Q_PROPERTY(QString defaultCategory READ defaultCategory WRITE setDefaultCategory NOTIFY defaultCategoryChanged)
    Q_PROPERTY(QString defaultServicePlugin READ defaultServicePlugin WRITE setDefaultServicePlugin
               NOTIFY defaultServicePluginChanged)
    Q_PROPERTY(bool usePlugins READ usePlugins WRITE setUsePlugins NOTIFY usePluginsChanged)
    Q_PROPERTY(QString downloadPath READ downloadPath WRITE setDownloadPath NOTIFY downloadPathChanged)
    Q_PROPERTY(bool createSubfolders READ createSubfolders WRITE setCreateSubfolders NOTIFY createSubfoldersChanged)
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
    Q_PROPERTY(bool webInterfaceEnabled READ webInterfaceEnabled WRITE setWebInterfaceEnabled
               NOTIFY webInterfaceEnabledChanged)
    Q_PROPERTY(int webInterfacePort READ webInterfacePort WRITE setWebInterfacePort NOTIFY webInterfacePortChanged)
    Q_PROPERTY(bool webInterfaceAuthenticationEnabled READ webInterfaceAuthenticationEnabled
               WRITE setWebInterfaceAuthenticationEnabled NOTIFY webInterfaceAuthenticationEnabledChanged)
    Q_PROPERTY(QString webInterfaceUsername READ webInterfaceUsername WRITE setWebInterfaceUsername
               NOTIFY webInterfaceUsernameChanged)
    Q_PROPERTY(QString webInterfacePassword READ webInterfacePassword WRITE setWebInterfacePassword
               NOTIFY webInterfacePasswordChanged)
    Q_PROPERTY(QByteArray transferViewHeaderState READ transferViewHeaderState WRITE setTransferViewHeaderState)
    Q_PROPERTY(QByteArray windowGeometry READ windowGeometry WRITE setWindowGeometry)
    Q_PROPERTY(QByteArray windowState READ windowState WRITE setWindowState)

public:
    ~Settings();

    static Settings* instance();

    static bool clipboardMonitorEnabled();

    static QString customCommand();
    static bool customCommandEnabled();

    static QString decaptchaPlugin();

    static QString defaultCategory();

    static QString defaultServicePlugin();
    static bool usePlugins();

    static QString downloadPath();
    static bool createSubfolders();

    static bool extractArchives();
    static bool deleteExtractedArchives();
    static QStringList archivePasswords();

    static int maximumConcurrentTransfers();
    static bool startTransfersAutomatically();

    static int nextAction();

    static bool networkProxyEnabled();
    static int networkProxyType();
    static QString networkProxyHost();
    static int networkProxyPort();
    static bool networkProxyAuthenticationEnabled();
    static QString networkProxyUsername();
    static QString networkProxyPassword();

    static bool webInterfaceEnabled();
    static int webInterfacePort();
    static bool webInterfaceAuthenticationEnabled();
    static QString webInterfaceUsername();
    static QString webInterfacePassword();
    
    static QByteArray transferViewHeaderState();
    static QByteArray windowGeometry();
    static QByteArray windowState();

public Q_SLOTS:    
    static void setClipboardMonitorEnabled(bool enabled);

    static void setCustomCommand(const QString &command);
    static void setCustomCommandEnabled(bool enabled);

    static void setDecaptchaPlugin(const QString &pluginId);
    
    static void setDefaultCategory(const QString &category);

    static void setDefaultServicePlugin(const QString &pluginId);
    static void setUsePlugins(bool enabled);

    static void setDownloadPath(const QString &path);
    static void setCreateSubfolders(bool enabled);

    static void setExtractArchives(bool enabled);
    static void setDeleteExtractedArchives(bool enabled);
    static void setArchivePasswords(const QStringList &passwords);

    static void setMaximumConcurrentTransfers(int maximum);

    static void setNextAction(int action);

    static void setStartTransfersAutomatically(bool enabled);

    static void setNetworkProxyEnabled(bool enabled);
    static void setNetworkProxyType(int type);
    static void setNetworkProxyHost(const QString &host);
    static void setNetworkProxyPort(int port);
    static void setNetworkProxyAuthenticationEnabled(bool enabled);
    static void setNetworkProxyUsername(const QString &username);
    static void setNetworkProxyPassword(const QString &password);

    static void setWebInterfaceEnabled(bool enabled);
    static void setWebInterfacePort(int port);
    static void setWebInterfaceAuthenticationEnabled(bool enabled);
    static void setWebInterfaceUsername(const QString &username);
    static void setWebInterfacePassword(const QString &password);
    
    static void setTransferViewHeaderState(const QByteArray &state);
    static void setWindowGeometry(const QByteArray &geometry);
    static void setWindowState(const QByteArray &state);

Q_SIGNALS:
    void clipboardMonitorEnabledChanged(bool enabled);
    void customCommandChanged(const QString &command);
    void customCommandEnabledChanged(bool enabled);
    void decaptchaPluginChanged(const QString &pluginId);
    void defaultCategoryChanged(const QString &category);
    void defaultServicePluginChanged(const QString &pluginId);
    void usePluginsChanged(bool enabled);
    void downloadPathChanged(const QString &path);
    void createSubfoldersChanged(bool enabled);
    void extractArchivesChanged(bool enabled);
    void deleteExtractedArchivesChanged(bool enabled);
    void archivePasswordsChanged(const QStringList &passwords);
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
    void webInterfaceEnabledChanged(bool enabled);
    void webInterfacePortChanged(int port);
    void webInterfaceAuthenticationEnabledChanged(bool enabled);
    void webInterfaceUsernameChanged(const QString &username);
    void webInterfacePasswordChanged(const QString &password);

private:
    Settings();

    static Settings *self;
};

#endif // SETTINGS_H
