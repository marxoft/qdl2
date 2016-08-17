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

#include "javascriptserviceplugin.h"
#include "logger.h"
#include "pluginsettings.h"
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QScriptEngine>
#include <QUrl>

JavaScriptServicePlugin::JavaScriptServicePlugin(const QString &id, const QString &fileName, QObject *parent) :
    ServicePlugin(parent),
    m_global(0),
    m_engine(0),
    m_nam(0),
    m_fileName(fileName),
    m_id(id),
    m_evaluated(false)
{
}

QString JavaScriptServicePlugin::fileName() const {
    return m_fileName;
}

QString JavaScriptServicePlugin::id() const {
    return m_id;
}      

ServicePlugin* JavaScriptServicePlugin::createPlugin(QObject *parent) {
    return new JavaScriptServicePlugin(id(), fileName(), parent);
}

void JavaScriptServicePlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
    if (manager) {
        m_nam = manager;
        
        if (m_global) {
            m_global->setNetworkAccessManager(manager);
        }
    }
}

void JavaScriptServicePlugin::initEngine() {
    if (m_evaluated) {
        return;
    }
    
    if (!m_engine) {
        m_engine = new QScriptEngine(this);
    }

    QFile file(fileName());
    
    if (file.open(QFile::ReadOnly)) {
        const QScriptValue result = m_engine->evaluate(file.readAll(), fileName());
        file.close();
        
        if (result.isError()) {
            Logger::log("JavaScriptServicePlugin::initEngine(): Error evaluating JavaScript file: "
                        + result.toString());
            return;
        }
        
        Logger::log("JavaScriptServicePlugin::initEngine(): JavaScript file evaluated OK", Logger::HighVerbosity);
        m_evaluated = true;
        m_global = new JavaScriptServicePluginGlobalObject(m_engine);
        
        if (m_nam) {
            m_global->setNetworkAccessManager(m_nam);
        }
        
        connect(m_global, SIGNAL(captchaRequest(QString, QString, QScriptValue)),
                this, SLOT(onCaptchaRequest(QString, QString, QScriptValue)));
        connect(m_global, SIGNAL(downloadRequest(QVariantMap, QString, QString)),
                this, SLOT(onDownloadRequest(QVariantMap, QString, QString)));
        connect(m_global, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
        connect(m_global, SIGNAL(settingsRequest(QString, QVariantList, QScriptValue)),
                this, SLOT(onSettingsRequest(QString, QVariantList, QScriptValue)));
        connect(m_global, SIGNAL(urlChecked(QVariantMap)), this, SLOT(onUrlChecked(QVariantMap)));
        connect(m_global, SIGNAL(urlChecked(QVariantList, QString)), this, SLOT(onUrlChecked(QVariantList, QString)));
        connect(m_global, SIGNAL(waitRequest(int, bool)), this, SIGNAL(waitRequest(int, bool)));
        
        m_engine->installTranslatorFunctions();
        m_engine->globalObject().setProperty("settings", m_engine->newQObject(new PluginSettings(id(), m_engine)));
    }
    else {
        Logger::log("JavaScriptServicePlugin::initEngine(): Error reading JavaScript file: "
                    + file.errorString());
    }
}

bool JavaScriptServicePlugin::cancelCurrentOperation() {
    if (!m_engine) {
        return false;
    }

    return m_engine->globalObject().property("cancelCurrentOperation").call(QScriptValue()).toBool();
}

void JavaScriptServicePlugin::checkUrl(const QString &url) {
    initEngine();
    QScriptValue func = m_engine->globalObject().property("checkUrl");

    if (func.isFunction()) {
        const QScriptValue result = func.call(QScriptValue(), QScriptValueList() << url);

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptServicePlugin::checkUrl(). Error calling checkUrl(): " + errorString);
            emit error(tr("Error calling checkUrl(): %1").arg(errorString));
        }
    }
    else {
        Logger::log("JavaScriptServicePlugin::checkUrl(). checkUrl() function not defined");
        emit error(tr("checkUrl() function not defined"));
    }
}

