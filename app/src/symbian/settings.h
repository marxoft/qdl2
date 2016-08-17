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
#include <QStringList>

class Settings : public QObject
{
    Q_OBJECT

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
    Q_PROPERTY(QString loggerFileName READ loggerFileName WRITE setLoggerFileName NOTIFY loggerFileNameChanged)
    Q_PROPERTY(int loggerVerbosity READ loggerVerbosity WRITE setLoggerVerbosity NOTIFY loggerVerbosityChanged)
    Q_PROPERTY(int maximumConcurrentTransfers READ maximumConcurrentTransfers WRITE setMaximumConcurrentTransfers
               NOTIFY maximumConcurrentTransfersChanged)
    Q_PROPERTY(bool startTransfersAutomatically READ startTransfersAutomatically
               WRITE setStartTransfersAutomatically NOTIFY startTransfersAutomaticallyChanged)
    Q_PROPERTY(int nextAction READ nextAction WRITE setNextAction NOTIFY nextActionChanged)
    Q_PROPERTY(bool networkProxyEnabled READ networkProxyEnabled WRITE setNetworkProxyEnabled
               NOTIFY networkProxyEnabledChanged)
    Q_PROPERTY(int networkProxyType READ networkProxyType WRITE setNetworkProxyType NOTIFY networkProxyTypeChanged)
    Q_PROPERTY(QString networkProxyHost READ networkProxyHost WRITE setNetworkProxyHost
               NOTIFY networkProxyHostChanged)
    Q_PROPERTY(int networkProxyPort READ networkProxyPort WRITE setNetworkProxyPort NOTIFY networkProxyPortChanged)
    Q_PROPERTY(bool networkProxyAuthenticationEnabled READ networkProxyAuthenticationEnabled
               WRITE setNetworkProxyAuthenticationEnabled NOTIFY networkProxyAuthenticationEnabledChanged)
    Q_PROPERTY(QString networkProxyUsername READ networkProxyUsername WRITE setNetworkProxyUsername
               NOTIFY networkProxyUsernameChanged)
    Q_PROPERTY(QString networkProxyPassword READ networkProxyPassword WRITE setNetworkProxyPassword
               NOTIFY networkProxyPasswordChanged)
    Q_PROPERTY(int screenOrientation READ screenOrientation WRITE setScreenOrientation
              NOTIFY screenOrientationChanged)

public:
    ~Settings();

    static Settings* instance();

    static QString customCommand();
    static bool customCommandEnabled();

    static QString decaptchaPlugin();

    static QString defaultCategory();

    static QString defaultServicePlugin();
    static bool usePlugins();

    static QString downloadPath();
    static bool createSubfolders();
    
    static QString loggerFileName();
    static int loggerVerbosity();

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
    
    static int screenOrientation();

public Q_SLOTS:
    static void setCustomCommand(const QString &command);
    static void setCustomCommandEnabled(bool enabled);

    static void setDecaptchaPlugin(const QString &pluginId);
    
    static void setDefaultCategory(const QString &category);

    static void setDefaultServicePlugin(const QString &pluginId);
    static void setUsePlugins(bool enabled);

    static void setDownloadPath(const QString &path);
    static void setCreateSubfolders(bool enabled);
    
    static void setLoggerFileName(const QString &fileName);
    static void setLoggerVerbosity(int verbosity);

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
    static void setNetworkProxy();
    
    static void setScreenOrientation(int orientation);

Q_SIGNALS:
    void customCommandChanged(const QString &command);
    void customCommandEnabledChanged(bool enabled);
    void decaptchaPluginChanged(const QString &pluginId);
    void defaultCategoryChanged(const QString &category);
    void defaultServicePluginChanged(const QString &pluginId);
    void usePluginsChanged(bool enabled);
    void downloadPathChanged(const QString &path);
    void createSubfoldersChanged(bool enabled);
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
    void screenOrientationChanged(int orientation);

private:
    Settings();

    static Settings *self;
};

#endif // SETTINGS_H
