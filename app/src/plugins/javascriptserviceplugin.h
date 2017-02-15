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
#include <QNetworkRequest>
#include <QScriptable>

class JavaScriptServicePlugin : public ServicePlugin
{
    Q_OBJECT

    Q_INTERFACES(ServicePlugin)

    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString id READ id)

public:
    explicit JavaScriptServicePlugin(const QString &id, const QString &fileName, QObject *parent = 0);

    QString fileName() const;

    QString id() const;

    virtual ServicePlugin* createPlugin(QObject *parent = 0);
    
    virtual void setNetworkAccessManager(QNetworkAccessManager *manager);
    
public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void checkUrl(const QString &url);
    virtual void getDownloadRequest(const QString &url);
    
    void submitCaptchaResponse(const QString &challenge, const QString &response);
    void submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
    void onCaptchaRequest(const QString &recaptchaPluginId, const QString &recaptchaKey, const QScriptValue &callback);
    void onDownloadRequest(const QNetworkRequest &request, const QString &method, const QString &data);
    void onSettingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);

private:
    void initEngine();
    
    JavaScriptPluginGlobalObject *m_global;
    QScriptEngine *m_engine;
    QPointer<QNetworkAccessManager> m_nam;

    QString m_fileName;
    QString m_id;

    QScriptValue m_callback;
    
    bool m_evaluated;
};

class JavaScriptServicePluginGlobalObject : public JavaScriptPluginGlobalObject
{
    Q_OBJECT

public:
    explicit JavaScriptServicePluginGlobalObject(QScriptEngine *engine);

Q_SIGNALS:
    void captchaRequest(const QString &recaptchaPluginId, const QString &recaptchaKey, const QScriptValue &callback);
    void downloadRequest(const QNetworkRequest &request, const QString &method = QString("GET"),
                         const QString &data = QString());
    void error(const QString &errorString);
    void settingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);
    void urlChecked(const UrlResult &result);
    void urlChecked(const UrlResultList &results, const QString &packageName);
    void waitRequest(int msecs, bool isLongDelay);

private:
    static QScriptValue newNetworkRequest(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newUrlResult(QScriptContext *context, QScriptEngine *engine);
};

class JavaScriptNetworkRequest : public QObject, public QScriptable
{
    Q_OBJECT
    
    Q_PROPERTY(QString url READ url WRITE setUrl)
    Q_PROPERTY(QVariantMap headers READ headers WRITE setHeaders)
    
public:
    explicit JavaScriptNetworkRequest(QObject *parent = 0);
    
    QString url() const;
    void setUrl(const QString &u);
    
    QVariantMap headers() const;
    void setHeaders(const QVariantMap &h);

public Q_SLOTS:
    QVariant header(const QString &name) const;
    void setHeader(const QString &name, const QVariant &value);
};

class JavaScriptUrlResult : public QObject, public QScriptable
{
    Q_OBJECT
    
    Q_PROPERTY(QString url READ url WRITE setUrl)
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)

public:
    explicit JavaScriptUrlResult(QObject *parent = 0);
    
    QString url() const;
    void setUrl(const QString &u);
    
    QString fileName() const;
    void setFileName(const QString &f);
};

Q_DECLARE_METATYPE(QNetworkRequest*)
Q_DECLARE_METATYPE(UrlResult*)

#endif // JAVASCRIPTSERVICEPLUGIN_H
