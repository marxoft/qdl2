/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef NOCAPTCHADIALOG_H
#define NOCAPTCHADIALOG_H

#include <QDialog>
#include <QUrl>

class QDialogButtonBox;
class QLabel;
class QTimer;
class QVBoxLayout;
class QWebView;

class NoCaptchaDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QString html READ html WRITE setHtml)
    Q_PROPERTY(QString response READ response)
    Q_PROPERTY(int timeout READ timeout WRITE setTimeout)
    Q_PROPERTY(int timeRemaining READ timeRemaining)

public:
    explicit NoCaptchaDialog(QWidget *parent = 0);

    QString html() const;
    void setHtml(const QString &html, const QUrl &baseUrl = QUrl());

    QString response() const;

    int timeout() const;
    void setTimeout(int timeout);

    int timeRemaining() const;   

private Q_SLOTS:
    void showError(const QString &errorString);
    void updateTimeRemaining();

    void onLoadStarted();
    void onLoadFinished(bool ok);
    void onTitleChanged(const QString &title);

private:
    void setResponse(const QString &response);

    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);

    QTimer *m_timer;
    QWebView *m_view;
    QLabel *m_label;
    QDialogButtonBox *m_buttonBox;
    QVBoxLayout *m_layout;

    QString m_response;

    int m_timeout;
    int m_remaining;
};

#endif // NOCAPTCHADIALOG_H
