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

#include "javascriptrecaptchaplugin.h"
#include "logger.h"
#include "pluginsettings.h"
#include <QFile>
#include <QImage>
#include <QScriptEngine>

JavaScriptRecaptchaPlugin::JavaScriptRecaptchaPlugin(const QString &id, const QString &fileName, QObject *parent) :
    RecaptchaPlugin(parent),
    m_engine(0),
    m_fileName(fileName),
    m_id(id)
{
}

QString JavaScriptRecaptchaPlugin::fileName() const {
    return m_fileName;
}

QString JavaScriptRecaptchaPlugin::id() const {
    return m_id;
}      

RecaptchaPlugin* JavaScriptRecaptchaPlugin::createPlugin(QObject *parent) {
    return new JavaScriptRecaptchaPlugin(id(), fileName(), parent);
}

void JavaScriptRecaptchaPlugin::initEngine() {
    if (!m_engine) {
        m_engine = new QScriptEngine(this);
        QFile file(fileName());

        if (file.open(QFile::ReadOnly)) {
            const QScriptValue result = m_engine->evaluate(file.readAll(), fileName());
            file.close();

            if (result.isError()) {
                Logger::log("JavaScriptRecaptchaPlugin::initEngine(): Error evaluating JavaScript file: "
                            + result.toString());
                delete m_engine;
                m_engine = 0;
                return;
            }
            
            Logger::log("JavaScriptRecaptchaPlugin::initEngine(): JavaScript file evaluated OK");            
        }
        else {
            Logger::log("JavaScriptRecaptchaPlugin::initEngine(): Error reading JavaScript file: "
                        + file.errorString());
            delete m_engine;
            m_engine = 0;
            return;
        }

        JavaScriptRecaptchaPluginGlobalObject *global = new JavaScriptRecaptchaPluginGlobalObject(m_engine);
        connect(global, SIGNAL(captcha(QString, QString)), this, SLOT(onCaptcha(QString, QString)));
        connect(global, SIGNAL(error(QString)), this, SIGNAL(error(QString)));
        connect(global, SIGNAL(settingsRequest(QString, QVariantList, QScriptValue)),
                this, SLOT(onSettingsRequest(QString, QVariantList, QScriptValue)));

        m_engine->installTranslatorFunctions();
        m_engine->globalObject().setProperty("settings", m_engine->newQObject(new PluginSettings(id(), m_engine)));
    }
}

bool JavaScriptRecaptchaPlugin::cancelCurrentOperation() {
    if (!m_engine) {
        return false;
    }

    return m_engine->globalObject().property("cancelCurrentOperation").call(QScriptValue()).toBool();
}

void JavaScriptRecaptchaPlugin::getCaptcha(const QString &captchaKey) {
    initEngine();
    QScriptValue func = m_engine->globalObject().property("getCaptcha");

    if (func.isFunction()) {
        const QScriptValue result = func.call(QScriptValue(), QScriptValueList() << captchaKey);

        if (result.isError()) {
            const QString errorString = result.toString();
            Logger::log("JavaScriptRecaptchaPlugin::getCaptcha(). Error calling getCaptcha(): " + errorString);
            emit error(tr("Error calling getCaptcha(): %1").arg(errorString));
        }
    }
    else {
        Logger::log("JavaScriptRecaptchaPlugin::getCaptcha(). getCaptcha() function not defined");
        emit error(tr("getCaptcha() function not defined"));
    }
}

void JavaScriptRecaptchaPlugin::submitSettingsResponse(const QVariantMap &settings) {
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

void JavaScriptRecaptchaPlugin::onCaptcha(const QString &challenge, const QString &imageData) {
    const QImage image = QImage::fromData(QByteArray::fromBase64(imageData.toUtf8()));

    if (image.isNull()) {
        emit error(tr("Invalid captcha image"));
    }
    else {
        emit captcha(challenge, image);
    }
}

void JavaScriptRecaptchaPlugin::onSettingsRequest(const QString &title, const QVariantList &settings,
                                                const QScriptValue &callback) {
    m_callback = callback;
    emit settingsRequest(title, settings, "submitSettingsResponse");
}

JavaScriptRecaptchaPluginGlobalObject::JavaScriptRecaptchaPluginGlobalObject(QScriptEngine *engine) :
    JavaScriptPluginGlobalObject(engine)
{
}
