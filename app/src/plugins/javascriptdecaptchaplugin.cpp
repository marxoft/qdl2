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

#include "javascriptdecaptchaplugin.h"
#include "logger.h"
#include "pluginsettings.h"
#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QNetworkAccessManager>

JavaScriptDecaptchaPlugin::JavaScriptDecaptchaPlugin(const QString &id, const QString &fileName, QObject *parent) :
    DecaptchaPlugin(parent),
    m_global(0),
    m_engine(0),
    m_nam(0),
    m_fileName(fileName),
    m_id(id),
    m_evaluated(false)
{
}

QString JavaScriptDecaptchaPlugin::fileName() const {
    return m_fileName;
}

QString JavaScriptDecaptchaPlugin::id() const {
    return m_id;
}      

DecaptchaPlugin* JavaScriptDecaptchaPlugin::createPlugin(QObject *parent) {
    return new JavaScriptDecaptchaPlugin(id(), fileName(), parent);
}

void JavaScriptDecaptchaPlugin::setNetworkAccessManager(QNetworkAccessManager *manager) {
    if (manager) {
        m_nam = manager;
        
        if (m_global) {
            m_global->setNetworkAccessManager(manager);
        }
    }
}

void JavaScriptDecaptchaPlugin::initEngine() {
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
            Logger::log("JavaScriptDecaptchaPlugin::initEngine(): Error evaluating JavaScript file: "
                        + result.toString());
            return;
        }
        
        Logger::log("JavaScriptDecaptchaPlugin::initEngine(): JavaScript file evaluated OK", Logger::HighVerbosity);
        m_evaluated = true;
        m_global = new JavaScriptDecaptchaPluginGlobalObject(m_engine);
        
        if (m_nam) {
            m_global->setNetworkAccessManager(m_nam);
        }
        
        connect(m_global, SIGNAL(captchaResponse(QString, QString)), this, SIGNAL(captchaResponse(QString, QString)));
        connect(m_global, SIGNAL(captchaResponseReported(QString)), this, SIGNAL(captchaResponseReported(QString)));
        connect(m_global, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
        connect(m_global, SIGNAL(settingsRequest(QString, QVariantList, QScriptValue)),
                this, SLOT(onSettingsRequest(QString, QVariantList, QScriptValue)));

        m_engine->installTranslatorFunctions();
        m_engine->globalObject().setProperty("settings", m_engine->newQObject(new PluginSettings(id(), m_engine)));
    }
    else {
        Logger::log("JavaScriptDecaptchaPlugin::initEngine(): Error reading JavaScript file: "
                    + file.errorString());
    }
}

bool JavaScriptDecaptchaPlugin::cancelCurrentOperation() {
    if (!m_engine) {
        return false;
    }

    return m_engine->globalObject().property("cancelCurrentOperation").call(QScriptValue()).toBool();
}

void JavaScriptDecaptchaPlugin::getCaptchaResponse(const QImage &image) {
    if (image.isNull()) {
        emit error(tr("Invalid captcha image"));
        return;
    }

    QByteArray ba;
    QBuffer buffer(&ba);
    
    if (buffer.open(QBuffer::WriteOnly)) {
        image.save(&buffer);
        buffer.close();
    }

    if (ba.isEmpty()) {
        emit error(tr("Invalid captcha image"));
        return;
    }
    
    initEngine();
    QScriptValue func = m_engine->globalObject().property("getCaptchaResponse");

    if (func.isFunction()) {
        const QScriptValue result = func.call(QScriptValue(), QScriptValueList() << QString::fromUtf8(ba.toBase64()));

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptDecaptchaPlugin::getCaptchaResponse(). Error calling getCaptchaResponse(): "
                        + errorString);
            emit error(tr("Error calling getCaptchaResponse(): %1").arg(errorString));
        }
    }
    else {
        Logger::log("JavaScriptDecaptchaPlugin::getCaptchaResponse(). getCaptchaResponse() function not defined");
        emit error(tr("getCaptchaResponse() function not defined"));
    }
}

void JavaScriptDecaptchaPlugin::reportCaptchaResponse(const QString &captchaId) {
    initEngine();
    QScriptValue func = m_engine->globalObject().property("reportCaptchaResponse");

    if (func.isFunction()) {
        const QScriptValue result = func.call(QScriptValue(), QScriptValueList() << captchaId);

        if (result.isError()) {
            emit error(tr("Error calling reportCaptchaResponse(): %1").arg(result.toString()));
        }
    }
    else {
        emit error(tr("reportCaptchaResponse() function not defined in the plugin"));
    }
}

void JavaScriptDecaptchaPlugin::submitSettingsResponse(const QVariantMap &settings) {
    initEngine();

    if (m_callback.isFunction()) {
        const QScriptValue result = m_callback.call(QScriptValue(), QScriptValueList()
                                                    << m_engine->toScriptValue(settings));

        if (result.isError()) {
            emit error(tr("Settings callback error: %1").arg(result.toString()));
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

void JavaScriptDecaptchaPlugin::onSettingsRequest(const QString &title, const QVariantList &settings,
                                                  const QScriptValue &callback) {
    m_callback = callback;
    emit settingsRequest(title, settings, "submitSettingsRepsonse");
}

JavaScriptDecaptchaPluginGlobalObject::JavaScriptDecaptchaPluginGlobalObject(QScriptEngine *engine) :
    JavaScriptPluginGlobalObject(engine)
{
}
