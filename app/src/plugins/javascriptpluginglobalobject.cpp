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
                                                 QScriptEngine::ExcludeChildObjects
                                                 | QScriptEngine::ExcludeDeleteLater);

    thisGlobal.setProperty("XMLHttpRequest", engine->newQMetaObject(&XMLHttpRequest::staticMetaObject,
                           engine->newFunction(newXMLHttpRequest)));
    
    QScriptValueIterator iterator(oldGlobal);

    while (iterator.hasNext()) {
        iterator.next();
        thisGlobal.setProperty(iterator.name(), iterator.value());
    }

    engine->setGlobalObject(thisGlobal);
}

QScriptValue JavaScriptPluginGlobalObject::newXMLHttpRequest(QScriptContext *context, QScriptEngine *engine) {
    XMLHttpRequest *request;
    
    if (JavaScriptPluginGlobalObject *obj =
        qobject_cast<JavaScriptPluginGlobalObject*>(engine->globalObject().toQObject())) {
        request = new XMLHttpRequest(obj->networkAccessManager(), context->argument(0).toQObject());
    }
    else {
        request = new XMLHttpRequest(context->argument(0).toQObject());
    }
    
    return engine->newQObject(request, QScriptEngine::ScriptOwnership);
}

QNetworkAccessManager* JavaScriptPluginGlobalObject::networkAccessManager() {
    return m_nam ? m_nam : m_nam = new QNetworkAccessManager(this);
}

void JavaScriptPluginGlobalObject::setNetworkAccessManager(QNetworkAccessManager *manager) {
    if (manager) {
        m_nam = manager;
    }
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
