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
#include "javascriptpluginglobalobject.h"

class QScriptEngine;

class JavaScriptServicePlugin : public ServicePlugin
{
    Q_OBJECT

    Q_INTERFACES(ServicePlugin)

    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString id READ id)

public:
    explicit JavaScriptServicePlugin(QObject *parent = 0);
    explicit JavaScriptServicePlugin(const QString &id, const QString &fileName, QObject *parent = 0);

    QString fileName() const;

    QString id() const;

    virtual ServicePlugin* createPlugin(QObject *parent = 0);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void checkUrl(const QString &url);
    virtual void getDownloadRequest(const QString &url);
    
    void submitCaptchaResponse(const QString &challenge, const QString &response);
    void submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
    void onCaptchaRequest(const QString &recaptchaPluginId, const QString &recaptchaKey, const QScriptValue &callback);
    void onDownloadRequest(const QVariantMap &request, const QString &method, const QString &data);
    void onSettingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);
    void onUrlChecked(const QVariantMap &result);
    void onUrlChecked(const QVariantList &results, const QString &packageName);

private:
    void initEngine();
    
    QScriptEngine *m_engine;

    QString m_fileName;
    QString m_id;

    QScriptValue m_callback;
};

class JavaScriptServicePluginGlobalObject : public JavaScriptPluginGlobalObject
{
    Q_OBJECT

public:
    explicit JavaScriptServicePluginGlobalObject(QScriptEngine *engine);

Q_SIGNALS:
    void captchaRequest(const QString &recaptchaPluginId, const QString &recaptchaKey, const QScriptValue &callback);
    void downloadRequest(const QVariantMap &request, const QString &method = QString("GET"),
                         const QString &data = QString());
    void error(const QString &errorString);
    void settingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);
    void urlChecked(const QVariantMap &result);
    void urlChecked(const QVariantList &results, const QString &packageName);
    void waitRequest(int msecs, bool isLongDelay);
};

#endif // JAVASCRIPTSERVICEPLUGIN_H
