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

#include "javascriptpluginglobalobject.h"
#include "javascriptcaptchatype.h"
#include "javascriptdecaptchaplugin.h"
#include "javascriptnetworkrequest.h"
#include "javascriptrecaptchaplugin.h"
#include "javascriptsearchplugin.h"
#include "javascriptsearchresult.h"
#include "javascriptserviceplugin.h"
#include "javascripturlresult.h"
#include "logger.h"
#include "xmlhttprequest.h"
#include <QNetworkAccessManager>
#include <QScriptValueIterator>
#include <QTimerEvent>

JavaScriptPluginGlobalObject::JavaScriptPluginGlobalObject(QScriptEngine *engine) :
    QObject(engine),
    m_engine(engine),
    m_nam(0)
{
    QScriptValue oldGlobal = engine->globalObject();
    QScriptValue thisGlobal = engine->newQObject(this, QScriptEngine::QtOwnership,
            QScriptEngine::ExcludeChildObjects | QScriptEngine::ExcludeDeleteLater);

    thisGlobal.setProperty("DecaptchaPlugin",
                engine->newQMetaObject(&JavaScriptDecaptchaPluginSignaller::staticMetaObject,
                engine->newFunction(newDecaptchaPlugin)));
    thisGlobal.setProperty("RecaptchaPlugin",
                engine->newQMetaObject(&JavaScriptRecaptchaPluginSignaller::staticMetaObject,
                engine->newFunction(newRecaptchaPlugin)));
    thisGlobal.setProperty("SearchPlugin",
                engine->newQMetaObject(&JavaScriptSearchPluginSignaller::staticMetaObject,
                engine->newFunction(newSearchPlugin)));
    thisGlobal.setProperty("ServicePlugin",
                engine->newQMetaObject(&JavaScriptServicePluginSignaller::staticMetaObject,
                engine->newFunction(newServicePlugin)));
    thisGlobal.setProperty("CaptchaType", engine->newQMetaObject(&JavaScriptCaptchaType::staticMetaObject));
    QScriptValue request = engine->newQObject(new JavaScriptNetworkRequest(engine));
    engine->setDefaultPrototype(qMetaTypeId<QNetworkRequest>(), request);
    engine->setDefaultPrototype(qMetaTypeId<QNetworkRequest*>(), request);
    thisGlobal.setProperty("NetworkRequest", engine->newFunction(newNetworkRequest));
    QScriptValue sr = engine->newQObject(new JavaScriptSearchResult(engine));
    engine->setDefaultPrototype(qMetaTypeId<SearchResult>(), sr);
    engine->setDefaultPrototype(qMetaTypeId<SearchResult*>(), sr);
    thisGlobal.setProperty("SearchResult", engine->newFunction(newSearchResult));
    qScriptRegisterSequenceMetaType<SearchResultList>(engine);
    QScriptValue ur = engine->newQObject(new JavaScriptUrlResult(engine));
    engine->setDefaultPrototype(qMetaTypeId<UrlResult>(), ur);
    engine->setDefaultPrototype(qMetaTypeId<UrlResult*>(), ur);
    thisGlobal.setProperty("UrlResult", engine->newFunction(newUrlResult));
    qScriptRegisterSequenceMetaType<UrlResultList>(engine);
    thisGlobal.setProperty("XMLHttpRequest", engine->newQMetaObject(&XMLHttpRequest::staticMetaObject,
                engine->newFunction(newXMLHttpRequest)));
    
    QScriptValueIterator iterator(oldGlobal);

    while (iterator.hasNext()) {
        iterator.next();
        thisGlobal.setProperty(iterator.name(), iterator.value());
    }

    engine->setGlobalObject(thisGlobal);
}

