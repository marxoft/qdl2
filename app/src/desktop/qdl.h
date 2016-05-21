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

#ifndef QDL_H
#define QDL_H

#include <QObject>
#include <QPointer>
#include <QVariantMap>

class MainWindow;

class Qdl : public QObject
{
    Q_OBJECT

    Q_ENUMS(Action UrlAction)

    Q_CLASSINFO("D-Bus Interface", "org.marxoft.qdl2")

public:
    enum Action {
        Continue = 0,
        Stop,
        Quit
    };

    enum UrlAction {
        AddUrls = 0,
        RetrieveUrls
    };
    
    ~Qdl();

    static Qdl* instance();

public Q_SLOTS:
    Q_SCRIPTABLE static void quit();
    Q_SCRIPTABLE static void showWindow();
    Q_SCRIPTABLE static void closeWindow();

    Q_SCRIPTABLE static void addTransfer(const QString &url);
    Q_SCRIPTABLE static void addTransfers(const QStringList &urls);
    Q_SCRIPTABLE static QVariantMap getTransfers(int offset = 0, int limit = -1, bool includeChildren = true);
    Q_SCRIPTABLE static QVariantMap getTransfer(const QString &id, bool includeChildren = true);
    Q_SCRIPTABLE static bool setTransferProperty(const QString &id, const QString &property, const QVariant &value);
    Q_SCRIPTABLE static bool setTransferProperties(const QString &id, const QVariantMap &properties);
    Q_SCRIPTABLE static bool startTransfer(const QString &id);
    Q_SCRIPTABLE static bool pauseTransfer(const QString &id);
    Q_SCRIPTABLE static bool removeTransfer(const QString &id, bool deleteFiles = false);
    Q_SCRIPTABLE static bool moveTransfer(const QString &sourceId, const QString &destinationId, int destinationRow = -1);
    Q_SCRIPTABLE static void startTransfers();
    Q_SCRIPTABLE static void pauseTransfers();

    Q_SCRIPTABLE static bool addCategory(const QString &name, const QString &path);
    Q_SCRIPTABLE static QVariantList getCategories();
    Q_SCRIPTABLE static QVariantMap getCategory(const QString &name);
    Q_SCRIPTABLE static bool removeCategory(const QString &name);

    Q_SCRIPTABLE static QStringList getClipboardUrls();
    Q_SCRIPTABLE static bool removeClipboardUrl(const QString &url);

    Q_SCRIPTABLE static QVariantList getDecaptchaPlugins();
    Q_SCRIPTABLE static QVariantMap getDecaptchaPlugin(const QString &id);
    Q_SCRIPTABLE static QVariantList getDecaptchaPluginSettings(const QString &id);
    Q_SCRIPTABLE static bool setDecaptchaPluginSettings(const QString &id, const QVariantMap &properties);

    Q_SCRIPTABLE static QVariantList getRecaptchaPlugins();
    Q_SCRIPTABLE static QVariantMap getRecaptchaPlugin(const QString &id);
    Q_SCRIPTABLE static QVariantList getRecaptchaPluginSettings(const QString &id);
    Q_SCRIPTABLE static bool setRecaptchaPluginSettings(const QString &id, const QVariantMap &properties);

    Q_SCRIPTABLE static QVariantList getServicePlugins();
    Q_SCRIPTABLE static QVariantMap getServicePlugin(const QString &id);
    Q_SCRIPTABLE static QVariantList getServicePluginSettings(const QString &id);
    Q_SCRIPTABLE static bool setServicePluginSettings(const QString &id, const QVariantMap &properties);

    Q_SCRIPTABLE static QVariantMap getSettings(const QStringList &settings);
    Q_SCRIPTABLE static bool setSettings(const QVariantMap &settings);

    Q_SCRIPTABLE static void addUrlChecks(const QStringList &urls, const QString &category = QString());
    Q_SCRIPTABLE static void clearUrlChecks();
    Q_SCRIPTABLE static QVariantMap getUrlChecks();
    Q_SCRIPTABLE static QVariantMap getUrlCheck(const QString &url);
    Q_SCRIPTABLE static bool removeUrlCheck(const QString &url);

    Q_SCRIPTABLE static void addUrlRetrievals(const QStringList &urls, const QString &pluginId = QString());
    Q_SCRIPTABLE static void clearUrlRetrievals();
    Q_SCRIPTABLE static QVariantMap getUrlRetrievals();
    Q_SCRIPTABLE static QVariantMap getUrlRetrieval(const QString &url);
    Q_SCRIPTABLE static bool removeUrlRetrieval(const QString &url);

private:
    Qdl();

    static Qdl *self;

    static QPointer<MainWindow> window;
};

#endif // QDL_H
