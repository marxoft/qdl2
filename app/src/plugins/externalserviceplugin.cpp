/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "externalserviceplugin.h"
#include "json.h"
#include <QNetworkRequest>
#include <QProcess>

ExternalServicePlugin::ExternalServicePlugin(QObject *parent) :
    ServicePlugin(parent),
    m_process(0)
{
}

ExternalServicePlugin::ExternalServicePlugin(const QString &fileName, QObject *parent) :
    ServicePlugin(parent),
    m_process(0),
    m_fileName(fileName)
{
}

QString ExternalServicePlugin::fileName() const {
    return m_fileName;
}

void ExternalServicePlugin::setFileName(const QString &name) {
    if (name != fileName()) {
        m_fileName = name;
        emit fileNameChanged();
    }
}

ServicePlugin* ExternalServicePlugin::createPlugin(QObject *parent) {
    return new ExternalServicePlugin(fileName(), parent);
}

bool ExternalServicePlugin::cancelCurrentOperation() {
    if (m_process) {
        disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, 0);
        m_process->kill();
    }
    
    return true;
}

void ExternalServicePlugin::checkUrl(const QString &url) {
    initProcess();

    if (m_process->state() != QProcess::NotRunning) {
        return;
    }
    
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onUrlChecked(int)));
    m_process->start(fileName(), QStringList() << QString("checkUrl") << url);
}

void ExternalServicePlugin::getDownloadRequest(const QString &url) {
    initProcess();

    if (m_process->state() != QProcess::NotRunning) {
        return;
    }
    
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onDownloadRequestFinished(int)));
    m_process->start(fileName(), QStringList() << QString("getDownloadRequest") << url);
}

void ExternalServicePlugin::submitCaptchaResponse(const QString &challenge, const QString &response) {
    initProcess();

    if (m_process->state() != QProcess::NotRunning) {
        return;
    }
    
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onDownloadRequestFinished(int)));
    m_process->start(fileName(), QStringList() << QString("submitCaptchaResponse") << challenge << response);
}

void ExternalServicePlugin::initProcess() {
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onProcessError()));
    }
}

void ExternalServicePlugin::onUrlChecked(int) {
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onUrlChecked(int)));
    bool ok;
    const QVariant v = QtJson::Json::parse(QString::fromUtf8(m_process->readAllStandardOutput()), ok);

    if (ok) {
        const QVariantMap result = v.toMap();

        if (result.value("status") == "ok") {
            QVariantList results = result.value("results").toList();
            
            if (results.isEmpty()) {
                emit error(tr("URL list is empty"));
                return;
            }
            
            UrlResultList urls;
            
            while (!results.isEmpty()) {
                const QVariantMap r = results.takeFirst().toMap();
                urls << UrlResult(r.value("url").toString(), r.value("fileName").toString());
            }
            
            emit urlChecked(urls);
            return;
        }

        if (result.contains("error")) {
            const QString errorString = result.value("error").toString();
            
            if (!errorString.isEmpty()) {
                emit error(errorString);
                return;
            }
        }
    }    
    
    emit error(tr("Unknown error"));
}

void ExternalServicePlugin::onDownloadRequestFinished(int) {
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onDownloadRequestFinished(int)));
    bool ok;
    const QVariant v = QtJson::Json::parse(QString::fromUtf8(m_process->readAllStandardOutput()), ok);

    if (ok) {
        const QVariantMap result = v.toMap();
        const QString status = result.value("status").toString();

        if (status == "ok") {
            if (result.contains("request")) {
                const QVariantMap request = result.value("request").toMap();

                if (request.contains("url")) {
                    const QUrl url = request.value("url").toUrl();

                    if (!url.isEmpty()) {
                        const QByteArray method = request.contains("method") ? request.value("method").toByteArray() : QByteArray("GET");
                        const QByteArray data = request.value("data").toByteArray();
                        QNetworkRequest nr(url);
                        
                        if (request.contains("headers")) {
                            const QVariantMap headers = request.value("headers").toMap();
                            QMapIterator<QString, QVariant> iterator(headers);
                            
                            while (iterator.hasNext()) {
                                iterator.next();
                                nr.setRawHeader(iterator.key().toUtf8(), iterator.value().toByteArray());
                            }
                        }
                        
                        emit downloadRequest(nr, method, data);
                        return;
                    }
                }
            }

            emit error(tr("Invalid download request"));
        }
        else if (status == "captcha") {
            if (result.contains("captcha")) {
                const QVariantMap captcha = result.value("captcha").toMap();
                
                if ((captcha.contains("id")) && (captcha.contains("key"))) {
                    const QString recaptchaId = captcha.value("id").toString();
                    const QString recaptchaKey = captcha.value("key").toString();
                    
                    if ((!recaptchaId.isEmpty()) && (!recaptchaKey.isEmpty())) {
                        emit captchaRequest(recaptchaId, recaptchaKey);
                        return;
                    }
                }
            }

            emit error(tr("Invalid captcha request"));
       }
       else if (status == "wait") {
           if (result.contains("wait")) {
               const QVariantMap wait = result.value("wait").toMap();
               
               if (wait.contains("duration")) {
                   const int waitTime = wait.value("duration").toInt();
                  
                   if (waitTime > 0) {
                       emit waitRequest(waitTime, wait.value("longDelay", true).toBool());
                       return;
                   }
               }
           }
           
           emit error(tr("Invalid wait time"));
       }       
       else if (result.contains("error")) {
           const QString errorString = result.value("error").toString();

           if (!errorString.isEmpty()) {
               emit error(errorString);
               return;
           }
       }
    }

    emit error(tr("Unknown error"));
}

void ExternalServicePlugin::onProcessError() {
    emit error(m_process->errorString());
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, 0);
}
