/*!
 * \file decaptchaplugin.h
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

#ifndef DECAPTCHAPLUGIN_H
#define DECAPTCHAPLUGIN_H

#include <QObject>
#include <QVariantList>

class QNetworkAccessManager;
class QImage;

/*!
 * The base class for decaptcha plugins.
 */
class DecaptchaPlugin : public QObject
{
    Q_OBJECT

public:
    explicit DecaptchaPlugin(QObject *parent = 0) :
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
     * 
     * This method must be re-implemented to retrieve a captcha response for the captcha with type \a captchaType
     * and data \a captchaData.
     */
    virtual void getCaptchaResponse(int captchaType, const QByteArray &captchaData, const QVariantMap &settings) = 0;

    /*!
     * Pure virtual method.
     * This method must be re-implemented to report the captcha response with id \a captchaId as incorrect.
     */
    virtual void reportCaptchaResponse(const QString &captchaId) = 0;

Q_SIGNALS:
    /*!
     * This signal should be emitted when a captcha response is successfully retrieved.
     */
    void captchaResponse(const QString &captchaId, const QString &response);

    /*!
     * This signal should be emitted when an incorrect captcha response has been successfully reported.
     */
    void captchaResponseReported(const QString &captchaId);

    /*!
     * This signal should be emitted when the plugin is unable to retrieve or report a captcha response.
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
 * Interface for creating instances of DecaptchaPlugin.
 *
 * \sa DecaptchaPlugin
 */
class DecaptchaPluginFactory
{

public:
    virtual ~DecaptchaPluginFactory() {}

    /*!
     * Pure virtual method.
     *
     * This method must be re-implemented to return an instance of DecaptchaPlugin with the parent set to \a parent.
     */
    virtual DecaptchaPlugin* createPlugin(QObject *parent = 0) = 0;
};

Q_DECLARE_INTERFACE(DecaptchaPluginFactory, "org.qdl2.DecaptchaPluginFactory")

#endif // DECAPTCHAPLUGIN_H