void JavaScriptServicePlugin::getDownloadRequest(const QString &url) {
    initEngine();
    QScriptValue func = m_engine->globalObject().property("getDownloadRequest");

    if (func.isFunction()) {
        const QScriptValue result = func.call(QScriptValue(), QScriptValueList() << url);

        if (result.isError()) {
            emit error(tr("Error calling getDownloadRequest(): %1").arg(result.toString()));
        }
    }
    else {
        emit error(tr("getDownloadRequest() function not defined in the plugin"));
    }
}

void JavaScriptServicePlugin::submitCaptchaResponse(const QString &challenge, const QString &response) {
    initEngine();

    if (m_callback.isFunction()) {
        const QScriptValue result = m_callback.call(QScriptValue(), QScriptValueList() << challenge << response);

        if (result.isError()) {
            emit error(tr("Decaptcha callback error: %1").arg(result.toString()));
        }
    }
    else {
        QScriptValue func = m_engine->globalObject().property(m_callback.toString());
        
        if (func.isFunction()) {
            const QScriptValue result = func.call(QScriptValue(), QScriptValueList() << challenge << response);
            
            if (result.isError()) {
                emit error(tr("Error calling %1: %1").arg(m_callback.toString()).arg(result.toString()));
            }
        }
        else {
            emit error(tr("%1 function not defined in the plugin").arg(m_callback.toString()));
        }
    }
}

void JavaScriptServicePlugin::submitSettingsResponse(const QVariantMap &settings) {
    initEngine();

    if (m_callback.isFunction()) {
        const QScriptValue result = m_callback.call(QScriptValue(), QScriptValueList()
                                                    << m_engine->toScriptValue(settings));

        if (result.isError()) {
            emit error(tr("Decaptcha callback error: %1").arg(result.toString()));
        }
    }
    else {
        QScriptValue func = m_engine->globalObject().property(m_callback.toString());

        if (func.isFunction()) {
            const QScriptValue result = func.call(QScriptValue(), QScriptValueList()
                                                  << m_engine->toScriptValue(settings));

            if (result.isError()) {
                emit error(tr("Error calling %1: %1").arg(m_callback.toString()).arg(result.toString()));
            }
        }
        else {
            emit error(tr("%1 function not defined in the plugin").arg(m_callback.toString()));
        }
    }
}

void JavaScriptServicePlugin::onCaptchaRequest(const QString &recaptchaPluginId, const QString &recaptchaKey,
                                               const QScriptValue &callback) {
    m_callback = callback;
    emit captchaRequest(recaptchaPluginId, recaptchaKey, "submitCaptchaResponse");
}

void JavaScriptServicePlugin::onDownloadRequest(const QVariantMap &request, const QString &method, const QString &data) {
    QNetworkRequest req(request.value("url").toUrl());

    if (request.contains("headers")) {
        QMapIterator<QString, QVariant> iterator(request.value("headers").toMap());
        
        while (iterator.hasNext()) {
            iterator.next();
            req.setRawHeader(iterator.key().toUtf8(), iterator.value().toByteArray());
        }
    }

    emit downloadRequest(req, method.toUtf8(), data.toUtf8());
}

void JavaScriptServicePlugin::onSettingsRequest(const QString &title, const QVariantList &settings,
                                                const QScriptValue &callback) {
    m_callback = callback;
    emit settingsRequest(title, settings, "submitSettingsResponse");
}

void JavaScriptServicePlugin::onUrlChecked(const QVariantMap &result) {
    if ((!result.contains("url")) || (!result.contains("fileName"))) {
        emit error(tr("URL result is invalid"));
        return;
    }

    emit urlChecked(UrlResult(result.value("url").toString(), result.value("fileName").toString()));
}

void JavaScriptServicePlugin::onUrlChecked(const QVariantList &results, const QString &packageName) {
    if (results.isEmpty()) {
        emit error(tr("URL list is empty"));
        return;
    }

    UrlResultList list;

    foreach (const QVariant &var, results) {
        const QVariantMap result = var.toMap();

        if ((result.contains("url")) && (result.contains("fileName"))) {
            list << UrlResult(result.value("url").toString(), result.value("fileName").toString());
        }
    }

    emit urlChecked(list, packageName);
}

JavaScriptServicePluginGlobalObject::JavaScriptServicePluginGlobalObject(QScriptEngine *engine) :
    JavaScriptPluginGlobalObject(engine)
{
}