QScriptValue JavaScriptPluginGlobalObject::newDecaptchaPlugin(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        return engine->newQObject(new JavaScriptDecaptchaPluginSignaller, QScriptEngine::ScriptOwnership);
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newRecaptchaPlugin(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        return engine->newQObject(new JavaScriptRecaptchaPluginSignaller, QScriptEngine::ScriptOwnership);
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newSearchPlugin(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        return engine->newQObject(new JavaScriptSearchPluginSignaller, QScriptEngine::ScriptOwnership);
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newServicePlugin(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        return engine->newQObject(new JavaScriptServicePluginSignaller, QScriptEngine::ScriptOwnership);
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newNetworkRequest(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        switch (context->argumentCount()) {
        case 0:
            return engine->toScriptValue(QNetworkRequest());
        case 1:
            return engine->toScriptValue(QNetworkRequest(context->argument(0).toString()));
        default:
            return context->throwError(QScriptContext::SyntaxError,
                    QObject::tr("NetworkRequest constructor requires either 0 or 1 arguments."));
        }
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newSearchResult(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        switch (context->argumentCount()) {
        case 0:
            return engine->toScriptValue(SearchResult());
        case 3:
            return engine->toScriptValue(SearchResult(context->argument(0).toString(), context->argument(1).toString(),
                                                      context->argument(2).toString()));
        default:
            return context->throwError(QScriptContext::SyntaxError,
                                       QObject::tr("SearchResult constructor requires either 0 or 3 arguments."));
        }
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newUrlResult(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        switch (context->argumentCount()) {
        case 0:
            return engine->toScriptValue(UrlResult());
        case 2:
            return engine->toScriptValue(UrlResult(context->argument(0).toString(), context->argument(1).toString()));
        default:
            return context->throwError(QScriptContext::SyntaxError,
                    QObject::tr("UrlResult constructor requires either 0 or 2 arguments."));
        }
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QScriptValue JavaScriptPluginGlobalObject::newXMLHttpRequest(QScriptContext *context, QScriptEngine *engine) {
    if (context->isCalledAsConstructor()) {
        XMLHttpRequest *request;
        
        if (JavaScriptPluginGlobalObject *obj =
            qobject_cast<JavaScriptPluginGlobalObject*>(engine->globalObject().toQObject())) {
            request = new XMLHttpRequest(obj->networkAccessManager());
        }
        else {
            request = new XMLHttpRequest;
        }
        
        return engine->newQObject(request, QScriptEngine::ScriptOwnership);
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QNetworkAccessManager* JavaScriptPluginGlobalObject::networkAccessManager() {
    return m_nam ? m_nam : m_nam = new QNetworkAccessManager(this);
}

QString JavaScriptPluginGlobalObject::atob(const QString &ascii) const {
    return QString::fromUtf8(QByteArray::fromBase64(ascii.toUtf8()));
}

QString JavaScriptPluginGlobalObject::btoa(const QString &binary) const {
    return QString::fromUtf8(binary.toUtf8().toBase64());
}

void JavaScriptPluginGlobalObject::clearInterval(int timerId) {
    if (m_intervals.contains(timerId)) {
        m_intervals.remove(timerId);
        killTimer(timerId);
    }
}

void JavaScriptPluginGlobalObject::clearTimeout(int timerId) {
    if (m_timeouts.contains(timerId)) {
        m_timeouts.remove(timerId);
        killTimer(timerId);
    }
}

void JavaScriptPluginGlobalObject::setInterval(const QScriptValue &function, int msecs) {
    if ((function.isFunction()) || (function.isString())) {
        m_intervals[startTimer(msecs)] = function;
    }
}

void JavaScriptPluginGlobalObject::setTimeout(const QScriptValue &function, int msecs) {
    if ((function.isFunction()) || (function.isString())) {
        m_timeouts[startTimer(msecs)] = function;
    }
}

bool JavaScriptPluginGlobalObject::callFunction(QScriptValue function) const {
    if (function.isFunction()) {
        const QScriptValue result = function.call(QScriptValue());

        if (result.isError()) {
            Logger::log("JavaScriptPluginGlobalObject::callFunction(). Error: " + result.toString());
            return false;
        }

        return true;
    }

    if (!m_engine.isNull()) {
        const QScriptValue result = m_engine->globalObject().property(function.toString()).call(QScriptValue());

        if (result.isError()) {
            Logger::log("JavaScriptPluginGlobalObject::callFunction(). Error: " + result.toString());
            return false;
        }

        return true;
    }
    
    return false;
}

void JavaScriptPluginGlobalObject::timerEvent(QTimerEvent *event) {
    if (m_intervals.contains(event->timerId())) {
        if (!callFunction(m_intervals.value(event->timerId()))) {
            clearInterval(event->timerId());
        }
        
        event->accept();
        return;
    }

    if (m_timeouts.contains(event->timerId())) {
        callFunction(m_timeouts.value(event->timerId()));
        clearTimeout(event->timerId());
        event->accept();
    }
}
