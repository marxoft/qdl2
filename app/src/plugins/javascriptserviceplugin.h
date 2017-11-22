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

#ifndef JAVASCRIPTSERVICEPLUGIN_H
#define JAVASCRIPTSERVICEPLUGIN_H

#include "serviceplugin.h"
#include <QPointer>
#include <QScriptEngine>

class JavaScriptServicePlugin : public ServicePlugin
{
    Q_OBJECT

public:
    explicit JavaScriptServicePlugin(const QScriptValue &value, QObject *parent = 0);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void checkUrl(const QString &url, const QVariantMap &settings);
    virtual void getDownloadRequest(const QString &url, const QVariantMap &settings);
    
    void submitCaptchaResponse(const QString &challenge, const QString &response);
    void submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
    void onCaptchaRequest(const QString &recaptchaPluginId, int captchaType, const QString &captchaKey,
            const QScriptValue &callback);
    void onDownloadRequest(const QNetworkRequest &request, const QString &method, const QString &data);
    void onSettingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);

private:
    bool init();

    QScriptValue m_plugin;
    QScriptValue m_callback;

    bool m_initted;
};

class JavaScriptServicePluginSignaller : public QObject
{
    Q_OBJECT

public:
    explicit JavaScriptServicePluginSignaller(QObject *parent = 0);

Q_SIGNALS:
    void captchaRequest(const QString &recaptchaPluginId, int captchaType, const QString &captchaKey,
            const QScriptValue &callback);
    void downloadRequest(const QNetworkRequest &request, const QString &method = QString("GET"),
            const QString &data = QString());
    void error(const QString &errorString);
    void settingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);
    void urlChecked(const UrlResult &result);
    void urlChecked(const UrlResultList &results, const QString &packageName);
};

class JavaScriptServicePluginFactory : public QObject, public ServicePluginFactory
{
    Q_OBJECT
    Q_INTERFACES(ServicePluginFactory)

public:
    explicit JavaScriptServicePluginFactory(const QString &fileName, QScriptEngine *engine, QObject *parent = 0);

    virtual ServicePlugin* createPlugin(QObject *parent = 0);

private:
    bool init();

    QPointer<QScriptEngine> m_engine;

    QString m_fileName;

    QScriptValue m_constructor;

    bool m_initted;
};

#endif // JAVASCRIPTSERVICEPLUGIN_H
