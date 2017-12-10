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

#ifndef JAVASCRIPTPLUGINGLOBALOBJECT_H
#define JAVASCRIPTPLUGINGLOBALOBJECT_H

#include <QObject>
#include <QPointer>
#include <QScriptEngine>

class QNetworkAccessManager;

class JavaScriptPluginGlobalObject : public QObject
{
    Q_OBJECT

public:
    explicit JavaScriptPluginGlobalObject(QScriptEngine *engine);
    
public Q_SLOTS:
    QString atob(const QString &ascii) const;
    QString btoa(const QString &binary) const;

    QString decodeHtml(const QString &text) const;
    QString encodeHtml(const QString &text) const;

    void clearInterval(int timerId);
    void clearTimeout(int timerId);

    int setInterval(const QScriptValue &function, int msecs);
    int setTimeout(const QScriptValue &function, int msecs);

private:
    static QScriptValue newDecaptchaPlugin(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newRecaptchaPlugin(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newSearchPlugin(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newServicePlugin(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newNetworkRequest(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newSearchResult(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newUrlResult(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue newXMLHttpRequest(QScriptContext *context, QScriptEngine *engine);
    
    QNetworkAccessManager* networkAccessManager();
        
    bool callFunction(QScriptValue function) const;
        
    virtual void timerEvent(QTimerEvent *event);

    QPointer<QScriptEngine> m_engine;
    
    QNetworkAccessManager *m_nam;
    
    QHash<int, QScriptValue> m_intervals;
    QHash<int, QScriptValue> m_timeouts;
};

#endif // JAVASCRIPTPLUGINGLOBALOBJECT_H
