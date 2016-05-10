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

#ifndef EXTERNALRECAPTCHAPLUGIN_H
#define EXTERNALRECAPTCHAPLUGIN_H

#include "recaptchaplugin.h"

class QProcess;

class ExternalRecaptchaPlugin : public RecaptchaPlugin
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)

public:
    explicit ExternalRecaptchaPlugin(QObject *parent = 0);
    explicit ExternalRecaptchaPlugin(const QString &fileName, QObject *parent = 0);

    QString fileName() const;
    void setFileName(const QString &name);

    virtual RecaptchaPlugin* createPlugin(QObject *parent = 0);

public Q_SLOTS:
    virtual bool cancelCurrentOperation();
    
    virtual void getCaptcha(const QString &captchaKey);

private Q_SLOTS:
    void onCaptchaReady(int statusCode);
    void onProcessError();

Q_SIGNALS:
    void fileNameChanged();

private:
    void initProcess();
    
    QProcess *m_process;

    QString m_fileName;
};

#endif // EXTERNALRECAPTCHAPLUGIN_H
