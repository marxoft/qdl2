/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef REQUEST_H
#define REQUEST_H

#include <QObject>
#include <QPointer>
#include <QVariantMap>

class QNetworkAccessManager;

class Request : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString errorString READ errorString NOTIFY finished)
    Q_PROPERTY(Operation operation READ operation WRITE setOperation NOTIFY operationChanged)
    Q_PROPERTY(QVariantMap params READ params WRITE setParams NOTIFY paramsChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QVariant postData READ postData WRITE setPostData NOTIFY postDataChanged)
    Q_PROPERTY(QVariant result READ result NOTIFY finished)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(int statusCode READ statusCode NOTIFY finished)

    Q_ENUMS(Operation Status)

public:
    enum Operation {
        GetOperation = 0,
        DeleteOperation,
        PostOperation,
        PutOperation
    };

    enum Status {
        Null = 0,
        Active,
        Finished,
        Canceled,
        Error
    };

    explicit Request(QObject *parent = 0);
    ~Request();

    QString errorString() const;

    Operation operation() const;
    void setOperation(Operation op);

    QVariantMap params() const;
    void setParams(const QVariantMap &p);

    QString path() const;
    void setPath(const QString &p);

    QVariant postData() const;
    void setPostData(const QVariant &data);

    QVariant result() const;

    Status status() const;

    int statusCode() const;

    static void setNetworkAccessManager(QNetworkAccessManager *manager);

public Q_SLOTS:
    void start();
    void cancel();

    void deleteResource(const QString &path);
    void deleteResource(const QString &path, const QVariantMap &params);
    void get(const QString &path);
    void get(const QString &path, const QVariantMap &params);
    void post(const QString &path, const QVariant &postData);
    void post(const QString &path, const QVariantMap &params, const QVariant &postData);
    void put(const QString &path, const QVariant &postData);
    void put(const QString &path, const QVariantMap &params, const QVariant &postData);

private Q_SLOTS:
    void onReplyFinished();

Q_SIGNALS:
    void finished(Request *r);
    void operationChanged(Request::Operation op);
    void paramsChanged(const QVariantMap &p);
    void pathChanged(const QString &p);
    void postDataChanged(const QVariant &data);
    void statusChanged(Request::Status s);

private:
    void setErrorString(const QString &e);

    void setResult(const QVariant &r);

    void setStatus(Status s);

    void setStatusCode(int code);

    QNetworkAccessManager* networkAccessManager();

    static QPointer<QNetworkAccessManager> nam;

    QString m_errorString;
    QString m_path;

    QVariantMap m_params;

    QVariant m_postData;
    QVariant m_result;

    Operation m_operation;

    Status m_status;

    int m_statusCode;
}; 

#endif // REQUEST_H
