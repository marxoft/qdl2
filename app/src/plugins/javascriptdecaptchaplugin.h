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
#include <QPointer>
#include <QScriptEngine>

class JavaScriptDecaptchaPlugin : public DecaptchaPlugin
{
    Q_OBJECT

public:
    explicit JavaScriptDecaptchaPlugin(const QScriptValue &plugin, QObject *parent = 0);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void getCaptchaResponse(int captchaType, const QByteArray &captchaData, const QVariantMap &settings);
    virtual void reportCaptchaResponse(const QString &captchaId);

    void submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
    void onSettingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);

private:
    bool init();
    
    QScriptValue m_plugin;
    QScriptValue m_callback;
    
    bool m_initted;
};

class JavaScriptDecaptchaPluginSignaller : public QObject
{
    Q_OBJECT

public:
    explicit JavaScriptDecaptchaPluginSignaller(QObject *parent = 0);

Q_SIGNALS:
    void captchaResponse(const QString &captchaId, const QString &response);
    void captchaResponseReported(const QString &captchaId);
    void error(const QString &errorString);
    void settingsRequest(const QString &title, const QVariantList &settings, const QScriptValue &callback);
};

class JavaScriptDecaptchaPluginFactory : public QObject, public DecaptchaPluginFactory
{
    Q_OBJECT
    Q_INTERFACES(DecaptchaPluginFactory)

public:
    explicit JavaScriptDecaptchaPluginFactory(const QString &fileName, QScriptEngine *engine, QObject *parent = 0);

    virtual DecaptchaPlugin* createPlugin(QObject *parent = 0);

private:
    bool init();

    QPointer<QScriptEngine> m_engine;

    QString m_fileName;

    QScriptValue m_constructor;

    bool m_initted;
};

#endif // JAVASCRIPTDECAPTCHAPLUGIN_H
