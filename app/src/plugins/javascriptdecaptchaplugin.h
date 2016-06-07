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

#ifndef JAVASCRIPTDECAPTCHAPLUGIN_H
#define JAVASCRIPTDECAPTCHAPLUGIN_H

#include "decaptchaplugin.h"
#include "javascriptpluginglobalobject.h"

class QScriptEngine;

class JavaScriptDecaptchaPlugin : public DecaptchaPlugin
{
    Q_OBJECT

    Q_INTERFACES(DecaptchaPlugin)

    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString id READ id)

public:
    explicit JavaScriptDecaptchaPlugin(QObject *parent = 0);
    explicit JavaScriptDecaptchaPlugin(const QString &id, const QString &fileName, QObject *parent = 0);

    QString fileName() const;

    QString id() const;

    virtual DecaptchaPlugin* createPlugin(QObject *parent = 0);
    
    virtual void setNetworkAccessManager(QNetworkAccessManager *manager);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void getCaptchaResponse(const QImage &image);
    virtual void reportCaptchaResponse(const QString &captchaId);

    void submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
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

class JavaScriptDecaptchaPluginGlobalObject : public JavaScriptPluginGlobalObject
{
    Q_OBJECT

public:
    explicit JavaScriptDecaptchaPluginGlobalObject(QScriptEngine *engine);

Q_SIGNALS:
    void captchaResponse(const QString &captchaId, const QString &response);
    void captchaResponseReported(const QString &captchaId);
    void error(const QString &errorString);
    void settingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);
};

#endif // JAVASCRIPTDECAPTCHAPLUGIN_H
