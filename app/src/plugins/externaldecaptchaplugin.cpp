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

#include "externaldecaptchaplugin.h"
#include "json.h"
#include <QDataStream>
#include <QImage>
#include <QProcess>

ExternalDecaptchaPlugin::ExternalDecaptchaPlugin(QObject *parent) :
    DecaptchaPlugin(parent),
    m_process(0)
{
}

ExternalDecaptchaPlugin::ExternalDecaptchaPlugin(const QString &fileName, QObject *parent) :
    DecaptchaPlugin(parent),
    m_process(0),
    m_fileName(fileName)
{
}

QString ExternalDecaptchaPlugin::fileName() const {
    return m_fileName;
}

void ExternalDecaptchaPlugin::setFileName(const QString &name) {
    if (name != fileName()) {
        m_fileName = name;
        emit fileNameChanged();
    }
}

DecaptchaPlugin* ExternalDecaptchaPlugin::createPlugin(QObject *parent) {
    return new ExternalDecaptchaPlugin(fileName(), parent);
}

bool ExternalDecaptchaPlugin::cancelCurrentOperation() {
    if (m_process) {
        disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, 0);
        m_process->kill();
    }
    
    return true;
}

void ExternalDecaptchaPlugin::getCaptchaResponse(const QImage &image) {
    initProcess();

    if (m_process->state() != QProcess::NotRunning) {
        return;
    }

    if (image.isNull()) {
        emit error(tr("Image is null"));
        return;
    }

    QByteArray ba;
    QDataStream stream(&ba, QIODevice::WriteOnly);
    stream << image;
    
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onCaptchaResponse(int)));
    m_process->start(fileName(), QStringList() << QString("getCaptchaResponse") << QString::fromUtf8(ba.toBase64()));
}

void ExternalDecaptchaPlugin::reportCaptchaResponse(const QString &captchaId) {
    initProcess();

    if (m_process->state() != QProcess::NotRunning) {
        return;
    }
    
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onDownloadRequestFinished(int)));
    m_process->start(fileName(), QStringList() << QString("reportCaptchaResponse") << captchaId);
}

void ExternalDecaptchaPlugin::initProcess() {
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onProcessError()));
    }
}

void ExternalDecaptchaPlugin::onCaptchaResponse(int) {
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onCaptchaResponse(int)));
    
    bool ok;
    const QVariant v = QtJson::Json::parse(QString::fromUtf8(m_process->readAllStandardOutput()), ok);

    if (ok) {
        const QVariantMap result = v.toMap();

        if (result.value("status") == "ok") {
            if (result.contains("captcha")) {
                const QVariantMap captcha = result.value("captcha").toMap();

                if ((captcha.contains("id")) && (captcha.contains("response"))) {
                    const QString captchaId = captcha.value("id").toString();
                    const QString response = captcha.value("response").toString();

                    if ((!captchaId.isEmpty()) && (!response.isEmpty())) {
                        emit captchaResponse(captchaId, response);
                        return;
                    }
                }
            }

            emit error(tr("No captcha response"));
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

void ExternalDecaptchaPlugin::onCaptchaReported(int) {
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onCaptchaReported(int)));
    
    bool ok;
    const QVariant v = QtJson::Json::parse(QString::fromUtf8(m_process->readAllStandardOutput()), ok);

    if (ok) {
        const QVariantMap result = v.toMap();

        if (result.value("status") == "ok") {
            if (result.contains("id")) {
                const QString captchaId = result.value("id").toString();

                if (!captchaId.isEmpty()) {
                    emit captchaResponseReported(captchaId);
                    return;
                }
            }

            emit error(tr("No captcha ID"));
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

void ExternalDecaptchaPlugin::onProcessError() {
    emit error(m_process->errorString());
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, 0);
}
