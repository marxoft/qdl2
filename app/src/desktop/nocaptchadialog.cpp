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

#include "nocaptchadialog.h"
#include "definitions.h"
#include "utils.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QWebFrame>
#include <QWebPage>
#include <QWebSettings>
#include <QWebView>

class NoCaptchaWebPage : public QWebPage
{
    Q_OBJECT

public:
    explicit NoCaptchaWebPage(QObject *parent = 0) :
        QWebPage(parent)
    {
    }

private:
    virtual QString userAgentForUrl(const QUrl &) const {
        return USER_AGENT;
    }
};

NoCaptchaDialog::NoCaptchaDialog(QWidget *parent) :
    QDialog(parent),
    m_timer(new QTimer(this)),
    m_view(new QWebView(this)),
    m_label(new QLabel(Utils::formatMSecs(CAPTCHA_TIMEOUT), this)),
    m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal, this)),
    m_layout(new QVBoxLayout(this)),
    m_timeout(CAPTCHA_TIMEOUT),
    m_remaining(CAPTCHA_TIMEOUT)
{
    setWindowTitle(tr("Please complete captcha"));

    m_timer->setInterval(1000);

    m_view->setPage(new NoCaptchaWebPage(m_view));

    m_layout->addWidget(m_view);
    m_layout->addWidget(m_label);
    m_layout->addWidget(m_buttonBox);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateTimeRemaining()));
    connect(m_view, SIGNAL(loadStarted()), this, SLOT(onLoadStarted()));
    connect(m_view, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));
    connect(m_view, SIGNAL(titleChanged(QString)), this, SLOT(onTitleChanged(QString)));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

NoCaptchaDialog::~NoCaptchaDialog() {
    QWebSettings::clearMemoryCaches();
}

QString NoCaptchaDialog::html() const {
    return m_view->page()->mainFrame()->toHtml();
}

void NoCaptchaDialog::setHtml(const QString &html, const QUrl &baseUrl) {
    m_view->setHtml(html, baseUrl);
}

QString NoCaptchaDialog::response() const {
    return m_response;
}

void NoCaptchaDialog::setResponse(const QString &response) {
    m_response = response;
}

int NoCaptchaDialog::timeout() const {
    return m_timeout;
}

void NoCaptchaDialog::setTimeout(int t) {
    m_timeout = t;
    m_remaining = t;

    if (t > 0) {
        m_label->show();
        m_timer->start();
    }
    else {
        m_label->hide();
        m_timer->stop();
    }
}

int NoCaptchaDialog::timeRemaining() const {
    return m_remaining;
}

void NoCaptchaDialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);

    if (!m_timer->isActive()) {
        m_remaining = timeout();

        if (m_remaining > 0) {
            m_timer->start();
        }
    }
}

void NoCaptchaDialog::closeEvent(QCloseEvent *event) {
    m_timer->stop();
    QDialog::closeEvent(event);
}

void NoCaptchaDialog::showError(const QString &errorString) {
    QMessageBox::critical(this, tr("Captcha error"), errorString);
}

void NoCaptchaDialog::updateTimeRemaining() {
    m_remaining -= m_timer->interval();
    m_label->setText(Utils::formatMSecs(m_remaining));

    if (m_remaining <= 0) {
        reject();
    }
}

void NoCaptchaDialog::onLoadStarted() {
    m_label->setText(tr("Loading"));
}

void NoCaptchaDialog::onLoadFinished(bool ok) {
    if (ok) {
        m_label->setText(tr("Ready"));
    }
    else {
        m_label->setText(tr("Error"));
    }
}

void NoCaptchaDialog::onTitleChanged(const QString &title) {
    if (title.startsWith("response=")) {
        setResponse(title.mid(9));
        accept();
    }
    else if (title.startsWith("error=")) {
        showError(title.mid(6));
    }
}

#include "moc_nocaptchadialog.cpp"
#include "nocaptchadialog.moc"
