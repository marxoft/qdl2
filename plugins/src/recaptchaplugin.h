/*!
 * \file recaptchaplugin.h
 *
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

#ifndef RECAPTCHAPLUGIN_H
#define RECAPTCHAPLUGIN_H

#include <QObject>
#include <QVariantList>

class QImage;
class QNetworkAccessManager;

/*!
 * The base class for recaptcha plugins.
 */
class RecaptchaPlugin : public QObject
{
    Q_OBJECT

public:
    explicit RecaptchaPlugin(QObject *parent = 0) :
        QObject(parent)
    {
    }

    /*!
     * Allows the plugin to share an application QNetworkNetworkManager instance.
     * 
     * The base implementation does nothing. If you choose to re-implement this function,
     * the plugin should not take ownership of the QNetworkAccessManager instance.
     * 
     * The QNetworkAccessManager is guaranteed to remain valid for the lifetime of the plugin.
     */
    virtual void setNetworkAccessManager(QNetworkAccessManager*) {}

public Q_SLOTS:
    /*!
     * Pure virtual method.
     *
     * This method must be re-implemented to return \c true if the current operation (if any) has been successfully 
     * canceled.
     */
    virtual bool cancelCurrentOperation() = 0;

    /*!
     * Pure virtual method.
     * This method must be re-implemented to retrieve a captcha of type \a captchaType for the \a captchaKey.
     */
    virtual void getCaptcha(int captchaType, const QString &captchaKey, const QVariantMap &settings) = 0;

Q_SIGNALS:
    /*!
     * This signal should be emitted when a captcha is successfully retrieved.
     */
    void captcha(int captchaType, const QByteArray &captchaData);

    /*!
     * This signal should be emitted when the plugin is unable to retrieve a captcha.
     */
    void error(const QString &errorString);

    /*!
     * This signal should be emitted when some additional data is required to complete an operation.
     *         
     * The \a callback method will be called when the user submits the settings and should take a QVariantMap
     * as its sole argument.
     */
    void settingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback);
};

/*!
 * Interface for creating instances of RecaptchaPlugin.
 *
 * \sa RecaptchaPlugin
 */
class RecaptchaPluginFactory
{

public:
    virtual ~RecaptchaPluginFactory() {}

    /*!
     * Pure virtual method.
     *
     * This method must be re-implemented to return an instance of RecaptchaPlugin with the parent set to \a parent.
     */
    virtual RecaptchaPlugin* createPlugin(QObject *parent = 0) = 0;
};

Q_DECLARE_INTERFACE(RecaptchaPluginFactory, "org.qdl2.RecaptchaPluginFactory")

#endif // RECAPTCHAPLUGIN_H
