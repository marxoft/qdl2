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

#include "externalrecaptchaplugin.h"
#include "json.h"
#include <QImage>
#include <QProcess>

ExternalRecaptchaPlugin::ExternalRecaptchaPlugin(QObject *parent) :
    RecaptchaPlugin(parent),
    m_process(0)
{
}

ExternalRecaptchaPlugin::ExternalRecaptchaPlugin(const QString &fileName, QObject *parent) :
    RecaptchaPlugin(parent),
    m_process(0),
    m_fileName(fileName)
{
}

QString ExternalRecaptchaPlugin::fileName() const {
    return m_fileName;
}

void ExternalRecaptchaPlugin::setFileName(const QString &name) {
    if (name != fileName()) {
        m_fileName = name;
        emit fileNameChanged();
    }
}

RecaptchaPlugin* ExternalRecaptchaPlugin::createPlugin(QObject *parent) {
    return new ExternalRecaptchaPlugin(fileName(), parent);
}

bool ExternalRecaptchaPlugin::cancelCurrentOperation() {
    if (m_process) {
        disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, 0);
        m_process->kill();
    }
    
    return true;
}

void ExternalRecaptchaPlugin::getCaptcha(const QString &captchaKey) {
    initProcess();

    if (m_process->state() != QProcess::NotRunning) {
        return;
    }
    
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onCaptchaReady(int)));
    m_process->start(fileName(), QStringList() << QString("getCaptcha") << captchaKey);
}

void ExternalRecaptchaPlugin::initProcess() {
    if (!m_process) {
        m_process = new QProcess(this);
        connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onProcessError()));
    }
}

void ExternalRecaptchaPlugin::onCaptchaReady(int) {
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onCaptchaReady(int)));
    
    bool ok;
    const QVariant v = QtJson::Json::parse(QString::fromUtf8(m_process->readAllStandardOutput()), ok);

    if (ok) {
        const QVariantMap result = v.toMap();

        if (result.value("status") == "ok") {
            if (result.contains("captcha")) {
                const QVariantMap cap = result.value("captcha").toMap();

                if ((cap.contains("challenge")) && (cap.contains("image"))) {
                    const QString challenge = cap.value("challenge").toString();
                    const QImage image = QImage::fromData(QByteArray::fromBase64(cap.value("image").toByteArray()));

                    if (challenge.isEmpty()) {
                        emit error(tr("No captcha challenge"));
                    }
                    else if (image.isNull()) {
                        emit error(tr("Captcha image is null"));
                    }
                    else {
                        emit captcha(challenge, image);
                    }

                    return;
                }
            }

            emit error(tr("No captcha image"));
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

void ExternalRecaptchaPlugin::onProcessError() {
    emit error(m_process->errorString());
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, 0);
}
